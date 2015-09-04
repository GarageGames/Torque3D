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

#include "platform/platform.h"
#include "gfx/gFont.h"

#include "core/resourceManager.h"
#include "core/stream/fileStream.h"
#include "core/strings/unicode.h"
#include "core/strings/findMatch.h"
#include "core/strings/stringFunctions.h"
#include "core/util/endian.h"
#include "core/util/safeDelete.h"
#include "console/console.h"
#include "console/engineAPI.h"
#include "platform/threads/mutex.h"
#include "zlib/zlib.h"


GFX_ImplementTextureProfile(GFXFontTextureProfile,
                            GFXTextureProfile::DiffuseMap, 
                            GFXTextureProfile::PreserveSize |
                            GFXTextureProfile::Static |
                            GFXTextureProfile::KeepBitmap |
                            GFXTextureProfile::NoMipmap, 
                            GFXTextureProfile::NONE);

template<> void *Resource<GFont>::create(const Torque::Path &path)
{
#ifdef TORQUE_DEBUG_RES_MANAGER
   Con::printf( "Resource<GFont>::create - [%s]", path.getFullPath().c_str() );
#endif

   return GFont::load( path );
}

template<> ResourceBase::Signature  Resource<GFont>::signature()
{
   return MakeFourCC('f','o','n','t');
}

/// Used for repacking in GFont::importStrip.
struct GlyphMap
{
   U32 charId;
   GBitmap *bitmap;
};

static S32 QSORT_CALLBACK GlyphMapCompare(const void *a, const void *b)
{
   S32 ha = ((GlyphMap *) a)->bitmap->getHeight();
   S32 hb = ((GlyphMap *) b)->bitmap->getHeight();

   return hb - ha;
}


const U32 GFont::csm_fileVersion = 3;

String GFont::getFontCacheFilename(const String &faceName, U32 size)
{
   return String::ToString("%s/%s %d (%s).uft",
      Con::getVariable("$GUI::fontCacheDirectory"), faceName.c_str(), size, getCharSetName(0));
}

GFont* GFont::load( const Torque::Path& path )
{
   FileStream  stream;

   stream.open( path.getFullPath(), Torque::FS::File::Read );
   if ( stream.getStatus() != Stream::Ok )
      return NULL;

   GFont *ret = new GFont;
   ret->mGFTFile = path;

   if(!ret->read(stream))
   {
      Con::errorf( "GFont::load - error reading '%s'", path.getFullPath().c_str() );
      SAFE_DELETE(ret);
   }
   else
   {
#ifndef TORQUE_OS_XENON
      PlatformFont   *platFont = createPlatformFont(ret->getFontFaceName(), ret->getFontSize(), ret->getFontCharSet());

      if ( platFont == NULL )
      {
         Con::errorf( "GFont::load - error creating platform font for '%s'", path.getFullPath().c_str() );
         SAFE_DELETE(ret);
      }
      else
         ret->setPlatformFont(platFont);
#endif
   }
   
   return ret;
}

Resource<GFont> GFont::create(const String &faceName, U32 size, const char *cacheDirectory, U32 charset /* = TGE_ANSI_CHARSET */)
{
   if( !cacheDirectory )
      cacheDirectory = Con::getVariable( "$GUI::fontCacheDirectory" );
      
   const Torque::Path   path( String::ToString("%s/%s %d (%s).uft",
      cacheDirectory, faceName.c_str(), size, getCharSetName(charset)) );

   Resource<GFont> ret;

   // If the file already exists attempt to load it
   if (Platform::isFile(path.getFullPath().c_str()))
   {
      ret = ResourceManager::get().load(path);

      if (ret != NULL)
      {
         ret->mGFTFile = path;
         return ret;
      }
   }

   // Otherwise attempt to have the platform generate a new font
   PlatformFont *platFont = createPlatformFont(faceName, size, charset);
   
   if (platFont == NULL)
   {
      String fontName;

#ifdef _XBOX
      //AssertFatal( false, "Font creation is not supported on the Xbox platform. Create the font files (*.uft) using the Windows/MacOS build of the project." );
      if(!faceName.equal("arial", String::NoCase) || size != 14)
      {
         return create("Arial", 14, cacheDirectory, charset);
      }
      return ret;
#endif

      // Couldn't load the requested font.  This probably will be common
      // since many unix boxes don't have arial or lucida console installed.
      // Attempt to map the font name into a font we're pretty sure exist
      // Lucida Console is a common code & console font on windows, and
      // Monaco is the recommended code & console font on mac.
      if (faceName.equal("arial", String::NoCase))
         fontName = "Helvetica";
      else if (faceName.equal("lucida console", String::NoCase))
         fontName = "Monaco";
      else if (faceName.equal("monaco", String::NoCase))
         fontName = "Courier";
      else
         return ret;

      return create(fontName, size, cacheDirectory, charset);
   }

   // Create the actual GFont and set some initial properties
   GFont *font = new GFont;
   font->mPlatformFont = platFont;
   font->mGFTFile = path;
   font->mFaceName = faceName;
   font->mSize = size;
   font->mCharSet = charset;

   font->mHeight   = platFont->getFontHeight();
   font->mBaseline = platFont->getFontBaseLine();
   font->mAscent   = platFont->getFontBaseLine();
   font->mDescent  = platFont->getFontHeight() - platFont->getFontBaseLine();

   // Flag it to save when we exit
   font->mNeedSave = true;

   // Load the newly created font into the ResourceManager
   ret.setResource(ResourceManager::get().load(path), font);

   return ret;
}

//-------------------------------------------------------------------------

GFont::GFont()
{
   VECTOR_SET_ASSOCIATION(mCharInfoList);
   VECTOR_SET_ASSOCIATION(mTextureSheets);

   for (U32 i = 0; i < (sizeof(mRemapTable) / sizeof(S32)); i++)
      mRemapTable[i] = -1;

   mCurX = mCurY = mCurSheet = -1;

   mPlatformFont = NULL;
   mSize = 0;
   mCharSet = 0;
   mNeedSave = false;
   
   mMutex = Mutex::createMutex();
}

GFont::~GFont()
{
   if(mNeedSave)
   {
      AssertFatal( mGFTFile.getFullPath().isNotEmpty(), "GFont::~GFont - path not set" );

      FileStream stream;
      stream.open(mGFTFile, Torque::FS::File::Write);

      if(stream.getStatus() == Stream::Ok) 
         write(stream);

      stream.close();
   }
   
   S32 i;

   for(i = 0;i < mCharInfoList.size();i++)
   {
       SAFE_DELETE_ARRAY(mCharInfoList[i].bitmapData);
   }

   for(i=0; i<mTextureSheets.size(); i++)
      mTextureSheets[i] = NULL;

   SAFE_DELETE(mPlatformFont);
   
   Mutex::destroyMutex(mMutex);
}

void GFont::dumpInfo() const
{
   // Number and extent of mapped characters?
   U32 mapCount = 0, mapBegin=0xFFFF, mapEnd=0;
   for(U32 i=0; i<0x10000; i++)
   {
      if(mRemapTable[i] != -1)
      {
         mapCount++;
         if(i<mapBegin) mapBegin = i;
         if(i>mapEnd)   mapEnd   = i;
      }
   }


   // Let's write out all the info we can on this font.
   Con::printf("   '%s' %dpt", mFaceName.c_str(), mSize);
   Con::printf("      - %d texture sheets, %d mapped characters.", mTextureSheets.size(), mapCount);

   if(mapCount)
      Con::printf("      - Codepoints range from 0x%x to 0x%x.", mapBegin, mapEnd);
   else
      Con::printf("      - No mapped codepoints.");
   Con::printf("      - Platform font is %s.", (mPlatformFont ? "present" : "not present") );
}

//-----------------------------------------------------------------------------

bool GFont::loadCharInfo(const UTF16 ch)
{
   PROFILE_SCOPE(GFont_loadCharInfo);

    if(mRemapTable[ch] != -1)
        return true;    // Not really an error

    if(mPlatformFont && mPlatformFont->isValidChar(ch))
    {
        Mutex::lockMutex(mMutex); // the CharInfo returned by mPlatformFont is static data, must protect from changes.
        PlatformFont::CharInfo &ci = mPlatformFont->getCharInfo(ch);
        if(ci.bitmapData)
            addBitmap(ci);

        mCharInfoList.push_back(ci);
        mRemapTable[ch] = mCharInfoList.size() - 1;
        
        mNeedSave = true;
        
        Mutex::unlockMutex(mMutex);
        return true;
    }

    return false;
}

void GFont::addBitmap(PlatformFont::CharInfo &charInfo)
{
   U32 nextCurX = U32(mCurX + charInfo.width ); /*7) & ~0x3;*/
   U32 nextCurY = U32(mCurY + mPlatformFont->getFontHeight()); // + 7) & ~0x3;

   // These are here for postmortem debugging.
   bool routeA = false, routeB = false;

   if(mCurSheet == -1 || nextCurY >= TextureSheetSize)
   {
      routeA = true;
      addSheet();

      // Recalc our nexts.
      nextCurX = U32(mCurX + charInfo.width); // + 7) & ~0x3;
      nextCurY = U32(mCurY + mPlatformFont->getFontHeight()); // + 7) & ~0x3;
   }

   if( nextCurX >= TextureSheetSize)
   {
      routeB = true;
      mCurX = 0;
      mCurY = nextCurY;

      // Recalc our nexts.
      nextCurX = U32(mCurX + charInfo.width); // + 7) & ~0x3;
      nextCurY = U32(mCurY + mPlatformFont->getFontHeight()); // + 7) & ~0x3;
   }

   // Check the Y once more - sometimes we advance to a new row and run off
   // the end.
   if(nextCurY >= TextureSheetSize)
   {
      routeA = true;
      addSheet();

      // Recalc our nexts.
      nextCurX = U32(mCurX + charInfo.width); // + 7) & ~0x3;
      nextCurY = U32(mCurY + mPlatformFont->getFontHeight()); // + 7) & ~0x3;
   }

    charInfo.bitmapIndex = mCurSheet;
    charInfo.xOffset = mCurX;
    charInfo.yOffset = mCurY;

   mCurX = nextCurX;

   S32 x, y;
   GBitmap *bmp = mTextureSheets[mCurSheet].getBitmap();

   AssertFatal(bmp->getFormat() == GFXFormatA8, "GFont::addBitmap - cannot added characters to non-greyscale textures!");

   for(y = 0;y < charInfo.height;y++)
      for(x = 0;x < charInfo.width;x++)
         *bmp->getAddress(x + charInfo.xOffset, y + charInfo.yOffset) = charInfo.bitmapData[y * charInfo.width + x];

   mTextureSheets[mCurSheet].refresh();
}

void GFont::addSheet()
{
    GBitmap *bitmap = new GBitmap(TextureSheetSize, TextureSheetSize, false, GFXFormatA8 );

    // Set everything to transparent.
    U8 *bits = bitmap->getWritableBits();
    dMemset(bits, 0, sizeof(U8) *TextureSheetSize*TextureSheetSize);

    GFXTexHandle handle = GFXTexHandle( bitmap, &GFXFontTextureProfile, true, avar("%s() - (line %d)", __FUNCTION__, __LINE__) );
    mTextureSheets.increment();
    mTextureSheets.last() = handle;

    mCurX = 0;
    mCurY = 0;
    mCurSheet = mTextureSheets.size() - 1;
}

//-----------------------------------------------------------------------------

const PlatformFont::CharInfo &GFont::getCharInfo(const UTF16 in_charIndex)
{
    PROFILE_SCOPE(GFont_getCharInfo);

    AssertFatal(in_charIndex, "GFont::getCharInfo - can't get info for char 0!");

    if(mRemapTable[in_charIndex] == -1)
        loadCharInfo(in_charIndex);

    AssertFatal(mRemapTable[in_charIndex] != -1, "No remap info for this character");
    
    return mCharInfoList[mRemapTable[in_charIndex]];
}

const PlatformFont::CharInfo &GFont::getDefaultCharInfo()
{
   static PlatformFont::CharInfo c;
   // c is initialized by the CharInfo default constructor.
   return c;
}

//-----------------------------------------------------------------------------

U32 GFont::getStrWidth(const UTF8* in_pString)
{
   AssertFatal(in_pString != NULL, "GFont::getStrWidth: String is NULL, height is undefined");
   // If we ain't running debug...
   if (in_pString == NULL || *in_pString == '\0')
      return 0;

   return getStrNWidth(in_pString, dStrlen(in_pString));
}

U32 GFont::getStrWidthPrecise(const UTF8* in_pString)
{
   AssertFatal(in_pString != NULL, "GFont::getStrWidth: String is NULL, height is undefined");
   // If we ain't running debug...
   if (in_pString == NULL)
      return 0;

   return getStrNWidthPrecise(in_pString, dStrlen(in_pString));
}

//-----------------------------------------------------------------------------
U32 GFont::getStrNWidth(const UTF8 *str, U32 n)
{
   // UTF8 conversion is expensive. Avoid converting in a tight loop.
   FrameTemp<UTF16> str16(n + 1);
   convertUTF8toUTF16N(str, str16, n + 1);
   return getStrNWidth(str16, dStrlen(str16));
}

U32 GFont::getStrNWidth(const UTF16 *str, U32 n)
{
   AssertFatal(str != NULL, "GFont::getStrNWidth: String is NULL");

   if (str == NULL || str[0] == '\0' || n == 0)   
      return 0;
      
   U32 totWidth = 0;
   UTF16 curChar;
   U32 charCount;
   
   for(charCount = 0; charCount < n; charCount++)
   {
      curChar = str[charCount];
      if(curChar == '\0')
         break;

      if(isValidChar(curChar))
      {
         const PlatformFont::CharInfo& rChar = getCharInfo(curChar);
         totWidth += rChar.xIncrement;
      }
      else if (curChar == dT('\t'))
      {
         const PlatformFont::CharInfo& rChar = getCharInfo(dT(' '));
         totWidth += rChar.xIncrement * TabWidthInSpaces;
      }
   }

   return(totWidth);
}

U32 GFont::getStrNWidthPrecise(const UTF8 *str, U32 n)
{
   FrameTemp<UTF16> str16(n + 1);
   convertUTF8toUTF16N(str, str16, n + 1);
   return getStrNWidthPrecise(str16, dStrlen(str16));
}

U32 GFont::getStrNWidthPrecise(const UTF16 *str, U32 n)
{
   AssertFatal(str != NULL, "GFont::getStrNWidth: String is NULL");

   if (str == NULL || str[0] == '\0' || n == 0)   
      return(0);
      
   U32 totWidth = 0;
   UTF16 curChar;
   U32 charCount = 0;
   
   for(charCount = 0; charCount < n; charCount++)
   {
      curChar = str[charCount];
      if(curChar == '\0')
         break;
         
      if(isValidChar(curChar))
      {
         const PlatformFont::CharInfo& rChar = getCharInfo(curChar);
         totWidth += rChar.xIncrement;
      }
      else if (curChar == dT('\t'))
      {
         const PlatformFont::CharInfo& rChar = getCharInfo(dT(' '));
         totWidth += rChar.xIncrement * TabWidthInSpaces;
      }
   }

   UTF16 endChar = str[getMin(charCount,n-1)];

   if (isValidChar(endChar))
   {
      const PlatformFont::CharInfo& rChar = getCharInfo(endChar);
      if (rChar.width != rChar.xIncrement)
         totWidth += (rChar.width - rChar.xIncrement);
   }

   return(totWidth);
}

U32 GFont::getBreakPos(const UTF16 *str16, U32 slen, U32 width, bool breakOnWhitespace)
{
   // Some early out cases.
   if(slen==0)
      return 0;

   U32 ret = 0;
   U32 lastws = 0;
   UTF16 c;
   U32 charCount = 0;

   for( charCount=0; charCount < slen; charCount++)
   {
      c = str16[charCount];
      if(c == '\0')
         break;
         
      if(c == dT('\t'))
         c = dT(' ');
      
      if(!isValidChar(c))
      {
         ret++;
         continue;
      }
      
      if(c == dT(' '))
         lastws = ret+1;

      const PlatformFont::CharInfo& rChar = getCharInfo(c);
      if(rChar.width > width || rChar.xIncrement > width)
      {
         if(lastws && breakOnWhitespace)
            return lastws;
         return ret;
      }

      width -= rChar.xIncrement;
      
      ret++;
   }
   return ret;
}

void GFont::wrapString(const UTF8 *txt, U32 lineWidth, Vector<U32> &startLineOffset, Vector<U32> &lineLen)
{
   // TODO: Is this error still true?
   //Con::errorf("GFont::wrapString(): Not yet converted to be UTF-8 safe");

   startLineOffset.clear();
   lineLen.clear();

   if (!txt || !txt[0] || lineWidth < getCharWidth('W')) //make sure the line width is greater then a single character
      return;

   U32 len = dStrlen(txt);

   U32 startLine; 

   for (U32 i = 0; i < len;)
   {
      U32 wide = 0; 
      startLine = i;
      startLineOffset.push_back(startLine);

      // loop until the string is too large
      bool needsNewLine = false;
      U32 lineStrWidth = 0;
      for (; i < len; i++)
      {
         if( txt[ i ] == '\n' )
         {
            needsNewLine = true;
            break;
         }
         else if(isValidChar(txt[i]))
         {
            lineStrWidth += getCharInfo(txt[i]).xIncrement;
            if(txt[i] < 0) // symbols which code > 127
            {  
               wide++; i++;
            }
            if( lineStrWidth > lineWidth )
            {
               needsNewLine = true;
               break;      
            }
         }
      }

      if (!needsNewLine)
      {
         // we are done!
         lineLen.push_back(i - startLine - wide);
         return;
      }

      S32 j; 

      // Did we hit a hardwrap (newline character) in the string.
      bool hardwrap = ( txt[i] == '\n' );
      
      if ( hardwrap )
      {
         j = i;
      }
      // determine where to put the newline
      // we need to backtrack until we find a space character 
      // we don't do this for hardwrap(s)
      else
      {        
         for (j = i - 1; j >= startLine; j--)
         {
            if ( dIsspace(txt[j]) || txt[i] == '\n' )
               break;
         }

         if (j < startLine)
         {
            // the line consists of a single word!              
            // So, just break up the word

            j = i - 1;
         }
      }

      lineLen.push_back(j - startLine - wide);
      i = j;

      // Now we need to increment through any space characters at the
      // beginning of the next line.
      // We don't skip spaces after a hardwrap because they were obviously intended.
      for (i++; i < len; i++)
      {         
         if ( txt[i] == '\n' )
            continue;

         if ( dIsspace( txt[i] ) && !hardwrap )
            continue;

         break;
      }
   }
}

//-----------------------------------------------------------------------------

bool GFont::read(Stream& io_rStream)
{
    // Handle versioning
    U32 version;
    io_rStream.read(&version);
    if(version != csm_fileVersion)
        return false;

    char buf[256];
    io_rStream.readString(buf);
    mFaceName = buf;

    io_rStream.read(&mSize);
    io_rStream.read(&mCharSet);

    io_rStream.read(&mHeight);
    io_rStream.read(&mBaseline);
    io_rStream.read(&mAscent);
    io_rStream.read(&mDescent);

    U32 size = 0;
    io_rStream.read(&size);
    mCharInfoList.setSize(size);
    U32 i;
    for(i = 0; i < size; i++)
    {
        PlatformFont::CharInfo *ci = &mCharInfoList[i];
        io_rStream.read(&ci->bitmapIndex);
        io_rStream.read(&ci->xOffset);
        io_rStream.read(&ci->yOffset);
        io_rStream.read(&ci->width);
        io_rStream.read(&ci->height);
        io_rStream.read(&ci->xOrigin);
        io_rStream.read(&ci->yOrigin);
        io_rStream.read(&ci->xIncrement);
        ci->bitmapData = NULL;
   }

   U32 numSheets = 0;
   io_rStream.read(&numSheets);
   
   for(i = 0; i < numSheets; i++)
   {
       GBitmap *bmp = new GBitmap;
       if(!bmp->readBitmap("png", io_rStream))
       {
           delete bmp;
           return false;
       }
       GFXTexHandle handle = GFXTexHandle(bmp, &GFXFontTextureProfile, true, avar("%s() - Read Font Sheet for %s %d (line %d)", __FUNCTION__, mFaceName.c_str(), mSize, __LINE__));
       //handle.setFilterNearest();
       mTextureSheets.push_back(handle);
   }
   
   // Read last position info
   io_rStream.read(&mCurX);
   io_rStream.read(&mCurY);
   io_rStream.read(&mCurSheet);

   // Read the remap table.
   U32 minGlyph, maxGlyph;
   io_rStream.read(&minGlyph);
   io_rStream.read(&maxGlyph);

   if(maxGlyph >= minGlyph)
   {
      // Length of buffer..
      U32 buffLen;
      io_rStream.read(&buffLen);

      // Read the buffer.
      FrameTemp<S32> inBuff(buffLen);
      io_rStream.read(buffLen, inBuff);

      // Decompress.
      uLongf destLen = (maxGlyph-minGlyph+1)*sizeof(S32);
      uncompress((Bytef*)&mRemapTable[minGlyph], &destLen, (Bytef*)(S32*)inBuff, buffLen);

      AssertISV(destLen == (maxGlyph-minGlyph+1)*sizeof(S32), "GFont::read - invalid remap table data!");

      // Make sure we've got the right endianness.
      for(i = minGlyph; i <= maxGlyph; i++)
         mRemapTable[i] = convertBEndianToHost(mRemapTable[i]);
   }
   
   return (io_rStream.getStatus() == Stream::Ok);
}

bool GFont::write(Stream& stream)
{
    // Handle versioning
    stream.write(csm_fileVersion);

    // Write font info
    stream.write(mFaceName);
    stream.write(mSize);
    stream.write(mCharSet);
   
    stream.write(mHeight);
    stream.write(mBaseline);
    stream.write(mAscent);
    stream.write(mDescent);

    // Write char info list
    stream.write(U32(mCharInfoList.size()));
    U32 i;
    for(i = 0; i < mCharInfoList.size(); i++)
    {
        const PlatformFont::CharInfo *ci = &mCharInfoList[i];
        stream.write(ci->bitmapIndex);
        stream.write(ci->xOffset);
        stream.write(ci->yOffset);
        stream.write(ci->width);
        stream.write(ci->height);
        stream.write(ci->xOrigin);
        stream.write(ci->yOrigin);
        stream.write(ci->xIncrement);
   }

   stream.write(mTextureSheets.size());
   for(i = 0; i < mTextureSheets.size(); i++)
       mTextureSheets[i].getBitmap()->writeBitmap("png", stream);

   stream.write(mCurX);
   stream.write(mCurY);
   stream.write(mCurSheet);

   // Get the min/max we have values for, and only write that range out.
   S32 minGlyph = S32_MAX, maxGlyph = 0;

   for(i = 0; i < 65536; i++)
   {
      if(mRemapTable[i] != -1)
      {
         if(i>maxGlyph) maxGlyph = i;
         if(i<minGlyph) minGlyph = i;
      }
   }

   stream.write(minGlyph);
   stream.write(maxGlyph);

   // Skip it if we don't have any glyphs to do...
   if(maxGlyph >= minGlyph)
   {
      // Put everything big endian, to be consistent. Do this inplace.
      for(i = minGlyph; i <= maxGlyph; i++)
         mRemapTable[i] = convertHostToBEndian(mRemapTable[i]);

      {
         // Compress.
         const U32 buffSize = 128 * 1024;
         FrameTemp<S32> outBuff(buffSize);
         uLongf destLen = buffSize * sizeof(S32);
         compress2((Bytef*)(S32*)outBuff, &destLen, (Bytef*)(S32*)&mRemapTable[minGlyph], (maxGlyph-minGlyph+1)*sizeof(S32), 9);

         // Write out.
         stream.write((U32)destLen);
         stream.write(destLen, outBuff);
      }

      // Put us back to normal.
      for(i = minGlyph; i <= maxGlyph; i++)
         mRemapTable[i] = convertBEndianToHost(mRemapTable[i]);
   }
   
   return (stream.getStatus() == Stream::Ok);
}

void GFont::exportStrip(const char *fileName, U32 padding, U32 kerning)
{
   // Figure dimensions of our strip by iterating over all the char infos.
   U32 totalHeight = 0;
   U32 totalWidth = 0;

   S32 heightMin=0, heightMax=0;

   for(S32 i=0; i<mCharInfoList.size(); i++)
   {
      totalWidth += mCharInfoList[i].width + kerning + 2*padding;
      heightMin = getMin((S32)heightMin, (S32)getBaseline() - (S32)mCharInfoList[i].yOrigin);
      heightMax = getMax((S32)heightMax, (S32)getBaseline() - (S32)mCharInfoList[i].yOrigin + (S32)mCharInfoList[i].height);
   }

   totalHeight = heightMax - heightMin + 2*padding;

   // Make the bitmap.
   GBitmap gb(totalWidth, totalHeight, false, mTextureSheets[0].getBitmap()->getFormat());

   dMemset(gb.getWritableBits(), 0, sizeof(U8) * totalHeight * totalWidth );

   // Ok, copy some rects, taking into account padding, kerning, offset.
   U32 curWidth = kerning + padding;

   for(S32 i=0; i<mCharInfoList.size(); i++)
   {
      // Skip invalid stuff.
      if(mCharInfoList[i].bitmapIndex == -1 || mCharInfoList[i].height == 0 || mCharInfoList[i].width == 0)
         continue;

      // Copy the rect.
      U32 bitmap = mCharInfoList[i].bitmapIndex;

      RectI ri(mCharInfoList[i].xOffset, mCharInfoList[i].yOffset, mCharInfoList[i].width, mCharInfoList[i].height );
      Point2I outRi(curWidth, padding + getBaseline() - mCharInfoList[i].yOrigin);
      gb.copyRect(mTextureSheets[bitmap].getBitmap(), ri, outRi); 

      // Advance.
      curWidth +=  mCharInfoList[i].width + kerning + 2*padding;
   }

   // Write the image!
   FileStream fs;
   
   fs.open( fileName, Torque::FS::File::Write );

   if(fs.getStatus() != Stream::Ok)
   {
      Con::errorf("GFont::exportStrip - failed to open '%s' for writing.", fileName);
      return;
   }
 
   // Done!
   gb.writeBitmap("png", fs);
}

void  GFont::setPlatformFont(PlatformFont *inPlatformFont)
{
   AssertFatal( mPlatformFont == NULL, "Trying to set platform font which already exists");

   mPlatformFont = inPlatformFont;
}

void GFont::importStrip(const char *fileName, U32 padding, U32 kerning)
{
   // Wipe our texture sheets, and reload bitmap data from the specified file.
   // Also deal with kerning.
   // Also, we may have to load RGBA instead of RGB.

   // Wipe our texture sheets.
   mCurSheet = mCurX = mCurY = 0;
   mTextureSheets.clear();

   //  Now, load the font strip.
   Resource<GBitmap> strip = GBitmap::load(fileName);

   if(!strip)
   {
      Con::errorf("GFont::importStrip - could not load file '%s'!", fileName);
      return;
   }

   // And get parsing and copying - load up all the characters as separate
   // GBitmaps, sort, then pack. Not terribly efficient but this is basically
   // on offline task anyway.

   // Ok, snag some glyphs.
   Vector<GlyphMap> glyphList;
   glyphList.reserve(mCharInfoList.size());

   U32 curWidth = 0;
   for(S32 i=0; i<mCharInfoList.size(); i++)
   {
      // Skip invalid stuff.
      if(mCharInfoList[i].bitmapIndex == -1 || mCharInfoList[i].height == 0 || mCharInfoList[i].width == 0)
         continue;

      // Allocate a new bitmap for this glyph, taking into account kerning and padding.
      glyphList.increment();
      GlyphMap& lastGlyphMap = glyphList.last();
      lastGlyphMap.bitmap = new GBitmap(mCharInfoList[i].width + kerning + 2 * padding, mCharInfoList[i].height + 2 * padding, false, strip->getFormat());
      lastGlyphMap.charId = i;

      // Copy the rect.
      RectI ri(curWidth, getBaseline() - mCharInfoList[i].yOrigin, lastGlyphMap.bitmap->getWidth(), lastGlyphMap.bitmap->getHeight());
      Point2I outRi(0,0);
      lastGlyphMap.bitmap->copyRect(strip, ri, outRi);

      // Update glyph attributes.
      mCharInfoList[i].width = lastGlyphMap.bitmap->getWidth();
      mCharInfoList[i].height = lastGlyphMap.bitmap->getHeight();
      mCharInfoList[i].xOffset -= kerning + padding;
      mCharInfoList[i].xIncrement += kerning;
      mCharInfoList[i].yOffset -= padding;

      // Advance.
      curWidth += ri.extent.x;
   }

   // Ok, we have a big list of glyphmaps now. So let's sort them, then pack them.
   dQsort(glyphList.address(), glyphList.size(), sizeof(GlyphMap), GlyphMapCompare);

   // They're sorted by height, so now we can do some sort of awesome packing.
   Point2I curSheetSize(256, 256);
   Vector<U32> sheetSizes;

   S32 curY = 0;
   S32 curX = 0;
   S32 curLnHeight = 0;
   S32 maxHeight = 0;
   for(U32 i = 0; i < glyphList.size(); i++)
   {
      PlatformFont::CharInfo *ci = &mCharInfoList[glyphList[i].charId];
      
      if(ci->height > maxHeight)
         maxHeight = ci->height;

      if(curX + ci->width > curSheetSize.x)
      {
         curY += curLnHeight;
         curX = 0;
         curLnHeight = 0;
      }

      if(curY + ci->height > curSheetSize.y)
      {
         sheetSizes.push_back(curSheetSize.y);
         curX = 0;
         curY = 0;
         curLnHeight = 0;
      }

      if(ci->height > curLnHeight)
         curLnHeight = ci->height;
      
      ci->bitmapIndex = sheetSizes.size();
      ci->xOffset = curX;
      ci->yOffset = curY;
      curX += ci->width;
   }

   // Terminate the packing loop calculations.
   curY += curLnHeight;

   if(curY < 64)
      curSheetSize.y = 64;
   else if(curY < 128)
      curSheetSize.y = 128;

   sheetSizes.push_back(curSheetSize.y);

   if(getHeight() + padding * 2 > maxHeight)
      maxHeight = getHeight() + padding * 2;

   // Allocate texture pages.
   for(S32 i=0; i<sheetSizes.size(); i++)
   {
      GBitmap *bitmap = new GBitmap(TextureSheetSize, TextureSheetSize, false, strip->getFormat());

      // Set everything to transparent.
      U8 *bits = bitmap->getWritableBits();
      dMemset(bits, 0, sizeof(U8) *TextureSheetSize*TextureSheetSize * strip->getBytesPerPixel());

      GFXTexHandle handle = GFXTexHandle( bitmap, &GFXFontTextureProfile, true, avar("%s() - Font Sheet for %s (line %d)", __FUNCTION__, fileName, __LINE__) );
      mTextureSheets.increment();
      mTextureSheets.last() = handle;
   }

   // Alright, we're ready to copy bits!
   for(S32 i=0; i<glyphList.size(); i++)
   {
      // Copy each glyph into the appropriate place.
      PlatformFont::CharInfo *ci = &mCharInfoList[glyphList[i].charId];
      U32 bi = ci->bitmapIndex;
      mTextureSheets[bi]->getBitmap()->copyRect(glyphList[i].bitmap, RectI(0,0, glyphList[i].bitmap->getWidth(),glyphList[i].bitmap->getHeight()), Point2I(ci->xOffset, ci->yOffset));
   }

   // Ok, all done! Just refresh some textures and we're set.
   for(S32 i=0; i<sheetSizes.size(); i++)
      mTextureSheets[i].refresh();
}

DefineEngineFunction( populateFontCacheString, void, ( const char *faceName, S32 fontSize, const char *string ),,
    "Populate the font cache for the specified font with characters from the specified string.\n"
    "@param faceName The name of the font face.\n"
    "@param fontSize The size of the font in pixels.\n"
    "@param string The string to populate.\n"
    "@ingroup Font\n" )
{
   Resource<GFont> f = GFont::create(faceName, fontSize, Con::getVariable("$GUI::fontCacheDirectory"));

   if(f == NULL)
   {
      Con::errorf("populateFontCacheString - could not load font '%s %d'", faceName, fontSize);
      return;
   }

   if(!f->hasPlatformFont())
   {
      Con::errorf("populateFontCacheString - font '%s %d' has no platform font. Cannot generate more characters.", faceName, fontSize);
      return;
   }

   // This has the side effect of generating character info, including the bitmaps.
   f->getStrWidthPrecise( string );
}

DefineEngineFunction( populateFontCacheRange, void, ( const char *faceName, S32 fontSize, U32 rangeStart, U32 rangeEnd ),,
   "Populate the font cache for the specified font with Unicode code points in the specified range.\n"
   "@param faceName The name of the font face.\n"
   "@param fontSize The size of the font in pixels.\n"
   "@param rangeStart The start Unicode point.\n"
   "@param rangeEnd The end Unicode point.\n"
   "@note We only support BMP-0, so code points range from 0 to 65535.\n"
   "@ingroup Font\n" )
{
   Resource<GFont> f = GFont::create(faceName, fontSize, Con::getVariable("$GUI::fontCacheDirectory"));

   if(f == NULL)
   {
      Con::errorf("populateFontCacheRange - could not load font '%s %d'", faceName, fontSize);
      return;
   }

   if(rangeStart > rangeEnd)
   {
      Con::errorf("populateFontCacheRange - range start is after end");
      return;
   }

   if(!f->hasPlatformFont())
   {
      Con::errorf("populateFontCacheRange - font '%s %d' has no platform font Cannot generate more characters.", faceName, fontSize);
      return;
   }

   // This has the side effect of generating character info, including the bitmaps.
   for(U32 i=rangeStart; i<rangeEnd; i++)
   {
      if(f->isValidChar(i))
         f->getCharWidth(i);
      else
         Con::warnf("populateFontCacheRange - skipping invalid char 0x%x",  i);
   }

   // All done!
}

DefineEngineFunction( dumpFontCacheStatus, void, (),,
   "Dumps to the console a full description of all cached fonts, along with "
   "info on the codepoints each contains.\n"
   "@ingroup Font\n" )
{
   Resource<GFont>   theFont = ResourceManager::get().startResourceList( Resource<GFont>::signature() );
 
   Con::printf("--------------------------------------------------------------------------");
   Con::printf("   Font Cache Usage Report");

   while( theFont != NULL )
   {
      theFont->dumpInfo();

      theFont = ResourceManager::get().nextResource();
   }
}

DefineEngineFunction( writeFontCache, void, (),, 
   "Force all cached fonts to serialize themselves to the cache.\n"
   "@ingroup Font\n" )
{
   Resource<GFont>   theFont = ResourceManager::get().startResourceList( Resource<GFont>::signature() );

   Con::printf("--------------------------------------------------------------------------");
   Con::printf("   Writing font cache to disk");

   while( theFont != NULL )
   {
      const String   fileName( theFont.getPath() );

      FileStream stream;
      stream.open(fileName, Torque::FS::File::Write);

      if(stream.getStatus() == Stream::Ok) 
      {
         Con::printf("      o Writing '%s' to disk...", fileName.c_str());
         theFont->write(stream);
      }
      else
      {
         Con::errorf("      o Could not open '%s' for write", fileName.c_str());
      }

      theFont = ResourceManager::get().nextResource();
  }
}

DefineEngineFunction( populateAllFontCacheString, void, ( const char *string ),,
   "Populate the font cache for all fonts with characters from the specified string.\n"
   "@ingroup Font\n" )
{
   Resource<GFont> theFont = ResourceManager::get().startResourceList( Resource<GFont>::signature() );

   Con::printf("Populating font cache with string '%s'", string);

   while( theFont != NULL )
   {
      if(theFont->hasPlatformFont())
      {
         // This has the side effect of generating character info, including the bitmaps.
         theFont->getStrWidthPrecise( string );
      }
      else
      {
         const String   fileName( theFont.getPath() );
         Con::errorf("populateAllFontCacheString - font '%s' has no platform font. Cannot generate more characters.", fileName.c_str());
      }

      theFont = ResourceManager::get().nextResource();
   }
}

DefineEngineFunction( populateAllFontCacheRange, void, ( U32 rangeStart, U32 rangeEnd ),,
   "Populate the font cache for all fonts with Unicode code points in the specified range.\n"
   "@param rangeStart The start Unicode point.\n"
   "@param rangeEnd The end Unicode point.\n"
   "@note We only support BMP-0, so code points range from 0 to 65535.\n"
   "@ingroup Font\n" )
{
   if(rangeStart > rangeEnd)
   {
      Con::errorf("populateAllFontCacheRange - range start is after end!");
      return;
   }

   Resource<GFont>   theFont = ResourceManager::get().startResourceList( Resource<GFont>::signature() );

   Con::printf("Populating font cache with range 0x%x to 0x%x", rangeStart, rangeEnd);

   while( theFont != NULL )
   {
      const String   fileName( theFont.getPath() );

      if(theFont->hasPlatformFont())
      {
         // This has the side effect of generating character info, including the bitmaps.
         Con::printf("   o Populating font '%s'", fileName.c_str());
         for(U32 i=rangeStart; i<rangeEnd; i++)
         {
            if(theFont->isValidChar(i))
               theFont->getCharWidth(i);
            else
               Con::warnf("populateAllFontCacheRange - skipping invalid char 0x%x",  i);
         }
      }
      else
      {
         Con::errorf("populateAllFontCacheRange - font '%s' has no platform font. Cannot generate more characters.", fileName.c_str());
      }

      theFont = ResourceManager::get().nextResource();
   }
}

DefineEngineFunction( exportCachedFont, void, 
   ( const char *faceName, S32 fontSize, const char *fileName, S32 padding, S32 kerning ),,
   "Export specified font to the specified filename as a PNG. The "
   "image can then be processed in Photoshop or another tool and "
   "reimported using importCachedFont. Characters in the font are "
   "exported as one long strip.\n"
   "@param faceName The name of the font face.\n"
   "@param fontSize The size of the font in pixels.\n"
   "@param fileName The file name and path for the output PNG.\n"
   "@param padding The padding between characters.\n"   
   "@param kerning The kerning between characters.\n"   
   "@ingroup Font\n" )
{
   // Tell the font to export itself.
   Resource<GFont> f = GFont::create(faceName, fontSize, Con::getVariable("$GUI::fontCacheDirectory"));

   if(f == NULL)
   {
      Con::errorf("exportCachedFont - could not load font '%s %d'", faceName, fontSize);
      return;
   }

   f->exportStrip(fileName, padding, kerning);
}

DefineEngineFunction( importCachedFont, void,
   ( const char *faceName, S32 fontSize, const char *fileName, S32 padding, S32 kerning ),,
   "Import an image strip from exportCachedFont. Call with the "
   "same parameters you called exportCachedFont.\n"
   "@param faceName The name of the font face.\n"
   "@param fontSize The size of the font in pixels.\n"
   "@param fileName The file name and path for the input PNG.\n"
   "@param padding The padding between characters.\n"   
   "@param kerning The kerning between characters.\n"   
   "@ingroup Font\n" )
{
   // Tell the font to import itself.
   Resource<GFont> f = GFont::create(faceName, fontSize, Con::getVariable("$GUI::fontCacheDirectory"));

   if(f == NULL)
   {
      Con::errorf("importCachedFont - could not load font '%s %d'", faceName, fontSize);
      return;
   }

   f->importStrip(fileName, padding, kerning);
}

DefineEngineFunction( duplicateCachedFont, void, 
   ( const char *oldFontName, S32 oldFontSize, const char *newFontName ),,
   "Copy the specified old font to a new name. The new copy will not have a "
   "platform font backing it, and so will never have characters added to it. "
   "But this is useful for making copies of fonts to add postprocessing effects "
   "to via exportCachedFont.\n"
   "@param oldFontName The name of the font face to copy.\n"
   "@param oldFontSize The size of the font to copy.\n"
   "@param newFontName The name of the new font face.\n"
   "@ingroup Font\n" )
{
   String newFontFile = GFont::getFontCacheFilename(newFontName, oldFontSize);

   // Load the original font.
   Resource<GFont> font = GFont::create(oldFontName, oldFontSize, Con::getVariable("$GUI::fontCacheDirectory"));

   // Deal with inexplicably missing or failed to load fonts.
   if (font == NULL)
   {
      Con::errorf(" o Couldn't find font : %s", oldFontName);
      return;
   }

   // Ok, dump info!
   FileStream stream;
   stream.open( newFontFile, Torque::FS::File::Write );
   if(stream.getStatus() == Stream::Ok) 
   {
      Con::printf( "      o Writing duplicate font '%s' to disk...", newFontFile.c_str() );
      font->write(stream);
      stream.close();
   }
   else
   {
      Con::errorf( "      o Could not open '%s' for write", newFontFile.c_str() );
   }
}

