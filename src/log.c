/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Paragraph logging implementation.
 */

#include <stdio.h>

#include <paragraph.h>

/* Exported function, documented in include/paragraph.h */
void paragraph_log(
		paragraph_log_t level,
		void *ctx,
		const char *fmt,
		va_list args)
{
	static const char * const strings[] = {
		[PARAGRAPH_LOG_DEBUG]   = "DEBUG",
		[PARAGRAPH_LOG_INFO]    = "INFO",
		[PARAGRAPH_LOG_NOTICE]  = "NOTICE",
		[PARAGRAPH_LOG_WARNING] = "WARNING",
		[PARAGRAPH_LOG_ERROR]   = "ERROR",
	};

	(void)(ctx);

	fprintf(stderr, "paragraph: %7.7s: ", strings[level]);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
}
