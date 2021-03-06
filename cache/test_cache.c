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

#include "test_cache_abstract.c"

#include <core/oiolog.h>

static void
test_cache_cycle_noop (void)
{
	struct oio_cache_s *c = oio_cache_make_NOOP ();
	test_cache_cycle (c);
	oio_cache_destroy (c);
}

static void
test_cache_cycle_lru (void)
{
	struct lru_tree_s *lru = lru_tree_create (
			(GCompareFunc)g_strcmp0, g_free, g_free, 0);
	g_assert_nonnull (lru);

	struct oio_cache_s *c = oio_cache_make_LRU (lru);

	test_cache_cycle (c);
	oio_cache_destroy (c);
}

int
main (int argc, char **argv)
{
	g_test_init (&argc, &argv, NULL);
	oio_log_lazy_init ();
	oio_log_init_level(GRID_LOGLVL_INFO);
	g_log_set_default_handler(oio_log_stderr, NULL);

	g_test_add_func("/cache/cycle/noop", test_cache_cycle_noop);
	g_test_add_func("/cache/cycle/lru", test_cache_cycle_lru);
	return g_test_run();
}

