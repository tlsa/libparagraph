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

	/* Invalidate the pointers to the things we don't own. */
	ctx->pw = NULL;
	ctx->cb_text = NULL;
	ctx->container_style = NULL;
}

/* Exported function, documented in `include/paragraph.h` */
paragraph_err_t paragraph_ctx_create(
		void *pw,
		paragraph_ctx_t **ctx_out,
		const paragraph_cb_text_t *cb_text,
		paragraph_style_t *container_style)
{
	paragraph_ctx_t *ctx;

	if (ctx_out == NULL || cb_text == NULL || container_style == NULL) {
		return PARAGRAPH_ERR_BAD_PARAM;
	}

	ctx = calloc(1, sizeof(*ctx));
	if (ctx == NULL) {
		return PARAGRAPH_ERR_OOM;
	}

	ctx->pw = pw;
	ctx->cb_text = cb_text;

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
