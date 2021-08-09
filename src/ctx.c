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

#include "ctx.h"

/* Exported function, documented in `include/paragraph.h` */
paragraph_err_t paragraph_ctx_create(
		void *pw,
		paragraph_ctx_t **ctx_out,
		const paragraph_config_t *config,
		const paragraph_cb_text_t *cb_text)
{
	paragraph_ctx_t *ctx;

	if (ctx_out == NULL || cb_text == NULL) {
		return PARAGRAPH_ERR_BAD_PARAM;
	}

	ctx = calloc(1, sizeof(*ctx));
	if (ctx == NULL) {
		return PARAGRAPH_ERR_OOM;
	}

	ctx->pw = pw;
	ctx->config = config;
	ctx->cb_text = cb_text;

	*ctx_out = ctx;
	return PARAGRAPH_OK;
}

/* Exported function, documented in `include/paragraph.h` */
paragraph_ctx_t *paragraph_ctx_destroy(
		paragraph_ctx_t *ctx)
{
	free(ctx);

	return NULL;
}
