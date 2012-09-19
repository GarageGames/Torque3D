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

#ifndef _T3DSCENECOMPONENT_H_
#define _T3DSCENECOMPONENT_H_

#include "T3DTransform.h"
#include "component/simComponent.h"
#include "sceneGraph/sceneobject.h"

class T3DSceneClient;

class IRenderable3D
{
public:
   // TODO: what parameters to use?
   virtual void Render() = 0;
};

class ISolid3D
{
public:
   virtual Box3F getObjectBox() = 0;
   virtual Transform3D * getTransform3D() = 0;
};

class T3DSceneComponent : public SceneObject, public Transform3D::IDirtyListener
{
   typedef SceneObject Parent;

public:

   T3DSceneComponent()
   {
      _transform = new Transform3DInPlace();
      _objectBox = new ValueWrapperInterface<Box3F>();
      _flags = T3DSceneComponent::Visible | T3DSceneComponent::DirtyObjectBox | T3DSceneComponent::DirtyWorldBox | T3DSceneComponent::UseOwnerObjectType;
      _visibilityLevel = 1.0f;
      _objectType = 0;
      _sceneGroup = NULL;
      _parentTransformName = NULL;
      setSceneGroup(NULL);
   }

   StringTableEntry getSceneGroup() const { return _sceneGroup; }
   void setSceneGroup(const char * sceneGroup);

   StringTableEntry getParentTransformName() const { return _parentTransformName; }
   void setParentTransformName(const char * name);

   U32 getObjectType() const { return _objectType; } // TODO: use owner if useowner set
   void setObjectType(U32 objectTypeMask);

   void setUseOwnerObjectType(bool val) { _SetFlag(T3DSceneComponent::UseOwnerObjectType, val); }
   bool useOwnerObjectType() const { return _TestFlag(T3DSceneComponent::UseOwnerObjectType); }

   bool isDirtyObjectBox() const { return _TestFlag(T3DSceneComponent::DirtyObjectBox); }
   void setDirtyObjectBox(bool val);

   bool isDirtyWorldBox() const { return _TestFlag(T3DSceneComponent::DirtyWorldBox); }
   void setDirtyWorldBox(bool val);

   bool isObjectBoxLocked() const { return _TestFlag(T3DSceneComponent::LockObjectBox); }
   void setObjectBoxLocked(bool val);

   bool isWorldBoxLocked() const { return _TestFlag(T3DSceneComponent::LockWorldBox); }
   void setWorldBoxLocked(bool val);

   bool doRenderObjectBounds() const { return _TestFlag(T3DSceneComponent::RenderObjectBounds); }
   void setRenderObjectBounds(bool val) { _SetFlag(T3DSceneComponent::RenderObjectBounds, val); }

   bool doRenderWorldBounds() const { return _TestFlag(T3DSceneComponent::RenderWorldBounds); }
   void setRenderWorldBounds(bool val) { _SetFlag(T3DSceneComponent::RenderWorldBounds, val); }

   bool doRenderSubBounds() const { return _TestFlag(T3DSceneComponent::RenderSubBounds); }
   void setRenderSubBounds(bool val) { _SetFlag(T3DSceneComponent::RenderSubBounds, val); }

   bool isVisible() const { return _TestFlag(T3DSceneComponent::Visible); }
   void setVisible(bool val) { _SetFlag(T3DSceneComponent::Visible, val); }

   float getVisibilityLevel() const { return _visibilityLevel; }
   void setVisibilityLevel(float val) { _visibilityLevel = val; }

   Point3F getPosition() const { return _transform->getPosition(); }
   void setPosition(const Point3F & pos);

   QuatF getRotation() const  { return _transform->getRotation(); }
   void setRotation(const QuatF & rotation);

   Point3F getScale() const { return _transform->getScale(); }
   void setScale(const Point3F & scale);

   Transform3D * getTransform3D(){ return _transform; }

   void setTransform(const MatrixF & mat);

   Box3F getObjectBox();
   Box3F getWorldBox();

   T3DSceneClient * getSceneClientList() const { return _sceneClientList; }

   void Render();

   void AddSceneClient(T3DSceneClient * sceneClient);
   void RemoveClientObject(T3DSceneClient * sceneClient);

   void OnTransformDirty() { setDirtyWorldBox(true); }

protected:

   bool onComponentRegister(SimComponent * owner);
   void onComponentUnRegister();

   void registerInterfaces(SimComponent * owner);

   void _ComputeObjectBox();
   void _UpdateWorldBox();
   void setTransform3D(Transform3D * transform);

   bool _TestFlag(U32 test) const
   {
      return (_flags & test) != T3DSceneComponent::None;
   }

   void _SetFlag(U32 test, bool value)
   {
      if (value)
         _flags |= test;
      else
         _flags &= ~test;
   }

   enum SceneFlags
   {
      None = 0,
      Visible = 1 << 0,
      DirtyObjectBox = 1 << 1,
      DirtyWorldBox = 1 << 2,
      LockObjectBox = 1 << 3,
      LockWorldBox = 1 << 4,
      UseOwnerObjectType = 1 << 5,
      RenderDebug = 1 << 6,
      RenderObjectBounds = 1 << 7,
      RenderWorldBounds = 1 << 8,
      RenderSubBounds = 1 << 9,
      LastFlag = 1 << 9
   };

   Transform3D * _transform;
   T3DSceneClient * _sceneClientList;
   ValueWrapperInterface<Box3F> * _objectBox;
   U32 _flags;
   float _visibilityLevel;
   U32 _objectType;
   StringTableEntry _sceneGroup;
   StringTableEntry _parentTransformName ;
};

#endif // #ifndef _T3DSCENECOMPONENT_H_
