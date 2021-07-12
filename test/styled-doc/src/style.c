/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2021 Michael Drake <tlsa@netsurf-browser.org>
 */

/**
 * \file
 * \brief Styled document style handling.
 */

#include <stdio.h>

#include <dom/dom.h>
#include <dom/walk.h>

#include <libcss/libcss.h>

#include <sd/style.h>

#include "style.h"

struct {
	bool initialised;

	css_unit_ctx unit_len_ctx;

	dom_string *dom_str_href;
	dom_string *dom_str__sd_key_style_node_data;
	dom_string *dom_str__sd_key_libcss_node_data;
} sd_style_g;

static void sd_style__cleanup(void)
{
	if (sd_style_g.dom_str_href != NULL) {
		dom_string_unref(sd_style_g.dom_str_href);
		sd_style_g.dom_str_href = NULL;
	}

	if (sd_style_g.dom_str__sd_key_style_node_data != NULL) {
		dom_string_unref(sd_style_g.dom_str__sd_key_style_node_data);
		sd_style_g.dom_str__sd_key_style_node_data = NULL;
	}

	if (sd_style_g.dom_str__sd_key_libcss_node_data != NULL) {
		dom_string_unref(sd_style_g.dom_str__sd_key_libcss_node_data);
		sd_style_g.dom_str__sd_key_libcss_node_data = NULL;
	}
}

bool sd_style_init(void)
{
	dom_exception derr;

	if (sd_style_g.initialised) {
		return true;
	}

	derr = dom_string_create_interned(
			(const uint8_t *)"_sd_key_style_node_data", 23,
			&sd_style_g.dom_str__sd_key_style_node_data);
	if ((derr != DOM_NO_ERR) ||
	    (sd_style_g.dom_str__sd_key_style_node_data == NULL)) {
		goto error;
	}

	derr = dom_string_create_interned(
			(const uint8_t *)"_sd_key_libcss_node_data", 24,
			&sd_style_g.dom_str__sd_key_libcss_node_data);
	if ((derr != DOM_NO_ERR) ||
	    (sd_style_g.dom_str__sd_key_libcss_node_data == NULL)) {
		goto error;
	}

	derr = dom_string_create_interned(
			(const uint8_t *)"href", 4,
			&sd_style_g.dom_str_href);
	if ((derr != DOM_NO_ERR) ||
	    (sd_style_g.dom_str_href == NULL)) {
		goto error;
	}

	sd_style_g.unit_len_ctx.device_dpi        = FIXTOINT(96);
	sd_style_g.unit_len_ctx.viewport_width    = FIXTOINT(800);
	sd_style_g.unit_len_ctx.viewport_height   = FIXTOINT(600);
	sd_style_g.unit_len_ctx.font_size_default = FIXTOINT(16);
	sd_style_g.unit_len_ctx.font_size_minimum = FIXTOINT(6);

	sd_style_g.initialised = true;
	return true;

error:
	sd_style__cleanup();
	return sd_style_g.initialised;
}

void sd_style_fini(void)
{
	if (!sd_style_g.initialised) {
		return;
	}

	sd_style__cleanup();
	sd_style_g.initialised = false;
}

static int strncmpi(const char *a, const char *b, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		if (a[i] != b[i]) {
			return a[i] - b[i];
		}
		if (a[i] == '\n') {
			break;
		}
	}

	return 0;
}

/**
 * Callback to retrieve a node's name.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Pointer to location to receive node name
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 */
static css_error node_name(void *pw, void *node, css_qname *qname)
{
	dom_node *n = node;
	dom_string *name;
	dom_exception err;

	(void)(pw);

	err = dom_node_get_node_name(n, &name);
	if (err != DOM_NO_ERR)
		return CSS_NOMEM;

	qname->ns = NULL;

	err = dom_string_intern(name, &qname->name);
	if (err != DOM_NO_ERR) {
		dom_string_unref(name);
		return CSS_NOMEM;
	}

	dom_string_unref(name);

	return CSS_OK;
}

/**
 * Callback to retrieve a node's classes.
 *
 * \param pw         HTML document
 * \param node       DOM node
 * \param classes    Pointer to location to receive class name array
 * \param n_classes  Pointer to location to receive length of class name array
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \note The returned array will be destroyed by libcss. Therefore, it must
 *       be allocated using the same allocator as used by libcss during style
 *       selection.
 */
static css_error node_classes(void *pw, void *node,
		lwc_string ***classes, uint32_t *n_classes)
{
	dom_node *n = node;
	dom_exception err;

	(void)(pw);

	*classes = NULL;
	*n_classes = 0;

	err = dom_element_get_classes(n, classes, n_classes);
	if (err != DOM_NO_ERR)
		return CSS_NOMEM;

	return CSS_OK;
}

/**
 * Callback to retrieve a node's ID.
 *
 * \param pw    HTML document
 * \param node  DOM node
 * \param id    Pointer to location to receive id value
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 */
static css_error node_id(void *pw, void *node, lwc_string **id)
{
	dom_node *n = node;
	dom_string *attr;
	dom_exception err;

	(void)(pw);

	*id = NULL;

	/** \todo Assumes an HTML DOM */
	err = dom_html_element_get_id(n, &attr);
	if (err != DOM_NO_ERR)
		return CSS_NOMEM;

	if (attr != NULL) {
		err = dom_string_intern(attr, id);
		if (err != DOM_NO_ERR) {
			dom_string_unref(attr);
			return CSS_NOMEM;
		}
		dom_string_unref(attr);
	}

	return CSS_OK;
}

/**
 * Callback to find a named ancestor node.
 *
 * \param pw        HTML document
 * \param node      DOM node
 * \param qname     Node name to search for
 * \param ancestor  Pointer to location to receive ancestor
 * \return CSS_OK.
 *
 * \post \a ancestor will contain the result, or NULL if there is no match
 */
static css_error named_ancestor_node(void *pw, void *node,
		const css_qname *qname, void **ancestor)
{
	(void)(pw);

	dom_element_named_ancestor_node(node, qname->name,
			(struct dom_element **)ancestor);
	dom_node_unref(*ancestor);

	return CSS_OK;
}

/**
 * Callback to find a named parent node
 *
 * \param pw      HTML document
 * \param node    DOM node
 * \param qname   Node name to search for
 * \param parent  Pointer to location to receive parent
 * \return CSS_OK.
 *
 * \post \a parent will contain the result, or NULL if there is no match
 */
static css_error named_parent_node(void *pw, void *node,
		const css_qname *qname, void **parent)
{
	(void)(pw);

	dom_element_named_parent_node(node, qname->name,
			(struct dom_element **)parent);
	dom_node_unref(*parent);

	return CSS_OK;
}

/**
 * Callback to find a named sibling node.
 *
 * \param pw       HTML document
 * \param node     DOM node
 * \param qname    Node name to search for
 * \param sibling  Pointer to location to receive sibling
 * \return CSS_OK.
 *
 * \post \a sibling will contain the result, or NULL if there is no match
 */
static css_error named_sibling_node(void *pw, void *node,
		const css_qname *qname, void **sibling)
{
	dom_node *n = node;
	dom_node *prev;
	dom_exception err;

	(void)(pw);

	*sibling = NULL;

	/* Find sibling element */
	err = dom_node_get_previous_sibling(n, &n);
	if (err != DOM_NO_ERR)
		return CSS_OK;

	while (n != NULL) {
		dom_node_type type;

		err = dom_node_get_node_type(n, &type);
		if (err != DOM_NO_ERR) {
			dom_node_unref(n);
			return CSS_OK;
		}

		if (type == DOM_ELEMENT_NODE)
			break;

		err = dom_node_get_previous_sibling(n, &prev);
		if (err != DOM_NO_ERR) {
			dom_node_unref(n);
			return CSS_OK;
		}

		dom_node_unref(n);
		n = prev;
	}

	if (n != NULL) {
		dom_string *name;

		err = dom_node_get_node_name(n, &name);
		if (err != DOM_NO_ERR) {
			dom_node_unref(n);
			return CSS_OK;
		}

		dom_node_unref(n);

		if (dom_string_caseless_lwc_isequal(name, qname->name)) {
			*sibling = n;
		}

		dom_string_unref(name);
	}

	return CSS_OK;
}

/**
 * Callback to find a named generic sibling node.
 *
 * \param pw       HTML document
 * \param node     DOM node
 * \param qname    Node name to search for
 * \param sibling  Pointer to location to receive ancestor
 * \return CSS_OK.
 *
 * \post \a sibling will contain the result, or NULL if there is no match
 */
static css_error named_generic_sibling_node(void *pw, void *node,
		const css_qname *qname, void **sibling)
{
	dom_node *n = node;
	dom_node *prev;
	dom_exception err;

	(void)(pw);

	*sibling = NULL;

	err = dom_node_get_previous_sibling(n, &n);
	if (err != DOM_NO_ERR)
		return CSS_OK;

	while (n != NULL) {
		dom_node_type type;
		dom_string *name;

		err = dom_node_get_node_type(n, &type);
		if (err != DOM_NO_ERR) {
			dom_node_unref(n);
			return CSS_OK;
		}

		if (type == DOM_ELEMENT_NODE) {
			err = dom_node_get_node_name(n, &name);
			if (err != DOM_NO_ERR) {
				dom_node_unref(n);
				return CSS_OK;
			}

			if (dom_string_caseless_lwc_isequal(name,
					qname->name)) {
				dom_string_unref(name);
				dom_node_unref(n);
				*sibling = n;
				break;
			}
			dom_string_unref(name);
		}

		err = dom_node_get_previous_sibling(n, &prev);
		if (err != DOM_NO_ERR) {
			dom_node_unref(n);
			return CSS_OK;
		}

		dom_node_unref(n);
		n = prev;
	}

	return CSS_OK;
}

/**
 * Callback to retrieve the parent of a node.
 *
 * \param pw      HTML document
 * \param node    DOM node
 * \param parent  Pointer to location to receive parent
 * \return CSS_OK.
 *
 * \post \a parent will contain the result, or NULL if there is no match
 */
static css_error parent_node(void *pw, void *node, void **parent)
{
	(void)(pw);

	dom_element_parent_node(node, (struct dom_element **)parent);
	dom_node_unref(*parent);

	return CSS_OK;
}

/**
 * Callback to retrieve the preceding sibling of a node.
 *
 * \param pw       HTML document
 * \param node     DOM node
 * \param sibling  Pointer to location to receive sibling
 * \return CSS_OK.
 *
 * \post \a sibling will contain the result, or NULL if there is no match
 */
static css_error sibling_node(void *pw, void *node, void **sibling)
{
	dom_node *n = node;
	dom_node *prev;
	dom_exception err;

	(void)(pw);

	*sibling = NULL;

	/* Find sibling element */
	err = dom_node_get_previous_sibling(n, &n);
	if (err != DOM_NO_ERR)
		return CSS_OK;

	while (n != NULL) {
		dom_node_type type;

		err = dom_node_get_node_type(n, &type);
		if (err != DOM_NO_ERR) {
			dom_node_unref(n);
			return CSS_OK;
		}

		if (type == DOM_ELEMENT_NODE)
			break;

		err = dom_node_get_previous_sibling(n, &prev);
		if (err != DOM_NO_ERR) {
			dom_node_unref(n);
			return CSS_OK;
		}

		dom_node_unref(n);
		n = prev;
	}

	if (n != NULL) {
		/** \todo Sort out reference counting */
		dom_node_unref(n);

		*sibling = n;
	}

	return CSS_OK;
}

/**
 * Callback to determine if a node has the given name.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_has_name(void *pw, void *node,
		const css_qname *qname, bool *match)
{
	dom_node *n = node;

	(void)(pw);

	if (lwc_string_length(qname->name) == 1 &&
	    lwc_string_data(qname->name)[0] == '*') {
		*match = true;

	} else {
		dom_string *name;
		dom_exception err;

		err = dom_node_get_node_name(n, &name);
		if (err != DOM_NO_ERR) {
			return CSS_BADPARM;
		}

		*match = dom_string_caseless_lwc_isequal(name, qname->name);

		dom_string_unref(name);
	}

	return CSS_OK;
}

/**
 * Callback to determine if a node has the given class.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param name   Name to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_has_class(void *pw, void *node,
		lwc_string *name, bool *match)
{
	dom_node *n = node;
	dom_exception err;

	(void)(pw);

	err = dom_element_has_class(n, name, match);

	assert(err == DOM_NO_ERR);

	return CSS_OK;
}

/**
 * Callback to determine if a node has the given id.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param name   Name to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_has_id(void *pw, void *node,
		lwc_string *name, bool *match)
{
	dom_node *n = node;
	dom_string *attr;
	dom_exception err;

	(void)(pw);

	*match = false;

	err = dom_html_element_get_id(n, &attr);
	if (err != DOM_NO_ERR)
		return CSS_OK;

	if (attr != NULL) {
		*match = dom_string_lwc_isequal(attr, name);

		dom_string_unref(attr);
	}

	return CSS_OK;
}

/**
 * Callback to determine if a node has an attribute with the given name.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_has_attribute(void *pw, void *node,
		const css_qname *qname, bool *match)
{
	dom_node *n = node;
	dom_string *name;
	dom_exception err;

	(void)(pw);

	err = dom_string_create_interned(
			(const uint8_t *) lwc_string_data(qname->name),
			lwc_string_length(qname->name), &name);
	if (err != DOM_NO_ERR)
		return CSS_NOMEM;

	err = dom_element_has_attribute(n, name, match);
	if (err != DOM_NO_ERR) {
		dom_string_unref(name);
		return CSS_OK;
	}

	dom_string_unref(name);

	return CSS_OK;
}

/**
 * Callback to determine if a node has an attribute with given name and value.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param value  Value to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_has_attribute_equal(void *pw, void *node,
		const css_qname *qname, lwc_string *value,
		bool *match)
{
	dom_node *n = node;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	(void)(pw);

	size_t vlen = lwc_string_length(value);

	if (vlen == 0) {
		*match = false;
		return CSS_OK;
	}

	err = dom_string_create_interned(
		(const uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	if (err != DOM_NO_ERR)
		return CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	if ((err != DOM_NO_ERR) || (atr_val == NULL)) {
		dom_string_unref(name);
		*match = false;
		return CSS_OK;
	}

	dom_string_unref(name);

	*match = dom_string_caseless_lwc_isequal(atr_val, value);

	dom_string_unref(atr_val);

	return CSS_OK;
}

/**
 * Callback to determine if a node has an attribute with the given name whose
 * value dashmatches that given.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param value  Value to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_has_attribute_dashmatch(void *pw, void *node,
		const css_qname *qname, lwc_string *value,
		bool *match)
{
	dom_node *n = node;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	(void)(pw);

	size_t vlen = lwc_string_length(value);

	if (vlen == 0) {
		*match = false;
		return CSS_OK;
	}

	err = dom_string_create_interned(
		(const uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	if (err != DOM_NO_ERR)
		return CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	if ((err != DOM_NO_ERR) || (atr_val == NULL)) {
		dom_string_unref(name);
		*match = false;
		return CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, value);

	/* check for dashmatch */
	if (*match == false) {
		const char *vdata = lwc_string_data(value);
		const char *data = (const char *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		if (len > vlen && data[vlen] == '-' &&
		    strncmpi(data, vdata, vlen) == 0) {
				*match = true;
		}
	}

	dom_string_unref(atr_val);

	return CSS_OK;
}

/**
 * Callback to determine if a node has an attribute with the given name whose
 * value includes that given.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param value  Value to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_has_attribute_includes(void *pw, void *node,
		const css_qname *qname, lwc_string *value,
		bool *match)
{
	dom_node *n = node;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;
	size_t vlen = lwc_string_length(value);
	const char *p;
	const char *start;
	const char *end;

	(void)(pw);

	*match = false;

	if (vlen == 0) {
		return CSS_OK;
	}

	err = dom_string_create_interned(
		(const uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	if (err != DOM_NO_ERR)
		return CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	if ((err != DOM_NO_ERR) || (atr_val == NULL)) {
		dom_string_unref(name);
		*match = false;
		return CSS_OK;
	}

	dom_string_unref(name);

	/* check for match */
	start = (const char *) dom_string_data(atr_val);
	end = start + dom_string_byte_length(atr_val);

	for (p = start; p <= end; p++) {
		if (*p == ' ' || *p == '\0') {
			if ((size_t) (p - start) == vlen &&
			    strncmpi(start,
					lwc_string_data(value),
					vlen) == 0) {
				*match = true;
				break;
			}

			start = p + 1;
		}
	}

	dom_string_unref(atr_val);

	return CSS_OK;
}

/**
 * Callback to determine if a node has an attribute with the given name whose
 * value has the prefix given.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param value  Value to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_has_attribute_prefix(void *pw, void *node,
		const css_qname *qname, lwc_string *value,
		bool *match)
{
	dom_node *n = node;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	(void)(pw);

	size_t vlen = lwc_string_length(value);

	if (vlen == 0) {
		*match = false;
		return CSS_OK;
	}

	err = dom_string_create_interned(
		(const uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	if (err != DOM_NO_ERR)
		return CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	if ((err != DOM_NO_ERR) || (atr_val == NULL)) {
		dom_string_unref(name);
		*match = false;
		return CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, value);

	/* check for prefix match */
	if (*match == false) {
		const char *data = (const char *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		if ((len >= vlen) &&
		    (strncmpi(data, lwc_string_data(value), vlen) == 0)) {
			*match = true;
		}
	}

	dom_string_unref(atr_val);

	return CSS_OK;
}

/**
 * Callback to determine if a node has an attribute with the given name whose
 * value has the suffix given.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param value  Value to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_has_attribute_suffix(void *pw, void *node,
		const css_qname *qname, lwc_string *value,
		bool *match)
{
	dom_node *n = node;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	(void)(pw);

	size_t vlen = lwc_string_length(value);

	if (vlen == 0) {
		*match = false;
		return CSS_OK;
	}

	err = dom_string_create_interned(
		(const uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	if (err != DOM_NO_ERR)
		return CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	if ((err != DOM_NO_ERR) || (atr_val == NULL)) {
		dom_string_unref(name);
		*match = false;
		return CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, value);

	/* check for prefix match */
	if (*match == false) {
		const char *data = (const char *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);

		const char *start = (char *) data + len - vlen;

		if ((len >= vlen) &&
		    (strncmpi(start, lwc_string_data(value), vlen) == 0)) {
			*match = true;
		}


	}

	dom_string_unref(atr_val);

	return CSS_OK;
}

/**
 * Callback to determine if a node has an attribute with the given name whose
 * value contains the substring given.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param qname  Name to match
 * \param value  Value to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK on success,
 *         CSS_NOMEM on memory exhaustion.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_has_attribute_substring(void *pw, void *node,
		const css_qname *qname, lwc_string *value,
		bool *match)
{
	dom_node *n = node;
	dom_string *name;
	dom_string *atr_val;
	dom_exception err;

	(void)(pw);

	size_t vlen = lwc_string_length(value);

	if (vlen == 0) {
		*match = false;
		return CSS_OK;
	}

	err = dom_string_create_interned(
		(const uint8_t *) lwc_string_data(qname->name),
		lwc_string_length(qname->name), &name);
	if (err != DOM_NO_ERR)
		return CSS_NOMEM;

	err = dom_element_get_attribute(n, name, &atr_val);
	if ((err != DOM_NO_ERR) || (atr_val == NULL)) {
		dom_string_unref(name);
		*match = false;
		return CSS_OK;
	}

	dom_string_unref(name);

	/* check for exact match */
	*match = dom_string_caseless_lwc_isequal(atr_val, value);

	/* check for prefix match */
	if (*match == false) {
		const char *vdata = lwc_string_data(value);
		const char *start = (const char *) dom_string_data(atr_val);
		size_t len = dom_string_byte_length(atr_val);
		const char *last_start = start + len - vlen;

		if (len >= vlen) {
			while (start <= last_start) {
				if (strncmpi(start, vdata,
						vlen) == 0) {
					*match = true;
					break;
				}

				start++;
			}
		}
	}

	dom_string_unref(atr_val);

	return CSS_OK;
}

/**
 * Callback to determine if a node is the root node of the document.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_is_root(void *pw, void *node, bool *match)
{
	dom_node *n = node;
	dom_node *parent;
	dom_node_type type;
	dom_exception err;

	(void)(pw);

	err = dom_node_get_parent_node(n, &parent);
	if (err != DOM_NO_ERR) {
		return CSS_NOMEM;
	}

	if (parent != NULL) {
		err = dom_node_get_node_type(parent, &type);

		dom_node_unref(parent);

		if (err != DOM_NO_ERR)
			return CSS_NOMEM;

		if (type != DOM_DOCUMENT_NODE) {
			*match = false;
			return CSS_OK;
		}
	}

	*match = true;

	return CSS_OK;
}

static int
node_count_siblings_check(dom_node *node,
			  bool check_name,
			  dom_string *name)
{
	dom_node_type type;
	int ret = 0;
	dom_exception exc;

	if (node == NULL)
		return 0;

	exc = dom_node_get_node_type(node, &type);
	if ((exc != DOM_NO_ERR) || (type != DOM_ELEMENT_NODE)) {
		return 0;
	}

	if (check_name) {
		dom_string *node_name = NULL;
		exc = dom_node_get_node_name(node, &node_name);

		if ((exc == DOM_NO_ERR) && (node_name != NULL)) {

			if (dom_string_caseless_isequal(name,
							node_name)) {
				ret = 1;
			}
			dom_string_unref(node_name);
		}
	} else {
		ret = 1;
	}

	return ret;
}

/**
 * Callback to count a node's siblings.
 *
 * \param pw         HTML document
 * \param n          DOM node
 * \param same_name  Only count siblings with the same name, or all
 * \param after      Count following instead of preceding siblings
 * \param count      Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a count will contain the number of siblings
 */
static css_error node_count_siblings(void *pw, void *n, bool same_name,
		bool after, int32_t *count)
{
	int32_t cnt = 0;
	dom_exception exc;
	dom_string *node_name = NULL;

	(void)(pw);

	if (same_name) {
		dom_node *node = n;
		exc = dom_node_get_node_name(node, &node_name);
		if ((exc != DOM_NO_ERR) || (node_name == NULL)) {
			return CSS_NOMEM;
		}
	}

	if (after) {
		dom_node *node = dom_node_ref(n);
		dom_node *next;

		do {
			exc = dom_node_get_next_sibling(node, &next);
			if ((exc != DOM_NO_ERR))
				break;

			dom_node_unref(node);
			node = next;

			cnt += node_count_siblings_check(node, same_name, node_name);
		} while (node != NULL);
	} else {
		dom_node *node = dom_node_ref(n);
		dom_node *next;

		do {
			exc = dom_node_get_previous_sibling(node, &next);
			if ((exc != DOM_NO_ERR))
				break;

			dom_node_unref(node);
			node = next;

			cnt += node_count_siblings_check(node, same_name, node_name);

		} while (node != NULL);
	}

	if (node_name != NULL) {
		dom_string_unref(node_name);
	}

	*count = cnt;
	return CSS_OK;
}

/**
 * Callback to determine if a node is empty.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node is empty and false otherwise.
 */
static css_error node_is_empty(void *pw, void *node, bool *match)
{
	dom_node *n = node, *next;
	dom_exception err;

	(void)(pw);

	*match = true;

	err = dom_node_get_first_child(n, &n);
	if (err != DOM_NO_ERR) {
		return CSS_BADPARM;
	}

	while (n != NULL) {
		dom_node_type ntype;
		err = dom_node_get_node_type(n, &ntype);
		if (err != DOM_NO_ERR) {
			dom_node_unref(n);
			return CSS_BADPARM;
		}

		if (ntype == DOM_ELEMENT_NODE ||
		    ntype == DOM_TEXT_NODE) {
			*match = false;
			dom_node_unref(n);
			break;
		}

		err = dom_node_get_next_sibling(n, &next);
		if (err != DOM_NO_ERR) {
			dom_node_unref(n);
			return CSS_BADPARM;
		}
		dom_node_unref(n);
		n = next;
	}

	return CSS_OK;
}

/**
 * Callback to determine if a node is a linking element.
 *
 * \param pw     HTML document
 * \param n      DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_is_link(void *pw, void *n, bool *match)
{
	dom_string *node_name = NULL;
	dom_node *node = n;
	dom_exception exc;

	(void)(pw);

	exc = dom_node_get_node_name(node, &node_name);
	if ((exc != DOM_NO_ERR) || (node_name == NULL)) {
		return CSS_NOMEM;
	}

	if (dom_string_length(node_name) == 1 &&
	    dom_string_data(node_name)[0] == 'a') {
		bool has_href;
		exc = dom_element_has_attribute(node, sd_style_g.dom_str_href,
				&has_href);
		if ((exc == DOM_NO_ERR) && (has_href)) {
			*match = true;
		} else {
			*match = false;
		}
	} else {
		*match = false;
	}
	dom_string_unref(node_name);

	return CSS_OK;
}

/**
 * Callback to determine if a node is a linking element whose target has been
 * visited.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_is_visited(void *pw, void *node, bool *match)
{
	(void)(pw);
	(void)(node);
	*match = false;
	return CSS_OK;
}

/**
 * Callback to determine if a node is currently being hovered over.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_is_hover(void *pw, void *node, bool *match)
{
	(void)(pw);
	(void)(node);
	*match = false;
	return CSS_OK;
}

/**
 * Callback to determine if a node is currently activated.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_is_active(void *pw, void *node, bool *match)
{
	(void)(pw);
	(void)(node);
	*match = false;
	return CSS_OK;
}

/**
 * Callback to determine if a node has the input focus.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_is_focus(void *pw, void *node, bool *match)
{
	(void)(pw);
	(void)(node);
	*match = false;
	return CSS_OK;
}

/**
 * Callback to determine if a node is enabled.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match with contain true if the node is enabled and false otherwise.
 */
static css_error node_is_enabled(void *pw, void *node, bool *match)
{
	(void)(pw);
	(void)(node);
	*match = false;
	return CSS_OK;
}

/**
 * Callback to determine if a node is disabled.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match with contain true if the node is disabled and false otherwise.
 */
static css_error node_is_disabled(void *pw, void *node, bool *match)
{
	(void)(pw);
	(void)(node);
	*match = false;
	return CSS_OK;
}

/**
 * Callback to determine if a node is checked.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match with contain true if the node is checked and false otherwise.
 */
static css_error node_is_checked(void *pw, void *node, bool *match)
{
	(void)(pw);
	(void)(node);
	*match = false;
	return CSS_OK;
}

/**
 * Callback to determine if a node is the target of the document URL.
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match with contain true if the node matches and false otherwise.
 */
static css_error node_is_target(void *pw, void *node, bool *match)
{
	(void)(pw);
	(void)(node);
	*match = false;
	return CSS_OK;
}

/**
 * Callback to determine if a node has the given language
 *
 * \param pw     HTML document
 * \param node   DOM node
 * \param lang   Language specifier to match
 * \param match  Pointer to location to receive result
 * \return CSS_OK.
 *
 * \post \a match will contain true if the node matches and false otherwise.
 */
static css_error node_is_lang(void *pw, void *node,
		lwc_string *lang, bool *match)
{
	(void)(pw);
	(void)(lang);
	(void)(node);
	*match = false;
	return CSS_OK;
}

/**
 * Callback to retrieve the User-Agent defaults for a CSS property.
 *
 * \param pw        HTML document
 * \param property  Property to retrieve defaults for
 * \param hint      Pointer to hint object to populate
 * \return CSS_OK       on success,
 *         CSS_INVALID  if the property should not have a user-agent default.
 */
static css_error ua_default_for_property(void *pw, uint32_t property, css_hint *hint)
{
	(void)(pw);

	if (property == CSS_PROP_COLOR) {
		hint->data.color = 0xff000000;
		hint->status = CSS_COLOR_COLOR;
	} else if (property == CSS_PROP_FONT_FAMILY) {
		hint->data.strings = NULL;
		hint->status = CSS_FONT_FAMILY_SERIF;
	} else if (property == CSS_PROP_QUOTES) {
		/** \todo Not exactly useful :) */
		hint->data.strings = NULL;
		hint->status = CSS_QUOTES_NONE;
	} else if (property == CSS_PROP_VOICE_FAMILY) {
		/** \todo Fix this when we have voice-family done */
		hint->data.strings = NULL;
		hint->status = 0;
	} else {
		return CSS_INVALID;
	}

	return CSS_OK;
}

static css_select_handler selection_handler;

static void sd_style__dom_user_data_handler_libcss(
		dom_node_operation operation,
		void *data,
		struct dom_node *src,
		struct dom_node *dst)
{
	css_error error;

	switch (operation) {
	case DOM_NODE_CLONED:
		error = css_libcss_node_data_handler(&selection_handler,
				CSS_NODE_CLONED,
				NULL, src, dst, data);
		if (error != CSS_OK) {
			fprintf(stderr, "Failed to clone libcss_node_data.\n");
		}
		break;

	case DOM_NODE_RENAMED:
		error = css_libcss_node_data_handler(&selection_handler,
				CSS_NODE_MODIFIED,
				NULL, src, NULL, data);
		if (error != CSS_OK) {
			fprintf(stderr, "Failed to update libcss_node_data.\n");
		}
		break;

	case DOM_NODE_IMPORTED:
	case DOM_NODE_ADOPTED:
	case DOM_NODE_DELETED:
		error = css_libcss_node_data_handler(&selection_handler,
				CSS_NODE_DELETED,
				NULL, src, NULL, data);
		if (error != CSS_OK) {
			fprintf(stderr, "Failed to delete libcss_node_data.\n");
		}
		break;

	default:
		fprintf(stderr, "User data operation not handled.\n");
	}
}

static void sd_style__dom_user_data_handler(dom_node_operation operation,
		dom_string *key, void *data, struct dom_node *src,
		struct dom_node *dst)
{
	dom_string *key_style = sd_style_g.dom_str__sd_key_style_node_data;
	dom_string *key_libcss = sd_style_g.dom_str__sd_key_libcss_node_data;

	if (data == NULL) {
		return;
	}

	if (dom_string_isequal(key_libcss, key)) {
		sd_style__dom_user_data_handler_libcss(operation, data,
				src, dst);

	} else if (dom_string_isequal(key_style, key)) {
		/* TODO: Support other operations. */
		assert(operation == DOM_NODE_DELETED);
		css_select_results_destroy(data);
	}
}

static css_error set_libcss_node_data(void *pw, void *node, void *libcss_node_data)
{
	dom_node *n = node;
	dom_exception err;
	void *old_node_data;

	(void)(pw);

	err = dom_node_set_user_data(n,
			sd_style_g.dom_str__sd_key_libcss_node_data,
			libcss_node_data, sd_style__dom_user_data_handler,
			(void *) &old_node_data);
	if (err != DOM_NO_ERR) {
		return CSS_NOMEM;
	}

	assert(old_node_data == NULL);

	return CSS_OK;
}

static css_error get_libcss_node_data(void *pw, void *node, void **libcss_node_data)
{
	dom_node *n = node;
	dom_exception err;

	(void)(pw);

	err = dom_node_get_user_data(n,
			sd_style_g.dom_str__sd_key_libcss_node_data,
			libcss_node_data);
	if (err != DOM_NO_ERR) {
		return CSS_NOMEM;
	}

	return CSS_OK;
}

static css_error node_presentational_hint(void *pw, void *node,
		uint32_t *nhints, css_hint **hints)
{
	(void)(pw);
	(void)(node);

	*nhints = 0;
	*hints = NULL;

	return CSS_OK;
}

static css_select_handler selection_handler = {
	CSS_SELECT_HANDLER_VERSION_1,

	node_name,
	node_classes,
	node_id,
	named_ancestor_node,
	named_parent_node,
	named_sibling_node,
	named_generic_sibling_node,
	parent_node,
	sibling_node,
	node_has_name,
	node_has_class,
	node_has_id,
	node_has_attribute,
	node_has_attribute_equal,
	node_has_attribute_dashmatch,
	node_has_attribute_includes,
	node_has_attribute_prefix,
	node_has_attribute_suffix,
	node_has_attribute_substring,
	node_is_root,
	node_count_siblings,
	node_is_empty,
	node_is_link,
	node_is_visited,
	node_is_hover,
	node_is_active,
	node_is_focus,
	node_is_enabled,
	node_is_disabled,
	node_is_checked,
	node_is_target,
	node_is_lang,
	node_presentational_hint,
	ua_default_for_property,
	set_libcss_node_data,
	get_libcss_node_data,
};

struct sd_style_ctx {
	css_select_ctx *select_ctx;
	dom_node *root;
	bool res;
};

static bool sd_style__element_compose(
		struct sd_style_ctx *ctx,
		dom_node *node,
		css_select_results *style)
{
	css_error cerr;
	int pseudo_element;

	if (node != ctx->root) {
		dom_exception derr;
		dom_element *parent;
		css_select_results *pstyle;
		css_computed_style *composed;

		derr = dom_element_parent_node((dom_element *)node, &parent);
		if (derr != DOM_NO_ERR) {
			return false;
		}

		derr = dom_node_get_user_data(parent,
				sd_style_g.dom_str__sd_key_style_node_data,
				&pstyle);
		dom_node_unref(parent);
		if (derr != DOM_NO_ERR || pstyle == NULL) {
			return false;
		}

		cerr = css_computed_style_compose(
				pstyle->styles[CSS_PSEUDO_ELEMENT_NONE],
				style->styles[CSS_PSEUDO_ELEMENT_NONE],
				&sd_style_g.unit_len_ctx, &composed);
		if (cerr != CSS_OK) {
			return false;
		}

		pseudo_element = CSS_PSEUDO_ELEMENT_NONE;
		css_computed_style_destroy(style->styles[pseudo_element]);
		style->styles[pseudo_element] = composed;
	}

	for (pseudo_element = CSS_PSEUDO_ELEMENT_NONE + 1;
			pseudo_element < CSS_PSEUDO_ELEMENT_COUNT;
			pseudo_element++) {
		css_computed_style *composed;

		if (style->styles[pseudo_element] == NULL)
			continue;

		cerr = css_computed_style_compose(
				style->styles[CSS_PSEUDO_ELEMENT_NONE],
				style->styles[pseudo_element],
				&sd_style_g.unit_len_ctx, &composed);
		if (cerr != CSS_OK) {
			return false;
		}

		css_computed_style_destroy(style->styles[pseudo_element]);
		style->styles[pseudo_element] = composed;
	}

	return true;
}

static bool sd_style__element(
		struct sd_style_ctx *ctx,
		dom_node *node)
{
	bool res;
	css_error cerr;
	dom_exception derr;
	void *old_node_data;
	css_media media = {
		.type = CSS_MEDIA_SCREEN,
	};
	css_select_results *style;

	cerr = css_select_style(ctx->select_ctx, node,
			&sd_style_g.unit_len_ctx, &media, NULL,
			&selection_handler, ctx, &style);
	if (cerr != CSS_OK) {
		fprintf(stderr, "Failed to select element style.\n");
		return false;
	}

	res = sd_style__element_compose(ctx, node, style);
	if (res != true) {
		css_select_results_destroy(style);
		return false;
	}

	derr = dom_node_set_user_data(node,
			sd_style_g.dom_str__sd_key_style_node_data,
			style, sd_style__dom_user_data_handler,
			(void *) &old_node_data);
	if (derr != DOM_NO_ERR) {
		css_select_results_destroy(style);
		return false;
	}

	assert(old_node_data == NULL);

	return true;
}

static enum dom_walk_cmd sd_style__document_cb(
		enum dom_walk_stage stage,
		dom_node_type type,
		dom_node *node,
		void *pw)
{
	struct sd_style_ctx *ctx = pw;

	(void)(stage);
	assert(stage == DOM_WALK_STAGE_ENTER);

	switch (type) {
	case DOM_ELEMENT_NODE:
		ctx->res = sd_style__element(ctx, node);
		if (ctx->res != true) {
			return DOM_WALK_CMD_ABORT;
		}
		break;

	default:
		break;
	}

	return DOM_WALK_CMD_CONTINUE;
}

static bool sd_style__document(
		css_select_ctx *select_ctx,
		dom_document *doc)
{
	bool res;
	dom_exception derr;
	dom_node *root = NULL;
	struct sd_style_ctx ctx = {
		.select_ctx = select_ctx,
		.res = true,
	};

	derr = dom_document_get_document_element(doc, &root);
	if (derr != DOM_NO_ERR || root == NULL) {
		fprintf(stderr, "%s: Failed to get root element\n", __func__);
		return false;
	}
	ctx.root = root;

	res = sd_style__element(&ctx, root);
	if (res != true) {
		dom_node_unref(root);
		return res;
	}

	derr = libdom_treewalk(DOM_WALK_ENABLE_ENTER,
			sd_style__document_cb,
			root, &ctx);
	dom_node_unref(root);
	if (derr != DOM_NO_ERR) {
		fprintf(stderr, "%s: Failed to walk DOM\n", __func__);
		return false;
	}
	if (ctx.res != true) {
		return ctx.res;
	}

	return true;
}

bool sd_style_annotate(
		dom_document *doc,
		css_stylesheet *sheet_ua,
		css_stylesheet *sheet_user)
{
	css_select_ctx *select_ctx = NULL;
	bool res = true;
	css_error cerr;

	cerr = css_select_ctx_create(&select_ctx);
	if (cerr != CSS_OK || select_ctx == NULL) {
		fprintf(stderr, "%s: Failed to create selection ctx\n",
				__func__);
		res = false;
		goto out;
	}

	cerr = css_select_ctx_append_sheet(select_ctx, sheet_ua,
			CSS_ORIGIN_UA, NULL);
	if (cerr != CSS_OK) {
		fprintf(stderr, "%s: Failed to add sheet to selection ctx\n",
				__func__);
		res = false;
		goto out;
	}

	cerr = css_select_ctx_append_sheet(select_ctx, sheet_user,
			CSS_ORIGIN_USER, NULL);
	if (cerr != CSS_OK) {
		fprintf(stderr, "%s: Failed to add sheet to selection ctx\n",
				__func__);
		res = false;
		goto out;
	}

	res = sd_style__document(select_ctx, doc);
	if (res != true) {
		goto out;
	}

out:
	if (select_ctx != NULL) {
		cerr = css_select_ctx_destroy(select_ctx);
		if (cerr != CSS_OK) {
			fprintf(stderr, "%s: Failed to destroy selection ctx\n",
					__func__);
		}
	}
	return res;
}

static bool sd_style__get(
		dom_node *node,
		css_select_results **style)
{
	dom_exception derr;

	derr = dom_node_get_user_data(node,
			sd_style_g.dom_str__sd_key_style_node_data,
			style);
	if (derr != DOM_NO_ERR) {
		return false;
	}

	return true;
}

bool sd_style_get(
		dom_node *node,
		dom_node_type type,
		css_select_results **style)
{
	switch (type) {
	case DOM_TEXT_NODE:
		node = dom_node_ref(node);
		while (type != DOM_ELEMENT_NODE) {
			dom_node *next;
			dom_exception derr;

			derr = dom_node_get_parent_node(node, &next);
			dom_node_unref(node);
			node = next;
			if (derr != DOM_NO_ERR || node == NULL) {
				return false;
			}

			derr = dom_node_get_node_type(node, &type);
			if (derr != DOM_NO_ERR) {
				dom_node_unref(node);
				return false;
			}
		}
		dom_node_unref(node);
		/* Fall through. */

	case DOM_ELEMENT_NODE:
		return sd_style__get(node, style);

	default:
		return false;
	}

	return true;
}
