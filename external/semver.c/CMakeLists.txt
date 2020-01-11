add_library( semver STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/semver.c
)

target_include_directories( semver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include )
