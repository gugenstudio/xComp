project( imgui )

file( GLOB SRCS "../../imgui/*.cpp"
                "../../imgui/*.h" )

include_directories( "../../imgui/backends" )

file( GLOB SRCS_IMPL "../../imgui/backends/*_opengl3.*"
                     "../../imgui/backends/*_opengl2.*"
                     "../../imgui/backends/*_glfw.*" )

file( GLOB SRCS_MISC "../../imgui/misc/cpp/*.cpp"
                     "../../imgui/misc/cpp/*.h" )

source_group( Lib  FILES ${SRCS} )
source_group( Impl FILES ${SRCS_IMPL} )
source_group( Misc FILES ${SRCS_MISC} )

add_library( ${PROJECT_NAME} STATIC ${SRCS}
                                    ${SRCS_IMPL}
                                    ${SRCS_MISC} )

