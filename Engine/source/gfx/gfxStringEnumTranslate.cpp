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

#include "core/strings/stringFunctions.h"

#include "gfx/gfxStringEnumTranslate.h"
#include "gfx/gfxAPI.h"
#include "console/console.h"

//------------------------------------------------------------------------------

const char *GFXStringIndexFormat[GFXIndexFormat_COUNT];
const char *GFXStringSamplerState[GFXSAMP_COUNT];
const char *GFXStringTextureFormat[GFXFormat_COUNT];
const char *GFXStringTiledTextureFormat[GFXFormat_COUNT];
const char *GFXStringRenderTargetFormat[GFXFormat_COUNT];
const char *GFXStringRenderState[GFXRenderState_COUNT];
const char *GFXStringTextureFilter[GFXTextureFilter_COUNT];
const char *GFXStringBlend[GFXBlend_COUNT];
const char *GFXStringBlendOp[GFXBlendOp_COUNT];
const char *GFXStringStencilOp[GFXStencilOp_COUNT];
const char *GFXStringCmpFunc[GFXCmp_COUNT];
const char *GFXStringCullMode[GFXCull_COUNT];
const char *GFXStringPrimType[GFXPT_COUNT];
const char *GFXStringTextureStageState[GFXTSS_COUNT];
const char *GFXStringTextureAddress[GFXAddress_COUNT];
const char *GFXStringTextureOp[GFXTOP_COUNT];
const char *GFXStringFillMode[GFXFill_COUNT];

StringValueLookupFn GFXStringRenderStateValueLookup[GFXRenderState_COUNT];
StringValueLookupFn GFXStringSamplerStateValueLookup[GFXSAMP_COUNT];
StringValueLookupFn GFXStringTextureStageStateValueLookup[GFXTSS_COUNT];

//------------------------------------------------------------------------------

const char *defaultStringValueLookup( const U32 &value )
{
   static char retbuffer[256];

   dSprintf( retbuffer, sizeof( retbuffer ), "%d", value );

   return retbuffer;
}

#define _STRING_VALUE_LOOKUP_FXN( table ) \
   const char * table##_lookup( const U32 &value ) { return table[value]; }

_STRING_VALUE_LOOKUP_FXN(GFXStringTextureAddress);
_STRING_VALUE_LOOKUP_FXN(GFXStringTextureFilter);
_STRING_VALUE_LOOKUP_FXN(GFXStringBlend);
_STRING_VALUE_LOOKUP_FXN(GFXStringTextureOp);
_STRING_VALUE_LOOKUP_FXN(GFXStringCmpFunc);
_STRING_VALUE_LOOKUP_FXN(GFXStringStencilOp);
_STRING_VALUE_LOOKUP_FXN(GFXStringCullMode);
_STRING_VALUE_LOOKUP_FXN(GFXStringBlendOp);

//------------------------------------------------------------------------------

#define INIT_LOOKUPTABLE( tablearray, enumprefix, type ) \
   for( S32 i = enumprefix##_FIRST; i < enumprefix##_COUNT; i++ ) \
      tablearray[i] = (type)GFX_UNINIT_VAL;
#define INIT_LOOKUPTABLE_EX( tablearray, enumprefix, type, typeTable ) \
   for( S32 i = enumprefix##_FIRST; i < enumprefix##_COUNT; i++ ) \
   {\
      tablearray[i] = (type)GFX_UNINIT_VAL;\
      typeTable[i] = &defaultStringValueLookup;\
   }

#define VALIDATE_LOOKUPTABLE( tablearray, enumprefix ) \
   for( S32 i = enumprefix##_FIRST; i < enumprefix##_COUNT; i++ ) \
      if( (intptr_t)tablearray[i] == GFX_UNINIT_VAL ) \
         Con::warnf( "GFXStringEnumTranslate: Unassigned value in " #tablearray ": %i", i ); \
      else if( (intptr_t)tablearray[i] == GFX_UNSUPPORTED_VAL ) \
         Con::warnf( "GFXStringEnumTranslate: Unsupported value in " #tablearray ": %i", i );

//------------------------------------------------------------------------------

#define GFX_STRING_ASSIGN_MACRO( table, indexEnum ) table[indexEnum] = #indexEnum;
#define GFX_STRING_ASSIGN_MACRO_EX( table, indexEnum, typeTable ) table[indexEnum] = #indexEnum; table##ValueLookup[indexEnum] = &typeTable##_lookup;

void GFXStringEnumTranslate::init()
{
   static bool sInitCalled = false;

   if( sInitCalled )
      return;

   sInitCalled = true;

   INIT_LOOKUPTABLE( GFXStringIndexFormat, GFXIndexFormat, const char * );
   GFX_STRING_ASSIGN_MACRO( GFXStringIndexFormat, GFXIndexFormat16 );
   GFX_STRING_ASSIGN_MACRO( GFXStringIndexFormat, GFXIndexFormat32 );
   VALIDATE_LOOKUPTABLE( GFXStringIndexFormat, GFXIndexFormat );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE_EX( GFXStringSamplerState, GFXSAMP, const char *, GFXStringSamplerStateValueLookup );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringSamplerState, GFXSAMPAddressU, GFXStringTextureAddress );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringSamplerState, GFXSAMPAddressV, GFXStringTextureAddress );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringSamplerState, GFXSAMPAddressW, GFXStringTextureAddress );
   GFX_STRING_ASSIGN_MACRO( GFXStringSamplerState, GFXSAMPBorderColor );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringSamplerState, GFXSAMPMagFilter, GFXStringTextureFilter );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringSamplerState, GFXSAMPMinFilter, GFXStringTextureFilter );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringSamplerState, GFXSAMPMipFilter, GFXStringTextureFilter );
   GFX_STRING_ASSIGN_MACRO( GFXStringSamplerState, GFXSAMPMipMapLODBias );
   GFX_STRING_ASSIGN_MACRO( GFXStringSamplerState, GFXSAMPMaxMipLevel );
   GFX_STRING_ASSIGN_MACRO( GFXStringSamplerState, GFXSAMPMaxAnisotropy );

   GFX_STRING_ASSIGN_MACRO( GFXStringSamplerState, GFXSAMPSRGBTexture );
   GFX_STRING_ASSIGN_MACRO( GFXStringSamplerState, GFXSAMPElementIndex );
   GFX_STRING_ASSIGN_MACRO( GFXStringSamplerState, GFXSAMPDMapOffset );

   VALIDATE_LOOKUPTABLE( GFXStringSamplerState, GFXSAMP );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXStringTextureFormat, GFXFormat, const char * );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatR8G8B8 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatR8G8B8A8 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatR8G8B8X8 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatB8G8R8A8 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatR32F );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatR5G6B5 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatR5G5B5A1 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatR5G5B5X1 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatA4L4 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatA8L8 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatA8 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatL8 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatDXT1 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatDXT2 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatDXT3 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatDXT4 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatDXT5 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatD32 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatD24X8 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatD24S8 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatD24FS8 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatD16 );

   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatR32G32B32A32F );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatR16G16B16A16F );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatL16 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatR16G16B16A16 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatR16G16 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatR16F );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatR16G16F );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFormat, GFXFormatR10G10B10A2 );
   VALIDATE_LOOKUPTABLE( GFXStringTextureFormat, GFXFormat);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE_EX( GFXStringRenderState, GFXRenderState, const char *, GFXStringRenderStateValueLookup );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSZEnable );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSFillMode );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSZWriteEnable );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSAlphaTestEnable );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringRenderState, GFXRSSrcBlend, GFXStringBlend );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringRenderState, GFXRSDestBlend, GFXStringBlend );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringRenderState, GFXRSCullMode, GFXStringCullMode );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringRenderState, GFXRSZFunc, GFXStringCmpFunc );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSAlphaRef );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringRenderState, GFXRSAlphaFunc, GFXStringCmpFunc );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSAlphaBlendEnable );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSStencilEnable );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringRenderState, GFXRSStencilFail, GFXStringStencilOp );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringRenderState, GFXRSStencilZFail, GFXStringStencilOp );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringRenderState, GFXRSStencilPass, GFXStringStencilOp );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringRenderState, GFXRSStencilFunc, GFXStringCmpFunc );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSStencilRef );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSStencilMask );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSStencilWriteMask );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap0 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap1 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap2 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap3 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap4 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap5 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap6 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap7 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSClipPlaneEnable );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSPointSize );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSPointSizeMin );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSPointSize_Max );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSPointSpriteEnable );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSMultiSampleantiAlias );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSMultiSampleMask );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSShadeMode );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSLastPixel );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSClipping );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSPointScaleEnable );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSPointScale_A );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSPointScale_B );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSPointScale_C );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSLighting );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSAmbient );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSFogVertexMode );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSColorVertex );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSLocalViewer );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSNormalizeNormals );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSDiffuseMaterialSource );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSSpecularMaterialSource );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSAmbientMaterialSource );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSEmissiveMaterialSource );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSVertexBlend );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSFogEnable );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSSpecularEnable );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSFogColor );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSFogTableMode );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSFogStart );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSFogEnd );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSFogDensity );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSRangeFogEnable );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSDebugMonitorToken );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSIndexedVertexBlendEnable );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSTweenFactor );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSTextureFactor );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSPatchEdgeStyle );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSDitherEnable );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSColorWriteEnable );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringRenderState, GFXRSBlendOp, GFXStringBlendOp );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSPositionDegree );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSNormalDegree );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSAntiAliasedLineEnable );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSAdaptiveTess_X );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSAdaptiveTess_Y );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSdaptiveTess_Z );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSAdaptiveTess_W );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSEnableAdaptiveTesselation );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSScissorTestEnable );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSSlopeScaleDepthBias );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSMinTessellationLevel );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSMaxTessellationLevel );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSTwoSidedStencilMode );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSCCWStencilFail );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSCCWStencilZFail );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSCCWStencilPass );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSCCWStencilFunc );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSColorWriteEnable1 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSColorWriteEnable2 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSolorWriteEnable3 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSBlendFactor );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSSRGBWriteEnable );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSDepthBias );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap8 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap9 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap10 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap11 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap12 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap13 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap14 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSWrap15 );
   GFX_STRING_ASSIGN_MACRO( GFXStringRenderState, GFXRSSeparateAlphaBlendEnable );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringRenderState, GFXRSSrcBlendAlpha, GFXStringBlend );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringRenderState, GFXRSDestBlendAlpha, GFXStringBlend );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringRenderState, GFXRSBlendOpAlpha, GFXStringBlendOp );

   VALIDATE_LOOKUPTABLE( GFXStringRenderState, GFXRenderState );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXStringTextureFilter, GFXTextureFilter, const char * );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFilter, GFXTextureFilterNone );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFilter, GFXTextureFilterPoint );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFilter, GFXTextureFilterLinear );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFilter, GFXTextureFilterAnisotropic );

   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFilter, GFXTextureFilterPyramidalQuad );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureFilter, GFXTextureFilterGaussianQuad );

   VALIDATE_LOOKUPTABLE( GFXStringTextureFilter, GFXTextureFilter );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXStringBlend, GFXBlend, const char * );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlend, GFXBlendZero );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlend, GFXBlendOne );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlend, GFXBlendSrcColor );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlend, GFXBlendInvSrcColor );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlend, GFXBlendSrcAlpha );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlend, GFXBlendInvSrcAlpha );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlend, GFXBlendDestAlpha );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlend, GFXBlendInvDestAlpha );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlend, GFXBlendDestColor );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlend, GFXBlendInvDestColor );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlend, GFXBlendSrcAlphaSat );
   VALIDATE_LOOKUPTABLE( GFXStringBlend, GFXBlend );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXStringBlendOp, GFXBlendOp, const char * );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlendOp, GFXBlendOpAdd );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlendOp, GFXBlendOpSubtract );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlendOp, GFXBlendOpRevSubtract );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlendOp, GFXBlendOpMin );
   GFX_STRING_ASSIGN_MACRO( GFXStringBlendOp, GFXBlendOpMax );
   VALIDATE_LOOKUPTABLE( GFXStringBlendOp, GFXBlendOp );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXStringStencilOp, GFXStencilOp, const char * );
   GFX_STRING_ASSIGN_MACRO( GFXStringStencilOp, GFXStencilOpKeep );
   GFX_STRING_ASSIGN_MACRO( GFXStringStencilOp, GFXStencilOpZero );
   GFX_STRING_ASSIGN_MACRO( GFXStringStencilOp, GFXStencilOpReplace );
   GFX_STRING_ASSIGN_MACRO( GFXStringStencilOp, GFXStencilOpIncrSat );
   GFX_STRING_ASSIGN_MACRO( GFXStringStencilOp, GFXStencilOpDecrSat );
   GFX_STRING_ASSIGN_MACRO( GFXStringStencilOp, GFXStencilOpInvert );
   GFX_STRING_ASSIGN_MACRO( GFXStringStencilOp, GFXStencilOpIncr );
   GFX_STRING_ASSIGN_MACRO( GFXStringStencilOp, GFXStencilOpDecr );
   VALIDATE_LOOKUPTABLE( GFXStringStencilOp, GFXStencilOp );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXStringCmpFunc, GFXCmp, const char * );
   GFX_STRING_ASSIGN_MACRO( GFXStringCmpFunc, GFXCmpNever );
   GFX_STRING_ASSIGN_MACRO( GFXStringCmpFunc, GFXCmpLess );
   GFX_STRING_ASSIGN_MACRO( GFXStringCmpFunc, GFXCmpEqual );
   GFX_STRING_ASSIGN_MACRO( GFXStringCmpFunc, GFXCmpLessEqual );
   GFX_STRING_ASSIGN_MACRO( GFXStringCmpFunc, GFXCmpGreater );
   GFX_STRING_ASSIGN_MACRO( GFXStringCmpFunc, GFXCmpNotEqual );
   GFX_STRING_ASSIGN_MACRO( GFXStringCmpFunc, GFXCmpGreaterEqual );
   GFX_STRING_ASSIGN_MACRO( GFXStringCmpFunc, GFXCmpAlways );
   VALIDATE_LOOKUPTABLE( GFXStringCmpFunc, GFXCmp );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXStringCullMode, GFXCull, const char * );
   GFX_STRING_ASSIGN_MACRO( GFXStringCullMode, GFXCullNone );
   GFX_STRING_ASSIGN_MACRO( GFXStringCullMode, GFXCullCW );
   GFX_STRING_ASSIGN_MACRO( GFXStringCullMode, GFXCullCCW );
   VALIDATE_LOOKUPTABLE( GFXStringCullMode, GFXCull );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXStringPrimType, GFXPT, const char * );
   GFX_STRING_ASSIGN_MACRO( GFXStringPrimType, GFXPointList );
   GFX_STRING_ASSIGN_MACRO( GFXStringPrimType, GFXLineList );
   GFX_STRING_ASSIGN_MACRO( GFXStringPrimType, GFXLineStrip );
   GFX_STRING_ASSIGN_MACRO( GFXStringPrimType, GFXTriangleList );
   GFX_STRING_ASSIGN_MACRO( GFXStringPrimType, GFXTriangleStrip );
   VALIDATE_LOOKUPTABLE( GFXStringPrimType, GFXPT );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE_EX( GFXStringTextureStageState, GFXTSS, const char *, GFXStringTextureStageStateValueLookup );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringTextureStageState, GFXTSSColorOp, GFXStringTextureOp );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSColorArg1 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSColorArg2 );
   GFX_STRING_ASSIGN_MACRO_EX( GFXStringTextureStageState, GFXTSSAlphaOp, GFXStringTextureOp );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSAlphaArg1 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSAlphaArg2 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSBumpEnvMat00 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSBumpEnvMat01 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSBumpEnvMat10 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSBumpEnvMat11 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSTexCoordIndex );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSBumpEnvlScale );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSBumpEnvlOffset );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSTextureTransformFlags );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSColorArg0 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSAlphaArg0 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSResultArg );

   GFX_STRING_ASSIGN_MACRO( GFXStringTextureStageState, GFXTSSConstant );
   VALIDATE_LOOKUPTABLE( GFXStringTextureStageState, GFXTSS );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXStringTextureAddress, GFXAddress, const char * );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureAddress, GFXAddressWrap );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureAddress, GFXAddressMirror );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureAddress, GFXAddressClamp );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureAddress, GFXAddressBorder );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureAddress, GFXAddressMirrorOnce );
   VALIDATE_LOOKUPTABLE(GFXStringTextureAddress, GFXAddress );
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
   INIT_LOOKUPTABLE( GFXStringTextureOp, GFXTOP, const char * );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPDisable );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPSelectARG1 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPSelectARG2 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPModulate );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPModulate2X );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPModulate4X );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPAdd );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPAddSigned );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPAddSigned2X );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPSubtract );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPAddSmooth );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPBlendDiffuseAlpha );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPBlendTextureAlpha );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPBlendFactorAlpha );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPBlendTextureAlphaPM );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPBlendCURRENTALPHA );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPPreModulate );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPModulateAlphaAddColor );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPModulateColorAddAlpha );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPModulateInvAlphaAddColor );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPModulateInvColorAddAlpha );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPBumpEnvMap );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPBumpEnvMapLuminance );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPDotProduct3 );
   GFX_STRING_ASSIGN_MACRO( GFXStringTextureOp, GFXTOPLERP );
   VALIDATE_LOOKUPTABLE( GFXStringTextureOp, GFXTOP );

   INIT_LOOKUPTABLE( GFXStringFillMode, GFXFill, const char * );
   GFX_STRING_ASSIGN_MACRO( GFXStringFillMode, GFXFillPoint );
   GFX_STRING_ASSIGN_MACRO( GFXStringFillMode, GFXFillWireframe );
   GFX_STRING_ASSIGN_MACRO( GFXStringFillMode, GFXFillSolid );
   VALIDATE_LOOKUPTABLE( GFXStringFillMode, GFXFill );
}
