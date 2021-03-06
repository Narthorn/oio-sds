/*
OpenIO SDS core library
Copyright (C) 2014 Worldine, original work as part of Redcurrant
Copyright (C) 2015 OpenIO, modified as part of OpenIO Software Defined Storage

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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "oio_core.h"

struct test_data_s {
	const char *url;
	const char *whole;

	const char *ns;
	const char *account;
	const char *ref;
	const char *type;
	const char *path;

	const char *hexa;
};

#define TEST_END {NULL,NULL, NULL,NULL,NULL,NULL,NULL, NULL}

static void
_test_field (const char *v, struct oio_url_s *u, enum oio_url_field_e f)
{
	if (v) {
		g_assert (oio_url_has (u, f));
		g_assert (!strcmp (v, oio_url_get (u, f)));
	} else {
		g_assert (!oio_url_has (u, f));
		g_assert (NULL == oio_url_get (u, f));
	}
}

static void
_test_url (guint idx, struct oio_url_s *u, struct test_data_s *td)
{
	(void) idx;
	_test_field (td->whole, u, OIOURL_WHOLE);
	_test_field (td->ns, u, OIOURL_NS);
	_test_field (td->account, u, OIOURL_ACCOUNT);
	_test_field (td->ref, u, OIOURL_USER);
	_test_field (td->type, u, OIOURL_TYPE);
	_test_field (td->path, u, OIOURL_PATH);
	if (td->hexa) {
		g_assert (oio_url_has (u, OIOURL_HEXID));
		g_assert (NULL != oio_url_get_id (u));
		g_assert (!g_ascii_strcasecmp (oio_url_get (u, OIOURL_HEXID), td->hexa));
	} else {
		g_assert (!oio_url_has (u, OIOURL_HEXID));
		g_assert (NULL == oio_url_get_id (u));
		g_assert (NULL == oio_url_get (u, OIOURL_HEXID));
	}
}

static struct oio_url_s *
_init_url (struct test_data_s *td)
{
	struct oio_url_s *url = oio_url_empty ();
	if (td->ns) oio_url_set (url, OIOURL_NS, td->ns);
	if (td->account) oio_url_set (url, OIOURL_ACCOUNT, td->account);
	if (td->ref) oio_url_set (url, OIOURL_USER, td->ref);
	if (td->type) oio_url_set (url, OIOURL_TYPE, td->type);
	if (td->path) oio_url_set (url, OIOURL_PATH, td->path);
	return url;
}

static void
test_configure_valid (void)
{
	static struct test_data_s tab[] = {
		{ "/NS//JFS",
			"NS//JFS/",
			"NS", HCURL_DEFAULT_ACCOUNT, "JFS", HCURL_DEFAULT_TYPE, NULL,
			"C3F36084054557E6DBA6F001C41DAFBFEF50FCC83DB2B3F782AE414A07BB3A7A"},

		{ "NS//JFS//1.",
			"NS//JFS//1.",
			"NS", HCURL_DEFAULT_ACCOUNT, "JFS", HCURL_DEFAULT_TYPE, "1.",
			"C3F36084054557E6DBA6F001C41DAFBFEF50FCC83DB2B3F782AE414A07BB3A7A"},

		TEST_END
	};

	guint idx = 0;
	for (struct test_data_s *th=tab; th->url ;th++) {
		struct oio_url_s *url;

		url = oio_url_init(th->url);
		g_assert(url != NULL);
		_test_url (idx++, url, th);
		oio_url_pclean (&url);

		url = _init_url (th);
		g_assert(url != NULL);
		_test_url (idx++, url, th);
		oio_url_pclean (&url);
	}
}

static void
test_configure_valid_old(void)
{
	static struct test_data_s tab[] = {
		{ "/NS/JFS",
			"NS//JFS/",
			"NS", HCURL_DEFAULT_ACCOUNT, "JFS", HCURL_DEFAULT_TYPE, NULL,
			"C3F36084054557E6DBA6F001C41DAFBFEF50FCC83DB2B3F782AE414A07BB3A7A"},

		{ "/NS/JFS/1.",
			"NS//JFS//1.",
			"NS", HCURL_DEFAULT_ACCOUNT, "JFS", HCURL_DEFAULT_TYPE, "1.",
			"C3F36084054557E6DBA6F001C41DAFBFEF50FCC83DB2B3F782AE414A07BB3A7A"},

		{ "NS//JFS//1.",
			"NS//JFS//%2F1.",
			"NS", HCURL_DEFAULT_ACCOUNT, "JFS", HCURL_DEFAULT_TYPE, "/1.",
			"C3F36084054557E6DBA6F001C41DAFBFEF50FCC83DB2B3F782AE414A07BB3A7A"},

		TEST_END
	};

	guint idx = 0;
	for (struct test_data_s *th=tab; th->url ;th++) {
		struct oio_url_s *url;

		url = oio_url_oldinit(th->url);
		g_assert(url != NULL);
		_test_url (idx++, url, th);
		oio_url_pclean (&url);

		url = _init_url (th);
		g_assert(url != NULL);
		_test_url (idx++, url, th);
		oio_url_pclean (&url);
	}
}

static void
test_configure_invalid(void)
{
	struct oio_url_s *url;

	url = oio_url_oldinit("");
	g_assert(url == NULL);

	url = oio_url_oldinit("/");
	g_assert(url == NULL);
}

int
main(int argc, char **argv)
{
	g_test_init (&argc, &argv, NULL);
	oio_log_lazy_init ();
	oio_log_init_level(GRID_LOGLVL_INFO);
	g_log_set_default_handler(oio_log_stderr, NULL);

	g_test_add_func("/core/url/configure/valid_old",
			test_configure_valid_old);
	g_test_add_func("/core/url/configure/valid",
			test_configure_valid);
	g_test_add_func("/core/url/configure/invalid",
			test_configure_invalid);
	return g_test_run();
}

