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

#ifndef _FORCEFIELD_H_
#define _FORCEFIELD_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _MBOX_H_
#include "math/mBox.h"
#endif
#ifndef _MSPHERE_H_
#include "math/mSphere.h"
#endif
#ifndef _MPLANE_H_
#include "math/mPlane.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif

//-------------------------------------- forward decls.
class  EditGeometry;
class  InteriorInstance;
class  Stream;
class  AbstractPolyList;
struct RayInfo;

//--------------------------------------------------------------------------
class ForceField
{
   static const U32 smFileVersion;
   friend class EditGeometry;
   friend class InteriorInstance;

   //-------------------------------------- Public interfaces
  public:
   ForceField();
   ~ForceField();

   bool         prepForRendering();
   void         render(const ColorF& color, const F32 fadeLevel);
   const Box3F& getBoundingBox() const;

   bool castRay(const Point3F&, const Point3F&, RayInfo*);
   bool buildPolyList(AbstractPolyList*, SphereF&);
   //-------------------------------------- Persistence interface
  public:
   bool read(Stream& stream);
   bool write(Stream& stream) const;

  public:
   static U16  getPlaneIndex(U16 index);
   static bool planeIsFlipped(U16 index);

   //-------------------------------------- BSP Structures
  private:
   struct IBSPNode {
      U16 planeIndex;
      U16 frontIndex;
      U16 backIndex;
      U16 __padding__;
   };
   struct IBSPLeafSolid {
      U32 surfaceIndex;
      U16 surfaceCount;
      U16 __padding__;
   };

   bool isBSPLeafIndex(U16 index) const;
   bool isBSPSolidLeaf(U16 index) const;
   bool isBSPEmptyLeaf(U16 index) const;
   U16  getBSPSolidLeafIndex(U16 index) const;

   bool writePlaneVector(Stream&) const;
   bool readPlaneVector(Stream&);

  private:
   const PlaneF& getPlane(U16 index) const;
   bool          areEqualPlanes(U16, U16) const;

   struct Surface {
      U32 windingStart;
      U32 fanMask;

      U16 planeIndex;
      U8  windingCount;
      U8  surfaceFlags;
   };

  protected:
   StringTableEntry         mName;
   ColorF                   mColor;
   Vector<StringTableEntry> mTriggers;

   Box3F                   mBoundingBox;
   SphereF                 mBoundingSphere;
   Vector<PlaneF>          mPlanes;
   Vector<Point3F>         mPoints;

   Vector<IBSPNode>        mBSPNodes;
   Vector<IBSPLeafSolid>   mBSPSolidLeaves;
   Vector<U32>             mSolidLeafSurfaces;

   bool                    mPreppedForRender;

   Vector<U32>             mWindings;
   Vector<Surface>         mSurfaces;

  protected:
   bool castRay_r(const U16, const Point3F&, const Point3F&, RayInfo*);
   void buildPolyList_r(const U16, Vector<U16>&, AbstractPolyList*, SphereF&);
   void collisionFanFromSurface(const Surface&, U32* fan, U32* numIndices) const;
};

//------------------------------------------------------------------------------
inline bool ForceField::isBSPLeafIndex(U16 index) const
{
   return (index & 0x8000) != 0;
}

inline bool ForceField::isBSPSolidLeaf(U16 index) const
{
   AssertFatal(isBSPLeafIndex(index) == true, "Error, only call for leaves!");
   return (index & 0x4000) != 0;
}

inline bool ForceField::isBSPEmptyLeaf(U16 index) const
{
   AssertFatal(isBSPLeafIndex(index) == true, "Error, only call for leaves!");
   return (index & 0x4000) == 0;
}

inline U16 ForceField::getBSPSolidLeafIndex(U16 index) const
{
   AssertFatal(isBSPSolidLeaf(index) == true, "Error, only call for leaves!");
   return (index & ~0xC000);
}

inline const PlaneF& ForceField::getPlane(U16 index) const
{
   AssertFatal(U32(index & ~0x8000) < mPlanes.size(),
               "ForceField::getPlane: planeIndex out of range");

   return mPlanes[index & ~0x8000];
}

inline U16 ForceField::getPlaneIndex(U16 index)
{
   return index & ~0x8000;
}

inline bool ForceField::planeIsFlipped(U16 index)
{
   return (index & 0x8000) != 0;
}

inline bool ForceField::areEqualPlanes(U16 o, U16 t) const
{
   return (o & ~0x8000) == (t & ~0x8000);
}

inline const Box3F& ForceField::getBoundingBox() const
{
   return mBoundingBox;
}

#endif  // _H_FORCEFIELD_

