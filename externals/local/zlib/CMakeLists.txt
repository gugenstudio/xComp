project( zlib )

# always optimized, we don't care to debug this
if (MSVC)
    set(CMAKE_C_FLAGS_DEBUG   "/Ox")
    set(CMAKE_CXX_FLAGS_DEBUG "/Ox")
else()
    set(CMAKE_C_FLAGS_DEBUG   "-O2")
    set(CMAKE_CXX_FLAGS_DEBUG "-O2")
endif()

set( SRCS
../../zlib/adler32.c
../../zlib/compress.c
../../zlib/crc32.c
../../zlib/deflate.c
../../zlib/infback.c
../../zlib/inffast.c
../../zlib/inflate.c
../../zlib/inftrees.c
../../zlib/trees.c
../../zlib/uncompr.c
../../zlib/zutil.c
)

set( INCS
../../zlib/crc32.h
../../zlib/deflate.h
../../zlib/inffast.h
../../zlib/inffixed.h
../../zlib/inflate.h
../../zlib/inftrees.h
../../zlib/trees.h
../../zlib/zconf.h
../../zlib/zlib.h
../../zlib/zutil.h
)

include_directories( ../../zlib )

source_group( Sources FILES ${SRCS} ${INCS} )

add_library( ${PROJECT_NAME} STATIC ${SRCS} ${INCS} )
set_property( TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1 )

