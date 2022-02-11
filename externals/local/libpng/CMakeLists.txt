project( libpng )

add_definitions( -DPNG_ARM_NEON_OPT=0 )

set( SRCS
../../libpng/png.c
../../libpng/pngerror.c
../../libpng/pngget.c
../../libpng/pngmem.c
../../libpng/pngpread.c
../../libpng/pngread.c
../../libpng/pngrio.c
../../libpng/pngrtran.c
../../libpng/pngrutil.c
../../libpng/pngset.c
../../libpng/pngtrans.c
../../libpng/pngwio.c
../../libpng/pngwrite.c
../../libpng/pngwtran.c
../../libpng/pngwutil.c
)

set( INCS
pnglibconf.h
../../libpng/png.h
../../libpng/pngconf.h
../../libpng/pngpriv.h
../../libpng/pngdebug.h
../../libpng/pnginfo.h
../../libpng/pngstruct.h
)

include_directories( ../../zlib )
include_directories( ../../libpng )

source_group( Sources FILES ${SRCS} ${INCS} )

add_library( ${PROJECT_NAME} STATIC ${SRCS} ${INCS} )
set_property( TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1 )

