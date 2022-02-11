project( imgui_tex_inspect )

add_definitions( -DIMGUI_IMPL_OPENGL_LOADER_GLEW )

file( GLOB SRCS "../../imgui_tex_inspect/imgui_tex_inspect.cpp"
                "../../imgui_tex_inspect/imgui_tex_inspect.h"
                "../../imgui_tex_inspect/imgui_tex_inspect_internal.h"
                "../../imgui_tex_inspect/backends/tex_inspect_opengl.cpp"
                "../../imgui_tex_inspect/backends/tex_inspect_opengl.h"
                )

source_group( Lib  FILES ${SRCS} )

add_library( ${PROJECT_NAME} STATIC ${SRCS} )

