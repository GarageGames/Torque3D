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

#include "T3D/physicalZone.h"
#include "core/stream/bitStream.h"
#include "collision/boxConvex.h"
#include "collision/clippedPolyList.h"
#include "console/consoleTypes.h"
#include "math/mathIO.h"
#include "scene/sceneRenderState.h"
#include "T3D/trigger.h"
#include "gfx/gfxTransformSaver.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"

//#include "console/engineTypes.h"
#include "sim/netConnection.h"
IMPLEMENT_CO_NETOBJECT_V1(PhysicalZone);

ConsoleDocClass( PhysicalZone,
   "@brief Physical Zones are areas that modify the player's gravity and/or velocity and/or applied force.\n\n"

   "The datablock properties determine how the physics, velocity and applied forces affect a player who enters this zone.\n"

   "@tsexample\n"
      "new PhysicalZone(Team1JumpPad) {\n"
         "velocityMod = \"1\";"
         "gravityMod = \"0\";\n"
         "appliedForce = \"0 0 20000\";\n"
         "polyhedron = \"0.0000000 0.0000000 0.0000000 1.0000000 0.0000000 0.0000000 0.0000000 -1.0000000 0.0000000 0.0000000 0.0000000 1.0000000\";\n"
         "position = \"273.559 -166.371 249.856\";\n"
         "rotation = \"0 0 1 13.0216\";\n"
         "scale = \"8 4.95 28.31\";\n"
         "isRenderEnabled = \"true\";\n"
         "canSaveDynamicFields = \"1\";\n"
         "enabled = \"1\";\n"
      "};\n"
   "@endtsexample\n\n"
   "@ingroup enviroMisc\n"
);

bool PhysicalZone::smRenderPZones = false;

DefineEngineMethod(PhysicalZone, activate, void, (),, "Activate the physical zone's effects.\n"
													"@tsexample\n"
														"// Activate effects for a specific physical zone.\n"
														"%thisPhysicalZone.activate();\n"
													"@endtsexample\n"
													"@ingroup Datablocks\n"
				  )
{
   if (object->isClientObject())
      return;

   object->activate();
}

DefineEngineMethod(PhysicalZone, deactivate, void, (),, "Deactivate the physical zone's effects.\n"
													"@tsexample\n"
														"// Deactivate effects for a specific physical zone.\n"
														"%thisPhysicalZone.deactivate();\n"
													"@endtsexample\n"
													"@ingroup Datablocks\n"
				  )
{
   if (object->isClientObject())
      return;

   object->deactivate();
}


//--------------------------------------------------------------------------
//--------------------------------------
//
PhysicalZone::PhysicalZone()
{
   mNetFlags.set(Ghostable | ScopeAlways);

   mTypeMask |= PhysicalZoneObjectType;

   mVelocityMod = 1.0f;
   mGravityMod  = 1.0f;
   mAppliedForce.set(0, 0, 0);

   mConvexList = new Convex;
   mActive = true;
   force_type = VECTOR;
   force_mag = 0.0f;
   orient_force = false;
   fade_amt = 1.0f;
}

PhysicalZone::~PhysicalZone()
{
   delete mConvexList;
   mConvexList = NULL;
}


ImplementEnumType( PhysicalZone_ForceType, "Possible physical zone force types.\n" "@ingroup PhysicalZone\n\n" )
   { PhysicalZone::VECTOR,          "vector",        "..." },
   { PhysicalZone::SPHERICAL,       "spherical",     "..." },
   { PhysicalZone::CYLINDRICAL,     "cylindrical",   "..." },
   // aliases
   { PhysicalZone::SPHERICAL,       "sphere",        "..." },
   { PhysicalZone::CYLINDRICAL,     "cylinder",      "..." },
EndImplementEnumType;

//--------------------------------------------------------------------------
void PhysicalZone::consoleInit()
{
   Con::addVariable( "$PhysicalZone::renderZones", TypeBool, &smRenderPZones, "If true, a box will render around the location of all PhysicalZones.\n"
	   "@ingroup EnviroMisc\n");
}

void PhysicalZone::initPersistFields()
{
   addGroup("Misc");
   addField("velocityMod",  TypeF32,               Offset(mVelocityMod,  PhysicalZone), "Multiply velocity of objects entering zone by this value every tick.");
   addField("gravityMod",   TypeF32,               Offset(mGravityMod,   PhysicalZone), "Gravity in PhysicalZone. Multiplies against standard gravity.");
   addField("appliedForce", TypePoint3F,           Offset(mAppliedForce, PhysicalZone), "Three-element floating point value representing forces in three axes to apply to objects entering PhysicalZone.");
   addField("polyhedron",   TypeTriggerPolyhedron, Offset(mPolyhedron,   PhysicalZone),
      "The polyhedron type is really a quadrilateral and consists of a corner"
      "point followed by three vectors representing the edges extending from the corner." );
   endGroup("Misc");

   addGroup("AFX");
   addField("forceType", TYPEID<PhysicalZone::ForceType>(), Offset(force_type, PhysicalZone));
   addField("orientForce", TypeBool, Offset(orient_force, PhysicalZone));
   endGroup("AFX");
   Parent::initPersistFields();
}

//--------------------------------------------------------------------------
bool PhysicalZone::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (mVelocityMod < -40.0f || mVelocityMod > 40.0f) {
      Con::errorf("PhysicalZone: velocity mod out of range.  [-40, 40]");
      mVelocityMod = mVelocityMod < -40.0f ? -40.0f : 40.0f;
   }
   if (mGravityMod < -40.0f || mGravityMod > 40.0f) {
      Con::errorf("PhysicalZone: GravityMod out of range.  [-40, 40]");
      mGravityMod = mGravityMod < -40.0f ? -40.0f : 40.0f;
   }
   static const char* coordString[] = { "x", "y", "z" };
   F32* p = mAppliedForce;
   for (U32 i = 0; i < 3; i++) {
      if (p[i] < -40000.0f || p[i] > 40000.0f) {
         Con::errorf("PhysicalZone: applied force: %s out of range.  [-40000, 40000]", coordString[i]);
         p[i] = p[i] < -40000.0f ? -40000.0f : 40000.0f;
      }
   }

   Polyhedron temp = mPolyhedron;
   setPolyhedron(temp);

   switch (force_type)
   {
   case SPHERICAL:
      force_mag = mAppliedForce.magnitudeSafe();
      break;
   case CYLINDRICAL:
      {
         Point3F force_vec = mAppliedForce;
         force_vec.z = 0.0;
         force_mag = force_vec.magnitudeSafe();
      }
      break;
   }
   addToScene();

   return true;
}


void PhysicalZone::onRemove()
{
   mConvexList->nukeList();

   removeFromScene();
   Parent::onRemove();
}

void PhysicalZone::inspectPostApply()
{
   Parent::inspectPostApply();

   setPolyhedron(mPolyhedron);
   setMaskBits(PolyhedronMask | MoveMask | SettingsMask | FadeMask);
}

//------------------------------------------------------------------------------
void PhysicalZone::setTransform(const MatrixF & mat)
{
   Parent::setTransform(mat);

      MatrixF base(true);
      base.scale(Point3F(1.0/mObjScale.x,
                         1.0/mObjScale.y,
                         1.0/mObjScale.z));
      base.mul(mWorldToObj);
      mClippedList.setBaseTransform(base);

   if (isServerObject())
      setMaskBits(MoveMask);
}


void PhysicalZone::prepRenderImage( SceneRenderState *state )
{
   // only render if selected or render flag is set
   if ( !smRenderPZones && !isSelected() )
      return;

   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind( this, &PhysicalZone::renderObject );
   ri->type = RenderPassManager::RIT_Editor;
   ri->defaultKey = 0;
   ri->defaultKey2 = 0;
   state->getRenderPass()->addInst( ri );
}


void PhysicalZone::renderObject(ObjectRenderInst *ri,
   SceneRenderState *state,
   BaseMatInstance *overrideMat)
{
   if (overrideMat)
      return;
 
   GFXStateBlockDesc desc;
   desc.setZReadWrite(true, false);
   desc.setBlend(true);
   desc.setCullMode(GFXCullNone);
 
   GFXTransformSaver saver;
 
   GFXDrawUtil *drawer = GFX->getDrawUtil();
 
   Point3F start = getBoxCenter();
   Box3F obb = mObjBox; //object bounding box 

   F32 baseForce = 10000; //roughly the ammount of force needed to push a player back as it walks into a zone. (used for visual scaling)

   Point3F forceDir = getForce(&start);
   F32 forceLen = forceDir.len()/ baseForce;
   forceDir.normalizeSafe();
   ColorI guideCol = LinearColorF(mFabs(forceDir.x), mFabs(forceDir.y), mFabs(forceDir.z), 0.125).toColorI();

   if (force_type == VECTOR)
   {
	   Point3F endPos = start + (forceDir * mMax(forceLen,0.75f));
	   drawer->drawArrow(desc, start, endPos, guideCol, 0.05f);
   }

   MatrixF mat = getRenderTransform();
   mat.scale(getScale());
 
   GFX->multWorld(mat);
   start = obb.getCenter();
 
   if (force_type == VECTOR)
   {
      drawer->drawPolyhedron(desc, mPolyhedron, ColorI(0, 255, 0, 45));
   }
   else if (force_type == SPHERICAL)
   {
	   F32 rad = obb.getBoundingSphere().radius/ 2;
      drawer->drawSphere(desc, rad, start, ColorI(0, 255, 0, 45));

	  rad = (rad + (mAppliedForce.most() / baseForce))/2;
	  desc.setFillModeWireframe();
	  drawer->drawSphere(desc, rad, start, ColorI(0, 0, 255, 255));
   }
   else
   {
      Point3F bottomPos = start;
      bottomPos.z -= obb.len_z() / 2;
 
      Point3F topPos = start;
      topPos.z += obb.len_z() / 2;
	  F32 rad = obb.len_x() / 2;
      drawer->drawCylinder(desc, bottomPos, topPos, rad, ColorI(0, 255, 0, 45));

	  Point3F force_vec = mAppliedForce; //raw relative-applied force here as oposed to derived
	  F32 hieght = (force_vec.z / baseForce);

	  if (force_vec.z<0)
		  bottomPos.z = (bottomPos.z + hieght)/2;
	  else
		  topPos.z = (topPos.z + hieght) / 2;

	  if (force_vec.x > force_vec.y)
		  rad = (rad + (force_vec.x / baseForce)) / 2;
	  else
		  rad = (rad + (force_vec.y / baseForce)) / 2;


	  desc.setFillModeWireframe();
	  drawer->drawCylinder(desc, bottomPos, topPos, rad, guideCol);
   }
 
   desc.setFillModeWireframe();
   drawer->drawPolyhedron(desc, mPolyhedron, ColorI::BLACK);
}

//--------------------------------------------------------------------------
U32 PhysicalZone::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 i;
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(mask & PolyhedronMask)) 
   {
      // Write the polyhedron
      stream->write(mPolyhedron.mPointList.size());
      for (i = 0; i < mPolyhedron.mPointList.size(); i++)
         mathWrite(*stream, mPolyhedron.mPointList[i]);

      stream->write(mPolyhedron.mPlaneList.size());
      for (i = 0; i < mPolyhedron.mPlaneList.size(); i++)
         mathWrite(*stream, mPolyhedron.mPlaneList[i]);

      stream->write(mPolyhedron.mEdgeList.size());
      for (i = 0; i < mPolyhedron.mEdgeList.size(); i++) {
         const Polyhedron::Edge& rEdge = mPolyhedron.mEdgeList[i];

         stream->write(rEdge.face[0]);
         stream->write(rEdge.face[1]);
         stream->write(rEdge.vertex[0]);
         stream->write(rEdge.vertex[1]);
      }
   }

   if (stream->writeFlag(mask & MoveMask))
   {
      stream->writeAffineTransform(mObjToWorld);
      mathWrite(*stream, mObjScale);
   }

   if (stream->writeFlag(mask & SettingsMask))
   {
      stream->write(mVelocityMod);
      stream->write(mGravityMod);
      mathWrite(*stream, mAppliedForce);
      stream->writeInt(force_type, FORCE_TYPE_BITS);
      stream->writeFlag(orient_force);
   }

   if (stream->writeFlag(mask & FadeMask))
   {
      U8 fade_byte = (U8)(fade_amt*255.0f);
      stream->write(fade_byte);
   }

   stream->writeFlag(mActive);

   return retMask;
}

void PhysicalZone::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   bool new_ph = false;
   if (stream->readFlag()) // PolyhedronMask
   {
      U32 i, size;
      Polyhedron tempPH;

      // Read the polyhedron
      stream->read(&size);
      tempPH.mPointList.setSize(size);
      for (i = 0; i < tempPH.mPointList.size(); i++)
         mathRead(*stream, &tempPH.mPointList[i]);

      stream->read(&size);
      tempPH.mPlaneList.setSize(size);
      for (i = 0; i < tempPH.mPlaneList.size(); i++)
         mathRead(*stream, &tempPH.mPlaneList[i]);

      stream->read(&size);
      tempPH.mEdgeList.setSize(size);
      for (i = 0; i < tempPH.mEdgeList.size(); i++) {
         Polyhedron::Edge& rEdge = tempPH.mEdgeList[i];

         stream->read(&rEdge.face[0]);
         stream->read(&rEdge.face[1]);
         stream->read(&rEdge.vertex[0]);
         stream->read(&rEdge.vertex[1]);
      }

      setPolyhedron(tempPH);
      new_ph = true;
   }

   if (stream->readFlag()) // MoveMask
   {
      MatrixF temp;
      stream->readAffineTransform(&temp);

      Point3F tempScale;
      mathRead(*stream, &tempScale);

      //if (!new_ph)
      //{
      //  Polyhedron rPolyhedron = mPolyhedron;
      //  setPolyhedron(rPolyhedron);
      //}
      setScale(tempScale);
      setTransform(temp);
   }

   if (stream->readFlag()) //SettingsMask
   {
      stream->read(&mVelocityMod);
      stream->read(&mGravityMod);
      mathRead(*stream, &mAppliedForce);
      force_type = stream->readInt(FORCE_TYPE_BITS); // AFX
      orient_force = stream->readFlag(); // AFX
   }

   if (stream->readFlag()) //FadeMask
   {
      U8 fade_byte;
      stream->read(&fade_byte);
      fade_amt = ((F32)fade_byte)/255.0f;
   }
   else
      fade_amt = 1.0f;

   mActive = stream->readFlag();
}


//--------------------------------------------------------------------------
void PhysicalZone::setPolyhedron(const Polyhedron& rPolyhedron)
{
   mPolyhedron = rPolyhedron;

   if (mPolyhedron.mPointList.size() != 0) {
      mObjBox.minExtents.set(1e10, 1e10, 1e10);
      mObjBox.maxExtents.set(-1e10, -1e10, -1e10);
      for (U32 i = 0; i < mPolyhedron.mPointList.size(); i++) {
         mObjBox.minExtents.setMin(mPolyhedron.mPointList[i]);
         mObjBox.maxExtents.setMax(mPolyhedron.mPointList[i]);
      }
   } else {
      mObjBox.minExtents.set(-0.5, -0.5, -0.5);
      mObjBox.maxExtents.set( 0.5,  0.5,  0.5);
   }

   MatrixF xform = getTransform();
   setTransform(xform);

   mClippedList.clear();
   mClippedList.mPlaneList = mPolyhedron.mPlaneList;

   MatrixF base(true);
   base.scale(Point3F(1.0/mObjScale.x,
                      1.0/mObjScale.y,
                      1.0/mObjScale.z));
   base.mul(mWorldToObj);

   mClippedList.setBaseTransform(base);
}


//--------------------------------------------------------------------------
void PhysicalZone::buildConvex(const Box3F& box, Convex* convex)
{
   // These should really come out of a pool
   mConvexList->collectGarbage();

   Box3F realBox = box;
   mWorldToObj.mul(realBox);
   realBox.minExtents.convolveInverse(mObjScale);
   realBox.maxExtents.convolveInverse(mObjScale);

   if (realBox.isOverlapped(getObjBox()) == false)
      return;

   // Just return a box convex for the entire shape...
   Convex* cc = 0;
   CollisionWorkingList& wl = convex->getWorkingList();
   for (CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext) {
      if (itr->mConvex->getType() == BoxConvexType &&
          itr->mConvex->getObject() == this) {
         cc = itr->mConvex;
         break;
      }
   }
   if (cc)
      return;

   // Create a new convex.
   BoxConvex* cp = new BoxConvex;
   mConvexList->registerObject(cp);
   convex->addToWorkingList(cp);
   cp->init(this);

   mObjBox.getCenter(&cp->mCenter);
   cp->mSize.x = mObjBox.len_x() / 2.0f;
   cp->mSize.y = mObjBox.len_y() / 2.0f;
   cp->mSize.z = mObjBox.len_z() / 2.0f;
}

bool PhysicalZone::testObject(SceneObject* enter)
{
   // TODO: This doesn't look like it's testing against the polyhedron at
   // all.  And whats the point of building a convex if no collision methods
   // are implemented?

   if (mPolyhedron.mPointList.size() == 0)
      return false;

   mClippedList.clear();

   SphereF sphere;
   sphere.center = (mWorldBox.minExtents + mWorldBox.maxExtents) * 0.5;
   VectorF bv = mWorldBox.maxExtents - sphere.center;
   sphere.radius = bv.len();

   enter->buildPolyList(PLC_Collision, &mClippedList, mWorldBox, sphere);
   return mClippedList.isEmpty() == false;
}

bool PhysicalZone::testBox( const Box3F &box ) const
{
   return mWorldBox.isOverlapped( box );
}

void PhysicalZone::activate()
{
   AssertFatal(isServerObject(), "Client objects not allowed in ForceFieldInstance::open()");

   if (mActive != true)
      setMaskBits(ActiveMask);
   mActive = true;
}

void PhysicalZone::deactivate()
{
   AssertFatal(isServerObject(), "Client objects not allowed in ForceFieldInstance::close()");

   if (mActive != false)
      setMaskBits(ActiveMask);
   mActive = false;
}

void PhysicalZone::onStaticModified(const char* slotName, const char*newValue)
{
   if (dStricmp(slotName, "appliedForce") == 0 || dStricmp(slotName, "forceType") == 0)
   {
      switch (force_type)
      {
      case SPHERICAL:
         force_mag = mAppliedForce.magnitudeSafe();
         break;
      case CYLINDRICAL:
         {
            Point3F force_vec = mAppliedForce;
            force_vec.z = 0.0;
            force_mag = force_vec.magnitudeSafe();
         }
         break;
      }
   }
}

const Point3F& PhysicalZone::getForce(const Point3F* center) const 
{ 
   static Point3F force_vec;

   if (force_type == VECTOR)
   {
      if (orient_force)
      {
         getTransform().mulV(mAppliedForce, &force_vec);
         force_vec *= fade_amt;
         return force_vec; 
      }
      force_vec = mAppliedForce;
      force_vec *= fade_amt;
      return force_vec;
   }

   if (!center)
   {
      force_vec.zero();
      return force_vec; 
   }

   if (force_type == SPHERICAL)
   {
      force_vec = *center - getPosition();
      force_vec.normalizeSafe();
      force_vec *= force_mag*fade_amt;
      return force_vec;
   }

   if (orient_force)
   {
      force_vec = *center - getPosition();
      getWorldTransform().mulV(force_vec);
      force_vec.z = 0.0f;
      force_vec.normalizeSafe();
      force_vec *= force_mag;
      force_vec.z = mAppliedForce.z;
      getTransform().mulV(force_vec);
      force_vec *= fade_amt;
      return force_vec;
   }

   force_vec = *center - getPosition();
   force_vec.z = 0.0f;
   force_vec.normalizeSafe();
   force_vec *= force_mag;
   force_vec *= fade_amt;
   return force_vec;
}

bool PhysicalZone::isExcludedObject(SceneObject* obj) const
{
   for (S32 i = 0; i < excluded_objects.size(); i++)
      if (excluded_objects[i] == obj)
         return true;

   return false;
}

void PhysicalZone::registerExcludedObject(SceneObject* obj)
{
   if (isExcludedObject(obj))
      return;

   excluded_objects.push_back(obj);
   setMaskBits(FadeMask);
}

void PhysicalZone::unregisterExcludedObject(SceneObject* obj)
{
   for (S32 i = 0; i < excluded_objects.size(); i++)
      if (excluded_objects[i] == obj)
      {
         excluded_objects.erase(i);
         setMaskBits(FadeMask);
         return;
      }
}

