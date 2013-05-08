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

#ifndef _CONVEX_H_
#define _CONVEX_H_

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

struct Collision;
class CollisionList;
struct CollisionStateList;
class AbstractPolyList;
class SceneObject;
class BaseMatInstance;
class Convex;


//----------------------------------------------------------------------------

class ConvexFeature
{
public:
   struct Edge {
      S32 vertex[2];
   };
   struct Face {
      VectorF normal;
      S32 vertex[3];
   };

   Vector<Point3F> mVertexList;
   Vector<Edge> mEdgeList;
   Vector<Face> mFaceList;
   BaseMatInstance* material;
   SceneObject* object;

   ConvexFeature()
      : mVertexList(64), mEdgeList(128), mFaceList(64), material( 0 )
   {
      VECTOR_SET_ASSOCIATION(mVertexList);
      VECTOR_SET_ASSOCIATION(mEdgeList);
      VECTOR_SET_ASSOCIATION(mFaceList);
   }

   void reset();

   bool collide(ConvexFeature& cf,CollisionList* cList, F32 tol = 0.1);
   void testVertex(const Point3F& v,CollisionList* cList,bool,F32 tol);
   void testEdge(ConvexFeature* cf,const Point3F& s1, const Point3F& e1, CollisionList* cList, F32 tol);
   bool inVolume(const Point3F& v);
};


//----------------------------------------------------------------------------

// TODO: This sucks... replace with registration object
// for assigning type ids.
enum ConvexType {
   TSConvexType,
   BoxConvexType,
   TerrainConvexType,
   ShapeBaseConvexType,
   TSStaticConvexType,
   AtlasChunkConvexType, ///< @deprecated
   AtlasConvexType,   
   TSPolysoupConvexType,
   MeshRoadConvexType,
	ConvexShapeCollisionConvexType,
   ForestConvexType
};


//----------------------------------------------------------------------------

struct CollisionState
{
   CollisionStateList* mLista;
   CollisionStateList* mListb;
   Convex* a;
   Convex* b;

   F32 dist;            // Current estimated distance
   VectorF v;           // Vector between closest points

   //
   CollisionState();
   virtual ~CollisionState();
   virtual void swap();
   virtual void set(Convex* a,Convex* b,const MatrixF& a2w, const MatrixF& b2w);
   virtual F32 distance(const MatrixF& a2w, const MatrixF& b2w, const F32 dontCareDist,
                       const MatrixF* w2a = NULL, const MatrixF* _w2b = NULL);
};


//----------------------------------------------------------------------------

struct CollisionStateList
{
   static CollisionStateList sFreeList;
   CollisionStateList* mNext;
   CollisionStateList* mPrev;
   CollisionState* mState;

   CollisionStateList();

   void linkAfter(CollisionStateList* next);
   void unlink();
   bool isEmpty() { return mNext == this; }

   static CollisionStateList* alloc();
   void free();
};


//----------------------------------------------------------------------------

struct CollisionWorkingList
{
   static CollisionWorkingList sFreeList;
   struct WLink {
      CollisionWorkingList* mNext;
      CollisionWorkingList* mPrev;
   } wLink;
   struct RLink {
      CollisionWorkingList* mNext;
      CollisionWorkingList* mPrev;
   } rLink;
   Convex* mConvex;

   void wLinkAfter(CollisionWorkingList* next);
   void rLinkAfter(CollisionWorkingList* next);
   void unlink();
   CollisionWorkingList();

   static CollisionWorkingList* alloc();
   void free();
};


//----------------------------------------------------------------------------

class Convex {

   /// @name Linked list management
   /// @{
   
   /// Next item in linked list of Convexes.
   Convex* mNext;

   /// Previous item in linked list of Convexes.
   Convex* mPrev; 

   /// Insert this Convex after the provided convex
   /// @param   next
   void linkAfter(Convex* next);

   /// Remove this Convex from the linked list
   void unlink();
   /// @}

   U32 mTag;
   static U32 sTag;

protected:
   CollisionStateList   mList;            ///< Objects we're testing against
   CollisionWorkingList mWorking;         ///< Objects within our bounds
   CollisionWorkingList mReference;       ///< Other convex testing against us
   SceneObject* mObject;                  ///< Object this Convex is built around
   ConvexType mType;                      ///< Type of Convex this is @see ConvexType

public:

   /// Constructor
   Convex();

   /// Destructor
   virtual ~Convex();

   /// Registers another Convex by linking it after this one
   void registerObject(Convex *convex);

   /// Runs through the linked list of Convex objects and removes the ones
   /// with no references
   void collectGarbage();

   /// Deletes all convex objects in the list
   void nukeList();

   /// Returns the type of this Convex
   ConvexType getType() const { return mType;   }

   /// Returns the object this Convex is built from
   SceneObject* getObject() const { return mObject; }

   /// Adds the provided Convex to the list of objects within the bounds of this Convex
   /// @param   ptr    Convex to add to the working list of this object
   void                  addToWorkingList(Convex* ptr);

   /// Returns the list of objects currently inside the bounds of this Convex
   CollisionWorkingList& getWorkingList() { return mWorking; }

   /// Finds the closest
   CollisionState* findClosestState(const MatrixF& mat, const Point3F& scale, const F32 dontCareDist = 1);

   /// Returns the list of objects this object is testing against
   CollisionStateList* getStateList() const { return mList.mNext; }

   /// Updates the CollisionStateList (mList) with new collision states and removing
   /// ones no longer under consideration
   /// @param   mat   Used as the matrix to create a bounding box for updating the list
   /// @param   scale   Used to scale the bounding box
   /// @param   displacement   Bounding box displacement (optional)
   void updateStateList(const MatrixF& mat, const Point3F& scale, const Point3F* displacement = NULL);

   /// Updates the working collision list of objects which are currently colliding with
   /// (inside the bounds of) this Convex.
   ///
   /// @param  box      Used as the bounding box.
   /// @param  colMask  Mask of objects to check against.
   void updateWorkingList(const Box3F& box, const U32 colMask);

   /// Clear out the working collision list of objects
   void clearWorkingList();

   /// Returns the transform of the object this is built around
   virtual const MatrixF& getTransform() const;

   /// Returns the scale of the object this is built around
   virtual const Point3F& getScale() const;

   /// Returns the bounding box for the object this is built around in world space
   virtual Box3F getBoundingBox() const;

   /// Returns the object space bounding box for the object this is built around
   /// transformed and scaled
   /// @param   mat   Matrix to transform the object-space box by
   /// @param   scale   Scaling factor to scale the bounding box by
   virtual Box3F getBoundingBox(const MatrixF& mat, const Point3F& scale) const;

   /// Returns the farthest point, along a vector, still bound by the convex
   /// @param   v   Vector
   virtual Point3F support(const VectorF& v) const;

   /// This is used by the GJK collision in Vehicle.
   /// The Convex class should add verts, edges, and faces to the passed
   /// ConvexFeature that face towards the passed normal vector. Verts added
   /// in this way should also be transformed by the passed matrix.
   /// @param  mat   Transform which should be applied to verts added to the ConvexFeature
   /// @param  n     Normal vector
   /// @param  cf    ConvexFeature to add data to.
   virtual void getFeatures(const MatrixF& mat,const VectorF& n, ConvexFeature* cf);

   /// Builds a collision poly list out of this convex
   /// @param   list   (Out) Poly list built
   virtual void getPolyList(AbstractPolyList* list);

   ///
   bool getCollisionInfo(const MatrixF& mat, const Point3F& scale, CollisionList* cList,F32 tol);

   /// Render convex(s) for debugging purposes.
   virtual void renderWorkingList();
   
   /// Render this convex for debugging purposes.
   virtual void render();   
};

#endif // _CONVEX_H_
