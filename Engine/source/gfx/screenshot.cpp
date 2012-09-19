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
#include "gfx/screenshot.h"

#include "math/util/frustum.h"
#include "core/stream/fileStream.h"
#include "gui/core/guiCanvas.h"
#include "gfx/bitmap/pngUtils.h"
#include "console/engineAPI.h"


// Note: This will be initialized by the device.
ScreenShot *gScreenShot = NULL;

inline void sBlendPixelRGB888( U8* src, U8* dst, F32 factor )
{   
   U32 inFactor = factor * BIT(8);
   U32 outFactor = BIT(8) - inFactor;
   
   dst[0] = ((U32)src[0]*inFactor + (U32)dst[0]*outFactor) >> 8;
   dst[1] = ((U32)src[1]*inFactor + (U32)dst[1]*outFactor) >> 8;
   dst[2] = ((U32)src[2]*inFactor + (U32)dst[2]*outFactor) >> 8;
}


ScreenShot::ScreenShot()
   :  mPending( false ),
      mWriteJPG( false ),
      mCurrTile( 0, 0 ),
      mTiles( 1 )
{
   mFilename[0] = 0;
}

void ScreenShot::setPending( const char *filename, bool writeJPG, S32 tiles, F32 overlap )
{
   dStrcpy( mFilename, filename );
   mWriteJPG = writeJPG;
   mTiles = getMax( tiles, 1 );
   mPixelOverlap.set(getMin(overlap, 0.25f), getMin(overlap, 0.25f));      

   mPending = true;
}
   
void ScreenShot::tileFrustum( Frustum& frustum )
{
   AssertFatal( mPending, "ScreenShot::tileFrustum() - This should only be called during screenshots!" );

   // We do not need to make changes on a single tile.
   if ( mTiles == 1 )
      return;

   frustum.tileFrustum(mTiles, mCurrTile, mFrustumOverlap);
}

void ScreenShot::tileGui( const Point2I &screenSize )
{
   AssertFatal( mPending, "ScreenShot::tileGui() - This should only be called during screenshots!" );

   // We do not need to make changes on a single tile.
   if ( mTiles == 1 )
      return;
   
   GFX->setWorldMatrix( MatrixF::Identity );

   S32 currTileX = mCurrTile.x;
   S32 currTileY = (S32)mTiles - mCurrTile.y - 1; //Vertically flipped tile index

   MatrixF tileMat( true );
   Point3F tilePos(0,0,0);
   tilePos.x = currTileX * (-screenSize.x) + mPixelOverlap.x * screenSize.x * (currTileX * 2 + 1);   
   tilePos.y = currTileY * (-screenSize.y) + mPixelOverlap.y * screenSize.y * (currTileY * 2 + 1);
   
   tileMat.setPosition( tilePos + Point3F(0.5f, 0.5f, 0) );
   tileMat.scale( Point3F( (F32)mTiles * (1-mPixelOverlap.x*2), (F32)mTiles * (1-mPixelOverlap.y*2), 0 ) );

   GFX->setViewMatrix( tileMat );
}

void ScreenShot::capture( GuiCanvas *canvas )
{
   AssertFatal( mPending, "ScreenShot::capture() - The capture wasn't pending!" );

   if ( mTiles == 1 )
   {
      _singleCapture( canvas );
      return;
   }

   char filename[256];

   Point2I canvasSize = canvas->getPlatformWindow()->getVideoMode().resolution;
   
   // Calculate the real final size taking overlap in account
   Point2I overlapPixels( canvasSize.x * mPixelOverlap.x, canvasSize.y * mPixelOverlap.y );   

   // Calculate the overlap to be used by the frustum tiling, 
   // so it properly expands the frustum to overlap the amount of pixels we want
   mFrustumOverlap.x = ((F32)canvasSize.x/(canvasSize.x - overlapPixels.x*2)) * ((F32)overlapPixels.x/(F32)canvasSize.x);
   mFrustumOverlap.y = ((F32)canvasSize.y/(canvasSize.y - overlapPixels.y*2)) * ((F32)overlapPixels.y/(F32)canvasSize.y);
   
   //overlapPixels.set(0,0);
   // Get a buffer to write a row of tiles into.
   GBitmap *outBuffer = new GBitmap( canvasSize.x * mTiles - overlapPixels.x * mTiles * 2 , canvasSize.y - overlapPixels.y );

   // Open up the file on disk.
   dSprintf( filename, 256, "%s.%s", mFilename, "png" );
   FileStream fs;
   if ( !fs.open( filename, Torque::FS::File::Write ) )
      Con::errorf( "ScreenShot::capture() - Failed to open output file '%s'!", filename );

   // Open a PNG stream for the final image
   DeferredPNGWriter pngWriter;
   pngWriter.begin(outBuffer->getFormat(), outBuffer->getWidth(), canvasSize.y * mTiles - overlapPixels.y * mTiles * 2, fs, 0);
   
   // Render each tile to generate a huge screenshot.
   for( U32 ty=0; ty < mTiles; ty++ )
   {
      for( S32 tx=0; tx < mTiles; tx++ )
      {
         // Set the current tile offset for tileFrustum().
         mCurrTile.set( tx, mTiles - ty - 1 );

         // Let the canvas render the scene.
         canvas->renderFrame( false );

         // Now grab the current back buffer.
         GBitmap *gb = _captureBackBuffer();

			// The current GFX device couldn't capture the backbuffer or it's unable of doing so.
			if (gb == NULL)
				return;

                  
         // Copy the captured bitmap into its tile
         // within the output bitmap.         
         const U32 inStride = gb->getWidth() * gb->getBytesPerPixel();         
         const U8 *inColor = gb->getBits() + inStride * overlapPixels.y;
         const U32 outStride = outBuffer->getWidth() * outBuffer->getBytesPerPixel();
         const U32 inOverlapOffset = overlapPixels.x * gb->getBytesPerPixel();         
         const U32 inOverlapStride = overlapPixels.x * gb->getBytesPerPixel()*2;
         const U32 outOffset = (tx * (gb->getWidth() - overlapPixels.x*2 )) * gb->getBytesPerPixel();
         U8 *outColor = outBuffer->getWritableBits() + outOffset;
         for( U32 row=0; row < gb->getHeight() - overlapPixels.y; row++ )
         {
            dMemcpy( outColor, inColor + inOverlapOffset, inStride - inOverlapStride );
            
            //Grandient blend the left overlap area of this tile over the previous tile left border
            if (tx && !(ty && row < overlapPixels.y))
            {
               U8 *blendOverlapSrc = (U8*)inColor;
               U8 *blendOverlapDst = outColor - inOverlapOffset;
               for ( U32 px=0; px < overlapPixels.x; px++)
               {
                  F32 blendFactor = (F32)px / (F32)overlapPixels.x;
                  sBlendPixelRGB888(blendOverlapSrc, blendOverlapDst, blendFactor);                 

                  blendOverlapSrc += gb->getBytesPerPixel();
                  blendOverlapDst += outBuffer->getBytesPerPixel();                   
               }               
            }

            //Gradient blend against the rows the excess overlap rows already in the buffer            
            if (ty && row < overlapPixels.y)
            {
               F32 rowBlendFactor = (F32)row / (F32)overlapPixels.y;
               U8 *blendSrc = outColor + outStride * (outBuffer->getHeight() - overlapPixels.y);
               U8 *blendDst = outColor;               
               for ( U32 px=0; px < gb->getWidth() - overlapPixels.x*2; px++)
               {                  
                  sBlendPixelRGB888(blendSrc, blendDst, 1.0-rowBlendFactor); 
                  blendSrc += gb->getBytesPerPixel();
                  blendDst += outBuffer->getBytesPerPixel();                   
               }                              
            }

            
            inColor += inStride;
            outColor += outStride;
         }

         delete gb;
      }

      // Write the captured tile row into the PNG stream
      pngWriter.append(outBuffer, outBuffer->getHeight()-overlapPixels.y);
   }

   //Close the PNG stream
   pngWriter.end();
   
   // We captured... clear the flag.
   mPending = false;
}


void ScreenShot::_singleCapture( GuiCanvas *canvas )
{
   // Let the canvas render the scene.
   canvas->renderFrame( false );

   // Now grab the current back buffer.
   GBitmap *bitmap = _captureBackBuffer();

	// The current GFX device couldn't capture the backbuffer or it's unable of doing so.
	if (bitmap == NULL)
		return;

   // We captured... clear the flag.
   mPending = false;

   // We gotta attach the extension ourselves.
   char filename[256];
   dSprintf( filename, 256, "%s.%s", mFilename, mWriteJPG ? "jpg" : "png" );

   // Open up the file on disk.
   FileStream fs;
   if ( !fs.open( filename, Torque::FS::File::Write ) )
      Con::errorf( "ScreenShot::_singleCapture() - Failed to open output file '%s'!", filename );
   else
   {
      // Write it and close.
      if ( mWriteJPG )
         bitmap->writeBitmap( "jpg", fs );
      else
         bitmap->writeBitmap( "png", fs );

      fs.close();
   }

   // Cleanup.
   delete bitmap;
}


DefineEngineFunction( screenShot, void, 
   ( const char *file, const char *format, U32 tileCount, F32 tileOverlap ),
   ( 1, 0 ),
   "Takes a screenshot with optional tiling to produce huge screenshots.\n"
   "@param file The output image file path.\n"
   "@param format Either JPEG or PNG.\n"
   "@param tileCount If greater than 1 will tile the current screen size to take a large format screenshot.\n"
   "@param tileOverlap The amount of horizontal and vertical overlap between the tiles used to remove tile edge artifacts from post effects.\n"
   "@ingroup GFX\n" )
{
   if ( !gScreenShot )
   {
      Con::errorf( "Screenshot module not initialized by device" );
      return;
   }

	Torque::Path ssPath( file );
   Torque::FS::CreatePath( ssPath );
   Torque::FS::FileSystemRef fs = Torque::FS::GetFileSystem(ssPath);
   Torque::Path newPath = fs->mapTo(ssPath);

   gScreenShot->setPending(   newPath.getFullPath(), 
                              dStricmp( format, "JPEG" ) == 0,
                              tileCount, 
                              tileOverlap );
}

