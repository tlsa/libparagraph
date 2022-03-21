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

#include "util.h"

/**
 * Paragraph content spec.
 */
typedef struct paragraph_content_entry_s {
	/** Content type. */
	enum paragraph_content_type_e type;
	/** Content type-specific data. */
	union {
		/** Data for type \ref PARAGRAPH_CONTENT_TEXT. */
		struct {
			const paragraph_string_t *string;
			const char *data;
			size_t len;
		} text;
		/** Data for type \ref PARAGRAPH_CONTENT_REPLACED. */
		struct {
			uint32_t px_width;
			uint32_t px_height;
		} replaced;
	};
	/** Client handle for content, e.g. corresponding DOM node. */
	void *pw;
	/**
	 * Style for content.
	 */
	paragraph_style_t *style;
} paragraph_content_entry_t;

typedef struct paragraph_content_s {
	paragraph_content_entry_t *entries;
	uint32_t entries_alloc;
	uint32_t entries_used;

	char *text; /**< Complete paragraph text. */
	size_t len; /**< Total byte-length of text. */
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

paragraph_err_t paragraph_content__get_text(
		paragraph_para_t *para,
		const char **text_out,
		size_t *len_out);

static inline const char *paragraph__content_typestr(
		enum paragraph_content_type_e type)
{
	static const char * const str[] = {
		[PARAGRAPH_CONTENT_TEXT]         = "TEXT",
		[PARAGRAPH_CONTENT_FLOAT]        = "FLOAT",
		[PARAGRAPH_CONTENT_REPLACED]     = "REPLACED",
		[PARAGRAPH_CONTENT_INLINE_START] = "INLINE START",
		[PARAGRAPH_CONTENT_INLINE_END]   = "INLINE END",
	};

	if (type >= PARAGRAPH_ARRAY_LEN(str)) {
		return "Invalid";
	}

	if (str[type] == NULL) {
		return "Invalid";
	}

	return str[type];
}

#endif
