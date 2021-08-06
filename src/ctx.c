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
 * \param[in]  ctx  The paragraph context.
 */
static void paragraph__ctx_destroy_internals(
		paragraph_ctx_t *ctx)
{
	/* Destroy the stuff we own. */
	paragraph__content_destroy(&ctx->content);
	paragraph_style__fini(&ctx->styles);

	/* Invalidate the pointers to the things we don't own. */
	ctx->pw = NULL;
	ctx->cb_text = NULL;
}

/* Exported function, documented in `include/paragraph.h` */
paragraph_err_t paragraph_ctx_create(
		void *pw,
		paragraph_ctx_t **ctx_out,
		const paragraph_cb_text_t *cb_text,
		paragraph_style_t *container_style)
{
	paragraph_ctx_t *ctx;
	paragraph_err_t err;

	if (ctx_out == NULL || cb_text == NULL || container_style == NULL) {
		return PARAGRAPH_ERR_BAD_PARAM;
	}

	ctx = calloc(1, sizeof(*ctx));
	if (ctx == NULL) {
		return PARAGRAPH_ERR_OOM;
	}

	ctx->pw = pw;
	ctx->cb_text = cb_text;

	paragraph_style__init(&ctx->styles);
	err = paragraph_style__push(&ctx->styles, container_style);
	if (err != PARAGRAPH_OK) {
		paragraph_ctx_destroy(ctx);
		return err;
	}

	*ctx_out = ctx;
	return PARAGRAPH_OK;
}

/* Exported function, documented in `include/paragraph.h` */
paragraph_ctx_t *paragraph_ctx_destroy(
		paragraph_ctx_t *ctx)
{
	paragraph__ctx_destroy_internals(ctx);
	free(ctx);

	return NULL;
}
