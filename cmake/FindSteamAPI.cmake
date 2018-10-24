#
# Finds Steam API library
#

find_library( STEAMAPI_LIB NAMES steam_api.lib libsteam_api.so PATHS ${CMAKE_SOURCE_DIR}/lib/public NO_DEFAULT_PATH )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( SteamAPI DEFAULT_MSG STEAMAPI_LIB )

if( STEAMAPI_LIB )
	add_library( SteamAPI SHARED IMPORTED )

	if( MSVC )
		set_property( TARGET SteamAPI PROPERTY IMPORTED_IMPLIB ${STEAMAPI_LIB} )
	else()
		set_property( TARGET SteamAPI PROPERTY IMPORTED_LOCATION ${STEAMAPI_LIB} )
	endif()
endif()

unset( STEAMAPI_LIB CACHE )
