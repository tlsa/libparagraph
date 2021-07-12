/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Styled document document loader.
 */

#ifndef SD__DOC_H
#define SD__DOC_H

bool sd_doc_load_data(
		const char *doc_data,
		size_t doc_len,
		dom_document **doc_out);

bool sd_doc_load_file(
		const char *doc_path,
		dom_document **doc_out);

#endif
