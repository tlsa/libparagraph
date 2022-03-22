/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2019 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Paragraph content handling.
 */

#include <stdlib.h>
#include <string.h>

#include <paragraph.h>

#include "content.h"
#include "style.h"
#include "para.h"
#include "ctx.h"
#include "log.h"

static void paragraph__content_entry_cleanup(
		paragraph_content_entry_t *content_entry)
{
	content_entry->style = paragraph_style__unref(content_entry->style);
}

/* Internally exported function, documented in `src/content.h` */
paragraph_err_t paragraph__content_destroy(
		paragraph_content_t *content)
{
	while (content->first != NULL) {
		paragraph_content_entry_t *content_entry = content->first;

		paragraph__content_entry_cleanup(content_entry);

		content->first = content_entry->next;
		free(content_entry);
	}

	content->count = 0;
	content->last = NULL;

	free(content->text);
	content->text = NULL;
	content->len = 0;

	return PARAGRAPH_OK;
}

/**
 * Get a new empty content entry at the end of the content entries array.
 *
 * \param[in]  para         The paragraph context to get new content entry for.
 * \param[out] entries_out  Returns pointer to new content entry on success.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
static paragraph_err_t paragraph__content_entry_get_new(
		paragraph_para_t *para,
		const paragraph_content_position_t *pos,
		paragraph_content_entry_t **entry_out)
{
	paragraph_content_t *content;
	paragraph_content_entry_t *entry;

	content = &para->content;

	entry = calloc(1, sizeof(*entry));
	if (entry == NULL) {
		return PARAGRAPH_ERR_OOM;
	}

	if (content->count == 0) {
		assert(content->first == NULL);
		assert(content->last == NULL);

		content->first = entry;
		content->last = entry;
	} else {
		paragraph_content_entry_t *rel = content->last;
		paragraph_content_pos_t place = PARAGRAPH_CONTENT_POS_AFTER;

		if (pos != NULL && pos->rel != NULL) {
			rel = (paragraph_content_entry_t *) pos->rel;
			place = pos->pos;
		}

		if (place == PARAGRAPH_CONTENT_POS_BEFORE) {
			if (content->first == rel) {
				content->first = entry;
			}

			entry->prev = rel->prev;
			entry->next = rel;
			rel->prev = entry;
		} else {
			if (content->last == rel) {
				content->last = entry;
			}

			entry->prev = rel;
			entry->next = rel->next;
			rel->next = entry;
		}
	}

	content->count++;

	*entry_out = entry;
	return PARAGRAPH_OK;
}

/**
 * Create a content entry in a paragraph.
 */
paragraph_err_t paragraph_content_add(
		paragraph_para_t *para,
		const paragraph_content_params_t *params,
		const paragraph_content_position_t *pos,
		paragraph_content_id_t **new)
{
	paragraph_err_t err;
	paragraph_content_entry_t *entry;
	enum paragraph_content_type_e type = params->type;

	err = paragraph__content_entry_get_new(para, pos, &entry);
	if (err != PARAGRAPH_OK) {
		return err;
	}

	entry->pw = params->pw;
	entry->type = type;

	paragraph__log(para->ctx->config, LOG_INFO,"%p: Add '%s'",
			params->pw, paragraph__content_typestr(type));

	switch (type) {
		case PARAGRAPH_CONTENT_TEXT:
			entry->text.string = params->text.string;
			para->ctx->cb_text->text_get(
					para->ctx->pw,
					params->text.string,
					&entry->text.data,
					&entry->text.len);

			para->content.len += entry->text.len;
			entry->style = paragraph_style__get_current(
					&para->styles);

			paragraph__log(para->ctx->config, LOG_INFO,
					"        content (%zu): \"%.*s\"",
					entry->text.len,
					(int)entry->text.len,
					entry->text.data);
			break;

		case PARAGRAPH_CONTENT_FLOAT:
			entry->style = paragraph_style__ref(
					params->floated.style);
			break;

		case PARAGRAPH_CONTENT_REPLACED:
			entry->replaced.px_width = params->replaced.px_width;
			entry->replaced.px_height = params->replaced.px_height;

			entry->style = paragraph_style__ref(
					params->replaced.style);
			break;

		case PARAGRAPH_CONTENT_INLINE_START:
			entry->style = paragraph_style__ref(
					params->inline_start.style);
			break;

		case PARAGRAPH_CONTENT_INLINE_END:
			entry->style = paragraph_style__get_current(
					&para->styles);
			break;

		default:
			return PARAGRAPH_ERR_BAD_TYPE;
	}

	*new = (void *)entry;
	return PARAGRAPH_OK;
}

paragraph_err_t paragraph_content__get_text(
		paragraph_para_t *para,
		const char **text_out,
		size_t *len_out)
{
	paragraph_content_t *content = &para->content;
	char *text;

	/** TODO: Cache? */
	/** TODO: Partial changes? */

	text = realloc(content->text, content->len);
	if (text == NULL) {
		return PARAGRAPH_ERR_OOM;
	}

	content->text = text;
	for (paragraph_content_entry_t *e = content->first;
			e != NULL; e = e->next) {
		if (e->type != PARAGRAPH_CONTENT_TEXT) {
			continue;
		}
		memcpy(text, e->text.data, e->text.len);
		text += e->text.len;
	}

	*text_out = content->text;
	*len_out = content->len;
	return PARAGRAPH_OK;
}
