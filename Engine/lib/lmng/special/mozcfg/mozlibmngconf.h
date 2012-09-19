/* ************************************************************************** */
/* *                                                                        * */
/* * project   : libmng                                                     * */
/* * file      : mozlibmngconf.h           copyright (c) G.R-P 2003-2005    * */
/* * version   : 1.0.10                                                     * */
/* *                                                                        * */
/* * purpose   : special config file for Mozilla                            * */
/* *                                                                        * */
/* * author    : Glenn Randers-Pehrson                                      * */
/* *                                                                        * */
/* * comment   : This is the configuration file designed to minimize        * */
/* *             footprint for the integration with Mozilla.                * */
/* *                                                                        * */
/* * changes   :                                                            * */
/* *                                                                        * */
/* ************************************************************************** */

#ifndef _mozlibmng_conf_h_
#define _mozlibmng_conf_h_

/* Mozilla defines */

/* One or none of these may be defined via MNG_CFLAGS in "configure" */

#if defined(MNG_BUILD_RAW_MNG)    || \
    defined(MNG_BUILD_FULL_MNG)   || \
    defined(MNG_BUILD_MOZ_MNG)    || \
    defined(MNG_BUILD_MOZ_NO_JNG) || \
    defined(MNG_BUILD_WEB_MNG)    || \
    defined(MNG_BUILD_WEB_NO_JNG) || \
    defined(MNG_BUILD_LC)         || \
    defined(MNG_BUILD_LC_NO_JNG)  || \
    defined(MNG_BUILD_VLC)
# define MNG_BUILD_DEFINED
#endif

#ifndef MNG_BUILD_DEFINED
#define MNG_BUILD_FULL_MNG
#define MNG_BUILD_DEFINED
#endif

#if defined(MNG_BUILD_FULL_MNG)
#define MNG_DISABLE_UNUSED 
#endif

#if defined(MNG_BUILD_MOZ_MNG)
#define MNG_DISABLE_UNUSED 
#define MNG_ENABLE_FOOTPRINT
#endif

#if defined(MNG_BUILD_MOZ_NO_JNG)
#define MNG_DISABLE_UNUSED 
#define MNG_ENABLE_FOOTPRINT
#define MNG_DISABLE_JNG
#endif

#if defined(MNG_BUILD_WEB_MNG)
#define MNG_DISABLE_UNUSED 
#define MNG_DISABLE_DELTA_PNG 
#define MNG_ENABLE_FOOTPRINT
#define MNG_SKIPCHUNK_MAGN
#endif

#if defined(MNG_BUILD_WEB_NO_JNG)
#define MNG_DISABLE_UNUSED 
#define MNG_DISABLE_DELTA_PNG 
#define MNG_ENABLE_FOOTPRINT
#define MNG_SKIPCHUNK_MAGN
#define MNG_DISABLE_JNG
#endif

#if defined(MNG_BUILD_LC)
#define MNG_DISABLE_DELTA_PNG 
#define MNG_DISABLE_UNUSED 
#define MNG_ENABLE_FOOTPRINT
#define MNG_DISABLE_16_BIT
#define MNG_DISABLE_NON_LC
#endif

#if defined(MNG_BUILD_LC_NO_JNG)
#define MNG_DISABLE_DELTA_PNG 
#define MNG_DISABLE_UNUSED 
#define MNG_ENABLE_FOOTPRINT
#define MNG_DISABLE_16_BIT
#define MNG_DISABLE_JNG
#define MNG_DISABLE_NON_LC
#endif

#if defined(MNG_BUILD_VLC)
#define MNG_DISABLE_DELTA_PNG 
#define MNG_DISABLE_UNUSED 
#define MNG_ENABLE_FOOTPRINT
#define MNG_DISABLE_16_BIT
#define MNG_DISABLE_JNG
#define MNG_DISABLE_NON_LC
#define MNG_DISABLE_NON_VLC
#endif

#if defined(MNG_ENABLE_FOOTPRINT)
/* Perform footprint optimizations */
#define MNG_OPTIMIZE_FOOTPRINT_COMPOSE
#define MNG_OPTIMIZE_FOOTPRINT_DIV
#define MNG_OPTIMIZE_FOOTPRINT_SWITCH
#define MNG_DECREMENT_LOOPS
#define MNG_USE_ZLIB_CRC
#define MNG_OPTIMIZE_FOOTPRINT_INIT
#define MNG_OPTIMIZE_FOOTPRINT_MAGN
#define MNG_OPTIMIZE_OBJCLEANUP
#define MNG_OPTIMIZE_CHUNKINITFREE
#define MNG_OPTIMIZE_CHUNKASSIGN
#endif

#if defined(MNG_DISABLE_UNUSED)
/* Eliminate unused features from libmng */
#define MNG_NO_VERSION_QUERY_SUPPORT
#define MNG_NO_OLD_VERSIONS

#ifdef MOZ_CAIRO_GFX
#define MNG_SKIPCANVAS_RGB8
#define MNG_SKIPCANVAS_RGB8_A8
#else
#define MNG_SKIPCANVAS_BGRA8_PM
#endif

#define MNG_SKIPCANVAS_ABGR8
#define MNG_SKIPCANVAS_ARGB8
#define MNG_SKIPCANVAS_BGR8
#define MNG_SKIPCANVAS_BGRX8
#define MNG_SKIPCANVAS_BGRA8
#define MNG_SKIPCANVAS_RGBA8_PM
#define MNG_SKIPCANVAS_ARGB8_PM
#define MNG_SKIPCANVAS_ABGR8_PM
#define MNG_SKIPCANVAS_RGBA8
#define MNG_SKIPCANVAS_RGB555
#define MNG_SKIPCANVAS_BGR555
#define MNG_SKIPCANVAS_RGB565
#define MNG_SKIPCANVAS_BGR565
#define MNG_SKIPCANVAS_RGBA565
#define MNG_SKIPCANVAS_BGRA565
#define MNG_SKIPCANVAS_BGR565_A8
#define MNG_SKIP_MAXCANVAS
#define MNG_SKIPCHUNK_tEXt
#define MNG_SKIPCHUNK_zTXt
#define MNG_SKIPCHUNK_iTXt
#define MNG_SKIPCHUNK_bKGD
#define MNG_SKIPCHUNK_cHRM
#define MNG_SKIPCHUNK_hIST
#define MNG_SKIPCHUNK_iCCP
#define MNG_SKIPCHUNK_pHYs
#define MNG_SKIPCHUNK_sBIT
#define MNG_SKIPCHUNK_sPLT
#define MNG_SKIPCHUNK_tIME
#define MNG_SKIPCHUNK_evNT
#define MNG_SKIPCHUNK_eXPI
#define MNG_SKIPCHUNK_fPRI
#define MNG_SKIPCHUNK_nEED
#define MNG_SKIPCHUNK_pHYg
/* Eliminate "critical" but safe-to-ignore chunks (see mng_read_unknown()) */
#define MNG_SKIPCHUNK_SAVE
#define MNG_SKIPCHUNK_SEEK
#define MNG_SKIPCHUNK_DBYK
#define MNG_SKIPCHUNK_ORDR
/* Eliminate unused zlib and jpeg "get" and "set" accessors */
#define MNG_NO_ACCESS_ZLIB
#define MNG_NO_ACCESS_JPEG
/* Eliminate other unused features */
#define MNG_NO_SUPPORT_FUNCQUERY
#define MNG_NO_DISPLAY_GO_SUPPORTED
#define MNG_NO_CURRENT_INFO
#define MNG_NO_DFLT_INFO
#define MNG_NO_LOOP_SIGNALS_SUPPORTED
#define MNG_NO_OPEN_CLOSE_STREAM
#endif

#if defined(MNG_DISABLE_16_BIT)
/* Eliminate 16-bit support from libmng */
#define MNG_NO_16BIT_SUPPORT
#endif

#if defined(MNG_DISABLE_DELTA_PNG)
/* Eliminate Delta-PNG feature from libmng */
#define MNG_NO_DELTA_PNG
#endif

#if defined(MNG_DISABLE_NON_LC)
/* Eliminate non-MNG-LC chunks */
#define MNG_SKIPCHUNK_BASI
#define MNG_SKIPCHUNK_CLIP
#define MNG_SKIPCHUNK_CLON
#define MNG_SKIPCHUNK_DISC
#define MNG_SKIPCHUNK_MOVE
#define MNG_SKIPCHUNK_SHOW
#define MNG_SKIPCHUNK_PAST
#endif

#if defined(MNG_DISABLE_JNG)
/* If you change this you should also manually remove or restore
   jng-recognition in mozilla/modules/libpr0n/src/imgLoader.cpp */
#define MNG_NO_INCLUDE_JNG
#endif

#if defined(MNG_DISABLE_NON_VLC)
/* Eliminate non-MNG-VLC chunks */
#define MNG_SKIPCHUNK_DEFI
#define MNG_SKIPCHUNK_FRAM
#define MNG_SKIPCHUNK_LOOP
#define MNG_SKIPCHUNK_MAGN
#endif

#if defined(MNG_DISABLE_OPTIONAL_VLC)
/* Eliminate optional MNG-VLC chunks */
#define MNG_SKIPCHUNK_TERM
#define MNG_SKIPCHUNK_BACK
#define MNG_SKIPCHUNK_gAMA
#define MNG_SKIPCHUNK_sRGB
#endif

#endif /* _mozlibmng_conf_h */
