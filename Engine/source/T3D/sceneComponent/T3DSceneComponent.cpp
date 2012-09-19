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

#include "T3DSceneComponent.h"
#include "T3DSceneClient.h"

void T3DSceneComponent::setSceneGroup(const char * sceneGroup)
{
   AssertFatal(getOwner()==NULL, "Changing scene group name after registration will have no effect.");
   if (sceneGroup == NULL)
      sceneGroup = StringTable->insert("");
   _sceneGroup = StringTable->insert(sceneGroup);
}

void T3DSceneComponent::setParentTransformName(const char * name)
{
   _parentTransformName = StringTable->insert(name);
   if (getOwner() != NULL)
   {
      Transform3D * old = _transform->getParentTransform();
      ValueWrapperInterface<Transform3D*> * iface = NULL;
      if (_parentTransformName != NULL)
         iface = getInterface<ValueWrapperInterface<Transform3D*> >("transform3D", _parentTransformName);
      _transform->setParentTransform(iface == NULL ? NULL : iface->get());
      if (_transform->getParentTransform() != old)
         setDirtyWorldBox(true);
   }
}

void T3DSceneComponent::setObjectType(U32 objectTypeMask)
{
   _objectType = objectTypeMask;
   setUseOwnerObjectType(true); 
}

void T3DSceneComponent::setDirtyObjectBox(bool val)
{
   _SetFlag(T3DSceneComponent::DirtyObjectBox, val);
   if (val && !isObjectBoxLocked())
      _ComputeObjectBox();
}

void T3DSceneComponent::setDirtyWorldBox(bool val)
{
   _SetFlag(T3DSceneComponent::DirtyWorldBox, val);
   if (val && !isWorldBoxLocked())
      _UpdateWorldBox();
}

void T3DSceneComponent::setObjectBoxLocked(bool val)
{
   _SetFlag(T3DSceneComponent::LockObjectBox, val);
   if (!val && isDirtyObjectBox())
      _ComputeObjectBox();
}

void T3DSceneComponent::setWorldBoxLocked(bool val)
{
   _SetFlag(T3DSceneComponent::LockWorldBox, val);
   if (!val && isDirtyWorldBox())
      _UpdateWorldBox();
}

void T3DSceneComponent::setPosition(const Point3F & pos)
{
   _transform->setPosition(pos);
   setDirtyWorldBox(true);
}

void T3DSceneComponent::setRotation(const QuatF & rotation)
{
   _transform->setRotation(rotation);
   setDirtyWorldBox(true);
}

void T3DSceneComponent::setScale(const Point3F & scale)
{
   _transform->setScale (scale);
   Parent::setScale(scale);
   setDirtyWorldBox(true);
}

void T3DSceneComponent::setTransform3D(Transform3D * transform)
{
   if (transform == NULL)
      // never let it be null
      return;
   if (_transform != NULL)
      delete _transform;
   _transform = transform;
   _transform->setDirtyListener(this);
}

void T3DSceneComponent::setTransform(const MatrixF & mat)
{
   _transform->setLocalMatrix(mat);
   Parent::setTransform(mat);
   setDirtyWorldBox(true);
}

Box3F T3DSceneComponent::getObjectBox()
{
   if (DirtyObjectBox && !LockObjectBox)
      _ComputeObjectBox();

   return _objectBox->get(); 
}

Box3F T3DSceneComponent::getWorldBox()
{
   if (DirtyWorldBox && !LockWorldBox)
      _UpdateWorldBox();

   MatrixF mat = getTransform3D()->getWorldMatrix();
   Box3F box = getObjectBox();

   mat.mul(box);
   return box;
}

void T3DSceneComponent::Render()
{
   if (_sceneClientList == NULL)
      return;

   PROFILE_SCOPE(T3DSceneComponent_Render);

   GFX->multWorld(_transform->getWorldMatrix());

   if (doRenderObjectBounds())
   {
      // TODO
   }
   if (doRenderWorldBounds())
   {
      // TODO
   }

   for (T3DSceneClient * walk = _sceneClientList; walk != NULL; walk = walk->getNextSceneClient())
   {
      IRenderable3D * render = dynamic_cast<IRenderable3D*>(walk);
      if (render != NULL)
         render->Render();
   }

   GFX->popWorldMatrix();
}

void T3DSceneComponent::AddSceneClient(T3DSceneClient * sceneClient)
{
   AssertFatal(sceneClient,"Cannot add a NULL scene client");

   // add to the front of the list
   sceneClient->setNextSceneClient(_sceneClientList);
   _sceneClientList = sceneClient;

   // extend object box
   ISolid3D * solid = dynamic_cast<ISolid3D*>(sceneClient);
   if (solid != NULL)
   {
      if (isObjectBoxLocked())
         setDirtyObjectBox(true);
      else
      {
         Box3F box = solid->getObjectBox();
         if (solid->getTransform3D() != NULL)
         {
            MatrixF mat;
            solid->getTransform3D()->getObjectMatrix(mat, true);
            mat.mul(box);
         }
         box.extend(_objectBox->get().min);
         box.extend(_objectBox->get().max);
         _objectBox->set(box);
      }

      // policy is that we become parent transform
      if (solid->getTransform3D() != NULL && solid->getTransform3D()->isChildOf(_transform, true))
         solid->getTransform3D()->setParentTransform(_transform);
   }
}

void T3DSceneComponent::RemoveClientObject(T3DSceneClient * sceneClient)
{
   AssertFatal(sceneClient != NULL, "Removing null scene client");
   if (sceneClient == NULL)
      return;
   if (_sceneClientList == sceneClient)
      _sceneClientList = _sceneClientList->getNextSceneClient();
   else
   {
      T3DSceneClient * walk = _sceneClientList;
      while (walk->getNextSceneClient() != sceneClient && walk != NULL)
         walk = walk->getNextSceneClient();
      if (walk != NULL)
         walk->setNextSceneClient(sceneClient->getNextSceneClient());
   }

   if (dynamic_cast<ISolid3D*>(sceneClient))
      setDirtyObjectBox(true);
}

bool T3DSceneComponent::onComponentRegister(SimComponent * owner)
{
   if (!Parent::onComponentRegister(owner) || !registerObject())
      return false;

   // Note: was added to scene graph in register object

   setTransform3D(_transform);
   setParentTransformName(_parentTransformName);

   return true;
}

void T3DSceneComponent::onComponentUnRegister()
{
   _sceneClientList = NULL;
   Parent::onComponentUnRegister();
}

void T3DSceneComponent::registerInterfaces(SimComponent * owner)
{
   Parent::registerInterfaces(owner);

   // TODO: need to figure out the best way to wrap these
   // we don't need to track these interfaces ourselves -- just create them here
   ComponentInterface * sceneComponent = new  ComponentInterface();
   ValueWrapperInterface<Transform3D*> * transform = new  ValueWrapperInterface<Transform3D*>(_transform);

   registerCachedInterface("sceneComponent", _sceneGroup, owner, sceneComponent);
   registerCachedInterface("transform3D", _sceneGroup, owner, transform);
   registerCachedInterface("box", _sceneGroup, this, _objectBox);
}

void T3DSceneComponent::_ComputeObjectBox()
{
   _objectBox->set(Box3F(10E30f, 10E30f, 10E30f, -10E30f, -10E30f, -10E30f));
   bool gotone = false;
   for (T3DSceneClient * walk = _sceneClientList; walk != NULL; walk = walk->getNextSceneClient())
   {
      ISolid3D * solid = dynamic_cast<ISolid3D*>(walk);
      if (solid == NULL)
         continue;

      Box3F box = solid->getObjectBox();
      if (solid->getTransform3D() != NULL)
      {
         MatrixF mat;
         solid->getTransform3D()->getObjectMatrix(mat, true);
         mat.mul(box);
      }
      box.extend(_objectBox->get().min);
      box.extend(_objectBox->get().max);
      _objectBox->set(box);
      gotone = true;
   }
   if (!gotone)
      _objectBox->set(Box3F());

   setDirtyObjectBox(false);
   setDirtyWorldBox(true);
}

void T3DSceneComponent::_UpdateWorldBox()
{
   setDirtyWorldBox(false);
   // TODO:
   // T3DSceneGraph.Instance.UpdateObject(this);
}
