/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Styled document style handling.
 */

#ifndef SD__STYLE_H
#define SD__STYLE_H

bool sd_style_init(void);

bool sd_style_annotate(
		dom_document *doc,
		css_stylesheet *sheet_ua,
		css_stylesheet *sheet_user);

void sd_style_fini(void);

#endif
