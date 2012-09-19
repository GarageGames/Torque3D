//---------------------------------------------------------------------------
#include <vcl.h>
#include "Main.h"
#include "libmng.h"
#pragma hdrstop

#ifndef CallbackH
#define CallbackH

//---------------------------------------------------------------------------
extern mng_ptr __stdcall myalloc( mng_size_t iSize );
//---------------------------------------------------------------------------
extern void __stdcall myfree( mng_ptr pPtr, mng_size_t iSize );
//---------------------------------------------------------------------------
extern mng_bool __stdcall myreaddata( mng_handle hMNG,
                                      mng_ptr    pBuf,
                                      mng_uint32 iSize,
                                      mng_uint32 *iRead );
//---------------------------------------------------------------------------
extern mng_bool __stdcall ProcessHeader(  mng_handle hHandle,
                                          mng_uint32 iWidth,
                                          mng_uint32 iHeight );
//---------------------------------------------------------------------------
extern mng_bool __stdcall myopenstream( mng_handle hMNG );
//---------------------------------------------------------------------------
extern mng_bool __stdcall myclosestream( mng_handle hMNG );
//---------------------------------------------------------------------------
extern mng_bool __stdcall myiterchunk ( mng_handle  hMNG,
                                        mng_handle  hChunk,
                                        mng_chunkid iChunktype,
                                        mng_uint32  iChunkseq );
//---------------------------------------------------------------------------
extern mng_bool __stdcall mytraceproc(  mng_handle  hHandle,
                                        mng_int32   iFuncnr,
                                        mng_int32   iFuncseq,
                                        mng_pchar   zFuncname );
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#endif
