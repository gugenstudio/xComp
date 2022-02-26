cmake_minimum_required(VERSION 3.7)

#==================================================================
# add new build types
message("* Adding build types...")
SET(CMAKE_CXX_FLAGS_DEBUGOPT
    "${CMAKE_CXX_FLAGS_DEBUG}"
    CACHE STRING "Flags used by the C++ compiler during DebugOpt builds."
    FORCE )
SET(CMAKE_C_FLAGS_DEBUGOPT
    "${CMAKE_C_FLAGS_DEBUG}"
    CACHE STRING "Flags used by the C compiler during DebugOpt builds."
    FORCE )
SET(CMAKE_EXE_LINKER_FLAGS_DEBUGOPT
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG}"
    CACHE STRING "Flags used for linking binaries during DebugOpt builds."
    FORCE )
SET(CMAKE_SHARED_LINKER_FLAGS_DEBUGOPT
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}"
    CACHE STRING "Flags used by the shared libraries linker during DebugOpt builds."
    FORCE )
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_DEBUGOPT
    CMAKE_C_FLAGS_DEBUGOPT
    CMAKE_EXE_LINKER_FLAGS_DEBUGOPT
    CMAKE_SHARED_LINKER_FLAGS_DEBUGOPT )

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release DebugOpt)
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
    "Reset the configurations to what we need"
    FORCE)
endif()

# specify which are the "debug" configs (for the linker)
set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS "Debug;DebugOpt")

# master project name
project (gugentools)

# specify linkage to static or shared libraries
set(LIBS_STATIC_LINK FALSE)

#
set(ENABLE_IMGUITEXINSPECT TRUE)

# other externals
set(ENABLE_OPENEXR TRUE)
#set(BUILD_TESTING FALSE)
set(OPENEXR_BUILD_TOOLS      OFF)
set(OPENEXR_BUILD_TESTS      OFF)
set(OPENEXR_BUILD_UTILS      OFF)
set(OPENEXR_BUILD_EXAMPLES   OFF)
set(OPENEXR_INSTALL_EXAMPLES OFF)
set(OPENEXR_INSTALL_DOCS     OFF)

#
set(ENABLE_OCIO TRUE)

# specify to use Unity Builds
set(BUILD_UNITY FALSE)

message("Disabled Precompiled Headers")
set(ENABLE_PCH OFF)

# specify the destination for the build products
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../../_bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../../_bin )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CPPFS_LIBRARIES "stdc++fs")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_C_FLAGS   "${CMAKE_CXX_FLAGS} -fPIC")
endif()

# additional compilation flags
if (MSVC)
    set(CMAKE_C_FLAGS_DEBUG       "${CMAKE_C_FLAGS_DEBUG}       /DDEBUG /D_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG}     /DDEBUG /D_DEBUG")
    set(CMAKE_C_FLAGS_DEBUGOPT    "${CMAKE_C_FLAGS_DEBUGOPT}    /DDEBUG /D_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUGOPT  "${CMAKE_CXX_FLAGS_DEBUGOPT}  /DDEBUG /D_DEBUG")
else()
    set(CMAKE_C_FLAGS_DEBUG       "${CMAKE_C_FLAGS_DEBUG}       -DDEBUG -D_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG}     -DDEBUG -D_DEBUG")
    set(CMAKE_C_FLAGS_DEBUGOPT    "${CMAKE_C_FLAGS_DEBUGOPT}    -DDEBUG -D_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUGOPT  "${CMAKE_CXX_FLAGS_DEBUGOPT}  -DDEBUG -D_DEBUG")
endif()

if (MSVC)
    # disable annoying secure CRT warnings
    add_definitions( /D_CRT_SECURE_NO_WARNINGS /wd4996 /nologo )
    #add_compile_options(/W4)
    # suppress class 'type1' needs to have dll-interface to be used by clients of class 'type2'
    add_compile_options(/wd4251)
    add_compile_options(/w34263 /w34266) # enable C4263 and 4266
    add_compile_options(/wd5054)
    add_compile_options(/wd4324) # structure was padded due to alignment specifier
endif()

if (MSVC)
    set(CMAKE_C_FLAGS_RELEASE     "${CMAKE_C_FLAGS_RELEASE}     /Ox /Ot /Oi /Ob2 /Oy /GS-")
    set(CMAKE_CXX_FLAGS_RELEASE   "${CMAKE_CXX_FLAGS_RELEASE}   /Ox /Ot /Oi /Ob2 /Oy /GS-")
    set(CMAKE_C_FLAGS_DEBUG       "${CMAKE_C_FLAGS_DEBUG}       /Od /Zi")
    set(CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG}     /Od /Zi")
    set(CMAKE_C_FLAGS_DEBUGOPT    "${CMAKE_C_FLAGS_DEBUGOPT}    /O2 /Zi /Ot /Oi /Ob2")
    set(CMAKE_CXX_FLAGS_DEBUGOPT  "${CMAKE_CXX_FLAGS_DEBUGOPT}  /O2 /Zi /Ot /Oi /Ob2")

    # reduce burden on iterators debugging
    set(CMAKE_C_FLAGS_DEBUG       "${CMAKE_C_FLAGS_DEBUG}       /D_ITERATOR_DEBUG_LEVEL=1")
    set(CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG}     /D_ITERATOR_DEBUG_LEVEL=1")
    set(CMAKE_C_FLAGS_DEBUGOPT    "${CMAKE_C_FLAGS_DEBUGOPT}    /D_ITERATOR_DEBUG_LEVEL=1")
    set(CMAKE_CXX_FLAGS_DEBUGOPT  "${CMAKE_CXX_FLAGS_DEBUGOPT}  /D_ITERATOR_DEBUG_LEVEL=1")

    string( REPLACE "/RTC1" ""  CMAKE_C_FLAGS_DEBUGOPT    ${CMAKE_C_FLAGS_DEBUGOPT} )
    string( REPLACE "/RTC1" ""  CMAKE_CXX_FLAGS_DEBUGOPT  ${CMAKE_CXX_FLAGS_DEBUGOPT} )

    add_definitions( "/fp:fast" )            # common params for FPU
    add_definitions( "/DWIN32" )             # some common params
    add_definitions( "/DNOMINMAX" )          # avoid crazy old Windows.h min and max macros
    add_definitions( "/MP" )                 # multi-process compilation
    add_definitions( "/D_WIN32_WINNT=0x0601" )
endif()

# opengl
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL)

if ((NOT OpenGL_FOUND))
	message( FATAL_ERROR "OpenGL was not found and BUILD_GUI is enabled. Exiting..." )
endif()

if (OpenGL_FOUND)
    # glew
    include_directories( externals/glew_base/include )
    add_subdirectory( externals/glew_base )
    add_definitions( -DGLEW_STATIC )

    # glfw
    option(GLFW_BUILD_EXAMPLES OFF)
    option(GLFW_BUILD_TESTS OFF)
    option(GLFW_BUILD_DOCS OFF)
    option(GLFW_INSTALL OFF)
    include_directories( externals/glfw/include )
    include_directories( externals/glfw/deps )
    add_subdirectory( externals/glfw )
endif()

# disable support in DImage
add_definitions( -DD_NOJPEG )

if (ENABLE_OPENEXR)
    add_definitions( -DENABLE_OPENEXR )
endif()

if (ENABLE_OCIO)
    add_definitions( -DENABLE_OCIO )
endif()

# include directories
include_directories( externals/rapidjson/include )
include_directories( externals/json/single_include )
include_directories( externals/zlib )
include_directories( externals/bzip2 )
include_directories( externals/libpng )
include_directories( externals/local/libpng )
if (ENABLE_OPENEXR)
    include_directories( externals/Imath/src/Imath             )
    include_directories( externals/openexr/src/lib/Iex         )
    include_directories( externals/openexr/src/lib/IlmThread   )
    include_directories( externals/openexr/src/lib/OpenEXR     )
    include_directories( externals/openexr/src/lib/OpenEXRCore )
    include_directories( externals/openexr/src/lib/OpenEXRUtil )
endif()
if (ENABLE_OCIO)
    # TODO
endif()
include_directories( externals/imgui )
include_directories( externals/imgui/backends )
include_directories( externals/imgui/misc/cpp )
include_directories( externals/implot )
if (ENABLE_IMGUITEXINSPECT)
    include_directories( externals/imgui_tex_inspect )
    include_directories( externals/imgui_tex_inspect/backends )
endif()

include_directories( libs/DSystem/src )
include_directories( libs/DMath/src )
include_directories( libs/DImage/src )
include_directories( libs/DNetwork/src )
include_directories( libs/Graphics/src )

# add a sub-projects
add_subdirectory( externals/local/zlib )
add_subdirectory( externals/local/bzip2 )
add_subdirectory( externals/local/libpng )
if (ENABLE_OPENEXR)
    SET(ZLIB_INCLUDE_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/externals/zlib")

    SET(ZLIB_LIBRARY_DIR     "${CMAKE_BINARY_DIR}/externals/local/zlib")

    if (MSVC)
        SET(ZLIB_LIBRARY_RELEASE "${ZLIB_LIBRARY_DIR}/Release/zlib.lib")
        SET(ZLIB_LIBRARY_DEBUG   "${ZLIB_LIBRARY_DIR}/Debug/zlib_d.lib")
    else()
        SET(ZLIB_LIBRARY         "${ZLIB_LIBRARY_DIR}/libzlib.a")
    endif()

    add_subdirectory( externals/Imath )
	find_package(Imath CONFIG)
    include_directories( ${CMAKE_BINARY_DIR}/externals/Imath/config )

    add_subdirectory( externals/openexr )
    find_package(OpenEXR CONFIG)
    include_directories( ${CMAKE_BINARY_DIR}/externals/openexr/cmake )
endif()

if (ENABLE_OCIO)
    set(OCIO_BUILD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/externals/local/ocio/_build)

    if (WIN32)
        set(OCIO_LIBRARY_DIRS
            ${OCIO_BUILD_ROOT}/src/OpenColorIO/Release
            ${OCIO_BUILD_ROOT}/src/OpenColorIO/Debug)

        set(OCIO_LIBRARIES
            optimized   ${OCIO_BUILD_ROOT}/src/OpenColorIO/Release/OpenColorIO.lib
            debug       ${OCIO_BUILD_ROOT}/src/OpenColorIO/Debug/OpenColorIO.lib)

    elseif (UNIX)
    endif()

    include_directories( externals/local/ocio/_build/include )
    include_directories( externals/ocio/include )
endif()

add_subdirectory( externals/local/imgui )
add_subdirectory( externals/local/implot )
if (ENABLE_IMGUITEXINSPECT)
    add_subdirectory( externals/local/imgui_tex_inspect )
endif()

add_subdirectory( libs/DSystem )
add_subdirectory( libs/DMath )
add_subdirectory( libs/DImage )
add_subdirectory( libs/DNetwork )
add_subdirectory( libs/Graphics )

if (ENABLE_IMGUITEXINSPECT)
    add_definitions( -DENABLE_IMGUITEXINSPECT )
endif()
add_subdirectory( apps )

