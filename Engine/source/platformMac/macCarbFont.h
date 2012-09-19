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

#include <ApplicationServices/ApplicationServices.h>
#include "platform/platformFont.h"


class MacCarbFont : public PlatformFont
{
private:
   // Caches style, layout and colorspace data to speed up character drawing.
   // TODO: style colors
   ATSUStyle      mStyle;       
   ATSUTextLayout mLayout;       
   CGColorSpaceRef mColorSpace;
   
   // Cache the baseline and height for the getter methods below.
   U32               mHeight;    // distance between lines
   U32               mBaseline;  // distance from drawing point to typographic baseline,
                                 // think of the drawing point as the upper left corner of a text box.
                                 // note: 'baseline' is synonymous with 'ascent' in Torque.
                                 
   // Cache the size and name requested in create()
   U32               mSize;
   StringTableEntry  mName;
   
public:
   MacCarbFont();
   virtual ~MacCarbFont();
   
   /// Look up the requested font, cache style, layout, colorspace, and some metrics.
   virtual bool create( const char* name, U32 size, U32 charset = TGE_ANSI_CHARSET);
   
   /// Determine if the character requested is a drawable character, or if it should be ignored.
   virtual bool isValidChar( const UTF16 ch) const;
   virtual bool isValidChar( const UTF8 *str) const;
   
   /// Get some vertical data on the font at large. Useful for drawing multiline text, and sizing text boxes.
   virtual U32 getFontHeight() const;
   virtual U32 getFontBaseLine() const;
   
   // Draw the character to a temporary bitmap, and fill the CharInfo with various text metrics.
   virtual PlatformFont::CharInfo &getCharInfo(const UTF16 ch) const;
   virtual PlatformFont::CharInfo &getCharInfo(const UTF8 *str) const;
};

inline U32 MacCarbFont::getFontHeight() const
{
   return mHeight;
}

inline U32 MacCarbFont::getFontBaseLine() const
{
   return mBaseline;
}