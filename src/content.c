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
	/* TODO: Free anything we own in each entry. */
	for (size_t i = 0; i < content->entries_used; i++) {
		paragraph__content_entry_cleanup(&content->entries[i]);
	}

	free(content->entries);
	content->entries = NULL;
	content->entries_used = 0;
	content->entries_alloc = 0;

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
	uint32_t entry_index;
	uint32_t entries_alloc;
	paragraph_content_t *content;

	content = &para->content;

	if (content->entries_alloc <= content->entries_used) {
		paragraph_content_entry_t *entries;

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

	if (pos != NULL && pos->rel != NULL) {
		entry_index = (paragraph_content_entry_t *) pos->rel -
				content->entries;

		if (pos->pos == PARAGRAPH_CONTENT_POS_AFTER) {
			entry_index++;
		}

		if (entry_index > content->entries_used) {
			entry_index = content->entries_used;
		} else {
			if (entry_index != content->entries_used) {
				paragraph_content_entry_t *entries;
				const size_t used = content->entries_used;
				const size_t entry_size = sizeof(*entries);
				const size_t remaining = used - entry_index;
				const size_t before = entry_size * entry_index;

				memmove(content->entries + before + entry_size,
					content->entries + before,
					remaining * entry_size);
			}
		}
	} else {
		entry_index = content->entries_used;
	}

	content->entries_used++;

	*entry_out = content->entries + entry_index;
	(*entry_out)->type = PARAGRAPH_CONTENT_NONE;

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
	for (size_t i = 0; i < content->entries_used; i++) {
		if (content->entries[i].type != PARAGRAPH_CONTENT_TEXT) {
			continue;
		}
		memcpy(text, content->entries[i].text.data,
				content->entries[i].text.len);
		text += content->entries[i].text.len;
	}

	*text_out = content->text;
	*len_out = content->len;
	return PARAGRAPH_OK;
}
