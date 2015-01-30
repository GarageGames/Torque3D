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
#include "terrain/terrData.h"
#include "gfx/bitmap/gBitmap.h"
#include "terrain/terrMaterial.h"
#include "core/stream/fileStream.h"
#include "console/engineAPI.h"

#ifdef TORQUE_TOOLS

bool TerrainBlock::exportHeightMap( const UTF8 *filePath, const String &format ) const
{

   GBitmap output(   mFile->mSize,
                     mFile->mSize,
                     false,
                     GFXFormatR5G6B5 );

   // First capture the max height... we'll normalize 
   // everything to this value.
   U16 maxHeight = 0;

   Vector<const U16>::iterator iBits = mFile->mHeightMap.begin();
   for ( S32 y = 0; y < mFile->mSize; y++ )
   {
      for ( S32 x = 0; x < mFile->mSize; x++ )
      {
         if ( *iBits > maxHeight )
            maxHeight = *iBits;
         ++iBits;
      }
   }

   // Now write out the map.
   iBits = mFile->mHeightMap.begin();
   U16 *oBits = (U16*)output.getWritableBits();
   for ( S32 y = 0; y < mFile->mSize; y++ )
   {
      for ( S32 x = 0; x < mFile->mSize; x++ )
      {
         // PNG expects big endian.
         U16 height = (U16)( ( (F32)(*iBits) / (F32)maxHeight ) * (F32)U16_MAX );
         *oBits = convertHostToBEndian( height );
         ++oBits;
         ++iBits;
      }
   }

   FileStream stream;
   if ( !stream.open( filePath, Torque::FS::File::Write ) )
   {
      Con::errorf( "TerrainBlock::exportHeightMap() - Error opening file for writing: %s !", filePath );
      return false;
   }

   if ( !output.writeBitmap( format, stream ) )
   {
      Con::errorf( "TerrainBlock::exportHeightMap() - Error writing %s: %s !", format.c_str(), filePath );
      return false;
   }

   // Print out the map size in meters, so that the user 
   // knows what values to use when importing it into 
   // another terrain tool.
   S32 dim = mSquareSize * mFile->mSize;
   S32 height = fixedToFloat( maxHeight );
   Con::printf( "Saved heightmap with dimensions %d x %d x %d.", dim, dim, height );

   return true;
}

bool TerrainBlock::exportLayerMaps( const UTF8 *filePrefix, const String &format ) const
{
   for(S32 i = 0; i < mFile->mMaterials.size(); i++)
   {
      Vector<const U8>::iterator iBits = mFile->mLayerMap.begin();
      
      GBitmap output(   mFile->mSize,
                        mFile->mSize,
                        false,
                        GFXFormatA8 );

      // Copy the layer data.
      U8 *oBits = (U8*)output.getWritableBits();
      dMemset( oBits, 0, mFile->mSize * mFile->mSize );

      for ( S32 y = 0; y < mFile->mSize; y++ )
      {
         for ( S32 x = 0; x < mFile->mSize; x++ )
         {
            if(*iBits == i)
               *oBits = 0xFF;
            ++iBits;
            ++oBits;
         }
      }

      // Whats the full file name for this layer.
      UTF8 filePath[1024];
      dSprintf( filePath, 1024, "%s_%d_%s.%s", filePrefix, i, mFile->mMaterials[i]->getInternalName(), format.c_str() );

      FileStream stream;
      if ( !stream.open( filePath, Torque::FS::File::Write ) )
      {
         Con::errorf( "TerrainBlock::exportLayerMaps() - Error opening file for writing: %s !", filePath );
         return false;
      }

      if ( !output.writeBitmap( format, stream ) )
      {
         Con::errorf( "TerrainBlock::exportLayerMaps() - Error writing %s: %s !", format.c_str(), filePath );
         return false;
      }
   }

   return true;
}

DefineConsoleMethod( TerrainBlock, exportHeightMap, bool, (const char * fileNameStr, const char * format), ( "png"), "(string filename, [string format]) - export the terrain block's heightmap to a bitmap file (default: png)" )
{
   UTF8 fileName[1024];
   Con::expandScriptFilename( fileName, sizeof( fileName ), fileNameStr );

   return object->exportHeightMap( fileName, format );
}

DefineConsoleMethod( TerrainBlock, exportLayerMaps, bool, (const char * filePrefixStr, const char * format), ( "png"), "(string filePrefix, [string format]) - export the terrain block's layer maps to bitmap files (default: png)" )
{
   UTF8 filePrefix[1024];
   Con::expandScriptFilename( filePrefix, sizeof( filePrefix ), filePrefixStr );

   return object->exportLayerMaps( filePrefix, format );
}
#endif
