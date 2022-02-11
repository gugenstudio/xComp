project(Graphics)

include_directories( . )

file( GLOB SRCS "src/*.cpp" )
file( GLOB INCS "src/*.h" )

set(GRAPHICSGUI "Graphicsgui")

source_group( src FILES ${SRCS} ${INCS} )

add_library( ${GRAPHICSGUI} STATIC ${SRCS} )
set_property( TARGET ${GRAPHICSGUI} PROPERTY POSITION_INDEPENDENT_CODE 1 )

target_link_libraries( ${GRAPHICSGUI} PUBLIC DSystem DMath DImage libpng ${OPENGL_LIBRARIES} glew glfw imgui implot )
target_compile_definitions( ${GRAPHICSGUI} PRIVATE ENABLE_OPENGL ENABLE_IMGUI )
if (ENABLE_IMGUITEXINSPECT)
    target_link_libraries( ${GRAPHICSGUI} PUBLIC imgui_tex_inspect )
    target_compile_definitions( ${GRAPHICSGUI} PRIVATE ENABLE_IMGUITEXINSPECT )
endif()

if(BUILD_UNITY)
    set_property( TARGET ${GRAPHICSGUI}   PROPERTY UNITY_BUILD ON )
endif(BUILD_UNITY)
