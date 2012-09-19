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

#ifndef _SHADOW_COMMON_H_
#define _SHADOW_COMMON_H_

#ifndef _DYNAMIC_CONSOLETYPES_H_
   #include "console/dynamicTypes.h"
#endif


///
enum ShadowType
{   
   ShadowType_None = -1,

   ShadowType_Spot,
   ShadowType_PSSM,

   ShadowType_Paraboloid,
   ShadowType_DualParaboloidSinglePass,
   ShadowType_DualParaboloid,
   ShadowType_CubeMap,

   ShadowType_Count,
};

DefineEnumType( ShadowType );


/// The different shadow filter modes used when rendering 
/// shadowed lights.
/// @see setShadowFilterMode
enum ShadowFilterMode
{
   ShadowFilterMode_None,
   ShadowFilterMode_SoftShadow,
   ShadowFilterMode_SoftShadowHighQuality
};

DefineEnumType( ShadowFilterMode );

#endif // _SHADOW_COMMON_H_
