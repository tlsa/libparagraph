/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Styled document loader style routines.
 */

#ifndef SD_STYLE_H
#define SD_STYLE_H

bool sd_style_get(
		dom_node *node,
		dom_node_type type,
		css_select_results **style);

#endif
