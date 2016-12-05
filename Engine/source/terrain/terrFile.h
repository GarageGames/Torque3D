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

#ifndef _TERRFILE_H_
#define _TERRFILE_H_

#ifndef _TVECTOR_H_
#include <core/util/tVector.h>
#endif
#ifndef _PATH_H_
#include <core/util/path.h>
#endif
#ifndef _MATERIALLIST_H_
#include "materials/materialList.h"
#endif
#ifndef _TERRMATERIAL_H_
#include "terrain/terrMaterial.h"
#endif

class TerrainMaterial;
class FileStream;
class GBitmap;


///
struct TerrainSquare
{
   F32 minHeight;
   F32 maxHeight;
   F32 heightDeviance;

   U16 flags;

   enum
   {
      Split45  = BIT(0),
      Empty    = BIT(1),
      HasEmpty = BIT(2),
   };
};


/// NOTE:  The terrain uses natural heights.
typedef F32 TerrainHeight;


/// 
class TerrainFile
{
protected:

   friend class TerrainBlock;

   /// The materials used to render the terrain.
   Vector<TerrainMaterial*> mMaterials;

   /// The dimensions of the layer and height maps.
   U32 mSize;

   /// The layer index at each height map sample.
   Vector<U8> mLayerMap;

   /// The natural height map.
   Vector<F32> mHeightMap;

   /// The memory pool used by the grid map layers.
   Vector<TerrainSquare> mGridMapPool;

   ///
   U32 mGridLevels;

   /// The grid map layers used to accelerate collision
   /// queries for the height map data.
   Vector<TerrainSquare*> mGridMap;
   
   /// MaterialList used to map terrain materials to material instances for the
   /// sake of collision (physics, etc.).
   MaterialList mMaterialInstMapping;

   /// The file version.
   U32 mFileVersion;     

   /// The dirty flag.
   bool mNeedsResaving;  

   /// The full path and name of the TerrainFile
   Torque::Path mFilePath;

   /// The internal loading function.
   void _load( FileStream &stream );

   /// The legacy file loading code.
   void _loadLegacy( FileStream &stream );

   // Load terrain from the format a version 7.
   void _loadV7( FileStream &stream );

   /// Used to populate the materail vector by finding the 
   /// TerrainMaterial objects by name.
   void _resolveMaterials( const Vector<String> &materials );

   /// 
   void _buildGridMap();
   
   ///
   void _initMaterialInstMapping();

public:

   enum Constants
   {
      // V7  Store heights as int. Limitation on height is 2047.
      // V8  Store heights as float. Any height is set up (natural height).
      //     @todo Verify a behavior with the negative heights.
      // @see save(), load()
      FILE_VERSION = 8
   };

   TerrainFile();

   virtual ~TerrainFile();

   ///
   static void create(  String *inOutFilename, 
                        U32 newSize, 
                        const Vector<String> &materials );

   ///
   static TerrainFile* load( const Torque::Path &path );

   bool save( const char *filename );

   // @return 0 when file is inaccessible.
   static U8 version( const Torque::Path &path );

   ///
   void import(   const GBitmap &heightMap, 
                  F32 heightScale,
                  const Vector<U8> &layerMap, 
                  const Vector<String> &materials,
                  bool flipYAxis = true );

   /// Updates the terrain grid for the specified area.
   void updateGrid( const Point2I &minPt, const Point2I &maxPt );

   /// Performs multiple smoothing steps on the heightmap.
   void smooth( F32 factor, U32 steps, bool updateCollision );

   void setSize( U32 newResolution, bool clear );

   TerrainSquare* findSquare( U32 level, U32 x, U32 y ) const;
   
   BaseMatInstance* getMaterialMapping( U32 index ) const;
   
   StringTableEntry getMaterialName( U32 x, U32 y) const;

   void setLayerIndex( U32 x, U32 y, U8 index );

   U8 getLayerIndex( U32 x, U32 y ) const;

   bool isEmptyAt( U32 x, U32 y ) const { return getLayerIndex( x, y ) == U8_MAX; }

   void setHeight( U32 x, U32 y, F32 height );

   const F32* getHeightAddress( U32 x, U32 y ) const;

   F32 getHeight( U32 x, U32 y ) const;

   F32 getMaxHeight() const { return mGridMap[mGridLevels]->maxHeight; }

   /// Returns the constant heightmap vector.
   const Vector<F32>& getHeightMap() const { return mHeightMap; }

   /// Sets a new heightmap state.
   void setHeightMap( const Vector<F32> &heightmap, bool updateCollision );

   /// Check if the given point is valid within the (non-tiled) terrain file.
   bool isPointInTerrain( U32 x, U32 y ) const;
};


inline TerrainSquare* TerrainFile::findSquare( U32 level, U32 x, U32 y ) const
{
   x %= mSize;
   y %= mSize;
   x >>= level;
   y >>= level;

   return mGridMap[level] + x + ( y << ( mGridLevels - level ) );
}

inline void TerrainFile::setHeight( U32 x, U32 y, F32 height )
{
   x %= mSize;
   y %= mSize;
   mHeightMap[ x + ( y * mSize ) ] = height;
}

inline const F32* TerrainFile::getHeightAddress( U32 x, U32 y ) const
{
   x %= mSize;
   y %= mSize;
   return &mHeightMap[ x + ( y * mSize ) ];
}

inline F32 TerrainFile::getHeight( U32 x, U32 y ) const
{
   x %= mSize;
   y %= mSize;
   return mHeightMap[ x + ( y * mSize ) ];
}

inline U8 TerrainFile::getLayerIndex( U32 x, U32 y ) const
{
   x %= mSize;
   y %= mSize;
   return mLayerMap[ x + ( y * mSize ) ];
}

inline void TerrainFile::setLayerIndex( U32 x, U32 y, U8 index )
{
   x %= mSize;
   y %= mSize;
   mLayerMap[ x + ( y * mSize ) ] = index;
}

inline BaseMatInstance* TerrainFile::getMaterialMapping( U32 index ) const
{
   if ( index < mMaterialInstMapping.size() )
      return mMaterialInstMapping.getMaterialInst( index );
   else
      return NULL;
}

inline StringTableEntry TerrainFile::getMaterialName( U32 x, U32 y) const
{
   x %= mSize;
   y %= mSize;
   const U8 &index = mLayerMap[ x + ( y * mSize ) ];

   if ( index < mMaterials.size() )
      return mMaterials[ index ]->getInternalName();

   return StringTable->EmptyString();
}


inline F32 getKFixedToFloat() {
   // NOTE: 1 / 32 = 0.03125
   return 32.0f;
}



inline bool TerrainFile::isPointInTerrain( U32 x, U32 y ) const
{
   if ( x < mSize && y < mSize)
      return true;

   return false;
}

#endif // _TERRFILE_H_ 
