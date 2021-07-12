/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Generic utility functionality.
 */

#include <stddef.h>

#include <paragraph.h>

#include "util.h"

/* Exported function, documented in `include/paragraph.h` */
const char *paragraph_strerror(paragraph_err_t err)
{
	static const char * const str[] = {
		[PARAGRAPH_OK]            = "Success",
		[PARAGRAPH_END_OF_LINE]   = "Success: End of line",
		[PARAGRAPH_ERR_OOM]       = "Out of memory",
		[PARAGRAPH_ERR_BAD_PARAM] = "Bad parameter",
	};

	if (err >= PARAGRAPH_ARRAY_LEN(str) || str[err] == NULL) {
		return "Unknown error";
	}

	return str[err];
}