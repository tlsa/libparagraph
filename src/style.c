/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Paragraph style implementation.
 */

#include <stdlib.h>

#include <paragraph.h>

#include "vec.h"
#include "style.h"

static const vec_opts_t options = {
	.sso_element_max = PARAGRAPH_STYLES_SSO,
};

paragraph_err_t paragraph_style__ensure(
		paragraph_styles_t *styles)
{
	return vec_ensure(
			(void **)&styles->array, 1,
			sizeof(*styles->array),
			styles->count,
			&styles->alloc,
			options);
}

paragraph_err_t paragraph_style__push(
		paragraph_styles_t *styles,
		paragraph_style_t *style)
{
	paragraph_err_t err;

	err = paragraph_style__ensure(styles);
	if (err != PARAGRAPH_OK) {
		return err;
	}

	styles->array[styles->count++] = paragraph_style__ref(style);
	return PARAGRAPH_OK;
}

/* Exported function, documented in styles.h. */
paragraph_err_t paragraph_style__pop(
		paragraph_styles_t *styles,
		paragraph_style_t **style_out)
{
	assert(styles->count > 0);

	*style_out = styles->array[--styles->count];
	paragraph_style__unref(*style_out);
	return PARAGRAPH_OK;
}

/* Exported function, documented in styles.h. */
void paragraph_style__fini(
		paragraph_styles_t *styles)
{
	while (styles->count > 0) {
		paragraph_style_t *style;
		paragraph_style__pop(styles, &style);
	}

	vec_free((void **)&styles->array, &styles->alloc, options);
}