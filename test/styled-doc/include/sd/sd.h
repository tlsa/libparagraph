/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Styled document loader.
 */

#ifndef SD_H
#define SD_H

bool sd_init(const char *ua_stylesheet, bool path);

bool sd_load_data(
		const char *css,
		const char *html,
		dom_document **doc_out);

bool sd_load_file(
		const char *css_path,
		const char *html_path,
		dom_document **doc_out);

void sd_free(dom_document *doc);

void sd_fini(void);

#endif
