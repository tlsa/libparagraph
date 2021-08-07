/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Vector data structure common code.
 *
 * The vectors support small size optimisation (no heap allocation at small
 * sizes), and they are copied to the heap when they outgrow their limit.
 * The vectors can be grown and freed, but not shrunken.  Once they've grown
 * to use the heap, they remain on the heap until freed.
 */

#ifndef PARAGRAPH__VEC_H
#define PARAGRAPH__VEC_H

#include <assert.h>
#include <string.h>

typedef struct {
	/**
	 * Maximum number of elements for small size optimisation.
	 * If zero, small size optimisation is disabled.
	 */
	const uint8_t sso_element_max;
} vec_opts_t;

/**
 * Increase vector allocation to hold the given element count and increment.
 *
 * If \ref element_alloc is increased beyond the the small size optimisation
 * limit, existing \ref element_count elements are copied to a new heap
 * allocation.
 *
 * If \ref element_count is less than the small size optimisation limit, then
 * the address pointed to by data must be caller memory, otherwise it must be
 * an allocation returned by this function.
 *
 * \param[in,out] data           Address to write address of any new allocation.
 * \param[in]     increment      Extra elements required beyond element_count.
 * \param[in]     element_size   Size of an element in bytes.
 * \param[in]     element_count  Current number of elements in data.
 * \param[in,out] element_alloc  Number of elements that can be stored in data.
 *                               Updated on success.
 * \param[in]     options        Vector options.
 * \return PARAGRAPH_OK on success, appropriate error otherwise.
 */
static inline paragraph_err_t vec_ensure(
		void **data,
		size_t increment,
		size_t element_size,
		size_t element_count,
		size_t *element_alloc,
		const vec_opts_t options)
{
	void *temp;
	void *orig = *data;
	const size_t count = element_count + increment;
	const size_t alloc = *element_alloc * 2 > count ?
			*element_alloc * 2 : count;

	assert(data != NULL);

	if (count <= *element_alloc) {
		return PARAGRAPH_OK;
	}

	if (*element_alloc <= options.sso_element_max) {
		orig = NULL;
	}

	temp = realloc(orig, alloc * element_size);
	if (temp == NULL) {
		return PARAGRAPH_ERR_OOM;
	}

	if (*element_alloc <= options.sso_element_max) {
		memcpy(temp, *data, element_count * element_size);
	}

	*element_alloc = alloc;
	*data = temp;

	return PARAGRAPH_OK;
}

/**
 * Free a vector.
 *
 * If the size of the allocation is smaller than the small size optimisation,
 * there is no heap allocation, so nothing will be freed.
 *
 * After this call, the data pointer will be set to NULL, so if small size
 * optimisation is in use and the array is to be reused, the client should
 * reset the data pointer to its own valid buffer of appropriate size.
 *
 * \param[in,out] data           The vector to free.
 * \param[in,out] element_alloc  Number of elements that can be stored in data.
 *                               Updated to zero.
 * \param[in]     options        Vector options.
 */
static inline void vec_free(
		void **data,
		size_t *element_alloc,
		const vec_opts_t options)
{
	if (*element_alloc > options.sso_element_max) {
		free(*data);
	}

	*data = NULL;
	*element_alloc = 0;
}

#endif
