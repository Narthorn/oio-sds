add_definitions(-DG_LOG_DOMAIN="oio.cluster.lib")

include_directories(BEFORE . .. ../..)

add_library(gridcluster SHARED
		gridcluster.c
		message.c)

set_target_properties(gridcluster PROPERTIES SOVERSION ${ABI_VERSION})

target_link_libraries(gridcluster
		metautils
		-lm ${GLIB2_LIBRARIES})

add_executable (tool_sysstat tool_sysstat.c)
target_link_libraries (tool_sysstat gridcluster)

install(FILES
			gridcluster.h
		DESTINATION include/cluster/lib
		PERMISSIONS
			OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)

install(TARGETS gridcluster
	LIBRARY DESTINATION ${LD_LIBDIR}
	RUNTIME DESTINATION bin)

