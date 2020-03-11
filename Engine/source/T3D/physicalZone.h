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

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#ifndef _H_PHYSICALZONE
#define _H_PHYSICALZONE

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _EARLYOUTPOLYLIST_H_
#include "collision/earlyOutPolyList.h"
#endif
#ifndef _MPOLYHEDRON_H_
#include "math/mPolyhedron.h"
#endif

class Convex;


class PhysicalZone : public SceneObject
{
   typedef SceneObject Parent;

   enum UpdateMasks {
      ActiveMask        = Parent::NextFreeMask << 0,
      SettingsMask      = Parent::NextFreeMask << 1,
      FadeMask          = Parent::NextFreeMask << 2,
      PolyhedronMask    = Parent::NextFreeMask << 3,
      MoveMask          = Parent::NextFreeMask << 4,
      ExclusionMask     = Parent::NextFreeMask << 5,
      NextFreeMask      = Parent::NextFreeMask << 6
   };

  protected:
   static bool smRenderPZones;

   F32        mVelocityMod;
   F32        mGravityMod;
   Point3F    mAppliedForce;

   // Basically ripped from trigger
   Polyhedron           mPolyhedron;
   EarlyOutPolyList     mClippedList;

   bool mActive;

   Convex* mConvexList;
   void buildConvex(const Box3F& box, Convex* convex);

  public:
   PhysicalZone();
   ~PhysicalZone();

   // SimObject
   DECLARE_CONOBJECT(PhysicalZone);
   static void consoleInit();
   static void initPersistFields();
   bool onAdd();
   void onRemove();
   void inspectPostApply();

   // NetObject
   U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn,           BitStream *stream);

   // SceneObject
   void setTransform(const MatrixF &mat);
   void prepRenderImage( SceneRenderState* state );

   inline F32 getVelocityMod() const      { return mVelocityMod; }
   inline F32 getGravityMod()  const      { return mGravityMod;  }
   // the scene object is now passed in to getForce() where
   // it is needed to calculate the applied force when the
   // force is radial.
   const Point3F& getForce(const Point3F* center=0) const;

   void setPolyhedron(const Polyhedron&);
   bool testObject(SceneObject*);

   bool testBox( const Box3F &box ) const;

   void renderObject( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat );

   void activate();
   void deactivate();
   inline bool isActive() const { return mActive; }

protected:
   friend class afxPhysicalZoneData;
   friend class afxEA_PhysicalZone;
   Vector<SceneObject*>  excluded_objects;
   S32    force_type;
   F32    force_mag;
   bool   orient_force;
   F32    fade_amt;
   void   setFadeAmount(F32 amt) { fade_amt = amt; if (fade_amt < 1.0f) setMaskBits(FadeMask); }
public:
   enum ForceType { VECTOR, SPHERICAL, CYLINDRICAL };
   enum { FORCE_TYPE_BITS = 2 };
   virtual void onStaticModified(const char* slotName, const char*newValue = NULL);
   bool   isExcludedObject(SceneObject*) const;
   void   registerExcludedObject(SceneObject*);
   void   unregisterExcludedObject(SceneObject*);
};

typedef PhysicalZone::ForceType PhysicalZone_ForceType;
DefineEnumType( PhysicalZone_ForceType );
#endif // _H_PHYSICALZONE

