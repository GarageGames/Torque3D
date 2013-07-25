#
# cMake configuration file for generating Theora library file for Torque3D engine
#

cmake_minimum_required(VERSION 2.8)

# Determine arch.
IF( CMAKE_SYSTEM_NAME STREQUAL "Darwin" )
    # CMAKE_SYSTEM_PROCESSOR always returns i386 for x86 and x86_64 on Mac
    IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
        SET( BUILD_ARCH "x86_64" )
    ELSEIF( CMAKE_SIZEOF_VOID_P EQUAL 4 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "i386" )
        SET( BUILD_ARCH "i386" )
    ELSEIF( CMAKE_SIZEOF_VOID_P EQUAL 4 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "ppc" )
        SET( BUILD_ARCH "ppc" )
    ELSE()
        SET( BUILD_ARCH "Unknown" )
    ENDIF()
ELSEIF( WIN32 )
    # CMAKE_SYSTEM_PROCESSOR is not reliable under windows
    IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
        SET( BUILD_ARCH "x86_64" )
    ELSEIF( CMAKE_SIZEOF_VOID_P EQUAL 4 )
        SET( BUILD_ARCH "x32" )
    ENDIF()
ELSE()
    # CMAKE_SYSTEM_PROCESSOR is reliable for *nix
    IF( CMAKE_SYSTEM_PROCESSOR MATCHES "i.86" )
        SET( BUILD_ARCH "i386" )
    ELSEIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET( BUILD_ARCH "x86_64" )
    ELSEIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND CMAKE_SIZEOF_VOID_P EQUAL 4 )
        SET( BUILD_ARCH "x32" )
    ELSE()
        SET( BUILD_ARCH ${CMAKE_SYSTEM_PROCESSOR} )
    ENDIF()
ENDIF()

SET( THEORA_SOURCES
    lib/cpu.c
    lib/dec/apiwrapper.c
    lib/dec/bitpack.c
    lib/dec/decapiwrapper.c
    lib/dec/decinfo.c
    lib/dec/decode.c
    lib/dec/dequant.c
    lib/dec/fragment.c
    lib/dec/huffdec.c
    lib/dec/idct.c
    lib/dec/info.c
    lib/dec/internal.c
    lib/dec/quant.c
    lib/dec/state.c
    lib/dec/x86_vc/mmxfrag.c
    lib/dec/x86_vc/mmxidct.c
    lib/dec/x86_vc/mmxloopfilter.c
    lib/dec/x86_vc/mmxstate.c
    lib/dec/x86_vc/x86state.c
    lib/enc/blockmap.c
    lib/enc/dct.c
    lib/enc/dct_decode.c
    lib/enc/dct_encode.c
    lib/enc/dsp.c
    lib/enc/encapiwrapper.c
    lib/enc/encode.c
    lib/enc/encoder_huffman.c
    lib/enc/encoder_idct.c
    lib/enc/encoder_quant.c
    lib/enc/encoder_toplevel.c
    lib/enc/frarray.c
    lib/enc/frinit.c
    lib/enc/mcomp.c
    lib/enc/misc_common.c
    lib/enc/pb.c
    lib/enc/pp.c
    lib/enc/reconstruct.c
    lib/enc/scan.c
)

IF( BUILD_ARCH STREQUAL "x32" )
    set( THEORA_SOURCES ${THEORA_SOURCES} 
	lib/enc/x86_32_vs/dsp_mmx.c
    lib/enc/x86_32_vs/fdct_mmx.c
    lib/enc/x86_32_vs/recon_mmx.c )
ELSEIF ( BUILD_ARCH STREQUAL "x86_64" )
    set( THEORA_SOURCES ${THEORA_SOURCES} 
    lib/enc/x86_64/dsp_mmx.c
    lib/enc/x86_64/fdct_mmx.c
    lib/enc/x86_64/recon_mmx.c )
ELSEIF ( BUILD_ARCH STREQUAL "i386" )
    set( THEORA_SOURCES ${THEORA_SOURCES} 
    lib/enc/x86_32/dsp_mmx.c
    lib/enc/x86_32/fdct_mmx.c
    lib/enc/x86_32/recon_mmx.c )
ENDIF()

SET( THEORA_HEADERS
    include/theora/codec.h
    include/theora/theora.h
    include/theora/theoradec.h
    include/theora/theoraenc.h
    lib/cpu.h
    lib/internal.h
    lib/dec/apiwrapper.h
    lib/dec/bitpack.h
    lib/dec/dct.h
    lib/dec/decint.h
    lib/dec/dequant.h
    lib/dec/huffdec.h
    lib/dec/huffman.h
    lib/dec/idct.h
    lib/dec/ocintrin.h
    lib/dec/quant.h
    lib/dec/x86_vc/x86int.h
    lib/enc/block_inline.h
    lib/enc/codec_internal.h
    lib/enc/dsp.h
    lib/enc/encoder_huffman.h
    lib/enc/encoder_lookup.h
    lib/enc/hufftables.h
    lib/enc/pp.h
    lib/enc/quant_lookup.h
    lib/enc/toplevel_lookup.h
)

SET( LIB_DIR ${CMAKE_SOURCE_DIR}/Engine/lib )
ADD_DEFINITIONS( -DUNICODE -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS )
INCLUDE_DIRECTORIES( ${LIB_DIR}/libtheora/include/theora ${LIB_DIR}/libtheora/include ${LIB_DIR}/libogg/include )
# Dushan - make this friendly name as in VS configuration provided before by projectGenerator
ADD_LIBRARY( libtheora ${THEORA_HEADERS} ${THEORA_SOURCES} )

# Dushan - NOTE to myself 
#        - test this on MacOS and FreeBSD
#        - Why we need this anyway on UNIX systems?
#        - We can simply use FindTheora.cmake file
IF( UNIX )
    SET_TARGET_PROPERTIES( libtheora PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC" )
ENDIF( UNIX )
