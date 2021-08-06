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
#include "ctx.h"

/* Internally exported function, documented in `src/content.h` */
paragraph_err_t paragraph__content_destroy(
		paragraph_content_t *content)
{
	/* TODO: Free anything we own in each entry. */

	free(content->entries);
	content->entries = NULL;
	content->entries_used = 0;
	content->entries_alloc = 0;

	return PARAGRAPH_OK;
}

/**
 * Get a new empty content entry at the end of the content entries array.
 *
 * \param[in]  ctx          The paragraph context to get new content entry for.
 * \param[out] entries_out  Returns pointer to new content entry on success.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
static paragraph_err_t paragraph__content_entry_get_new(
		paragraph_ctx_t *ctx,
		paragraph_content_entry_t **entry_out)
{
	uint32_t entries_alloc;
	paragraph_content_t *content;
	paragraph_content_entry_t *entries;

	content = &ctx->content;

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
	return PARAGRAPH_OK;
}

/* Exported function, documented in `include/paragraph.h` */
paragraph_err_t paragraph_content_add_text(
		paragraph_ctx_t *ctx,
		const paragraph_string_t *text,
		void *handle)
{
	paragraph_err_t err;
	paragraph_content_entry_t *entry;

	err = paragraph__content_entry_get_new(ctx, &entry);
	if (err != PARAGRAPH_OK) {
		return err;
	}

	entry->type = PARAGRAPH_CONTENT_TEXT;
	entry->data.text = text;
	entry->handle = handle;
	entry->style = paragraph_style__get_current(&ctx->styles);

	return PARAGRAPH_OK;
}

/* Exported function, documented in `include/paragraph.h` */
paragraph_err_t paragraph_content_add_replaced(
		paragraph_ctx_t *ctx,
		uint32_t px_width,
		uint32_t px_height,
		void *handle,
		paragraph_style_t *style)
{
	paragraph_err_t err;
	paragraph_content_entry_t *entry;

	err = paragraph__content_entry_get_new(ctx, &entry);
	if (err != PARAGRAPH_OK) {
		return err;
	}

	entry->type = PARAGRAPH_CONTENT_REPLACED;
	entry->data.replaced.px_width = px_width;
	entry->data.replaced.px_height = px_height;
	entry->handle = handle;
	entry->style = style;

	return PARAGRAPH_OK;
}

/* Exported function, documented in `include/paragraph.h` */
paragraph_err_t paragraph_content_add_float(
		paragraph_ctx_t *ctx,
		void *handle,
		paragraph_style_t *style)
{
	paragraph_err_t err;
	paragraph_content_entry_t *entry;

	err = paragraph__content_entry_get_new(ctx, &entry);
	if (err != PARAGRAPH_OK) {
		return err;
	}

	entry->type = PARAGRAPH_CONTENT_FLOAT;
	entry->handle = handle;
	entry->style = style;

	return PARAGRAPH_OK;
}
