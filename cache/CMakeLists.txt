add_definitions(-DG_LOG_DOMAIN="oio.cache")

include_directories(BEFORE . ..)

add_library(oiocache SHARED cache.c cache_noop.c cache_lru.c)
target_link_libraries(oiocache metautils ${GLIB2_LIBRARIES})

install(TARGETS oiocache 
		LIBRARY DESTINATION ${LD_LIBDIR}
		RUNTIME DESTINATION bin
		PUBLIC_HEADER DESTINATION include)

add_executable(test_cache test_cache.c)
target_link_libraries(test_cache oiocache)

add_test(NAME cache/abstract COMMAND test_cache)

# Optional cache libraries
pkg_search_module(HIREDIS hiredis)

if (HIREDIS_FOUND)
	add_library(oiocache_redis SHARED cache_redis.c)
	target_link_libraries(oiocache_redis ${HIREDIS_LIBRARIES})

	add_executable(test_cache_redis test_cache_redis.c)
	target_link_libraries(test_cache_redis oiocache oiocache_redis)

	install(TARGETS oiocache_redis
	        LIBRARY DESTINATION ${LD_LIBDIR}
			RUNTIME DESTINATION bin
			PUBLIC_HEADER DESTINATION include)

	add_test(NAME cache/redis COMMAND test_cache_redis)
endif ()
