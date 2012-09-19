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

#include "platformWin32/platformWin32.h"
#include "platformWin32/winFont.h"

#include "gfx/gFont.h"
#include "gfx/bitmap/gBitmap.h"
#include "math/mRect.h"
#include "console/console.h"
#include "core/strings/unicode.h"
#include "core/strings/stringFunctions.h"
#include "core/stringTable.h"

static HDC fontHDC = NULL;
static HBITMAP fontBMP = NULL;

static U32 charsetMap[]=
{
    ANSI_CHARSET,
    SYMBOL_CHARSET,
    SHIFTJIS_CHARSET,
    HANGEUL_CHARSET,
    HANGUL_CHARSET,
    GB2312_CHARSET,
    CHINESEBIG5_CHARSET,
    OEM_CHARSET,
    JOHAB_CHARSET,
    HEBREW_CHARSET,
    ARABIC_CHARSET,
    GREEK_CHARSET,
    TURKISH_CHARSET,
    VIETNAMESE_CHARSET,
    THAI_CHARSET,
    EASTEUROPE_CHARSET,
    RUSSIAN_CHARSET,
    MAC_CHARSET,
    BALTIC_CHARSET,
};
#define NUMCHARSETMAP   (sizeof(charsetMap) / sizeof(U32))

void createFontInit(void);
void createFontShutdown(void);
void CopyCharToBitmap(GBitmap *pDstBMP, HDC hSrcHDC, const RectI &r);

void createFontInit()
{
   //shared library sets the appInstance here
   winState.appInstance = GetModuleHandle(NULL);
   fontHDC = CreateCompatibleDC(NULL);
   fontBMP = CreateCompatibleBitmap(fontHDC, 256, 256);
}

void createFontShutdown()
{
   DeleteObject(fontBMP);
   DeleteObject(fontHDC);
}

void CopyCharToBitmap(GBitmap *pDstBMP, HDC hSrcHDC, const RectI &r)
{
   for (S32 i = r.point.y; i < r.point.y + r.extent.y; i++)
   {
      for (S32 j = r.point.x; j < r.point.x + r.extent.x; j++)
      {
         COLORREF color = GetPixel(hSrcHDC, j, i);
         if (color)
            *pDstBMP->getAddress(j, i) = 255;
         else
            *pDstBMP->getAddress(j, i) = 0;
      }
   }
}

//-----------------------------------------------------------------------------
// WinFont class
//-----------------------------------------------------------------------------

BOOL CALLBACK EnumFamCallBack(LPLOGFONT logFont, LPNEWTEXTMETRIC textMetric, DWORD fontType, LPARAM lParam)
{
   if( !( fontType & TRUETYPE_FONTTYPE ) )
      return true;

   Vector<StringTableEntry>* fonts = (Vector< StringTableEntry>*)lParam;

   const U32 len = dStrlen( logFont->lfFaceName ) * 3 + 1;
   FrameTemp<UTF8> buffer( len );
   convertUTF16toUTF8( logFont->lfFaceName, buffer, len );

   fonts->push_back( StringTable->insert( buffer ) );

   return true;
}

void PlatformFont::enumeratePlatformFonts( Vector<StringTableEntry>& fonts, UTF16* fontFamily )
{
   EnumFontFamilies( fontHDC, fontFamily, (FONTENUMPROC)EnumFamCallBack, (LPARAM)&fonts );
}

PlatformFont *createPlatformFont(const char *name, U32 size, U32 charset /* = TGE_ANSI_CHARSET */)
{
    PlatformFont *retFont = new WinFont;

    if(retFont->create(name, size, charset))
        return retFont;

    delete retFont;
    return NULL;
}

WinFont::WinFont() : mFont(NULL)
{
}

WinFont::~WinFont()
{
    if(mFont)
    {
        DeleteObject(mFont);
    }
}

bool WinFont::create(const char *name, U32 size, U32 charset /* = TGE_ANSI_CHARSET */)
{
   if(name == NULL || size < 1)
      return false;

   if(charset > NUMCHARSETMAP)
      charset = TGE_ANSI_CHARSET;

   U32 weight = 0;
   U32 doItalic = 0;

   String nameStr = name;
   nameStr = nameStr.trim();
   
   bool haveModifier;
   do
   {
      haveModifier = false;
      if( nameStr.compare( "Bold", 4, String::NoCase | String::Right ) == 0 )
      {
         weight = 700;
         nameStr = nameStr.substr( 0, nameStr.length() - 4 ).trim();
         haveModifier = true;
      }
      if( nameStr.compare( "Italic", 6, String::NoCase | String::Right ) == 0 )
      {
         doItalic = 1;
         nameStr = nameStr.substr( 0, nameStr.length() - 6 ).trim();
         haveModifier = true;
      }
   }
   while( haveModifier );

#ifdef UNICODE
   const UTF16* n = nameStr.utf16();
   mFont = CreateFont(size,0,0,0,weight,doItalic,0,0,DEFAULT_CHARSET,OUT_TT_PRECIS,0,PROOF_QUALITY,0,n);
#else
   mFont = CreateFont(size,0,0,0,weight,doItalic,0,0,charsetMap[charset],OUT_TT_PRECIS,0,PROOF_QUALITY,0,name);
#endif
   if(mFont == NULL)
      return false;

   SelectObject(fontHDC, fontBMP);
   SelectObject(fontHDC, mFont);
   GetTextMetrics(fontHDC, &mTextMetric);

   return true;
}

bool WinFont::isValidChar(const UTF16 ch) const
{
    return ch != 0 /* && (ch >= mTextMetric.tmFirstChar && ch <= mTextMetric.tmLastChar)*/;
}

bool WinFont::isValidChar(const UTF8 *str) const
{
    return isValidChar(oneUTF8toUTF32(str));
}


PlatformFont::CharInfo &WinFont::getCharInfo(const UTF16 ch) const
{
   static PlatformFont::CharInfo c;

    dMemset(&c, 0, sizeof(c));
    c.bitmapIndex = -1;

    static U8 scratchPad[65536];

    COLORREF backgroundColorRef = RGB(  0,   0,   0);
	 COLORREF foregroundColorRef = RGB(255, 255, 255);
    SelectObject(fontHDC, fontBMP);
    SelectObject(fontHDC, mFont);
    SetBkColor(fontHDC, backgroundColorRef);
    SetTextColor(fontHDC, foregroundColorRef);
    
    MAT2 matrix;
    GLYPHMETRICS metrics;
    RectI clip;
    
    FIXED zero;
    zero.fract = 0;
    zero.value = 0;
    FIXED one;
    one.fract = 0;
    one.value = 1;
    
    matrix.eM11 = one;
    matrix.eM12 = zero;
    matrix.eM21 = zero;
    matrix.eM22 = one;


    if(GetGlyphOutline(
            fontHDC,	// handle of device context 
            ch,	// character to query 
            GGO_GRAY8_BITMAP,	// format of data to return 
            &metrics,	// address of structure for metrics 
            sizeof(scratchPad),	// size of buffer for data 
            scratchPad,	// address of buffer for data 
            &matrix 	// address of transformation matrix structure  
            ) != GDI_ERROR)
    {
        U32 rowStride = (metrics.gmBlackBoxX + 3) & ~3; // DWORD aligned
        U32 size = rowStride * metrics.gmBlackBoxY;
        
        // [neo, 5/7/2007 - #3055] 
        // If we get large font sizes rowStride * metrics.gmBlackBoxY will
        // be larger than scratch pad size and so overwrite mem, boom!
        // Added range check < scratchPad for now but we need to review what
        // to do here - do we want to call GetGlyphOutline() first with null
        // values and get the real size to alloc buffer?
        //if( size > sizeof( scratchPad ) )
          //   DebugBreak();

        for(U32 j = 0; j < size && j < sizeof(scratchPad); j++)
        {
            U32 pad = U32(scratchPad[j]) << 2;
            if(pad > 255)
                pad = 255;
            scratchPad[j] = pad;
        }
        S32 inc = metrics.gmCellIncX;
        if(inc < 0)
            inc = -inc;
        
        c.xOffset = 0;
        c.yOffset = 0;
        c.width = metrics.gmBlackBoxX;
        c.height = metrics.gmBlackBoxY;
        c.xOrigin = metrics.gmptGlyphOrigin.x;
        c.yOrigin = metrics.gmptGlyphOrigin.y;
        c.xIncrement = metrics.gmCellIncX;

        c.bitmapData = new U8[c.width * c.height];
        AssertFatal( c.bitmapData != NULL, "Could not allocate memory for font bitmap data!");
        for(U32 y = 0; S32(y) < c.height; y++)
        {
            U32 x;
            for(x = 0; x < c.width; x++)
            {
               // [neo, 5/7/2007 - #3055] 
               // See comments above about scratchPad overrun
               S32 spi = y * rowStride + x;

               if( spi >= sizeof(scratchPad) )
                  return c;

                c.bitmapData[y * c.width + x] = scratchPad[spi];
            }
        }
    }
    else
    {
      SIZE size;
      GetTextExtentPoint32W(fontHDC, &ch, 1, &size);
      if(size.cx)
      {
          c.xIncrement = size.cx;
          c.bitmapIndex = 0;
      }
    }

    return c;
}

PlatformFont::CharInfo &WinFont::getCharInfo(const UTF8 *str) const
{
    return getCharInfo(oneUTF8toUTF32(str));
}
