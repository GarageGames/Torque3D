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
   Vector<Point3F> points;
   Vector<Point3F> normals;
   Vector<Point2F> uvs;
   Vector<Point2F> uv2s;
   Vector<ColorI> colors;
   Vector<TSDrawPrimitive> primitives;
   Vector<U32> indices;

   // Skin elements
   Vector<F32> weight;
   Vector<S32> boneIndex;
   Vector<S32> vertexIndex;
   Vector<S32> nodeIndex;
   Vector<MatrixF> initialTransforms;
   Vector<Point3F> initialVerts;
   Vector<Point3F> initialNorms;

   U32 flags;
   U32 vertsPerFrame;
   S32 numFrames;
   S32 numMatFrames;

   // Loader elements (can be discarded after loading)
   S32                           detailSize;
   MatrixF                       objectOffset;
   Vector<AppNode*>              bones;
   static Vector<AppMaterial*>   appMaterials;

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
