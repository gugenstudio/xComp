project( bzip2 )

# always optimized, we don't care to debug this
if (MSVC)
    set(CMAKE_C_FLAGS_DEBUG   "/Ox")
    set(CMAKE_CXX_FLAGS_DEBUG "/Ox")
else()
    set(CMAKE_C_FLAGS_DEBUG   "-O2")
    set(CMAKE_CXX_FLAGS_DEBUG "-O2")
endif()

set( SRCS
../../bzip2/blocksort.c
../../bzip2/bzlib.c
../../bzip2/compress.c
../../bzip2/crctable.c
../../bzip2/decompress.c
../../bzip2/huffman.c
../../bzip2/randtable.c
)

set( INCS
../../bzip2/bzlib.h
../../bzip2/bzlib_private.h
)

include_directories( ../../bzip2 )

source_group( Sources FILES ${SRCS} ${INCS} )

add_library( ${PROJECT_NAME} STATIC ${SRCS} ${INCS} )
set_property( TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1 )

