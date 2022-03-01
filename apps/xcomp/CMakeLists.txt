project(xcomp)

include_directories(${OPENSSL_INCLUDE_DIR})
include_directories( . )
include_directories( src )
include_directories( ../src_common )

set(XCOMPGUIBIN "xcomp")

file( GLOB GUISRCS "src/*.cpp" )
file( GLOB GUIINCS "src/*.h" )

source_group( src_launch FILES ${GUISRCS} ${GUIINCS} )

if (MSVC)
    set( RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../resources )

    set( RESOURCE_PATHFNAME ${RESOURCE_DIR}/xcomp_resources.rc )

    set_source_files_properties( ${RESOURCE_DIR}/xcomp_icon.ico
                                 ${RESOURCE_PATHFNAME}
                                 PROPERTIES LANGUAGE RC)
endif ()

add_executable( ${XCOMPGUIBIN} ${GUISRCS} ${RESOURCE_PATHFNAME} )
target_compile_definitions( ${XCOMPGUIBIN} PRIVATE ENABLE_IMGUI )

if (ENABLE_OPENEXR)
    target_link_libraries( ${XCOMPGUIBIN} PUBLIC
        DSystem DMath DImage DNetwork glew Graphics imgui implot
        commongui ${CPPFS_LIBRARIES} ${OPENSSL_LIBRARIES} OpenEXR )
else()
    target_link_libraries( ${XCOMPGUIBIN} PUBLIC
        DSystem DMath DImage DNetwork glew Graphics imgui implot
        commongui ${CPPFS_LIBRARIES} ${OPENSSL_LIBRARIES} )
endif()

if (ENABLE_OCIO)
    target_link_libraries( ${XCOMPGUIBIN} PUBLIC ${OCIO_LIBRARIES} )
endif()

if (MSVC)
    target_link_libraries(${XCOMPGUIBIN} PRIVATE)
elseif(APPLE)
    target_link_libraries(${XCOMPGUIBIN} PRIVATE -lpthread )
else()
    target_link_libraries(${XCOMPGUIBIN} PRIVATE -lpthread )
endif()

if(BUILD_UNITY)
    set_property( TARGET ${XCOMPGUIBIN} PROPERTY UNITY_BUILD ON )
endif(BUILD_UNITY)

if (ENABLE_PCH)
    target_precompile_headers(${XCOMPGUIBIN} PRIVATE [["stdafx.h"]])
endif()

set_property(TARGET ${XCOMPGUIBIN} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/../debug_dir")

if (ENABLE_OCIO)
    Copy_OCIO_DLLs_to_RuntimeOut()
endif()
