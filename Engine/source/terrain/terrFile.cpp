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
#include "terrain/terrFile.h"

#include "core/stream/fileStream.h"
#include "core/resourceManager.h"
#include "terrain/terrMaterial.h"
#include "gfx/gfxTextureHandle.h"
#include "gfx/bitmap/gBitmap.h"
#include "platform/profiler.h"
#include "math/mPlane.h"


template<>
void* Resource<TerrainFile>::create( const Torque::Path &path )
{
   return TerrainFile::load( path );
}

template<> ResourceBase::Signature Resource<TerrainFile>::signature()
{
   return MakeFourCC('t','e','r','d');
}


TerrainFile::TerrainFile()
   : mNeedsResaving( false ),
     mFileVersion( FILE_VERSION ),
     mSize( 256 )
{
   mLayerMap.setSize( mSize * mSize );
   dMemset( mLayerMap.address(), 0, mLayerMap.memSize() );

   mHeightMap.setSize( mSize * mSize );
   dMemset( mHeightMap.address(), 0, mHeightMap.memSize() );
}

TerrainFile::~TerrainFile()
{
}

static U16 calcDev( const PlaneF &pl, const Point3F &pt )
{
   F32 z = (pl.d + pl.x * pt.x + pl.y * pt.y) / -pl.z;
   F32 diff = z - pt.z;
   if(diff < 0.0f)
      diff = -diff;

   if(diff > 0xFFFF)
      return 0xFFFF;
   else
      return U16(diff);
}

static U16 Umax( U16 u1, U16 u2 )
{
   return u1 > u2 ? u1 : u2;
}


inline U32 getMostSignificantBit( U32 v )
{
   U32 bit = 0;

   while ( v >>= 1 )
     bit++;

   return bit;
}

void TerrainFile::_buildGridMap()
{
   // The grid level count is the same as the
   // most significant bit of the size.  While 
   // we loop we take the time to calculate the
   // grid memory pool size.
   mGridLevels = 0;
   U32 size = mSize;
   U32 poolSize = size * size;
   while ( size >>= 1 )
   {
      poolSize += size * size;
      mGridLevels++;
   }

   mGridMapPool.setSize( poolSize ); 
   mGridMapPool.compact();
   mGridMap.setSize( mGridLevels + 1 );
   mGridMap.compact();

   // Assign memory from the pool to each grid level.
   TerrainSquare *sq = mGridMapPool.address();
   for ( S32 i = mGridLevels; i >= 0; i-- )
   {
      mGridMap[i] = sq;
      sq += 1 << ( 2 * ( mGridLevels - i ) );
   }

   for( S32 i = mGridLevels; i >= 0; i-- )
   {
      S32 squareCount = 1 << ( mGridLevels - i );
      S32 squareSize = mSize / squareCount;

      for ( S32 squareX = 0; squareX < squareCount; squareX++ )
      {
         for ( S32 squareY = 0; squareY < squareCount; squareY++ )
         {
            U16 min = 0xFFFF;
            U16 max = 0;
            U16 mindev45 = 0;
            U16 mindev135 = 0;

            // determine max error for both possible splits.

            const Point3F p1(0, 0, getHeight(squareX * squareSize, squareY * squareSize));
            const Point3F p2(0, (F32)squareSize, getHeight(squareX * squareSize, squareY * squareSize + squareSize));
            const Point3F p3((F32)squareSize, (F32)squareSize, getHeight(squareX * squareSize + squareSize, squareY * squareSize + squareSize));
            const Point3F p4((F32)squareSize, 0, getHeight(squareX * squareSize + squareSize, squareY * squareSize));

            // pl1, pl2 = split45, pl3, pl4 = split135
            const PlaneF pl1(p1, p2, p3);
            const PlaneF pl2(p1, p3, p4);
            const PlaneF pl3(p1, p2, p4);
            const PlaneF pl4(p2, p3, p4);

            bool parentSplit45 = false;
            TerrainSquare *parent = NULL;
            if ( i < mGridLevels )
            {
               parent = findSquare( i+1, squareX * squareSize, squareY * squareSize );
               parentSplit45 = parent->flags & TerrainSquare::Split45;
            }

            bool empty = true;
            bool hasEmpty = false;

            for ( S32 sizeX = 0; sizeX <= squareSize; sizeX++ )
            {
               for ( S32 sizeY = 0; sizeY <= squareSize; sizeY++ )
               {
                  S32 x = squareX * squareSize + sizeX;
                  S32 y = squareY * squareSize + sizeY;

                  if(sizeX != squareSize && sizeY != squareSize)
                  {
                     if ( !isEmptyAt( x, y ) )
                        empty = false;
                     else
                        hasEmpty = true;
                  }

                  U16 ht = getHeight( x, y );
                  if ( ht < min )
                     min = ht;
                  if( ht > max )
                     max = ht;

                  Point3F pt( (F32)sizeX, (F32)sizeY, (F32)ht );
                  U16 dev;

                  if(sizeX < sizeY)
                     dev = calcDev(pl1, pt);
                  else if(sizeX > sizeY)
                     dev = calcDev(pl2, pt);
                  else
                     dev = Umax(calcDev(pl1, pt), calcDev(pl2, pt));

                  if(dev > mindev45)
                     mindev45 = dev;

                  if(sizeX + sizeY < squareSize)
                     dev = calcDev(pl3, pt);
                  else if(sizeX + sizeY > squareSize)
                     dev = calcDev(pl4, pt);
                  else
                     dev = Umax(calcDev(pl3, pt), calcDev(pl4, pt));

                  if(dev > mindev135)
                     mindev135 = dev;
               }
            }

            TerrainSquare *sq = findSquare( i, squareX * squareSize, squareY * squareSize );
            sq->minHeight = min;
            sq->maxHeight = max;

            sq->flags = empty ? TerrainSquare::Empty : 0;
            if ( hasEmpty )
               sq->flags |= TerrainSquare::HasEmpty;

            bool shouldSplit45 = ((squareX ^ squareY) & 1) == 0;
            bool split45;

            //split45 = shouldSplit45;
            if ( i == 0 )
               split45 = shouldSplit45;
            else if( i < 4 && shouldSplit45 == parentSplit45 )
               split45 = shouldSplit45;
            else
               split45 = mindev45 < mindev135;

            //split45 = shouldSplit45;
            if(split45)
            {
               sq->flags |= TerrainSquare::Split45;
               sq->heightDeviance = mindev45;
            }
            else
               sq->heightDeviance = mindev135;

            if( parent )
               if (  parent->heightDeviance < sq->heightDeviance )
                     parent->heightDeviance = sq->heightDeviance;
         }
      }
   }

   /*
   for ( S32 y = 0; y < mSize; y += 2 )
   {
      for ( S32 x=0; x < mSize; x += 2 )
      {
         GridSquare *sq = findSquare(1, Point2I(x, y));
         GridSquare *s1 = findSquare(0, Point2I(x, y));
         GridSquare *s2 = findSquare(0, Point2I(x+1, y));
         GridSquare *s3 = findSquare(0, Point2I(x, y+1));
         GridSquare *s4 = findSquare(0, Point2I(x+1, y+1));
         sq->flags |= (s1->flags | s2->flags | s3->flags | s4->flags) & ~(GridSquare::MaterialStart -1);
      }
   }
   */
}

void TerrainFile::_initMaterialInstMapping()
{
   mMaterialInstMapping.clearMatInstList();
   
   for( U32 i = 0; i < mMaterials.size(); ++ i )
   {
      Torque::Path path( mMaterials[ i ]->getDiffuseMap() );
      mMaterialInstMapping.push_back( path.getFileName() );
   }
   
   mMaterialInstMapping.mapMaterials();
}

bool TerrainFile::save( const char *filename )
{
   FileStream stream;
   stream.open( filename, Torque::FS::File::Write );
   if ( stream.getStatus() != Stream::Ok )
      return false;

   stream.write( (U8)FILE_VERSION );

   stream.write( mSize );

   // Write out the height map.
   for ( U32 i=0; i < mHeightMap.size(); i++)
      stream.write( mHeightMap[i] );

   // Write out the layer map.
   for ( U32 i=0; i < mLayerMap.size(); i++)
      stream.write( mLayerMap[i] );

   // Write out the material names.
   stream.write( (U32)mMaterials.size() );
   for ( U32 i=0; i < mMaterials.size(); i++ )
      stream.write( String( mMaterials[i]->getInternalName() ) );

   return stream.getStatus() == FileStream::Ok;
}

TerrainFile* TerrainFile::load( const Torque::Path &path )
{
   FileStream stream;

   stream.open( path.getFullPath(), Torque::FS::File::Read );
   if ( stream.getStatus() != Stream::Ok )
   {
      Con::errorf( "Resource<TerrainFile>::create - could not open '%s'", path.getFullPath().c_str() );
      return NULL;
   }

   U8 version;
   stream.read(&version);
   if (version > TerrainFile::FILE_VERSION)
   {
      Con::errorf( "Resource<TerrainFile>::create - file version '%i' is newer than engine version '%i'", version, TerrainFile::FILE_VERSION );
      return NULL;
   }

   TerrainFile *ret = new TerrainFile;
   ret->mFileVersion = version;
   ret->mFilePath = path;

   if ( version >= 7 )
      ret->_load( stream );
   else
      ret->_loadLegacy( stream );

   // Update the collision structures.
   ret->_buildGridMap();
   
   // Do the material mapping.
   ret->_initMaterialInstMapping();
   
   return ret;
}

void TerrainFile::_load( FileStream &stream )
{
   // NOTE: We read using a loop instad of in one large chunk
   // because the stream will do endian conversions for us when
   // reading one type at a time.

   stream.read( &mSize );

   // Load the heightmap.
   mHeightMap.setSize( mSize * mSize );
   for ( U32 i=0; i < mHeightMap.size(); i++ )
      stream.read( &mHeightMap[i] );

   // Load the layer index map.
   mLayerMap.setSize( mSize * mSize );
   for ( U32 i=0; i < mLayerMap.size(); i++ )
      stream.read( &mLayerMap[i] );

   // Get the material name count.
   U32 materialCount;
   stream.read( &materialCount );
   Vector<String> materials;
   materials.setSize( materialCount );

   // Load the material names.
   for ( U32 i=0; i < materialCount; i++ )
      stream.read( &materials[i] );

   // Resolve the TerrainMaterial objects from the names.
   _resolveMaterials( materials );
}

void TerrainFile::_loadLegacy(  FileStream &stream )
{
   // Some legacy constants.
   enum 
   {
      MaterialGroups = 8,
      BlockSquareWidth = 256,
   };

   const U32 sampleCount = BlockSquareWidth * BlockSquareWidth;
   mSize = BlockSquareWidth;

   // Load the heightmap.
   mHeightMap.setSize( sampleCount );
   for ( U32 i=0; i < mHeightMap.size(); i++ )
      stream.read( &mHeightMap[i] );

   // Prior to version 7 we stored this weird material struct.
   const U32 MATERIAL_GROUP_MASK = 0x7;
   struct Material 
   {
      enum Flags 
      {
         Plain          = 0,
         Rotate         = 1,
         FlipX          = 2,
         FlipXRotate    = 3,
         FlipY          = 4,
         FlipYRotate    = 5,
         FlipXY         = 6,
         FlipXYRotate   = 7,
         RotateMask     = 7,
         Empty          = 8,
         Modified       = BIT(7),

         // must not clobber TerrainFile::MATERIAL_GROUP_MASK bits!
         PersistMask       = BIT(7)
      };

      U8 flags;
      U8 index;
   };

   // Temp locals for loading before we convert to the new
   // version 7+ format.
   U8 baseMaterialMap[sampleCount] = { 0 };
   U8 *materialAlphaMap[MaterialGroups] = { 0 };
   Material materialMap[BlockSquareWidth * BlockSquareWidth];

   // read the material group map and flags...
   dMemset(materialMap, 0, sizeof(materialMap));

   AssertFatal(!(Material::PersistMask & MATERIAL_GROUP_MASK),
      "Doh! We have flag clobberage...");

   for (S32 j=0; j < sampleCount; j++)
   {
      U8 val;
      stream.read(&val);

      //
      baseMaterialMap[j] = val & MATERIAL_GROUP_MASK;
      materialMap[j].flags = val & Material::PersistMask;
   }

   // Load the material names.
   Vector<String> materials;
   for ( U32 i=0; i < MaterialGroups; i++ )
   {
      String matName;
      stream.read( &matName );
      if ( matName.isEmpty() )
         continue;

      if ( mFileVersion > 3 && mFileVersion < 6 )
      {
         // Between version 3 and 5 we store the texture file names 
         // relative to the terrain file.  We restore the full path
         // here so that we can create a TerrainMaterial from it.
         materials.push_back( Torque::Path::CompressPath( mFilePath.getRoot() + mFilePath.getPath() + '/' + matName ) );
      }
      else
         materials.push_back( matName );
   }

   if ( mFileVersion <= 3 )
   {
      GFXTexHandle terrainMat;
      Torque::Path matRelPath;

      // Try to automatically fix up our material file names
      for (U32 i = 0; i < materials.size(); i++)
      {
         if ( materials[i].isEmpty() )
            continue;
            
         terrainMat.set( materials[i], &GFXDefaultPersistentProfile, avar( "%s() - (line %d)", __FUNCTION__, __LINE__ ) );
         if ( terrainMat )
            continue;

         matRelPath = materials[i];

         String path = matRelPath.getPath();

         String::SizeType n = path.find( '/', 0, String::NoCase );
         if ( n != String::NPos )
         {
            matRelPath.setPath( String(Con::getVariable( "$defaultGame" )) + path.substr( n, path.length() - n ) );

            terrainMat.set( matRelPath, &GFXDefaultPersistentProfile, avar( "%s() - (line %d)", __FUNCTION__, __LINE__ ) );
            if ( terrainMat )
            {
               materials[i] = matRelPath.getFullPath();
               mNeedsResaving = true;
            }
         }
         
      } // for (U32 i = 0; i < TerrainBlock::MaterialGroups; i++)
      
   } // if ( mFileVersion <= 3 )

   if ( mFileVersion == 1 )
   {
      for( S32 j = 0; j < sampleCount; j++ )
      {
         if ( materialAlphaMap[baseMaterialMap[j]] == NULL )
         {
            materialAlphaMap[baseMaterialMap[j]] = new U8[sampleCount];
            dMemset(materialAlphaMap[baseMaterialMap[j]], 0, sampleCount);
         }

         materialAlphaMap[baseMaterialMap[j]][j] = 255;
      }
   }
   else
   {
      for( S32 k=0; k < materials.size(); k++ )
      {
         AssertFatal(materialAlphaMap[k] == NULL, "Bad assumption.  There should be no alpha map at this point...");
         materialAlphaMap[k] = new U8[sampleCount];
         stream.read(sampleCount, materialAlphaMap[k]);
      }
   }

   // Throw away the old texture and heightfield scripts.
   if ( mFileVersion >= 3 )
   {
      U32 len;
      stream.read(&len);
      char *textureScript = (char *)dMalloc(len + 1);
      stream.read(len, textureScript);
      dFree( textureScript );

      stream.read(&len);
      char *heightfieldScript = (char *)dMalloc(len + 1);
      stream.read(len, heightfieldScript);
      dFree( heightfieldScript );
   }

   // Load and throw away the old edge terrain paths.
   if ( mFileVersion >= 5 )
   {
      stream.readSTString(true);
      stream.readSTString(true);
   }

   U32 layerCount = materials.size() - 1;

   // Ok... time to convert all this mess to the layer index map!
   for ( U32 i=0; i < sampleCount; i++ )
   {
      // Find the greatest layer.
      U32 layer = 0;
      U32 lastValue = 0;
      for ( U32 k=0; k < MaterialGroups; k++ )
      {
         if ( materialAlphaMap[k] && materialAlphaMap[k][i] > lastValue )
         {
            layer = k;
            lastValue = materialAlphaMap[k][i];
         }
      }

      // Set the layer index.   
      mLayerMap[i] = getMin( layer, layerCount );
   }
   
   // Cleanup.
   for ( U32 i=0; i < MaterialGroups; i++ )
      delete [] materialAlphaMap[i];

   // Force resaving on these old file versions.
   //mNeedsResaving = false;

   // Resolve the TerrainMaterial objects from the names.
   _resolveMaterials( materials );
}

void TerrainFile::_resolveMaterials( const Vector<String> &materials )
{
   mMaterials.clear();

   for ( U32 i=0; i < materials.size(); i++ )
      mMaterials.push_back( TerrainMaterial::findOrCreate( materials[i] ) );

   // If we didn't get any materials then at least
   // add a warning material so we will render.
   if ( mMaterials.empty() )
      mMaterials.push_back( TerrainMaterial::getWarningMaterial() );
}

void TerrainFile::setSize( U32 newSize, bool clear )
{
   // Make sure the resolution is a power of two.
   newSize = getNextPow2( newSize );

   // 
   if ( clear )
   {
      mLayerMap.setSize( newSize * newSize );
      mLayerMap.compact();
      dMemset( mLayerMap.address(), 0, mLayerMap.memSize() );

      // Initialize the elevation to something above
      // zero so that we have room to excavate by default.
      U16 elev = floatToFixed( 512.0f );

      mHeightMap.setSize( newSize * newSize );
      mHeightMap.compact();
      for ( U32 i = 0; i < mHeightMap.size(); i++ )
         mHeightMap[i] = elev;
   }
   else
   {
      // We're resizing here!



   }

   mSize = newSize;

   _buildGridMap();
}

void TerrainFile::smooth( F32 factor, U32 steps, bool updateCollision )
{
   const U32 blockSize = mSize * mSize;

   // Grab some temp buffers for our smoothing results.
   Vector<F32> h1, h2;
   h1.setSize( blockSize );
   h2.setSize( blockSize );

   // Fill the first buffer with the current heights.   
   for ( U32 i=0; i < blockSize; i++ )
      h1[i] = (F32)mHeightMap[i];

   // factor of 0.0 = NO Smoothing
   // factor of 1.0 = MAX Smoothing
   const F32 matrixM = 1.0f - getMax(0.0f, getMin(1.0f, factor));
   const F32 matrixE = (1.0f-matrixM) * (1.0f/12.0f) * 2.0f;
   const F32 matrixC = matrixE * 0.5f;

   // Now loop for our interations.
   F32 *src = h1.address();
   F32 *dst = h2.address();
   for ( U32 s=0; s < steps; s++ )
   {
      for ( S32 y=0; y < mSize; y++ )
      {
         for ( S32 x=0; x < mSize; x++ )
         {
            F32 samples[9];

            S32 c = 0;
            for (S32 i = y-1; i < y+2; i++)
               for (S32 j = x-1; j < x+2; j++)
               {
                  if ( i < 0 || j < 0 || i >= mSize || j >= mSize )
                     samples[c++] = src[ x + ( y * mSize ) ];
                  else
                     samples[c++] = src[ j + ( i * mSize ) ];
               }

            //  0  1  2
            //  3 x,y 5
            //  6  7  8

            dst[ x + ( y * mSize ) ] =
               ((samples[0]+samples[2]+samples[6]+samples[8]) * matrixC) +
               ((samples[1]+samples[3]+samples[5]+samples[7]) * matrixE) +
               (samples[4] * matrixM);
         }
      }

      // Swap!
      F32 *tmp = dst;
      dst = src;
      src = tmp;
   }

   // Copy the results back to the height map.
   for ( U32 i=0; i < blockSize; i++ )
      mHeightMap[i] = (U16)mCeil( (F32)src[i] );

   if ( updateCollision )
      _buildGridMap();
}

void TerrainFile::setHeightMap( const Vector<U16> &heightmap, bool updateCollision )
{
   AssertFatal( mHeightMap.size() == heightmap.size(), "TerrainFile::setHeightMap - Incorrect heightmap size!" );
   dMemcpy( mHeightMap.address(), heightmap.address(), mHeightMap.size() ); 

   if ( updateCollision )
      _buildGridMap();
}

void TerrainFile::import(  const GBitmap &heightMap, 
                           F32 heightScale,
                           const Vector<U8> &layerMap, 
                           const Vector<String> &materials,
                           bool flipYAxis )
{
   AssertFatal( heightMap.getWidth() == heightMap.getHeight(), "TerrainFile::import - Height map is not square!" );
   AssertFatal( isPow2( heightMap.getWidth() ), "TerrainFile::import - Height map is not power of two!" );

   const U32 newSize = heightMap.getWidth();
   if ( newSize != mSize )
   {
      mHeightMap.setSize( newSize * newSize );
      mHeightMap.compact();
      mSize = newSize;
   }

   // Convert the height map to heights.
   U16 *oBits = mHeightMap.address();
   if ( heightMap.getFormat() == GFXFormatR5G6B5 )
   {
      const F32 toFixedPoint = ( 1.0f / (F32)U16_MAX ) * floatToFixed( heightScale );
      const U16 *iBits = (const U16*)heightMap.getBits();
      if ( flipYAxis )
      {
         for ( U32 i = 0; i < mSize * mSize; i++ )
         {
            U16 height = convertBEndianToHost( *iBits );
            *oBits = (U16)mCeil( (F32)height * toFixedPoint );
            ++oBits;
            ++iBits;
         }
      }
      else
      {
         for(S32 y = mSize - 1; y >= 0; y--) {
            for(U32 x = 0; x < mSize; x++) {
               U16 height = convertBEndianToHost( *iBits );
               mHeightMap[x + y * mSize] = (U16)mCeil( (F32)height * toFixedPoint );
               ++iBits;
            }
         }
      }
   }
   else
   {
      const F32 toFixedPoint = ( 1.0f / (F32)U8_MAX ) * floatToFixed( heightScale );
      const U8 *iBits = heightMap.getBits();
      if ( flipYAxis )
      {
         for ( U32 i = 0; i < mSize * mSize; i++ )
         {
            *oBits = (U16)mCeil( ((F32)*iBits) * toFixedPoint );
            ++oBits;
            iBits += heightMap.getBytesPerPixel();
         }
      }
      else
      {
         for(S32 y = mSize - 1; y >= 0; y--) {
            for(U32 x = 0; x < mSize; x++) {
               mHeightMap[x + y * mSize] = (U16)mCeil( ((F32)*iBits) * toFixedPoint );
               iBits += heightMap.getBytesPerPixel();
            }
         }
      }
   }

   // Copy over the layer map.
   AssertFatal( layerMap.size() == mHeightMap.size(), "TerrainFile::import - Layer map is the wrong size!" );
   mLayerMap = layerMap;
   mLayerMap.compact();

   // Resolve the materials.
   _resolveMaterials( materials );

   // Rebuild the collision grid map.
   _buildGridMap();
}


void TerrainFile::create(  String *inOutFilename, 
                           U32 newSize, 
                           const Vector<String> &materials )
{
   // Determine the path and basename - first try using the input filename (mission name)
   Torque::Path basePath( *inOutFilename );
   if ( !basePath.getExtension().equal("mis") )
   {
      // Use the default path and filename
      String terrainDirectory( Con::getVariable( "$pref::Directories::Terrain" ) );
      if ( terrainDirectory.isEmpty() )
      {
         terrainDirectory = "art/terrains";
      }
      basePath.setPath( terrainDirectory );
      basePath.setFileName( "terrain" );
   }

   // Construct a default file name
   (*inOutFilename) = Torque::FS::MakeUniquePath( basePath.getRootAndPath(), basePath.getFileName(), "ter" );

   // Create the file
   TerrainFile *file = new TerrainFile;

   for ( U32 i=0; i < materials.size(); i++ )
      file->mMaterials.push_back( TerrainMaterial::findOrCreate( materials[i] ) );

   file->setSize( newSize, true );
   file->save( *inOutFilename );

   delete file;
}

inline void getMinMax( U16 &inMin, U16 &inMax, U16 height )
{
   if ( height < inMin )
      inMin = height;
   if ( height > inMax )
      inMax = height;
}

inline void checkSquare( TerrainSquare *parent, const TerrainSquare *child )
{
   if(parent->minHeight > child->minHeight)
      parent->minHeight = child->minHeight;
   if(parent->maxHeight < child->maxHeight)
      parent->maxHeight = child->maxHeight;

   if ( child->flags & (TerrainSquare::Empty | TerrainSquare::HasEmpty) )
      parent->flags |= TerrainSquare::HasEmpty;
}

void TerrainFile::updateGrid( const Point2I &minPt, const Point2I &maxPt )
{
   // here's how it works:
   // for the current terrain renderer we only care about
   // the minHeight and maxHeight on the GridSquare
   // so we do one pass through, updating minHeight and maxHeight
   // on the level 0 squares, then we loop up the grid map from 1 to
   // the top, expanding the bounding boxes as necessary.
   // this should end up being way, way, way, way faster for the terrain
   // editor

   PROFILE_SCOPE( TerrainFile_UpdateGrid );

   for ( S32 y = minPt.y - 1; y < maxPt.y + 1; y++ )
   {
      for ( S32 x = minPt.x - 1; x < maxPt.x + 1; x++ )
      {
         S32 px = x;
         S32 py = y;
         if ( px < 0 )
            px += mSize;
         if ( py < 0 )
            py += mSize;

         TerrainSquare *sq = findSquare( 0, px, py );
         sq->minHeight = 0xFFFF;
         sq->maxHeight = 0;

         // Update the empty state.
         if ( isEmptyAt( x, y ) )
            sq->flags |= TerrainSquare::Empty;
         else
            sq->flags &= ~TerrainSquare::Empty;

         getMinMax( sq->minHeight, sq->maxHeight, getHeight( x, y ) );
         getMinMax( sq->minHeight, sq->maxHeight, getHeight( x+1, y ) );
         getMinMax( sq->minHeight, sq->maxHeight, getHeight( x, y+1 ) );
         getMinMax( sq->minHeight, sq->maxHeight, getHeight( x+1, y+1 ) );
      }
   }

   // ok, all the level 0 grid squares are updated:
   // now update all the parent grid squares that need to be updated:
   for( S32 level = 1; level <= mGridLevels; level++ )
   {
      S32 size = 1 << level;
      S32 halfSize = size >> 1;  

      for( S32 y = (minPt.y - 1) >> level; y < (maxPt.y + size) >> level; y++ )
      {
         for ( S32 x = (minPt.x - 1) >> level; x < (maxPt.x + size) >> level; x++ )
         {
            S32 px = x << level;
            S32 py = y << level;

            TerrainSquare *sq = findSquare(level, px, py);
            sq->minHeight = 0xFFFF;
            sq->maxHeight = 0;
            sq->flags &= ~( TerrainSquare::Empty | TerrainSquare::HasEmpty );

            checkSquare( sq, findSquare( level - 1, px, py ) );
            checkSquare( sq, findSquare( level - 1, px + halfSize, py ) );
            checkSquare( sq, findSquare( level - 1, px, py + halfSize ) );
            checkSquare( sq, findSquare( level - 1, px + halfSize, py + halfSize ) );
         }
      }
   }
}
