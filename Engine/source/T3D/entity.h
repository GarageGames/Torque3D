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

#ifndef ENTITY_H
#define ENTITY_H

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#ifndef _MOVEMANAGER_H_
#include "T3D/gameBase/moveManager.h"
#endif
#ifndef COMPONENT_H
#include "T3D/components/component.h"
#endif
#ifndef _CONTAINERQUERY_H_
#include "T3D/containerQuery.h"
#endif

class Component;

//**************************************************************************
// Entity
//**************************************************************************
class Entity : public GameBase
{
   typedef GameBase Parent;
   friend class Component;

private:
   Point3F             mPos;
   RotationF           mRot;

   Vector<Component*>         mComponents;

   Vector<Component*>         mToLoadComponents;

   bool                       mStartComponentUpdate;

   ContainerQueryInfo containerInfo;

   bool mInitialized;

   Signal< void(Component*) > onComponentAdded;
   Signal< void(Component*) > onComponentRemoved;

   Signal< void(MatrixF*) > onTransformSet;

protected:

   virtual void   processTick(const Move* move);
   virtual void   advanceTime(F32 dt);
   virtual void   interpolateTick(F32 delta);

   void prepRenderImage(SceneRenderState *state);

   virtual bool onAdd();
   virtual void onRemove();

public:
   struct StateDelta
   {
      Move move;                    ///< Last move from server
      F32 dt;                       ///< Last interpolation time
      // Interpolation data
      Point3F pos;
      Point3F posVec;
      QuatF rot[2];
      // Warp data
      S32 warpTicks;                ///< Number of ticks to warp
      S32 warpCount;                ///< Current pos in warp
      Point3F warpOffset;
      QuatF warpRot[2];
   };

   enum MaskBits
   {
      TransformMask = Parent::NextFreeMask << 0,
      BoundsMask = Parent::NextFreeMask << 1,
      ComponentsMask = Parent::NextFreeMask << 2,
      NoWarpMask = Parent::NextFreeMask << 3,
      NextFreeMask = Parent::NextFreeMask << 4
   };

   StateDelta mDelta;
   S32 mPredictionCount;            ///< Number of ticks to predict

   Move lastMove;

   //
   Entity();
   ~Entity();

   static void    initPersistFields();
   virtual void onPostAdd();

   virtual void setTransform(const MatrixF &mat);
   virtual void setRenderTransform(const MatrixF &mat);

   void setTransform(Point3F position, RotationF rotation);

   void setRenderTransform(Point3F position, RotationF rotation);

   virtual MatrixF getTransform();
   virtual Point3F getPosition() const { return mPos; }

   //void setTransform(Point3F position, RotationF rot);

   //void setRotation(RotationF rotation);

   void setRotation(RotationF rotation) {
      mRot = rotation;
      setMaskBits(TransformMask);
   };
   RotationF getRotation() { return mRot; }

   void setMountOffset(Point3F posOffset);
   void setMountRotation(EulerF rotOffset);

   //static bool _setEulerRotation( void *object, const char *index, const char *data );
   static bool _setPosition(void *object, const char *index, const char *data);
   static const char * _getPosition(void* obj, const char* data);

   static bool _setRotation(void *object, const char *index, const char *data);
   static const char * _getRotation(void* obj, const char* data);

   virtual void getMountTransform(S32 index, const MatrixF &xfm, MatrixF *outMat);
   virtual void getRenderMountTransform(F32 delta, S32 index, const MatrixF &xfm, MatrixF *outMat);

   void setForwardVector(VectorF newForward, VectorF upVector = VectorF::Zero);

   virtual void mountObject(SceneObject *obj, S32 node, const MatrixF &xfm = MatrixF::Identity);
   void mountObject(SceneObject* objB, MatrixF txfm);
   void onMount(SceneObject *obj, S32 node);
   void onUnmount(SceneObject *obj, S32 node);

   // NetObject
   U32 packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn, BitStream *stream);

   void setComponentsDirty();
   void setComponentDirty(Component *comp, bool forceUpdate = false);

   //Components
   virtual bool deferAddingComponents() const { return true; }

   template <class T>
   T* getComponent();
   template <class T>
   Vector<T*> getComponents();

   Component* getComponent(String componentType);

   U32 getComponentCount() const
   { 
      return mComponents.size(); 
   }

   virtual void setObjectBox(Box3F objBox);

   void resetWorldBox() { Parent::resetWorldBox(); }
   void resetObjectBox() { Parent::resetObjectBox(); }
   void resetRenderWorldBox() { Parent::resetRenderWorldBox(); }

   //function redirects for collisions
   bool castRay(const Point3F &start, const Point3F &end, RayInfo* info);
   bool castRayRendered(const Point3F &start, const Point3F &end, RayInfo* info);
   bool buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F &box, const SphereF &sphere);
   virtual void buildConvex(const Box3F& box, Convex* convex);

   Signal< void(SimObject*, String, String) > onDataSet;
   virtual void setDataField(StringTableEntry slotName, const char *array, const char *value);
   virtual void onStaticModified(const char* slotName, const char* newValue);

   //void pushEvent(const char* eventName, Vector<const char*> eventParams);

   void updateContainer();

   ContainerQueryInfo getContainerInfo() { return containerInfo; }

   //camera stuff
   virtual void getCameraTransform(F32* pos, MatrixF* mat);
   virtual void onCameraScopeQuery(NetConnection* connection, CameraScopeQuery* query);

   //Heirarchy stuff
   virtual void addObject(SimObject* object);
   virtual void removeObject(SimObject* object);

   virtual SimObject* findObjectByInternalName(StringTableEntry internalName, bool searchChildren);

   //component stuff
   bool addComponent(Component *comp);
   bool removeComponent(Component *comp, bool deleteComponent);
   void clearComponents(bool deleteComponents = true);
   Component* getComponent(const U32 index) const;

   void onInspect();
   void onEndInspect();

   virtual void write(Stream &stream, U32 tabStop, U32 flags);

   // TamlChildren
   virtual U32 getTamlChildCount(void) const
   {
      U32 componentCount = getComponentCount();
      U32 childSize = (U32)size();
      return componentCount + childSize;
   }

   virtual SimObject* getTamlChild(const U32 childIndex) const;

   virtual void addTamlChild(SimObject* pSimObject)
   {
      // Sanity!
      AssertFatal(pSimObject != NULL, "SimSet::addTamlChild() - Cannot add a NULL child object.");

      addObject(pSimObject);
   }

   Box3F getObjectBox() { return mObjBox; }
   MatrixF getWorldToObj() { return mWorldToObj; }
   MatrixF getObjToWorld() { return mObjToWorld; }

   DECLARE_CONOBJECT(Entity);

};

template <class T>
T *Entity::getComponent()
{
   U32 componentCount = getComponentCount();
   for (U32 i = 0; i < componentCount; i++)
   {
      T* t = dynamic_cast<T *>(mComponents[i]);

      if (t) 
      {
         return t;
      }
   }
   return NULL;
}

template <class T>
Vector<T*> Entity::getComponents()
{
   Vector<T*> foundObjects;

   T *curObj;
   Component* comp;

   // Loop through our child objects.
   for (U32 i = 0; i < mComponents.size(); i++)
   {
      curObj = dynamic_cast<T*>(mComponents[i]);

      // Add this child object if appropriate.
      if (curObj)
         foundObjects.push_back(curObj);
   }

   return foundObjects;
}
#endif //ENTITY_H
