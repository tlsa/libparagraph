/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2019 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Paragraph layout interface.
 */

#ifndef PARAGRAPH__LAYOUT_H
#define PARAGRAPH__LAYOUT_H

typedef struct paragraph_layout_s {
} paragraph_layout_t;

/**
 * Destroy all layout.
 *
 * Note, the passed object itself is not freed, only its contents are
 * destroyed.
 *
 * \param[in]  layout  The layout object to destroy all layout inside.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph__layout_destroy(
		paragraph_layout_t *layout);

#endif
