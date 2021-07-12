/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2019 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Paragraph content interface.
 */

#ifndef PARAGRAPH__CONTENT_H
#define PARAGRAPH__CONTENT_H

typedef struct paragraph_content_entry_s {
	enum {
		PARAGRAPH_CONTENT_TEXT,
		PARAGRAPH_CONTENT_FLOAT,
		PARAGRAPH_CONTENT_REPLACED,
	} type;
	union {
		const paragraph_string_t *text;
		struct {
			uint32_t px_width;
			uint32_t px_height;
		} replaced;
	} data;
	void *handle;
	const paragraph_style_t *style;
} paragraph_content_entry_t;

typedef struct paragraph_content_s {
	paragraph_content_entry_t *entries;
	uint32_t entries_alloc;
	uint32_t entries_used;
} paragraph_content_t;

/**
 * Destroy all content.
 *
 * Note, the passed object itself is not freed, only its contents are
 * destroyed.
 *
 * \param[in]  content  The content object to destroy all content inside.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph__content_destroy(
		paragraph_content_t *content);

#endif