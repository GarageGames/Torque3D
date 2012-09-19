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

#include "platform/platform.h"
#include "T3D/trigger.h"

#include "scene/sceneRenderState.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "collision/boxConvex.h"

#include "core/stream/bitStream.h"
#include "math/mathIO.h"
#include "gfx/gfxTransformSaver.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/gfxDrawUtil.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsBody.h"
#include "T3D/physics/physicsCollision.h"


bool Trigger::smRenderTriggers = false;

//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------

IMPLEMENT_CO_DATABLOCK_V1(TriggerData);

ConsoleDocClass( TriggerData,
   "@brief Defines shared properties for Trigger objects.\n\n"

   "The primary focus of the TriggerData datablock is the callbacks it provides when an object is "
   "within or leaves the Trigger bounds.\n"

   "@see Trigger.\n"
   "@ingroup gameObjects\n"
   "@ingroup Datablocks\n"
);

IMPLEMENT_CALLBACK( TriggerData, onEnterTrigger, void, ( Trigger* trigger, GameBase* obj ), ( trigger, obj ),
   "@brief Called when an object enters the volume of the Trigger instance using this TriggerData.\n\n"

   "@param trigger the Trigger instance whose volume the object entered\n"
   "@param obj the object that entered the volume of the Trigger instance\n" );

IMPLEMENT_CALLBACK( TriggerData, onTickTrigger, void, ( Trigger* trigger ), ( trigger ),
   "@brief Called every tickPeriodMS number of milliseconds (as specified in the TriggerData) whenever "
   "one or more objects are inside the volume of the trigger.\n\n"

   "The Trigger has methods to retrieve the objects that are within the Trigger's bounds if you "
   "want to do something with them in this callback.\n"

   "@param trigger the Trigger instance whose volume the object is inside\n"
   
   "@see tickPeriodMS\n"
   "@see Trigger::getNumObjects()\n"
   "@see Trigger::getObject()\n");

IMPLEMENT_CALLBACK( TriggerData, onLeaveTrigger, void, ( Trigger* trigger, GameBase* obj ), ( trigger, obj ),
   "@brief Called when an object leaves the volume of the Trigger instance using this TriggerData.\n\n"

   "@param trigger the Trigger instance whose volume the object left\n"
   "@param obj the object that left the volume of the Trigger instance\n" );

TriggerData::TriggerData()
{
   tickPeriodMS = 100;
   isClientSide = false;
}

bool TriggerData::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

void TriggerData::initPersistFields()
{
   addGroup("Callbacks");

      addField( "tickPeriodMS",  TypeS32,    Offset( tickPeriodMS, TriggerData ),
         "@brief Time in milliseconds between calls to onTickTrigger() while at least one object is within a Trigger's bounds.\n\n"
         "@see onTickTrigger()\n");
      addField( "clientSide",    TypeBool,   Offset( isClientSide, TriggerData ),
         "Forces Trigger callbacks to only be called on clients.");

   endGroup("Callbacks");

   Parent::initPersistFields();
}


//--------------------------------------------------------------------------
void TriggerData::packData(BitStream* stream)
{
   Parent::packData(stream);
   stream->write(tickPeriodMS);
   stream->write(isClientSide);
}

void TriggerData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
   stream->read(&tickPeriodMS);
   stream->read(&isClientSide);
}


//--------------------------------------------------------------------------

IMPLEMENT_CO_NETOBJECT_V1(Trigger);

ConsoleDocClass( Trigger,
   "@brief A Trigger is a volume of space that initiates script callbacks "
   "when objects pass through the Trigger.\n\n"

   "TriggerData provides the callbacks for the Trigger when an object enters, stays inside "
   "or leaves the Trigger's volume.\n\n"

   "@see TriggerData\n"
   "@ingroup gameObjects\n"
);

IMPLEMENT_CALLBACK( Trigger, onAdd, void, ( U32 objectId ), ( objectId ),
   "@brief Called when the Trigger is being created.\n\n"
   "@param objectId the object id of the Trigger being created\n" );

IMPLEMENT_CALLBACK( Trigger, onRemove, void, ( U32 objectId ), ( objectId ),
   "@brief Called just before the Trigger is deleted.\n\n"
   "@param objectId the object id of the Trigger being deleted\n" );

Trigger::Trigger()
{
   // Don't ghost by default.
   mNetFlags.set(Ghostable | ScopeAlways);

   mTypeMask |= TriggerObjectType;

   mObjScale.set(1, 1, 1);
   mObjToWorld.identity();
   mWorldToObj.identity();

   mDataBlock = NULL;

   mLastThink = 0;
   mCurrTick  = 0;

   mConvexList = new Convex;

   mPhysicsRep = NULL;
}

Trigger::~Trigger()
{
   delete mConvexList;
   mConvexList = NULL;
   SAFE_DELETE( mPhysicsRep );
}

bool Trigger::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
   // Collide against bounding box
   F32 st,et,fst = 0,fet = 1;
   F32 *bmin = &mObjBox.minExtents.x;
   F32 *bmax = &mObjBox.maxExtents.x;
   F32 const *si = &start.x;
   F32 const *ei = &end.x;

   for (int i = 0; i < 3; i++)
   {
      if (*si < *ei)
      {
         if (*si > *bmax || *ei < *bmin)
            return false;
         F32 di = *ei - *si;
         st = (*si < *bmin)? (*bmin - *si) / di: 0;
         et = (*ei > *bmax)? (*bmax - *si) / di: 1;
      }
      else
      {
         if (*ei > *bmax || *si < *bmin)
            return false;
         F32 di = *ei - *si;
         st = (*si > *bmax)? (*bmax - *si) / di: 0;
         et = (*ei < *bmin)? (*bmin - *si) / di: 1;
      }
      if (st > fst) fst = st;
      if (et < fet) fet = et;
      if (fet < fst)
         return false;
      bmin++; bmax++;
      si++; ei++;
   }

   info->normal = start - end;
   info->normal.normalizeSafe();
   getTransform().mulV( info->normal );

   info->t = fst;
   info->object = this;
   info->point.interpolate(start,end,fst);
   info->material = 0;
   return true;
}


//--------------------------------------------------------------------------
/* Console polyhedron data type exporter
   The polyhedron type is really a quadrilateral and consists of a corner
   point follow by three vectors representing the edges extending from the
   corner.
*/
DECLARE_STRUCT( Polyhedron );
IMPLEMENT_STRUCT( Polyhedron, Polyhedron,,
   "" )
END_IMPLEMENT_STRUCT;
ConsoleType( floatList, TypeTriggerPolyhedron, Polyhedron )


ConsoleGetType( TypeTriggerPolyhedron )
{
   U32 i;
   Polyhedron* pPoly = reinterpret_cast<Polyhedron*>(dptr);

   // First point is corner, need to find the three vectors...`
   Point3F origin = pPoly->pointList[0];
   U32 currVec = 0;
   Point3F vecs[3];
   for (i = 0; i < pPoly->edgeList.size(); i++) {
      const U32 *vertex = pPoly->edgeList[i].vertex;
      if (vertex[0] == 0)
         vecs[currVec++] = pPoly->pointList[vertex[1]] - origin;
      else
         if (vertex[1] == 0)
            vecs[currVec++] = pPoly->pointList[vertex[0]] - origin;
   }
   AssertFatal(currVec == 3, "Internal error: Bad trigger polyhedron");

   // Build output string.
   char* retBuf = Con::getReturnBuffer(1024);
   dSprintf(retBuf, 1023, "%7.7f %7.7f %7.7f %7.7f %7.7f %7.7f %7.7f %7.7f %7.7f %7.7f %7.7f %7.7f",
            origin.x, origin.y, origin.z,
            vecs[0].x, vecs[0].y, vecs[0].z,
            vecs[2].x, vecs[2].y, vecs[2].z,
            vecs[1].x, vecs[1].y, vecs[1].z);

   return retBuf;
}

/* Console polyhedron data type loader
   The polyhedron type is really a quadrilateral and consists of an corner
   point follow by three vectors representing the edges extending from the
   corner.
*/
ConsoleSetType( TypeTriggerPolyhedron )
{
   if (argc != 1) {
      Con::printf("(TypeTriggerPolyhedron) multiple args not supported for polyhedra");
      return;
   }

   Point3F origin;
   Point3F vecs[3];

   U32 numArgs = dSscanf(argv[0], "%g %g %g %g %g %g %g %g %g %g %g %g",
                         &origin.x, &origin.y, &origin.z,
                         &vecs[0].x, &vecs[0].y, &vecs[0].z,
                         &vecs[1].x, &vecs[1].y, &vecs[1].z,
                         &vecs[2].x, &vecs[2].y, &vecs[2].z);
   if (numArgs != 12) {
      Con::printf("Bad polyhedron!");
      return;
   }

   Polyhedron* pPoly = reinterpret_cast<Polyhedron*>(dptr);

   // This setup goes against conventions for Polyhedrons in that it a) sets up
   // edges with CCW instead of CW order for face[0] and that it b) lets plane
   // normals face outwards rather than inwards.

   pPoly->pointList.setSize(8);
   pPoly->pointList[0] = origin;
   pPoly->pointList[1] = origin + vecs[0];
   pPoly->pointList[2] = origin + vecs[1];
   pPoly->pointList[3] = origin + vecs[2];
   pPoly->pointList[4] = origin + vecs[0] + vecs[1];
   pPoly->pointList[5] = origin + vecs[0] + vecs[2];
   pPoly->pointList[6] = origin + vecs[1] + vecs[2];
   pPoly->pointList[7] = origin + vecs[0] + vecs[1] + vecs[2];

   Point3F normal;
   pPoly->planeList.setSize(6);

   mCross(vecs[2], vecs[0], &normal);
   pPoly->planeList[0].set(origin, normal);
   mCross(vecs[0], vecs[1], &normal);
   pPoly->planeList[1].set(origin, normal);
   mCross(vecs[1], vecs[2], &normal);
   pPoly->planeList[2].set(origin, normal);
   mCross(vecs[1], vecs[0], &normal);
   pPoly->planeList[3].set(pPoly->pointList[7], normal);
   mCross(vecs[2], vecs[1], &normal);
   pPoly->planeList[4].set(pPoly->pointList[7], normal);
   mCross(vecs[0], vecs[2], &normal);
   pPoly->planeList[5].set(pPoly->pointList[7], normal);

   pPoly->edgeList.setSize(12);
   pPoly->edgeList[0].vertex[0]  = 0; pPoly->edgeList[0].vertex[1]  = 1; pPoly->edgeList[0].face[0]  = 0; pPoly->edgeList[0].face[1]  = 1;
   pPoly->edgeList[1].vertex[0]  = 1; pPoly->edgeList[1].vertex[1]  = 5; pPoly->edgeList[1].face[0]  = 0; pPoly->edgeList[1].face[1]  = 4;
   pPoly->edgeList[2].vertex[0]  = 5; pPoly->edgeList[2].vertex[1]  = 3; pPoly->edgeList[2].face[0]  = 0; pPoly->edgeList[2].face[1]  = 3;
   pPoly->edgeList[3].vertex[0]  = 3; pPoly->edgeList[3].vertex[1]  = 0; pPoly->edgeList[3].face[0]  = 0; pPoly->edgeList[3].face[1]  = 2;
   pPoly->edgeList[4].vertex[0]  = 3; pPoly->edgeList[4].vertex[1]  = 6; pPoly->edgeList[4].face[0]  = 3; pPoly->edgeList[4].face[1]  = 2;
   pPoly->edgeList[5].vertex[0]  = 6; pPoly->edgeList[5].vertex[1]  = 2; pPoly->edgeList[5].face[0]  = 2; pPoly->edgeList[5].face[1]  = 5;
   pPoly->edgeList[6].vertex[0]  = 2; pPoly->edgeList[6].vertex[1]  = 0; pPoly->edgeList[6].face[0]  = 2; pPoly->edgeList[6].face[1]  = 1;
   pPoly->edgeList[7].vertex[0]  = 1; pPoly->edgeList[7].vertex[1]  = 4; pPoly->edgeList[7].face[0]  = 4; pPoly->edgeList[7].face[1]  = 1;
   pPoly->edgeList[8].vertex[0]  = 4; pPoly->edgeList[8].vertex[1]  = 2; pPoly->edgeList[8].face[0]  = 1; pPoly->edgeList[8].face[1]  = 5;
   pPoly->edgeList[9].vertex[0]  = 4; pPoly->edgeList[9].vertex[1]  = 7; pPoly->edgeList[9].face[0]  = 4; pPoly->edgeList[9].face[1]  = 5;
   pPoly->edgeList[10].vertex[0] = 5; pPoly->edgeList[10].vertex[1] = 7; pPoly->edgeList[10].face[0] = 3; pPoly->edgeList[10].face[1] = 4;
   pPoly->edgeList[11].vertex[0] = 7; pPoly->edgeList[11].vertex[1] = 6; pPoly->edgeList[11].face[0] = 3; pPoly->edgeList[11].face[1] = 5;
}


//-----------------------------------------------------------------------------
void Trigger::consoleInit()
{
   Con::addVariable( "$Trigger::renderTriggers", TypeBool, &smRenderTriggers,
      "@brief Forces all Trigger's to render.\n\n"
      "Used by the Tools and debug render modes.\n"
      "@ingroup gameObjects" );
}

void Trigger::initPersistFields()
{
   addField("polyhedron", TypeTriggerPolyhedron, Offset(mTriggerPolyhedron, Trigger),
      "@brief Defines a non-rectangular area for the trigger.\n\n"
      "Rather than the standard rectangular bounds, this optional parameter defines a quadrilateral "
      "trigger area.  The quadrilateral is defined as a corner point followed by three vectors "
      "representing the edges extending from the corner.\n");

   addProtectedField("enterCommand", TypeCommand, Offset(mEnterCommand, Trigger), &setEnterCmd, &defaultProtectedGetFn,
      "The command to execute when an object enters this trigger. Object id stored in %%obj. Maximum 1023 characters." );
   addProtectedField("leaveCommand", TypeCommand, Offset(mLeaveCommand, Trigger), &setLeaveCmd, &defaultProtectedGetFn,
      "The command to execute when an object leaves this trigger. Object id stored in %%obj. Maximum 1023 characters." );
   addProtectedField("tickCommand", TypeCommand, Offset(mTickCommand, Trigger), &setTickCmd, &defaultProtectedGetFn,
      "The command to execute while an object is inside this trigger. Maximum 1023 characters." );

   Parent::initPersistFields();
}

bool Trigger::setEnterCmd( void *object, const char *index, const char *data )
{
   static_cast<Trigger*>(object)->setMaskBits(EnterCmdMask);
   return true; // to update the actual field
}

bool Trigger::setLeaveCmd(void *object, const char *index, const char *data)
{
   static_cast<Trigger*>(object)->setMaskBits(LeaveCmdMask);
   return true; // to update the actual field
}

bool Trigger::setTickCmd(void *object, const char *index, const char *data)
{
   static_cast<Trigger*>(object)->setMaskBits(TickCmdMask);
   return true; // to update the actual field
}

//--------------------------------------------------------------------------

bool Trigger::onAdd()
{
   if(!Parent::onAdd())
      return false;

   onAdd_callback( getId() );

   Polyhedron temp = mTriggerPolyhedron;
   setTriggerPolyhedron(temp);

   addToScene();

   if (isServerObject())
      scriptOnAdd();
      
   return true;
}

void Trigger::onRemove()
{
   onRemove_callback( getId() );

   mConvexList->nukeList();

   removeFromScene();
   Parent::onRemove();
}

bool Trigger::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   mDataBlock = dynamic_cast<TriggerData*>( dptr );
   if ( !mDataBlock || !Parent::onNewDataBlock( dptr, reload ) )
      return false;

   scriptOnNewDataBlock();
   return true;
}

void Trigger::onDeleteNotify( SimObject *obj )
{
   GameBase* pScene = dynamic_cast<GameBase*>( obj );

   if  ( pScene != NULL && mDataBlock != NULL )
   {
      for ( U32 i = 0; i < mObjects.size(); i++ )
      {
         if ( pScene == mObjects[i] )
         {
            mObjects.erase(i);
            if (mDataBlock)
               mDataBlock->onLeaveTrigger_callback( this, pScene );
            break;
         }
      }
   }

   Parent::onDeleteNotify( obj );
}

void Trigger::inspectPostApply()
{
   setTriggerPolyhedron(mTriggerPolyhedron);
   setMaskBits(PolyMask);
   Parent::inspectPostApply();
}

//--------------------------------------------------------------------------

void Trigger::buildConvex(const Box3F& box, Convex* convex)
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


//------------------------------------------------------------------------------

void Trigger::setTransform(const MatrixF & mat)
{
   Parent::setTransform(mat);

   if ( mPhysicsRep )
      mPhysicsRep->setTransform( mat );

   if (isServerObject()) {
      MatrixF base(true);
      base.scale(Point3F(1.0/mObjScale.x,
                         1.0/mObjScale.y,
                         1.0/mObjScale.z));
      base.mul(mWorldToObj);
      mClippedList.setBaseTransform(base);

      setMaskBits(TransformMask | ScaleMask);
   }
}

void Trigger::prepRenderImage( SceneRenderState *state )
{
   // only render if selected or render flag is set
   if ( !smRenderTriggers && !isSelected() )
      return;

   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->renderDelegate.bind( this, &Trigger::renderObject );
   ri->type = RenderPassManager::RIT_Editor;      
   ri->translucentSort = true;
   ri->defaultKey = 1;
   state->getRenderPass()->addInst( ri );
}

void Trigger::renderObject( ObjectRenderInst *ri,
                            SceneRenderState *state,
                            BaseMatInstance *overrideMat )
{
   if(overrideMat)
      return;

   GFXStateBlockDesc desc;
   desc.setZReadWrite( true, false );
   desc.setBlend( true );

   // Trigger polyhedrons are set up with outward facing normals and CCW ordering
   // so can't enable backface culling.
   desc.setCullMode( GFXCullNone );

   GFXTransformSaver saver;

   MatrixF mat = getRenderTransform();
   mat.scale( getScale() );

   GFX->multWorld( mat );

   GFXDrawUtil *drawer = GFX->getDrawUtil();
   
   drawer->drawPolyhedron( desc, mTriggerPolyhedron, ColorI( 255, 192, 0, 45 ) );

   // Render wireframe.

   desc.setFillModeWireframe();
   drawer->drawPolyhedron( desc, mTriggerPolyhedron, ColorI::BLACK );
}

void Trigger::setTriggerPolyhedron(const Polyhedron& rPolyhedron)
{
   mTriggerPolyhedron = rPolyhedron;

   if (mTriggerPolyhedron.pointList.size() != 0) {
      mObjBox.minExtents.set(1e10, 1e10, 1e10);
      mObjBox.maxExtents.set(-1e10, -1e10, -1e10);
      for (U32 i = 0; i < mTriggerPolyhedron.pointList.size(); i++) {
         mObjBox.minExtents.setMin(mTriggerPolyhedron.pointList[i]);
         mObjBox.maxExtents.setMax(mTriggerPolyhedron.pointList[i]);
      }
   } else {
      mObjBox.minExtents.set(-0.5, -0.5, -0.5);
      mObjBox.maxExtents.set( 0.5,  0.5,  0.5);
   }

   MatrixF xform = getTransform();
   setTransform(xform);

   mClippedList.clear();
   mClippedList.mPlaneList = mTriggerPolyhedron.planeList;
//   for (U32 i = 0; i < mClippedList.mPlaneList.size(); i++)
//      mClippedList.mPlaneList[i].neg();

   MatrixF base(true);
   base.scale(Point3F(1.0/mObjScale.x,
                      1.0/mObjScale.y,
                      1.0/mObjScale.z));
   base.mul(mWorldToObj);

   mClippedList.setBaseTransform(base);

   SAFE_DELETE( mPhysicsRep );

   if ( PHYSICSMGR )
   {
      PhysicsCollision *colShape = PHYSICSMGR->createCollision();

      MatrixF colMat( true );      
      colMat.displace( Point3F( 0, 0, mObjBox.getExtents().z * 0.5f * mObjScale.z ) );
      
      colShape->addBox( mObjBox.getExtents() * 0.5f * mObjScale, colMat );
      //MatrixF colMat( true );
      //colMat.scale( mObjScale );
      //colShape->addConvex( mTriggerPolyhedron.pointList.address(), mTriggerPolyhedron.pointList.size(), colMat );

      PhysicsWorld *world = PHYSICSMGR->getWorld( isServerObject() ? "server" : "client" );
      mPhysicsRep = PHYSICSMGR->createBody();
      mPhysicsRep->init( colShape, 0, PhysicsBody::BF_TRIGGER | PhysicsBody::BF_KINEMATIC, this, world );
      mPhysicsRep->setTransform( getTransform() );
   }
}


//--------------------------------------------------------------------------

bool Trigger::testObject(GameBase* enter)
{
   if (mTriggerPolyhedron.pointList.size() == 0)
      return false;

   mClippedList.clear();

   SphereF sphere;
   sphere.center = (mWorldBox.minExtents + mWorldBox.maxExtents) * 0.5;
   VectorF bv = mWorldBox.maxExtents - sphere.center;
   sphere.radius = bv.len();

   enter->buildPolyList(PLC_Collision, &mClippedList, mWorldBox, sphere);
   return mClippedList.isEmpty() == false;
}


void Trigger::potentialEnterObject(GameBase* enter)
{
   if( (!mDataBlock || mDataBlock->isClientSide) && isServerObject() )
      return;
   if( (mDataBlock && !mDataBlock->isClientSide) && isGhost() )
      return;

   for (U32 i = 0; i < mObjects.size(); i++) {
      if (mObjects[i] == enter)
         return;
   }

   if (testObject(enter) == true) {
      mObjects.push_back(enter);
      deleteNotify(enter);

      if(!mEnterCommand.isEmpty())
      {
         String command = String("%obj = ") + enter->getIdString() + ";" + mEnterCommand;
         Con::evaluate(command.c_str());
      }

      if( mDataBlock )
         mDataBlock->onEnterTrigger_callback( this, enter );
   }
}


void Trigger::processTick(const Move* move)
{
   Parent::processTick(move);

   if (!mDataBlock)
      return;
   if (mDataBlock->isClientSide && isServerObject())
      return;
   if (!mDataBlock->isClientSide && isClientObject())
      return;

   //
   if (mObjects.size() == 0)
      return;

   if (mLastThink + mDataBlock->tickPeriodMS < mCurrTick)
   {
      mCurrTick  = 0;
      mLastThink = 0;

      for (S32 i = S32(mObjects.size() - 1); i >= 0; i--)
      {
         if (testObject(mObjects[i]) == false)
         {
            GameBase* remove = mObjects[i];
            mObjects.erase(i);
            clearNotify(remove);
            
            if (!mLeaveCommand.isEmpty())
            {
               String command = String("%obj = ") + remove->getIdString() + ";" + mLeaveCommand;
               Con::evaluate(command.c_str());
            }

            mDataBlock->onLeaveTrigger_callback( this, remove );
         }
      }

      if (!mTickCommand.isEmpty())
         Con::evaluate(mTickCommand.c_str());

      if (mObjects.size() != 0)
         mDataBlock->onTickTrigger_callback( this );
   }
   else
   {
      mCurrTick += TickMs;
   }
}

//--------------------------------------------------------------------------

U32 Trigger::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 i;
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if( stream->writeFlag( mask & TransformMask ) )
   {
      stream->writeAffineTransform(mObjToWorld);
   }

   // Write the polyhedron
   if( stream->writeFlag( mask & PolyMask ) )
   {
      stream->write(mTriggerPolyhedron.pointList.size());
      for (i = 0; i < mTriggerPolyhedron.pointList.size(); i++)
         mathWrite(*stream, mTriggerPolyhedron.pointList[i]);

      stream->write(mTriggerPolyhedron.planeList.size());
      for (i = 0; i < mTriggerPolyhedron.planeList.size(); i++)
         mathWrite(*stream, mTriggerPolyhedron.planeList[i]);

      stream->write(mTriggerPolyhedron.edgeList.size());
      for (i = 0; i < mTriggerPolyhedron.edgeList.size(); i++) {
         const Polyhedron::Edge& rEdge = mTriggerPolyhedron.edgeList[i];

         stream->write(rEdge.face[0]);
         stream->write(rEdge.face[1]);
         stream->write(rEdge.vertex[0]);
         stream->write(rEdge.vertex[1]);
      }
   }

   if( stream->writeFlag( mask & EnterCmdMask ) )
      stream->writeLongString(CMD_SIZE-1, mEnterCommand.c_str());
   if( stream->writeFlag( mask & LeaveCmdMask ) )
      stream->writeLongString(CMD_SIZE-1, mLeaveCommand.c_str());
   if( stream->writeFlag( mask & TickCmdMask ) )
      stream->writeLongString(CMD_SIZE-1, mTickCommand.c_str());

   return retMask;
}

void Trigger::unpackUpdate(NetConnection* con, BitStream* stream)
{
   Parent::unpackUpdate(con, stream);

   U32 i, size;

   // Transform
   if( stream->readFlag() )
   {
      MatrixF temp;
      stream->readAffineTransform(&temp);
      setTransform(temp);
   }

   // Read the polyhedron
   if( stream->readFlag() )
   {
      Polyhedron tempPH;
      stream->read(&size);
      tempPH.pointList.setSize(size);
      for (i = 0; i < tempPH.pointList.size(); i++)
         mathRead(*stream, &tempPH.pointList[i]);

      stream->read(&size);
      tempPH.planeList.setSize(size);
      for (i = 0; i < tempPH.planeList.size(); i++)
         mathRead(*stream, &tempPH.planeList[i]);

      stream->read(&size);
      tempPH.edgeList.setSize(size);
      for (i = 0; i < tempPH.edgeList.size(); i++) {
         Polyhedron::Edge& rEdge = tempPH.edgeList[i];

         stream->read(&rEdge.face[0]);
         stream->read(&rEdge.face[1]);
         stream->read(&rEdge.vertex[0]);
         stream->read(&rEdge.vertex[1]);
      }
      setTriggerPolyhedron(tempPH);
   }

   if( stream->readFlag() )
   {
      char buf[CMD_SIZE];
      stream->readLongString(CMD_SIZE-1, buf);
      mEnterCommand = buf;
   }
   if( stream->readFlag() )
   {
      char buf[CMD_SIZE];
      stream->readLongString(CMD_SIZE-1, buf);
      mLeaveCommand = buf;
   }
   if( stream->readFlag() )
   {
      char buf[CMD_SIZE];
      stream->readLongString(CMD_SIZE-1, buf);
      mTickCommand = buf;
   }
}

//ConsoleMethod( Trigger, getNumObjects, S32, 2, 2, "")
DefineEngineMethod( Trigger, getNumObjects, S32, (),,
   "@brief Get the number of objects that are within the Trigger's bounds.\n\n"
   "@see getObject()\n")
{
   return object->getNumTriggeringObjects();
}

//ConsoleMethod( Trigger, getObject, S32, 3, 3, "(int idx)")
DefineEngineMethod( Trigger, getObject, S32, ( S32 index ),,
   "@brief Retrieve the requested object that is within the Trigger's bounds.\n\n"
   "@param index Index of the object to get (range is 0 to getNumObjects()-1)\n"
   "@returns The SimObjectID of the object, or -1 if the requested index is invalid.\n"
   "@see getNumObjects()\n")
{
   if (index >= object->getNumTriggeringObjects() || index < 0)
      return -1;
   else
      return object->getObject(U32(index))->getId();
}
