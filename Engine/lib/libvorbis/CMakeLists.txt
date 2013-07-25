#
# cMake configuration file for generating Vorbis library file for Torque3D engine
#

SET( VORBIS_SOURCES
    lib/analysis.c
    lib/bitrate.c
    lib/block.c
    lib/codebook.c
    lib/envelope.c
    lib/floor0.c
    lib/floor1.c
    lib/info.c
    lib/lookup.c
    lib/lpc.c
    lib/lsp.c
    lib/mapping0.c
    lib/mdct.c
    lib/psy.c
    lib/registry.c
    lib/res0.c
    lib/sharedbook.c
    lib/smallft.c
    lib/synthesis.c
    lib/vorbisenc.c
    lib/vorbisfile.c
    lib/window.c
)

SET( VORBIS_HEADERS
    include/vorbis/codec.h
    include/vorbis/vorbisenc.h
    include/vorbis/vorbisfile.h
    lib/backends.h
    lib/bitrate.h
    lib/codebook.h
    lib/codec_internal.h
    lib/envelope.h
    lib/highlevel.h
    lib/lookup.h
    lib/lookup_data.h
    lib/lpc.h
    lib/lsp.h
    lib/masking.h
    lib/mdct.h
    lib/misc.h
    lib/os.h
    lib/psy.h
    lib/registry.h
    lib/scales.h
    lib/smallft.h
    lib/window.h
    lib/books/coupled/res_books_stereo.h
    lib/books/floor/floor_books.h
    lib/books/uncoupled/res_books_uncoupled.h
    lib/modes/floor_all.h
    lib/modes/psych_11.h
    lib/modes/psych_16.h
    lib/modes/psych_44.h
    lib/modes/psych_8.h
    lib/modes/residue_16.h
    lib/modes/residue_44.h
    lib/modes/residue_44u.h
    lib/modes/residue_8.h
    lib/modes/setup_11.h
    lib/modes/setup_16.h
    lib/modes/setup_22.h
    lib/modes/setup_32.h
    lib/modes/setup_44.h
    lib/modes/setup_44u.h
    lib/modes/setup_8.h
    lib/modes/setup_X.h
)

SET( LIB_DIR ${CMAKE_SOURCE_DIR}/Engine/lib )
ADD_DEFINITIONS( -DUNICODE -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS )
INCLUDE_DIRECTORIES( ${LIB_DIR}/libvorbis/include ${LIB_DIR}/libogg/include )
# Dushan - make this friendly name as in VS configuration provided before by projectGenerator
ADD_LIBRARY(libvorbis ${VORBIS_HEADERS} ${VORBIS_SOURCES})

# Dushan - NOTE to myself 
#        - test this on MacOS and FreeBSD
#        - Why we need this anyway on UNIX systems?
#        - We can simply use FindVorbis.cmake file
IF( UNIX )
    SET_TARGET_PROPERTIES( libvorbis PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC" )
ENDIF( UNIX )
