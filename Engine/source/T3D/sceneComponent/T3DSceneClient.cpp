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

#include "T3DSceneClient.h"

//---------------------------------------------------
// T3DSceneClient
//---------------------------------------------------

void T3DSceneClient::setSceneGroupName(const char * name)
{
   _sceneGroupName = StringTable->insert(name);
   if (getOwner() != NULL)
   {
      if (_sceneGroup != NULL)
         _sceneGroup->RemoveClientObject(this);
      _sceneGroup = NULL;

      ValueWrapperInterface<T3DSceneComponent*> * iface = getInterface<ValueWrapperInterface<T3DSceneComponent*> >("sceneComponent", _sceneGroupName);
      if (iface != NULL)
      {
         _sceneGroup = iface->get();
         _sceneGroup->AddSceneClient(this);
      }
   }
}

bool T3DSceneClient::onComponentRegister(SimComponent * owner)
{
   if (!Parent::onComponentRegister(owner))
      return false;

   // lookup scene group and add ourself
   setSceneGroupName(_sceneGroupName);

   if (_sceneGroupName != NULL && dStricmp(_sceneGroupName, "none") && _sceneGroup == NULL)
      // tried to add ourself to a scene group but failed, fail to add component
      return false;

   return true;
}

void T3DSceneClient::registerInterfaces(SimComponent * owner)
{
   Parent::registerInterfaces(owner);
   registerCachedInterface("sceneClient", NULL, this, new ValueWrapperInterface<T3DSceneClient>());
}

//---------------------------------------------------
// T3DSceneClient
//---------------------------------------------------

Box3F T3DSolidSceneClient::getWorldBox()
{
   MatrixF mat = getTransform();
   Box3F box = _objectBox->get();
   mat.mul(box);
   return box;
}

const MatrixF & T3DSolidSceneClient::getTransform()
{
   if (_transform != NULL)
      return _transform->getWorldMatrix();
   else if (getSceneGroup() != NULL)
      return getSceneGroup()->getTransform3D()->getWorldMatrix();
   else
      return MatrixF::smIdentity;
}

void T3DSolidSceneClient::setTransform3D(Transform3D *  transform)
{
   if (_transform != NULL)
      _transform->setDirtyListener(NULL);
   _transform = transform;

   _transform->setDirtyListener(this);
   OnTransformDirty();
}

void T3DSolidSceneClient::OnTransformDirty()
{
   // TODO: need a way to skip this...a flag, but we don't want to add a bool just for that
   // reason we might want to skip it is if we have a renderable that orbits an object but always
   // stays within object box.  Want to be able to use that info to skip object box updates.
   if (getSceneGroup() != NULL)
      getSceneGroup()->setDirtyObjectBox(true);
}
