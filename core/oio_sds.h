/*
OpenIO SDS core library
Copyright (C) 2015 OpenIO, original work as part of OpenIO Software Defined Storage

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.
*/

#ifndef OIO_SDS__sdk__oio_sds_h
#define OIO_SDS__sdk__oio_sds_h 1

#ifdef __cplusplus
extern "C" {
#endif

struct oio_sds_s;
struct oio_error_s;
struct oio_url_s;

enum oio_sds_config_e
{
	/* expects an <int> as a number of seconds */
	OIOSDS_CFG_TIMEOUT_PROXY = 1,

	/* expects an <int> as a number of seconds */
	OIOSDS_CFG_TIMEOUT_RAWX,

	/* expects an <int> used for its boolean value */
	OIOSDS_CFG_FLAG_SYNCATDOWNLOAD,
};

/* API-global --------------------------------------------------------------- */

extern volatile int oio_sds_default_autocreate;

/* OpenIO SDK internally relies on GLib-2.0 logging features,
 * so this only sets a callback into GLib's system. The calling app
 * keeps the freedom to change this. */

/* Configures the GLib-2.0 to send the logging output to the syslog. This
 * function does not call openlog() */
void oio_log_to_syslog (void);

/* Configures the GLib-2.0 to send the logging output to the standard error
 * output. The format follow an internal rules of OpeIO. If the walling app
 * wants to another format, it is its responsibility. */
void oio_log_to_stderr (void);

/* As the name suggests, it turns of the log output from the OpenIO's SDK */
void oio_log_nothing (void);

/* Increases the verbosity of the internal logging output.
 * OpenIO's log levels are ERROR, WARNING, NOTICE, INFO, DEBUG, TRACE.
 * The default level is WARNING.
 * DEBUG: output the SDK behavior.
 * TRACE: also outputs the licurl behavior. */
void oio_log_more (void);

/** @return a NULL-terminated array of strings where
 * result[(2*i)]   is the name of the i-th configuration directive 
 * result[(2*i)+1] is the value of the i-th configuration directive.
 * The output has to be freed with free().
 */
char ** oio_sds_get_compile_options (void);

/* Error management --------------------------------------------------------- */

void oio_error_free (struct oio_error_s *e);

void oio_error_pfree (struct oio_error_s **pe);

int oio_error_code (const struct oio_error_s *e);

const char * oio_error_message (const struct oio_error_s *e);

/* Client-related features -------------------------------------------------- */

/* constructor */
struct oio_error_s * oio_sds_init (struct oio_sds_s **out, const char *ns);

/* destructor */
void oio_sds_free (struct oio_sds_s *sds);

/* Calls oio_sds_free() on *psds, then set it to NULL */
void oio_sds_pfree (struct oio_sds_s **psds);

/* return 0 on success, or errno in case of error */
int oio_sds_configure (struct oio_sds_s *sds, enum oio_sds_config_e what,
		void *pv, unsigned int vlen);

/* Download ----------------------------------------------------------------- */

/* Expected to return 0 when the data's managemenr succeeded, and something
 * else when it failed. */
typedef int (*oio_sds_dl_hook_f) (void*, const unsigned char*, size_t);

struct oio_sds_dl_dst_s
{
	/* output variable: how many bytes have been read, at all */
	size_t out_size;

	enum {
		OIO_DL_DST_HOOK_SEQUENTIAL = 1,
		OIO_DL_DST_BUFFER,
		OIO_DL_DST_FILE,
	} type;

	union {
		struct {
			const char *path;
		} file;
		struct {
			unsigned char *ptr;
			size_t length;
		} buffer;
		struct {
			oio_sds_dl_hook_f cb;
			void *ctx;

			/* set 'length' to ((size_t)-1) to mark it unset and allow the
			 * whole content to be downloaded. If set, it must be coherent
			 * with the ranges provided. */
			size_t length;
		} hook;
	} data;
};

struct oio_sds_dl_range_s
{
	size_t offset;
	size_t size;
};

struct oio_sds_dl_src_s
{
	struct oio_url_s *url;

	/* if not set, the whole content will be read at once.
	 * To be set, it must contain a pointer to a NULL-terminated array
	 * of pointers to ranges. */
	struct oio_sds_dl_range_s **ranges;
};

struct oio_error_s* oio_sds_download (struct oio_sds_s *sds,
		struct oio_sds_dl_src_s *src, struct oio_sds_dl_dst_s *dst);

/* Downloads the whole file
 * works with fully qualified urls (content) and local paths */
struct oio_error_s* oio_sds_download_to_file (struct oio_sds_s *sds,
		struct oio_url_s *u, const char *local);

/* Upload ------------------------------------------------------------------- */

typedef ssize_t (*oio_sds_ul_hook_f) (void*, unsigned char *p, size_t s);

struct oio_sds_ul_src_s
{
	enum {
		OIO_UL_SRC_HOOK_SEQUENTIAL = 1,
		OIO_UL_SRC_BUFFER,
		OIO_UL_SRC_FILE,
	} type;

	union {
		struct {
			const char *path;
			size_t offset;
			size_t size;
		} file;
		struct {
			unsigned char *ptr;
			size_t length;
		} buffer;
		struct {
			oio_sds_ul_hook_f cb;
			void *ctx;
			size_t size;
		} hook;
	} data;
};

struct oio_sds_ul_dst_s
{
	struct oio_url_s *url;

	/* Should the container be autocreated */
	int autocreate;

	/* output variable: how many bytes have been uploaded */
	size_t out_size;

	/* Optional: the unique content name */
	const char *content_id;
};

/* works with fully qualified urls (content) and local paths */
struct oio_error_s* oio_sds_upload (struct oio_sds_s *sds,
		struct oio_sds_ul_src_s *src, struct oio_sds_ul_dst_s *dst);

/* Simply wraps oio_sds_upload() without the autocreation flag
 * set. */
struct oio_error_s* oio_sds_upload_from_file (struct oio_sds_s *sds,
		struct oio_url_s *u, const char *local);

/* List --------------------------------------------------------------------- */

struct oio_sds_list_param_s
{
	struct oio_url_s *url;
	const char *prefix;
	const char *marker;
	const char *end;

	/* 0 means not set */
	size_t max_items;

	/* 0 means no set */
	char delimiter;

	char flag_nodeleted : 1;
	char flag_allversions : 1;
};

struct oio_sds_list_item_s
{
	const char *name;
	const char *hash;
	size_t size;
	size_t version;
};

struct oio_sds_list_listener_s
{
	void *ctx;

	/* called for each item listed */
	int (*on_item) (void *ctx, const struct oio_sds_list_item_s *item);

	/* called for each sub-prefix detected (depends on the delimiter) */
	int (*on_prefix) (void *ctx, const char *prefix);

	/* called once, with no warranty to be called before 'on_item' nor
	 * 'on_bound' */
	int (*on_bound) (void *ctx, const char *next_marker);
	size_t out_count;
	int out_truncated;
};

struct oio_error_s* oio_sds_list (struct oio_sds_s *sds,
		struct oio_sds_list_param_s *param,
		struct oio_sds_list_listener_s *listener);

/* --------------------------------------------------------------------------- */

/* works with fully qualified urls (content) */
struct oio_error_s* oio_sds_delete (struct oio_sds_s *sds, struct oio_url_s *u);

/* currently works with fully qualified urls (content) */
struct oio_error_s* oio_sds_has (struct oio_sds_s *sds, struct oio_url_s *url,
		int *phas);

/* Creates an alias named 'url' pointing on the physical content 'content_id'
 * in the same container.
 *  'url' be a fully qualified content URI.
 *  'content_id' must be an hexadecimal string. */
struct oio_error_s* oio_sds_link (struct oio_sds_s *sds, struct oio_url_s *url,
		const char *content_id);

/* Attempts a link with oio_sds_link(), then calls oio_sds_upload_from_source()
 * in case of error, if the link failed because of a content not found.
 * The underlying call to oio_sds_link() requires the 'content_id' field of
 * 'dst' to be set to a non-NULL value. */
struct oio_error_s* oio_sds_link_or_upload (struct oio_sds_s *sds,
		struct oio_sds_ul_src_s *src, struct oio_sds_ul_dst_s *dst);

/* DEPRECATED --------------------------------------------------------------- */

struct oio_source_s {
	int autocreate;
	enum {
		OIO_SRC_NONE = 0, /* do not use this */
		OIO_SRC_FILE,
	} type;
	union {
		const char *path;
	} data;
};

/* XXX @deprecated use oio_sds_upload() instead */
struct oio_error_s* oio_sds_upload_from_source (struct oio_sds_s *sds,
		struct oio_url_s *u, struct oio_source_s *src)
	__attribute__ ((deprecated));

#ifdef __cplusplus
}
#endif
#endif /*OIO_SDS__sdk__oio_sds_h*/
