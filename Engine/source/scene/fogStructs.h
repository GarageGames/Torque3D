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

#ifndef _FOGSTRUCTS_H_
#define _FOGSTRUCTS_H_

/// The aerial fog settings.
struct FogData
{   
   F32 density;
   F32 densityOffset;
   F32 atmosphereHeight;
   LinearColorF color;

   FogData()
   {
      density = 0.0f;
      densityOffset = 0.0f;
      atmosphereHeight = 0.0f;
      color.set( 0.5f, 0.5f, 0.5f, 1.0f );
   }
};


/// The water fog settings.
struct WaterFogData
{   
   F32 density;
   F32 densityOffset;   
   F32 wetDepth;
   F32 wetDarkening;
   ColorI color;
   PlaneF plane;
   F32 depthGradMax;

   WaterFogData()
   {
      density = 0.0f;
      densityOffset = 0.0f;     
      wetDepth = 0.0f;
      wetDarkening = 0.0f;
      color.set( 0.5f, 0.5f, 0.5f, 1.0f );
      plane.set( 0.0f, 0.0f, 1.0f, 1e10 ); // Default to global bounds distance
      depthGradMax = 0.0f;
   }
};

#endif // _FOGSTRUCTS_H_