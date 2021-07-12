/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Styled document stylesheet loader.
 */

#ifndef SD__SHEET_H
#define SD__SHEET_H

bool sd_sheet_load_data(
		const char *css,
		size_t css_len,
		const char *url,
		const char *title,
		css_stylesheet **sheet_out);

bool sd_sheet_load_file(
		const char *css_path,
		const char *title,
		css_stylesheet **sheet_out);

#endif
