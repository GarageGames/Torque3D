unit libmng;

{****************************************************************************}
{*                                                                          *}
{*  COPYRIGHT NOTICE:                                                       *}
{*                                                                          *}
{*  Copyright (c) 2000-2004 Gerard Juyn (gerard@libmng.com)                 *}
{*  [You may insert additional notices after this sentence if you modify    *}
{*   this source]                                                           *}
{*                                                                          *}
{*  For the purposes of this copyright and license, "Contributing Authors"  *}
{*  is defined as the following set of individuals:                         *}
{*                                                                          *}
{*     Gerard Juyn                                                          *}
{*     (hopefully some more to come...)                                     *}
{*                                                                          *}
{*  The MNG Library is supplied "AS IS".  The Contributing Authors          *}
{*  disclaim all warranties, expressed or implied, including, without       *}
{*  limitation, the warranties of merchantability and of fitness for any    *}
{*  purpose.  The Contributing Authors assume no liability for direct,      *}
{*  indirect, incidental, special, exemplary, or consequential damages,     *}
{*  which may result from the use of the MNG Library, even if advised of    *}
{*  the possibility of such damage.                                         *}
{*                                                                          *}
{*  Permission is hereby granted to use, copy, modify, and distribute this  *}
{*  source code, or portions hereof, for any purpose, without fee, subject  *}
{*  to the following restrictions:                                          *}
{*                                                                          *}
{*  1. The origin of this source code must not be misrepresented;           *}
{*     you must not claim that you wrote the original software.             *}
{*                                                                          *}
{*  2. Altered versions must be plainly marked as such and must not be      *}
{*     misrepresented as being the original source.                         *}
{*                                                                          *}
{*  3. This Copyright notice may not be removed or altered from any source  *}
{*     or altered source distribution.                                      *}
{*                                                                          *}
{*  The Contributing Authors specifically permit, without fee, and          *}
{*  encourage the use of this source code as a component to supporting      *}
{*  the MNG and JNG file format in commercial products.  If you use this    *}
{*  source code in a product, acknowledgment would be highly appreciated.   *}
{*                                                                          *}
{****************************************************************************}
{*                                                                          *}
{*  project   : libmng                                                      *}
{*  file      : libmng.pas                copyright (c) 2000-2004 G.Juyn    *}
{*  version   : 1.0.8                                                       *}
{*                                                                          *}
{*  purpose   : libmng.dll wrapper unit                                     *}
{*                                                                          *}
{*  author    : G.Juyn                                                      *}
{*  web       : http://www.3-t.com                                          *}
{*  email     : mailto:info (at) 3-t (dot) com                              *}
{*                                                                          *}
{*  comment   : contains the pascal-translation of libmng.h                 *}
{*              can be used by Delphi programs to access the libmng.dll     *}
{*                                                                          *}
{*  changes   : 0.5.1 - 05/02/2000 - G.Juyn                                 *}
{*              - added this version block                                  *}
{*              0.5.1 - 05/08/2000 - G.Juyn                                 *}
{*              - changed to stdcall convention                             *}
{*              0.5.1 - 05/11/2000 - G.Juyn                                 *}
{*              - changed callback prototypes                               *}
{*              - added TRUE/FALSE/NULL constants                           *}
{*              - added setoutputprofile2 & setsrgbprofile2                 *}
{*              - added several new types                                   *}
{*              - added chunk-access functions                              *}
{*              - added new error- & tracecodes                             *}
{*                                                                          *}
{*              0.5.2 - 05/24/2000 - G.Juyn                                 *}
{*              - removed error- & trace-strings since they are now         *}
{*                provided by the library                                   *}
{*                                                                          *}
{*              0.5.3 - 06/21/2000 - G.Juyn                                 *}
{*              - fixed definition of imagetype                             *}
{*              - added definition of speedtype                             *}
{*              - added get/set speed parameter                             *}
{*              - added get imagelevel parameter                            *}
{*              0.5.3 - 06/26/2000 - G.Juyn                                 *}
{*              - changed definition of userdata to mng_ptr                 *}
{*              0.5.3 - 06/28/2000 - G.Juyn                                 *}
{*              - added mng_size_t definition                               *}
{*              - changed definition of memory alloc size to mng_size_t     *}
{*              0.5.3 - 06/29/2000 - G.Juyn                                 *}
{*              - changed order of refresh parameters                       *}
{*              - changed definition of mng_handle                          *}
{*                                                                          *}
{*              0.9.0 - 06/30/2000 - G.Juyn                                 *}
{*              - changed refresh parameters to 'x,y,width,height'          *}
{*                                                                          *}
{*              0.9.1 - 07/08/2000 - G.Juyn                                 *}
{*              - added libmng errorcode constants                          *}
{*              0.9.1 - 07/10/2000 - G.Juyn                                 *}
{*              - added new libmng functions                                *}
{*              0.9.1 - 07/19/2000 - G.Juyn                                 *}
{*              - fixed several type definitions                            *}
{*              0.9.1 - 07/25/2000 - G.Juyn                                 *}
{*              - fixed definition of mng_imgtype                           *}
{*                                                                          *}
{*              0.9.2 - 08/04/2000 - G.Juyn                                 *}
{*              - fixed in line with libmng.h                               *}
{*              0.9.2 - 08/05/2000 - G.Juyn                                 *}
{*              - added function to set simplicity field                    *}
{*                                                                          *}
{*              0.9.3 - 10/21/2000 - G.Juyn                                 *}
{*              - added several new HLAPI entry points                      *}
{*                                                                          *}
{*              1.0.5 - 09/16/2002 - G.Juyn                                 *}
{*              - added dynamic MNG features                                *}
{*                                                                          *}
{*              1.0.8 - 04/12/2004 - G.Juyn                                 *}
{*              - added CRC existence & checking flags                      *}
{*              - added push mechanisms                                     *}
{*                                                                          *}
{****************************************************************************}

interface

{****************************************************************************}

const MNG_TRUE       = TRUE;
      MNG_FALSE      = FALSE;
      MNG_NULL       = nil;

type  mng_uint32     = cardinal;
      mng_int32      = integer;
      mng_uint16     = word;
      mng_int16      = smallint;
      mng_uint8      = byte;
      mng_int8       = shortint;
      mng_bool       = boolean;
      mng_ptr        = pointer;
      mng_pchar      = pchar;

      mng_handle     = pointer;
      mng_retcode    = mng_int32;
      mng_chunkid    = mng_uint32;

      mng_size_t     = cardinal;

      mng_imgtype    = (mng_it_unknown, mng_it_png, mng_it_mng, mng_it_jng);
      mng_speedtype  = (mng_st_normal, mng_st_fast, mng_st_slow, mng_st_slowest);

      mng_uint32p    = ^mng_uint32;
      mng_uint16p    = ^mng_uint16;
      mng_uint8p     = ^mng_uint8;
      mng_chunkidp   = ^mng_chunkid;

      mng_palette8e  = packed record             { 8-bit palette element }
                         iRed   : mng_uint8;
                         iGreen : mng_uint8;
                         iBlue  : mng_uint8;
                       end;

      mng_palette8   = packed array [0 .. 255] of mng_palette8e;

      mng_uint8arr   = packed array [0 .. 255] of mng_uint8;
      mng_uint8arr4  = packed array [0 ..   3] of mng_uint8;
      mng_uint16arr  = packed array [0 .. 255] of mng_uint16;
      mng_uint32arr2 = packed array [0 ..   1] of mng_uint32;

{****************************************************************************}

type mng_memalloc      = function  (    iLen         : mng_size_t) : mng_ptr; stdcall;
type mng_memfree       = procedure (    pPtr         : mng_ptr;
                                        iLen         : mng_size_t); stdcall;

type mng_releasedata   = procedure (    pUserData    : mng_ptr;
                                        pData        : mng_ptr;
                                        iLength      : mng_size_t); stdcall;

type mng_openstream    = function  (    hHandle      : mng_handle) : mng_bool; stdcall;
type mng_closestream   = function  (    hHandle      : mng_handle) : mng_bool; stdcall;

type mng_readdata      = function  (    hHandle      : mng_handle;
                                        pBuf         : mng_ptr;
                                        iBuflen      : mng_uint32;
                                    var pRead        : mng_uint32) : mng_bool; stdcall;

type mng_writedata     = function  (    hHandle      : mng_handle;
                                        pBuf         : mng_ptr;
                                        iBuflen      : mng_uint32;
                                    var pWritten     : mng_uint32) : mng_bool; stdcall;

type mng_errorproc     = function  (    hHandle      : mng_handle;
                                        iErrorcode   : mng_retcode;
                                        iSeverity    : mng_uint8;
                                        iChunkname   : mng_chunkid;
                                        iChunkseq    : mng_uint32;
                                        iExtra1      : mng_int32;
                                        iExtra2      : mng_int32;
                                        zErrortext   : mng_pchar ) : mng_bool; stdcall;
type mng_traceproc     = function  (    hHandle      : mng_handle;
                                        iFuncnr      : mng_int32;
                                        iFuncseq     : mng_uint32;
                                        zFuncname    : mng_pchar ) : mng_bool; stdcall;

type mng_processheader = function  (    hHandle      : mng_handle;
                                        iWidth       : mng_uint32;
                                        iHeight      : mng_uint32) : mng_bool; stdcall;
type mng_processtext   = function  (    hHandle      : mng_handle;
                                        iType        : mng_uint8;
                                        zKeyword     : mng_pchar;
                                        zText        : mng_pchar;
                                        zLanguage    : mng_pchar;
                                        zTranslation : mng_pchar ) : mng_bool; stdcall;

type mng_processsave   = function  (    hHandle      : mng_handle) : mng_bool; stdcall;
type mng_processseek   = function  (    hHandle      : mng_handle;
                                        zName        : mng_pchar ) : mng_bool; stdcall;

type mng_processneed   = function  (    hHandle      : mng_handle;
                                        zKeyword     : mng_pchar ) : mng_bool; stdcall;

type mng_processunknown = function (    hHandle      : mng_handle;
                                        iChunkid     : mng_chunkid;
                                        iRawlen      : mng_uint32;
                                        pRawdata     : mng_ptr   ) : mng_bool; stdcall;

type mng_getcanvasline = function  (    hHandle      : mng_handle;
                                        iLinenr      : mng_uint32) : mng_ptr; stdcall;
type mng_getalphaline  = function  (    hHandle      : mng_handle;
                                        iLinenr      : mng_uint32) : mng_ptr; stdcall;
type mng_getbkgdline   = function  (    hHandle      : mng_handle;
                                        iLinenr      : mng_uint32) : mng_ptr; stdcall;
type mng_refresh       = function  (    hHandle      : mng_handle;
                                        iX           : mng_uint32;
                                        iY           : mng_uint32;
                                        iWidth       : mng_uint32;
                                        iHeight      : mng_uint32) : mng_bool; stdcall;

type mng_gettickcount  = function  (    hHandle      : mng_handle) : mng_uint32; stdcall;
type mng_settimer      = function  (    hHandle      : mng_handle;
                                        iMsecs       : mng_uint32) : mng_bool; stdcall;

type mng_processgamma  = function  (    hHandle      : mng_handle;
                                        iGamma       : mng_uint32) : mng_bool; stdcall;
type mng_processchroma = function  (    hHandle      : mng_handle;
                                        iWhitex      : mng_uint32;
                                        iWhitey      : mng_uint32;
                                        iRedx        : mng_uint32;
                                        iRedy        : mng_uint32;
                                        iGreenx      : mng_uint32;
                                        iGreeny      : mng_uint32;
                                        iBluex       : mng_uint32;
                                        iBluey       : mng_uint32) : mng_bool; stdcall;
type mng_processsrgb   = function  (    hHandle      : mng_handle;
                                        iIntent      : mng_uint8 ) : mng_bool; stdcall;
type mng_processiccp   = function  (    hHandle      : mng_handle;
                                        iProfilesize : mng_uint32;
                                        pProfile     : mng_ptr   ) : mng_bool; stdcall;
type mng_processarow   = function  (    hHandle      : mng_handle;
                                        iRowsamples  : mng_uint32;
                                        bIsRGBA16    : mng_bool;
                                        pRow         : mng_ptr   ) : mng_bool; stdcall;

type mng_iteratechunk  = function  (    hHandle      : mng_handle;
                                        hChunk       : mng_handle;
                                        iChunkid     : mng_chunkid;
                                        iChunkseq    : mng_uint32) : mng_bool; stdcall;

{****************************************************************************}

function  mng_initialize          (    pUserdata       : mng_ptr;
                                       fMemalloc       : mng_memalloc;
                                       fMemfree        : mng_memfree;
                                       fTraceproc      : mng_traceproc    ) : mng_handle;        stdcall;

function  mng_reset               (    hHandle         : mng_handle       ) : mng_retcode;       stdcall;

function  mng_cleanup             (var hHandle         : mng_handle       ) : mng_retcode;       stdcall;

function  mng_read                (    hHandle         : mng_handle       ) : mng_retcode;       stdcall;
function  mng_read_pushdata       (    hHandle         : mng_handle;
                                       pData           : mng_ptr;
                                       iLength         : mng_uint32;
                                       bTakeownership  : mng_bool         ) : mng_retcode;       stdcall;
function  mng_read_pushsig        (    hHandle         : mng_handle;
                                       eSigtype        : mng_imgtype      ) : mng_retcode;       stdcall;
function  mng_read_pushchunk      (    hHandle         : mng_handle;
                                       pData           : mng_ptr;
                                       iLength         : mng_uint32;
                                       bTakeownership  : mng_bool         ) : mng_retcode;       stdcall;
function  mng_read_resume         (    hHandle         : mng_handle       ) : mng_retcode;       stdcall;
function  mng_write               (    hHandle         : mng_handle       ) : mng_retcode;       stdcall;
function  mng_create              (    hHandle         : mng_handle       ) : mng_retcode;       stdcall;

function  mng_readdisplay         (    hHandle         : mng_handle       ) : mng_retcode;       stdcall;
function  mng_display             (    hHandle         : mng_handle       ) : mng_retcode;       stdcall;
function  mng_display_resume      (    hHandle         : mng_handle       ) : mng_retcode;       stdcall;
function  mng_display_freeze      (    hHandle         : mng_handle       ) : mng_retcode;       stdcall;
function  mng_display_reset       (    hHandle         : mng_handle       ) : mng_retcode;       stdcall;
function  mng_display_goframe     (    hHandle         : mng_handle;
                                       iFramenr        : mng_uint32       ) : mng_retcode;       stdcall;
function  mng_display_golayer     (    hHandle         : mng_handle;
                                       iLayernr        : mng_uint32       ) : mng_retcode;       stdcall;
function  mng_display_gotime      (    hHandle         : mng_handle;
                                       iPlaytime       : mng_uint32       ) : mng_retcode;       stdcall;

function  mng_trapevent           (    hHandle         : mng_handle;
                                       iEventtype      : mng_uint8;
                                       iX              : mng_int32;
                                       iY              : mng_int32        ) : mng_retcode;       stdcall;     

function  mng_getlasterror        (    hHandle         : mng_handle;
                                   var iSeverity       : mng_uint8;
                                   var iChunkname      : mng_chunkid;
                                   var iChunkseq       : mng_uint32;
                                   var iExtra1         : mng_int32;
                                   var iExtra2         : mng_int32;
                                   var zErrortext      : mng_pchar        ) : mng_retcode;       stdcall;

{****************************************************************************}

function  mng_setcb_memalloc      (    hHandle         : mng_handle;
                                       fProc           : mng_memalloc     ) : mng_retcode;       stdcall;
function  mng_setcb_memfree       (    hHandle         : mng_handle;
                                       fProc           : mng_memfree      ) : mng_retcode;       stdcall;
function  mng_setcb_releasedata   (    hHandle         : mng_handle;
                                       fProc           : mng_releasedata  ) : mng_retcode;       stdcall;

function  mng_setcb_openstream    (    hHandle         : mng_handle;
                                       fProc           : mng_openstream   ) : mng_retcode;       stdcall;
function  mng_setcb_closestream   (    hHandle         : mng_handle;
                                       fProc           : mng_closestream  ) : mng_retcode;       stdcall;

function  mng_setcb_readdata      (    hHandle         : mng_handle;
                                       fProc           : mng_readdata     ) : mng_retcode;       stdcall;

function  mng_setcb_writedata     (    hHandle         : mng_handle;
                                       fProc           : mng_writedata    ) : mng_retcode;       stdcall;

function  mng_setcb_errorproc     (    hHandle         : mng_handle;
                                       fProc           : mng_errorproc    ) : mng_retcode;       stdcall;
function  mng_setcb_traceproc     (    hHandle         : mng_handle;
                                       fProc           : mng_traceproc    ) : mng_retcode;       stdcall;

function  mng_setcb_processheader (    hHandle         : mng_handle;
                                       fProc           : mng_processheader) : mng_retcode;       stdcall;
function  mng_setcb_processtext   (    hHandle         : mng_handle;
                                       fProc           : mng_processtext  ) : mng_retcode;       stdcall;

function  mng_setcb_getcanvasline (    hHandle         : mng_handle;
                                       fProc           : mng_getcanvasline) : mng_retcode;       stdcall;
function  mng_setcb_getalphaline  (    hHandle         : mng_handle;
                                       fProc           : mng_getalphaline ) : mng_retcode;       stdcall;
function  mng_setcb_getbkgdline   (    hHandle         : mng_handle;
                                       fProc           : mng_getbkgdline  ) : mng_retcode;       stdcall;
function  mng_setcb_refresh       (    hHandle         : mng_handle;
                                       fProc           : mng_refresh      ) : mng_retcode;       stdcall;

function  mng_setcb_gettickcount  (    hHandle         : mng_handle;
                                       fProc           : mng_gettickcount ) : mng_retcode;       stdcall;
function  mng_setcb_settimer      (    hHandle         : mng_handle;
                                       fProc           : mng_settimer     ) : mng_retcode;       stdcall;

function  mng_setcb_processgamma  (    hHandle         : mng_handle;
                                       fProc           : mng_processgamma ) : mng_retcode;       stdcall;
function  mng_setcb_processchroma (    hHandle         : mng_handle;
                                       fProc           : mng_processchroma) : mng_retcode;       stdcall;
function  mng_setcb_processsrgb   (    hHandle         : mng_handle;
                                       fProc           : mng_processsrgb  ) : mng_retcode;       stdcall;
function  mng_setcb_processiccp   (    hHandle         : mng_handle;
                                       fProc           : mng_processiccp  ) : mng_retcode;       stdcall;
function  mng_setcb_processarow   (    hHandle         : mng_handle;
                                       fProc           : mng_processarow  ) : mng_retcode;       stdcall;

{****************************************************************************}

function  mng_getcb_memalloc      (    hHandle         : mng_handle       ) : mng_memalloc;      stdcall;
function  mng_getcb_memfree       (    hHandle         : mng_handle       ) : mng_memfree;       stdcall;
function  mng_getcb_releasedata   (    hHandle         : mng_handle       ) : mng_releasedata;   stdcall;

function  mng_getcb_openstream    (    hHandle         : mng_handle       ) : mng_openstream;    stdcall;
function  mng_getcb_closestream   (    hHandle         : mng_handle       ) : mng_closestream;   stdcall;

function  mng_getcb_readdata      (    hHandle         : mng_handle       ) : mng_readdata;      stdcall;

function  mng_getcb_writedata     (    hHandle         : mng_handle       ) : mng_writedata;     stdcall;

function  mng_getcb_errorproc     (    hHandle         : mng_handle       ) : mng_errorproc;     stdcall;
function  mng_getcb_traceproc     (    hHandle         : mng_handle       ) : mng_traceproc;     stdcall;

function  mng_getcb_processheader (    hHandle         : mng_handle       ) : mng_processheader; stdcall;
function  mng_getcb_processtext   (    hHandle         : mng_handle       ) : mng_processtext;   stdcall;

function  mng_getcb_getcanvasline (    hHandle         : mng_handle       ) : mng_getcanvasline; stdcall;
function  mng_getcb_getalphaline  (    hHandle         : mng_handle       ) : mng_getalphaline;  stdcall;
function  mng_getcb_getbkgdline   (    hHandle         : mng_handle       ) : mng_getbkgdline;   stdcall;
function  mng_getcb_refresh       (    hHandle         : mng_handle       ) : mng_refresh;       stdcall;

function  mng_getcb_gettickcount  (    hHandle         : mng_handle       ) : mng_gettickcount;  stdcall;
function  mng_getcb_settimer      (    hHandle         : mng_handle       ) : mng_settimer;      stdcall;

function  mng_getcb_processgamma  (    hHandle         : mng_handle       ) : mng_processgamma;  stdcall;
function  mng_getcb_processchroma (    hHandle         : mng_handle       ) : mng_processchroma; stdcall;
function  mng_getcb_processsrgb   (    hHandle         : mng_handle       ) : mng_processsrgb;   stdcall;
function  mng_getcb_processiccp   (    hHandle         : mng_handle       ) : mng_processiccp;   stdcall;
function  mng_getcb_processarow   (    hHandle         : mng_handle       ) : mng_processarow;   stdcall;

{****************************************************************************}

function  mng_set_userdata        (    hHandle         : mng_handle;
                                       pUserdata       : mng_ptr          ) : mng_retcode;       stdcall;

function  mng_set_canvasstyle     (    hHandle         : mng_handle;
                                       iStyle          : mng_uint32       ) : mng_retcode;       stdcall;
function  mng_set_bkgdstyle       (    hHandle         : mng_handle;
                                       iStyle          : mng_uint32       ) : mng_retcode;       stdcall;

function  mng_set_bgcolor         (    hHandle         : mng_handle;
                                       iRed            : mng_uint16;
                                       iGreen          : mng_uint16;
                                       iBlue           : mng_uint16       ) : mng_retcode;       stdcall;

function  mng_set_usebkgd         (    hHandle         : mng_handle;
                                       bUseBKGD        : mng_bool         ) : mng_retcode;       stdcall;

function  mng_set_storechunks     (    hHandle         : mng_handle;
                                       bStorechunks    : mng_bool         ) : mng_retcode;       stdcall;

function  mng_set_cacheplayback   (    hHandle         : mng_handle;
                                       bCacheplayback  : mng_bool         ) : mng_retcode;       stdcall;

function  mng_set_viewgammaint    (    hHandle         : mng_handle;
                                       iGamma          : mng_uint32       ) : mng_retcode;       stdcall;
function  mng_set_displaygammaint (    hHandle         : mng_handle;
                                       iGamma          : mng_uint32       ) : mng_retcode;       stdcall;
function  mng_set_dfltimggammaint (    hHandle         : mng_handle;
                                       iGamma          : mng_uint32       ) : mng_retcode;       stdcall;

function  mng_set_srgb            (    hHandle         : mng_handle;
                                       bIssRGB         : mng_bool         ) : mng_retcode;       stdcall;
function  mng_set_outputprofile   (    hHandle         : mng_handle;
                                       zFilename       : mng_pchar        ) : mng_retcode;       stdcall;
function  mng_set_outputprofile2  (    hHandle         : mng_handle;
                                       iProfilesize    : mng_uint32;
                                       pProfile        : mng_ptr          ) : mng_retcode;       stdcall;
function  mng_set_srgbprofile     (    hHandle         : mng_handle;
                                       zFilename       : mng_pchar        ) : mng_retcode;       stdcall;
function  mng_set_srgbprofile2    (    hHandle         : mng_handle;
                                       iProfilesize    : mng_uint32;
                                       pProfile        : mng_ptr          ) : mng_retcode;       stdcall;

function  mng_set_maxcanvaswidth  (    hHandle         : mng_handle;
                                       iMaxwidth       : mng_uint32       ) : mng_retcode;       stdcall;
function  mng_set_maxcanvasheight (    hHandle         : mng_handle;
                                       iMaxheight      : mng_uint32       ) : mng_retcode;       stdcall;
function  mng_set_maxcanvassize   (    hHandle         : mng_handle;
                                       iMaxwidth       : mng_uint32;
                                       iMaxheight      : mng_uint32       ) : mng_retcode;       stdcall;

function  mng_set_suspensionmode  (    hHandle         : mng_handle;
                                       bSuspensionmode : mng_bool         ) : mng_retcode;       stdcall;

function  mng_set_speed           (    hHandle         : mng_handle;
                                       iSpeed          : mng_speedtype    ) : mng_retcode;       stdcall;

function  mng_set_crcmode         (    hHandle         : mng_handle;
                                       iCrcmode        : mng_uint32       ) : mng_retcode;       stdcall;

{****************************************************************************}

function  mng_get_userdata        (    hHandle         : mng_handle       ) : mng_ptr;           stdcall;

function  mng_get_sigtype         (    hHandle         : mng_handle       ) : mng_imgtype;       stdcall;
function  mng_get_imagetype       (    hHandle         : mng_handle       ) : mng_imgtype;       stdcall;
function  mng_get_imagewidth      (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_imageheight     (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_ticks           (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_framecount      (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_layercount      (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_playtime        (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_simplicity      (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;

function  mng_get_canvasstyle     (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_bkgdstyle       (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;

procedure mng_get_bgcolor         (    hHandle         : mng_handle;
                                   var iRed            : mng_uint16;
                                   var iGreen          : mng_uint16;
                                   var iBlue           : mng_uint16       );                     stdcall;

function  mng_get_usebkgd         (    hHandle         : mng_handle       ) : mng_bool;          stdcall;

function  mng_get_storechunks     (    hHandle         : mng_handle       ) : mng_bool;          stdcall;
function  mng_get_cacheplayback   (    hHandle         : mng_handle       ) : mng_bool;          stdcall;

function  mng_get_viewgammaint    (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_displaygammaint (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_dfltimggammaint (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;

function  mng_get_srgb            (    hHandle         : mng_handle       ) : mng_bool;          stdcall;

function  mng_get_maxcanvaswidth  (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_maxcanvasheight (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;

function  mng_get_suspensionmode  (    hHandle         : mng_handle       ) : mng_bool;          stdcall;

function  mng_get_speed           (    hHandle         : mng_handle       ) : mng_speedtype;     stdcall;
function  mng_get_imagelevel      (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;

function  mng_get_starttime       (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_runtime         (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_currentframe    (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_currentlayer    (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_currentplaytime (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_totalframes     (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_totallayers     (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;
function  mng_get_totalplaytime   (    hHandle         : mng_handle       ) : mng_uint32;        stdcall;

function  mng_status_error        (    hHandle         : mng_handle       ) : mng_bool;          stdcall;
function  mng_status_reading      (    hHandle         : mng_handle       ) : mng_bool;          stdcall;
function  mng_status_suspendbreak (    hHandle         : mng_handle       ) : mng_bool;          stdcall;
function  mng_status_creating     (    hHandle         : mng_handle       ) : mng_bool;          stdcall;
function  mng_status_writing      (    hHandle         : mng_handle       ) : mng_bool;          stdcall;
function  mng_status_displaying   (    hHandle         : mng_handle       ) : mng_bool;          stdcall;
function  mng_status_running      (    hHandle         : mng_handle       ) : mng_bool;          stdcall;
function  mng_status_timerbreak   (    hHandle         : mng_handle       ) : mng_bool;          stdcall;
function  mng_status_dynamic      (    hHandle         : mng_handle       ) : mng_bool;          stdcall;

{****************************************************************************}

function  mng_iterate_chunks      (    hHandle         : mng_handle;
                                       iChunkseq       : mng_uint32;
                                       fProc           : mng_iteratechunk ) : mng_retcode;       stdcall;

{****************************************************************************}

function  mng_getchunk_ihdr       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iWidth             : mng_uint32;
                                   var iHeight            : mng_uint32;
                                   var iBitdepth          : mng_uint8;
                                   var iColortype         : mng_uint8;
                                   var iCompression       : mng_uint8;
                                   var iFilter            : mng_uint8;
                                   var iInterlace         : mng_uint8    ) : mng_retcode; stdcall;

function  mng_getchunk_plte       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iCount             : mng_uint32;
                                   var aPalette           : mng_palette8 ) : mng_retcode; stdcall;

function  mng_getchunk_idat       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iRawlen            : mng_uint32;
                                   var pRawdata           : mng_ptr      ) : mng_retcode; stdcall;

function  mng_getchunk_trns       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var bEmpty             : mng_bool;
                                   var bGlobal            : mng_bool;
                                   var iType              : mng_uint8;
                                   var iCount             : mng_uint32;
                                   var aAlphas            : mng_uint8arr;
                                   var iGray              : mng_uint16;
                                   var iRed               : mng_uint16;
                                   var iGreen             : mng_uint16;
                                   var iBlue              : mng_uint16;
                                   var iRawlen            : mng_uint32;
                                   var aRawdata           : mng_uint8arr ) : mng_retcode; stdcall;

function  mng_getchunk_gama       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var bEmpty             : mng_bool;
                                   var iGamma             : mng_uint32   ) : mng_retcode; stdcall;

function  mng_getchunk_chrm       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var bEmpty             : mng_bool;
                                   var iWhitepointx       : mng_uint32;
                                   var iWhitepointy       : mng_uint32;
                                   var iRedx              : mng_uint32;
                                   var iRedy              : mng_uint32;
                                   var iGreenx            : mng_uint32;
                                   var iGreeny            : mng_uint32;
                                   var iBluex             : mng_uint32;
                                   var iBluey             : mng_uint32   ) : mng_retcode; stdcall;

function  mng_getchunk_srgb       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var bEmpty             : mng_bool;
                                   var iRenderingintent   : mng_uint8    ) : mng_retcode; stdcall;

function  mng_getchunk_iccp       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var bEmpty             : mng_bool;
                                   var iNamesize          : mng_uint32;
                                   var zName              : mng_pchar;
                                   var iCompression       : mng_uint8;
                                   var iProfilesize       : mng_uint32;
                                   var pProfile           : mng_ptr      ) : mng_retcode; stdcall;

function  mng_getchunk_text       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iKeywordsize       : mng_uint32;
                                   var zKeyword           : mng_pchar;
                                   var iTextsize          : mng_uint32;
                                   var zText              : mng_pchar    ) : mng_retcode; stdcall;

function  mng_getchunk_ztxt       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iKeywordsize       : mng_uint32;
                                   var zKeyword           : mng_pchar;
                                   var iCompression       : mng_uint8;
                                   var iTextsize          : mng_uint32;
                                   var zText              : mng_pchar    ) : mng_retcode; stdcall;

function  mng_getchunk_itxt       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iKeywordsize       : mng_uint32;
                                   var zKeyword           : mng_pchar;
                                   var iCompressionflag   : mng_uint8;
                                   var iCompressionmethod : mng_uint8;
                                   var iLanguagesize      : mng_uint32;
                                   var zLanguage          : mng_pchar;
                                   var iTranslationsize   : mng_uint32;
                                   var zTranslation       : mng_pchar;
                                   var iTextsize          : mng_uint32;
                                   var zText              : mng_pchar    ) : mng_retcode; stdcall;

function  mng_getchunk_bkgd       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var bEmpty             : mng_bool;
                                   var iType              : mng_uint8;
                                   var iIndex             : mng_uint8;
                                   var iGray              : mng_uint16;
                                   var iRed               : mng_uint16;
                                   var iGreen             : mng_uint16;
                                   var iBlue              : mng_uint16   ) : mng_retcode; stdcall;

function  mng_getchunk_phys       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var bEmpty             : mng_bool;
                                   var iSizex             : mng_uint32;
                                   var iSizey             : mng_uint32;
                                   var iUnit              : mng_uint8    ) : mng_retcode; stdcall;

function  mng_getchunk_sbit       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var bEmpty             : mng_bool;
                                   var iType              : mng_uint8;
                                   var aBits              : mng_uint8arr4) : mng_retcode; stdcall;

function  mng_getchunk_splt       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var bEmpty             : mng_bool;
                                   var iNamesize          : mng_uint32;
                                   var zName              : mng_pchar;
                                   var iSampledepth       : mng_uint8;
                                   var iEntrycount        : mng_uint32;
                                   var pEntries           : mng_ptr      ) : mng_retcode; stdcall;

function  mng_getchunk_hist       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iEntrycount        : mng_uint32;
                                   var aEntries           : mng_uint16arr) : mng_retcode; stdcall;

function  mng_getchunk_time       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iYear              : mng_uint16;
                                   var iMonth             : mng_uint8;
                                   var iDay               : mng_uint8;
                                   var iHour              : mng_uint8;
                                   var iMinute            : mng_uint8;
                                   var iSecond            : mng_uint8    ) : mng_retcode; stdcall;

function  mng_getchunk_mhdr       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iWidth             : mng_uint32;
                                   var iHeight            : mng_uint32;
                                   var iTicks             : mng_uint32;
                                   var iLayercount        : mng_uint32;
                                   var iFramecount        : mng_uint32;
                                   var iPlaytime          : mng_uint32;
                                   var iSimplicity        : mng_uint32   ) : mng_retcode; stdcall;

function  mng_getchunk_loop       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iLevel             : mng_uint8;
                                   var iRepeat            : mng_uint32;
                                   var iTermination       : mng_uint8;
                                   var iItermin           : mng_uint32;
                                   var iItermax           : mng_uint32;
                                   var iCount             : mng_uint32;
                                   var pSignals           : mng_uint32p  ) : mng_retcode; stdcall;

function  mng_getchunk_endl       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iLevel             : mng_uint8    ) : mng_retcode; stdcall;

function  mng_getchunk_defi       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iObjectid          : mng_uint16;
                                   var iDonotshow         : mng_uint8;
                                   var iConcrete          : mng_uint8;
                                   var bHasloca           : mng_bool;
                                   var iXlocation         : mng_int32;
                                   var iYlocation         : mng_int32;
                                   var bHasclip           : mng_bool;
                                   var iLeftcb            : mng_int32;
                                   var iRightcb           : mng_int32;
                                   var iTopcb             : mng_int32;
                                   var iBottomcb          : mng_int32    ) : mng_retcode; stdcall;

function  mng_getchunk_basi       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iWidth             : mng_uint32;
                                   var iHeight            : mng_uint32;
                                   var iBitdepth          : mng_uint8;
                                   var iColortype         : mng_uint8;
                                   var iCompression       : mng_uint8;
                                   var iFilter            : mng_uint8;
                                   var iInterlace         : mng_uint8;
                                   var iRed               : mng_uint16;
                                   var iGreen             : mng_uint16;
                                   var iBlue              : mng_uint16;
                                   var iAlpha             : mng_uint16;
                                   var iViewable          : mng_uint8    ) : mng_retcode; stdcall;

function  mng_getchunk_clon       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iSourceid          : mng_uint16;
                                   var iCloneid           : mng_uint16;
                                   var iClonetype         : mng_uint8;
                                   var iDonotshow         : mng_uint8;
                                   var iConcrete          : mng_uint8;
                                   var bHasloca           : mng_bool;
                                   var iLocationtype      : mng_uint8;
                                   var iLocationx         : mng_int32;
                                   var iLocationy         : mng_int32    ) : mng_retcode; stdcall;

function  mng_getchunk_past       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iDestid            : mng_uint16;
                                   var iTargettype        : mng_uint8;
                                   var iTargetx           : mng_int32;
                                   var iTargety           : mng_int32;
                                   var iCount             : mng_uint32   ) : mng_retcode; stdcall;

function  mng_getchunk_past_src   (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                       iEntry             : mng_uint32;
                                   var iSourceid          : mng_uint16;
                                   var iComposition       : mng_uint8;
                                   var iOrientation       : mng_uint8;
                                   var iOffsettype        : mng_uint8;
                                   var iOffsetx           : mng_int32;
                                   var iOffsety           : mng_int32;
                                   var iBoundarytype      : mng_uint8;
                                   var iBoundaryl         : mng_int32;
                                   var iBoundaryr         : mng_int32;
                                   var iBoundaryt         : mng_int32;
                                   var iBoundaryb         : mng_int32    ) : mng_retcode; stdcall;

function  mng_getchunk_disc       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iCount             : mng_uint32;
                                   var pObjectids         : mng_uint16p  ) : mng_retcode; stdcall;

function  mng_getchunk_back       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iRed               : mng_uint16;
                                   var iGreen             : mng_uint16;
                                   var iBlue              : mng_uint16;
                                   var iMandatory         : mng_uint8;
                                   var iImageid           : mng_uint16;
                                   var iTile              : mng_uint8    ) : mng_retcode; stdcall;

function  mng_getchunk_fram       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var bEmpty             : mng_bool;
                                   var iMode              : mng_uint8;
                                   var iNamesize          : mng_uint32;
                                   var zName              : mng_pchar;
                                   var iChangedelay       : mng_uint8;
                                   var iChangetimeout     : mng_uint8;
                                   var iChangeclipping    : mng_uint8;
                                   var iChangesyncid      : mng_uint8;
                                   var iDelay             : mng_uint32;
                                   var iTimeout           : mng_uint32;
                                   var iBoundarytype      : mng_uint8;
                                   var iBoundaryl         : mng_int32;
                                   var iBoundaryr         : mng_int32;
                                   var iBoundaryt         : mng_int32;
                                   var iBoundaryb         : mng_int32;
                                   var iCount             : mng_uint32;
                                   var pSyncids           : mng_uint32p  ) : mng_retcode; stdcall;

function  mng_getchunk_move       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iFirstid           : mng_uint16;
                                   var iLastid            : mng_uint16;
                                   var iMovetype          : mng_uint8;
                                   var iMovex             : mng_int32;
                                   var iMovey             : mng_int32    ) : mng_retcode; stdcall;

function  mng_getchunk_clip       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iFirstid           : mng_uint16;
                                   var iLastid            : mng_uint16;
                                   var iCliptype          : mng_uint8;
                                   var iClipl             : mng_int32;
                                   var iClipr             : mng_int32;
                                   var iClipt             : mng_int32;
                                   var iClipb             : mng_int32    ) : mng_retcode; stdcall;

function  mng_getchunk_show       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var bEmpty             : mng_bool;
                                   var iFirstid           : mng_uint16;
                                   var iLastid            : mng_uint16;
                                   var iMode              : mng_uint8    ) : mng_retcode; stdcall;

function  mng_getchunk_term       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iTermaction        : mng_uint8;
                                   var iIteraction        : mng_uint8;
                                   var iDelay             : mng_uint32;
                                   var iItermax           : mng_uint32   ) : mng_retcode; stdcall;

function  mng_getchunk_save       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var bEmpty             : mng_bool;
                                   var iOffsettype        : mng_uint8;
                                   var iCount             : mng_uint32   ) : mng_retcode; stdcall;

function  mng_getchunk_save_entry (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                       iEntry             : mng_uint32;
                                   var iEntrytype         : mng_uint8;
                                   var iOffset            : mng_uint32arr2;
                                   var iStarttime         : mng_uint32arr2;
                                   var iLayernr           : mng_uint32;
                                   var iFramenr           : mng_uint32;
                                   var iNamesize          : mng_uint32;
                                   var zName              : mng_pchar    ) : mng_retcode; stdcall;

function  mng_getchunk_seek       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iNamesize          : mng_uint32;
                                   var zName              : mng_pchar    ) : mng_retcode; stdcall;

function  mng_getchunk_expi       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iSnapshotid        : mng_uint16;
                                   var iNamesize          : mng_uint32;
                                   var zName              : mng_pchar    ) : mng_retcode; stdcall;

function  mng_getchunk_fpri       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iDeltatype         : mng_uint8;
                                   var iPriority          : mng_uint8    ) : mng_retcode; stdcall;

function  mng_getchunk_need       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iKeywordssize      : mng_uint32;
                                   var zKeywords          : mng_pchar    ) : mng_retcode; stdcall;

function  mng_getchunk_phyg       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var bEmpty             : mng_bool;
                                   var iSizex             : mng_uint32;
                                   var iSizey             : mng_uint32;
                                   var iUnit              : mng_uint8    ) : mng_retcode; stdcall;

function  mng_getchunk_jhdr       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iWidth             : mng_uint32;
                                   var iHeight            : mng_uint32;
                                   var iColortype         : mng_uint8;
                                   var iImagesampledepth  : mng_uint8;
                                   var iImagecompression  : mng_uint8;
                                   var iImageinterlace    : mng_uint8;
                                   var iAlphasampledepth  : mng_uint8;
                                   var iAlphacompression  : mng_uint8;
                                   var iAlphafilter       : mng_uint8;
                                   var iAlphainterlace    : mng_uint8    ) : mng_retcode; stdcall;

function  mng_getchunk_jdat       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iRawlen            : mng_uint32;
                                   var pRawdata           : mng_ptr      ) : mng_retcode; stdcall;

function  mng_getchunk_dhdr       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iObjectid          : mng_uint16;
                                   var iImagetype         : mng_uint8;
                                   var iDeltatype         : mng_uint8;
                                   var iBlockwidth        : mng_uint32;
                                   var iBlockheight       : mng_uint32;
                                   var iBlockx            : mng_uint32;
                                   var iBlocky            : mng_uint32   ) : mng_retcode; stdcall;

function  mng_getchunk_prom       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iColortype         : mng_uint8;
                                   var iSampledepth       : mng_uint8;
                                   var iFilltype          : mng_uint8    ) : mng_retcode; stdcall;

function  mng_getchunk_pplt       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iCount             : mng_uint32   ) : mng_retcode; stdcall;

function  mng_getchunk_pplt_entry (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                       iEntry             : mng_uint32;
                                   var iRed               : mng_uint16;
                                   var iGreen             : mng_uint16;
                                   var iBlue              : mng_uint16;
                                   var iAlpha             : mng_uint16;
                                   var bUsed              : mng_bool     ) : mng_retcode; stdcall;

function  mng_getchunk_drop       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iCount             : mng_uint32;
                                   var pChunknames        : mng_chunkidp ) : mng_retcode; stdcall;

function  mng_getchunk_dbyk       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iChunkname         : mng_chunkid;
                                   var iPolarity          : mng_uint8;
                                   var iKeywordssize      : mng_uint32;
                                   var zKeywords          : mng_pchar    ) : mng_retcode; stdcall;

function  mng_getchunk_ordr       (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iCount             : mng_uint32   ) : mng_retcode; stdcall;

function  mng_getchunk_ordr_entry (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                       iEntry             : mng_uint32;
                                   var iChunkname         : mng_chunkid;
                                   var iOrdertype         : mng_uint8    ) : mng_retcode; stdcall;

function  mng_getchunk_unknown    (    hHandle            : mng_handle;
                                       hChunk             : mng_handle;
                                   var iChunkname         : mng_chunkid;
                                   var iRawlen            : mng_uint32;
                                   var pRawdata           : mng_ptr      ) : mng_retcode; stdcall;

{****************************************************************************}

function  mng_putchunk_ihdr       (    hHandle            : mng_handle;
                                       iWidth             : mng_uint32;
                                       iHeight            : mng_uint32;
                                       iBitdepth          : mng_uint8;
                                       iColortype         : mng_uint8;
                                       iCompression       : mng_uint8;
                                       iFilter            : mng_uint8;
                                       iInterlace         : mng_uint8    ) : mng_retcode; stdcall;

function  mng_putchunk_plte       (    hHandle            : mng_handle;
                                       iCount             : mng_uint32;
                                       aPalette           : mng_palette8 ) : mng_retcode; stdcall;

function  mng_putchunk_idat       (    hHandle            : mng_handle;
                                       iRawlen            : mng_uint32;
                                       pRawdata           : mng_ptr      ) : mng_retcode; stdcall;

function  mng_putchunk_iend       (    hHandle            : mng_handle   ) : mng_retcode; stdcall;

function  mng_putchunk_trns       (    hHandle            : mng_handle;
                                       bEmpty             : mng_bool;
                                       bGlobal            : mng_bool;
                                       iType              : mng_uint8;
                                       iCount             : mng_uint32;
                                       aAlphas            : mng_uint8arr;
                                       iGray              : mng_uint16;
                                       iRed               : mng_uint16;
                                       iGreen             : mng_uint16;
                                       iBlue              : mng_uint16;
                                       iRawlen            : mng_uint32;
                                       aRawdata           : mng_uint8arr ) : mng_retcode; stdcall;

function  mng_putchunk_gama       (    hHandle            : mng_handle;
                                       bEmpty             : mng_bool;
                                       iGamma             : mng_uint32   ) : mng_retcode; stdcall;

function  mng_putchunk_chrm       (    hHandle            : mng_handle;
                                       bEmpty             : mng_bool;
                                       iWhitepointx       : mng_uint32;
                                       iWhitepointy       : mng_uint32;
                                       iRedx              : mng_uint32;
                                       iRedy              : mng_uint32;
                                       iGreenx            : mng_uint32;
                                       iGreeny            : mng_uint32;
                                       iBluex             : mng_uint32;
                                       iBluey             : mng_uint32   ) : mng_retcode; stdcall;

function  mng_putchunk_srgb       (    hHandle            : mng_handle;
                                       bEmpty             : mng_bool;
                                       iRenderingintent   : mng_uint8    ) : mng_retcode; stdcall;

function  mng_putchunk_iccp       (    hHandle            : mng_handle;
                                       bEmpty             : mng_bool;
                                       iNamesize          : mng_uint32;
                                       zName              : mng_pchar;
                                       iCompression       : mng_uint8;
                                       iProfilesize       : mng_uint32;
                                       pProfile           : mng_ptr      ) : mng_retcode; stdcall;

function  mng_putchunk_text       (    hHandle            : mng_handle;
                                       iKeywordsize       : mng_uint32;
                                       zKeyword           : mng_pchar;
                                       iTextsize          : mng_uint32;
                                       zText              : mng_pchar    ) : mng_retcode; stdcall;

function  mng_putchunk_ztxt       (    hHandle            : mng_handle;
                                       iKeywordsize       : mng_uint32;
                                       zKeyword           : mng_pchar;
                                       iCompression       : mng_uint8;
                                       iTextsize          : mng_uint32;
                                       zText              : mng_pchar    ) : mng_retcode; stdcall;

function  mng_putchunk_itxt       (    hHandle            : mng_handle;
                                       iKeywordsize       : mng_uint32;
                                       zKeyword           : mng_pchar;
                                       iCompressionflag   : mng_uint8;
                                       iCompressionmethod : mng_uint8;
                                       iLanguagesize      : mng_uint32;
                                       zLanguage          : mng_pchar;
                                       iTranslationsize   : mng_uint32;
                                       zTranslation       : mng_pchar;
                                       iTextsize          : mng_uint32;
                                       zText              : mng_pchar    ) : mng_retcode; stdcall;

function  mng_putchunk_bkgd       (    hHandle            : mng_handle;
                                       bEmpty             : mng_bool;
                                       iType              : mng_uint8;
                                       iIndex             : mng_uint8;
                                       iGray              : mng_uint16;
                                       iRed               : mng_uint16;
                                       iGreen             : mng_uint16;
                                       iBlue              : mng_uint16   ) : mng_retcode; stdcall;

function  mng_putchunk_phys       (    hHandle            : mng_handle;
                                       bEmpty             : mng_bool;
                                       iSizex             : mng_uint32;
                                       iSizey             : mng_uint32;
                                       iUnit              : mng_uint8    ) : mng_retcode; stdcall;

function  mng_putchunk_sbit       (    hHandle            : mng_handle;
                                       bEmpty             : mng_bool;
                                       iType              : mng_uint8;
                                       aBits              : mng_uint8arr4) : mng_retcode; stdcall;

function  mng_putchunk_splt       (    hHandle            : mng_handle;
                                       bEmpty             : mng_bool;
                                       iNamesize          : mng_uint32;
                                       zName              : mng_pchar;
                                       iSampledepth       : mng_uint8;
                                       iEntrycount        : mng_uint32;
                                       pEntries           : mng_ptr      ) : mng_retcode; stdcall;

function  mng_putchunk_hist       (    hHandle            : mng_handle;
                                       iEntrycount        : mng_uint32;
                                       aEntries           : mng_uint16arr) : mng_retcode; stdcall;

function  mng_putchunk_time       (    hHandle            : mng_handle;
                                       iYear              : mng_uint16;
                                       iMonth             : mng_uint8;
                                       iDay               : mng_uint8;
                                       iHour              : mng_uint8;
                                       iMinute            : mng_uint8;
                                       iSecond            : mng_uint8    ) : mng_retcode; stdcall;

function  mng_putchunk_mhdr       (    hHandle            : mng_handle;
                                       iWidth             : mng_uint32;
                                       iHeight            : mng_uint32;
                                       iTicks             : mng_uint32;
                                       iLayercount        : mng_uint32;
                                       iFramecount        : mng_uint32;
                                       iPlaytime          : mng_uint32;
                                       iSimplicity        : mng_uint32   ) : mng_retcode; stdcall;

function  mng_putchunk_mend       (    hHandle            : mng_handle   ) : mng_retcode; stdcall;

function  mng_putchunk_loop       (    hHandle            : mng_handle;
                                       iLevel             : mng_uint8;
                                       iRepeat            : mng_uint32;
                                       iTermination       : mng_uint8;
                                       iItermin           : mng_uint32;
                                       iItermax           : mng_uint32;
                                       iCount             : mng_uint32;
                                       pSignals           : mng_uint32p  ) : mng_retcode; stdcall;

function  mng_putchunk_endl       (    hHandle            : mng_handle;
                                       iLevel             : mng_uint8    ) : mng_retcode; stdcall;

function  mng_putchunk_defi       (    hHandle            : mng_handle;
                                       iObjectid          : mng_uint16;
                                       iDonotshow         : mng_uint8;
                                       iConcrete          : mng_uint8;
                                       bHasloca           : mng_bool;
                                       iXlocation         : mng_int32;
                                       iYlocation         : mng_int32;
                                       bHasclip           : mng_bool;
                                       iLeftcb            : mng_int32;
                                       iRightcb           : mng_int32;
                                       iTopcb             : mng_int32;
                                       iBottomcb          : mng_int32    ) : mng_retcode; stdcall;

function  mng_putchunk_basi       (    hHandle            : mng_handle;
                                       iWidth             : mng_uint32;
                                       iHeight            : mng_uint32;
                                       iBitdepth          : mng_uint8;
                                       iColortype         : mng_uint8;
                                       iCompression       : mng_uint8;
                                       iFilter            : mng_uint8;
                                       iInterlace         : mng_uint8;
                                       iRed               : mng_uint16;
                                       iGreen             : mng_uint16;
                                       iBlue              : mng_uint16;
                                       iAlpha             : mng_uint16;
                                       iViewable          : mng_uint8    ) : mng_retcode; stdcall;

function  mng_putchunk_clon       (    hHandle            : mng_handle;
                                       iSourceid          : mng_uint16;
                                       iCloneid           : mng_uint16;
                                       iClonetype         : mng_uint8;
                                       iDonotshow         : mng_uint8;
                                       iConcrete          : mng_uint8;
                                       bHasloca           : mng_bool;
                                       iLocationtype      : mng_uint8;
                                       iLocationx         : mng_int32;
                                       iLocationy         : mng_int32    ) : mng_retcode; stdcall;

function  mng_putchunk_past       (    hHandle            : mng_handle;
                                       iDestid            : mng_uint16;
                                       iTargettype        : mng_uint8;
                                       iTargetx           : mng_int32;
                                       iTargety           : mng_int32;
                                       iCount             : mng_uint32   ) : mng_retcode; stdcall;

function  mng_putchunk_past_src   (    hHandle            : mng_handle;
                                       iEntry             : mng_uint32;
                                       iSourceid          : mng_uint16;
                                       iComposition       : mng_uint8;
                                       iOrientation       : mng_uint8;
                                       iOffsettype        : mng_uint8;
                                       iOffsetx           : mng_int32;
                                       iOffsety           : mng_int32;
                                       iBoundarytype      : mng_uint8;
                                       iBoundaryl         : mng_int32;
                                       iBoundaryr         : mng_int32;
                                       iBoundaryt         : mng_int32;
                                       iBoundaryb         : mng_int32    ) : mng_retcode; stdcall;

function  mng_putchunk_disc       (    hHandle            : mng_handle;
                                       iCount             : mng_uint32;
                                       pObjectids         : mng_uint16p  ) : mng_retcode; stdcall;

function  mng_putchunk_back       (    hHandle            : mng_handle;
                                       iRed               : mng_uint16;
                                       iGreen             : mng_uint16;
                                       iBlue              : mng_uint16;
                                       iMandatory         : mng_uint8;
                                       iImageid           : mng_uint16;
                                       iTile              : mng_uint8    ) : mng_retcode; stdcall;

function  mng_putchunk_fram       (    hHandle            : mng_handle;
                                       bEmpty             : mng_bool;
                                       iMode              : mng_uint8;
                                       iNamesize          : mng_uint32;
                                       zName              : mng_pchar;
                                       iChangedelay       : mng_uint8;
                                       iChangetimeout     : mng_uint8;
                                       iChangeclipping    : mng_uint8;
                                       iChangesyncid      : mng_uint8;
                                       iDelay             : mng_uint32;
                                       iTimeout           : mng_uint32;
                                       iBoundarytype      : mng_uint8;
                                       iBoundaryl         : mng_int32;
                                       iBoundaryr         : mng_int32;
                                       iBoundaryt         : mng_int32;
                                       iBoundaryb         : mng_int32;
                                       iCount             : mng_uint32;
                                       pSyncids           : mng_uint32p  ) : mng_retcode; stdcall;

function  mng_putchunk_move       (    hHandle            : mng_handle;
                                       iFirstid           : mng_uint16;
                                       iLastid            : mng_uint16;
                                       iMovetype          : mng_uint8;
                                       iMovex             : mng_int32;
                                       iMovey             : mng_int32    ) : mng_retcode; stdcall;

function  mng_putchunk_clip       (    hHandle            : mng_handle;
                                       iFirstid           : mng_uint16;
                                       iLastid            : mng_uint16;
                                       iCliptype          : mng_uint8;
                                       iClipl             : mng_int32;
                                       iClipr             : mng_int32;
                                       iClipt             : mng_int32;
                                       iClipb             : mng_int32    ) : mng_retcode; stdcall;

function  mng_putchunk_show       (    hHandle            : mng_handle;
                                       bEmpty             : mng_bool;
                                       iFirstid           : mng_uint16;
                                       iLastid            : mng_uint16;
                                       iMode              : mng_uint8    ) : mng_retcode; stdcall;

function  mng_putchunk_term       (    hHandle            : mng_handle;
                                       iTermaction        : mng_uint8;
                                       iIteraction        : mng_uint8;
                                       iDelay             : mng_uint32;
                                       iItermax           : mng_uint32   ) : mng_retcode; stdcall;

function  mng_putchunk_save       (    hHandle            : mng_handle;
                                       bEmpty             : mng_bool;
                                       iOffsettype        : mng_uint8;
                                       iCount             : mng_uint32   ) : mng_retcode; stdcall;

function  mng_putchunk_save_entry (    hHandle            : mng_handle;
                                       iEntry             : mng_uint32;
                                       iEntrytype         : mng_uint8;
                                       iOffset            : mng_uint32arr2;
                                       iStarttime         : mng_uint32arr2;
                                       iLayernr           : mng_uint32;
                                       iFramenr           : mng_uint32;
                                       iNamesize          : mng_uint32;
                                       zName              : mng_pchar    ) : mng_retcode; stdcall;

function  mng_putchunk_seek       (    hHandle            : mng_handle;
                                       iNamesize          : mng_uint32;
                                       zName              : mng_pchar    ) : mng_retcode; stdcall;

function  mng_putchunk_expi       (    hHandle            : mng_handle;
                                       iSnapshotid        : mng_uint16;
                                       iNamesize          : mng_uint32;
                                       zName              : mng_pchar    ) : mng_retcode; stdcall;

function  mng_putchunk_fpri       (    hHandle            : mng_handle;
                                       iDeltatype         : mng_uint8;
                                       iPriority          : mng_uint8    ) : mng_retcode; stdcall;

function  mng_putchunk_need       (    hHandle            : mng_handle;
                                       iKeywordssize      : mng_uint32;
                                       zKeywords          : mng_pchar    ) : mng_retcode; stdcall;

function  mng_putchunk_phyg       (    hHandle            : mng_handle;
                                       bEmpty             : mng_bool;
                                       iSizex             : mng_uint32;
                                       iSizey             : mng_uint32;
                                       iUnit              : mng_uint8    ) : mng_retcode; stdcall;

function  mng_putchunk_jhdr       (    hHandle            : mng_handle;
                                       iWidth             : mng_uint32;
                                       iHeight            : mng_uint32;
                                       iColortype         : mng_uint8;
                                       iImagesampledepth  : mng_uint8;
                                       iImagecompression  : mng_uint8;
                                       iImageinterlace    : mng_uint8;
                                       iAlphasampledepth  : mng_uint8;
                                       iAlphacompression  : mng_uint8;
                                       iAlphafilter       : mng_uint8;
                                       iAlphainterlace    : mng_uint8    ) : mng_retcode; stdcall;

function  mng_putchunk_jdat       (    hHandle            : mng_handle;
                                       iRawlen            : mng_uint32;
                                       pRawdata           : mng_ptr      ) : mng_retcode; stdcall;

function  mng_putchunk_dhdr       (    hHandle            : mng_handle;
                                       iObjectid          : mng_uint16;
                                       iImagetype         : mng_uint8;
                                       iDeltatype         : mng_uint8;
                                       iBlockwidth        : mng_uint32;
                                       iBlockheight       : mng_uint32;
                                       iBlockx            : mng_uint32;
                                       iBlocky            : mng_uint32   ) : mng_retcode; stdcall;

function  mng_putchunk_prom       (    hHandle            : mng_handle;
                                       iColortype         : mng_uint8;
                                       iSampledepth       : mng_uint8;
                                       iFilltype          : mng_uint8    ) : mng_retcode; stdcall;

function  mng_putchunk_pplt       (    hHandle            : mng_handle;
                                       iCount             : mng_uint32   ) : mng_retcode; stdcall;

function  mng_putchunk_pplt_entry (    hHandle            : mng_handle;
                                       iEntry             : mng_uint32;
                                       iRed               : mng_uint16;
                                       iGreen             : mng_uint16;
                                       iBlue              : mng_uint16;
                                       iAlpha             : mng_uint16;
                                       bUsed              : mng_bool     ) : mng_retcode; stdcall;

function  mng_putchunk_drop       (    hHandle            : mng_handle;
                                       iCount             : mng_uint32;
                                       pChunknames        : mng_chunkidp ) : mng_retcode; stdcall;

function  mng_putchunk_dbyk       (    hHandle            : mng_handle;
                                       iChunkname         : mng_chunkid;
                                       iPolarity          : mng_uint8;
                                       iKeywordssize      : mng_uint32;
                                       zKeywords          : mng_pchar    ) : mng_retcode; stdcall;

function  mng_putchunk_ordr       (    hHandle            : mng_handle;
                                       iCount             : mng_uint32   ) : mng_retcode; stdcall;

function  mng_putchunk_ordr_entry (    hHandle            : mng_handle;
                                       iEntry             : mng_uint32;
                                       iChunkname         : mng_chunkid;
                                       iOrdertype         : mng_uint8    ) : mng_retcode; stdcall;

function  mng_putchunk_unknown    (    hHandle            : mng_handle;
                                       iChunkname         : mng_chunkid;
                                       iRawlen            : mng_uint32;
                                       pRawdata           : mng_ptr      ) : mng_retcode; stdcall;

{****************************************************************************}

function  mng_updatemngheader     (    hHandle            : mng_handle;
                                       iFramecount        : mng_uint32;
                                       iLayercount        : mng_uint32;
                                       iPlaytime          : mng_uint32   ) : mng_retcode; stdcall;
                                       
function  mng_updatemngsimplicity (    hHandle            : mng_handle;
                                       iSimplicity        : mng_uint32   ) : mng_retcode; stdcall;

{****************************************************************************}

const MNG_NOERROR          = 0;

      MNG_OUTOFMEMORY      = 1;
      MNG_INVALIDHANDLE    = 2;
      MNG_NOCALLBACK       = 3;
      MNG_UNEXPECTEDEOF    = 4;
      MNG_ZLIBERROR        = 5;
      MNG_JPEGERROR        = 6;
      MNG_LCMSERROR        = 7;
      MNG_NOOUTPUTPROFILE  = 8;
      MNG_NOSRGBPROFILE    = 9;
      MNG_BUFOVERFLOW      = 10;
      MNG_FUNCTIONINVALID  = 11;
      MNG_OUTPUTERROR      = 12;
      MNG_JPEGBUFTOOSMALL  = 13;
      MNG_NEEDMOREDATA     = 14;
      MNG_NEEDTIMERWAIT    = 15;
      MNG_NEEDSECTIONWAIT  = 16;

      MNG_APPIOERROR       = 901;
      MNG_APPTIMERERROR    = 902;
      MNG_APPCMSERROR      = 903;
      MNG_APPMISCERROR     = 904;
      MNG_APPTRACEABORT    = 905;

      MNG_INTERNALERROR    = 999;

      MNG_INVALIDSIG       = 1025;
      MNG_INVALIDCRC       = 1027;
      MNG_INVALIDLENGTH    = 1028;
      MNG_SEQUENCEERROR    = 1029;
      MNG_CHUNKNOTALLOWED  = 1030;
      MNG_MULTIPLEERROR    = 1031;
      MNG_PLTEMISSING      = 1032;
      MNG_IDATMISSING      = 1033;
      MNG_CANNOTBEEMPTY    = 1034;
      MNG_GLOBALLENGTHERR  = 1035;
      MNG_INVALIDBITDEPTH  = 1036;
      MNG_INVALIDCOLORTYPE = 1037;
      MNG_INVALIDCOMPRESS  = 1038;
      MNG_INVALIDFILTER    = 1039;
      MNG_INVALIDINTERLACE = 1040;
      MNG_NOTENOUGHIDAT    = 1041;
      MNG_PLTEINDEXERROR   = 1042;
      MNG_NULLNOTFOUND     = 1043;
      MNG_KEYWORDNULL      = 1044;
      MNG_OBJECTUNKNOWN    = 1045;
      MNG_OBJECTEXISTS     = 1046;
      MNG_TOOMUCHIDAT      = 1047;
      MNG_INVSAMPLEDEPTH   = 1048;
      MNG_INVOFFSETSIZE    = 1049;
      MNG_INVENTRYTYPE     = 1050;
      MNG_ENDWITHNULL      = 1051;
      MNG_INVIMAGETYPE     = 1052;
      MNG_INVDELTATYPE     = 1053;
      MNG_INVALIDINDEX     = 1054;
      MNG_TOOMUCHJDAT      = 1055;
      MNG_JPEGPARMSERR     = 1056;
      MNG_INVFILLMETHOD    = 1057;
      MNG_OBJNOTCONCRETE   = 1058;
      MNG_TARGETNOALPHA    = 1059;
      MNG_MNGTOOCOMPLEX    = 1060;
      MNG_UNKNOWNCRITICAL  = 1061;
      MNG_UNSUPPORTEDNEED  = 1062;
      MNG_INVALIDDELTA     = 1063;
      MNG_INVALIDMETHOD    = 1064;
      MNG_IMPROBABLELENGTH = 1065;
      MNG_INVALIDBLOCK     = 1066;
      MNG_INVALIDEVENT     = 1067;
      MNG_INVALIDMASK      = 1068;
      MNG_NOMATCHINGLOOP   = 1069;
      MNG_SEEKNOTFOUND     = 1070;

      MNG_INVALIDCNVSTYLE  = 2049;
      MNG_WRONGCHUNK       = 2050;
      MNG_INVALIDENTRYIX   = 2051;
      MNG_NOHEADER         = 2052;
      MNG_NOCORRCHUNK      = 2053;
      MNG_NOMHDR           = 2054;

      MNG_IMAGETOOLARGE    = 4097;
      MNG_NOTANANIMATION   = 4098;
      MNG_FRAMENRTOOHIGH   = 4099;
      MNG_LAYERNRTOOHIGH   = 4100;
      MNG_PLAYTIMETOOHIGH  = 4101;
      MNG_FNNOTIMPLEMENTED = 4102;

      MNG_IMAGEFROZEN      = 8193;

{****************************************************************************}

const MNG_CANVAS_RGB8      = $00000000;
      MNG_CANVAS_RGBA8     = $00001000;
      MNG_CANVAS_ARGB8     = $00003000;
      MNG_CANVAS_RGB8_A8   = $00005000;
      MNG_CANVAS_BGR8      = $00000001;
      MNG_CANVAS_BGRX8     = $00010001;
      MNG_CANVAS_BGRA8     = $00001001;
      MNG_CANVAS_ABGR8     = $00003001;
      MNG_CANVAS_RGB16     = $00000100;          { not supported yet }
      MNG_CANVAS_RGBA16    = $00001100;          { not supported yet }
      MNG_CANVAS_ARGB16    = $00003100;          { not supported yet }
      MNG_CANVAS_BGR16     = $00000101;          { not supported yet }
      MNG_CANVAS_BGRA16    = $00001101;          { not supported yet }
      MNG_CANVAS_ABGR16    = $00003101;          { not supported yet }
      MNG_CANVAS_GRAY8     = $00000002;          { not supported yet }
      MNG_CANVAS_GRAY16    = $00000102;          { not supported yet }
      MNG_CANVAS_GRAYA8    = $00001002;          { not supported yet }
      MNG_CANVAS_GRAYA16   = $00001102;          { not supported yet }
      MNG_CANVAS_AGRAY8    = $00003002;          { not supported yet }
      MNG_CANVAS_AGRAY16   = $00003102;          { not supported yet }
      MNG_CANVAS_DX15      = $00000003;          { not supported yet }
      MNG_CANVAS_DX16      = $00000004;          { not supported yet }

{****************************************************************************}

const MNG_UINT_HUH  = $40404040;

      MNG_UINT_BACK = $4241434b;
      MNG_UINT_BASI = $42415349;
      MNG_UINT_CLIP = $434c4950;
      MNG_UINT_CLON = $434c4f4e;
      MNG_UINT_DBYK = $4442594b;
      MNG_UINT_DEFI = $44454649;
      MNG_UINT_DHDR = $44484452;
      MNG_UINT_DISC = $44495343;
      MNG_UINT_DROP = $44524f50;
      MNG_UINT_ENDL = $454e444c;
      MNG_UINT_FRAM = $4652414d;
      MNG_UINT_IDAT = $49444154;
      MNG_UINT_IEND = $49454e44;
      MNG_UINT_IHDR = $49484452;
      MNG_UINT_IJNG = $494a4e47;
      MNG_UINT_IPNG = $49504e47;
      MNG_UINT_JDAT = $4a444154;
      MNG_UINT_JHDR = $4a484452;
      MNG_UINT_JSEP = $4a534550;
      MNG_UINT_LOOP = $4c4f4f50;
      MNG_UINT_MEND = $4d454e44;
      MNG_UINT_MHDR = $4d484452;
      MNG_UINT_MOVE = $4d4f5645;
      MNG_UINT_ORDR = $4f524452;
      MNG_UINT_PAST = $50415354;
      MNG_UINT_PLTE = $504c5445;
      MNG_UINT_PPLT = $50504c54;
      MNG_UINT_PROM = $50524f4d;
      MNG_UINT_SAVE = $53415645;
      MNG_UINT_SEEK = $5345454b;
      MNG_UINT_SHOW = $53484f57;
      MNG_UINT_TERM = $5445524d;
      MNG_UINT_bKGD = $624b4744;
      MNG_UINT_cHRM = $6348524d;
      MNG_UINT_eXPI = $65585049;
      MNG_UINT_fPRI = $66505249;
      MNG_UINT_gAMA = $67414d41;
      MNG_UINT_hIST = $68495354;
      MNG_UINT_iCCP = $69434350;
      MNG_UINT_iTXt = $69545874;
      MNG_UINT_nEED = $6e454544;
      MNG_UINT_oFFs = $6f464673;
      MNG_UINT_pCAL = $7043414c;
      MNG_UINT_pHYg = $70444167;
      MNG_UINT_pHYs = $70485973;
      MNG_UINT_sBIT = $73424954;
      MNG_UINT_sCAL = $7343414c;
      MNG_UINT_sPLT = $73504c54;
      MNG_UINT_sRGB = $73524742;
      MNG_UINT_tEXt = $74455874;
      MNG_UINT_tIME = $74494d45;
      MNG_UINT_tRNS = $74524e53;
      MNG_UINT_zTXt = $7a545874;

      MNG_UINT_evNT = $65764e54;

{****************************************************************************}

implementation

{****************************************************************************}

const mngdll = 'libmng.dll';

{****************************************************************************}

function mng_initialize;           external mngdll;
function mng_reset;                external mngdll;
function mng_cleanup;              external mngdll;

function mng_read;                 external mngdll;
function mng_read_pushdata;        external mngdll;
function mng_read_pushsig;         external mngdll;
function mng_read_pushchunk;       external mngdll;
function mng_read_resume;          external mngdll;
function mng_write;                external mngdll;
function mng_create;               external mngdll;

function mng_readdisplay;          external mngdll;
function mng_display;              external mngdll;
function mng_display_resume;       external mngdll;
function mng_display_freeze;       external mngdll;
function mng_display_reset;        external mngdll;
function mng_display_goframe;      external mngdll;
function mng_display_golayer;      external mngdll;
function mng_display_gotime;       external mngdll;

function mng_trapevent;            external mngdll;

function mng_getlasterror;         external mngdll;

{****************************************************************************}

function mng_setcb_memalloc;       external mngdll;
function mng_setcb_memfree;        external mngdll;
function mng_setcb_releasedata;    external mngdll;

function mng_setcb_openstream;     external mngdll;
function mng_setcb_closestream;    external mngdll;

function mng_setcb_readdata;       external mngdll;

function mng_setcb_writedata;      external mngdll;

function mng_setcb_errorproc;      external mngdll;
function mng_setcb_traceproc;      external mngdll;

function mng_setcb_processheader;  external mngdll;
function mng_setcb_processtext;    external mngdll;

function mng_setcb_getcanvasline;  external mngdll;
function mng_setcb_getalphaline;   external mngdll;
function mng_setcb_getbkgdline;    external mngdll;
function mng_setcb_refresh;        external mngdll;

function mng_setcb_gettickcount;   external mngdll;
function mng_setcb_settimer;       external mngdll;

function mng_setcb_processgamma;   external mngdll;
function mng_setcb_processchroma;  external mngdll;
function mng_setcb_processsrgb;    external mngdll;
function mng_setcb_processiccp;    external mngdll;
function mng_setcb_processarow;    external mngdll;

{****************************************************************************}

function mng_getcb_memalloc;       external mngdll;
function mng_getcb_memfree;        external mngdll;
function mng_getcb_releasedata;    external mngdll;

function mng_getcb_openstream;     external mngdll;
function mng_getcb_closestream;    external mngdll;

function mng_getcb_readdata;       external mngdll;

function mng_getcb_writedata;      external mngdll;

function mng_getcb_errorproc;      external mngdll;
function mng_getcb_traceproc;      external mngdll;

function mng_getcb_processheader;  external mngdll;
function mng_getcb_processtext;    external mngdll;

function mng_getcb_getcanvasline;  external mngdll;
function mng_getcb_getalphaline;   external mngdll;
function mng_getcb_getbkgdline;    external mngdll;
function mng_getcb_refresh;        external mngdll;

function mng_getcb_gettickcount;   external mngdll;
function mng_getcb_settimer;       external mngdll;

function mng_getcb_processgamma;   external mngdll;
function mng_getcb_processchroma;  external mngdll;
function mng_getcb_processsrgb;    external mngdll;
function mng_getcb_processiccp;    external mngdll;
function mng_getcb_processarow;    external mngdll;

{****************************************************************************}

function mng_set_userdata;         external mngdll;

function mng_set_canvasstyle;      external mngdll;
function mng_set_bkgdstyle;        external mngdll;

function mng_set_bgcolor;          external mngdll;
function mng_set_usebkgd;          external mngdll;

function mng_set_storechunks;      external mngdll;
function mng_set_cacheplayback;    external mngdll;

// function mng_set_viewgamma;        external mngdll;
// function mng_set_displaygamma;     external mngdll;
// function mng_set_dfltimggamma;     external mngdll;
function mng_set_viewgammaint;     external mngdll;
function mng_set_displaygammaint;  external mngdll;
function mng_set_dfltimggammaint;  external mngdll;

function mng_set_srgb;             external mngdll;
function mng_set_outputprofile;    external mngdll;
function mng_set_outputprofile2;   external mngdll;
function mng_set_srgbprofile;      external mngdll;
function mng_set_srgbprofile2;     external mngdll;

function mng_set_maxcanvaswidth;   external mngdll;
function mng_set_maxcanvasheight;  external mngdll;
function mng_set_maxcanvassize;    external mngdll;

function mng_set_suspensionmode;   external mngdll;
function mng_set_speed;            external mngdll;
function mng_set_crcmode;          external mngdll;           

{****************************************************************************}

function  mng_get_userdata;        external mngdll;

function  mng_get_sigtype;         external mngdll;
function  mng_get_imagetype;       external mngdll;
function  mng_get_imagewidth;      external mngdll;
function  mng_get_imageheight;     external mngdll;
function  mng_get_ticks;           external mngdll;
function  mng_get_framecount;      external mngdll;
function  mng_get_layercount;      external mngdll;
function  mng_get_playtime;        external mngdll;
function  mng_get_simplicity;      external mngdll;

function  mng_get_canvasstyle;     external mngdll;
function  mng_get_bkgdstyle;       external mngdll;

procedure mng_get_bgcolor;         external mngdll;
function  mng_get_usebkgd;         external mngdll;

function  mng_get_storechunks;     external mngdll;
function  mng_get_cacheplayback;   external mngdll;

// function  mng_get_viewgamma;       external mngdll;
// function  mng_get_displaygamma;    external mngdll;
// function  mng_get_dfltimggamma;    external mngdll;
function  mng_get_viewgammaint;    external mngdll;
function  mng_get_displaygammaint; external mngdll;
function  mng_get_dfltimggammaint; external mngdll;

function  mng_get_srgb;            external mngdll;

function  mng_get_maxcanvaswidth;  external mngdll;
function  mng_get_maxcanvasheight; external mngdll;

function  mng_get_suspensionmode;  external mngdll;

function  mng_get_speed;           external mngdll;
function  mng_get_imagelevel;      external mngdll;

function  mng_get_starttime;       external mngdll;
function  mng_get_runtime;         external mngdll;
function  mng_get_currentframe;    external mngdll;
function  mng_get_currentlayer;    external mngdll;
function  mng_get_currentplaytime; external mngdll;
function  mng_get_totalframes;     external mngdll;
function  mng_get_totallayers;     external mngdll;
function  mng_get_totalplaytime;   external mngdll;

function  mng_status_error;        external mngdll;
function  mng_status_reading;      external mngdll;
function  mng_status_suspendbreak; external mngdll;
function  mng_status_creating;     external mngdll;
function  mng_status_writing;      external mngdll;
function  mng_status_displaying;   external mngdll;
function  mng_status_running;      external mngdll;
function  mng_status_timerbreak;   external mngdll;
function  mng_status_dynamic;      external mngdll;

{****************************************************************************}

function  mng_iterate_chunks;      external mngdll;

{****************************************************************************}

function  mng_getchunk_ihdr;       external mngdll;
function  mng_getchunk_plte;       external mngdll;
function  mng_getchunk_idat;       external mngdll;
function  mng_getchunk_trns;       external mngdll;
function  mng_getchunk_gama;       external mngdll;
function  mng_getchunk_chrm;       external mngdll;
function  mng_getchunk_srgb;       external mngdll;
function  mng_getchunk_iccp;       external mngdll;
function  mng_getchunk_text;       external mngdll;
function  mng_getchunk_ztxt;       external mngdll;
function  mng_getchunk_itxt;       external mngdll;
function  mng_getchunk_bkgd;       external mngdll;
function  mng_getchunk_phys;       external mngdll;
function  mng_getchunk_sbit;       external mngdll;
function  mng_getchunk_splt;       external mngdll;
function  mng_getchunk_hist;       external mngdll;
function  mng_getchunk_time;       external mngdll;
function  mng_getchunk_mhdr;       external mngdll;
function  mng_getchunk_loop;       external mngdll;
function  mng_getchunk_endl;       external mngdll;
function  mng_getchunk_defi;       external mngdll;
function  mng_getchunk_basi;       external mngdll;
function  mng_getchunk_clon;       external mngdll;
function  mng_getchunk_past;       external mngdll;
function  mng_getchunk_past_src;   external mngdll;
function  mng_getchunk_disc;       external mngdll;
function  mng_getchunk_back;       external mngdll;
function  mng_getchunk_fram;       external mngdll;
function  mng_getchunk_move;       external mngdll;
function  mng_getchunk_clip;       external mngdll;
function  mng_getchunk_show;       external mngdll;
function  mng_getchunk_term;       external mngdll;
function  mng_getchunk_save;       external mngdll;
function  mng_getchunk_save_entry; external mngdll;
function  mng_getchunk_seek;       external mngdll;
function  mng_getchunk_expi;       external mngdll;
function  mng_getchunk_fpri;       external mngdll;
function  mng_getchunk_need;       external mngdll;
function  mng_getchunk_phyg;       external mngdll;
function  mng_getchunk_jhdr;       external mngdll;
function  mng_getchunk_jdat;       external mngdll;
function  mng_getchunk_dhdr;       external mngdll;
function  mng_getchunk_prom;       external mngdll;
function  mng_getchunk_pplt;       external mngdll;
function  mng_getchunk_pplt_entry; external mngdll;
function  mng_getchunk_drop;       external mngdll;
function  mng_getchunk_dbyk;       external mngdll;
function  mng_getchunk_ordr;       external mngdll;
function  mng_getchunk_ordr_entry; external mngdll;
function  mng_getchunk_unknown;    external mngdll;

{****************************************************************************}

function  mng_putchunk_ihdr;       external mngdll;
function  mng_putchunk_plte;       external mngdll;
function  mng_putchunk_idat;       external mngdll;
function  mng_putchunk_iend;       external mngdll;
function  mng_putchunk_trns;       external mngdll;
function  mng_putchunk_gama;       external mngdll;
function  mng_putchunk_chrm;       external mngdll;
function  mng_putchunk_srgb;       external mngdll;
function  mng_putchunk_iccp;       external mngdll;
function  mng_putchunk_text;       external mngdll;
function  mng_putchunk_ztxt;       external mngdll;
function  mng_putchunk_itxt;       external mngdll;
function  mng_putchunk_bkgd;       external mngdll;
function  mng_putchunk_phys;       external mngdll;
function  mng_putchunk_sbit;       external mngdll;
function  mng_putchunk_splt;       external mngdll;
function  mng_putchunk_hist;       external mngdll;
function  mng_putchunk_time;       external mngdll;
function  mng_putchunk_mhdr;       external mngdll;
function  mng_putchunk_mend;       external mngdll;
function  mng_putchunk_loop;       external mngdll;
function  mng_putchunk_endl;       external mngdll;
function  mng_putchunk_defi;       external mngdll;
function  mng_putchunk_basi;       external mngdll;
function  mng_putchunk_clon;       external mngdll;
function  mng_putchunk_past;       external mngdll;
function  mng_putchunk_past_src;   external mngdll;
function  mng_putchunk_disc;       external mngdll;
function  mng_putchunk_back;       external mngdll;
function  mng_putchunk_fram;       external mngdll;
function  mng_putchunk_move;       external mngdll;
function  mng_putchunk_clip;       external mngdll;
function  mng_putchunk_show;       external mngdll;
function  mng_putchunk_term;       external mngdll;
function  mng_putchunk_save;       external mngdll;
function  mng_putchunk_save_entry; external mngdll;
function  mng_putchunk_seek;       external mngdll;
function  mng_putchunk_expi;       external mngdll;
function  mng_putchunk_fpri;       external mngdll;
function  mng_putchunk_need;       external mngdll;
function  mng_putchunk_phyg;       external mngdll;
function  mng_putchunk_jhdr;       external mngdll;
function  mng_putchunk_jdat;       external mngdll;
function  mng_putchunk_dhdr;       external mngdll;
function  mng_putchunk_prom;       external mngdll;
function  mng_putchunk_pplt;       external mngdll;
function  mng_putchunk_pplt_entry; external mngdll;
function  mng_putchunk_drop;       external mngdll;
function  mng_putchunk_dbyk;       external mngdll;
function  mng_putchunk_ordr;       external mngdll;
function  mng_putchunk_ordr_entry; external mngdll;
function  mng_putchunk_unknown;    external mngdll;

{****************************************************************************}

function  mng_updatemngheader;     external mngdll;
function  mng_updatemngsimplicity; external mngdll;

{****************************************************************************}

end.
