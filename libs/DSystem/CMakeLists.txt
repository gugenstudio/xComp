project(DSystem)

include_directories( . )
file( GLOB SRCS "src/*.cpp" )
file( GLOB INCS "src/*.h" )

source_group( src FILES ${SRCS} ${INCS} )

add_library( ${PROJECT_NAME} STATIC ${SRCS} ${INCS} )
set_property( TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1 )

if (MSVC OR APPLE)
else()
    target_link_libraries(${PROJECT_NAME} stdc++fs)
endif()

target_link_libraries(${PROJECT_NAME} zlib libpng bzip2)

if(BUILD_UNITY)
    set_property( TARGET ${PROJECT_NAME} PROPERTY UNITY_BUILD ON )
endif(BUILD_UNITY)
