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

#ifndef CORE_INTERFACES_H
#define CORE_INTERFACES_H

#ifndef _SCENERENDERSTATE_H_
#include "scene/sceneRenderState.h"
#endif

template<typename T>
class Interface
{
public:
   static Vector<T*> all;

   Interface()
   {
      all.push_back((T*)this);
   }
   virtual ~Interface()
   {
      for (U32 i = 0; i < all.size(); i++)
      {
         if (all[i] == (T*)this)
         {
            all.erase(i);
            return;
         }
      }
   }
};
template<typename T> Vector<T*> Interface<T>::all(0);

//Basically a file for generic interfaces that many behaviors may make use of
class SetTransformInterface// : public Interface<SetTransformInterface>
{
public:
   virtual void setTransform( MatrixF transform );
   virtual void setTransform( Point3F pos, EulerF rot );
};

class UpdateInterface : public Interface<UpdateInterface>
{
public:
   virtual void processTick(){}
   virtual void interpolateTick(F32 dt){}
   virtual void advanceTime(F32 dt){}
};

class BehaviorFieldInterface// : public Interface<BehaviorFieldInterface>
{
public:
   virtual void onFieldChange(const char* fieldName, const char* newValue){};
};

class CameraInterface// : public Interface<CameraInterface>
{
public:
   virtual bool getCameraTransform(F32* pos,MatrixF* mat)=0;
   virtual void onCameraScopeQuery(NetConnection *cr, CameraScopeQuery * query)=0;
   virtual Frustum getFrustum()=0;
   virtual F32 getCameraFov()=0;
   virtual void setCameraFov(F32 fov)=0;

   virtual bool isValidCameraFov(F32 fov)=0;
};

class CastRayInterface// : public Interface<CastRayInterface>
{
public:
   virtual bool castRay(const Point3F &start, const Point3F &end, RayInfo* info)=0;
};

class EditorInspectInterface// : public Interface<EditorInspectInterface>
{
public:
   virtual void onInspect()=0;
   virtual void onEndInspect()=0;
};

#endif