###################################################
#                                                 #
#	Bugfixed and Improved HL CMake build file     #
#                                                 #
#   Based on Half-Life Enhanced CMake build file  #
#                                                 #
###################################################
cmake_minimum_required( VERSION 3.5.2 )

# Source additional modules from the "cmake" directory
list( APPEND CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_LIST_DIR}/cmake"
)

include( InputFilesList )
include( MSVCRuntime )
include( WinXPSupport )

project( BugfixedHL )

#C++11 support
set( CMAKE_CXX_FLAGS "-std=c++11" )

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" )
	# Always build as 32 bit
	# Additional debug info for GDB.
	set( SHARED_COMPILER_FLAGS "${SHARED_COMPILER_FLAGS} -m32 -g" )
endif()

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SHARED_COMPILER_FLAGS}" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SHARED_COMPILER_FLAGS}" )

unset( SHARED_COMPILER_FLAGS )

#Get the game directory
set( GAME_BIN_PATH "$ENV{GamePath}" CACHE PATH "Game directory path (where hl.exe is)" )

if( "${GAME_BIN_PATH}" STREQUAL "" )

	message( FATAL_ERROR "Unable to get game directory path" )

endif()

#Base dir for game outputs
set( GAME_BASE_PATH "${GAME_BIN_PATH}/valve" )

message( STATUS "Base Game Path: ${GAME_BASE_PATH}" )

#Shared preprocessor definitions
set( SHARED_DEFS 
)

if( MSVC )
	set( SHARED_DEFS
		${SHARED_DEFS}
		_CRT_SECURE_NO_WARNINGS
		_SCL_SECURE_NO_WARNINGS
	)
else()
	#All non-MSVC targets need this. That includes Cygwin and Mingw.
	#Match Source 2013 in defining GNUC for UNIX platforms.
	#Match Source 2013 in defining POSIX for UNIX platforms.
	set( SHARED_DEFS
		${SHARED_DEFS}
		GNUC
		POSIX
		_stricmp=strcasecmp
		_strnicmp=strncasecmp
	)

	if( APPLE )
		#Define OSX so code can check for it.
		set( SHARED_DEFS
			${SHARED_DEFS}
			OSX
		)
	endif()

	if( UNIX )
		#Define LINUX so code can check for it.
		#Define _LINUX for Source SDK dependencies.
		set( SHARED_DEFS
			${SHARED_DEFS}
			LINUX
			_LINUX
		)
	endif()
endif()

#Shared definitions for the client & server libaries.
set( SHARED_GAME_DEFS
	CLIENT_WEAPONS
	NO_MALLOC_OVERRIDE
)

#Shared linker flags
set( SHARED_GAME_LINKER_FLAGS
)

if( MSVC )
	#Set Windows subsystem
	set( SHARED_GAME_LINKER_FLAGS
		${SHARED_GAME_LINKER_FLAGS} "/SUBSYSTEM:WINDOWS "
)
elseif( UNIX )
	#From the Github 2013 Makefile, match the settings for Unix environments.
	# force 387 for FP math so the precision between win32 and linux and osx match
	# Trigger an error if any code tries to use an implicit return type
	# Default visibility is hidden unless explicitly altered with __attribute__( visibility() )
	set( SHARED_GAME_LINKER_FLAGS
		${SHARED_GAME_LINKER_FLAGS} "-mfpmath=387 -Werror=return-type -fvisibility=hidden "
	)

	if( APPLE )
		set( SHARED_GAME_LINKER_FLAGS
			${SHARED_GAME_LINKER_FLAGS} "-momit-leaf-frame-pointer -mtune=core2 "
		)
	else()
		#Linux, Cygwin, etc.
	endif()
endif()

# Warning level
if( MSVC )
	set( WARNING_LEVEL_STRICTEST "/W4 " )
else()
	set( WARNING_LEVEL_STRICTEST "-Wall -Wno-write-strings -Wno-unknown-pragmas -Wno-endif-labels -Wno-switch -Wno-unused-variable -Wno-unused-function" )
endif()

#Shared directories
#Include paths for external dependencies. Should always come after all project paths.
set( SHARED_EXTERNAL_INCLUDE_PATHS
)

set( SHARED_INCLUDE_PATHS
	common
	#public
	#public/math
	engine
	pm_shared
	game_shared
	dlls
	#source_sdk/public/tier1
	#source_sdk/public
	#source_sdk/common
	${CMAKE_SOURCE_DIR}  #Should ideally get rid of this.
)

add_subdirectory( common )
add_subdirectory( engine )
add_subdirectory( pm_shared )
add_subdirectory( game_shared )
#add_subdirectory( public )

#Save the shares sources into SHARED_SRCS
get_sources( SHARED_SRCS )

clear_sources()

#Find shared dependencies
#set( tier0_DIR ${GAME_BIN_PATH} CACHE PATH "tier0 library location" )
#find_package( tier0 MODULE REQUIRED )

#set( vstdlib_DIR ${GAME_BIN_PATH} CACHE PATH "vstdlib library location" )
#find_package( vstdlib MODULE REQUIRED )

set( THREADS_PREFER_PTHREAD_FLAG ON )
find_package( Threads REQUIRED )

if( UNIX AND NOT APPLE )
	set( UNIX_FS_LIB stdc++fs )
endif()

set( SHARED_DEPENDENCIES
	#tier0
	#vstdlib
	#tier1
)

#Set up source_sdk projects first
#add_subdirectory( source_sdk )

#--------------------------------------------------------------------------------------------------------
# Client library
#--------------------------------------------------------------------------------------------------------

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

#Add in the client directories
add_subdirectory( cl_dll )
#add_subdirectory( source_sdk/common )
#add_subdirectory( source_sdk/public )

preprocess_sources()

# appversion.h generator
add_custom_command (
	OUTPUT ${CMAKE_SOURCE_DIR}/cl_dll/appversion.h
	COMMAND sh -c 'CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/cl_dll/appversion.sh'
)

add_library( client SHARED ${PREP_SRCS} ${CMAKE_SOURCE_DIR}/cl_dll/appversion.h )

#Needs static CRT to avoid alloc issues.
configure_msvc_runtime( client STATIC )

check_winxp_support( client )

target_include_directories( client PRIVATE
    cl_dll
	utils/vgui/include_DLLEXPORT
	${SHARED_INCLUDE_PATHS}
	${SHARED_EXTERNAL_INCLUDE_PATHS}
)

target_compile_definitions( client PRIVATE
	${SHARED_DEFS}
	${SHARED_GAME_DEFS}
	CLIENT_DLL
	PCRE_STATIC
	HAVE_CONFIG_H
)

#Find client dependencies
set( vgui_DIR ${GAME_BIN_PATH} CACHE PATH "VGUI1 shared library location" )
find_package( vgui MODULE REQUIRED )

set( SDL2_DIR ${GAME_BIN_PATH} CACHE PATH "SDL2 shared library location" )
find_package( SDL2 MODULE REQUIRED )

#Set the list of platform specific libraries
if( WIN32 )
	set( CLIENT_PLAT_LIBS 
        dbghelp
        psapi
        winmm
        wsock32
    )
else()
	set( CLIENT_PLAT_LIBS "dl" )
endif()

#Link with client dependencies
target_link_libraries( client 
	${DELAYIMP}
	vgui
	${VGUI2_LIBS}
	${CLIENT_PLAT_LIBS}
	SDL2
	${SHARED_DEPENDENCIES}
	Threads::Threads
	${UNIX_FS_LIB}
)

#CMake places libraries in /Debug or /Release on Windows, so explicitly set the paths for both.
#On Linux, it uses LIBRARY_OUTPUT_DIRECTORY
set_target_properties( client PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${GAME_BASE_PATH}/cl_dlls"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${GAME_BASE_PATH}/cl_dlls"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${GAME_BASE_PATH}/cl_dlls"
)

set( CLIENT_LINK_FLAGS )

if( MSVC )
	set( CLIENT_LINK_FLAGS "${CLIENT_LINK_FLAGS} " )
elseif( NOT APPLE )
	#Generate a map file that provides information about the linking stage.
	set( CLIENT_LINK_FLAGS
		${CLIENT_LINK_FLAGS} "-Wl,-Map,client_map.txt"
	)
endif()

set_target_properties( client PROPERTIES
	COMPILE_FLAGS "${WARNING_LEVEL_STRICTEST} -static-libgcc -static-libstdc++"
	LINK_FLAGS "${SHARED_GAME_LINKER_FLAGS} ${CLIENT_LINK_FLAGS} -static-libgcc -static-libstdc++"
)

#No lib prefix
set_target_properties( client PROPERTIES PREFIX "" )

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}" )

#Set the startup project to the client
#TODO: does this even work?
set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT client )

clear_sources()

#
#End client library
#

#--------------------------------------------------------------------------------------------------------
# Server library
#--------------------------------------------------------------------------------------------------------

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

#Add in the server directories
add_subdirectory( dlls )

preprocess_sources()

# appversion.h generator
add_custom_command (
	OUTPUT ${CMAKE_SOURCE_DIR}/dlls/appversion.h
	COMMAND sh -c 'CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/dlls/appversion.sh'
)

add_library( hl SHARED ${PREP_SRCS} ${CMAKE_SOURCE_DIR}/dlls/appversion.h )

#Needs static CRT to avoid alloc issues.
configure_msvc_runtime( hl STATIC )

check_winxp_support( hl )

target_include_directories( hl PRIVATE
	${SHARED_INCLUDE_PATHS}
	${SHARED_EXTERNAL_INCLUDE_PATHS}
)

target_compile_definitions( hl PRIVATE
	${SHARED_DEFS}
	${SHARED_GAME_DEFS}
	SERVER_DLL
	VALVE_DLL
    CLIENT_WEAPONS
)

#Set the list of platform specific libraries
if( WIN32 )
	set( SERVER_PLAT_LIBS Ws2_32 Shlwapi )
else()
	set( SERVER_PLAT_LIBS dl )
endif()

#Link with server dependencies
target_link_libraries( hl 
	${DELAYIMP}
	${SERVER_PLAT_LIBS}
	${SHARED_DEPENDENCIES}
	${UNIX_FS_LIB}
)

#CMake places libraries in /Debug or /Release on Windows, so explicitly set the paths for both.
#On Linux, it uses LIBRARY_OUTPUT_DIRECTORY
set_target_properties( hl PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${GAME_BASE_PATH}/dlls"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${GAME_BASE_PATH}/dlls"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${GAME_BASE_PATH}/dlls"
)

set( SERVER_LINK_FLAGS )

if( MSVC )
	#Copy the def file so it's local to the binary directory
	file( COPY ${CMAKE_SOURCE_DIR}/hl.def DESTINATION ${CMAKE_BINARY_DIR} )

	#Add the module definition file. Needed for GiveFnptrsToDll function export.
	#Add delay loaded libraries.
	set( SERVER_LINK_FLAGS "/DEF:hl.def ${SQL_DLL_DELAYLOAD}" )
else()
	if( NOT APPLE )
		#Generate a map file that provides information about the linking stage.
		set( SERVER_LINK_FLAGS
			${SERVER_LINK_FLAGS} "-Wl,-Map,hl_map.txt "
		)
	endif()
endif()

#SQL libraries are delay loaded to keep them in the game directory.
set_target_properties( hl PROPERTIES
	COMPILE_FLAGS "${WARNING_LEVEL_STRICTEST}" 
	LINK_FLAGS "${SHARED_GAME_LINKER_FLAGS} ${SERVER_LINK_FLAGS}"
)

#No lib prefix
set_target_properties( hl PROPERTIES PREFIX "" )

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}" )

clear_sources()

#
#End server library
#

