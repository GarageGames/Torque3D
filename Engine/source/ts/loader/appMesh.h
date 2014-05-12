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

#ifndef _APPMESH_H_
#define _APPMESH_H_

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _APPMATERIAL_H_
#include "ts/loader/appMaterial.h"
#endif
#ifndef _APPSEQUENCE_H_
#include "ts/loader/appSequence.h"
#endif

class AppNode;

class AppMesh
{
public:
   // Mesh and skin elements
   Vector<Point3F> mPoints;
   Vector<Point3F> mNormals;
   Vector<Point2F> mUVs;
   Vector<Point2F> mUV2s;
   Vector<ColorI> mColors;
   Vector<TSDrawPrimitive> mPrimitives;
   Vector<U32> mIndices;

   // Skin elements
   Vector<F32> mWeight;
   Vector<S32> mBoneIndex;
   Vector<S32> mVertexIndex;
   Vector<S32> mNodeIndex;
   Vector<MatrixF> mInitialTransforms;
   Vector<Point3F> mInitialVerts;
   Vector<Point3F> mInitialNorms;

   U32 mFlags;
   U32 mVertsPerFrame;
   S32 mNumFrames;
   S32 mNumMatFrames;

   // Loader elements (can be discarded after loading)
   S32                           mDetailSize;
   MatrixF                       mObjectOffset;
   Vector<AppNode*>              mBones;
   static Vector<AppMaterial*>   mAppMaterials;

public:
   AppMesh();
   virtual ~AppMesh();

   void computeBounds(Box3F& bounds);
   void computeNormals();

   // Create a TSMesh object
   TSMesh* constructTSMesh();

   virtual const char * getName(bool allowFixed=true) = 0;

   virtual MatrixF getMeshTransform(F32 time) = 0;
   virtual F32 getVisValue(F32 time) = 0;

   virtual bool getFloat(const char* propName, F32& defaultVal) = 0;
   virtual bool   getInt(const char* propName, S32& defaultVal) = 0;
   virtual bool  getBool(const char* propName, bool& defaultVal) = 0;

   virtual bool animatesVis(const AppSequence* appSeq) { return false; }
   virtual bool animatesMatFrame(const AppSequence* appSeq) { return false; }
   virtual bool animatesFrame(const AppSequence* appSeq) { return false; }

   virtual bool isBillboard();
   virtual bool isBillboardZAxis();

   virtual bool isSkin() { return false; }
   virtual void lookupSkinData() = 0;

   virtual void lockMesh(F32 t, const MatrixF& objectOffset) { }
};

#endif // _APPMESH_H_
