#
# Finds DInput library
#

if( NOT MSVC )
	message( FATAL_ERROR "DInput is only supported on Windows with MSVC++" )
endif()

find_library( DINPUT_LIB NAMES dinput8.lib PATHS ${CMAKE_SOURCE_DIR}/cl_dll/dinput NO_DEFAULT_PATH )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( DINPUT DEFAULT_MSG DINPUT_LIB )

if( DINPUT_LIB )
	add_library( DINPUT SHARED IMPORTED )
	set_property( TARGET DINPUT PROPERTY IMPORTED_IMPLIB ${DINPUT_LIB} )
endif()

unset( DINPUT_LIB CACHE )
