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

#include "gfx/gfxAPI.h"


IMPLEMENT_SCOPE( GFXAPI, GFX,,
   "Graphics subystem." );

IMPLEMENT_STRUCT( GFXVideoMode,
   GFXVideoMode, GFXAPI,
   "Descriptor for a specific video mode." )
   FIELD( resolution, resolution, 1, "Width and height of the mode's resolution in pixels." )
   FIELD( bitDepth, bitDepth, 1, "Bits per pixel." )
   FIELD( refreshRate, refreshRate, 1, "Frequency at which the screen is refreshed (in Hertz)." )
   FIELD( fullScreen, fullScreen, 1, "Whether this is a fullscreen or windowed mode." )
   FIELD( wideScreen, wideScreen, 1, "Whether this is a widescreen display mode." )
   FIELD( antialiasLevel, antialiasLevel, 1, "Maximum or desired antialiasing level." )
END_IMPLEMENT_STRUCT;

ImplementEnumType( GFXAdapterType,
   "Back-end graphics API used by the GFX subsystem.\n\n"
   "@ingroup GFX" )
   { OpenGL, "OpenGL", "OpenGL." },
   { Direct3D8, "D3D8", "Direct3D 8." },
   { Direct3D9, "D3D9", "Direct3D 9." },
   { NullDevice, "NullDevice", "Null device for dedicated servers." },
   { Direct3D9_360, "Xenon", "Direct3D 9 on Xbox 360." }
EndImplementEnumType;

ImplementEnumType( GFXBlend,
   "The supported blend modes.\n"
   "@ingroup GFX" )

   { GFXBlendZero, "GFXBlendZero", "(0, 0, 0, 0)" },
   { GFXBlendOne, "GFXBlendOne", "(1, 1, 1, 1)" },
   { GFXBlendSrcColor, "GFXBlendSrcColor", "(Rs, Gs, Bs, As)" },
   { GFXBlendInvSrcColor, "GFXBlendInvSrcColor", "(1 - Rs, 1 - Gs, 1 - Bs, 1 - As)" },
   { GFXBlendSrcAlpha, "GFXBlendSrcAlpha", "(As, As, As, As)" },
   { GFXBlendInvSrcAlpha, "GFXBlendInvSrcAlpha", "( 1 - As, 1 - As, 1 - As, 1 - As)" },
   { GFXBlendDestAlpha, "GFXBlendDestAlpha", "(Ad Ad Ad Ad)" },
   { GFXBlendInvDestAlpha, "GFXBlendInvDestAlpha", "(1 - Ad 1 - Ad 1 - Ad 1 - Ad)" },
   { GFXBlendDestColor, "GFXBlendDestColor", "(Rd, Gd, Bd, Ad)" },
   { GFXBlendInvDestColor, "GFXBlendInvDestColor", "(1 - Rd, 1 - Gd, 1 - Bd, 1 - Ad)" },
   { GFXBlendSrcAlphaSat, "GFXBlendSrcAlphaSat", "(f, f, f, 1) where f = min(As, 1 - Ad)" }

EndImplementEnumType;

ImplementEnumType( GFXCmpFunc,
   "The supported comparison functions.\n"
   "@ingroup GFX" )

   { GFXCmpNever, "GFXCmpNever" },
   { GFXCmpLess, "GFXCmpLess" },
   { GFXCmpEqual, "GFXCmpEqual" },
   { GFXCmpLessEqual, "GFXCmpLessEqual" },
   { GFXCmpGreater, "GFXCmpGreater" },
   { GFXCmpNotEqual, "GFXCmpNotEqual" },
   { GFXCmpGreaterEqual, "GFXCmpGreaterEqual" },
   { GFXCmpAlways, "GFXCmpAlways" },

EndImplementEnumType;

ImplementEnumType( GFXTextureAddressMode,
   "The texture address modes.\n"
   "@ingroup GFX" )

   { GFXAddressWrap,          "GFXAddressWrap" },
   { GFXAddressMirror,        "GFXAddressMirror" },
   { GFXAddressClamp,         "GFXAddressClamp" },
   { GFXAddressBorder,        "GFXAddressBorder" },
   { GFXAddressMirrorOnce,    "GFXAddressMirrorOnce" }

EndImplementEnumType;

ImplementEnumType( GFXTextureFilterType,
   "The texture filter types.\n"
   "@ingroup GFX" )

   { GFXTextureFilterNone,    "GFXTextureFilterNone" },
   { GFXTextureFilterPoint,   "GFXTextureFilterPoint" },
   { GFXTextureFilterLinear,  "GFXTextureFilterLinear" },
   { GFXTextureFilterAnisotropic, "GFXTextureFilterAnisotropic" },
   { GFXTextureFilterPyramidalQuad, "GFXTextureFilterPyramidalQuad" },
   { GFXTextureFilterGaussianQuad, "GFXTextureFilterGaussianQuad" }

EndImplementEnumType;

ImplementEnumType( GFXTextureOp,
   "The texture operators.\n"
   "@ingroup GFX" )

   { GFXTOPDisable, "GFXTOPDisable" },
   { GFXTOPSelectARG1, "GFXTOPSelectARG1" },
   { GFXTOPSelectARG2, "GFXTOPSelectARG2" },
   { GFXTOPModulate, "GFXTOPModulate" },
   { GFXTOPModulate2X, "GFXTOPModulate2X" },
   { GFXTOPModulate4X, "GFXTOPModulate4X" },
   { GFXTOPAdd, "GFXTOPAdd" },
   { GFXTOPAddSigned, "GFXTOPAddSigned" },
   { GFXTOPAddSigned2X, "GFXTOPAddSigned2X" },
   { GFXTOPSubtract, "GFXTOPSubtract" },
   { GFXTOPAddSmooth, "GFXTOPAddSmooth" }, 
   { GFXTOPBlendDiffuseAlpha, "GFXTOPBlendDiffuseAlpha" },
   { GFXTOPBlendTextureAlpha, "GFXTOPBlendTextureAlpha" },
   { GFXTOPBlendFactorAlpha, "GFXTOPBlendFactorAlpha" },
   { GFXTOPBlendTextureAlphaPM, "GFXTOPBlendTextureAlphaPM" },
   { GFXTOPBlendCURRENTALPHA, "GFXTOPBlendCURRENTALPHA" },
   { GFXTOPPreModulate, "GFXTOPPreModulate" },
   { GFXTOPModulateAlphaAddColor, "GFXTOPModulateAlphaAddColor" },
   { GFXTOPModulateColorAddAlpha, "GFXTOPModulateColorAddAlpha" },
   { GFXTOPModulateInvAlphaAddColor, "GFXTOPModulateInvAlphaAddColor" },
   { GFXTOPModulateInvColorAddAlpha, "GFXTOPModulateInvColorAddAlpha" },
   { GFXTOPBumpEnvMap, "GFXTOPBumpEnvMap" },
   { GFXTOPBumpEnvMapLuminance, "GFXTOPBumpEnvMapLuminance" },
   { GFXTOPDotProduct3, "GFXTOPDotProduct3" },
   { GFXTOPLERP, "GFXTOPLERP" }

EndImplementEnumType;

ImplementEnumType( GFXTextureArgument,
   "The texture arguments.\n"
   "@ingroup GFX" )

   { GFXTADiffuse, "GFXTADiffuse" },
   { GFXTACurrent, "GFXTACurrent" },
   { GFXTATexture, "GFXTATexture" },
   { GFXTATFactor, "GFXTATFactor" },
   { GFXTASpecular, "GFXTASpecular" },
   { GFXTATemp, "GFXTATemp" },
   { GFXTAConstant, "GFXTAConstant" },

   { GFXTAComplement, "OneMinus" },
   { GFXTAAlphaReplicate, "AlphaReplicate" }

EndImplementEnumType;

ImplementEnumType( GFXTextureTransformFlags,
   "The texture transform state flags.\n"
   "@ingroup GFX" )

   { GFXTTFFDisable, "GFXTTFDisable" },
   { GFXTTFFCoord1D, "GFXTTFFCoord1D" },
   { GFXTTFFCoord2D, "GFXTTFFCoord2D" },
   { GFXTTFFCoord3D, "GFXTTFFCoord3D" },
   { GFXTTFFCoord4D, "GFXTTFFCoord4D" },
   { GFXTTFFProjected, "GFXTTFProjected" }

EndImplementEnumType;


ImplementEnumType( GFXFormat,
   "The texture formats.\n"
   "@note Not all formats are supported on all platforms.\n"
   "@ingroup GFX" )

   { GFXFormatR8G8B8, "GFXFormatR8G8B8" },
   { GFXFormatR8G8B8A8, "GFXFormatR8G8B8A8" },
   { GFXFormatR8G8B8X8, "GFXFormatR8G8B8X8" },
   { GFXFormatR32F, "GFXFormatR32F" },
   { GFXFormatR5G6B5, "GFXFormatR5G6B5" },
   { GFXFormatR5G5B5A1, "GFXFormatR5G5B5A1" },
   { GFXFormatR5G5B5X1, "GFXFormatR5G5B5X1" },
   { GFXFormatA4L4, "GFXFormatA4L4" },
   { GFXFormatA8L8, "GFXFormatA8L8" },
   { GFXFormatA8, "GFXFormatA8" },
   { GFXFormatL8, "GFXFormatL8" },
   { GFXFormatDXT1, "GFXFormatDXT1" },
   { GFXFormatDXT2, "GFXFormatDXT2" }, 
   { GFXFormatDXT3, "GFXFormatDXT3" }, 
   { GFXFormatDXT4, "GFXFormatDXT4" }, 
   { GFXFormatDXT5, "GFXFormatDXT5" }, 
   { GFXFormatD32, "GFXFormatD32" }, 
   { GFXFormatD24X8, "GFXFormatD24X8" },
   { GFXFormatD24S8, "GFXFormatD24S8" },
   { GFXFormatD24FS8, "GFXFormatD24FS8" },
   { GFXFormatD16, "GFXFormatD16" }, 

   { GFXFormatR32G32B32A32F, "GFXFormatR32G32B32A32F" }, 
   { GFXFormatR16G16B16A16F, "GFXFormatR16G16B16A16F" }, 
   { GFXFormatL16, "GFXFormatL16" }, 
   { GFXFormatR16G16B16A16, "GFXFormatR16G16B16A16" }, 
   { GFXFormatR16G16, "GFXFormatR16G16" }, 
   { GFXFormatR16F, "GFXFormatR16F" }, 
   { GFXFormatR16G16F, "GFXFormatR16G16F" }, 
   { GFXFormatR10G10B10A2, "GFXFormatR10G10B10A2" },

EndImplementEnumType;


ImplementEnumType( GFXCullMode,
   "The render cull modes.\n"
   "@ingroup GFX" )

   { GFXCullNone, "GFXCullNone" },
   { GFXCullCW, "GFXCullCW" },
   { GFXCullCCW, "GFXCullCCW" }

EndImplementEnumType;


ImplementEnumType( GFXStencilOp,
   "The stencil operators.\n"
   "@ingroup GFX" )

   { GFXStencilOpKeep, "GFXStencilOpKeep" },
   { GFXStencilOpZero, "GFXStencilOpZero" },
   { GFXStencilOpReplace, "GFXStencilOpReplace" },
   { GFXStencilOpIncrSat, "GFXStencilOpIncrSat" },
   { GFXStencilOpDecrSat, "GFXStencilOpDecrSat" },
   { GFXStencilOpInvert, "GFXStencilOpInvert" },
   { GFXStencilOpIncr, "GFXStencilOpIncr" },
   { GFXStencilOpDecr, "GFXStencilOpDecr" },

EndImplementEnumType;


ImplementEnumType( GFXBlendOp,
   "The blend operators.\n"
   "@ingroup GFX" )

   { GFXBlendOpAdd, "GFXBlendOpAdd" },
   { GFXBlendOpSubtract, "GFXBlendOpSubtract" },
   { GFXBlendOpRevSubtract, "GFXBlendOpRevSubtract" },
   { GFXBlendOpMin, "GFXBlendOpMin" },
   { GFXBlendOpMax, "GFXBlendOpMax" }

EndImplementEnumType;
