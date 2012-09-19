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

#ifndef _SCENELIGHTING_H_
#define _SCENELIGHTING_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _SGSCENEPERSIST_H_
#include "lighting/common/scenePersist.h"
#endif
#ifndef _LIGHTINFO_H_
#include "lighting/lightInfo.h"
#endif

class ShadowVolumeBSP;
class LightInfo;
class AvailableSLInterfaces;

class SceneLighting : public SimObject
{
   typedef SimObject Parent;
protected:
   AvailableSLInterfaces* mLightingInterfaces;
   virtual void getMLName(const char* misName, const U32 missionCRC, const U32 buffSize, char* filenameBuffer);   
public:
   S32 sgTimeTemp;
   S32 sgTimeTemp2;

   virtual void sgNewEvent(U32 light, S32 object, U32 event);

   virtual void sgLightingStartEvent();
   virtual void sgLightingCompleteEvent();

   virtual void sgTGEPassSetupEvent();
   virtual void sgTGELightStartEvent(U32 light);
   virtual void sgTGELightProcessEvent(U32 light, S32 object);
   virtual void sgTGELightCompleteEvent(U32 light);
   virtual void sgTGESetProgress(U32 light, S32 object);

   virtual void sgSGPassSetupEvent();
   virtual void sgSGObjectStartEvent(S32 object);
   virtual void sgSGObjectProcessEvent(U32 light, S32 object);
   virtual void sgSGObjectCompleteEvent(S32 object);
   virtual void sgSGSetProgress(U32 light, S32 object);

   // 'sg' prefix omitted to conform with existing 'addInterior' method...
   void addStatic(ShadowVolumeBSP *shadowVolume, SceneObject *sceneobject, LightInfo *light, S32 level);

   // persist objects moved to 'sgScenePersist.h' for clarity...
   // everything below this line should be original code...

   U32 calcMissionCRC();

   bool verifyMissionInfo(PersistInfo::PersistChunk *);
   bool getMissionInfo(PersistInfo::PersistChunk *);

   bool loadPersistInfo(const char *);
   bool savePersistInfo(const char *);

   class ObjectProxy;

   enum {
      SHADOW_DETAIL = -1
   };

   //------------------------------------------------------------------------------
   /// Create a proxy for each object to store data.
   class ObjectProxy
   {
   public:
      SimObjectPtr<SceneObject>     mObj;
      U32                           mChunkCRC;

      ObjectProxy(SceneObject * obj) : mObj(obj){mChunkCRC = 0;}
      virtual ~ObjectProxy(){}
      SceneObject * operator->() {return(mObj);}
      SceneObject * getObject() {return(mObj);}

      /// @name Lighting Interface
      /// @{
      virtual bool loadResources() {return(true);}
      virtual void init() {}
      virtual bool tgePreLight(LightInfo* light) { return preLight(light); }
      virtual bool preLight(LightInfo *) {return(false);}
      virtual void light(LightInfo *) {}
      virtual void postLight(bool lastLight) {}
      /// @}

      /// @name Lighting events
      /// @{
      // Called when the lighting process begins
      virtual void processLightingStart() {}
      // Called when a TGELight event is started, return true if status has been reported to console
      virtual void processTGELightProcessEvent(U32 curr, U32 max, LightInfo*) { Con::printf("      Lighting object %d of %d...", (curr+1), max); }
      // Called for lighting kit lights
      virtual bool processStartObjectLightingEvent(U32 current, U32 max) { Con::printf("    Lighting object %d of %d... %s: %s", (current+1), max, mObj->getClassName(), mObj->getName()); return true; }
      // Called once per object and SG light - used for calling light on an object
      virtual void processSGObjectProcessEvent(LightInfo* currLight) { light(currLight); };
      /// @}

      /// @name Persistence
      ///
      /// We cache lighting information to cut down on load times.
      ///
      /// There are flags such as ForceAlways and LoadOnly which allow you
      /// to control this behaviour.
      /// @{
      bool calcValidation();
      bool isValidChunk(PersistInfo::PersistChunk *);

      virtual U32 getResourceCRC() = 0;
      virtual bool setPersistInfo(PersistInfo::PersistChunk *);
      virtual bool getPersistInfo(PersistInfo::PersistChunk *);
      /// @}

      // Called to figure out if this object should be added to the shadow volume
      virtual bool supportsShadowVolume() { return false; }
      // Called to retrieve the clip planes of the object.  Currently used for terrain lighting, but could be used to speed up other
      // lighting calculations.  
      virtual void getClipPlanes(Vector<PlaneF>& planes) { }
      // Called to add the object to the shadow volume
      virtual void addToShadowVolume(ShadowVolumeBSP * shadowVolume, LightInfo * light, S32 level) { } ;
   };

   typedef Vector<ObjectProxy*>  ObjectProxyList;

   ObjectProxyList            mSceneObjects;
   ObjectProxyList            mLitObjects;

   LightInfoList              mLights;

   SceneLighting(AvailableSLInterfaces* lightingInterfaces);
   ~SceneLighting();

   enum Flags {
      ForceAlways    = BIT(0),   ///< Regenerate the scene lighting no matter what.
      ForceWritable  = BIT(1),   ///< Regenerate the scene lighting only if we can write to the lighting cache files.
      LoadOnly       = BIT(2),   ///< Just load cached lighting data.
   };
   bool lightScene(const char *, BitSet32 flags = 0);
   bool isLighting();

   S32                        mStartTime;
   char                       mFileName[1024];
   SceneManager * mSceneManager;

   bool light(BitSet32);
   void completed(bool success);
   void processEvent(U32 light, S32 object);
   void processCache();
};

class sgSceneLightingProcessEvent : public SimEvent
{
private:
   U32 sgLightIndex;
   S32 sgObjectIndex;
   U32 sgEvent;

public:
   enum sgEventTypes
   {
      sgLightingStartEventType,
      sgLightingCompleteEventType,

      sgSGPassSetupEventType,
      sgSGObjectStartEventType,
      sgSGObjectCompleteEventType,
      sgSGObjectProcessEventType,

      sgTGEPassSetupEventType,
      sgTGELightStartEventType,
      sgTGELightCompleteEventType,
      sgTGELightProcessEventType
   };

   sgSceneLightingProcessEvent(U32 lightIndex, S32 objectIndex, U32 event)
   {
      sgLightIndex = lightIndex;
      sgObjectIndex = objectIndex;
      sgEvent = event;
   }
   void process(SimObject * object)
   {
      AssertFatal(object, "SceneLightingProcessEvent:: null event object!");
      if(!object)
         return;

      SceneLighting *sl = static_cast<SceneLighting*>(object);
      switch(sgEvent)
      {
      case sgLightingStartEventType:
         sl->sgLightingStartEvent();
         break;
      case sgLightingCompleteEventType:
         sl->sgLightingCompleteEvent();
         break;

      case sgTGEPassSetupEventType:
         sl->sgTGEPassSetupEvent();
         break;
      case sgTGELightStartEventType:
         sl->sgTGELightStartEvent(sgLightIndex);
         break;
      case sgTGELightProcessEventType:
         sl->sgTGELightProcessEvent(sgLightIndex, sgObjectIndex);
         break;
      case sgTGELightCompleteEventType:
         sl->sgTGELightCompleteEvent(sgLightIndex);
         break;

      case sgSGPassSetupEventType:
         sl->sgSGPassSetupEvent();
         break;
      case sgSGObjectStartEventType:
         sl->sgSGObjectStartEvent(sgObjectIndex);
         break;
      case sgSGObjectProcessEventType:
         sl->sgSGObjectProcessEvent(sgLightIndex, sgObjectIndex);
         break;
      case sgSGObjectCompleteEventType:
         sl->sgSGObjectCompleteEvent(sgObjectIndex);
         break;

      default:
         return;
      };
   };
};

extern SceneLighting *gLighting;

#endif//_SGSCENELIGHTING_H_
