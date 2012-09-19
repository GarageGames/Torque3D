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

#ifndef _GFXFONTBATCHER_H_
#define _GFXFONTBATCHER_H_

#include "core/dataChunker.h"
#include "gfx/gfxDevice.h"
#include "gfx/gFont.h"

#define TEXT_MAG 1

class FontRenderBatcher
{
   struct CharMarker 
   {
      S32 c;
      F32 x;
      GFXVertexColor color; 
      PlatformFont::CharInfo *ci;
   };

   struct SheetMarker
   {
      S32 numChars;
      S32 startVertex;
      CharMarker charIndex[1];
   };

   DataChunker mStorage;
   Vector<SheetMarker *> mSheets;
   GFont *mFont;
   U32 mLength;
   GFXStateBlockRef mFontSB;

   SheetMarker &getSheetMarker(U32 sheetID);

public:
   FontRenderBatcher();

   void init(GFont *font, U32 n);

   void queueChar(UTF16 c, S32 &currentX, GFXVertexColor &currentColor);

   void render(F32 rot, const Point2F &offset );
};

#endif