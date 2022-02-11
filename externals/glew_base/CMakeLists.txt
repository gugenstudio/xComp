project( glew )

find_package(OpenGL)

set( SRCS src/glew.c )

if (WIN32)

	set( INCS_PLATFORM include/GL/wglew.h)

endif (WIN32)

set( INCS include/GL/glew.h)

include_directories( include )

source_group( Sources FILES ${SRCS} ${INCS} ${INCS_PLATFORM} )

add_definitions( -DGLEW_STATIC )

add_library( ${PROJECT_NAME} STATIC ${SRCS} ${INCS} ${INCS_PLATFORM} )
set_property( TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1 )

