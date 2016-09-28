//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
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

#import "platform/platformFont.h"
#import <CoreText/CoreText.h>
//-----------------------------------------------------------------------------

class OSXFont : public PlatformFont
{
private:
   
   // Font reference.
   CTFontRef       mFontRef;
   
   // Distance from drawing point to typographic baseline.
   // Think of the drawing point as the upper left corner of a text box.
   // NOTE: 'baseline' is synonymous with 'ascent' in Torque.
   U32             mBaseline;
   
   // Distance between lines.
   U32             mHeight;
   
   // Glyph rendering color-space.
   CGColorSpaceRef mColorSpace;
   
public:
   OSXFont();
   virtual ~OSXFont();
   
   /// Look up the requested font, cache style, layout, colorspace, and some metrics.
   virtual bool create( const char* name, dsize_t size, U32 charset = TGE_ANSI_CHARSET);
   
   /// Determine if the character requested is a drawable character, or if it should be ignored.
   virtual bool isValidChar( const UTF16 character) const;
   virtual bool isValidChar( const UTF8* str) const;
   
   /// Get some vertical data on the font at large. Useful for drawing multiline text, and sizing text boxes.
   virtual U32 getFontHeight() const { return mHeight; }
   virtual U32 getFontBaseLine() const { return mBaseline; }
   
   // Draw the character to a temporary bitmap, and fill the CharInfo with various text metrics.
   virtual PlatformFont::CharInfo &getCharInfo(const UTF16 character) const;
   virtual PlatformFont::CharInfo &getCharInfo(const UTF8 *str) const;
};