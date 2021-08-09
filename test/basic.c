#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <dom/dom.h>
#include <dom/walk.h>

#include <libcss/libcss.h>

#include <paragraph.h>

#include <sd/sd.h>
#include <sd/style.h>

#define UNUSED(_v) ((void)(_v))
#define SLEN(_s) (sizeof(_s) - 1)

struct paragraph_sd_ctx {
	dom_string *str_key_paragraph;
	dom_string *str_head;
	dom_string *str_p;
} paragraph_sd_g;

struct text {
	char *str;
	size_t len;
};

paragraph_err_t bitmap_font_measure_text(
		void *pw,
		const paragraph_text_t *text,
		const paragraph_style_t *style,
		uint32_t *width_out,
		uint32_t *height_out,
		uint32_t *baseline_out)
{
	UNUSED(pw);
	UNUSED(style);

	if (text == NULL) {
		return PARAGRAPH_ERR_BAD_PARAM;
	}

	*width_out = text->len * 8;
	*height_out = 16;
	*baseline_out = 12;

	return PARAGRAPH_OK;
}

paragraph_err_t simple_string_text_get(
		void *pw,
		const paragraph_string_t *text,
		const char **data_out,
		char *len_out)
{
	UNUSED(pw);

	if (text == NULL) {
		return PARAGRAPH_ERR_BAD_PARAM;
	}

	*data_out = text;
	*len_out = strlen(text);

	return PARAGRAPH_OK;
}

paragraph_cb_text_t cb_text = {
	.measure_text = bitmap_font_measure_text,
	.text_get     = simple_string_text_get,
};

static void paragraph_sd_ctx_fini(void)
{
	if (paragraph_sd_g.str_key_paragraph != NULL) {
		dom_string_unref(paragraph_sd_g.str_key_paragraph);
		paragraph_sd_g.str_key_paragraph = NULL;
	}

	if (paragraph_sd_g.str_head != NULL) {
		dom_string_unref(paragraph_sd_g.str_head);
		paragraph_sd_g.str_head = NULL;
	}

	if (paragraph_sd_g.str_p != NULL) {
		dom_string_unref(paragraph_sd_g.str_p);
		paragraph_sd_g.str_p = NULL;
	}
}

static bool paragraph_sd_ctx_init(void)
{
	dom_exception derr;

	derr = dom_string_create_interned(
			(const uint8_t *)"_key_paragraph",
			SLEN("_key_paragraph"),
			&paragraph_sd_g.str_key_paragraph);
	if ((derr != DOM_NO_ERR) ||(paragraph_sd_g.str_key_paragraph == NULL)) {
		goto error;
	}

	derr = dom_string_create_interned(
			(const uint8_t *)"head", SLEN("head"),
			&paragraph_sd_g.str_head);
	if ((derr != DOM_NO_ERR) || (paragraph_sd_g.str_head == NULL)) {
		goto error;
	}

	derr = dom_string_create_interned(
			(const uint8_t *)"p", SLEN("p"),
			&paragraph_sd_g.str_p);
	if ((derr != DOM_NO_ERR) || (paragraph_sd_g.str_p == NULL)) {
		goto error;
	}

	return true;

error:
	paragraph_sd_ctx_fini();
	return false;
}

static void paragraph_sd__dom_user_data_handler(dom_node_operation operation,
		dom_string *key, void *data, struct dom_node *src,
		struct dom_node *dst)
{
	UNUSED(src);
	UNUSED(dst);

	if (data == NULL) {
		return;
	}

	if (dom_string_isequal(paragraph_sd_g.str_key_paragraph, key)) {
		/* TODO: Support other operations. */
		assert(operation == DOM_NODE_DELETED);
		paragraph_destroy(data);
	}
}

static bool paragraph_sd_create(
		paragraph_ctx_t *ctx,
		dom_node_type type,
		dom_node *node)
{
	bool res;
	void *old_para;
	dom_exception derr;
	paragraph_err_t err;
	paragraph_para_t *para;
	css_select_results *style;

	assert(type == DOM_ELEMENT_NODE);

	res = sd_style_get(node, type, &style);
	if (res != true || style == NULL) {
		fprintf(stderr, "%s: Failed to get paragraph style\n",
				__func__);
		return false;
	}

	err = paragraph_create(NULL, ctx, &para, style);
	if (err != PARAGRAPH_OK) {
		fprintf(stderr, "%s: Failed to create paragraph context: %s\n",
				__func__, paragraph_strerror(err));
		return false;
	}

	derr = dom_node_set_user_data(node, paragraph_sd_g.str_key_paragraph,
			para, paragraph_sd__dom_user_data_handler,
			(void *) &old_para);
	if (derr != DOM_NO_ERR) {
		para = paragraph_destroy(para);
		return false;
	}
	assert(old_para == NULL);

	return true;
}

static bool paragraph_sd_get_para(
		dom_node *node,
		paragraph_para_t **para_out)
{
	paragraph_para_t *para = NULL;
	dom_node *next;

	next = dom_node_ref(node);
	while (next != NULL) {
		dom_exception derr;

		node = next;
		derr = dom_node_get_user_data(node,
				paragraph_sd_g.str_key_paragraph,
				&para);
		if (derr == DOM_NO_ERR && para != NULL) {
			dom_node_unref(node);
			*para_out = para;
			return true;
		}

		derr = dom_node_get_parent_node(node, &next);
		dom_node_unref(node);
		if (derr != DOM_NO_ERR) {
			return false;
		}
	}

	return false;
}

static bool paragraph_sd_add_text(
		dom_node_type type,
		dom_node *node)
{
	bool res;
	dom_exception derr;
	dom_string *content;
	paragraph_err_t err;
	paragraph_para_t *para;

	assert(type == DOM_TEXT_NODE);

	res = paragraph_sd_get_para(node, &para);
	if (res != true) {
		return res;
	}

	derr = dom_characterdata_get_data(node, &content);
	if (derr != DOM_NO_ERR || content == NULL) {
		fprintf(stderr, "%s: Failed to get text's character data\n",
				__func__);
		return false;
	}

	/* TODO: pass ref in, and allow libparagraph to unref */
	dom_string_unref(content);

	err = paragraph_content_add_text(para, content, node);
	if (err != PARAGRAPH_OK) {
		fprintf(stderr, "%s: Failed to add text: %s\n",
				__func__, paragraph_strerror(err));
		return false;
	}

	return true;
}

static bool paragraph_sd_add_start(
		dom_node_type type,
		dom_node *node)
{
	bool res;
	paragraph_err_t err;
	paragraph_para_t *para;
	css_select_results *style;

	assert(type == DOM_ELEMENT_NODE);

	res = sd_style_get(node, type, &style);
	if (res != true || style == NULL) {
		fprintf(stderr, "%s: Failed to get paragraph style\n",
				__func__);
		return false;
	}

	res = paragraph_sd_get_para(node, &para);
	if (res != true) {
		/* Assume this element isn't inside a <p>. */
		return true;
	}

	err = paragraph_content_add_inline_start(para, node, style);
	if (err != PARAGRAPH_OK) {
		fprintf(stderr, "%s: Failed to add text: %s\n",
				__func__, paragraph_strerror(err));
		return false;
	}

	return true;
}

static bool paragraph_sd_add_end(
		dom_node_type type,
		dom_node *node)
{
	bool res;
	paragraph_err_t err;
	paragraph_para_t *para;

	assert(type == DOM_ELEMENT_NODE);

	res = paragraph_sd_get_para(node, &para);
	if (res != true) {
		/* Assume this element isn't inside a <p>. */
		return true;
	}

	err = paragraph_content_add_inline_end(para, node);
	if (err != PARAGRAPH_OK) {
		fprintf(stderr, "%s: Failed to add text: %s\n",
				__func__, paragraph_strerror(err));
		return false;
	}

	return true;
}

static enum dom_walk_cmd paragraph_sd_cb(
		enum dom_walk_stage stage,
		dom_node_type type,
		dom_node *node,
		void *pw)
{
	paragraph_ctx_t *ctx = pw;
	dom_exception derr;
	dom_string *name;
	bool res;

	switch (type) {
	case DOM_ELEMENT_NODE:
		derr = dom_node_get_node_name(node, &name);
		if ((derr != DOM_NO_ERR) || (name == NULL)) {
			return DOM_WALK_CMD_ABORT;
		}

		switch (stage) {
		case DOM_WALK_STAGE_ENTER:
			if (dom_string_caseless_isequal(name,
					paragraph_sd_g.str_head)) {
				dom_string_unref(name);
				return DOM_WALK_CMD_SKIP;
			}

			if (dom_string_caseless_isequal(name,
					paragraph_sd_g.str_p)) {
				res = paragraph_sd_create(ctx, type, node);
				if (res != true) {
					dom_string_unref(name);
					return DOM_WALK_CMD_ABORT;
				}
			} else {
				res = paragraph_sd_add_start(type, node);
				if (res != true) {
					dom_string_unref(name);
					return DOM_WALK_CMD_ABORT;
				}
			}
			break;

		case DOM_WALK_STAGE_LEAVE:
			res = paragraph_sd_add_end(type, node);
			if (res != true) {
				dom_string_unref(name);
				return DOM_WALK_CMD_ABORT;
			}
			break;
		}

		dom_string_unref(name);
		break;

	case DOM_TEXT_NODE:
		if (stage == DOM_WALK_STAGE_ENTER) {
			res = paragraph_sd_add_text(type, node);
			if (res != true) {
				return res;
			}
		}
		break;

	default:
		break;
	}

	return DOM_WALK_CMD_CONTINUE;
}

static bool paragraph_sd(
		paragraph_ctx_t *ctx,
		dom_document *doc)
{
	dom_exception derr;
	dom_node *root;

	derr = dom_document_get_document_element(doc, &root);
	if (derr != DOM_NO_ERR || root == NULL) {
		fprintf(stderr, "%s: Failed to get root element\n", __func__);
		return false;
	}

	derr = libdom_treewalk(
			DOM_WALK_ENABLE_ALL,
			paragraph_sd_cb,
			root, ctx);
	dom_node_unref(root);
	if (derr != DOM_NO_ERR) {
		fprintf(stderr, "%s: Failed to walk DOM\n", __func__);
		return false;
	}

	return true;
}

static bool run_test(
		paragraph_ctx_t *ctx,
		const char *css,
		const char *html)
{
	dom_document *doc;
	bool res;

	res = paragraph_sd_ctx_init();
	if (res != true) {
		return res;
	}

	res = sd_load_data(css, html, &doc);
	if (res != true) {
		paragraph_sd_ctx_fini();
		return res;
	}

	res = paragraph_sd(ctx, doc);
	sd_free(doc);
	if (res == false) {
		paragraph_sd_ctx_fini();
		return res;
	}

	paragraph_sd_ctx_fini();
	return true;
}

static bool example_latin(void)
{
	bool res;
	const char *html =
			"<html>\n"
			"  <head><title>Title</title></head>\n"
			"  <body>\n"
			"    <p>This is a <em>simple</em> test!\n"
			"  </body>\n"
			"</html>\n";
	const char *css =
			"p > em {font_size: 200%;}\n";
	paragraph_config_t para_config = {
		.log_fn = NULL,
	};
	paragraph_ctx_t *ctx;
	paragraph_err_t err;

	err = paragraph_ctx_create(NULL, &ctx, &para_config, &cb_text);
	if (err != PARAGRAPH_OK) {
		fprintf(stderr, "Failed to create paragraph context: %s\n",
				paragraph_strerror(err));
		return false;
	}

	res = run_test(ctx, css, html);
	ctx = paragraph_ctx_destroy(ctx);

	return res;
}

int main(int argc, char *argv[])
{
	bool res;

	UNUSED(argc);
	UNUSED(argv);

	if (!sd_init(
			"html, body, p { display: block }"
			"head { display: none }",
			false)) {
		return EXIT_FAILURE;
	}

	res = example_latin();
	sd_fini();
	if (res != true) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
