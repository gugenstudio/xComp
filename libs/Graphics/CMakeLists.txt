project(Graphics)

include_directories( . )

file( GLOB SRCS "src/*.cpp" )
file( GLOB INCS "src/*.h" )

set(GRAPHICS "Graphics")

source_group( src FILES ${SRCS} ${INCS} )

add_library( ${GRAPHICS} STATIC ${SRCS} )
set_property( TARGET ${GRAPHICS} PROPERTY POSITION_INDEPENDENT_CODE 1 )

target_link_libraries( ${GRAPHICS} PUBLIC DSystem DMath DImage libpng ${OPENGL_LIBRARIES} glew glfw imgui implot )
target_compile_definitions( ${GRAPHICS} PRIVATE ENABLE_OPENGL ENABLE_IMGUI )
if (ENABLE_IMGUITEXINSPECT)
    target_link_libraries( ${GRAPHICS} PUBLIC imgui_tex_inspect )
    target_compile_definitions( ${GRAPHICS} PRIVATE ENABLE_IMGUITEXINSPECT )
endif()

if(BUILD_UNITY)
    set_property( TARGET ${GRAPHICS}   PROPERTY UNITY_BUILD ON )
endif(BUILD_UNITY)
