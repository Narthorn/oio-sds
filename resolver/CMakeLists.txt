add_definitions(-DG_LOG_DOMAIN="oio.resolver")

include_directories(BEFORE . ..)

include_directories(AFTER
		${CMAKE_CURRENT_BINARY_DIR}/..
		${CMAKE_CURRENT_BINARY_DIR}/../metautils/lib)

add_library(hcresolve SHARED
		hc_resolver.c)

target_link_libraries(hcresolve
		meta0remote meta1remote metautils gridcluster
		${GLIB2_LIBRARIES})

install(FILES
			hc_resolver.h
		DESTINATION include/resolver
		PERMISSIONS
			OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)

install(TARGETS hcresolve
		LIBRARY DESTINATION ${LD_LIBDIR}
		RUNTIME DESTINATION bin)

