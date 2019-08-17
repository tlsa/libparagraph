/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2019 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Paragraph content interface.
 */

#ifndef PARAGRAPH__CTX_H
#define PARAGRAPH__CTX_H

#include "content.h"

struct paragraph_ctx_s {
	void *pw;
	const paragraph_cb_text_t *cb_text;
	const paragraph_style_t *container_style;

	paragraph_content_t content;
};

#endif
