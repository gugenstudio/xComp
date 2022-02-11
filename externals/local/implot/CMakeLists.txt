project( implot )

file( GLOB SRCS "../../implot/implot.cpp"
                "../../implot/implot.h"
                "../../implot/implot_internal.h"
                "../../implot/implot_items.cpp"
                )

source_group( Lib  FILES ${SRCS} )

add_library( ${PROJECT_NAME} STATIC ${SRCS} )

