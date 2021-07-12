/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Styled document loader file handler.
 */

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "file.h"

bool sd_file_load(const char *path, uint8_t **data_out, size_t *len_out)
{
	uint8_t *data;
	size_t read;
	FILE *file;
	long len;

	file = fopen(path, "rb");
	if (file == NULL) {
		printf("File could not be opened: %s\n",
				strerror(errno));
		return false;
	}
	fseek(file, 0, SEEK_END);
	len = ftell(file);
	if (len < 0) {
		printf("Could not find end of file: %s\n",
				strerror(errno));
		fclose(file);
		return false;
	}
	fseek(file, 0, SEEK_SET);

	data = malloc(len);
	if (data == NULL) {
		printf("Could not allocate space for file\n");
		fclose(file);
		return false;
	}

	read = fread(data, 1, len, file);
	fclose(file);

	if (((long)read) != len) {
		printf("Read unexpected data length from file\n");
		return false;
	}

	*len_out = len;
	*data_out = data;
	return true;
}
