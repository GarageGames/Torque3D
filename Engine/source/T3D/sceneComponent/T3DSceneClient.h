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

#ifndef _T3DSCENECLIENT_H_
#define _T3DSCENECLIENT_H_

#include "component/simComponent.h"
#include "T3DSceneComponent.h"

class T3DSceneClient : public SimComponent
{
   typedef SimComponent Parent;

public:
   T3DSceneClient()
   {
      _nextClient = NULL;
      _sceneGroup = NULL;
      _sceneGroupName = NULL;
      _sceneClientName = NULL;
   }

   T3DSceneClient * getNextSceneClient() { return _nextClient; }
   // TODO: internal
   void setNextSceneClient(T3DSceneClient * client) { _nextClient = client; }

   T3DSceneComponent * getSceneGroup() { return _sceneGroup; }

   StringTableEntry getSceneGroupName() { return _sceneGroupName; }
   void setSceneGroupName(const char * name);

   StringTableEntry getSceneClientName() { return _sceneClientName; }
   void setSceneClientName(const char * name) { _sceneClientName = StringTable->insert(name); }

protected:

   bool onComponentRegister(SimComponent * owner);
   void registerInterfaces(SimComponent * owner);

   T3DSceneClient * _nextClient;
   T3DSceneComponent * _sceneGroup;
   StringTableEntry _sceneGroupName;
   StringTableEntry _sceneClientName;
};

class T3DSolidSceneClient : public T3DSceneClient, public ISolid3D, public Transform3D::IDirtyListener
{
public:

   T3DSolidSceneClient()
   {
      _transform = NULL;
      _objectBox = new ValueWrapperInterface<Box3F>();
   }

   Box3F getObjectBox() { return _objectBox->get(); }
   void setObjectBox(const Box3F & box) { _objectBox->set(box); }
   Box3F getWorldBox();
   const MatrixF & getTransform();
   Transform3D * getTransform3D() { return _transform; }

   void OnTransformDirty();

protected:


   // TODO: internal
   void setTransform3D(Transform3D *  transform);

   Transform3D * _transform;
   ValueWrapperInterface<Box3F> * _objectBox;
};

#endif // #ifndef _T3DSCENECLIENT_H_
