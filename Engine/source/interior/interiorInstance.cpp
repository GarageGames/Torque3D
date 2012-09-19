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
#include "interior/interiorInstance.h"

#include "interior/interior.h"
#include "console/consoleTypes.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "scene/zones/sceneTraversalState.h"
#include "scene/zones/sceneRootZone.h"
#include "core/stream/bitStream.h"
#include "core/stream/fileStream.h"
#include "gfx/bitmap/gBitmap.h"
#include "math/mathIO.h"
#include "gui/worldEditor/editor.h"
#include "interior/interiorResObjects.h"
#include "scene/simPath.h"
#include "interior/forceField.h"
#include "lighting/lightManager.h"
#include "collision/convex.h"
#include "sfx/sfxProfile.h"
#include "sfx/sfxEnvironment.h"
#include "core/frameAllocator.h"
#include "sim/netConnection.h"
#include "platform/profiler.h"
#include "gui/3d/guiTSControl.h"
#include "math/mathUtils.h"
#include "renderInstance/renderPassManager.h"
#include "core/resourceManager.h"
#include "materials/materialManager.h"
#include "materials/materialFeatureTypes.h"
#include "materials/matInstance.h"
#include "collision/concretePolyList.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsBody.h"
#include "T3D/physics/physicsCollision.h"
#include "console/engineAPI.h"

#ifdef TORQUE_COLLADA
#include "ts/collada/colladaUtils.h"
#endif


IMPLEMENT_CO_NETOBJECT_V1(InteriorInstance);

ConsoleDocClass( InteriorInstance,
   "@brief Object used to represent buildings and other architectural structures (legacy).\n\n"
   
   "Interiors are made up entirely from convex hulls or, as they are more commonly known as by game "
   "artists, brushes. So what you see is what you collide against. There is no difference between the "
   "visible meshes and the collision meshes.\n\n"

   "Unlike a DTS or COLLADA mesh, interiors do not support any animation. They also do not support "
   "transparent textures. If you need animation or transparency then you are forced to use other model objects.\n\n"

   "It is important to note that interiors are no longer the preferred format for large structures. It is an " 
   "old format, which does not have much to offer above DTS or COLLADA. They are still included in Torque 3D "
   "for the sake of backwards compatibility for developers porting older TGE or TGEA projects. It will be "
   "deprecated soon.\n\n"
   
   "@ingroup gameObjects"
);


static const U32 csgMaxZoneSize = 256;
static bool sgScopeBoolArray[256];


bool InteriorInstance::smDontRestrictOutside = false;
F32  InteriorInstance::smDetailModification = 1.0f;



//-----------------------------------------------------------------------------

InteriorInstance::InteriorInstance()
{
   mAlarmState        = false;

   mInteriorFileName = NULL;
   mTypeMask |= InteriorObjectType | StaticObjectType | StaticShapeObjectType;
   mZoneFlags.clear( ZoneFlag_IsClosedOffSpace ); // Interiors are open spaces.

   mForcedDetailLevel = -1;

   mConvexList = new Convex;
   mCRC = 0;

   mPhysicsRep = NULL;
}

//-----------------------------------------------------------------------------

InteriorInstance::~InteriorInstance()
{
   delete mConvexList;
   mConvexList = NULL;

   // GFX2_RENDER_MERGE
   //for (U32 i = 0; i < mReflectPlanes.size(); i++)
   //   mReflectPlanes[i].clearTextures();
}

//-----------------------------------------------------------------------------

void InteriorInstance::inspectPostApply()
{
   // Apply any transformations set in the editor
   Parent::inspectPostApply();

   // Update the Transform on Editor Apply.
   setMaskBits(TransformMask);
}

//-----------------------------------------------------------------------------

void InteriorInstance::initPersistFields()
{
   addGroup("Media");

      addProtectedField( "interiorFile",  TypeFilename,  Offset( mInteriorFileName, InteriorInstance ),
         &_setInteriorFile, &defaultProtectedGetFn,
         "Path and filename of the Interior file (.DIF) to load for this InteriorInstance.");

   endGroup("Media");

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

void InteriorInstance::consoleInit()
{
   //-------------------------------------- Class level variables
   Con::addVariable( "pref::Interior::VertexLighting", TypeBool, &Interior::smUseVertexLighting,
      "Forces all InteriorInstances to not render their lightmaps.\n"
      "@ingroup Interior" );
   Con::addVariable( "pref::Interior::detailAdjust", TypeF32, &InteriorInstance::smDetailModification,
      "Forces all InteriorInstance rendering to a particular detail level.\n"
      "@ingroup Interior" );

   // DEBUG ONLY!!!
#ifndef TORQUE_SHIPPING
   Con::addVariable( "Interior::DontRestrictOutside", TypeBool, &smDontRestrictOutside,
      "Render only the outside zone of all InteriorInstances.\n"
      "@ingroup Interior" );
#endif
}

//-----------------------------------------------------------------------------

#ifdef TORQUE_COLLADA

void InteriorInstance::exportToCollada(bool bakeTransform)
{
   if (mInteriorRes->getNumDetailLevels() == 0)
   {
      Con::errorf("InteriorInstance::exportToCollada() called an InteriorInstance with no Interior");
      return;
   }

   // For now I am only worrying about the highest lod
   Interior* pInterior = mInteriorRes->getDetailLevel(0);

   if (!pInterior)
   {
      Con::errorf("InteriorInstance::exportToCollada() called an InteriorInstance with an invalid Interior");
      return;
   }

   // Get an optimized version of our mesh
   OptimizedPolyList interiorMesh;

   if (bakeTransform)
   {
      MatrixF mat = getTransform();
      Point3F scale = getScale();

      pInterior->buildExportPolyList(interiorMesh, &mat, &scale);
   }
   else
      pInterior->buildExportPolyList(interiorMesh);

   // Get our export path
   Torque::Path colladaFile = mInteriorRes.getPath();

   // Make sure to set our Collada extension
   colladaFile.setExtension("dae");

   // Use the InteriorInstance name if possible
   String meshName = getName();

   // Otherwise use the DIF's file name
   if (meshName.isEmpty())
      meshName = colladaFile.getFileName();

   // If we are baking the transform then append
   // a CRC version of the transform to the mesh/file name
   if (bakeTransform)
   {
      F32 trans[19];

      const MatrixF& mat = getTransform();
      const Point3F& scale = getScale();

      // Copy in the transform
      for (U32 i = 0; i < 4; i++)
      {
         for (U32 j = 0; j < 4; j++)
         {
            trans[i * 4 + j] = mat(i, j);
         }
      }

      // Copy in the scale
      trans[16] = scale.x;
      trans[17] = scale.y;
      trans[18] = scale.z;

      U32 crc = CRC::calculateCRC(trans, sizeof(F32) * 19);

      meshName += String::ToString("_%x", crc);
   }

   // Set the file name as the meshName
   colladaFile.setFileName(meshName);

   // Use a ColladaUtils function to do the actual export to a Collada file
   ColladaUtils::exportToCollada(colladaFile, interiorMesh, meshName);
}
#endif

//-----------------------------------------------------------------------------

bool InteriorInstance::onAdd()
{
   if(! _loadInterior())
      return false;

   if(!Parent::onAdd())
      return false;

   addToScene();

   if ( PHYSICSMGR && mInteriorRes && mInteriorRes->getNumDetailLevels() > 0 )
   {
      // TODO: We need to cache the collision by resource name
      // and reuse it across multiple instances.

      // Get the interior collision geometry.
      ConcretePolyList polylist;
      mInteriorRes->getDetailLevel(0)->buildPolyList( &polylist, Box3F(999999.0f), MatrixF::Identity, getScale() );
      polylist.triangulate();

      // Look out... this could possibly happen!
      if ( !polylist.isEmpty() )
      {
         // Use a triangle mesh for collision.
         PhysicsCollision *colShape = PHYSICSMGR->createCollision();
         colShape->addTriangleMesh( polylist.mVertexList.address(), 
                                    polylist.mVertexList.size(),
                                    polylist.mIndexList.address(),
                                    polylist.mIndexList.size() / 3,
                                    MatrixF::Identity );

         PhysicsWorld *world = PHYSICSMGR->getWorld( isServerObject() ? "server" : "client" );
         mPhysicsRep = PHYSICSMGR->createBody();
         mPhysicsRep->init( colShape, 0, 0, this, world );
         mPhysicsRep->setTransform( getTransform() );
      }
   }

   return true;
}

//-----------------------------------------------------------------------------

void InteriorInstance::onRemove()
{
   SAFE_DELETE( mPhysicsRep );

   _unloadInterior();

   removeFromScene();

   Parent::onRemove();
}

//-----------------------------------------------------------------------------

bool InteriorInstance::_loadInterior()
{
   U32 i;

   // Load resource
   mInteriorRes = ResourceManager::get().load(mInteriorFileName);
   if (bool(mInteriorRes) == false) {
      Con::errorf(ConsoleLogEntry::General, "Unable to load interior: %s", mInteriorFileName);
      NetConnection::setLastError("Unable to load interior: %s", mInteriorFileName);
      return false;
   }
   if(isClientObject())
   {
      if(mCRC != mInteriorRes.getChecksum())
      {
         NetConnection::setLastError("Local interior file '%s' does not match version on server.", mInteriorFileName);
         return false;
      }
      for (i = 0; i < mInteriorRes->getNumDetailLevels(); i++) {
         // ok, if the material list load failed...
         // if this is a local connection, we'll assume that's ok
         // and just have white textures...
         // otherwise we want to return false.
         Interior* pInterior = mInteriorRes->getDetailLevel(i);
         if(!pInterior->prepForRendering(mInteriorRes.getPath().getFullPath().c_str()) )
         {
            if(!bool(mServerObject))
            {
               return false;
            }
         }
      }

      // copy planar reflect list from top detail level - for now
      Interior* pInterior = mInteriorRes->getDetailLevel(0);
      if( pInterior->mReflectPlanes.size() )
      {
         for ( i = 0; i < pInterior->mReflectPlanes.size(); i++ )
         {
            mPlaneReflectors.increment();
            PlaneReflector &plane = mPlaneReflectors.last();

            plane.refplane = pInterior->mReflectPlanes[i];
            plane.objectSpace = true;
            plane.registerReflector( this, &mReflectorDesc );
         }         
      }

   }
   else
      mCRC = mInteriorRes.getChecksum();

   // Ok, everything's groovy!  Let's cache our hashed filename for renderimage sorting...
   mInteriorFileHash = _StringTable::hashString(mInteriorFileName);

   // Setup bounding information
   mObjBox = mInteriorRes->getDetailLevel(0)->getBoundingBox();
   resetWorldBox();
   setRenderTransform(mObjToWorld);


   // Do any handle loading, etc. required.

   if (isClientObject()) {

      for (i = 0; i < mInteriorRes->getNumDetailLevels(); i++) {
         Interior* pInterior = mInteriorRes->getDetailLevel(i);

         // Force the lightmap manager to download textures if we're
         // running the mission editor.  Normally they are only
         // downloaded after the whole scene is lit.
         gInteriorLMManager.addInstance(pInterior->getLMHandle(), mLMHandle, this);
         if (gEditingMission)  {
            gInteriorLMManager.useBaseTextures(pInterior->getLMHandle(), mLMHandle);
            gInteriorLMManager.downloadGLTextures(pInterior->getLMHandle());
         }

         // Install material list
         //         mMaterialMaps.push_back(new MaterialList(pInterior->mMaterialList));
      }

   } else {

   }

   setMaskBits(0xffffffff);
   return true;
}

//-----------------------------------------------------------------------------

void InteriorInstance::_unloadInterior()
{
   mConvexList->nukeList();
   delete mConvexList;
   mConvexList = new Convex;

   if(isClientObject())
   {
      if(bool(mInteriorRes) && mLMHandle != 0xFFFFFFFF)
      {
         for(U32 i = 0; i < mInteriorRes->getNumDetailLevels(); i++)
         {
            Interior * pInterior = mInteriorRes->getDetailLevel(i);
            if (pInterior->getLMHandle() != 0xFFFFFFFF)
               gInteriorLMManager.removeInstance(pInterior->getLMHandle(), mLMHandle);
         }
      }
      
      if( mPlaneReflectors.size() )
      {
         for ( U32 i = 0; i < mPlaneReflectors.size(); i++ )
         {
            mPlaneReflectors[i].unregisterReflector();
         }         
         mPlaneReflectors.clear();
      }
   }
}

//-----------------------------------------------------------------------------

bool InteriorInstance::onSceneAdd()
{
   AssertFatal(mInteriorRes, "Error, should not have been added to the scene if there's no interior!");

   if (Parent::onSceneAdd() == false)
      return false;

   U32 maxNumZones = 0;

   for (U32 i = 0; i < mInteriorRes->getNumDetailLevels(); i++)
   {
      if (mInteriorRes->getDetailLevel(i)->mZones.size() > maxNumZones)
         maxNumZones = mInteriorRes->getDetailLevel(i)->mZones.size();
   }

   if( maxNumZones > 1 )
   {
      SceneZoneSpaceManager* zoneManager = getSceneManager()->getZoneManager();
      if( zoneManager )
      {
         zoneManager->registerZones(this, (maxNumZones - 1));

         // Connect to outdoor zone.
         zoneManager->getRootZone()->connectZoneSpace( this );
         connectZoneSpace( zoneManager->getRootZone() );
      }
   }

   return true;
}

//-----------------------------------------------------------------------------

void InteriorInstance::onSceneRemove()
{
   // Disconnect from root zone in case we have connected.

   SceneZoneSpaceManager* zoneManager = getSceneManager()->getZoneManager();
   if( zoneManager )
      zoneManager->getRootZone()->disconnectZoneSpace( this );

   Parent::onSceneRemove();
}

//-----------------------------------------------------------------------------

bool InteriorInstance::_getOverlappingZones( const Box3F& aabb, const MatrixF& transform, const Point3F& scale, U32* outZones, U32& outNumZones )
{
   MatrixF xForm(true);
   Point3F invScale(1.0f / getScale().x,
      1.0f / getScale().y,
      1.0f / getScale().z);
   xForm.scale(invScale);
   xForm.mul(getWorldTransform());
   xForm.mul(transform);
   xForm.scale(scale);

   U32 waterMark = FrameAllocator::getWaterMark();

   U16* zoneVector = (U16*)FrameAllocator::alloc(mInteriorRes->getDetailLevel(0)->mZones.size() * sizeof(U16));
   U32 numRetZones = 0;

   bool outsideToo = mInteriorRes->getDetailLevel(0)->scanZones(aabb,
      xForm,
      zoneVector,
      &numRetZones
   );

   if (numRetZones > SceneObject::MaxObjectZones)
   {
      Con::warnf(ConsoleLogEntry::General, "Too many zones returned for query on %s.  Returning first %d",
         mInteriorFileName, SceneObject::MaxObjectZones);
   }

   for (U32 i = 0; i < getMin(numRetZones, U32(SceneObject::MaxObjectZones)); i++)
      outZones[i] = zoneVector[i] + mZoneRangeStart - 1;
   outNumZones = numRetZones;

   FrameAllocator::setWaterMark(waterMark);

   return outsideToo;
}

//-----------------------------------------------------------------------------

bool InteriorInstance::getOverlappingZones( const Box3F& aabb, U32* outZones, U32& outNumZones )
{
   return _getOverlappingZones( aabb, MatrixF::Identity, Point3F( 1.f, 1.f, 1.f ), outZones, outNumZones );
}

//-----------------------------------------------------------------------------

bool InteriorInstance::getOverlappingZones( SceneObject* obj, U32* outZones, U32& outNumZones )
{
   return _getOverlappingZones( obj->getObjBox(), obj->getTransform(), obj->getScale(), outZones, outNumZones );
}

//-----------------------------------------------------------------------------

U32 InteriorInstance::getPointZone(const Point3F& p)
{
   AssertFatal(mInteriorRes, "Error, no interior!");

   Point3F osPoint = p;
   mWorldToObj.mulP(osPoint);
   osPoint.convolveInverse(mObjScale);

   S32 zone = mInteriorRes->getDetailLevel(0)->getZoneForPoint(osPoint);

   // If we're in solid (-1) or outside, we need to return 0
   if (zone == -1 || zone == 0)
      return SceneZoneSpaceManager::InvalidZoneId;

   return (zone-1) + mZoneRangeStart;
}

//-----------------------------------------------------------------------------

// does a hack check to determine how much a point is 'inside'.. should have
// portals prebuilt with the transfer energy to each other portal in the zone
// from the neighboring zone.. these values can be used to determine the factor
// from within an individual zone.. also, each zone could be marked with
// average material property for eax environment audio
// ~0: outside -> 1: inside
bool InteriorInstance::getPointInsideScale(const Point3F & pos, F32 * pScale)
{
   AssertFatal(mInteriorRes, "InteriorInstance::getPointInsideScale: no interior");

   Interior * interior = mInteriorRes->getDetailLevel(0);

   Point3F p = pos;
   mWorldToObj.mulP(p);
   p.convolveInverse(mObjScale);

   U32 zoneIndex = interior->getZoneForPoint(p);
   if(zoneIndex == -1)  // solid?
   {
      *pScale = 1.f;
      return(true);
   }
   else if(zoneIndex == 0) // outside?
   {
      *pScale = 0.f;
      return(true);
   }

   U32 waterMark = FrameAllocator::getWaterMark();
   const Interior::Portal** portals = (const Interior::Portal**)FrameAllocator::alloc(256 * sizeof(const Interior::Portal*));
   U32 numPortals = 0;

   Interior::Zone & zone = interior->mZones[zoneIndex];

   U32 i;
   for(i = 0; i < zone.portalCount; i++)
   {
      const Interior::Portal & portal = interior->mPortals[interior->mZonePortalList[zone.portalStart + i]];
      if(portal.zoneBack == 0 || portal.zoneFront == 0) {
         AssertFatal(numPortals < 256, "Error, overflow in temporary portal buffer!");
         portals[numPortals++] = &portal;
      }
   }

   // inside?
   if(numPortals == 0)
   {
      *pScale = 1.f;

      FrameAllocator::setWaterMark(waterMark);
      return(true);
   }

   Point3F* portalCenters = (Point3F*)FrameAllocator::alloc(numPortals * sizeof(Point3F));
   U32 numPortalCenters = 0;

   // scale using the distances to the portals in this zone...
   for(i = 0; i < numPortals; i++)
   {
      const Interior::Portal * portal = portals[i];
      if(!portal->triFanCount)
         continue;

      Point3F center(0, 0, 0);
      for(U32 j = 0; j < portal->triFanCount; j++)
      {
         const Interior::TriFan & fan = interior->mWindingIndices[portal->triFanStart + j];
         U32 numPoints = fan.windingCount;

         if(!numPoints)
            continue;

         for(U32 k = 0; k < numPoints; k++)
         {
            const Point3F & a = interior->mPoints[interior->mWindings[fan.windingStart + k]].point;
            center += a;
         }

         center /= (F32)numPoints;
         portalCenters[numPortalCenters++] = center;
      }
   }

   // 'magic' check here...
   F32 magic = Con::getFloatVariable("Interior::insideDistanceFalloff", 10.f);

   F32 val = 0.f;
   for(i = 0; i < numPortalCenters; i++)
      val += 1.f - mClampF(Point3F(portalCenters[i] - p).len() / magic, 0.f, 1.f);

   *pScale = 1.f - mClampF(val, 0.f, 1.f);

   FrameAllocator::setWaterMark(waterMark);
   return(true);
}

//-----------------------------------------------------------------------------

void InteriorInstance::_renderObject( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* overrideMat )
{
#ifndef TORQUE_SHIPPING
   if (Interior::smRenderMode == 0)
      return;

   if (overrideMat)
      return;

   if(gEditingMission && isHidden())
      return;

   U32 detailLevel = 0;
   detailLevel = _calcDetailLevel(state, state->getCameraPosition());

   Interior* pInterior = mInteriorRes->getDetailLevel( detailLevel );

   if (!pInterior)
      return;

   PROFILE_START( IRO_DebugRender );

   GFX->pushWorldMatrix();

   // setup world matrix - for fixed function
   MatrixF world = GFX->getWorldMatrix();
   world.mul( getRenderTransform() );
   world.scale( getScale() );
   GFX->setWorldMatrix( world );

   // setup world matrix - for shaders
   MatrixF proj = GFX->getProjectionMatrix();
   proj.mul(world);

   SceneData sgData;

   sgData = pInterior->setupSceneGraphInfo( this, state );
   ZoneVisDeterminer zoneVis = pInterior->setupZoneVis( this, state );
   pInterior->debugRender( zoneVis, sgData, this, proj );

   GFX->popWorldMatrix();

   PROFILE_END();
#endif
}

//-----------------------------------------------------------------------------

U32 InteriorInstance::_calcDetailLevel(SceneRenderState* state, const Point3F& wsPoint)
{
   AssertFatal(mInteriorRes, "Error, should not try to calculate the deatil level without a resource to work with!");
   AssertFatal(_getNumCurrZones() > 0, "Error, must belong to a zone for this to work");

   if (smDetailModification < 0.3f)
      smDetailModification = 0.3f;
   if (smDetailModification > 1.0f)
      smDetailModification = 1.0f;

   // Early out for simple interiors
   if (mInteriorRes->getNumDetailLevels() == 1)
      return 0;

   if((mForcedDetailLevel >= 0) && (mForcedDetailLevel < mInteriorRes->getNumDetailLevels()))
      return(mForcedDetailLevel);

   Point3F osPoint = wsPoint;
   mRenderWorldToObj.mulP(osPoint);
   osPoint.convolveInverse(mObjScale);

   // First, see if the point is in the object space bounding box of the highest detail
   //  If it is, then the detail level is zero.
   if (mObjBox.isContained(osPoint))
      return 0;

   // Otherwise, we're going to have to do some ugly trickery to get the projection.
   //  I've stolen the worldToScreenScale from dglMatrix, we'll have to calculate the
   //  projection of the bounding sphere of the lowest detail level.
   //  worldToScreenScale = (near * view.extent.x) / (right - left)
   F32 worldToScreenScale   = state->getWorldToScreenScale().x;
   const SphereF& lowSphere = mInteriorRes->getDetailLevel(mInteriorRes->getNumDetailLevels() - 1)->mBoundingSphere;
   F32 dist                 = (lowSphere.center - osPoint).len();
   F32 projRadius           = (lowSphere.radius / dist) * worldToScreenScale;

   // Scale the projRadius based on the objects maximum scale axis
   projRadius *= getMax(mFabs(mObjScale.x), getMax(mFabs(mObjScale.y), mFabs(mObjScale.z)));

   // Multiply based on detail preference...
   projRadius *= smDetailModification;

   // Ok, now we have the projected radius, we need to search through the interiors to
   //  find the largest interior that will support this projection.
   U32 final = mInteriorRes->getNumDetailLevels() - 1;
   for (U32 i = 0; i< mInteriorRes->getNumDetailLevels() - 1; i++) {
      Interior* pDetail = mInteriorRes->getDetailLevel(i);

      if (pDetail->mMinPixels < projRadius) {
         final = i;
         break;
      }
   }

   // Ok, that's it.
   return final;
}

//-----------------------------------------------------------------------------

void InteriorInstance::traverseZones( SceneTraversalState* state )
{
   U32 startZone = getPointZone( state->getCullingState()->getCameraState().getViewPosition() );
   if( startZone != SceneZoneSpaceManager::InvalidZoneId )
      startZone = startZone - mZoneRangeStart + 1;
   else
      startZone = SceneZoneSpaceManager::RootZoneId;

   traverseZones( state, startZone );
}

//-----------------------------------------------------------------------------

void InteriorInstance::traverseZones( SceneTraversalState* state, U32 startZoneId )
{
   PROFILE_SCOPE( InteriorInstance_traverseZones );

   SceneCullingState* cullingState = state->getCullingState();

   // [rene, 23-Mar-11] This is a really gross hack.  It effectively renders all zoning in interiors
   //    ineffective and just lets them render with the root frustum.  It's just that after untangling
   //    DMM's mess in the sceneGraph system, I just don't have the energy anymore to also dig through
   //    the ungodly mess that is the interior code and since this is all quasi-deprecated-and-soon-to-die
   //    anyway it would just be wasted effort.

   for( U32 i = getZoneRangeStart(); i < ( getZoneRangeStart() + getZoneRange() ); ++ i )
      cullingState->addCullingVolumeToZone( i, state->getCurrentCullingVolume() );

#if 0
   U32 baseZoneForPrep = getCurrZone( 0 );
   bool multipleZones = ( getNumCurrZones() > 1 );

   Frustum outFrustum;
   bool continueOut = mInteriorRes->getDetailLevel( 0 )->traverseZones(
      renderState,
      frustum,
      baseZoneForPrep,
      startZoneId,
      mZoneRangeStart,
      mRenderObjToWorld,
      mObjScale,
      smDontRestrictOutside | multipleZones,
      renderState->isInvertedWorld(),
      outFrustum
   );

   if( smDontRestrictOutside )
      continueOut = true;
#endif

   if( true )// continueOut )
   {
      state->pushZone( startZoneId );
      _traverseConnectedZoneSpaces( state );
      state->popZone();
   }
}

//-----------------------------------------------------------------------------

void InteriorInstance::prepRenderImage( SceneRenderState* state )
{
   PROFILE_SCOPE( InteriorInstance_prepRenderImage );   

   U32 detailLevel = _calcDetailLevel( state, state->getCameraPosition() );
   Interior* pInterior = getResource()->getDetailLevel( detailLevel );
   pInterior->prepBatchRender( this, state );
}

//-----------------------------------------------------------------------------

bool InteriorInstance::castRay(const Point3F& s, const Point3F& e, RayInfo* info)
{
   info->object = this;
   return mInteriorRes->getDetailLevel(0)->castRay(s, e, info);
}

//-----------------------------------------------------------------------------

bool InteriorInstance::buildPolyList(PolyListContext context, AbstractPolyList* list, const Box3F& wsBox, const SphereF&)
{
   if (bool(mInteriorRes) == false)
      return false;

   // Setup collision state data
   list->setTransform(&getTransform(), getScale());
   list->setObject(this);

   return mInteriorRes->getDetailLevel(0)->buildPolyList(list, wsBox, mWorldToObj, getScale());
}

//-----------------------------------------------------------------------------

void InteriorInstance::buildConvex(const Box3F& box, Convex* convex)
{
   if (bool(mInteriorRes) == false)
      return;

   mConvexList->collectGarbage();

   Box3F realBox = box;
   mWorldToObj.mul(realBox);
   realBox.minExtents.convolveInverse(mObjScale);
   realBox.maxExtents.convolveInverse(mObjScale);

   if (realBox.isOverlapped(getObjBox()) == false)
      return;

   U32 waterMark = FrameAllocator::getWaterMark();

   if ((convex->getObject()->getTypeMask() & VehicleObjectType) &&
       mInteriorRes->getDetailLevel(0)->mVehicleConvexHulls.size() > 0)
   {
      // Can never have more hulls than there are hulls in the interior...
      U16* hulls = (U16*)FrameAllocator::alloc(mInteriorRes->getDetailLevel(0)->mVehicleConvexHulls.size() * sizeof(U16));
      U32 numHulls = 0;

      Interior* pInterior = mInteriorRes->getDetailLevel(0);
      if (pInterior->getIntersectingVehicleHulls(realBox, hulls, &numHulls) == false) {
         FrameAllocator::setWaterMark(waterMark);
         return;
      }

      for (U32 i = 0; i < numHulls; i++) {
         // See if this hull exists in the working set already...
         Convex* cc = 0;
         CollisionWorkingList& wl = convex->getWorkingList();
         for (CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext) {
            if (itr->mConvex->getType() == InteriorConvexType &&
                (static_cast<InteriorConvex*>(itr->mConvex)->getObject() == this &&
                 static_cast<InteriorConvex*>(itr->mConvex)->hullId    == -S32(hulls[i] + 1))) {
               cc = itr->mConvex;
               break;
            }
         }
         if (cc)
            continue;

         // Create a new convex.
         InteriorConvex* cp = new InteriorConvex;
         mConvexList->registerObject(cp);
         convex->addToWorkingList(cp);
         cp->mObject   = this;
         cp->pInterior = pInterior;
         cp->hullId    = -S32(hulls[i] + 1);
         cp->box.minExtents.x = pInterior->mVehicleConvexHulls[hulls[i]].minX;
         cp->box.minExtents.y = pInterior->mVehicleConvexHulls[hulls[i]].minY;
         cp->box.minExtents.z = pInterior->mVehicleConvexHulls[hulls[i]].minZ;
         cp->box.maxExtents.x = pInterior->mVehicleConvexHulls[hulls[i]].maxX;
         cp->box.maxExtents.y = pInterior->mVehicleConvexHulls[hulls[i]].maxY;
         cp->box.maxExtents.z = pInterior->mVehicleConvexHulls[hulls[i]].maxZ;
      }
   }
   else
   {
      // Can never have more hulls than there are hulls in the interior...
      U16* hulls = (U16*)FrameAllocator::alloc(mInteriorRes->getDetailLevel(0)->mConvexHulls.size() * sizeof(U16));
      U32 numHulls = 0;

      Interior* pInterior = mInteriorRes->getDetailLevel(0);
      if (pInterior->getIntersectingHulls(realBox, hulls, &numHulls) == false) {
         FrameAllocator::setWaterMark(waterMark);
         return;
      }

      for (U32 i = 0; i < numHulls; i++) {
         // See if this hull exists in the working set already...
         Convex* cc = 0;
         CollisionWorkingList& wl = convex->getWorkingList();
         for (CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext) {
            if (itr->mConvex->getType() == InteriorConvexType &&
                (static_cast<InteriorConvex*>(itr->mConvex)->getObject() == this &&
                 static_cast<InteriorConvex*>(itr->mConvex)->hullId    == hulls[i])) {
               cc = itr->mConvex;
               break;
            }
         }
         if (cc)
            continue;

         // Create a new convex.
         InteriorConvex* cp = new InteriorConvex;
         mConvexList->registerObject(cp);
         convex->addToWorkingList(cp);
         cp->mObject   = this;
         cp->pInterior = pInterior;
         cp->hullId    = hulls[i];
         cp->box.minExtents.x = pInterior->mConvexHulls[hulls[i]].minX;
         cp->box.minExtents.y = pInterior->mConvexHulls[hulls[i]].minY;
         cp->box.minExtents.z = pInterior->mConvexHulls[hulls[i]].minZ;
         cp->box.maxExtents.x = pInterior->mConvexHulls[hulls[i]].maxX;
         cp->box.maxExtents.y = pInterior->mConvexHulls[hulls[i]].maxY;
         cp->box.maxExtents.z = pInterior->mConvexHulls[hulls[i]].maxZ;
      }
   }
   FrameAllocator::setWaterMark(waterMark);
}

//-----------------------------------------------------------------------------

U32 InteriorInstance::packUpdate(NetConnection* c, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(c, mask, stream);

   if (stream->writeFlag((mask & InitMask) != 0)) {
      // Initial update, write the whole kit and kaboodle
      stream->write(mCRC);

      stream->writeString(mInteriorFileName);

      // Write the alarm state
      stream->writeFlag(mAlarmState);
   }
   else
   {
      stream->writeFlag(mAlarmState);
   }

   return retMask;
}

//-----------------------------------------------------------------------------

void InteriorInstance::unpackUpdate(NetConnection* c, BitStream* stream)
{
   Parent::unpackUpdate(c, stream);

   MatrixF temp;
   Point3F tempScale;

   if (stream->readFlag()) {
      bool isNewUpdate(mInteriorRes);

      if(isNewUpdate)
         _unloadInterior();

      // Initial Update
      // CRC
      stream->read(&mCRC);

      // File
      mInteriorFileName = stream->readSTString();

      // Alarm state: Note that we handle this ourselves on the initial update
      //  so that the state is always full on or full off...
      mAlarmState = stream->readFlag();

      if(isNewUpdate)
      {
         if(! _loadInterior())
            Con::errorf("InteriorInstance::unpackUpdate - Unable to load new interior");
      }
   }
   else
   {
      setAlarmMode(stream->readFlag());
   }
}

//-----------------------------------------------------------------------------

Interior* InteriorInstance::getDetailLevel(const U32 level)
{
   return mInteriorRes->getDetailLevel(level);
}

//-----------------------------------------------------------------------------

U32 InteriorInstance::getNumDetailLevels()
{
   return mInteriorRes->getNumDetailLevels();
}

//-----------------------------------------------------------------------------

void InteriorInstance::setAlarmMode(const bool alarm)
{
   if (mInteriorRes->getDetailLevel(0)->mHasAlarmState == false)
      return;

   if (mAlarmState == alarm)
      return;

   mAlarmState = alarm;
   if (isServerObject())
   {
      setMaskBits(AlarmMask);
   }
   else
   {
      // DMMTODO: Invalidate current light state
   }
}

//-----------------------------------------------------------------------------

void InteriorInstance::_createTriggerTransform(const InteriorResTrigger* trigger, MatrixF* transform)
{
   Point3F offset;
   MatrixF xform = getTransform();
   xform.getColumn(3, &offset);

   Point3F triggerOffset = trigger->mOffset;
   triggerOffset.convolve(mObjScale);
   getTransform().mulV(triggerOffset);
   offset += triggerOffset;
   xform.setColumn(3, offset);

   *transform = xform;
}

//-----------------------------------------------------------------------------

bool InteriorInstance::readLightmaps(GBitmap**** lightmaps)
{
   AssertFatal(mInteriorRes, "Error, no interior loaded!");
   AssertFatal(lightmaps, "Error, no lightmaps or numdetails result pointers");
   AssertFatal(*lightmaps == NULL, "Error, already have a pointer in the lightmaps result field!");

   // Load resource
   FileStream* pStream;
   if((pStream = FileStream::createAndOpen( mInteriorFileName, Torque::FS::File::Read )) == NULL)
   {
      Con::errorf(ConsoleLogEntry::General, "Unable to load interior: %s", mInteriorFileName);
      return false;
   }

   InteriorResource* pResource = new InteriorResource;
   bool success = pResource->read(*pStream);
   delete pStream;

   if (success == false)
   {
      delete pResource;
      return false;
   }
   AssertFatal(pResource->getNumDetailLevels() == mInteriorRes->getNumDetailLevels(),
               "Mismatched detail levels!");

   *lightmaps  = new GBitmap**[mInteriorRes->getNumDetailLevels()];

   for (U32 i = 0; i < pResource->getNumDetailLevels(); i++)
   {
      Interior* pInterior = pResource->getDetailLevel(i);
      (*lightmaps)[i] = new GBitmap*[pInterior->mLightmaps.size()];
      for (U32 j = 0; j < pInterior->mLightmaps.size(); j++)
      {
         ((*lightmaps)[i])[j] = pInterior->mLightmaps[j];
         pInterior->mLightmaps[j] = NULL;
      }
      pInterior->mLightmaps.clear();
   }

   delete pResource;
   return true;
}

//-----------------------------------------------------------------------------

S32 InteriorInstance::getSurfaceZone(U32 surfaceindex, Interior *detail)
{
	AssertFatal(((surfaceindex >= 0) && (surfaceindex < detail->surfaceZones.size())), "Bad surface index!");
	S32 zone = detail->surfaceZones[surfaceindex];
	if(zone > -1)
		return zone + mZoneRangeStart;
	return _getCurrZone(0);
}

//-----------------------------------------------------------------------------

bool InteriorInstance::_setInteriorFile( void *object, const char *, const char *data )
{
   if(data == NULL)
      return true;

   InteriorInstance *inst = static_cast<InteriorInstance *>(object);

   if(inst->isProperlyAdded())
      inst->_unloadInterior();

   inst->mInteriorFileName = StringTable->insert(data);

   if(inst->isProperlyAdded())
   {
      if(! inst->_loadInterior())
         Con::errorf("InteriorInstance::setInteriorFile - Unable to load new interior");
   }

   return false;
}

//=============================================================================
//    Console API.
//=============================================================================
// MARK: ---- Console API ----

ConsoleFunctionGroupBegin(Interiors, "");

//-----------------------------------------------------------------------------

#ifndef TORQUE_SHIPPING

DefineEngineFunction( setInteriorRenderMode, void, ( S32 mode ),,
   "Globally changes how InteriorInstances are rendered. Useful for debugging geometry and rendering artifacts\n"
   
   "@note This does not work in shipping mode\n\n"

   "@param mode The render mode can be one of the following numbers:\n\n"
   "NormalRender            = 0,\n\n"
   "NormalRenderLines       = 1,\n\n"
   "ShowDetail              = 2,\n\n"
   "ShowAmbiguous           = 3,\n\n"
   "ShowOrphan              = 4,\n\n"
   "ShowLightmaps           = 5,\n\n"
   "ShowTexturesOnly        = 6,\n\n"
   "ShowPortalZones         = 7,\n\n"
   "ShowOutsideVisible      = 8,\n\n"
   "ShowCollisionFans       = 9,\n\n"
   "ShowStrips              = 10,\n\n"
   "ShowNullSurfaces        = 11,\n\n"
   "ShowLargeTextures       = 12,\n\n"
   "ShowHullSurfaces        = 13,\n\n"
   "ShowVehicleHullSurfaces = 14,\n\n"
   "ShowVertexColors        = 15,\n\n"
   "ShowDetailLevel         = 16\n\n"
   
   "@ingroup Game" )
{
	if (mode < 0 || mode > Interior::ShowDetailLevel)
		mode = 0;

	Interior::smRenderMode = mode;
}

//-----------------------------------------------------------------------------

ConsoleFunction( setInteriorFocusedDebug, void, 2, 2, "(bool enable)"
				"@brief No longer properly supported\n\n"
				"@internal")
{
   if (dAtob(argv[1])) {
      Interior::smFocusedDebug = true;
   } else {
      Interior::smFocusedDebug = false;
   }
}

#endif

//-----------------------------------------------------------------------------

ConsoleDocFragment _isPointInside1(
   "@brief Check to see if a point in world space is inside of an interior.\n\n"

   "@param position The position to check in world space.\n\n"

   "@tsexample\n"
      "// Check to see if a point is inside any interior\n"
      "%point = \"100 100 100\";\n"
      "%isInside = isPointInside(%point);\n"
   "@endtsexample\n\n"

   "@ingroup Game",
   NULL,
   "bool isPointInside( Point3F position );"
);
ConsoleDocFragment _isPointInside2(
   "Check to see if a set of coordinates in world space are inside of an interior.\n\n"
   "@param x X-coordinate for position in world space.\n"
   "@param y Y-coordinate for position in world space.\n"
   "@param z Z-coordinate for position in world space.\n"
   "@tsexample\n\n"
   "// Check to see if a point is inside any interior\n"
   "%isInside = isPointInside(100, 100, 100);\n"
   "@endtsexample\n\n"
   "@ingroup Game",
   NULL,
   "bool isPointInside( F32 x, F32 y, F32 z );"
);

ConsoleFunction( isPointInside, bool, 2, 4, "Check to see if a point in world space is inside of an interior."
				"@hide")
{
   static bool lastValue = false;

   if(!(argc == 2 || argc == 4))
   {
      Con::errorf(ConsoleLogEntry::General, "cIsPointInside: invalid parameters");
      return(lastValue);
   }

   Point3F pos;
   if(argc == 2)
      dSscanf(argv[1], "%g %g %g", &pos.x, &pos.y, &pos.z);
   else
   {
      pos.x = dAtof(argv[1]);
      pos.y = dAtof(argv[2]);
      pos.z = dAtof(argv[3]);
   }

   RayInfo collision;
   if(gClientContainer.castRay(pos, Point3F(pos.x, pos.y, pos.z - 2000.f), InteriorObjectType, &collision))
   {
      if(collision.face == -1)
         Con::errorf(ConsoleLogEntry::General, "cIsPointInside: failed to find hit face on interior");
      else
      {
         InteriorInstance * interior = dynamic_cast<InteriorInstance *>(collision.object);
         if(interior)
            lastValue = !interior->getDetailLevel(0)->isSurfaceOutsideVisible(collision.face);
         else
            Con::errorf(ConsoleLogEntry::General, "cIsPointInside: invalid interior on collision");
      }
   }

   return(lastValue);
}


ConsoleFunctionGroupEnd(Interiors);

//-----------------------------------------------------------------------------

#ifdef TORQUE_COLLADA

DefineEngineMethod( InteriorInstance, exportToCollada, void, ( bool bakeTransform ),,
   "@brief Exports the Interior to a Collada file\n\n"

   "@param bakeTransform Bakes the InteriorInstance's transform into the vertex positions\n\n"
   
   "@tsexample\n"
   "// Export to COLLADA, do not bakeTransform\n"
   "%interiorObject.exportToCollada(0);\n"
   "@endtsexample\n\n")
{
	object->exportToCollada(bakeTransform);
}
#endif

//-----------------------------------------------------------------------------

DefineEngineMethod( InteriorInstance, setAlarmMode, void, ( const char* alarmMode),,
   "@brief This sets the alarm mode of the interior\n\n"

   "The alarm mode is used when debugging bad geometry for an interior. When on, the the bad verties "
   "will be rendered a different color.\n\n"

   "@param alarmMode If true the interior will be in an alarm state next frame. Options are \'On\' or \'Off\'.\n\n"
   
   "@tsexample\n"
   "// Turn on alarm mode debugging for interior\n"
   "%interiorObject.setAlarmMode(\"On\");\n"
   "@endtsexample\n\n")
{
	AssertFatal(dynamic_cast<InteriorInstance*>(object) != NULL,
      "Error, how did a non-interior get here?");

   bool alarm;
   if (dStricmp(alarmMode, "On") == 0)
      alarm = true;
   else
      alarm = false;

   InteriorInstance* interior = static_cast<InteriorInstance*>(object);
   if (interior->isClientObject()) {
      Con::errorf(ConsoleLogEntry::General, "InteriorInstance: client objects may not receive console commands.  Ignored");
      return;
   }

   interior->setAlarmMode(alarm);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( InteriorInstance, getNumDetailLevels, S32, (),,
   "@brief Get the number of detail levels interior was created with\n\n"
   
   "@tsexample\n"
   "%numLODs = %interiorObject.getNumDetailLevels();\n"
   "echo(%numLODs);\n"
   "@endtsexample\n\n")
{
	InteriorInstance * instance = static_cast<InteriorInstance*>(object);
	return(instance->getNumDetailLevels());
}

//-----------------------------------------------------------------------------

DefineEngineMethod( InteriorInstance, setDetailLevel, void, (S32 level),,
   "@brief Manually changes the current detail level, rather than automatically via view distance\n\n"
   
   "@param level Detail level to force.\n\n"

   "@tsexample\n"
   "%interiorObject.setDetailLevel(2);\n"
   "@endtsexample\n\n")
{
	InteriorInstance * instance = static_cast<InteriorInstance*>(object);
	if(instance->isServerObject())
	{
		NetConnection * toServer = NetConnection::getConnectionToServer();
		NetConnection * toClient = NetConnection::getLocalClientConnection();
		if(!toClient || !toServer)
			return;

		S32 index = toClient->getGhostIndex(instance);
		if(index == -1)
			return;

		InteriorInstance * clientInstance = dynamic_cast<InteriorInstance*>(toServer->resolveGhost(index));
		if(clientInstance)
			clientInstance->setDetailLevel(level);
	}
	else
		instance->setDetailLevel(level);
}

//-----------------------------------------------------------------------------

//These functions are duplicated in tsStatic, shapeBase, and interiorInstance.
//They each function a little differently; but achieve the same purpose of gathering
//target names/counts without polluting simObject.

DefineEngineMethod( InteriorInstance, getTargetName, const char*, (S32 detailLevel, S32 targetNum),,
   "@brief Get the name of the indexed shape material\n\n"
   
   "@param	detailLevel Target LOD\n"
   "@param	targetNum Index mapped to the target\n\n"

   "@return The name of the target (material) at the specified detail level and index\n\n"

   "@tsexample\n"
   "// First level of detail, top of the index map\n"
   "%targetName = %interiorObject.getTargetName(1, 0);\n"
   "echo(%targetName);\n"
   "@endtsexample\n\n")
{
	Interior* obj = object->getDetailLevel(detailLevel);

	if(obj)
		return obj->getTargetName(targetNum);

	return "";
}

//-----------------------------------------------------------------------------

DefineEngineMethod( InteriorInstance, getTargetCount, S32, (U32 detailLevel),,
   "@brief Get the number of materials used by interior\n\n"
   
   "@param	detailLevel Interior level of detail to scan\n"

   "@return The number of materials used by the interior at a specified detail level\n\n"

   "@tsexample\n"
   "// Find materials used at first level of detail\n"
   "%targetCount = %interiorObject.getTargetCount(1);\n"
   "echo(%targetCount);\n"
   "@endtsexample\n\n")
{
	Interior* obj = object->getDetailLevel(detailLevel);
	if(obj)
		return obj->getTargetCount();

	return -1;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( InteriorInstance, changeMaterial, void, (const char* mapTo, Material* oldMat, Material* newMat),,
   "@brief Change one of the materials on the shape.\n\n"

   "This method changes materials per mapTo with others. The material that "
   "is being replaced is mapped to unmapped_mat as a part of this transition.\n\n"

   "@note Warning, right now this only sort of works. It doesn't do a live "
   "update like it should.\n\b"

   "@param mapTo The name of the material target to remap (from getTargetName)\n"
   "@param oldMat The old Material that was mapped \n"
   "@param newMat The new Material to map\n\n"

   "@tsexample\n"
   "// remap the first material in the shape\n"
   "%mapTo = %interiorObject.getTargetName( 0 );\n"
   "%interiorObject.changeMaterial( %mapTo, 0, MyMaterial );\n"
   "@endtsexample\n" )
{
   // if no valid new material, theres no reason for doing this
   if( !newMat )
   {
      Con::errorf("InteriorInstance::changeMaterial failed: New material does not exist!");
      return;
   }

   // simple parsing through the interiors detail levels looking for the correct mapto.
   // break when we find the correct detail level to depend on.
   U32 level;
   S32 matIndex = -1;
   for( level = 0; level < object->getNumDetailLevels(); level++ )
   {
      matIndex = object->getDetailLevel(level)->mMaterialList->getMaterialNameList().find_next(String(mapTo));
      if (matIndex >= 0)
         break;
   }

   if (matIndex == -1)
   {
      Con::errorf("InteriorInstance::changeMaterial failed: Invalid mapTo name '%s'", mapTo);
      return;
   }

   // initilize server/client versions
   Interior *serverObj = object->getDetailLevel( level );

   InteriorInstance * instanceClientObj = dynamic_cast< InteriorInstance* > ( object->getClientObject() );
   Interior *clientObj = instanceClientObj ? instanceClientObj->getDetailLevel( level ) : NULL;

   if(serverObj)
   {
      // Lets remap the old material off, so as to let room for our current material room to claim its spot
      if( oldMat )
         oldMat->mMapTo = String("unmapped_mat");

      newMat->mMapTo = mapTo;

      // Map the material in the in the matmgr
      MATMGR->mapMaterial( mapTo, newMat->mMapTo );

      // Replace instances with the new material being traded in. Lets make sure that we only
      // target the specific targets per inst. This technically is only done here for interiors for 
      // safe keeping. The remapping that truly matters most (for on the fly changes) are done in the node lists
      delete serverObj->mMaterialList->mMatInstList[matIndex];
      serverObj->mMaterialList->mMatInstList[matIndex] = newMat->createMatInstance();

      // Finishing the safekeeping
      const GFXVertexFormat *flags = getGFXVertexFormat<GFXVertexPNTTB>();
      FeatureSet features = MATMGR->getDefaultFeatures();
      serverObj->mMaterialList->getMaterialInst(matIndex)->init( features, flags );

      if (clientObj)
      {
         delete clientObj->mMaterialList->mMatInstList[matIndex];
         clientObj->mMaterialList->mMatInstList[matIndex] = newMat->createMatInstance();
         clientObj->mMaterialList->getMaterialInst(matIndex)->init( features, flags );

         // These loops are referenced in interior.cpp's initMatInstances
         // Made a couple of alterations to tailor specifically towards one changing one instance
         for( U32 i=0; i<clientObj->getNumZones(); i++ )
         {
            for( U32 j=0; j<clientObj->mZoneRNList[i].renderNodeList.size(); j++ )
            {
               BaseMatInstance *matInst = clientObj->mZoneRNList[i].renderNodeList[j].matInst;
               Material* refMat = dynamic_cast<Material*>(matInst->getMaterial());

               if(refMat == oldMat)
               {
                  clientObj->mZoneRNList[i].renderNodeList[j].matInst = newMat->createMatInstance();
                  clientObj->mZoneRNList[i].renderNodeList[j].matInst->init(MATMGR->getDefaultFeatures(), getGFXVertexFormat<GFXVertexPNTTB>());

                  //if ( pMat )
                     //mHasTranslucentMaterials |= pMat->mTranslucent && !pMat->mTranslucentZWrite;
               }
            }
         }

         // Lets reset the clientObj settings in order to accomodate the new material
         clientObj->fillSurfaceTexMats();
         clientObj->createZoneVBs();
         clientObj->cloneMatInstances();
         clientObj->createReflectPlanes();
         clientObj->initMatInstances();
      }
   }
}

//-----------------------------------------------------------------------------

DefineEngineMethod( InteriorInstance, getModelFile, const char*, (),,
   "@brief Get the interior file name\n\n"
   

   "@return The name of the interior's model file in .DIF.\n\n"

   "@tsexample\n"
   "%interiorObject.getModelFile();\n"
   "@endtsexample\n\n")
{
	return object->getInteriorFileName();
}
