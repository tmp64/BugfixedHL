#
# Finds Steam API library
#

if( WIN32 )
	find_library( STEAMAPI_LIB NAMES steam_api.lib PATHS ${CMAKE_SOURCE_DIR}/lib/public NO_DEFAULT_PATH )

	include( FindPackageHandleStandardArgs )
	find_package_handle_standard_args( SteamAPI DEFAULT_MSG STEAMAPI_LIB )

	if( STEAMAPI_LIB )
		add_library( SteamAPI SHARED IMPORTED )
		set_property( TARGET SteamAPI PROPERTY IMPORTED_IMPLIB ${STEAMAPI_LIB} )
	endif()

	unset( STEAMAPI_LIB CACHE )
endif()
