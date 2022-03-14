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
	/* TODO: Free anything we own in each entry. */
	for (size_t i = 0; i < content->entries_used; i++) {
		paragraph__content_entry_cleanup(&content->entries[i]);
	}

	free(content->entries);
	content->entries = NULL;
	content->entries_used = 0;
	content->entries_alloc = 0;

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
		paragraph_content_entry_t **entry_out)
{
	uint32_t entries_alloc;
	paragraph_content_t *content;
	paragraph_content_entry_t *entries;

	content = &para->content;

	if (content->entries_alloc <= content->entries_used) {
		if (content->entries_alloc == 0) {
			entries_alloc = 8;
		} else {
			entries_alloc = content->entries_alloc * 2;
		}

		entries = realloc(content->entries,
				entries_alloc * sizeof(*entries));
		if (entries == NULL) {
			return PARAGRAPH_ERR_OOM;
		}

		content->entries = entries;
		content->entries_alloc = entries_alloc;
	}

	*entry_out = content->entries + content->entries_used++;
	(*entry_out)->type = PARAGRAPH_CONTENT_NONE;

	return PARAGRAPH_OK;
}

/* Exported function, documented in `include/paragraph.h` */
paragraph_err_t paragraph_content_add_text(
		paragraph_para_t *para,
		const paragraph_string_t *text,
		void *handle)
{
	paragraph_err_t err;
	paragraph_content_entry_t *entry;

	err = paragraph__content_entry_get_new(para, &entry);
	if (err != PARAGRAPH_OK) {
		return err;
	}

	entry->type = PARAGRAPH_CONTENT_TEXT;
	entry->text.string = text;
	entry->handle = handle;
	entry->style = paragraph_style__get_current(&para->styles);

	para->ctx->cb_text->text_get(para->ctx->pw, text,
			&entry->text.data,
			&entry->text.len);

	paragraph__log(para->ctx->config, LOG_INFO,
			"%p: Add text (%zu): \"%.*s\"",
			handle, entry->text.len,
			(int)entry->text.len,
			entry->text.data);

	para->content.len += entry->text.len;

	return PARAGRAPH_OK;
}

/* Exported function, documented in `include/paragraph.h` */
paragraph_err_t paragraph_content_add_replaced(
		paragraph_para_t *para,
		uint32_t px_width,
		uint32_t px_height,
		void *handle,
		paragraph_style_t *style)
{
	paragraph_err_t err;
	paragraph_content_entry_t *entry;

	err = paragraph__content_entry_get_new(para, &entry);
	if (err != PARAGRAPH_OK) {
		return err;
	}

	entry->type = PARAGRAPH_CONTENT_REPLACED;
	entry->replaced.px_width = px_width;
	entry->replaced.px_height = px_height;
	entry->handle = handle;
	entry->style = style;

	return PARAGRAPH_OK;
}

/* Exported function, documented in `include/paragraph.h` */
paragraph_err_t paragraph_content_add_float(
		paragraph_para_t *para,
		void *handle,
		paragraph_style_t *style)
{
	paragraph_err_t err;
	paragraph_content_entry_t *entry;

	err = paragraph__content_entry_get_new(para, &entry);
	if (err != PARAGRAPH_OK) {
		return err;
	}

	entry->type = PARAGRAPH_CONTENT_FLOAT;
	entry->handle = handle;
	entry->style = style;

	return PARAGRAPH_OK;
}

/* Exported function, documented in `include/paragraph.h` */
paragraph_err_t paragraph_content_add_inline_start(
		paragraph_para_t *para,
		void *handle,
		paragraph_style_t *style)
{
	paragraph_err_t err;
	paragraph_content_entry_t *entry;

	err = paragraph__content_entry_get_new(para, &entry);
	if (err != PARAGRAPH_OK) {
		return err;
	}

	entry->type = PARAGRAPH_CONTENT_INLINE_START;
	entry->handle = handle;
	entry->style = paragraph_style__ref(style);

	paragraph__log(para->ctx->config, LOG_INFO, "%p: Add inline start!", handle);
	return paragraph_style__push(&para->styles, style);
}

/* Exported function, documented in `include/paragraph.h` */
paragraph_err_t paragraph_content_add_inline_end(
		paragraph_para_t *para,
		void *handle)
{
	paragraph_err_t err;
	paragraph_style_t *style;
	paragraph_content_entry_t *entry;

	err = paragraph__content_entry_get_new(para, &entry);
	if (err != PARAGRAPH_OK) {
		return err;
	}

	err = paragraph_style__pop(&para->styles, &style);
	if (err != PARAGRAPH_OK) {
		return err;
	}

	entry->type = PARAGRAPH_CONTENT_INLINE_END;
	entry->handle = handle;
	entry->style = paragraph_style__ref(style);

	paragraph__log(para->ctx->config, LOG_INFO, "%p: Add inline end!", handle);
	return PARAGRAPH_OK;
}
