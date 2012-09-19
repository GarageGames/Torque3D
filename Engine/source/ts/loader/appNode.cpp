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

#include "ts/loader/appNode.h"

AppNode::AppNode()
{
   mName = NULL;
   mParentName = NULL;
}

AppNode::~AppNode()
{
   dFree( mName );
   dFree( mParentName );

   // delete children and meshes
   for (S32 i = 0; i < mChildNodes.size(); i++)
      delete mChildNodes[i];
   for (S32 i = 0; i < mMeshes.size(); i++)
      delete mMeshes[i];
}

S32 AppNode::getNumMesh()
{
   if (mMeshes.size() == 0)
      buildMeshList();
   return mMeshes.size();
}

AppMesh* AppNode::getMesh(S32 idx)
{
   return (idx < getNumMesh() && idx >= 0) ? mMeshes[idx] : NULL;
}

S32 AppNode::getNumChildNodes()
{
   if (mChildNodes.size() == 0)
      buildChildList();
   return mChildNodes.size();
}

AppNode * AppNode::getChildNode(S32 idx)
{
   return (idx < getNumChildNodes() && idx >= 0) ? mChildNodes[idx] : NULL;
}

bool AppNode::isBillboard()
{
   return !dStrnicmp(getName(),"BB::",4) || !dStrnicmp(getName(),"BB_",3) || isBillboardZAxis();
}

bool AppNode::isBillboardZAxis()
{
   return !dStrnicmp(getName(),"BBZ::",5) || !dStrnicmp(getName(),"BBZ_",4);
}

bool AppNode::isDummy()
{
   // naming convention should work well enough...
   // ...but can override this method if one wants more
   return !dStrnicmp(getName(), "dummy", 5);
}

bool AppNode::isBounds()
{
   // naming convention should work well enough...
   // ...but can override this method if one wants more
   return !dStricmp(getName(), "bounds");
}

bool AppNode::isSequence()
{
   // naming convention should work well enough...
   // ...but can override this method if one wants more
   return !dStrnicmp(getName(), "Sequence", 8);
}

bool AppNode::isRoot()
{
   // we assume root node isn't added, so this is never true
   // but allow for possibility (by overriding this method)
   // so that isParentRoot still works.
   return false;
}
