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

#ifndef _ABSTRACTPOLYLIST_H_
#define _ABSTRACTPOLYLIST_H_

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _MPLANETRANSFORMER_H_
#include "math/mPlaneTransformer.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

class SceneObject;
class BaseMatInstance;


/// A polygon filtering interface.
///
/// The various AbstractPolyList subclasses are used in Torque as an interface to
/// handle spatial queries. SceneObject::buildPolyList() takes an implementor of
/// AbstractPolyList (such as ConcretePolyList, ClippedPolyList, etc.) and a
/// bounding volume. The function runs geometry data from all the objects in the
/// bounding volume through the passed PolyList.
///
/// This interface only provides a method to get data INTO your implementation. Different
/// subclasses provide different interfaces to get data back out, depending on their
/// specific quirks.
///
/// The physics engine now uses convex hulls for collision detection.
///
/// @see Convex
class AbstractPolyList
{
protected:
   // User set state
   SceneObject* mCurrObject;

   MatrixF  mBaseMatrix;               // Base transform
   MatrixF  mTransformMatrix;          // Current object transform
   MatrixF  mMatrix;                   // Base * current transform
   Point3F  mScale;

   PlaneTransformer mPlaneTransformer;

   bool     mInterestNormalRegistered;
   Point3F  mInterestNormal;

public:
   AbstractPolyList();
   virtual ~AbstractPolyList();

   /// @name Common Interface
   /// @{
   void setBaseTransform(const MatrixF& mat);

   /// Sets the transform applying to the current stream of
   /// vertices.
   ///
   /// @param  mat   Transformation of the object. (in)
   /// @param  scale Scaling of the object. (in)
   void setTransform(const MatrixF* mat, const Point3F& scale);

   /// Gets the transform applying to the current stream of
   /// vertices.
   ///
   /// @param  mat   Transformation of the object. (out)
   /// @param  scale Scaling of the object. (out)
   void getTransform(MatrixF* mat, Point3F * scale);

   /// This is called by the object which is currently feeding us
   /// vertices, to tell us who it is.
   void setObject(SceneObject*);

   /// Add a box via the query interface (below). This wraps some calls
   /// to addPoint and addPlane.
   void addBox(const Box3F &box, BaseMatInstance* material = NULL);

   void doConstruct();
   /// @}

   /// @name Query Interface
   ///
   /// It is through this interface that geometry data is fed to the
   /// PolyList. The order of calls are:
   ///   - begin()
   ///   - One or more calls to vertex() and one call to plane(), in any order.
   ///   - end()
   ///
   /// @code
   ///   // Example code that adds data to a PolyList.
   ///   // See AbstractPolyList::addBox() for the function this was taken from.
   ///
   ///   // First, we add points... (note that we use base to track the start of adding.)
   ///   U32 base = addPoint(pos);
   ///   pos.y += dy; addPoint(pos);
   ///   pos.x += dx; addPoint(pos);
   ///   pos.y -= dy; addPoint(pos);
   ///   pos.z += dz; addPoint(pos);
   ///   pos.x -= dx; addPoint(pos);
   ///   pos.y += dy; addPoint(pos);
   ///   pos.x += dx; addPoint(pos);
   ///
   ///   // Now we add our surfaces. (there are six, as we are adding a cube here)
   ///   for (S32 i = 0; i < 6; i++) {
   ///      // Begin a surface
   ///      begin(0,i);
   ///
   ///      // Calculate the vertex ids; we have a lookup table to tell us offset from base.
   ///      // In your own code, you might use a different method.
   ///      S32 v1 = base + PolyFace[i][0];
   ///      S32 v2 = base + PolyFace[i][1];
   ///      S32 v3 = base + PolyFace[i][2];
   ///      S32 v4 = base + PolyFace[i][3];
   ///
   ///      // Reference the four vertices that make up this surface.
   ///      vertex(v1);
   ///      vertex(v2);
   ///      vertex(v3);
   ///      vertex(v4);
   ///
   ///      // Indicate the plane of this surface.
   ///      plane(v1,v2,v3);
   ///
   ///      // End the surface.
   ///      end();
   ///   }
   /// @endcode
   /// @{

   /// Are we empty of data?
   virtual bool isEmpty() const = 0;

   /// Adds a point to the poly list, and returns
   /// an ID number for that point.
   virtual U32  addPoint(const Point3F& p) = 0;

   /// Adds a point and normal to the poly list, and returns
   /// an ID number for them.  Normals are ignored for polylists
   /// that do not support them.
   virtual U32  addPointAndNormal(const Point3F& p, const Point3F& normal) { return addPoint( p ); }

   /// Adds a plane to the poly list, and returns
   /// an ID number for that point.
   virtual U32  addPlane(const PlaneF& plane) = 0;

   /// Start a surface.
   ///
   /// @param  material    A material ID for this surface.
   /// @param  surfaceKey  A key value to associate with this surface.
   virtual void begin(BaseMatInstance* material,U32 surfaceKey) = 0;

   /// Indicate the plane of the surface.
   virtual void plane(U32 v1,U32 v2,U32 v3) = 0;
   /// Indicate the plane of the surface.
   virtual void plane(const PlaneF& p) = 0;
   /// Indicate the plane of the surface.
   virtual void plane(const U32 index) = 0;

   /// Reference a vertex which is in this surface.
   virtual void vertex(U32 vi) = 0;

   /// Mark the end of a surface.
   virtual void end() = 0;

   /// Return list transform and bounds in list space.
   ///
   /// @returns False if no data is available.
   virtual bool getMapping(MatrixF *, Box3F *);
   /// @}

   /// @name Interest
   ///
   /// This is a mechanism to let you specify interest in a specific normal.
   /// If you set a normal you're interested in, then any planes facing "away"
   /// from that normal are culled from the results.
   ///
   /// This is handy if you're using this to do a physics check, as you're not
   /// interested in polygons facing away from you (since you don't collide with
   /// the backsides/insides of things).
   /// @{

   void setInterestNormal(const Point3F& normal);
   void clearInterestNormal()                        { mInterestNormalRegistered = false; }


   virtual bool isInterestedInPlane(const PlaneF& plane);
   virtual bool isInterestedInPlane(const U32 index);

   /// @}

  protected:
   /// A helper function to convert a plane index to a PlaneF structure.
   virtual const PlaneF& getIndexedPlane(const U32 index) = 0;
};


inline AbstractPolyList::AbstractPolyList()
{
   doConstruct();
}

inline void AbstractPolyList::doConstruct()
{
   mCurrObject = NULL;
   mBaseMatrix.identity();
   mMatrix.identity();
   mScale.set(1, 1, 1);

   mPlaneTransformer.setIdentity();

   mInterestNormalRegistered = false;
}

inline void AbstractPolyList::setBaseTransform(const MatrixF& mat)
{
   mBaseMatrix = mat;
}

inline void AbstractPolyList::setTransform(const MatrixF* mat, const Point3F& scale)
{
   mMatrix = mBaseMatrix;
   mTransformMatrix = *mat;
   mMatrix.mul(*mat);
   mScale = scale;

   mPlaneTransformer.set(mMatrix, mScale);
}

inline void AbstractPolyList::getTransform(MatrixF* mat, Point3F * scale)
{
   *mat   = mTransformMatrix;
   *scale = mScale;
}

inline void AbstractPolyList::setObject(SceneObject* obj)
{
   mCurrObject = obj;
}


#endif // _ABSTRACTPOLYLIST_H_
