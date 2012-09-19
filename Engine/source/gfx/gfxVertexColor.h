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

#ifndef _GFXVERTEXCOLOR_H_
#define _GFXVERTEXCOLOR_H_

#ifndef _SWIZZLE_H_
#include "core/util/swizzle.h"
#endif


class ColorI;


class GFXVertexColor 
{

private:
   U32 packedColorData;
   static Swizzle<U8, 4> *mDeviceSwizzle;
   
public:
   static void setSwizzle( Swizzle<U8, 4> *val ) { mDeviceSwizzle = val; }

   GFXVertexColor() : packedColorData( 0xFFFFFFFF ) {} // White with full alpha
   GFXVertexColor( const ColorI &color ) { set( color ); }

   void set( U8 red, U8 green, U8 blue, U8 alpha = 255 )
   {
      packedColorData = red << 0 | green << 8 | blue << 16 | alpha << 24;
      mDeviceSwizzle->InPlace( &packedColorData, sizeof( packedColorData ) );
   }

   void set( const ColorI &color )
   {
      mDeviceSwizzle->ToBuffer( &packedColorData, (U8 *)&color, sizeof( packedColorData ) );
   }

   GFXVertexColor &operator=( const ColorI &color ) { set( color ); return *this; }
   operator const U32 *() const { return &packedColorData; }
   const U32& getPackedColorData() const { return packedColorData; }

   void getColor( ColorI *color ) const
   {
      mDeviceSwizzle->ToBuffer( color, &packedColorData, sizeof( packedColorData ) );
   }      
};

#endif
