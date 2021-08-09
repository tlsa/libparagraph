/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Paragraph logging interface.
 */

#ifndef PARAGRAPH__LOG_H
#define PARAGRAPH__LOG_H

enum log_level {
	LOG_DEBUG      = PARAGRAPH_LOG_DEBUG,   /**< Debug level logging. */
	LOG_INFO       = PARAGRAPH_LOG_INFO,    /**< Info level logging. */
	LOG_NOTICE     = PARAGRAPH_LOG_NOTICE,  /**< Notice level logging. */
	LOG_WARNING    = PARAGRAPH_LOG_WARNING, /**< Warning level logging. */
	LOG_ERROR      = PARAGRAPH_LOG_ERROR,   /**< Error level logging. */
};

#ifdef NDEBUG
#define LOG_LEVEL_MIN_COMPILED LOG_INFO
#else
#define LOG_LEVEL_MIN_COMPILED LOG_DEBUG
#endif

/**
 * Log to client's logging function, if provided.
 *
 * \param[in] cfg    Paragraph client config structure.
 * \param[in] level  Log level of message to log.
 * \param[in] fmt    Format string for message to log.
 * \param[in] ...    Additional arguments used by fmt.
 */
static inline void paragraph__log(
		const paragraph_config_t *cfg,
		enum log_level log_level,
		const char *fmt, ...)
{
	paragraph_log_t level = (paragraph_log_t)log_level;

	if (log_level < LOG_LEVEL_MIN_COMPILED) {
		return;
	}

	if (level >= cfg->log_level && cfg->log_fn != NULL) {
		va_list args;
		va_start(args, fmt);
		cfg->log_fn(level, cfg->log_ctx, fmt, args);
		va_end(args);
	}
}

#endif
