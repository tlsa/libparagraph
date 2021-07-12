/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Paragraph utility functionality interface.
 */

#ifndef PARAGRAPH__UTIL_H
#define PARAGRAPH__UTIL_H

/**
 *  Get the number of elements in an array.
 *
 * \param[in]  _a  Array to get length of.
 * \return number of elements in array.
 */
#define PARAGRAPH_ARRAY_LEN(_a) ((sizeof(_a))/(sizeof(*_a)))

#endif
