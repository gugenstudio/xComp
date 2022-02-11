project(DNetwork)

include_directories(${OPENSSL_INCLUDE_DIR})

include_directories( DSystem/src )

include_directories( . )
file( GLOB SRCS "src/*.cpp" )
file( GLOB INCS "src/*.h" )

source_group( src FILES ${SRCS} ${INCS} )

add_library( ${PROJECT_NAME} STATIC ${SRCS} )
set_property( TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1 )

target_link_libraries( ${PROJECT_NAME} DSystem )

if(BUILD_UNITY)
    set_property( TARGET ${PROJECT_NAME} PROPERTY UNITY_BUILD ON )
endif(BUILD_UNITY)

