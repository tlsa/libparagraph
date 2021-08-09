/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Paragraph context interface.
 */

#ifndef PARAGRAPH__CTX_H
#define PARAGRAPH__CTX_H

struct paragraph_ctx_s {
	void *pw;
	const paragraph_cb_text_t *cb_text;
};

#endif
