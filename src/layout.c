/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2019 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Paragraph layout handling.
 */

#include <stdlib.h>

#include <paragraph.h>

#include "content.h"
#include "layout.h"

/* Exported function, documented in `include/paragraph.h` */
paragraph_err_t paragraph_layout_line(
		paragraph_para_t *para,
		uint32_t available_width,
		paragraph_layout_text_fn text_fn,
		paragraph_layout_replaced_fn replaced_fn,
		uint32_t *line_height_out)
{
	paragraph_err_t err;
	const char *text;
	size_t len;

	err = paragraph_content__get_text(para, &text, &len);
	if (err != PARAGRAPH_OK) {
		return err;
	}

	/* Get runs from content (same style) */
	/* Split runs for  */

	return PARAGRAPH_OK;
}
