/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Paragraph style interface.
 */

#ifndef PARAGRAPH__STYLE_H
#define PARAGRAPH__STYLE_H

#include <assert.h>

#include "util.h"

#define PARAGRAPH_STYLES_SSO 2

typedef struct paragraph_styles {
	size_t count;
	size_t alloc;
	paragraph_style_t **array;
	paragraph_style_t *sso[PARAGRAPH_STYLES_SSO];
} paragraph_styles_t;

static inline void paragraph_style__init(
		paragraph_styles_t *styles)
{
	styles->count = 0;
	styles->array = styles->sso;
	styles->alloc = PARAGRAPH_ARRAY_LEN(styles->sso);
}

static inline paragraph_style_t *paragraph_style__ref(
		paragraph_style_t *style)
{
	/* TODO: Call client ref function. */
	return style;
}

static inline paragraph_style_t *paragraph_style__unref(
		paragraph_style_t *style)
{
	(void)(style);
	/* TODO: Call client unref function. */
	return NULL;
}

static inline paragraph_style_t *paragraph_style__get_current(
		const paragraph_styles_t *styles)
{
	assert(styles != NULL);
	assert(styles->count > 0);
	assert(styles->array[styles->count - 1] != NULL);

	return styles->array[styles->count - 1];
}

paragraph_err_t paragraph_style__push(
		paragraph_styles_t *styles,
		paragraph_style_t *style);

paragraph_err_t paragraph_style__pop(
		paragraph_styles_t *styles,
		paragraph_style_t **style_out);

void paragraph_style__fini(
		paragraph_styles_t *styles);

#endif
