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


/// Conversion from 11.5 fixed point to floating point.
inline F32 fixedToFloat( U16 val )
{
   return F32(val) * 0.03125f;
}

/// Conversion from floating point to 11.5 fixed point.
inline U16 floatToFixed( F32 val )
{
   return U16(val * 32.0f + 0.5f);
}

///
struct TerrainSquare
{
   U16 minHeight;

   U16 maxHeight;

   U16 heightDeviance;

   U16 flags;

   enum 
   {
      Split45  = BIT(0),

      Empty    = BIT(1),

      HasEmpty = BIT(2),
   };
};


/// NOTE:  The terrain uses 11.5 fixed point which gives
/// us a height range from 0->2048 in 1/32 increments.
typedef U16 TerrainHeight;


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

   /// The fixed point height map.
   /// @see fixedToFloat
   Vector<U16> mHeightMap;

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
      FILE_VERSION = 7
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

   void setHeight( U32 x, U32 y, U16 height );

   void setHeight( U32 i, U16 height );

   const U16* getHeightAddress( U32 x, U32 y ) const;

   U16 getHeight( U32 x, U32 y ) const;

   void getHeight( F32* h, const Point2I& ) const;

   void getHeight4(
      F32* a,  F32* b,  F32* c,  F32* d,
      const Point2I&  pa,
      const Point2I&  pb,
      const Point2I&  pc,
      const Point2I&  pd
   ) const;

   U16 getMaxHeight() const { return mGridMap[mGridLevels]->maxHeight; }

   /// Returns the constant heightmap vector.
   const Vector<U16>& getHeightMap() const { return mHeightMap; }

   /// Sets a new heightmap state.
   void setHeightMap( const Vector<U16> &heightmap, bool updateCollision );

   /// Check if the given point is valid within the (non-tiled) terrain file.
   bool isPointInTerrain( U32 x, U32 y ) const;

private:
   // Clamp X and Y to the size of terrain.
   void clamp( U32* x,  U32* y ) const;
   inline static void error(char c, U32 v ) {
	   static const char* s = "TerrainFile Coord '%c == %d' out of range. Fix it in the algorithm.";
#if 1
      // only first error
      static bool  first = true;
      if ( first ) {
         Con::errorf( s,  c, v );
         first = false;
      }
#else
      // all errors
      Con::errorf( s,  c, v );
#endif
   }
};

// @todo ! Need AssertFatal() and debug all clients.
// @todo ! Verify all methods of TerrainFile.
inline void TerrainFile::clamp( U32* x,  U32* y ) const
{
#if 0
   // old clamp
   *x %= mSize;
   *y %= mSize;
#else


   if (*x >= mSize) {
      error( 'x', *x );
      *x = mSize - 1;
   }
   if (*y >= mSize) {
      error( 'y', *y );
      *y = mSize - 1;
   }
#endif
}


inline TerrainSquare* TerrainFile::findSquare( U32 level, U32 x, U32 y ) const
{
   clamp( &x, &y );
   x >>= level;
   y >>= level;

   return mGridMap[level] + x + ( y << ( mGridLevels - level ) );
}

inline void TerrainFile::setHeight( U32 x, U32 y, U16 height )
{
	clamp( &x, &y );
   setHeight( x + y * mSize,  height );
}

inline void TerrainFile::setHeight( U32 i, U16 height )
{
   mHeightMap[ i ] = height;
}

inline const U16* TerrainFile::getHeightAddress( U32 x, U32 y ) const
{
	clamp( &x, &y );
   return &mHeightMap[ x + ( y * mSize ) ];
}

inline U16 TerrainFile::getHeight( U32 x, U32 y ) const
{
	clamp( &x, &y );
   return mHeightMap[ x + ( y * mSize ) ];
}

inline void TerrainFile::getHeight( F32* h,  const Point2I&  p ) const
{
   const Point2I pp(
      mClamp( p.x, 0, mSize - 1 ),
      mClamp( p.y, 0, mSize - 1 )
   );
   *h = fixedToFloat( getHeight( (U32)pp.x, (U32)pp.y ) );
}

inline void TerrainFile::getHeight4(
   F32* a,  F32* b,  F32* c,  F32* d,
   const Point2I&  pa,
   const Point2I&  pb,
   const Point2I&  pc,
   const Point2I&  pd
) const
{
   getHeight( a, pa );
   getHeight( b, pb );
   getHeight( c, pc );
   getHeight( d, pd );
}

inline U8 TerrainFile::getLayerIndex( U32 x, U32 y ) const
{
	clamp( &x, &y );
   return mLayerMap[ x + ( y * mSize ) ];
}

inline void TerrainFile::setLayerIndex( U32 x, U32 y, U8 index )
{
	clamp( &x, &y );
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
	clamp( &x, &y );
   const U8 &index = mLayerMap[ x + ( y * mSize ) ];

   if ( index < mMaterials.size() )
      return mMaterials[ index ]->getInternalName();

   return StringTable->EmptyString();
}



inline bool TerrainFile::isPointInTerrain( U32 x, U32 y ) const
{
   if ( x < mSize && y < mSize)
      return true;

   return false;
}

#endif // _TERRFILE_H_ 
