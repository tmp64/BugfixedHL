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
include( Ternary )

project( BugfixedHL )

#C++14 support
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set( SHARED_GAME_LINKER_FLAGS
)

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" )
	# Always build as 32 bit
	# Additional debug info for GDB.
	set( SHARED_COMPILER_FLAGS "${SHARED_COMPILER_FLAGS} -m32 -g" )
	
	option( GCC_NO_OPTIMIZE "GCC: Disable optimizations (-O0)" )
	if( GCC_NO_OPTIMIZE )
		set( SHARED_COMPILER_FLAGS "${SHARED_COMPILER_FLAGS} -O0" )
	endif()
	
	option( GCC_CHECK_FOR_UB "GCC: Enable run-time checks for undefined behavior" )
	if( GCC_CHECK_FOR_UB )
		set( SHARED_COMPILER_FLAGS       "${SHARED_COMPILER_FLAGS} -fsanitize=address,undefined -fstack-protector" )
		set( SHARED_GAME_LINKER_FLAGS "${SHARED_GAME_LINKER_FLAGS} -fsanitize=address,undefined -fstack-protector" )
	endif()
endif()

if( MSVC )
	# Enable exception support
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif()

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SHARED_COMPILER_FLAGS}" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SHARED_COMPILER_FLAGS}" )

unset( SHARED_COMPILER_FLAGS )

# VGUI2 option
option( USE_VGUI2 "Whether to include VGUI2 features" )
ternary( USE_VGUI2_DEFINE USE_VGUI2 1 0 )

# Autodeploy option
option( AUTO_DEPLOY "Whether to automatically deploy to deploy paths" 1 )

# Updater option
option( USE_UPDATER "Enable autoupdater module" 0 )
ternary( USE_UPDATER_DEFINE USE_UPDATER 1 0 )
if( USE_UPDATER )
	set( USE_UPDATER_DEFINE "USE_UPDATER=${USE_UPDATER_DEFINE}" )
else()
	set( USE_UPDATER_DEFINE "" )
endif()

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
		_alloca=alloca
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
if( MSVC )
	#Set Windows subsystem
	set( SHARED_GAME_LINKER_FLAGS "${SHARED_GAME_LINKER_FLAGS} /SUBSYSTEM:WINDOWS "
)
elseif( UNIX )
	#From the Github 2013 Makefile, match the settings for Unix environments.
	# force 387 for FP math so the precision between win32 and linux and osx match
	# Trigger an error if any code tries to use an implicit return type
	# Default visibility is hidden unless explicitly altered with __attribute__( visibility() )
	set( SHARED_GAME_LINKER_FLAGS "${SHARED_GAME_LINKER_FLAGS} -mfpmath=387 -Werror=return-type -fvisibility=hidden" )

	if( APPLE )
		set( SHARED_GAME_LINKER_FLAGS "${SHARED_GAME_LINKER_FLAGS} -momit-leaf-frame-pointer -mtune=core2" )
	else()
		#Linux, Cygwin, etc.
	endif()
endif()

# Warning level
if( MSVC )
	set( WARNING_LEVEL " " )
else()
	set( WARNING_LEVEL "-Wno-write-strings -Wno-unknown-pragmas -Wno-endif-labels -Wno-switch -Wno-unused-variable" )
	set( WARNING_LEVEL "${WARNING_LEVEL} -Wno-unused-but-set-variable -Wno-unused-function -Wno-unused-value" )
	set( WARNING_LEVEL "${WARNING_LEVEL} -Wno-pointer-arith" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-delete-non-virtual-dtor -Wno-invalid-offsetof -Wno-conversion-null" )
endif()

#Shared directories
#Include paths for external dependencies. Should always come after all project paths.
set( SHARED_EXTERNAL_INCLUDE_PATHS
)

set( SHARED_INCLUDE_PATHS
	common
	engine
	pm_shared
	game_shared
	dlls
)

add_subdirectory( common )
add_subdirectory( engine )
add_subdirectory( pm_shared )
add_subdirectory( game_shared )

#Save the shares sources into SHARED_SRCS
get_sources( SHARED_SRCS )

clear_sources()

set( THREADS_PREFER_PTHREAD_FLAG ON )
find_package( Threads REQUIRED )

if( UNIX AND NOT APPLE )
	set( UNIX_FS_LIB stdc++fs )
endif()

# Source SDK code
set( SOURCE_SDK_SRC )
set( SOURCE_SDK_DEPENDENCIES )
set( SOURCE_SDK_INCLUDE_PATHS )
set( SOURCE_SDK_DEFINES )

if ( USE_VGUI2 )
	option( VGUI2_BUILD_4554 "Support game build 4554" )

	find_package( tier0 MODULE REQUIRED )
	find_package( vstdlib MODULE REQUIRED )
	
	add_subdirectory( public )
	add_subdirectory( source_sdk )
	
	# Order matters!
	set( SOURCE_SDK_DEPENDENCIES
		vgui2_controls
		tier1
		tier0
		vstdlib
	)
	
	# steam_api.dll in 4554 is too old to use headers from Source SDK 2013
	if( NOT VGUI2_BUILD_4554 )
		find_package( SteamAPI MODULE REQUIRED )
		set( SOURCE_SDK_DEPENDENCIES
			${SOURCE_SDK_DEPENDENCIES}
			SteamAPI
		)
	endif()
	
	set( SOURCE_SDK_INCLUDE_PATHS
		public
		public/math
		source_sdk/public/tier1
		source_sdk/public
		source_sdk/common
	)
	
	set( SOURCE_SDK_DEFINES
		VERSION_SAFE_STEAM_API_INTERFACES
		USE_VGUI2=${USE_VGUI2_DEFINE}
	)
	
	if( VGUI2_BUILD_4554 )
		set( SOURCE_SDK_DEFINES 
		${SOURCE_SDK_DEFINES} 
		VGUI2_BUILD_4554
		NO_STEAM
	)
	endif()
else()
	set( SOURCE_SDK_DEFINES
		NO_STEAM
	)
endif()

set ( BUGFIXEDAPI_SRCS )
add_subdirectory( bugfixedapi )
get_sources( BUGFIXEDAPI_SRCS )
clear_sources()

# Find CURL for updater
set( UPDATER_LIBS )
set( UPDATER_INCLUDE_PATHS )
if( USE_UPDATER )
	find_package( CURL MODULE REQUIRED )
	set( UPDATER_LIBS
		${CURL_LIBRARIES}
	)
	set( UPDATER_INCLUDE_PATHS
		${CURL_INCLUDE_DIRS}
		game_shared/updater
	)
endif()

#--------------------------------------------------------------------------------------------------------
# Client library
#--------------------------------------------------------------------------------------------------------

# Generate appversion.h during configuration before adding any sources
if ( WIN32 )
	execute_process( COMMAND ${CMAKE_SOURCE_DIR}/scripts/GenerateAppVersion.bat "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}\\cl_dll\\appversion.h" )
else()
	execute_process( COMMAND ${CMAKE_SOURCE_DIR}/scripts/GenerateAppVersion.sh "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/cl_dll/appversion.h" )
endif()

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

#Add in the client directories
add_subdirectory( game_shared/client )
add_subdirectory( cl_dll )

if( USE_UPDATER )
	add_subdirectory( game_shared/updater )
endif()

preprocess_sources()

add_library( client SHARED ${PREP_SRCS} ${CMAKE_SOURCE_DIR}/cl_dll/appversion.h )

# appversion.h generator target
# A hack to make PRE_BUILD be actually PRE_BUILD on non-MSVC (non-Windows, generally speaking)
if ( NOT MSVC )
	add_custom_target( client_appversion ${CMAKE_SOURCE_DIR}/scripts/GenerateAppVersion.sh "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/cl_dll/appversion.h" )
	add_dependencies( client client_appversion )
endif()

if ( WIN32 )
	# Generate appversion.h before building the library on Windows
	add_custom_command( TARGET client
		PRE_BUILD
		COMMAND ${CMAKE_SOURCE_DIR}/scripts/GenerateAppVersion.bat "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}\\cl_dll\\appversion.h"
	)
	
    if ( AUTO_DEPLOY )
	    add_custom_command( TARGET client
		    POST_BUILD
		    COMMAND ${CMAKE_SOURCE_DIR}/scripts/DeployLibs.bat "${CMAKE_SOURCE_DIR}\\scripts\\PublishPathClient.txt" "$<TARGET_FILE:client>" "$<TARGET_PDB_FILE:client>"
	    )
    endif()
else()
	if( UNIX AND NOT APPLE )
		# Fix vgui.so path in client.so
		# " The best kostil' is not kostil' " (c) EzicMan
		add_custom_command( TARGET client
			POST_BUILD
			COMMAND ${CMAKE_SOURCE_DIR}/scripts/FixClientLibPath.sh "$<TARGET_FILE:client>"
		)
	endif()
    
    if ( AUTO_DEPLOY )
	    add_custom_command( TARGET client
		    POST_BUILD
		    COMMAND ${CMAKE_SOURCE_DIR}/scripts/DeployLibs.sh "${CMAKE_SOURCE_DIR}/scripts/PublishPathClient.txt" "$<TARGET_FILE:client>"
	    )
    endif()
endif()

#Needs static CRT to avoid alloc issues.
configure_msvc_runtime( client STATIC )

check_winxp_support( client )

target_include_directories( client PRIVATE
	game_shared/client
    cl_dll
	utils/vgui/include_DLLEXPORT
	${SHARED_INCLUDE_PATHS}
	${SHARED_EXTERNAL_INCLUDE_PATHS}
	${SOURCE_SDK_INCLUDE_PATHS}
	${UPDATER_INCLUDE_PATHS}
)

target_compile_definitions( client PRIVATE
	${SHARED_DEFS}
	${SHARED_GAME_DEFS}
	CLIENT_DLL
	PCRE_STATIC
	HAVE_CONFIG_H
	${SOURCE_SDK_DEFINES}
	LODEPNG_NO_COMPILE_CPP
	${USE_UPDATER_DEFINE}
)

#Find client dependencies
find_package( vgui MODULE REQUIRED )
find_package( SDL2 MODULE REQUIRED )

if ( MSVC )
	# DInput for direct mouse input
	find_package( DInput REQUIRED )
	find_package( DXGUID REQUIRED )
endif()

#Set the list of platform specific libraries
if( WIN32 )
	set( CLIENT_PLAT_LIBS 
        dbghelp
        psapi
        winmm
        wsock32
		DINPUT
		DXGUID
    )
else()
	set( CLIENT_PLAT_LIBS 
		dl
		SDL2
	)
endif()

#Link with client dependencies
target_link_libraries( client 
	vgui
	${VGUI2_LIBS}
	${CLIENT_PLAT_LIBS}
	${SHARED_DEPENDENCIES}
	${SOURCE_SDK_DEPENDENCIES}
	Threads::Threads
	${UNIX_FS_LIB}
	${UPDATER_LIBS}
)

set( CLIENT_LINK_FLAGS )
set ( CLIENT_COMPILE_FLAGS )

if( MSVC )
	set( CLIENT_LINK_FLAGS "${CLIENT_LINK_FLAGS} " )
elseif( NOT APPLE )
	set( CLIENT_COMPILE_FLAGS
		${CLIENT_COMPILE_FLAGS} ""
	)

	#Generate a map file that provides information about the linking stage.
	set( CLIENT_LINK_FLAGS
		${CLIENT_LINK_FLAGS} "-Wl,-Map,client_map.txt"
	)
endif()

set_target_properties( client PROPERTIES
	COMPILE_FLAGS "${WARNING_LEVEL} ${CLIENT_COMPILE_FLAGS}"
	LINK_FLAGS "${SHARED_GAME_LINKER_FLAGS} ${CLIENT_LINK_FLAGS}"
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

# Generate appversion.h during configuration before adding any sources
if ( WIN32 )
	execute_process( COMMAND ${CMAKE_SOURCE_DIR}/scripts/GenerateAppVersion.bat "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}\\dlls\\appversion.h" )
else()
	execute_process( COMMAND ${CMAKE_SOURCE_DIR}/scripts/GenerateAppVersion.sh "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/dlls/appversion.h" )
endif()

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

# Add BugfixedHL API sources
add_sources( "${BUGFIXEDAPI_SRCS}" )

#Add in the server directories
add_subdirectory( game_shared/server )
add_subdirectory( dlls )

preprocess_sources()

add_library( hl SHARED ${PREP_SRCS} ${CMAKE_SOURCE_DIR}/dlls/appversion.h )

# appversion.h generator target
# A hack to make PRE_BUILD be actually PRE_BUILD on non-MSVC (non-Windows, generally speaking)
if ( NOT MSVC )
	add_custom_target( hl_appversion ${CMAKE_SOURCE_DIR}/scripts/GenerateAppVersion.sh "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/dlls/appversion.h" )
	add_dependencies( hl hl_appversion )
endif()

if ( WIN32 )
	# Generate appversion.h before building the library on Windows
	add_custom_command( TARGET hl
		PRE_BUILD
		COMMAND ${CMAKE_SOURCE_DIR}/scripts/GenerateAppVersion.bat "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}\\dlls\\appversion.h"
	)
	
    if ( AUTO_DEPLOY )
	    add_custom_command( TARGET hl
		    POST_BUILD
		    COMMAND ${CMAKE_SOURCE_DIR}/scripts/DeployLibs.bat "${CMAKE_SOURCE_DIR}\\scripts\\PublishPathServer.txt" "$<TARGET_FILE:hl>" "$<TARGET_PDB_FILE:hl>"
	    )
    endif()
else()
    if ( AUTO_DEPLOY )
	    add_custom_command( TARGET hl
		    POST_BUILD
		    COMMAND ${CMAKE_SOURCE_DIR}/scripts/DeployLibs.sh "${CMAKE_SOURCE_DIR}/scripts/PublishPathServer.txt" "$<TARGET_FILE:hl>"
	    )
    endif()
endif()

#Needs static CRT to avoid alloc issues.
configure_msvc_runtime( hl STATIC )

check_winxp_support( hl )

target_include_directories( hl PRIVATE
	${SHARED_INCLUDE_PATHS}
	${SHARED_EXTERNAL_INCLUDE_PATHS}
	game_shared/server
	bugfixedapi
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
	set( SERVER_PLAT_LIBS )
else()
	set( SERVER_PLAT_LIBS dl )
endif()

#Link with server dependencies
target_link_libraries( hl 
	${SERVER_PLAT_LIBS}
	${SHARED_DEPENDENCIES}
	${UNIX_FS_LIB}
)

set( SERVER_LINK_FLAGS )

if( MSVC )
	
else()
	if( NOT APPLE )
		#Generate a map file that provides information about the linking stage.
		set( SERVER_LINK_FLAGS
			${SERVER_LINK_FLAGS} "-Wl,-Map,hl_map.txt "
		)
	endif()
endif()

set_target_properties( hl PROPERTIES
	COMPILE_FLAGS "${WARNING_LEVEL}" 
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

#--------------------------------------------------------------------------------------------------------
# BugfixedAPI AMXX module
#--------------------------------------------------------------------------------------------------------

# Generate appversion.h during configuration before adding any sources
if ( WIN32 )
	execute_process( COMMAND ${CMAKE_SOURCE_DIR}/scripts/GenerateAppVersion.bat "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}\\bugfixedapi_amxx\\appversion.h" )
else()
	execute_process( COMMAND ${CMAKE_SOURCE_DIR}/scripts/GenerateAppVersion.sh "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/bugfixedapi_amxx/appversion.h" )
endif()

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

# Add BugfixedHL API sources
add_sources( "${BUGFIXEDAPI_SRCS}" )

add_subdirectory( bugfixedapi_amxx )

preprocess_sources()

add_library( bugfixedapi_amxx SHARED ${PREP_SRCS} )

# appversion.h generator target
# A hack to make PRE_BUILD be actually PRE_BUILD on non-MSVC (non-Windows, generally speaking)
if ( NOT MSVC )
	add_custom_target( bugfixedapi_amxx_appversion ${CMAKE_SOURCE_DIR}/scripts/GenerateAppVersion.sh "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/bugfixedapi_amxx/appversion.h" )
	add_dependencies( bugfixedapi_amxx bugfixedapi_amxx_appversion )
endif()

if ( WIN32 )
	# Generate appversion.h before building the library on Windows
	add_custom_command( TARGET bugfixedapi_amxx
		PRE_BUILD
		COMMAND ${CMAKE_SOURCE_DIR}/scripts/GenerateAppVersion.bat "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}\\bugfixedapi_amxx\\appversion.h"
	)
	
    if ( AUTO_DEPLOY )
	    add_custom_command( TARGET bugfixedapi_amxx
		    POST_BUILD
		    COMMAND ${CMAKE_SOURCE_DIR}/scripts/DeployLibs.bat "${CMAKE_SOURCE_DIR}\\scripts\\PublishPathAMXXModules.txt" "$<TARGET_FILE:bugfixedapi_amxx>" "$<TARGET_PDB_FILE:bugfixedapi_amxx>"
	    )
    endif()
else()
    if ( AUTO_DEPLOY )
	    add_custom_command( TARGET bugfixedapi_amxx
		    POST_BUILD
		    COMMAND ${CMAKE_SOURCE_DIR}/scripts/DeployLibs.sh "${CMAKE_SOURCE_DIR}/scripts/PublishPathAMXXModules.txt" "$<TARGET_FILE:bugfixedapi_amxx>"
	    )
    endif()
endif()

#Needs static CRT to avoid alloc issues.
configure_msvc_runtime( bugfixedapi_amxx STATIC )

check_winxp_support( bugfixedapi_amxx )

target_include_directories( bugfixedapi_amxx PRIVATE
	${SHARED_INCLUDE_PATHS}
	${SHARED_EXTERNAL_INCLUDE_PATHS}
	bugfixedapi_amxx
	bugfixedapi
	metamod-hl1/metamod
)

target_compile_definitions( bugfixedapi_amxx PRIVATE
	${SHARED_DEFS}
	${SHARED_GAME_DEFS}
	USE_METAMOD
	HAVE_STDINT_H
)

#Set the list of platform specific libraries
if( WIN32 )
	set( SERVER_PLAT_LIBS )
else()
	set( SERVER_PLAT_LIBS dl )
endif()

#Link with server dependencies
target_link_libraries( bugfixedapi_amxx 
	${SERVER_PLAT_LIBS}
	${SHARED_DEPENDENCIES}
	${UNIX_FS_LIB}
)

set( SERVER_LINK_FLAGS )

if( MSVC )
	
else()
	if( NOT APPLE )
		#Generate a map file that provides information about the linking stage.
		set( SERVER_LINK_FLAGS
			${SERVER_LINK_FLAGS} "-Wl,-Map,hl_map.txt "
		)
	endif()
endif()

set_target_properties( bugfixedapi_amxx PROPERTIES
	COMPILE_FLAGS "${WARNING_LEVEL}" 
	LINK_FLAGS "${SHARED_GAME_LINKER_FLAGS} ${SERVER_LINK_FLAGS}"
)

#No lib prefix
set_target_properties( bugfixedapi_amxx PROPERTIES PREFIX "" )
if ( NOT MSVC )
	set_target_properties( bugfixedapi_amxx PROPERTIES OUTPUT_NAME "bugfixedapi_amxx_i386" )
endif()

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}" )

clear_sources()
