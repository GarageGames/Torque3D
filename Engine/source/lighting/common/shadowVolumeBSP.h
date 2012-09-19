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

#ifndef _SHADOWVOLUMEBSP_H_
#define _SHADOWVOLUMEBSP_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _DATACHUNKER_H_
#include "core/dataChunker.h"
#endif
#ifndef _LIGHTMANAGER_H_
#include "lighting/lightManager.h"
#endif

/// Used to calculate shadows.
class ShadowVolumeBSP
{
   public:
      ShadowVolumeBSP();
      ~ShadowVolumeBSP();

      struct SVNode;
      struct SurfaceInfo
      {
         U32               mSurfaceIndex;
         U32               mPlaneIndex;
         Vector<U32>       mShadowed;
         SVNode *          mShadowVolume;
      };

      struct SVNode
      {
         enum Side
         {
            Front       = 0,
            Back        = 1,
            On          = 2,
            Split       = 3
         };

         SVNode *       mFront;
         SVNode *       mBack;
         U32            mPlaneIndex;
         U32            mShadowVolume;

         /// Used with shadowed interiors.
         SurfaceInfo *  mSurfaceInfo;
      };

      struct SVPoly
      {
         enum {
            MaxWinding  = 32
         };

         U32               mWindingCount;
         Point3F           mWinding[MaxWinding];

         PlaneF            mPlane;
         SVNode *          mTarget;
         U32               mShadowVolume;
         SVPoly *          mNext;
         SurfaceInfo *     mSurfaceInfo;
      };

      void insertPoly(SVNode **, SVPoly *);
      void insertPolyFront(SVNode **, SVPoly *);
      void insertPolyBack(SVNode **, SVPoly *);

      void splitPoly(SVPoly *, const PlaneF &, SVPoly **, SVPoly **);
      void insertShadowVolume(SVNode **, U32);
      void addUniqueVolume(SurfaceInfo *, U32);

      SVNode::Side whichSide(SVPoly *, const PlaneF &) const;

      //
      bool testPoint(SVNode *, const Point3F &);
      bool testPoly(SVNode *, SVPoly *);
      void addToPolyList(SVPoly **, SVPoly *) const;
      void clipPoly(SVNode *, SVPoly **, SVPoly *);
      void clipToSelf(SVNode *, SVPoly **, SVPoly *);
      F32 getPolySurfaceArea(SVPoly *) const;
      F32 getClippedSurfaceArea(SVNode *, SVPoly *);
      void movePolyList(SVPoly **, SVPoly *) const;
      F32 getLitSurfaceArea(SVPoly *, SurfaceInfo *);

      Vector<SurfaceInfo *>   mSurfaces;

      Chunker<SVNode>         mNodeChunker;
      Chunker<SVPoly>         mPolyChunker;

      SVNode * createNode();
      void recycleNode(SVNode *);

      SVPoly * createPoly();
      void recyclePoly(SVPoly *);

      U32 insertPlane(const PlaneF &);
      const PlaneF & getPlane(U32) const;

      //
      SVNode *          mSVRoot;
      Vector<SVNode*>   mShadowVolumes;
      SVNode * getShadowVolume(U32);

      Vector<PlaneF>    mPlanes;
      SVNode *          mNodeStore;
      SVPoly *          mPolyStore;

      // used to remove the last inserted interior from the tree
      Vector<SVNode*>   mParentNodes;
      SVNode *          mFirstInteriorNode;
      void removeLastInterior();

      /// @name  Access functions
      /// @{
      void insertPoly(SVPoly * poly) {insertPoly(&mSVRoot, poly);}
      bool testPoint(Point3F & pnt) {return(testPoint(mSVRoot, pnt));}
      bool testPoly(SVPoly * poly) {return(testPoly(mSVRoot, poly));}
      F32 getClippedSurfaceArea(SVPoly * poly) {return(getClippedSurfaceArea(mSVRoot, poly));}
      /// @}

      /// @name Helpers
      /// @{
      void buildPolyVolume(SVPoly *, LightInfo *);
      SVPoly * copyPoly(SVPoly *);
      /// @}
};

#endif
