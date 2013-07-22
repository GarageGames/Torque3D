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

#ifndef _SCENECONTAINER_H_
#define _SCENECONTAINER_H_

#ifndef _MBOX_H_
#include "math/mBox.h"
#endif

#ifndef _MSPHERE_H_
#include "math/mSphere.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _MPOLYHEDRON_H_
#include "math/mPolyhedron.h"
#endif

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif


/// @file
/// SceneObject database.


class SceneObject;
class AbstractPolyList;
class OptimizedPolyList;
class Frustum;
class Point3F;

struct RayInfo;


template< typename T >
class SceneObjectRefBase
{
   public:

      /// Object that is referenced in the link.
      SceneObject* object;

      /// Next link in chain of container.
      T* nextInBin;

      /// Previous link in chain of container.
      T* prevInBin;

      /// Next link in chain that is associated with #object.
      T* nextInObj;
};


/// Reference to a scene object.
class SceneObjectRef : public SceneObjectRefBase< SceneObjectRef > {};


/// A contextual hint passed to the polylist methods which
/// allows it to return the appropriate geometry.
enum PolyListContext
{
   /// A hint that the polyist is intended 
   /// for collision testing.
   PLC_Collision,
   
   /// A hint that the polyist is for decal
   /// geometry generation.
   PLC_Decal,

   /// A hint that the polyist is used for
   /// selection from an editor or other tool.
   PLC_Selection,

   /// A hint that the polylist is used for
   /// building a representation of the environment
   /// used for navigation.
   PLC_Navigation,

   /// A hint that the polyist will be used
   /// to export geometry and would like to have
   /// texture coords and materials.   
   PLC_Export
};


/// For simple queries.  Simply creates a vector of the objects
class SimpleQueryList
{
   public:

      Vector< SceneObject* > mList;

      SimpleQueryList() 
      {
         VECTOR_SET_ASSOCIATION( mList );
      }

      void insertObject( SceneObject* obj ) { mList.push_back(obj); }
      static void insertionCallback( SceneObject* obj, void* key )
      {
         SimpleQueryList* pList = reinterpret_cast< SimpleQueryList* >( key );
         pList->insertObject( obj );
      }
};


//----------------------------------------------------------------------------

/// Database for SceneObjects.
///
/// ScenceContainer implements a grid-based spatial subdivision for the contents of a scene.
class SceneContainer
{
      enum CastRayType
      {
         CollisionGeometry,
         RenderedGeometry,
      };

   public:

      struct Link
      {
         Link* next;
         Link* prev;
         Link();
         void unlink();
         void linkAfter(Link* ptr);
      };

      struct CallbackInfo 
      {
         PolyListContext context;
         AbstractPolyList* polyList;
         Box3F boundingBox;
         SphereF boundingSphere;
         void *key;
      };

   private:

      Link mStart;
      Link mEnd;

      /// Container queries based on #mCurrSeqKey are are not re-entrant;
      /// this is used to detect when it happens.
      bool mSearchInProgress;

      /// Current sequence key.
      U32 mCurrSeqKey;

      SceneObjectRef* mFreeRefPool;
      Vector< SceneObjectRef* > mRefPoolBlocks;

      SceneObjectRef* mBinArray;
      SceneObjectRef mOverflowBin;

      /// A vector that contains just the water and physical zone
      /// object types which is used to optimize searches.
      Vector< SceneObject* > mWaterAndZones;

      /// Vector that contains just the terrain objects in the container.
      Vector< SceneObject* > mTerrains;

      static const U32 csmNumBins;
      static const F32 csmBinSize;
      static const F32 csmTotalBinSize;
      static const U32 csmRefPoolBlockSize;

   public:

      SceneContainer();
      ~SceneContainer();

      /// Return a vector containing all the water and physical zone objects in this container.
      const Vector< SceneObject* >& getWaterAndPhysicalZones() const { return mWaterAndZones; }

      /// Return a vector containing all terrain objects in this container.
      const Vector< SceneObject* >& getTerrains() const { return mTerrains; }

      /// @name Basic database operations
      /// @{

      ///
      typedef void ( *FindCallback )( SceneObject* object, void* key );

      /// Find all objects of the given type(s) and invoke the given callback for each
      /// of them.
      /// @param mask Object type mask (@see SimObjectTypes).
      /// @param callback Pointer to function to invoke for each object.
      /// @param key User data to pass to the "key" argument of @a callback.
      void findObjects( U32 mask, FindCallback callback, void* key = NULL );

      void findObjects( const Box3F& box, U32 mask, FindCallback, void *key = NULL );
      void findObjects( const Frustum& frustum, U32 mask, FindCallback, void *key = NULL );

      void polyhedronFindObjects( const Polyhedron& polyhedron, U32 mask, FindCallback, void *key = NULL );

      /// Find all objects of the given type(s) and add them to the given vector.
      /// @param mask Object type mask (@see SimObjectTypes).
      /// @param outFound Vector to add found objects to.
      void findObjectList( U32 mask, Vector< SceneObject* >* outFound );

      ///
      void findObjectList( const Box3F& box, U32 mask, Vector< SceneObject* >* outFound );

      ///
      void findObjectList( const Frustum& frustum, U32 mask, Vector< SceneObject* >* outFound );

      /// @}

      /// @name Line intersection
      /// @{

      typedef bool ( *CastRayCallback )( RayInfo* ri );

      /// Test against collision geometry -- fast.
      bool castRay( const Point3F &start, const Point3F &end, U32 mask, RayInfo* info, CastRayCallback callback = NULL );

      /// Test against rendered geometry -- slow.
      bool castRayRendered( const Point3F &start, const Point3F &end, U32 mask, RayInfo* info, CastRayCallback callback = NULL );

      bool collideBox(const Point3F &start, const Point3F &end, U32 mask, RayInfo* info);

      /// @}

      /// @name Poly list
      /// @{

      ///
      bool buildPolyList(  PolyListContext context, 
                           const Box3F &box, 
                           U32 typeMask, 
                           AbstractPolyList *polylist );

      /// @}

      /// Add an object to the database.
      /// @param object A SceneObject.
      bool addObject( SceneObject* object );

      /// Remove an object from the database.
      /// @param object A SceneObject.
      bool removeObject( SceneObject* object );

      void addRefPoolBlock();
      SceneObjectRef* allocateObjectRef();
      void freeObjectRef(SceneObjectRef*);
      void insertIntoBins( SceneObject* object );
      void removeFromBins( SceneObject* object );

      /// Make sure that we're not just sticking the object right back
      /// where it came from.  The overloaded insertInto is so we don't calculate
      /// the ranges twice.
      void checkBins( SceneObject* object );
      void insertIntoBins(SceneObject*, U32, U32, U32, U32);

      void initRadiusSearch(const Point3F& searchPoint,
         const F32      searchRadius,
         const U32      searchMask);
      void initTypeSearch(const U32      searchMask);
      SceneObject* containerSearchNextObject();
      U32  containerSearchNext();
      F32  containerSearchCurrDist();
      F32  containerSearchCurrRadiusDist();

   private:

      Vector<SimObjectPtr<SceneObject>*>  mSearchList;///< Object searches to support console querying of the database.  ONLY WORKS ON SERVER
      S32                                 mCurrSearchPos;
      Point3F                             mSearchReferencePoint;

      void cleanupSearchVectors();

      /// Base cast ray code
      bool _castRay( U32 type, const Point3F &start, const Point3F &end, U32 mask, RayInfo* info, CastRayCallback callback );

      void _findSpecialObjects( const Vector< SceneObject* >& vector, U32 mask, FindCallback, void *key = NULL );
      void _findSpecialObjects( const Vector< SceneObject* >& vector, const Box3F &box, U32 mask, FindCallback callback, void *key = NULL );   

      static void getBinRange( const F32 min, const F32 max, U32& minBin, U32& maxBin );
};

//-----------------------------------------------------------------------------

extern SceneContainer gServerContainer;
extern SceneContainer gClientContainer;

//-----------------------------------------------------------------------------

inline void SceneContainer::freeObjectRef(SceneObjectRef* trash)
{
   trash->object = NULL;
   trash->nextInBin = NULL;
   trash->prevInBin = NULL;
   trash->nextInObj = mFreeRefPool;
   mFreeRefPool     = trash;
}

//-----------------------------------------------------------------------------

inline SceneObjectRef* SceneContainer::allocateObjectRef()
{
   if( mFreeRefPool == NULL )
      addRefPoolBlock();
   AssertFatal( mFreeRefPool!=NULL, "Error, should always have a free reference here!" );

   SceneObjectRef* ret = mFreeRefPool;
   mFreeRefPool = mFreeRefPool->nextInObj;

   ret->nextInObj = NULL;
   return ret;
}

#endif // !_SCENECONTAINER_H_