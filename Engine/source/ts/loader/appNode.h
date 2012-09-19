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

#ifndef _APPNODE_H_
#define _APPNODE_H_

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _APPMESH_H_
#include "ts/loader/appMesh.h"
#endif

class AppNode
{
   friend class TSShapeLoader;

   // add attached meshes and child nodes to app node
   // the reason these are tracked by AppNode is that
   // AppNode is responsible for deleting all it's children
   // and attached meshes.
   virtual void buildMeshList() = 0;
   virtual void buildChildList() = 0;

protected:

   S32 mParentIndex;
   Vector<AppMesh*>  mMeshes;
   Vector<AppNode*>  mChildNodes;
   char* mName;
   char* mParentName;

public:

   AppNode();
   virtual ~AppNode();

   S32 getNumMesh();
   AppMesh* getMesh(S32 idx);

   S32 getNumChildNodes();
   AppNode* getChildNode(S32 idx);

   virtual MatrixF getNodeTransform(F32 time) = 0;

   virtual bool isEqual(AppNode* node) = 0;

   virtual bool animatesTransform(const AppSequence* appSeq) = 0;

   virtual const char* getName() = 0;
   virtual const char* getParentName() = 0;

   virtual bool getFloat(const char* propName, F32& defaultVal) = 0;
   virtual bool getInt(const char* propName, S32& defaultVal) = 0;
   virtual bool getBool(const char* propName, bool& defaultVal) = 0;

   virtual bool isBillboard();
   virtual bool isBillboardZAxis();
   virtual bool isParentRoot() = 0;
   virtual bool isDummy();
   virtual bool isBounds();
   virtual bool isSequence();
   virtual bool isRoot();
};

#endif // _APPNODE_H_
