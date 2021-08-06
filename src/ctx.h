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
#include "style.h"

struct paragraph_ctx_s {
	void *pw;
	const paragraph_cb_text_t *cb_text;

	paragraph_styles_t styles;
	paragraph_content_t content;
};

#endif
