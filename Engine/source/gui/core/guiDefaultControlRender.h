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

#ifndef _H_GUIDEFAULTCONTROLRENDER_
#define _H_GUIDEFAULTCONTROLRENDER_

#ifndef _MRECT_H_
#include "math/mRect.h"
#endif

class GuiControlProfile;
class ColorI;

void renderRaisedBox( const RectI &bounds, GuiControlProfile *profile);
void renderSlightlyRaisedBox( const RectI &bounds, GuiControlProfile *profile);
void renderLoweredBox( const RectI &bounds, GuiControlProfile *profile);
void renderSlightlyLoweredBox( const RectI &bounds, GuiControlProfile *profile);
void renderBorder( const RectI &bounds, GuiControlProfile *profile);
void renderFilledBorder( const RectI &bounds, GuiControlProfile *profile );
void renderFilledBorder( const RectI &bounds, const ColorI &borderColor, const ColorI &fillColor, U32 thickness = 1 );
void renderSizableBitmapBordersFilled( const RectI &bounds, S32 baseMultiplier, GuiControlProfile *profile); //  Added
void renderSizableBitmapBordersFilledIndex( const RectI &bounds, S32 startIndex, GuiControlProfile *profile);
void renderFixedBitmapBordersFilled( const RectI &bounds, S32 baseMultiplier, GuiControlProfile *profile); //  Added
void renderFixedBitmapBordersFilled( const RectI &bounds, S32 startIndex, GuiControlProfile *profile);

#endif
