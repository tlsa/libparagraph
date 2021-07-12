/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2019-2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Paragraph public API.
 */

#ifndef PARAGRAPH_PARAGRAPH_H
#define PARAGRAPH_PARAGRAPH_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/** 22:10 fixed point math */
#define PARAGRAPH_RADIX_POINT 10

/** Type for fixed point numbers */
typedef int32_t paragraph_fixed_t;

typedef struct paragraph_ctx_s paragraph_ctx_t;

/**
 * LibParagraph return codes.
 */
typedef enum paragraph_err {
	PARAGRAPH_OK,            /* Success. */
	PARAGRAPH_END_OF_LINE,   /* Success: Can't fit more on this line. */
	PARAGRAPH_ERR_OOM,       /* Out of memory. */
	PARAGRAPH_ERR_BAD_PARAM, /* Bad parameter. */
	PARAGRAPH_ERR_UNKNOWN,   /* TODO: Replace with proper errors. */
} paragraph_err_t;



typedef void paragraph_style_t;
typedef void paragraph_string_t;

typedef struct paragraph_text_s {
	paragraph_string_t *text;
	size_t offset;
	size_t len;
} paragraph_text_t;

typedef struct paragraph_position_s {
	uint32_t x;
	uint32_t y;
} paragraph_position_t;

/**
 * These are implemented by the chosen backends.
 */
typedef struct paragraph_callbacks_s {
	paragraph_err_t (*measure_text)(
			void *pw,
			const paragraph_text_t *text,
			const paragraph_style_t *style,
			uint32_t *width_out,
			uint32_t *height_out,
			uint32_t *baseline_out);
	paragraph_err_t (*text_get)(
			void *pw,
			const paragraph_string_t *text,
			const char **data_out,
			char *len_out);
} paragraph_cb_text_t;

/**
 * Create a paragraph context.
 *
 * \param[in]  pw               Client's private data.
 * \param[out] ctx_out          Returns the newly created context on success.
 * \param[in]  cb_text          Client callback table.
 * \param[in]  container_style  The style of the paragraph container.  This is
 *                              needed for properties like `text-justify`.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_ctx_create(
		void *pw,
		paragraph_ctx_t **ctx_out,
		const paragraph_cb_text_t *cb_text,
		const paragraph_style_t *container_style);

/**
 * Destroy a paragraph context.
 *
 * This frees any memory and resources owned by the context, frees the
 * context itself, and returns NULL.
 *
 * Assign the returned NULL to the context pointer being freed, so that wild
 * pointers to freed memory aren't left lying around:
 *
 * ```c
 * ctx = paragraph_ctx_destroy(ctx);
 * ```
 */
paragraph_ctx_t *paragraph_ctx_destroy(
		paragraph_ctx_t *ctx);

/**
 * Add text to a layout context.
 *
 * Note that both `handle` and `style` must remain valid and unmodified until
 * the paragraph context is either reset or destroyed.
 *
 * The scale is not passed in here.  The client must know the scale to measure
 * text with by storing it in its `pw`.
 *
 * \param[in] ctx     The paragraph context to add text to.
 * \param[in] text    The text to add to the context.
 * \param[in] handle  The client handle for the text.  e.g. a layout node.
 * \param[in] style   The computed style that applies to the text.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_content_add_text(
		paragraph_ctx_t *ctx,
		const paragraph_string_t *text,
		void *handle,
		const paragraph_style_t *style);

/**
 * Add replaced object to a layout context.
 *
 * This is an object that has pre-determined dimensions, for example:
 *
 * * an image
 * * a form control
 * * an inline-block
 *
 * Note that both `handle` and `style` must remain valid and unmodified until
 * the paragraph context is either reset or destroyed.
 *
 * Note that the width and height are passed **pre-scaled**.
 *
 * \param[in] ctx     The paragraph context to add replaced object to.
 * \param[in] width   The width of the replaced content.
 * \param[in] height  The height of the replaced content.
 * \param[in] handle  The client handle for replaced element.  e.g. layout node.
 * \param[in] style   The computed style that applies to the text.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_content_add_replaced(
		paragraph_ctx_t *ctx,
		uint32_t px_width,
		uint32_t px_height,
		void *handle,
		const paragraph_style_t *style);

/**
 * Add floated object to a layout context.
 *
 * This is an object that has the `float` property set to either `left` or
 * `right`.
 *
 * Note that both `handle` and `style` must remain valid and unmodified until
 * the paragraph context is either reset or destroyed.
 *
 * \param[in] ctx     The paragraph context to add replaced object to.
 * \param[in] handle  The client handle for the text.  e.g. a layout node.
 * \param[in] style   The computed style that applies to the text.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_content_add_float(
		paragraph_ctx_t *ctx,
		void *handle,
		const paragraph_style_t *style);

/**
 * Get the minimum and maximum widths of the paragraph context.
 *
 * \param[in]  ctx     The paragraph context to get min / max widths from.
 * \param[out] min     Pointer to place to store minimum width, or NULL.
 * \param[out] max     Pointer to place to store maximum width, or NULL.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_get_min_max_width(
		paragraph_ctx_t *ctx,
		uint32_t *min,
		uint32_t *max);

/**
 * Client callback function for laying out text.
 *
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
typedef paragraph_err_t (* const paragraph_layout_text_fn)(
		void *pw,
		void *handle,
		const paragraph_style_t *style,
		const paragraph_text_t *text,
		const paragraph_position_t *pos);

/**
 * Client callback function for laying out replaced objects.
 *
 * /param[in]  pw
 * /param[in]  handle
 * /param[in]  style
 * /param[in]  pos              Position of the content.
 *
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
typedef paragraph_err_t (* const paragraph_layout_replaced_fn)(
		void *pw,
		void *handle,
		const paragraph_style_t *style,
		const paragraph_position_t *pos);

/**
 * Client callback function for laying out replaced objects.
 *
 * /param[in]  pw
 * /param[in]  handle
 * /param[in]  style
 * /param[in]  pos              Position of the content.
 * /param[out] available_width  On success, updated to the new available width.
 *
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
typedef paragraph_err_t (* const paragraph_layout_float_fn)(
		void *pw,
		void *handle,
		const paragraph_style_t *style,
		uint32_t *available_width);

/**
 * Perform layout of a line from the paragraph context.
 *
 * Once all the content has been added to the paragraph context, this is
 * called.  One call will lay out a single line from the context.  If there
 * is more content remaining that could not fit the line, then \ret
 * PARAGRAPH_END_OF_LINE will be returned.  Calling this function again
 * on the same paragraph context will lay out the next line.  Once all
 * the content has undergone layout, \ref PARAGRAPH_OK will be returned.
 *
 * If some content on this line is to be floated, callbacks for that content
 * will emerge first.  Once all the floated content from the line has been
 * handled, callbacks for the in-flow content will emerge.
 *
 * \param[in]  ctx              The paragraph context to lay out.
 * \param[in]  available_width  The containing block width in physical pixels.
 * \param[in]  text_fn          Callback for providing layout info for text.
 * \param[in]  replaced_fn      Callback for providing layout info for replaced.
 * \param[out] line_height_out  On success, return the line height.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_layout_line(
                paragraph_ctx_t *ctx,
                uint32_t available_width,
                paragraph_layout_text_fn text_fn,
                paragraph_layout_replaced_fn replaced_fn,
                uint32_t *line_height_out);

/**
 * Convert a paragraph error code to a string.
 *
 * \param[in]  err  Error code to convert to a string.
 * \return Error string for error code value.
 */
const char *paragraph_strerror(paragraph_err_t err);

#ifdef __cplusplus
}
#endif

#endif
