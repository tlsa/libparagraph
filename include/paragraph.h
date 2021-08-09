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

#include <stdarg.h>
#include <stdint.h>

/** 22:10 fixed point math */
#define PARAGRAPH_RADIX_POINT 10

/** Type for fixed point numbers */
typedef int32_t paragraph_fixed_t;

typedef struct paragraph_ctx_s paragraph_ctx_t;

typedef struct paragraph_para_s paragraph_para_t;

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

/** Paragraph logging levels. */
typedef enum paragraph_log_e {
	PARAGRAPH_LOG_DEBUG,   /**< Debug level logging. */
	PARAGRAPH_LOG_INFO,    /**< Info level logging. */
	PARAGRAPH_LOG_NOTICE,  /**< Notice level logging. */
	PARAGRAPH_LOG_WARNING, /**< Warning level logging. */
	PARAGRAPH_LOG_ERROR,   /**< Error level logging. */
} paragraph_log_t;

/**
 * Paragraph logging function prototype.
 *
 * Clients may implement this to manage logging from Paragraph themselves.
 * Otherwise, consider using the standard logging function, \ref paragraph_log.
 *
 * \param[in] level  Log level of message to log.
 * \param[in] ctx    Client's private logging context.
 * \param[in] fmt    Format string for message to log.
 * \param[in] args   Additional arguments used by fmt.
 */
typedef void (*paragraph_log_fn_t)(
		paragraph_log_t level,
		void *ctx,
		const char *fmt,
		va_list args);

typedef struct paragraph_config {
	/**
	 * Client function to use for logging.
	 *
	 * Clients can implement their own logging function and set it here.
	 * Otherwise, set `log_fn` to \ref paragraph_log if default
	 * logging to `stderr` is suitable (see its documentation for more
	 * details), or set to `NULL` to suppress all logging.
	 */
	paragraph_log_fn_t log_fn;
	/**
	 * Client logging function context pointer.
	 *
	 * Clients using their own custom logging function can pass their
	 * context here, which will be passed through to their log_fn.
	 *
	 * The default logging function, \ref paragraph_log doesn't require a
	 * logging context, so pass NULL for the log_ctx if using that.
	 */
	void *log_ctx;
	/**
	 * Minimum logging priority level to be issued.
	 *
	 * Specifying e.g. \ref PARAGRAPH_LOG_WARNING will cause only warnings
	 * and errors to emerge.
	 */
	paragraph_log_t log_level;
} paragraph_config_t;

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
 * Initialise the library and create a context.
 *
 * The returned context pointer is used when creating paragraphs.
 * It is destroyed with \ref paragraph_ctx_destroy and it must not be
 * destroyed before all the paragraphs created with it have been destroyed.
 *
 * \param[in]  pw       Client's private data.
 * \param[out] ctx_out  Returns the newly created library context on success.
 * \param[in]  cb_text  Client callback table.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_ctx_create(
		void *pw,
		paragraph_ctx_t **ctx_out,
		const paragraph_config_t *config,
		const paragraph_cb_text_t *cb_text);

/**
 * Destroy a library context.
 *
 * This frees any memory and resources owned by the library context, frees the
 * context itself, and returns NULL.
 *
 * Assign the returned NULL to the paragraph pointer being freed, so that wild
 * pointers to freed memory aren't left lying around:
 *
 * ```c
 * ctx = paragraph_ctx_destroy(ctx);
 * ```
 */
paragraph_ctx_t *paragraph_ctx_destroy(
		paragraph_ctx_t *ctx);

/**
 * Create a paragraph.
 *
 * \param[in]  pw               Client's private data.
 * \param[in]  ctx              Library context.
 * \param[out] para_out         Returns the newly created paragraph on success.
 * \param[in]  container_style  The style of the paragraph container.  This is
 *                              needed for properties like `text-justify`.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_create(
		void *pw,
		paragraph_ctx_t *ctx,
		paragraph_para_t **para_out,
		paragraph_style_t *container_style);

/**
 * Destroy a paragraph.
 *
 * This frees any memory and resources owned by the paragraph, frees the
 * paragraph itself, and returns NULL.
 *
 * Assign the returned NULL to the paragraph pointer being freed, so that wild
 * pointers to freed memory aren't left lying around:
 *
 * ```c
 * para = paragraph_destroy(para);
 * ```
 */
paragraph_para_t *paragraph_destroy(
		paragraph_para_t *para);

/**
 * Push an inline start to a paragraph.
 *
 * Note that both `handle` and `style` must remain valid and unmodified until
 * the paragraph is either reset or destroyed.
 *
 * \param[in] ctx     The paragraph to add style to.
 * \param[in] handle  The client handle for the start.  e.g. a DOM node.
 * \param[in] style   The new computed style.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_content_add_inline_start(
		paragraph_para_t *para,
		void *handle,
		paragraph_style_t *style);

/**
 * Pop an inline style from a paragraph.
 *
 * Note that both `handle` and `style` must remain valid and unmodified until
 * the paragraph is either reset or destroyed.
 *
 * \param[in] ctx     The paragraph to add style to.
 * \param[in] handle  The client handle for the start.  e.g. a DOM node.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_content_add_inline_end(
		paragraph_para_t *para,
		void *handle);

/**
 * Add text to a paragraph.
 *
 * Note that `handle` must remain valid and unmodified until
 * the paragraph is either reset or destroyed.
 *
 * The scale is not passed in here.  The client must know the scale to measure
 * text with by storing it in its `pw`.
 *
 * \param[in] ctx     The paragraph to add text to.
 * \param[in] text    The text to add to the paragraph.
 * \param[in] handle  The client handle for the text.  e.g. a layout node.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_content_add_text(
		paragraph_para_t *para,
		const paragraph_string_t *text,
		void *handle);

/**
 * Add replaced object to a paragraph.
 *
 * This is an object that has pre-determined dimensions, for example:
 *
 * * an image
 * * a form control
 * * an inline-block
 *
 * Note that both `handle` and `style` must remain valid and unmodified until
 * the paragraph is either reset or destroyed.
 *
 * Note that the width and height are passed **pre-scaled**.
 *
 * \param[in] ctx     The paragraph to add replaced object to.
 * \param[in] width   The width of the replaced content.
 * \param[in] height  The height of the replaced content.
 * \param[in] handle  The client handle for replaced element.  e.g. layout node.
 * \param[in] style   The computed style that applies to the text.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_content_add_replaced(
		paragraph_para_t *para,
		uint32_t px_width,
		uint32_t px_height,
		void *handle,
		paragraph_style_t *style);

/**
 * Add floated object to a paragraph.
 *
 * This is an object that has the `float` property set to either `left` or
 * `right`.
 *
 * Note that both `handle` and `style` must remain valid and unmodified until
 * the paragraph is either reset or destroyed.
 *
 * \param[in] ctx     The paragraph to add replaced object to.
 * \param[in] handle  The client handle for the text.  e.g. a layout node.
 * \param[in] style   The computed style that applies to the text.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_content_add_float(
		paragraph_para_t *para,
		void *handle,
		paragraph_style_t *style);

/**
 * Get the minimum and maximum widths of the paragraph.
 *
 * \param[in]  ctx     The paragraph to get min / max widths from.
 * \param[out] min     Pointer to place to store minimum width, or NULL.
 * \param[out] max     Pointer to place to store maximum width, or NULL.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_get_min_max_width(
		paragraph_para_t *para,
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
 * Perform layout of a line from the paragraph.
 *
 * Once all the content has been added to the paragraph, this is
 * called.  One call will lay out a single line from the paragraph.  If there
 * is more content remaining that could not fit the line, then \ret
 * PARAGRAPH_END_OF_LINE will be returned.  Calling this function again
 * on the same paragraph will lay out the next line.  Once all
 * the content has undergone layout, \ref PARAGRAPH_OK will be returned.
 *
 * If some content on this line is to be floated, callbacks for that content
 * will emerge first.  Once all the floated content from the line has been
 * handled, callbacks for the in-flow content will emerge.
 *
 * \param[in]  para             The paragraph to lay out.
 * \param[in]  available_width  The containing block width in physical pixels.
 * \param[in]  text_fn          Callback for providing layout info for text.
 * \param[in]  replaced_fn      Callback for providing layout info for replaced.
 * \param[out] line_height_out  On success, return the line height.
 * \return \ref PARAGRAPH_OK on success, or appropriate error otherwise.
 */
paragraph_err_t paragraph_layout_line(
		paragraph_para_t *para,
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
