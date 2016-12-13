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

#import <Cocoa/Cocoa.h>
#import "platform/platform.h"
#import "core/util/tVector.h"
#import "math/mMathFn.h"
#import "platformMac/macFont.h"
#import "core/stringTable.h"
#import "core/strings/unicode.h"
#import "console/console.h"


//------------------------------------------------------------------------------

PlatformFont* createPlatformFont( const char* name, dsize_t size, U32 charset )
{
   PlatformFont* pFont = new OSXFont();
   
   if ( pFont->create(name, size, charset) )
      return pFont;
   
   delete pFont;
   
   return NULL;
}

//------------------------------------------------------------------------------

void PlatformFont::enumeratePlatformFonts( Vector<StringTableEntry>& fonts, UTF16 *fontFamily )
{
   // Fetch available fonts.
   NSArray* availableFonts = [[NSFontManager sharedFontManager] availableFontNamesWithTraits:0];
   
   // Enumerate font names.
   for (id fontName in availableFonts)
   {
      fonts.push_back( StringTable->insert( [fontName UTF8String] ) );
   }
   
   // Release font name.
   [availableFonts release];
}

//------------------------------------------------------------------------------

OSXFont::OSXFont()
{
   // Reset the rendering color-space.
   mColorSpace = NULL;
}

//------------------------------------------------------------------------------

OSXFont::~OSXFont()
{
   // Destroy the rendering color-space.
   CGColorSpaceRelease( mColorSpace );
}

//------------------------------------------------------------------------------

bool OSXFont::create( const char* name, dsize_t size, U32 charset )
{
   // Sanity!
   AssertFatal( name != NULL, "Cannot create a NULL font name." );
   
   // Generate compatible font name.
   CFStringRef fontName = CFStringCreateWithCString( kCFAllocatorDefault, name, kCFStringEncodingUTF8 );
   
   // Sanity!
   if ( !fontName )
   {
      Con::errorf("Could not handle font name of '%s'.", name );
      return false;
   }
   
   // Use Windows as a baseline (96 DPI) and adjust accordingly.
   F32 scaledSize = size * (72.0f/96.0f);
   scaledSize = mRound(scaledSize);
   
   // Create the font reference.
   mFontRef = CTFontCreateWithName( fontName, scaledSize, NULL );
   
   // Sanity!
   if ( !mFontRef )
   {
      Con::errorf( "Could not generate a font reference to font name '%s' of size '%d'", name, size );
      return false;
   }
   
   // Fetch font metrics.
   CGFloat ascent = CTFontGetAscent( mFontRef );
   CGFloat descent = CTFontGetDescent( mFontRef );
   
   // Set baseline.
   mBaseline = (U32)mRound(ascent);
   
   // Set height.
   mHeight = (U32)mRound( ascent + descent );
   
   // Create a gray-scale color-space.
   mColorSpace = CGColorSpaceCreateDeviceGray();
   
   // Return status.
   return true;
}

//------------------------------------------------------------------------------

bool OSXFont::isValidChar( const UTF8* str ) const
{
   // since only low order characters are invalid, and since those characters
   // are single codeunits in UTF8, we can safely cast here.
   return isValidChar((UTF16)*str);
}

//------------------------------------------------------------------------------

bool OSXFont::isValidChar( const UTF16 character) const
{
   // We cut out the ASCII control chars here. Only printable characters are valid.
   // 0x20 == 32 == space
   if( character < 0x20 )
      return false;
   
   return true;
}

//------------------------------------------------------------------------------

PlatformFont::CharInfo& OSXFont::getCharInfo(const UTF8 *str) const
{
   return getCharInfo( oneUTF32toUTF16(oneUTF8toUTF32(str,NULL)) );
}

//------------------------------------------------------------------------------

PlatformFont::CharInfo& OSXFont::getCharInfo(const UTF16 character) const
{
   // Declare and clear out the CharInfo that will be returned.
   static PlatformFont::CharInfo characterInfo;
   dMemset(&characterInfo, 0, sizeof(characterInfo));
   
   // prep values for GFont::addBitmap()
   characterInfo.bitmapIndex = 0;
   characterInfo.xOffset = 0;
   characterInfo.yOffset = 0;
   
   CGGlyph characterGlyph;
   CGRect characterBounds;
   CGSize characterAdvances;
   UniChar unicodeCharacter = character;
   
   // Fetch font glyphs.
   if ( !CTFontGetGlyphsForCharacters( mFontRef, &unicodeCharacter, &characterGlyph, (CFIndex)1) )
   {
      // Sanity!
      //AssertFatal( false, "Cannot create font glyph." );
      Con::warnf("Font glyph is messed up. Some characters may render incorrectly.");
   }
   
   // Fetch glyph bounding box.
   CTFontGetBoundingRectsForGlyphs( mFontRef, kCTFontHorizontalOrientation, &characterGlyph, &characterBounds, (CFIndex)1 );
   
   // Fetch glyph advances.
   CTFontGetAdvancesForGlyphs( mFontRef, kCTFontHorizontalOrientation, &characterGlyph, &characterAdvances, (CFIndex)1 );
   
   // Set character metrics,
   characterInfo.xOrigin = (S32)mRound( characterBounds.origin.x );
   characterInfo.yOrigin = (S32)mRound( characterBounds.origin.y );
   characterInfo.width = (U32)mCeil( characterBounds.size.width ) + 2;
   characterInfo.height = (U32)mCeil( characterBounds.size.height ) + 2;
   characterInfo.xIncrement = (S32)mRound( characterAdvances.width );
   
   // Finish if character is undrawable.
   if ( characterInfo.width == 0 && characterInfo.height == 0 )
      return characterInfo;
   
   // Clamp character minimum width.
   if ( characterInfo.width == 0 )
      characterInfo.width = 2;
   
   if ( characterInfo.height == 0 )
      characterInfo.height = 1;
   
   
   // Allocate a bitmap surface.
   const U32 bitmapSize = characterInfo.width * characterInfo.height;
   characterInfo.bitmapData = new U8[bitmapSize];
   dMemset(characterInfo.bitmapData, 0x00, bitmapSize);
   
   // Create a bitmap context.
   CGContextRef bitmapContext = CGBitmapContextCreate( characterInfo.bitmapData, characterInfo.width, characterInfo.height, 8, characterInfo.width, mColorSpace, kCGImageAlphaNone );
   
   // Sanity!
   AssertFatal( bitmapContext != NULL, "Cannot create font context." );
   
   // Render font anti-aliased if font is arbitrarily small.
   CGContextSetShouldAntialias( bitmapContext, true);
   CGContextSetShouldSmoothFonts( bitmapContext, true);
   CGContextSetRenderingIntent( bitmapContext, kCGRenderingIntentAbsoluteColorimetric);
   CGContextSetInterpolationQuality( bitmapContext, kCGInterpolationNone);
   CGContextSetGrayFillColor( bitmapContext, 1.0, 1.0);
   CGContextSetTextDrawingMode( bitmapContext,  kCGTextFill);
   
   // Draw glyph.
   CGPoint renderOrigin;
   renderOrigin.x = -characterInfo.xOrigin;
   renderOrigin.y = -characterInfo.yOrigin;
   
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
   CTFontDrawGlyphs( mFontRef, &characterGlyph, &renderOrigin, 1, bitmapContext );
#else
   CGFontRef cgFont = CTFontCopyGraphicsFont(mFontRef, NULL);
   CGContextSetFont(bitmapContext, cgFont);
   CGContextSetFontSize(bitmapContext, CTFontGetSize(mFontRef));
   CGContextShowGlyphsAtPositions(bitmapContext, &characterGlyph, &renderOrigin, 1);
   CFRelease(cgFont);
#endif
   
   
#if 0
   Con::printf("Width:%f, Height:%f, OriginX:%f, OriginY:%f",
               characterBounds.size.width,
               characterBounds.size.height,
               characterBounds.origin.x,
               characterBounds.origin.y );
#endif
   
   // Adjust the y origin for the glyph size.
   characterInfo.yOrigin += characterInfo.height;// + mHeight;
   
   // Release the bitmap context.
   CGContextRelease( bitmapContext );
   
   // Return character information.
   return characterInfo;
}