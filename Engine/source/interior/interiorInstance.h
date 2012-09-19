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

#ifndef _INTERIORINSTANCE_H_
#define _INTERIORINSTANCE_H_

#ifndef _SCENEZONESPACE_H_
#include "scene/zones/sceneZoneSpace.h"
#endif

#ifndef _INTERIORRES_H_
#include "interior/interiorRes.h"
#endif

#ifndef _INTERIORLMMANAGER_H_
#include "interior/interiorLMManager.h"
#endif

#ifndef _BITVECTOR_H_
#include "core/bitVector.h"
#endif

#ifndef _COLOR_H_
#include "core/color.h"
#endif

#ifndef _INTERIOR_H_
#include "interior.h"
#endif

#ifndef _REFLECTOR_H_
#include "scene/reflector.h"
#endif

#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif


class AbstractPolyList;
class InteriorSubObject;
class InteriorResTrigger;
class MaterialList;
class TextureObject;
class Convex;
class SFXProfile;
class SFXEnvironment;
class PhysicsBody;


/// Instance of a DIF interior.
class InteriorInstance : public SceneZoneSpace
{
   public:

      friend class Interior;
   
      typedef SceneZoneSpace Parent;

      static bool smDontRestrictOutside;
      static F32  smDetailModification;

   protected:

      enum UpdateMaskBits
      {
         InitMask       = Parent::NextFreeMask << 0,
         AlarmMask      = Parent::NextFreeMask << 1,

         // Reserved for light updates (8 bits for now)
         _lightupdate0  = Parent::NextFreeMask << 2,
         _lightupdate1  = Parent::NextFreeMask << 3,
         _lightupdate2  = Parent::NextFreeMask << 4,
         _lightupdate3  = Parent::NextFreeMask << 5,
         _lightupdate4  = Parent::NextFreeMask << 6,
         _lightupdate5  = Parent::NextFreeMask << 7,
         _lightupdate6  = Parent::NextFreeMask << 8,
         _lightupdate7  = Parent::NextFreeMask << 9,

         SkinBaseMask   = Parent::NextFreeMask << 10,
         NextFreeMask   = Parent::NextFreeMask << 11,
      };

      enum
      {
         LightUpdateBitStart = 3,
         LightUpdateBitEnd   = 10
      };

      enum AlarmState {
         Normal          = 0,
         Alarm           = 1
      };

      /// Alarm state of the interior
      bool mAlarmState;

      /// File name of the interior this instance encapuslates
      StringTableEntry mInteriorFileName;

      /// Hash for interior file name, used for sorting
      U32 mInteriorFileHash;

      /// Interior managed by resource manager
      Resource<InteriorResource> mInteriorRes;

      /// Forced LOD, if -1 auto LOD
      S32 mForcedDetailLevel;

       /// CRC for the interior
      U32 mCRC;

      /// Handle to the light manager
      LM_HANDLE mLMHandle;

      Convex* mConvexList;

      PhysicsBody* mPhysicsRep;

      Vector< PlaneReflector > mPlaneReflectors;
      ReflectorDesc mReflectorDesc;

      U32 _calcDetailLevel( SceneRenderState* state, const Point3F& wsPoint );
      bool _loadInterior();
      void _unloadInterior();
      void _renderObject( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* );
      bool _getOverlappingZones( const Box3F& aabb, const MatrixF& transform, const Point3F& scale, U32* outZones, U32& outNumZones );

      /// Creates a transform based on an trigger area
      /// @param   trigger   Trigger to create a transform for
      /// @param   transform Transform generated (out)
      void _createTriggerTransform(const InteriorResTrigger *trigger, MatrixF *transform);

      // SceneObject.
      virtual bool onSceneAdd();
      virtual void onSceneRemove();

   public:

      InteriorInstance();
      virtual ~InteriorInstance();

      StringTableEntry getInteriorFileName() { return mInteriorFileName; }

      S32 getSurfaceZone(U32 surfaceindex, Interior *detail);
      
      /// Exports the interior to a Collada file
      /// @param   bakeTransform  Bakes the InteriorInstance's transform into the vertex positions
      void exportToCollada(bool bakeTransform = false);

      /// Returns the Light Manager handle
      LM_HANDLE getLMHandle() { return(mLMHandle); }

      /// Reads the lightmaps of the interior into the provided pointer
      /// @param   lightmaps   Lightmaps in the interior (out)
      bool readLightmaps(GBitmap ****lightmaps);

      /// This is used to determine just how 'inside' a point is in an interior.
      /// This is used by the environmental audio code for audio properties and the
      /// function always returns true.
      /// @param   pos   Point to test
      /// @param   pScale   How inside is the point 0 = totally outside, 1 = totally inside (out)
      bool getPointInsideScale(const Point3F & pos, F32 * pScale);   // ~0: outside -> 1: inside

      /// Returns the interior resource
      Resource<InteriorResource> & getResource() {return(mInteriorRes);} // SceneLighting::InteriorProxy interface

      /// Returns the CRC for validation
      U32 getCRC() { return(mCRC); }

      /// @name Alarm States
      /// @{

      /// This returns true if the interior is in an alarm state. Alarm state
      /// will put different lighting into the interior and also possibly
      /// have an audio element also.
      bool inAlarmState() {return(mAlarmState);}

      /// This sets the alarm mode of the interior.
      /// @param   alarm   If true the interior will be in an alarm state next frame
      void setAlarmMode(const bool alarm);

      /// @}

      /// @name Subobject access interface
      /// @{

      /// Returns the number of detail levels for an object
      U32 getNumDetailLevels();

      /// Gets the interior associated with a particular detail level
      /// @param   level   Detail level
      Interior* getDetailLevel(const U32 level);

      /// Sets the detail level to render manually
      /// @param   level   Detail level to force
      void setDetailLevel(S32 level = -1) { mForcedDetailLevel = level; }

      /// @}

      // SimObject.
      DECLARE_CONOBJECT( InteriorInstance );

      virtual bool onAdd();
      virtual void onRemove();
      virtual void inspectPostApply();

      static void initPersistFields();
      static void consoleInit();

      // NetObject.
      virtual U32 packUpdate( NetConnection* conn, U32 mask, BitStream* stream );
      virtual void unpackUpdate( NetConnection* conn, BitStream* stream );

      // SceneObject.
      virtual bool buildPolyList(PolyListContext context, AbstractPolyList *polyList, const Box3F &box, const SphereF &sphere);
      virtual bool castRay(const Point3F &start, const Point3F &end, RayInfo *info);
      virtual void buildConvex(const Box3F& box,Convex* convex);
      virtual void prepRenderImage( SceneRenderState *state );

      // SceneZoneSpace.
      virtual void traverseZones( SceneTraversalState* state );
      virtual void traverseZones( SceneTraversalState* state, U32 startZoneId );
      virtual U32 getPointZone( const Point3F& p );
      virtual bool getOverlappingZones( const Box3F& aabb, U32* outZones, U32& outNumZones );
      virtual bool getOverlappingZones( SceneObject* obj, U32* outZones, U32& outNumZones );

   private:

      // Protected field accessors
      static bool _setInteriorFile( void *object, const char *, const char *data ); 
};

#endif //_INTERIORBLOCK_H_

