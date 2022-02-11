project( DMath )

file( GLOB_RECURSE SRCS "src/*.cpp" )
file( GLOB_RECURSE INCS "src/*.h" )

include_directories( src )

source_group( Sources FILES ${SRCS} ${INCS} )

add_library( ${PROJECT_NAME} STATIC ${SRCS} )
set_property( TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1 )

target_link_libraries( ${PROJECT_NAME} DSystem )

if(BUILD_UNITY)
    set_property( TARGET ${PROJECT_NAME} PROPERTY UNITY_BUILD ON )
endif(BUILD_UNITY)
