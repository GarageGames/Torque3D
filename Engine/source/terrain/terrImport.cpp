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
#include "sim/netConnection.h"
#include "core/strings/stringUnit.h"
#include "core/resourceManager.h"
#include "gui/worldEditor/terrainEditor.h"
#include "util/noise2d.h"
#include "core/volume.h"

using namespace Torque;

ConsoleStaticMethod( TerrainBlock, createNew, S32, 5, 5, 
   "TerrainBlock.create( String terrainName, U32 resolution, String materialName, bool genNoise )\n"
   "" )
{
   const UTF8 *terrainName = argv[1];
   U32 resolution = dAtoi( argv[2] );
   const UTF8 *materialName = argv[3];
   bool genNoise = dAtob( argv[4] );

   Vector<String> materials;
   materials.push_back( materialName );

   TerrainBlock *terrain = new TerrainBlock();

   // We create terrains based on level name. If the user wants to rename the terrain names; they have to
   // rename it themselves in their file browser. The main reason for this is so we can easily increment for ourselves;
   // and because its too easy to rename the terrain object and forget to take care of the terrain filename afterwards.
   FileName terrFileName( Con::getVariable("$Client::MissionFile") );
   String terrainDirectory( Con::getVariable( "$pref::Directories::Terrain" ) );
   if ( terrainDirectory.isEmpty() )
   {
      terrainDirectory = "art/terrains/";
   }
   terrFileName.replace("tools/levels/", terrainDirectory);
   terrFileName.replace("levels/", terrainDirectory);

   TerrainFile::create( &terrFileName, resolution, materials );

   if( !terrain->setFile( terrFileName ) )
   {
      Con::errorf( "TerrainBlock::createNew - error creating '%s'", terrFileName.c_str() );
      return 0;
   }
   
   terrain->setPosition( Point3F( 0, 0, 0 ) );

   const U32 blockSize = terrain->getBlockSize();

   if ( genNoise )
   {
      TerrainFile *file = terrain->getFile();

      Vector<F32> floatHeights;
      floatHeights.setSize( blockSize * blockSize );

      Noise2D noise;
      noise.setSeed( 134208587 );
      
      // Set up some defaults.
      F32 octaves = 3.0f;
      U32 freq = 4;
      F32 roughness = 0.0f;
      noise.fBm( &floatHeights, blockSize, freq, 1.0f - roughness, octaves );

      F32 height = 0;

      F32 omax, omin;
      noise.getMinMax( &floatHeights, &omin, &omax, blockSize );
         
      F32 terrscale = 300.0f / (omax - omin);
      for ( S32 y = 0; y < blockSize; y++ )
      {
         for ( S32 x = 0; x < blockSize; x++ )
         {
            // Very important to subtract the min
            // noise value when using the noise functions
            // for terrain, otherwise floatToFixed() will
            // wrap negative values to U16_MAX, creating
            // a very ugly terrain.
            height = (floatHeights[ x + (y * blockSize) ] - omin) * terrscale + 30.0f;
            file->setHeight( x, y, floatToFixed( height ) );
         }
      }

      terrain->updateGrid( Point2I::Zero, Point2I( blockSize, blockSize ) );
      terrain->updateGridMaterials( Point2I::Zero, Point2I( blockSize, blockSize ) );
   }

   terrain->registerObject( terrainName );

   // Add to mission group!
   SimGroup *missionGroup;
   if( Sim::findObject( "MissionGroup", missionGroup ) )
      missionGroup->addObject( terrain );

   return terrain->getId();
}

ConsoleStaticMethod( TerrainBlock, import, S32, 7, 8, 
   "( String terrainName, String heightMap, F32 metersPerPixel, F32 heightScale, String materials, String opacityLayers[, bool flipYAxis=true] )\n"
   "" )
{
   // Get the parameters.
   const UTF8 *terrainName = argv[1];
   const UTF8 *hmap = argv[2];
   F32 metersPerPixel = dAtof(argv[3]);
   F32 heightScale = dAtof(argv[4]);
   const UTF8 *opacityFiles = argv[5];
   const UTF8 *materialsStr = argv[6];
   bool flipYAxis = argc == 8? dAtob(argv[7]) : true;

   // First load the height map and validate it.
   Resource<GBitmap> heightmap = GBitmap::load( hmap );
   if ( !heightmap )
   {
      Con::errorf( "Heightmap failed to load!" );
      return 0;
   }

   U32 terrSize = heightmap->getWidth();
   U32 hheight = heightmap->getHeight();
   if ( terrSize != hheight || !isPow2( terrSize ) )
   {
      Con::errorf( "Height map must be square and power of two in size!" );
      return 0;
   }
   else if ( terrSize < 128 || terrSize > 4096 )
   {
      Con::errorf( "Height map must be between 128 and 4096 in size!" );
      return 0;
   }

   U32 fileCount = StringUnit::getUnitCount( opacityFiles, "\n" ); 
   Vector<U8> layerMap;
   layerMap.setSize( terrSize * terrSize );
   {
      Vector<GBitmap*> bitmaps;
  
      for ( U32 i = 0; i < fileCount; i++ )
      {
         String fileNameWithChannel = StringUnit::getUnit( opacityFiles, i, "\n" );
         String fileName = StringUnit::getUnit( fileNameWithChannel, 0, "\t" );
         String channel = StringUnit::getUnit( fileNameWithChannel, 1, "\t" );
         
         if ( fileName.isEmpty() )
            continue;

         if ( !channel.isEmpty() )
         {
            // Load and push back the bitmap here.
            Resource<GBitmap> opacityMap = ResourceManager::get().load( fileName );
            if ( terrSize != opacityMap->getWidth() || terrSize != opacityMap->getHeight() )
            {
               Con::errorf( "The opacity map '%s' doesn't match height map size!", fileName.c_str() );
               return 0;
            }

            // Always going to be one channel.
            GBitmap *opacityMapChannel = new GBitmap( terrSize, 
                                                      terrSize, 
                                                      false, 
                                                      GFXFormatA8 );         

            if ( opacityMap->getBytesPerPixel() > 1 )
            {
               if ( channel.equal( "R", 1 ) )
                  opacityMap->copyChannel( 0, opacityMapChannel );
               else if ( channel.equal( "G", 1 ) )
                  opacityMap->copyChannel( 1, opacityMapChannel );
               else if ( channel.equal( "B", 1 ) )
                  opacityMap->copyChannel( 2, opacityMapChannel );
               else if ( channel.equal( "A", 1 ) )
                  opacityMap->copyChannel( 3, opacityMapChannel );

               bitmaps.push_back( opacityMapChannel );
            }
            else
            {
               opacityMapChannel->copyRect( opacityMap, RectI( 0, 0, terrSize, terrSize ), Point2I( 0, 0 ) );
               bitmaps.push_back( opacityMapChannel );
            }
         }
      }

      // Ok... time to convert all this opacity layer 
      // mess to the layer index map!
      U32 layerCount = bitmaps.size() - 1;
      U32 layer, lastValue;
      U8 value;
      for ( U32 i = 0; i < terrSize * terrSize; i++ )
      {
         // Find the greatest layer.
         layer = lastValue = 0;
         for ( U32 k=0; k < bitmaps.size(); k++ )
         {
            value = bitmaps[k]->getBits()[i];
            if ( value >= lastValue )
            {
               layer = k;
               lastValue = value;
            }
         }

         // Set the layer index.   
         layerMap[i] = getMin( layer, layerCount );
      }

      // Cleanup the bitmaps.
      for ( U32 i=0; i < bitmaps.size(); i++ )
         delete bitmaps[i];
   }

   U32 matCount = StringUnit::getUnitCount( materialsStr, "\t\n" );
   if( matCount != fileCount)
   {
      Con::errorf("Number of Materials and Layer maps must be equal.");
      return 0;
   }

   Vector<String> materials;
   for ( U32 i = 0; i < matCount; i++ )
   {
      String matStr = StringUnit::getUnit( materialsStr, i, "\t\n" );
      // even if matStr is empty, insert it as a placeholder (will be replaced with warning material later)
      materials.push_back( matStr );
   }

   // Do we have an existing terrain with that name... then update it!
   TerrainBlock *terrain = dynamic_cast<TerrainBlock*>( Sim::findObject( terrainName ) );
   if ( terrain )
      terrain->import( (*heightmap), heightScale, metersPerPixel, layerMap, materials, flipYAxis );
   else
   {
      terrain = new TerrainBlock();
      terrain->assignName( terrainName );
      terrain->import( (*heightmap), heightScale, metersPerPixel, layerMap, materials, flipYAxis );
      terrain->registerObject();

      // Add to mission group!
      SimGroup *missionGroup;
      if (  Sim::findObject( "MissionGroup", missionGroup ) )
         missionGroup->addObject( terrain );
   }

   return terrain->getId();
}

bool TerrainBlock::import( const GBitmap &heightMap, 
                           F32 heightScale, 
                           F32 metersPerPixel,
                           const Vector<U8> &layerMap, 
                           const Vector<String> &materials,
                           bool flipYAxis)
{
   AssertFatal( isServerObject(), "TerrainBlock::import - This should only be called on the server terrain!" );

   AssertFatal( heightMap.getWidth() == heightMap.getHeight(), "TerrainBlock::import - Height map is not square!" );
   AssertFatal( isPow2( heightMap.getWidth() ), "TerrainBlock::import - Height map is not power of two!" );

   // If we don't have a terrain file then add one.
   if ( !mFile )
   {
      // Get a unique file name for the terrain.
      String fileName( getName() );
      if ( fileName.isEmpty() )
         fileName = "terrain";
      mTerrFileName = FS::MakeUniquePath( "levels", fileName, "ter" );

      // TODO: We have to save and reload the file to get
      // it into the resource system.  This creates lots
      // of temporary unused files when the terrain is
      // discarded because of undo or quit.
      TerrainFile *file = new TerrainFile;
      file->save( mTerrFileName );
      delete file;
      mFile = ResourceManager::get().load( mTerrFileName );
   }

   // The file does a bunch of the work.
   mFile->import( heightMap, heightScale, layerMap, materials, flipYAxis );

   // Set the square size.
   mSquareSize = metersPerPixel;

   if ( isProperlyAdded() )
   {
      // Update the server bounds.
      _updateBounds();

      // Make sure the client gets updated.
      setMaskBits( HeightMapChangeMask | SizeMask );
   }

   return true;
}

