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

#include "gfx/util/screenspace.h"

// The conversion from screen space to the render target
// is made more complex because screen space is relative
// to the viewport.
void ScreenSpace::RenderTargetParameters(const Point3I &targetSize, const RectI &targetViewport, Point4F &rtParams)
{
   // Top->Down
   Point2F targetOffset(   (F32)targetViewport.point.x / (F32)targetSize.x,
                           (F32)targetViewport.point.y / (F32)targetSize.y );

   // Bottom->Up
   //Point2F targetOffset(   (F32)targetViewport.point.x / (F32)targetSize.x,
   //                        ( (F32)targetSize.y - (F32)(targetViewport.point.y + targetViewport.extent.y ) ) / (F32)targetSize.y );


   // Get the scale to convert from the 
   // screen space to the target size.
   Point2F targetScale( (F32)targetViewport.extent.x / (F32)targetSize.x,
                        (F32)targetViewport.extent.y / (F32)targetSize.y );

   // Get the target half pixel size.
   const Point2F halfPixel( 0.5f / targetSize.x,
                            0.5f / targetSize.y );

   rtParams.set( targetOffset.x + halfPixel.x,
                 targetOffset.y + halfPixel.y,
                 targetScale.x,
                 targetScale.y );
}