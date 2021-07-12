/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Styled document file loader.
 */

#ifndef SD__FILE_H
#define SD__FILE_H

bool sd_file_load(const char *path, uint8_t **data_out, size_t *len_out);

#endif
