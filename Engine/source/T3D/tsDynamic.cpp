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
// TSDynamic is a copy of TSStatic with a few minor modifications.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "tsDynamic.h"

#include "core/resourceManager.h"
#include "core/stream/bitStream.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "scene/sceneObjectLightingPlugin.h"
#include "lighting/lightManager.h"
#include "math/mathIO.h"
#include "ts/tsShapeInstance.h"
#include "ts/tsMaterialList.h"
#include "console/consoleTypes.h"
#include "T3D/shapeBase.h"
#include "sim/netConnection.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxTransformSaver.h"
#include "ts/tsRenderState.h"
#include "collision/boxConvex.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsBody.h"
#include "T3D/physics/physicsCollision.h"
#include "materials/materialDefinition.h"
#include "materials/materialManager.h"
#include "materials/matInstance.h"
#include "materials/materialFeatureData.h"
#include "materials/materialFeatureTypes.h"
#include "console/engineAPI.h"

using namespace Torque;

extern bool gEditingMission;

IMPLEMENT_CO_NETOBJECT_V1(TSDynamic);

ConsoleDocClass( TSDynamic,
   "@brief A simple object derived from a 3D model file and placed within the game world.\n\n"

   "TSDynamic provides basic 3d shape rendering and is intended to be used as a base for"
   " classes that will provide motion.\n\n"

   "@ingroup gameObjects\n");

TSDynamic::TSDynamic()
{
   mNetFlags.set(Ghostable | ScopeAlways);

   mTypeMask |= StaticObjectType | StaticShapeObjectType;

   mShapeName        = "";
   mShapeInstance    = NULL;

   mPlayAmbient      = true;
   mAmbientThread    = NULL;

   mAllowPlayerStep = true;

   mConvexList = new Convex;

   mRenderNormalScalar = 0;
   mForceDetail = -1;

   mMeshCulling = false;
   mUseOriginSort = false;

   mPhysicsRep = NULL;

   mCollisionType = CollisionMesh;
   mDecalType = CollisionMesh;
}

TSDynamic::~TSDynamic()
{
   delete mConvexList;
   mConvexList = NULL;
}

void TSDynamic::initPersistFields()
{
   addGroup("Media");

      addField("shapeName",   TypeShapeFilename,  Offset( mShapeName, TSDynamic ),
         "%Path and filename of the model file (.DTS, .DAE) to use for this TSDynamic." );

      addProtectedField( "skin", TypeRealString, Offset( mAppliedSkinName, TSDynamic ), &_setFieldSkin, &_getFieldSkin,
      "@brief The skin applied to the shape.\n\n"

      "'Skinning' the shape effectively renames the material targets, allowing "
      "different materials to be used on different instances of the same model.\n\n"

      "Any material targets that start with the old skin name have that part "
      "of the name replaced with the new skin name. The initial old skin name is "
      "\"base\". For example, if a new skin of \"blue\" was applied to a model "
      "that had material targets <i>base_body</i> and <i>face</i>, the new targets "
      "would be <i>blue_body</i> and <i>face</i>. Note that <i>face</i> was not "
      "renamed since it did not start with the old skin name of \"base\".\n\n"

      "To support models that do not use the default \"base\" naming convention, "
      "you can also specify the part of the name to replace in the skin field "
      "itself. For example, if a model had a material target called <i>shapemat</i>, "
      "we could apply a new skin \"shape=blue\", and the material target would be "
      "renamed to <i>bluemat</i> (note \"shape\" has been replaced with \"blue\").\n\n"

      "Multiple skin updates can also be applied at the same time by separating "
      "them with a semicolon. For example: \"base=blue;face=happy_face\".\n\n"

      "Material targets are only renamed if an existing Material maps to that "
      "name, or if there is a diffuse texture in the model folder with the same "
      "name as the new target.\n\n" );

   endGroup("Media");

   addGroup("Rendering");

      addField( "playAmbient",   TypeBool,   Offset( mPlayAmbient, TSDynamic ),
         "Enables automatic playing of the animation sequence named \"ambient\" (if it exists) when the TSDynamic is loaded.");
      addField( "meshCulling",   TypeBool,   Offset( mMeshCulling, TSDynamic ), 
         "Enables detailed culling of meshes within the TSDynamic. Should only be used "
         "with large complex shapes like buildings which contain many submeshes." );
      addField( "originSort",    TypeBool,   Offset( mUseOriginSort, TSDynamic ), 
         "Enables translucent sorting of the TSDynamic by its origin instead of the bounds." );

   endGroup("Rendering");

   addGroup("Collision");

      addField( "collisionType",    TypeTSMeshType,   Offset( mCollisionType,   TSDynamic ),
         "The type of mesh data to use for collision queries." );
      addField( "decalType",        TypeTSMeshType,   Offset( mDecalType,   TSDynamic ),
         "The type of mesh data used to clip decal polygons against." );
      addField( "allowPlayerStep",  TypeBool,         Offset( mAllowPlayerStep, TSDynamic ), 
         "@brief Allow a Player to walk up sloping polygons in the TSDynamic (based on the collisionType).\n\n"
         "When set to false, the slightest bump will stop the player from walking on top of the object.\n");
   
   endGroup("Collision");

   addGroup("Debug");

      addField( "renderNormals", TypeF32, Offset( mRenderNormalScalar, TSDynamic ),
         "Debug rendering mode shows the normals for each point in the TSDynamic's mesh." );
      addField( "forceDetail",   TypeS32, Offset( mForceDetail, TSDynamic ),
         "Forces rendering to a particular detail level." );

   endGroup("Debug");

   Parent::initPersistFields();
}

bool TSDynamic::_setFieldSkin( void *object, const char *index, const char *data )
{
   TSDynamic *ts = static_cast<TSDynamic*>( object );
   if ( ts )
      ts->setSkinName( data );
   return false;
}

const char *TSDynamic::_getFieldSkin( void *object, const char *data )
{
   TSDynamic *ts = static_cast<TSDynamic*>( object );
   return ts ? ts->mSkinNameHandle.getString() : "";
}

void TSDynamic::inspectPostApply()
{
   // Apply any transformations set in the editor
   Parent::inspectPostApply();

   if(isServerObject()) 
   {
      setMaskBits(AdvancedStaticOptionsMask);
      prepCollision();
   }

   _updateShouldTick();
}

bool TSDynamic::onAdd()
{
   PROFILE_SCOPE(TSDynamic_onAdd);

   if ( !Parent::onAdd() )
      return false;

   // Setup the shape.
   if ( !_createShape() )
   {
      Con::errorf( "TSDynamic::onAdd() - Shape creation failed!" );
      return false;
   }

   setRenderTransform(mObjToWorld);

   // Register for the resource change signal.
   ResourceManager::get().getChangedSignal().notify( this, &TSDynamic::_onResourceChanged );

   addToScene();

   _updateShouldTick();

   return true;
}

bool TSDynamic::_createShape()
{
   // Cleanup before we create.
   mCollisionDetails.clear();
   mLOSDetails.clear();
   SAFE_DELETE( mPhysicsRep );
   SAFE_DELETE( mShapeInstance );
   mAmbientThread = NULL;
   mShape = NULL;

   if (!mShapeName || mShapeName[0] == '\0') 
   {
      Con::errorf( "TSDynamic::_createShape() - No shape name!" );
      return false;
   }

   mShapeHash = _StringTable::hashString(mShapeName);

   mShape = ResourceManager::get().load(mShapeName);
   if ( bool(mShape) == false )
   {
      Con::errorf( "TSDynamic::_createShape() - Unable to load shape: %s", mShapeName );
      return false;
   }

   if (  isClientObject() && 
         !mShape->preloadMaterialList(mShape.getPath()) && 
         NetConnection::filesWereDownloaded() )
      return false;

   mObjBox = mShape->bounds;
   resetWorldBox();

   mShapeInstance = new TSShapeInstance( mShape, isClientObject() );

   if( isGhost() )
   {
      // Reapply the current skin
      mAppliedSkinName = "";
      reSkin();
   }

   prepCollision();

   // Find the "ambient" animation if it exists
   S32 ambientSeq = mShape->findSequence("ambient");

   if ( ambientSeq > -1 && !mAmbientThread )
      mAmbientThread = mShapeInstance->addThread();

   if ( mAmbientThread )
      mShapeInstance->setSequence( mAmbientThread, ambientSeq, 0);

   return true;
}

void TSDynamic::prepCollision()
{
   // Let the client know that the collision was updated
   setMaskBits( UpdateCollisionMask );

   // Allow the ShapeInstance to prep its collision if it hasn't already
   if ( mShapeInstance )
      mShapeInstance->prepCollision();

   // Cleanup any old collision data
   mCollisionDetails.clear();
   mLOSDetails.clear();
   mConvexList->nukeList();

   if ( mCollisionType == CollisionMesh || mCollisionType == VisibleMesh )
      mShape->findColDetails( mCollisionType == VisibleMesh, &mCollisionDetails, &mLOSDetails );

   _updatePhysics();
}

void TSDynamic::_updatePhysics()
{
   SAFE_DELETE( mPhysicsRep );

   if ( !PHYSICSMGR || mCollisionType == None )
      return;

   PhysicsCollision *colShape = NULL;
   if ( mCollisionType == Bounds )
   {
      MatrixF offset( true );
      offset.setPosition( mShape->center );
      colShape = PHYSICSMGR->createCollision();
      colShape->addBox( getObjBox().getExtents() * 0.5f * mObjScale, offset );         
   }
   else
      colShape = mShape->buildColShape( mCollisionType == VisibleMesh, getScale() );

   if ( colShape )
   {
      PhysicsWorld *world = PHYSICSMGR->getWorld( isServerObject() ? "server" : "client" );
      mPhysicsRep = PHYSICSMGR->createBody();
      mPhysicsRep->init( colShape, 0, 0, this, world );
      mPhysicsRep->setTransform( getTransform() );
   }
}

void TSDynamic::onRemove()
{
   SAFE_DELETE( mPhysicsRep );

   mConvexList->nukeList();

   removeFromScene();

   // Remove the resource change signal.
   ResourceManager::get().getChangedSignal().remove( this, &TSDynamic::_onResourceChanged );

   delete mShapeInstance;
   mShapeInstance = NULL;

   mAmbientThread = NULL;

   Parent::onRemove();
}

void TSDynamic::_onResourceChanged( const Torque::Path &path )
{
   if ( path != Path( mShapeName ) )
      return;
   
   _createShape();
   _updateShouldTick();
}

void TSDynamic::setSkinName( const char *name )
{
   if ( !isGhost() )
   {
      if ( name[0] != '\0' )
      {
         // Use tags for better network performance
         // Should be a tag, but we'll convert to one if it isn't.
         if ( name[0] == StringTagPrefixByte )
            mSkinNameHandle = NetStringHandle( U32(dAtoi(name + 1)) );
         else
            mSkinNameHandle = NetStringHandle( name );
      }
      else
         mSkinNameHandle = NetStringHandle();

      setMaskBits( SkinMask );
   }
}

void TSDynamic::reSkin()
{
   if ( isGhost() && mShapeInstance && mSkinNameHandle.isValidString() )
   {
      Vector<String> skins;
      String(mSkinNameHandle.getString()).split( ";", skins );

      for (int i = 0; i < skins.size(); i++)
      {
         String oldSkin( mAppliedSkinName.c_str() );
         String newSkin( skins[i] );

         // Check if the skin handle contains an explicit "old" base string. This
         // allows all models to support skinning, even if they don't follow the 
         // "base_xxx" material naming convention.
         S32 split = newSkin.find( '=' );    // "old=new" format skin?
         if ( split != String::NPos )
         {
            oldSkin = newSkin.substr( 0, split );
            newSkin = newSkin.erase( 0, split+1 );
         }

         mShapeInstance->reSkin( newSkin, oldSkin );
         mAppliedSkinName = newSkin;
      }
   }
}

void TSDynamic::processTick( const Move *move )
{
   if ( isServerObject() && mPlayAmbient && mAmbientThread )
      mShapeInstance->advanceTime( TickSec, mAmbientThread );

   if ( isMounted() )
   {
      MatrixF mat( true );
      mMount.object->getMountTransform(mMount.node, mMount.xfm, &mat );
      setTransform( mat );
   }
}

void TSDynamic::interpolateTick( F32 delta )
{
}

void TSDynamic::advanceTime( F32 dt )
{
   if ( mPlayAmbient && mAmbientThread )
      mShapeInstance->advanceTime( dt, mAmbientThread );

   if ( isMounted() )
   {
      MatrixF mat( true );
      mMount.object->getRenderMountTransform( dt, mMount.node, mMount.xfm, &mat );
      setRenderTransform( mat );
   }
}

bool TSDynamic::_getShouldTick()
{
   return (mPlayAmbient && mAmbientThread) || isMounted();
}

void TSDynamic::_updateShouldTick()
{
   bool shouldTick = _getShouldTick();

   if ( isTicking() != shouldTick )
      setProcessTick( shouldTick );
}

void TSDynamic::prepRenderImage( SceneRenderState* state )
{
   if( !mShapeInstance )
      return;

   Point3F cameraOffset;
   getRenderTransform().getColumn(3,&cameraOffset);
   cameraOffset -= state->getDiffuseCameraPosition();
   F32 dist = cameraOffset.len();
   if (dist < 0.01f)
      dist = 0.01f;

   F32 invScale = (1.0f/getMax(getMax(mObjScale.x,mObjScale.y),mObjScale.z));   

   if ( mForceDetail == -1 )
      mShapeInstance->setDetailFromDistance( state, dist * invScale );
   else
      mShapeInstance->setCurrentDetail( mForceDetail );

   if ( mShapeInstance->getCurrentDetail() < 0 )
      return;

   GFXTransformSaver saver;
   
   // Set up our TS render state.
   TSRenderState rdata;
   rdata.setSceneState( state );
   rdata.setFadeOverride( 1.0f );
   rdata.setOriginSort( mUseOriginSort );

   // If we have submesh culling enabled then prepare
   // the object space frustum to pass to the shape.
   Frustum culler;
   if ( mMeshCulling )
   {
      culler = state->getCullingFrustum();
      MatrixF xfm( true );
      xfm.scale( Point3F::One / getScale() );
      xfm.mul( getRenderWorldTransform() );
      xfm.mul( culler.getTransform() );
      culler.setTransform( xfm );
      rdata.setCuller( &culler );
   }

   // We might have some forward lit materials
   // so pass down a query to gather lights.
   LightQuery query;
   query.init( getWorldSphere() );
   rdata.setLightQuery( &query );

   MatrixF mat = getRenderTransform();
   mat.scale( mObjScale );
   GFX->setWorldMatrix( mat );

   mShapeInstance->animate();
   mShapeInstance->render( rdata );

   if ( mRenderNormalScalar > 0 )
   {
      ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind( this, &TSDynamic::_renderNormals );
      ri->type = RenderPassManager::RIT_Editor;
      state->getRenderPass()->addInst( ri );
   }
}

void TSDynamic::_renderNormals( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat )
{
   PROFILE_SCOPE( TSDynamic_RenderNormals );

   GFXTransformSaver saver;

   MatrixF mat = getRenderTransform();
   mat.scale( mObjScale );
   GFX->multWorld( mat );

   S32 dl = mShapeInstance->getCurrentDetail();
   mShapeInstance->renderDebugNormals( mRenderNormalScalar, dl );
}

void TSDynamic::onScaleChanged()
{
   Parent::onScaleChanged();

   if ( mPhysicsRep )
   {
      // If the editor is enabled delay the scale operation
      // by a few milliseconds so that we're not rebuilding
      // during an active scale drag operation.
      if ( gEditingMission )
         mPhysicsRep->queueCallback( 500, Delegate<void()>( this, &TSDynamic::_updatePhysics ) );
      else
         _updatePhysics();
   }

   setMaskBits( ScaleMask );
}

void TSDynamic::setTransform(const MatrixF & mat)
{
   mObjToWorld = mWorldToObj = mat;
   mWorldToObj.affineInverse();

   // Update the world-space AABB.
   resetWorldBox();

   // If we're in a SceneManager, sync our scene state.
   if( mSceneManager != NULL )
      mSceneManager->notifyObjectDirty( this );

   if ( mPhysicsRep )
      mPhysicsRep->setTransform( mat );

   // Since this is a static it's render transform changes 1
   // to 1 with it's collision transform... no interpolation.
   setRenderTransform(mat);
}

U32 TSDynamic::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if ( stream->writeFlag( mask & TransformMask ) )  
      mathWrite( *stream, getTransform() );

   if ( stream->writeFlag( mask & ScaleMask ) )  
   {
      // Only write one bit if the scale is one.
      if ( stream->writeFlag( mObjScale != Point3F::One ) )
         mathWrite( *stream, mObjScale );   
   }

   if ( stream->writeFlag( mask & UpdateCollisionMask ) )
      stream->write( (U32)mCollisionType );

   if ( stream->writeFlag( mask & SkinMask ) )
      con->packNetStringHandleU( stream, mSkinNameHandle );

   if ( stream->writeFlag( mask & AdvancedStaticOptionsMask ) )  
   {
      stream->writeString( mShapeName );
      stream->write( (U32)mDecalType );

      stream->writeFlag( mAllowPlayerStep );
      stream->writeFlag( mMeshCulling );
      stream->writeFlag( mUseOriginSort );

      stream->write( mRenderNormalScalar );

      stream->write( mForceDetail );

      stream->writeFlag( mPlayAmbient );

      if ( mLightPlugin )
         retMask |= mLightPlugin->packUpdate(this, AdvancedStaticOptionsMask, con, mask, stream);
   }

   return retMask;
}

void TSDynamic::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   // TransformMask
   if ( stream->readFlag() ) 
   {
      MatrixF mat;
      mathRead( *stream, &mat );
      setTransform(mat);
      setRenderTransform(mat);
   }

   // ScaleMask
   Point3F scale;
   if ( stream->readFlag() ) 
   {
      if ( stream->readFlag() )
      {
         VectorF scale;
         mathRead( *stream, &scale );
         setScale( scale );
      }
      else
         setScale( Point3F::One );
   }

   if ( stream->readFlag() ) // UpdateCollisionMask
   {
      U32 collisionType = CollisionMesh;

      stream->read( &collisionType );

      // Handle it if we have changed CollisionType's
      if ( (MeshType)collisionType != mCollisionType )
      {
         mCollisionType = (MeshType)collisionType;

         if ( isProperlyAdded() && mShapeInstance )
            prepCollision();
      }
   }

   if (stream->readFlag())    // SkinMask
   {
      NetStringHandle skinDesiredNameHandle = con->unpackNetStringHandleU(stream);;
      if (mSkinNameHandle != skinDesiredNameHandle)
      {
         mSkinNameHandle = skinDesiredNameHandle;
         reSkin();
      }
   }

   // AdvancedStaticOptionsMask
   if ( stream->readFlag() ) 
   {
      mShapeName = stream->readSTString();

      stream->read( (U32*)&mDecalType );
      mAllowPlayerStep = stream->readFlag();
      mMeshCulling = stream->readFlag();   
      mUseOriginSort = stream->readFlag();

      stream->read( &mRenderNormalScalar );

      stream->read( &mForceDetail );

      mPlayAmbient = stream->readFlag();

      if ( mLightPlugin )
         mLightPlugin->unpackUpdate(this, con, stream);

      if ( isProperlyAdded() )
         _updateShouldTick();
   }
}

//----------------------------------------------------------------------------
bool TSDynamic::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
   if ( mCollisionType == None )
      return false;

   if ( !mShapeInstance )
      return false;

   if ( mCollisionType == Bounds )
   {
      F32 st, et, fst = 0.0f, fet = 1.0f;
      F32 *bmin = &mObjBox.minExtents.x;
      F32 *bmax = &mObjBox.maxExtents.x;
      F32 const *si = &start.x;
      F32 const *ei = &end.x;

      for ( U32 i = 0; i < 3; i++ )
      {
         if (*si < *ei) 
         {
            if ( *si > *bmax || *ei < *bmin )
               return false;
            F32 di = *ei - *si;
            st = ( *si < *bmin ) ? ( *bmin - *si ) / di : 0.0f;
            et = ( *ei > *bmax ) ? ( *bmax - *si ) / di : 1.0f;
         }
         else 
         {
            if ( *ei > *bmax || *si < *bmin )
               return false;
            F32 di = *ei - *si;
            st = ( *si > *bmax ) ? ( *bmax - *si ) / di : 0.0f;
            et = ( *ei < *bmin ) ? ( *bmin - *si ) / di : 1.0f;
         }
         if ( st > fst ) fst = st;
         if ( et < fet ) fet = et;
         if ( fet < fst )
            return false;
         bmin++; bmax++;
         si++; ei++;
      }

      info->normal = start - end;
      info->normal.normalizeSafe();
      getTransform().mulV( info->normal );

      info->t = fst;
      info->object = this;
      info->point.interpolate( start, end, fst );
      info->material = NULL;
      return true;
   }
   else
   {
      RayInfo shortest = *info;
      RayInfo localInfo;
      shortest.t = 1e8f;
      localInfo.generateTexCoord = info->generateTexCoord;

      for ( U32 i = 0; i < mLOSDetails.size(); i++ )
      {
         mShapeInstance->animate( mLOSDetails[i] );

         if ( mShapeInstance->castRayOpcode( mLOSDetails[i], start, end, &localInfo ) )
         {
            localInfo.object = this;

            if (localInfo.t < shortest.t)
               shortest = localInfo;
         }
      }

      if (shortest.object == this)
      {
         // Copy out the shortest time...
         *info = shortest;
         return true;
      }
   }

   return false;
}

bool TSDynamic::castRayRendered(const Point3F &start, const Point3F &end, RayInfo *info)
{
   if ( !mShapeInstance )
      return false;

   // Cast the ray against the currently visible detail
   RayInfo localInfo;
   bool res = mShapeInstance->castRayOpcode( mShapeInstance->getCurrentDetail(), start, end, &localInfo );

   if ( res )
   {
      *info = localInfo;
      info->object = this;
      return true;
   }

   return false;
}

bool TSDynamic::buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F &box, const SphereF &)
{
   if ( !mShapeInstance )
      return false;

   // This is safe to set even if we're not outputing 
   polyList->setTransform( &mObjToWorld, mObjScale );
   polyList->setObject( this );

   if ( context == PLC_Export )
   {
      // Use highest detail level
      S32 dl = 0;

      // Try to call on the client so we can export materials
      if ( isServerObject() && getClientObject() )
         dynamic_cast<TSDynamic*>(getClientObject())->mShapeInstance->buildPolyList( polyList, dl );
      else
          mShapeInstance->buildPolyList( polyList, dl );
   }
   else if ( context == PLC_Selection )
   {
      // Use the last rendered detail level
      S32 dl = mShapeInstance->getCurrentDetail();
      mShapeInstance->buildPolyListOpcode( dl, polyList, box );
   }
   else
   {
      // Figure out the mesh type we're looking for.
      MeshType meshType = ( context == PLC_Decal ) ? mDecalType : mCollisionType;

      if ( meshType == None )
         return false;
      else if ( meshType == Bounds )
         polyList->addBox( mObjBox );
      else if ( meshType == VisibleMesh )
          mShapeInstance->buildPolyList( polyList, 0 );
      else
      {
         // Everything else is done from the collision meshes
         // which may be built from either the visual mesh or
         // special collision geometry.
         for ( U32 i = 0; i < mCollisionDetails.size(); i++ )
            mShapeInstance->buildPolyListOpcode( mCollisionDetails[i], polyList, box );
      }
   }

   return true;
}

void TSDynamic::buildConvex(const Box3F& box, Convex* convex)
{
   if ( mCollisionType == None )
      return;

   if ( mShapeInstance == NULL )
      return;

   // These should really come out of a pool
   mConvexList->collectGarbage();

   if ( mCollisionType == Bounds )
   {
      // Just return a box convex for the entire shape...
      Convex* cc = 0;
      CollisionWorkingList& wl = convex->getWorkingList();
      for (CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext)
      {
         if (itr->mConvex->getType() == BoxConvexType &&
             itr->mConvex->getObject() == this)
         {
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
   else  // CollisionMesh || VisibleMesh
   {
      TSStaticPolysoupConvex::smCurObject = this;

      for (U32 i = 0; i < mCollisionDetails.size(); i++)
         mShapeInstance->buildConvexOpcode( mObjToWorld, mObjScale, mCollisionDetails[i], box, convex, mConvexList );

      TSStaticPolysoupConvex::smCurObject = NULL;
   }
}

void TSDynamic::onMount( SceneObject *obj, S32 node )
{
   Parent::onMount(obj, node);
   processAfter(obj);

   // Make sure we're in the process list if we need to be explicitly after another
   if ( mProcessLink.next == this )
      getProcessList()->addObject( this );

   _updateShouldTick();
}

void TSDynamic::onUnmount( SceneObject *obj, S32 node )
{
   Parent::onUnmount(obj, node);
   obj->clearProcessAfter();
   _updateShouldTick();
}

//------------------------------------------------------------------------
//These functions are duplicated in tsStatic, tsDynamic and shapeBase.
//They each function a little differently; but achieve the same purpose of gathering
//target names/counts without polluting simObject.

DefineEngineMethod( TSDynamic, getTargetName, const char*, ( S32 index ),(0),
   "Get the name of the indexed shape material.\n"
   "@param index index of the material to get (valid range is 0 - getTargetCount()-1).\n"
   "@return the name of the indexed material.\n"
   "@see getTargetCount()\n")
{
   TSDynamic *obj = dynamic_cast< TSDynamic* > ( object );
   if(obj)
   {
      // Try to use the client object (so we get the reskinned targets in the Material Editor)
      if ((TSDynamic*)obj->getClientObject())
         obj = (TSDynamic*)obj->getClientObject();

      return obj->getShapeInstance()->getTargetName(index);
   }

   return "";
}

DefineEngineMethod( TSDynamic, getTargetCount, S32,(),,
   "Get the number of materials in the shape.\n"
   "@return the number of materials in the shape.\n"
   "@see getTargetName()\n")
{
   TSDynamic *obj = dynamic_cast< TSDynamic* > ( object );
   if(obj)
   {
      // Try to use the client object (so we get the reskinned targets in the Material Editor)
      if ((TSDynamic*)obj->getClientObject())
         obj = (TSDynamic*)obj->getClientObject();

      return obj->getShapeInstance()->getTargetCount();
   }

   return -1;
}

// This method is able to change materials per map to with others. The material that is being replaced is being mapped to
// unmapped_mat as a part of this transition

DefineEngineMethod( TSDynamic, changeMaterial, void, ( const char* mapTo, Material* oldMat, Material* newMat ),("",NULL,NULL),
   "@brief Change one of the materials on the shape.\n\n"

   "This method changes materials per mapTo with others. The material that "
   "is being replaced is mapped to unmapped_mat as a part of this transition.\n"

   "@note Warning, right now this only sort of works. It doesn't do a live "
   "update like it should.\n"

   "@param mapTo the name of the material target to remap (from getTargetName)\n"
   "@param oldMat the old Material that was mapped \n"
   "@param newMat the new Material to map\n\n"

   "@tsexample\n"
      "// remap the first material in the shape\n"
      "%mapTo = %obj.getTargetName( 0 );\n"
      "%obj.changeMaterial( %mapTo, 0, MyMaterial );\n"
   "@endtsexample\n" )
{
   // if no valid new material, theres no reason for doing this
   if( !newMat )
   {
      Con::errorf("TSShape::changeMaterial failed: New material does not exist!");
      return;
   }

   // Check the mapTo name exists for this shape
   S32 matIndex = object->getShape()->materialList->getMaterialNameList().find_next(String(mapTo));
   if (matIndex < 0)
   {
      Con::errorf("TSShape::changeMaterial failed: Invalid mapTo name '%s'", mapTo);
      return;
   }

   // Lets remap the old material off, so as to let room for our current material room to claim its spot
   if( oldMat )
      oldMat->mMapTo = String("unmapped_mat");

   newMat->mMapTo = mapTo;

   // Map the material by name in the matmgr
   MATMGR->mapMaterial( mapTo, newMat->getName() );

   // Replace instances with the new material being traded in. Lets make sure that we only
   // target the specific targets per inst, this is actually doing more than we thought
   delete object->getShape()->materialList->mMatInstList[matIndex];
   object->getShape()->materialList->mMatInstList[matIndex] = newMat->createMatInstance();

   // Finish up preparing the material instances for rendering
   const GFXVertexFormat *flags = getGFXVertexFormat<GFXVertexPNTTB>();
   FeatureSet features = MATMGR->getDefaultFeatures();
   object->getShape()->materialList->getMaterialInst(matIndex)->init( features, flags );
}

DefineEngineMethod( TSDynamic, getModelFile, const char *, (),,
   "@brief Get the model filename used by this shape.\n\n"

   "@return the shape filename\n\n"
   "@tsexample\n"
      "// Acquire the model filename used on this shape.\n"
      "%modelFilename = %obj.getModelFile();\n"
   "@endtsexample\n"
   )
{
   return object->getShapeFileName();
}
