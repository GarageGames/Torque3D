#
# cMake configuration file for generating MNG library file for Torque3D engine
#

SET( LIBMNG_SOURCES
    libmng_callback_xs.c
    libmng_chunk_descr.c
    libmng_chunk_io.c
    libmng_chunk_prc.c
    libmng_chunk_xs.c
    libmng_cms.c
    libmng_display.c
    libmng_dither.c
    libmng_error.c
    libmng_filter.c
    libmng_hlapi.c
    libmng_jpeg.c
    libmng_object_prc.c
    libmng_pixels.c
    libmng_prop_xs.c
    libmng_read.c
    libmng_trace.c
    libmng_write.c
    libmng_zlib.c
)

SET( LIBMNG_HEADERS
    libmng.h
    libmng_chunks.h
    libmng_chunk_descr.h
    libmng_chunk_io.h
    libmng_chunk_prc.h
    libmng_cms.h
    libmng_conf.h
    libmng_data.h
    libmng_display.h
    libmng_dither.h
    libmng_error.h
    libmng_filter.h
    libmng_jpeg.h
    libmng_memory.h
    libmng_objects.h
    libmng_object_prc.h
    libmng_pixels.h
    libmng_read.h
    libmng_trace.h
    libmng_types.h
    libmng_write.h
    libmng_zlib.h
)

SET( LIB_DIR ${CMAKE_SOURCE_DIR}/Engine/lib )
ADD_DEFINITIONS( -DMNG_OPTIMIZE_OBJCLEANUP -DUNICODE -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS )
INCLUDE_DIRECTORIES( ${LIB_DIR}/lmng ${LIB_DIR}/lpng ${LIB_DIR}/zlib ${LIB_DIR}/ljpeg )
# Dushan - make this friendly name as in VS configuration provided before by projectGenerator
ADD_LIBRARY( lmng ${LIBMNG_HEADERS} ${LIBMNG_SOURCES} )

# Dushan - NOTE to myself 
#        - test this on MacOS and FreeBSD
IF( UNIX )
    SET_TARGET_PROPERTIES( lmng PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC" )
ENDIF( UNIX )
