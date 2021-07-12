/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Styled document document loader.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <dom/dom.h>
#include <dom/bindings/hubbub/parser.h>

#include "doc.h"
#include "file.h"


bool sd_doc_load_data(
		const char *doc_data,
		size_t doc_len,
		dom_document **doc_out)
{
	dom_hubbub_parser_params params = {
		.fix_enc = true,
	};
	dom_hubbub_parser *parser = NULL;
	dom_hubbub_error error;
	dom_document *doc;

	error = dom_hubbub_parser_create(&params, &parser, &doc);
	if (error != DOM_HUBBUB_OK) {
		fprintf(stderr, "%s: Error: Can't create Hubbub Parser\n",
				__func__);
		return false;
	}

	error = dom_hubbub_parser_parse_chunk(parser,
			(const uint8_t*)doc_data, doc_len);
	if (error != DOM_HUBBUB_OK) {
		dom_hubbub_parser_destroy(parser);
		fprintf(stderr, "%s: Error: Parsing errors occurred\n",
				__func__);
		return false;
	}

	error = dom_hubbub_parser_completed(parser);
	if (error != DOM_HUBBUB_OK) {
		dom_hubbub_parser_destroy(parser);
		fprintf(stderr, "%s: Error: Parsing completion error\n",
				__func__);
		return false;
	}

	dom_hubbub_parser_destroy(parser);

	*doc_out = doc;
	return true;
}

bool sd_doc_load_file(
		const char *doc_path,
		dom_document **doc_out)
{
	bool res;
	size_t len;
	uint8_t *data;

	res = sd_file_load(doc_path, &data, &len);
	if (res != true) {
		return res;
	}

	res = sd_doc_load_data((const char *)(void *)data, len, doc_out);
	free(data);

	return res;
}
