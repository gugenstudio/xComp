project( DImage )

file( GLOB SRCS "src/*.cpp" )
file( GLOB INCS "src/*.h" )

include_directories( . )
include_directories( src )

source_group( Sources FILES ${SRCS} ${INCS} )

add_definitions( -DGLEW_STATIC )

add_library( ${PROJECT_NAME} STATIC ${SRCS} )
set_property( TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1 )
target_link_libraries(${PROJECT_NAME} DMath)

if(BUILD_UNITY)
    set_property( TARGET ${PROJECT_NAME} PROPERTY UNITY_BUILD ON )
endif(BUILD_UNITY)

if (ENABLE_PCH)
    target_precompile_headers(${PROJECT_NAME} PRIVATE [["stdafx.h"]])
endif(ENABLE_PCH)
