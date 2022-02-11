project(common)

include_directories(${CPPREST_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories( . )
include_directories( ../src_common )

set(COMMONGUI "commongui")

file( GLOB COMMONGUISRCS "../src_common/*.cpp" )
file( GLOB COMMONGUIINCS "../src_common/*.h" )

source_group( src_common FILES ${COMMONGUISRCS} ${COMMONGUISRCS} )

add_library(commonguiobj OBJECT ${COMMONGUISRCS} ${COMMONGUISRCS})
set_property(TARGET commonguiobj PROPERTY POSITION_INDEPENDENT_CODE 1)
target_compile_definitions( commonguiobj PRIVATE ENABLE_IMGUI )

if(BUILD_UNITY)
    set_property( TARGET commonguiobj PROPERTY UNITY_BUILD ON )
endif(BUILD_UNITY)

add_library( ${COMMONGUI} STATIC $<TARGET_OBJECTS:commonguiobj> )
target_compile_definitions( ${COMMONGUI} PRIVATE ENABLE_IMGUI )

# link the libs
target_link_libraries( ${COMMONGUI} DImage DNetwork DSystem imgui implot Graphicsgui ${CPPFS_LIBRARIES} ${OPENSSL_LIBRARIES} )

if (ENABLE_PCH)
    target_precompile_headers(${COMMONGUI} PRIVATE [["stdafx.h"]])
endif()

