#
# Bugfixed and Improved Half-Life Release
#

cmake_minimum_required( VERSION 3.5.2 )

# Source additional modules from the "cmake" directory

#-----------------------------------------------------------------
# CMake modules
#-----------------------------------------------------------------
list( APPEND CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_LIST_DIR}/cmake"
)

include( InputFilesList )
include( MSVCRuntime )
include( WinXPSupport )
include( Ternary )
include( GetGitRevisionDescription )

#-----------------------------------------------------------------
# Version
#-----------------------------------------------------------------
# Also needs to be changed in scripts/BuildRelease.ps1
set( BHL_REPO_VERSION_MAJOR 1 )
set( BHL_REPO_VERSION_MINOR 5 )
set( BHL_REPO_VERSION_PATCH 0 )
set( BHL_REPO_VERSION_TAG "dev" )

if( NOT BHL_VERSION_MAJOR )
	set( BHL_VERSION_MAJOR ${BHL_REPO_VERSION_MAJOR} )
endif()
if( NOT BHL_VERSION_MINOR )
	set( BHL_VERSION_MINOR ${BHL_REPO_VERSION_MINOR} )
endif()
if( NOT BHL_VERSION_PATCH )
	set( BHL_VERSION_PATCH ${BHL_REPO_VERSION_PATCH} )
endif()
if( NOT BHL_VERSION_TAG )
	set( BHL_VERSION_TAG ${BHL_REPO_VERSION_TAG} )
elseif( "${BHL_VERSION_TAG}" STREQUAL "no_tag" )
	set( BHL_VERSION_TAG "" )
endif()

unset( BHL_REPO_VERSION_MAJOR )
unset( BHL_REPO_VERSION_MINOR )
unset( BHL_REPO_VERSION_PATCH )
unset( BHL_REPO_VERSION_TAG )
# Use BHL_VERSION_XXX from now on
# BHL_VERSION_METADATA as well (if set externally)

project( BugfixedHL VERSION "${BHL_VERSION_MAJOR}.${BHL_VERSION_MINOR}.${BHL_VERSION_PATCH}" )

#-----------------------------------------------------------------
# Compiler checks
#-----------------------------------------------------------------
if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" )
	if( WIN32 )
		message( WARNING "You are building with GCC or CLang on Windows. This is not supported." )
	endif()
elseif( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )

else()
	message( WARNING "Unknown compiler deteceted: ${CMAKE_CXX_COMPILER_ID}. This is not supported." )
endif()

if( NOT CMAKE_SIZEOF_VOID_P EQUAL 4 )
	if( NO_SIZEOF_VOID_P_CHECK )
		message( WARNING "You are not building for 32 bit systems. This is not supported." )
	else()
		set( PLAT_ERROR "" )
		if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" )
			set( PLAT_ERROR "Did you forget CMAKE_TOOLCHAIN_FILE?" )
		elseif( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
			set( PLAT_ERROR "Are you generating for Win64?" )
		else()
			set( PLAT_ERROR "Check your compiler options." )
		endif()
		message( FATAL_ERROR "You are not building for 32 bit systems. ${PLAT_ERROR} Use NO_SIZEOF_VOID_P_CHECK to disable this error." )
	endif()
endif()

#-----------------------------------------------------------------
# Compiler settings
#-----------------------------------------------------------------
# C++14 support
set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

set( C_CXX_COMPILER_FLAGS "" )			# Flags for C and C++ compilers
set( SHARED_GAME_LINKER_FLAGS "" )		# Flags for Game DLL linker

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" )
	
	#-----------------------------------------------------------------
	# Linker options
	#-----------------------------------------------------------------
	# From the Github 2013 Makefile, match the settings for Unix environments.
	# - Force 387 for FP math so the precision between win32 and linux and osx match
	# - Trigger an error if any code tries to use an implicit return type
	# - Default visibility is hidden unless explicitly altered with __attribute__( visibility() )
	set( SHARED_GAME_LINKER_FLAGS "${SHARED_GAME_LINKER_FLAGS} -mfpmath=387 -Werror=return-type -fvisibility=hidden" )

	if( APPLE )
		set( SHARED_GAME_LINKER_FLAGS "${SHARED_GAME_LINKER_FLAGS} -momit-leaf-frame-pointer -mtune=core2" )
	else()
		#Linux, Cygwin, etc.
	endif()
	
	#-----------------------------------------------------------------
	# Additional compiler options
	#-----------------------------------------------------------------
	# Undefined behavior sanitizer
	option( GCC_CHECK_FOR_UB "GCC: Enable run-time checks for undefined behavior" )
	if( GCC_CHECK_FOR_UB )
		set( C_CXX_COMPILER_FLAGS       "${C_CXX_COMPILER_FLAGS} -fsanitize=address,undefined -fstack-protector" )
		set( SHARED_GAME_LINKER_FLAGS "${SHARED_GAME_LINKER_FLAGS} -fsanitize=address,undefined -fstack-protector" )
	endif()
	
	# Link-Time Optimizations
	option( GCC_USE_LTO "GCC: Enable Link-Time Optimizations (LTO)" )
	if( GCC_USE_LTO )
		set( C_CXX_COMPILER_FLAGS       "${C_CXX_COMPILER_FLAGS} -flto" )
		set( SHARED_GAME_LINKER_FLAGS "${SHARED_GAME_LINKER_FLAGS} -flto" )
	endif()
	
	# Static link with libstdc++
	option( GCC_STATIC_LIBCPP "GCC: Link statically with libstdc++" )
	if( GCC_STATIC_LIBCPP )
		set( SHARED_GAME_LINKER_FLAGS "${SHARED_GAME_LINKER_FLAGS} -static-libstdc++" )
	endif()
	
elseif( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )

	# Set Windows subsystem
	set( SHARED_GAME_LINKER_FLAGS "${SHARED_GAME_LINKER_FLAGS} /SUBSYSTEM:WINDOWS " )
	
endif()

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_CXX_COMPILER_FLAGS}" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_CXX_COMPILER_FLAGS}" )

unset( C_CXX_COMPILER_FLAGS )

#-----------------------------------------------------------------
# Compiler-specific definitions
#-----------------------------------------------------------------
set( SHARED_DEFS "" )		# Defines used to build all targets

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" )
	# All non-MSVC targets need this. That includes Cygwin and Mingw.
	# Match Source 2013 in defining GNUC and POSIX for UNIX platforms.
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
elseif( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	set( SHARED_DEFS
		${SHARED_DEFS}
		_CRT_SECURE_NO_WARNINGS
		_SCL_SECURE_NO_WARNINGS
	)
endif()

#-----------------------------------------------------------------
# Warning level
#-----------------------------------------------------------------
set( WARNING_LEVEL_C "" )
set( WARNING_LEVEL_CXX "" )
set( WARNING_LEVEL_C_CXX "" )

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" )
	set( WARNING_LEVEL_C_CXX "${WARNING_LEVEL_C_CXX} -Wno-write-strings -Wno-unknown-pragmas -Wno-endif-labels -Wno-switch -Wno-unused-variable" )
	set( WARNING_LEVEL_C_CXX "${WARNING_LEVEL_C_CXX} -Wno-unused-but-set-variable -Wno-unused-function -Wno-unused-value" )
	set( WARNING_LEVEL_C_CXX "${WARNING_LEVEL_C_CXX} -Wno-pointer-arith" )
    set( WARNING_LEVEL_CXX "${WARNING_LEVEL_CXX} -Wno-delete-non-virtual-dtor -Wno-invalid-offsetof -Wno-conversion-null" )
elseif( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	
endif()

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_CXX_COMPILER_FLAGS} ${WARNING_LEVEL_C} ${WARNING_LEVEL_C_CXX}" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_CXX_COMPILER_FLAGS} ${WARNING_LEVEL_CXX} ${WARNING_LEVEL_C_CXX}" )

#-----------------------------------------------------------------
# Build options
#-----------------------------------------------------------------
option( AUTO_DEPLOY		"Whether to automatically deploy to deploy paths" )
option( USE_VGUI2		"Whether to include VGUI2 features" )
option( USE_UPDATER		"Enable update notifier module" )

#-----------------------------------------------------------------
# appversion.h generator
#-----------------------------------------------------------------
find_package( Git )
if( GIT_FOUND )
	# Get branch name
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_BRANCH
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	
	# Get last commit hash (short)
	execute_process(
		COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_COMMIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	
	# Get whether worktree is dirty
	git_local_changes( GIT_DIRTY )
	if( GIT_DIRTY STREQUAL "CLEAN" )
		set( GIT_DIRTY FALSE )
	else()
		set( GIT_DIRTY TRUE )
	endif()
else()
	set( GIT_BRANCH "unknown" )
	set( GIT_COMMIT_HASH "0000000" )
	set( GIT_DIRTY FALSE )
endif()

# Assemble version string
set( BHL_VERSION_STRING "${BHL_VERSION_MAJOR}.${BHL_VERSION_MINOR}.${BHL_VERSION_PATCH}" )
if( BHL_VERSION_TAG )
	set( BHL_VERSION_STRING "${BHL_VERSION_STRING}-${BHL_VERSION_TAG}" )
endif()

if( BHL_FORCE_BRANCH )
	set( BHL_VERSION_STRING "${BHL_VERSION_STRING}+${BHL_FORCE_BRANCH}.${GIT_COMMIT_HASH}" )
else()
	set( BHL_VERSION_STRING "${BHL_VERSION_STRING}+${GIT_BRANCH}.${GIT_COMMIT_HASH}" )
endif()

if( GIT_DIRTY )
	set( BHL_VERSION_STRING "${BHL_VERSION_STRING}.m" )
endif()

# Generate version file
file( MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated" )
configure_file( "common/appversion.h.in" "${CMAKE_BINARY_DIR}/generated/appversion.h" )

#-----------------------------------------------------------------
# Autodeploy macro
#-----------------------------------------------------------------
if( AUTO_DEPLOY )
	macro( add_auto_deploy TARGET_NAME PUBLISH_PATHS_FILE )
		if ( WIN32 )
			add_custom_command( TARGET ${TARGET_NAME}
				POST_BUILD
				COMMAND ${CMAKE_SOURCE_DIR}/scripts/DeployLibs.bat "${CMAKE_SOURCE_DIR}\\scripts\\${PUBLISH_PATHS_FILE}" "$<TARGET_FILE:${TARGET_NAME}>" "$<TARGET_PDB_FILE:${TARGET_NAME}>"
			)
		else()
			add_custom_command( TARGET ${TARGET_NAME}
				POST_BUILD
				COMMAND ${CMAKE_SOURCE_DIR}/scripts/DeployLibs.sh "${CMAKE_SOURCE_DIR}/scripts/${PUBLISH_PATHS_FILE}" "$<TARGET_FILE:${TARGET_NAME}>"
			)
		endif()
	endmacro()
else()
	macro( add_auto_deploy )
	endmacro()
endif()

#-----------------------------------------------------------------
# Common sources
#-----------------------------------------------------------------
set( SHARED_INCLUDE_PATHS
	${CMAKE_BINARY_DIR}/generated
	common
	engine
	public
	pm_shared
	game_shared
	dlls
)

add_subdirectory( common )
add_subdirectory( engine )
add_subdirectory( pm_shared )
add_subdirectory( game_shared )

get_sources( SHARED_SRCS )
clear_sources()

set ( BUGFIXEDAPI_SRCS )
add_subdirectory( bugfixedapi )
get_sources( BUGFIXEDAPI_SRCS )
clear_sources()

set( SHARED_GAME_DEFS
	CLIENT_WEAPONS
)

#-----------------------------------------------------------------
# Libraries
#-----------------------------------------------------------------
set( THREADS_PREFER_PTHREAD_FLAG ON )
find_package( Threads REQUIRED )
add_subdirectory( external/nlohmann_json )
add_subdirectory( external/semver.c )

set( SHARED_DEPENDENCIES
	semver
)

#-----------------------------------------------------------------
# VGUI2
#-----------------------------------------------------------------
set( VGUI2_LIBS "" )
set( VGUI2_INCLUDE_PATHS "" )
set( VGUI2_DEFINES "" )

if( USE_VGUI2 )
	option( VGUI2_BUILD_4554 "Support game build 4554" )
	
	find_package( tier0 MODULE REQUIRED )
	find_package( vstdlib MODULE REQUIRED )
	
	add_subdirectory( public )
	add_subdirectory( source_sdk )
	
	# Order matters (or it won't work on Linux
	set( VGUI2_LIBS
		${VGUI2_LIBS}
		vgui2_controls
		tier1
		tier0
		vstdlib
	)
	
	# steam_api.dll in 4554 is too old to use headers from Source SDK 2013
	if( NOT VGUI2_BUILD_4554 )
		find_package( SteamAPI MODULE REQUIRED )
		set( VGUI2_LIBS
			${VGUI2_LIBS}
			SteamAPI
		)
	endif()
	
	set( VGUI2_INCLUDE_PATHS
		${VGUI2_INCLUDE_PATHS}
		public
		public/math
		source_sdk/public/tier1
		source_sdk/public
		source_sdk/common
	)
	
	set( VGUI2_DEFINES
		${VGUI2_DEFINES} 
		NO_MALLOC_OVERRIDE
		VERSION_SAFE_STEAM_API_INTERFACES
		USE_VGUI2=1
	)
	
	if( VGUI2_BUILD_4554 )
		set( VGUI2_DEFINES 
			${VGUI2_DEFINES} 
			VGUI2_BUILD_4554
			NO_STEAM
		)
	endif()
else()
	set( VGUI2_DEFINES
		${VGUI2_DEFINES} 
		NO_STEAM
	)
endif()

#-----------------------------------------------------------------
# Updater
#-----------------------------------------------------------------
set( UPDATER_LIBS "" )
set( UPDATER_INCLUDE_PATHS "" )
set( UPDATER_DEFINES "" )

if( USE_UPDATER )
	option( CURL_STATICLIB "Enable if cURL was built as a static library" 0 )

	if( WIN32 )
		option( CURL_REPO_LIBS "Use static curl from external/curl on Windows" 1 )
		
		if( CURL_REPO_LIBS )
			set( CURL_STATICLIB 1 CACHE BOOL "" FORCE )
			set( CURL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/curl/include" CACHE PATH "" FORCE )
			set( CURL_LIBRARY "" CACHE FILEPATH "" FORCE )
			set( CURL_LIBRARY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/external/curl/lib/libcurl_static_debug.lib" CACHE FILEPATH "" FORCE )
			set( CURL_LIBRARY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/external/curl/lib/libcurl_static.lib" CACHE FILEPATH "" FORCE )
		endif()
	endif()

	find_package( CURL MODULE REQUIRED )
	set( UPDATER_LIBS
		${CURL_LIBRARIES}
		nlohmann_json
	)
	set( UPDATER_INCLUDE_PATHS
		${CURL_INCLUDE_DIRS}
		game_shared/updater
	)
	
	set( UPDATER_DEFINES
		USE_UPDATER=1
	)
	
	if( CURL_STATICLIB )
		set( UPDATER_DEFINES
			${UPDATER_DEFINES}
			CURL_STATICLIB=1
		)
		if( WIN32 )
			set( UPDATER_LIBS
				${UPDATER_LIBS}
				wldap32.lib
				Crypt32.lib
				Ws2_32.lib
			)
		endif()
	endif()
	
endif()

#-----------------------------------------------------------------
# Client library
#-----------------------------------------------------------------
# Add in the shared sources
add_sources( "${SHARED_SRCS}" )

# Add in the client directories
add_subdirectory( game_shared/client )
add_subdirectory( cl_dll )

if( USE_UPDATER )
	add_subdirectory( game_shared/updater )
endif()

preprocess_sources()
add_library( client SHARED ${PREP_SRCS} )
configure_msvc_runtime( client STATIC )
check_winxp_support( client )

# Fix vgui.so (and others) path in client.so
# See scripts/FixClientLibPath.sh for more details
# Needs to be run before auto deploy
if( UNIX AND NOT APPLE )
	add_custom_command( TARGET client
		POST_BUILD
		COMMAND ${CMAKE_SOURCE_DIR}/scripts/FixClientLibPath.sh "$<TARGET_FILE:client>"
	)
endif()

add_auto_deploy( client PublishPathClient.txt )

target_include_directories( client PRIVATE
	game_shared/client
    cl_dll
	utils/vgui/include_DLLEXPORT
	${SHARED_INCLUDE_PATHS}
	${VGUI2_INCLUDE_PATHS}
	${UPDATER_INCLUDE_PATHS}
	${SHARED_EXTERNAL_INCLUDE_PATHS}
)

target_compile_definitions( client PRIVATE
	${SHARED_DEFS}
	${SHARED_GAME_DEFS}
	CLIENT_DLL
	${VGUI2_DEFINES}
	${UPDATER_DEFINES}
	
	# pcre
	PCRE_STATIC
	HAVE_CONFIG_H
	
	# lodepng
	LODEPNG_NO_COMPILE_CPP
)

# Dependencies
find_package( vgui MODULE REQUIRED )
find_package( SDL2 MODULE REQUIRED )

if( WIN32 )
	# DInput for direct mouse input
	find_package( DInput REQUIRED )
	find_package( DXGUID REQUIRED )
endif()

# Set the list of platform specific libraries
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

# Link with dependencies
target_link_libraries( client 
	vgui
	${VGUI2_LIBS}
	${CLIENT_PLAT_LIBS}
	${SHARED_DEPENDENCIES}
	${SOURCE_SDK_DEPENDENCIES}
	Threads::Threads
	${UPDATER_LIBS}
)

set( CLIENT_LINK_FLAGS "")
set( CLIENT_COMPILE_FLAGS "")

set_target_properties( client PROPERTIES
	COMPILE_FLAGS "${CLIENT_COMPILE_FLAGS}"
	LINK_FLAGS "${SHARED_GAME_LINKER_FLAGS} ${CLIENT_LINK_FLAGS}"
)

# Remove "lib" prefix
set_target_properties( client PROPERTIES PREFIX "" )

create_source_groups( "${CMAKE_SOURCE_DIR}" )
clear_sources()

#-----------------------------------------------------------------
# Server library
#-----------------------------------------------------------------
#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

# Add BugfixedHL API sources
add_sources( "${BUGFIXEDAPI_SRCS}" )

#Add in the server directories
add_subdirectory( game_shared/server )
add_subdirectory( dlls )

preprocess_sources()
add_library( hl SHARED ${PREP_SRCS} )
configure_msvc_runtime( hl STATIC )
check_winxp_support( hl )
add_auto_deploy( hl PublishPathServer.txt )

target_include_directories( hl PRIVATE
	${SHARED_INCLUDE_PATHS}
	game_shared/server
	bugfixedapi
	${SHARED_EXTERNAL_INCLUDE_PATHS}
)

target_compile_definitions( hl PRIVATE
	${SHARED_DEFS}
	${SHARED_GAME_DEFS}
	SERVER_DLL
	VALVE_DLL
)

# Set the list of platform specific libraries
if( WIN32 )
	set( SERVER_PLAT_LIBS )
else()
	set( SERVER_PLAT_LIBS
		dl
	)
endif()

# Link with dependencies
target_link_libraries( hl 
	${SERVER_PLAT_LIBS}
	${SHARED_DEPENDENCIES}
	Threads::Threads
)

set( SERVER_LINK_FLAGS "")
set( SERVER_COMPILE_FLAGS "")

set_target_properties( hl PROPERTIES
	COMPILE_FLAGS "${SERVER_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_GAME_LINKER_FLAGS} ${SERVER_LINK_FLAGS}"
)

# Remove "lib" prefix
set_target_properties( hl PROPERTIES PREFIX "" )
create_source_groups( "${CMAKE_SOURCE_DIR}" )
clear_sources()

#-----------------------------------------------------------------
# BugfixedAPI AMXX module
#-----------------------------------------------------------------
#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

# Add BugfixedHL API sources
add_sources( "${BUGFIXEDAPI_SRCS}" )

add_subdirectory( bugfixedapi_amxx )

preprocess_sources()
add_library( bugfixedapi_amxx SHARED ${PREP_SRCS} )
configure_msvc_runtime( bugfixedapi_amxx STATIC )
check_winxp_support( bugfixedapi_amxx )
add_auto_deploy( bugfixedapi_amxx PublishPathAMXXModules.txt )

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
	set( BUGFIXEDAPI_AMXX_PLAT_LIBS )
else()
	set( BUGFIXEDAPI_AMXX_PLAT_LIBS
		dl
	)
endif()

# Link with dependencies
target_link_libraries( bugfixedapi_amxx 
	${BUGFIXEDAPI_AMXX_PLAT_LIBS}
	${SHARED_DEPENDENCIES}
)

set( BUGFIXEDAPI_AMXX_LINK_FLAGS )
set( BUGFIXEDAPI_AMXX_COMPILE_FLAGS )

set_target_properties( bugfixedapi_amxx PROPERTIES
	COMPILE_FLAGS "${BUGFIXEDAPI_AMXX_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_GAME_LINKER_FLAGS} ${BUGFIXEDAPI_AMXX_LINK_FLAGS}"
)

# Remove "lib" prefix
set_target_properties( bugfixedapi_amxx PROPERTIES PREFIX "" )
if ( NOT MSVC )
	# Add i386 to the file name
	set_target_properties( bugfixedapi_amxx PROPERTIES OUTPUT_NAME "bugfixedapi_amxx_i386" )
endif()

create_source_groups( "${CMAKE_SOURCE_DIR}" )
clear_sources()
