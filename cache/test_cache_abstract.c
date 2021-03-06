/*
OpenIO SDS cache
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

#include <metautils/lib/lrutree.h>
#include <cache/cache.h>

static void
test_found (struct oio_cache_s *c, const char *k, const char *v)
{
	gchar *value = NULL;
	enum oio_cache_status_e rc = oio_cache_get (c, k, &value);
	g_assert_cmpint (rc, ==, OIO_CACHE_OK);
	g_assert_nonnull (value);
	g_assert_cmpstr (value, ==, v);
	g_free (value);
}

static void
test_not_found (struct oio_cache_s *c, const char *k)
{
	enum oio_cache_status_e rc;
	gchar *value = NULL;

	rc = oio_cache_get (c, k, &value);
	g_assert_null (value);
	g_assert_cmpint (rc, ==, OIO_CACHE_NOTFOUND);
}

static void
test_cache_cycle (struct oio_cache_s *c)
{
	const char *k = "NOTFOUND";
	const char *v = "plop";

	g_assert_nonnull (c);
	test_not_found (c, k);

	enum oio_cache_status_e rc = oio_cache_put (c, k, v);
	if (rc == OIO_CACHE_OK) {
		test_found (c, k, v);
		rc = oio_cache_del (c, k);
		g_assert_cmpint (rc, ==, OIO_CACHE_OK);
	} else {
		g_assert_cmpint (rc, ==, OIO_CACHE_DISCONNECTED);
	}

	test_not_found (c, k);
}

