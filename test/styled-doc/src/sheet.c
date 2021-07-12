/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Styled document style handling.
 */

#include <stdio.h>

#include <libcss/libcss.h>

#include "file.h"
#include "sheet.h"

static css_error sd_sheet__resolve_url(
		void *pw,
		const char *base,
		lwc_string *rel,
		lwc_string **abs)
{
	(void)(pw);
	(void)(base);

	/* TODO: Something useful. */
	*abs = lwc_string_ref(rel);

	return CSS_OK;
}

bool sd_sheet_load_data(
		const char *css,
		size_t css_len,
		const char *url,
		const char *title,
		css_stylesheet **sheet_out)
{
	css_stylesheet *sheet;
	const css_stylesheet_params params = {
		.params_version = CSS_STYLESHEET_PARAMS_VERSION_1,
		.resolve = sd_sheet__resolve_url,
		.level = CSS_LEVEL_21,
		.charset = "UTF-8",
		.title = title,
		.url = url,
	};
	css_error cerr;


	cerr = css_stylesheet_create(&params, &sheet);
	if (cerr != CSS_OK) {
		fprintf(stderr, "%s: Error: Failed to create stylesheet\n",
				__func__);
		return false;
	}

	cerr = css_stylesheet_append_data(sheet,
			(const uint8_t *)css, css_len);
	if (cerr != CSS_OK && cerr != CSS_NEEDDATA) {
		fprintf(stderr, "%s: Error: Failed to add data to stylesheet\n",
				__func__);
		goto error;
	}

	cerr = css_stylesheet_data_done(sheet);
	if (cerr != CSS_OK) {
		fprintf(stderr, "%s: Error: Failed to finish stylesheet\n",
				__func__);
		goto error;
	}

	*sheet_out = sheet;
	return true;

error:
	css_stylesheet_destroy(sheet);
	return false;
}

bool sd_sheet_load_file(
		const char *css_path,
		const char *title,
		css_stylesheet **sheet_out)
{
	bool res;
	size_t len;
	uint8_t *data;

	res = sd_file_load(css_path, &data, &len);
	if (res != true) {
		return res;
	}

	res = sd_sheet_load_data((const char *)(void *)data, len,
			css_path, title, sheet_out);
	free(data);

	return res;
}
