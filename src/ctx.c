/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2019 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Paragraph context handling.
 */

#include <stdlib.h>

#include <paragraph.h>

#include "content.h"
#include "style.h"
#include "ctx.h"

/**
 * Destroy the contents of a paragraph context.
 *
 * \param[in]  para  The paragraph context.
 */
static void paragraph__ctx_destroy_internals(
		paragraph_para_t *para)
{
	/* Destroy the stuff we own. */
	paragraph__content_destroy(&para->content);
	paragraph_style__fini(&para->styles);

	/* Invalidate the pointers to the things we don't own. */
	para->pw = NULL;
	para->cb_text = NULL;
}

/* Exported function, documented in `include/paragraph.h` */
paragraph_err_t paragraph_para_create(
		void *pw,
		paragraph_para_t **para_out,
		const paragraph_cb_text_t *cb_text,
		paragraph_style_t *container_style)
{
	paragraph_para_t *para;
	paragraph_err_t err;

	if (para_out == NULL || cb_text == NULL || container_style == NULL) {
		return PARAGRAPH_ERR_BAD_PARAM;
	}

	para = calloc(1, sizeof(*para));
	if (para == NULL) {
		return PARAGRAPH_ERR_OOM;
	}

	para->pw = pw;
	para->cb_text = cb_text;

	paragraph_style__init(&para->styles);
	err = paragraph_style__push(&para->styles, container_style);
	if (err != PARAGRAPH_OK) {
		paragraph_para_destroy(para);
		return err;
	}

	*para_out = para;
	return PARAGRAPH_OK;
}

/* Exported function, documented in `include/paragraph.h` */
paragraph_para_t *paragraph_para_destroy(
		paragraph_para_t *para)
{
	paragraph__ctx_destroy_internals(para);
	free(para);

	return NULL;
}
