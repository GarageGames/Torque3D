//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------
#ifndef _GFXSTRINGENUMTRANSLATE_H_
#define _GFXSTRINGENUMTRANSLATE_H_

#ifndef _PLATFORM_H_
   #include "platform/platform.h"
#endif
#ifndef _GFXENUMS_H_
   #include "gfx/gfxEnums.h"
#endif
#ifndef _CONSOLE_H_
   #include "console/console.h"
#endif
#ifndef _DYNAMIC_CONSOLETYPES_H_
   #include "console/dynamicTypes.h"
#endif
#ifndef _GFXAPI_H_
   #include "gfx/gfxAPI.h"
#endif


//------------------------------------------------------------------------------

namespace GFXStringEnumTranslate
{
   void init();
};

//------------------------------------------------------------------------------

extern const char *GFXStringIndexFormat[GFXIndexFormat_COUNT];
extern const char *GFXStringSamplerState[GFXSAMP_COUNT];
extern const char *GFXStringTextureFormat[GFXFormat_COUNT];
extern const char *GFXStringRenderState[GFXRenderState_COUNT];
extern const char *GFXStringTextureFilter[GFXTextureFilter_COUNT];
extern const char *GFXStringBlend[GFXBlend_COUNT];
extern const char *GFXStringBlendOp[GFXBlendOp_COUNT];
extern const char *GFXStringStencilOp[GFXStencilOp_COUNT];
extern const char *GFXStringCmpFunc[GFXCmp_COUNT];
extern const char *GFXStringCullMode[GFXCull_COUNT];
extern const char *GFXStringPrimType[GFXPT_COUNT];
extern const char *GFXStringTextureStageState[GFXTSS_COUNT];
extern const char *GFXStringTextureAddress[GFXAddress_COUNT];
extern const char *GFXStringTextureOp[GFXTOP_COUNT];
extern const char *GFXStringFillMode[GFXFill_COUNT];

typedef const char *(*StringValueLookupFn)( const U32 &value );

extern StringValueLookupFn GFXStringRenderStateValueLookup[GFXRenderState_COUNT];
extern StringValueLookupFn GFXStringSamplerStateValueLookup[GFXSAMP_COUNT];
extern StringValueLookupFn GFXStringTextureStageStateValueLookup[GFXTSS_COUNT];

#define GFXREVERSE_LOOKUP( tablearray, enumprefix, val ) \
   for( S32 i = enumprefix##_FIRST; i < enumprefix##_COUNT; i++ ) \
      if( (S32)tablearray##[i] == val ) \
      { \
         val = i; \
         break; \
      } \


// TODOS!
extern EnumTable gTextureArgumentEnumTable_M;
//extern EnumTable srcBlendFactorTable;
//extern EnumTable dstBlendFactorTable;
//extern EnumTable::Enums srcBlendFactorLookup[9];
//extern EnumTable::Enums dstBlendFactorLookup[9];

#endif