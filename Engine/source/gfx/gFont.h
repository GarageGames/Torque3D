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

#ifndef _GFONT_H_
#define _GFONT_H_

//Includes
#ifndef _RESOURCE_H_
#include "core/resource.h"
#endif
#ifndef _PLATFORMFONT_H_
#include "platform/platformFont.h"
#endif
#ifndef _GBITMAP_H_
#include "gfx/bitmap/gBitmap.h"
#endif
#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif


GFX_DeclareTextureProfile(GFXFontTextureProfile);

class GFont
{
public:
   enum Constants 
   {
      TabWidthInSpaces = 3,
      TextureSheetSize = 256,
   };

public:
   GFont();
   virtual ~GFont();
   
   static Resource<GFont> create(const String &faceName, U32 size, const char *cacheDirectory = 0, U32 charset = TGE_ANSI_CHARSET);

   GFXTexHandle getTextureHandle(S32 index) const { return mTextureSheets[index]; }

   const PlatformFont::CharInfo& getCharInfo(const UTF16 in_charIndex);
   static const PlatformFont::CharInfo& getDefaultCharInfo();

   U32  getCharHeight(const UTF16 in_charIndex);
   U32  getCharWidth(const UTF16 in_charIndex);
   U32  getCharXIncrement(const UTF16 in_charIndex);
   
   bool isValidChar(const UTF16 in_charIndex) const;

   const U32 getHeight() const   { return mHeight; }
   const U32 getBaseline() const { return mBaseline; }
   const U32 getAscent() const   { return mAscent; }
   const U32 getDescent() const  { return mDescent; }

   U32 getBreakPos(const UTF16 *string, U32 strlen, U32 width, bool breakOnWhitespace);

   /// These are the preferred width functions.
   U32 getStrNWidth(const UTF16*, U32 n);
   U32 getStrNWidthPrecise(const UTF16*, U32 n);
   
   /// These UTF8 versions of the width functions will be deprecated, please avoid them.
   U32 getStrWidth(const UTF8*);   // Note: ignores c/r
   U32 getStrNWidth(const UTF8*, U32 n);
   U32 getStrWidthPrecise(const UTF8*);   // Note: ignores c/r
   U32 getStrNWidthPrecise(const UTF8*, U32 n);
   void wrapString(const UTF8 *string, U32 width, Vector<U32> &startLineOffset, Vector<U32> &lineLen);

   /// Dump information about this font to the console.
   void dumpInfo() const;

   /// Export to an image strip for image processing.
   void exportStrip(const char *fileName, U32 padding, U32 kerning);

   /// Import an image strip generated with exportStrip, make sure parameters match!
   void importStrip(const char *fileName, U32 padding, U32 kerning);

   void  setPlatformFont(PlatformFont *inPlatformFont);

   /// Query as to presence of platform font. If absent, we cannot generate more
   /// chars!
   const bool hasPlatformFont() const
   {
      return mPlatformFont != NULL;
   }

   /// Query to determine if we should use add or modulate (as A8 textures
   /// are treated as having 0 for RGB).
   bool isAlphaOnly() const
   {
      return mTextureSheets[0]->getBitmap()->getFormat() == GFXFormatA8;
   }

   /// Get the filename for a cached font.
   static String getFontCacheFilename(const String &faceName, U32 faceSize);

   /// Get the face name of the font.
   String   getFontFaceName() const { return mFaceName; };
   U32      getFontSize() const { return mSize; }
   U32      getFontCharSet() const { return mCharSet; }

   bool read(Stream& io_rStream);
   bool write(Stream& io_rStream);

   static GFont* load( const Torque::Path& path );

protected:
   bool loadCharInfo(const UTF16 ch);
   void addBitmap(PlatformFont::CharInfo &charInfo);
   void addSheet(void);
   void assignSheet(S32 sheetNum, GBitmap *bmp);

   void *mMutex;

private:
   static const U32 csm_fileVersion;

   PlatformFont *mPlatformFont;
   Vector<GFXTexHandle>mTextureSheets;

   S32 mCurX;
   S32 mCurY;
   S32 mCurSheet;

   bool mNeedSave;
   Torque::Path mGFTFile;
   String mFaceName;
   U32 mSize;
   U32 mCharSet;

   U32 mHeight;
   U32 mBaseline;
   U32 mAscent;
   U32 mDescent;

   /// List of character info structures, must be accessed through the 
   /// getCharInfo(U32) function to account for remapping.
   Vector<PlatformFont::CharInfo>  mCharInfoList;

   /// Index remapping
   S32             mRemapTable[65536];
};

inline U32 GFont::getCharXIncrement(const UTF16 in_charIndex)
{
    const PlatformFont::CharInfo& rChar = getCharInfo(in_charIndex);
    return rChar.xIncrement;
}

inline U32 GFont::getCharWidth(const UTF16 in_charIndex)
{
    const PlatformFont::CharInfo& rChar = getCharInfo(in_charIndex);
    return rChar.width;
}

inline U32 GFont::getCharHeight(const UTF16 in_charIndex)
{
    const PlatformFont::CharInfo& rChar = getCharInfo(in_charIndex);
    return rChar.height;
}

inline bool GFont::isValidChar(const UTF16 in_charIndex) const
{
   if(mRemapTable[in_charIndex] != -1)
      return true;

   if(mPlatformFont)
      return mPlatformFont->isValidChar(in_charIndex);

   return false;
}

#endif //_GFONT_H_
