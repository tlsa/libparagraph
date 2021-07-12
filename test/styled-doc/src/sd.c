/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Styled document loader.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <dom/dom.h>

#include <libcss/libcss.h>

#include <sd/sd.h>

#include "doc.h"
#include "sheet.h"

struct {
	bool initialised;

	css_stylesheet *sheet_ua;
} sd_ctx;

bool sd_init(const char *ua_stylesheet, bool path)
{
	bool res;

	if (sd_ctx.initialised == true) {
		fprintf(stderr, "%s: Error: Already initialised\n", __func__);
		return false;
	}

	if (path) {
		res = sd_sheet_load_file(ua_stylesheet,
				"User Agent Stylesheet",
				&sd_ctx.sheet_ua);
	} else {
		res = sd_sheet_load_data(ua_stylesheet,
				strlen(ua_stylesheet),
				"ua.css", "User Agent Stylesheet",
				&sd_ctx.sheet_ua);
	}

	if (res != true) {
		return false;
	}

	sd_ctx.initialised = true;
	return true;
}

bool sd_load_data(
		const char *css,
		const char *html,
		dom_document **doc_out)
{
	css_stylesheet *sheet_user;
	dom_document *doc;
	bool res;

	res = sd_sheet_load_data(css, strlen(css),
			"user.css", "User Stylesheet",
			&sheet_user);
	if (res != true) {
		return res;
	}

	res = sd_doc_load_data(html, strlen(html), &doc);
	if (res != true) {
		css_stylesheet_destroy(sheet_user);
		return res;
	}

	res = sd_style_annotate(doc, sd_ctx.sheet_ua, sheet_user);
	css_stylesheet_destroy(sheet_user);
	if (res != true) {
		dom_node_unref(doc);
		return res;
	}

	*doc_out = doc;
	return true;
}

bool sd_load_file(
		const char *css_path,
		const char *html_path,
		dom_document **doc_out)
{
	css_stylesheet *sheet_user;
	dom_document *doc;
	bool res;

	res = sd_sheet_load_file(css_path,
			"User Stylesheet",
			&sheet_user);
	if (res != true) {
		return res;
	}

	res = sd_doc_load_file(html_path, &doc);
	if (res != true) {
		css_stylesheet_destroy(sheet_user);
		return res;
	}

	css_stylesheet_destroy(sheet_user);

	*doc_out = doc;
	return true;
}

void sd_free(dom_document *doc)
{
	dom_node_unref(doc);
}

void sd__fini_lwc_callback(lwc_string *str, void *pw)
{
	(void)(pw);

	fprintf(stderr, "Leaked string: %.*s\n",
			(int)lwc_string_length(str),
			lwc_string_data(str));
}

void sd_fini(void)
{
	if (sd_ctx.initialised == false) {
		fprintf(stderr, "%s: Error: Not initialised\n", __func__);
		return;
	}

	if (sd_ctx.sheet_ua != NULL) {
		css_stylesheet_destroy(sd_ctx.sheet_ua);
		sd_ctx.sheet_ua = NULL;
	}

	dom_namespace_finalise();
	lwc_iterate_strings(sd__fini_lwc_callback, NULL);

	sd_ctx.initialised = false;
}
