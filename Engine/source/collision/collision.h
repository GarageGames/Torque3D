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

#ifndef _COLLISION_H_
#define _COLLISION_H_

#ifndef _DATACHUNKER_H_
#include "core/dataChunker.h"
#endif
#ifndef _MPLANE_H_
#include "math/mPlane.h"
#endif
#ifndef _MPOINT2_H_
#include "math/mPoint2.h"
#endif

class SceneObject;
class BaseMatInstance;

//----------------------------------------------------------------------------

struct Collision
{
   SceneObject* object;
   Point3F point;
   VectorF normal;
   BaseMatInstance* material;

   // generate UV coordinate across (TSStatic) mesh based on 
   // matching normals, this isn't done by default and is 
   // primarily of interest in matching a collision point to 
   // either a GUI control coordinate or finding a hit pixel in texture space
   bool generateTexCoord;

   // If generateTexCoord is set this will either be invalid (-1, -1)
   // or a value in texture space (assuming 0..1 UV mapping)
   Point2F texCoord;

   // Face and Face dot are currently only set by the extrudedPolyList
   // clipper.  Values are otherwise undefined.
   U32 face;                  // Which face was hit
   F32 faceDot;               // -Dot of face with poly normal
   F32 distance;
   
   Collision() :
      object( NULL ),
      material( NULL ),
      generateTexCoord(false),
      texCoord(-1.0f, -1.0f)
   {
   }
};

class CollisionList
{
public:
   enum
   {
      MaxCollisions = 64
   };

protected:
   dsize_t mCount;
   Collision mCollision[MaxCollisions];
   F32 mT;
   // MaxHeight is currently only set by the extrudedPolyList
   // clipper.  It represents the maximum vertex z value of
   // the returned collision surfaces.
   F32 mMaxHeight;

public:
   // Constructor
   CollisionList( /* const dsize_t reserveSize = MaxCollisions */ ) :
      mCount( 0 ), mT( 0.0f ), mMaxHeight( 0.0f )
   {

   }

   // Accessors
   int getCount() const { return mCount; }
   F32 getTime() const { return mT; }
   F32 getMaxHeight() const { return mMaxHeight; }

   const Collision &operator[] ( const dsize_t idx ) const
   {
      AssertFatal( idx < mCount, "Out of bounds index." );
      return mCollision[idx];
   }

   Collision &operator[] ( const dsize_t idx )
   {
      AssertFatal( idx < mCount, "Out of bounds index." );
      return mCollision[idx];
   }

   // Increment does NOT reset the collision which it returns. It is the job of
   // the caller to make sure that the entry has data properly assigned to it.
   Collision &increment()
   {
      return mCollision[mCount++];
   }

   void clear()
   {
      mCount = 0;
   }

   void setTime( const F32 t )
   {
      mT = t;
   }

   void setMaxHeight( const F32 height )
   {
      mMaxHeight = height;
   }
};


//----------------------------------------------------------------------------
// BSP Collision tree
// Solid nodes are represented by structures with NULL frontNode and
// backNode pointers. The material field is only valid on a solid node.
// There is no structure for empty nodes, frontNode or backNode
// should be set to NULL to represent empty half-spaces.

struct BSPNode
{
   U32 material;
   PlaneF plane;
   BSPNode *frontNode, *backNode;
};

typedef Chunker<BSPNode> BSPTree;

/// Extension of the collision structure to allow use with raycasting.
/// @see Collision
struct RayInfo : public Collision 
{
   RayInfo() : userData( NULL ) {}

   // The collision struct has object, point, normal & material.

   /// Distance along ray to contact point.
   F32 t; 

   /// Set the point of intersection according to t and the given ray.
   ///
   /// Several pieces of code will not use ray information but rather rely
   /// on contact points directly, so it is a good thing to always set
   /// this in castRay functions.
   void setContactPoint( const Point3F& start, const Point3F& end )
   {
      Point3F startToEnd = end - start;
      startToEnd *= t;
      point = startToEnd + start;
   }

   /// A generic data void pointer.
   /// Passing a void* around to random objects of unknown class types that may 
   /// interpret it differently would be very dangerous.  Only use userData when 
   /// you call castRay/etc on an individual object of a known type.
   void *userData;
};


#endif // _COLLISION_H_
