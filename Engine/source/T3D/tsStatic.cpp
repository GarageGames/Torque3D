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
#include "T3D/tsStatic.h"

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
#include "T3D/accumulationVolume.h"

using namespace Torque;

extern bool gEditingMission;

IMPLEMENT_CO_NETOBJECT_V1(TSStatic);

ConsoleDocClass( TSStatic,
   "@brief A static object derived from a 3D model file and placed within the game world.\n\n"

   "TSStatic is the most basic 3D shape in Torque.  Unlike StaticShape it doesn't make use of "
   "a datablock.  It derrives directly from SceneObject.  This makes TSStatic extremely light "
   "weight, which is why the Tools use this class when you want to drop in a DTS or DAE object.\n\n"

   "While a TSStatic doesn't provide any motion -- it stays were you initally put it -- it does allow for "
   "a single ambient animation sequence to play when the object is first added to the scene.\n\n"

   "@tsexample\n"
         "new TSStatic(Team1Base) {\n"
         "   shapeName = \"art/shapes/desertStructures/station01.dts\";\n"
         "   playAmbient = \"1\";\n"
         "   receiveSunLight = \"1\";\n"
         "   receiveLMLighting = \"1\";\n"
         "   useCustomAmbientLighting = \"0\";\n"
         "   customAmbientLighting = \"0 0 0 1\";\n"
         "   collisionType = \"Visible Mesh\";\n"
         "   decalType = \"Collision Mesh\";\n"
         "   allowPlayerStep = \"1\";\n"
         "   renderNormals = \"0\";\n"
         "   forceDetail = \"-1\";\n"
         "   position = \"315.18 -180.418 244.313\";\n"
         "   rotation = \"0 0 1 195.952\";\n"
         "   scale = \"1 1 1\";\n"
         "   isRenderEnabled = \"true\";\n"
         "   canSaveDynamicFields = \"1\";\n"
         "};\n"
   "@endtsexample\n"

   "@ingroup gameObjects\n"
);

TSStatic::TSStatic()
:
   cubeDescId( 0 ),
   reflectorDesc( NULL )
{
   mNetFlags.set(Ghostable | ScopeAlways);

   mTypeMask |= StaticObjectType | StaticShapeObjectType;

   mShapeName        = "";
   mShapeInstance    = NULL;

   mPlayAmbient      = true;
   mAmbientThread    = NULL;

   mAllowPlayerStep = false;

   mConvexList = new Convex;

   mRenderNormalScalar = 0;
   mForceDetail = -1;

   mMeshCulling = false;
   mUseOriginSort = false;

   mUseAlphaFade     = false;
   mAlphaFadeStart   = 100.0f;
   mAlphaFadeEnd     = 150.0f;
   mInvertAlphaFade  = false;
   mAlphaFade = 1.0f;
   mPhysicsRep = NULL;

   mCollisionType = CollisionMesh;
   mDecalType = CollisionMesh;
}

TSStatic::~TSStatic()
{
   delete mConvexList;
   mConvexList = NULL;
}

ImplementEnumType( TSMeshType,
   "Type of mesh data available in a shape.\n"
   "@ingroup gameObjects" )
   { TSStatic::None,          "None",           "No mesh data." },
   { TSStatic::Bounds,        "Bounds",         "Bounding box of the shape." },
   { TSStatic::CollisionMesh, "Collision Mesh", "Specifically desingated \"collision\" meshes." },
   { TSStatic::VisibleMesh,   "Visible Mesh",   "Rendered mesh polygons." },
EndImplementEnumType;


void TSStatic::initPersistFields()
{
   addGroup("Media");

      addField("shapeName",   TypeShapeFilename,  Offset( mShapeName, TSStatic ),
         "%Path and filename of the model file (.DTS, .DAE) to use for this TSStatic." );

      addProtectedField( "skin", TypeRealString, Offset( mAppliedSkinName, TSStatic ), &_setFieldSkin, &_getFieldSkin,
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

      addField( "playAmbient",   TypeBool,   Offset( mPlayAmbient, TSStatic ),
         "Enables automatic playing of the animation sequence named \"ambient\" (if it exists) when the TSStatic is loaded.");
      addField( "meshCulling",   TypeBool,   Offset( mMeshCulling, TSStatic ), 
         "Enables detailed culling of meshes within the TSStatic. Should only be used "
         "with large complex shapes like buildings which contain many submeshes." );
      addField( "originSort",    TypeBool,   Offset( mUseOriginSort, TSStatic ), 
         "Enables translucent sorting of the TSStatic by its origin instead of the bounds." );

   endGroup("Rendering");

   addGroup( "Reflection" );
      addField( "cubeReflectorDesc", TypeRealString, Offset( cubeDescName, TSStatic ), 
         "References a ReflectorDesc datablock that defines performance and quality properties for dynamic reflections.\n");
   endGroup( "Reflection" );

   addGroup("Collision");

      addField( "collisionType",    TypeTSMeshType,   Offset( mCollisionType,   TSStatic ),
         "The type of mesh data to use for collision queries." );
      addField( "decalType",        TypeTSMeshType,   Offset( mDecalType,   TSStatic ),
         "The type of mesh data used to clip decal polygons against." );
      addField( "allowPlayerStep",  TypeBool,         Offset( mAllowPlayerStep, TSStatic ), 
         "@brief Allow a Player to walk up sloping polygons in the TSStatic (based on the collisionType).\n\n"
         "When set to false, the slightest bump will stop the player from walking on top of the object.\n");
   
   endGroup("Collision");

   addGroup( "AlphaFade" );  
      addField( "alphaFadeEnable",   TypeBool,   Offset(mUseAlphaFade,    TSStatic), "Turn on/off Alpha Fade" );  
      addField( "alphaFadeStart",    TypeF32,    Offset(mAlphaFadeStart,  TSStatic), "Distance of start Alpha Fade" );  
      addField( "alphaFadeEnd",      TypeF32,    Offset(mAlphaFadeEnd,    TSStatic), "Distance of end Alpha Fade" );  
      addField( "alphaFadeInverse", TypeBool,    Offset(mInvertAlphaFade, TSStatic), "Invert Alpha Fade's Start & End Distance" );  
   endGroup( "AlphaFade" );

   addGroup("Debug");

      addField( "renderNormals", TypeF32, Offset( mRenderNormalScalar, TSStatic ),
         "Debug rendering mode shows the normals for each point in the TSStatic's mesh." );
      addField( "forceDetail",   TypeS32, Offset( mForceDetail, TSStatic ),
         "Forces rendering to a particular detail level." );

   endGroup("Debug");

   Parent::initPersistFields();
}

bool TSStatic::_setFieldSkin( void *object, const char *index, const char *data )
{
   TSStatic *ts = static_cast<TSStatic*>( object );
   if ( ts )
      ts->setSkinName( data );
   return false;
}

const char *TSStatic::_getFieldSkin( void *object, const char *data )
{
   TSStatic *ts = static_cast<TSStatic*>( object );
   return ts ? ts->mSkinNameHandle.getString() : "";
}

void TSStatic::inspectPostApply()
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

bool TSStatic::onAdd()
{
   PROFILE_SCOPE(TSStatic_onAdd);

   if ( isServerObject() )
   {
      // Handle the old "usePolysoup" field
      SimFieldDictionary* fieldDict = getFieldDictionary();

      if ( fieldDict )
      {
         StringTableEntry slotName = StringTable->insert( "usePolysoup" );

         SimFieldDictionary::Entry * entry = fieldDict->findDynamicField( slotName );

         if ( entry )
         {
            // Was "usePolysoup" set?
            bool usePolysoup = dAtob( entry->value );

            // "usePolysoup" maps to the new VisibleMesh type
            if ( usePolysoup )
               mCollisionType = VisibleMesh;

            // Remove the field in favor on the new "collisionType" field
            fieldDict->setFieldValue( slotName, "" );
         }
      }
   }

   if ( !Parent::onAdd() )
      return false;

   // Setup the shape.
   if ( !_createShape() )
   {
      Con::errorf( "TSStatic::onAdd() - Shape creation failed!" );
      return false;
   }

   setRenderTransform(mObjToWorld);

   // Register for the resource change signal.
   ResourceManager::get().getChangedSignal().notify( this, &TSStatic::_onResourceChanged );

   addToScene();

   if ( isClientObject() )
   {      
      mCubeReflector.unregisterReflector();

      if ( reflectorDesc )
         mCubeReflector.registerReflector( this, reflectorDesc );      
   }

   _updateShouldTick();

   // Accumulation and environment mapping
   if (isClientObject() && mShapeInstance)
   {
      AccumulationVolume::addObject(this);
   }

   return true;
}

bool TSStatic::_createShape()
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
      Con::errorf( "TSStatic::_createShape() - No shape name!" );
      return false;
   }

   mShapeHash = _StringTable::hashString(mShapeName);

   mShape = ResourceManager::get().load(mShapeName);
   if ( bool(mShape) == false )
   {
      Con::errorf( "TSStatic::_createShape() - Unable to load shape: %s", mShapeName );
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

   // Resolve CubeReflectorDesc.
   if ( cubeDescName.isNotEmpty() )
   {
      Sim::findObject( cubeDescName, reflectorDesc );
   }
   else if( cubeDescId > 0 )
   {
      Sim::findObject( cubeDescId, reflectorDesc );
   }

   return true;
}

void TSStatic::prepCollision()
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

void TSStatic::_updatePhysics()
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

void TSStatic::onRemove()
{
   SAFE_DELETE( mPhysicsRep );

   // Accumulation
   if ( isClientObject() && mShapeInstance )
   {
      if ( mShapeInstance->hasAccumulation() ) 
         AccumulationVolume::removeObject(this);
   }

   mConvexList->nukeList();

   removeFromScene();

   // Remove the resource change signal.
   ResourceManager::get().getChangedSignal().remove( this, &TSStatic::_onResourceChanged );

   delete mShapeInstance;
   mShapeInstance = NULL;

   mAmbientThread = NULL;
   if ( isClientObject() )
       mCubeReflector.unregisterReflector();

   Parent::onRemove();
}

void TSStatic::_onResourceChanged( const Torque::Path &path )
{
   if ( path != Path( mShapeName ) )
      return;
   
   _createShape();
   _updateShouldTick();
}

void TSStatic::setSkinName( const char *name )
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

void TSStatic::reSkin()
{
   if ( isGhost() && mShapeInstance && mSkinNameHandle.isValidString() )
   {
      Vector<String> skins;
      String(mSkinNameHandle.getString()).split( ";", skins );

      for (S32 i = 0; i < skins.size(); i++)
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

void TSStatic::processTick( const Move *move )
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

void TSStatic::interpolateTick( F32 delta )
{
}

void TSStatic::advanceTime( F32 dt )
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

void TSStatic::_updateShouldTick()
{
   bool shouldTick = (mPlayAmbient && mAmbientThread) || isMounted();

   if ( isTicking() != shouldTick )
      setProcessTick( shouldTick );
}

void TSStatic::prepRenderImage( SceneRenderState* state )
{
   if( !mShapeInstance )
      return;

   Point3F cameraOffset;
   getRenderTransform().getColumn(3,&cameraOffset);
   cameraOffset -= state->getDiffuseCameraPosition();
   F32 dist = cameraOffset.len();
   if (dist < 0.01f)
      dist = 0.01f;

   if (mUseAlphaFade)
   {
      mAlphaFade = 1.0f;
      if ((mAlphaFadeStart < mAlphaFadeEnd) && mAlphaFadeStart > 0.1f)
      {
         if (mInvertAlphaFade)
         {
            if (dist <= mAlphaFadeStart)
            {
               return;
            }
            if (dist < mAlphaFadeEnd)
            {
               mAlphaFade = ((dist - mAlphaFadeStart) / (mAlphaFadeEnd - mAlphaFadeStart));
            }
         }
         else
         {
            if (dist >= mAlphaFadeEnd)
            {
               return;
            }
            if (dist > mAlphaFadeStart)
            {
               mAlphaFade -= ((dist - mAlphaFadeStart) / (mAlphaFadeEnd - mAlphaFadeStart));
            }
         }
      }
   }

   F32 invScale = (1.0f/getMax(getMax(mObjScale.x,mObjScale.y),mObjScale.z));   

   // If we're currently rendering our own reflection we
   // don't want to render ourselves into it.
   if ( mCubeReflector.isRendering() )
      return;


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

   if ( mCubeReflector.isEnabled() )
      rdata.setCubemap( mCubeReflector.getCubemap() );

   // Acculumation
   rdata.setAccuTex(mAccuTex);

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

   if ( state->isDiffusePass() && mCubeReflector.isEnabled() && mCubeReflector.getOcclusionQuery() )
   {
       RenderPassManager *pass = state->getRenderPass();
       OccluderRenderInst *ri = pass->allocInst<OccluderRenderInst>();  
       
       ri->type = RenderPassManager::RIT_Occluder;
       ri->query = mCubeReflector.getOcclusionQuery();
       mObjToWorld.mulP( mObjBox.getCenter(), &ri->position );
       ri->scale.set( mObjBox.getExtents() );
       ri->orientation = pass->allocUniqueXform( mObjToWorld ); 
       ri->isSphere = false;
       state->getRenderPass()->addInst( ri );
   }

   mShapeInstance->animate();
   if(mShapeInstance)
   {
      if (mUseAlphaFade)
      {
         mShapeInstance->setAlphaAlways(mAlphaFade);
         S32 s = mShapeInstance->mMeshObjects.size();
         
         for(S32 x = 0; x < s; x++)
         {
            mShapeInstance->mMeshObjects[x].visible = mAlphaFade;
         }
      }
   }
   mShapeInstance->render( rdata );

   if ( mRenderNormalScalar > 0 )
   {
      ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind( this, &TSStatic::_renderNormals );
      ri->type = RenderPassManager::RIT_Editor;
      state->getRenderPass()->addInst( ri );
   }
}

void TSStatic::_renderNormals( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat )
{
   PROFILE_SCOPE( TSStatic_RenderNormals );

   GFXTransformSaver saver;

   MatrixF mat = getRenderTransform();
   mat.scale( mObjScale );
   GFX->multWorld( mat );

   S32 dl = mShapeInstance->getCurrentDetail();
   mShapeInstance->renderDebugNormals( mRenderNormalScalar, dl );
}

void TSStatic::onScaleChanged()
{
   Parent::onScaleChanged();

   if ( mPhysicsRep )
   {
      // If the editor is enabled delay the scale operation
      // by a few milliseconds so that we're not rebuilding
      // during an active scale drag operation.
      if ( gEditingMission )
         mPhysicsRep->queueCallback( 500, Delegate<void()>( this, &TSStatic::_updatePhysics ) );
      else
         _updatePhysics();
   }

   setMaskBits( ScaleMask );
}

void TSStatic::setTransform(const MatrixF & mat)
{
   Parent::setTransform(mat);
   if ( !isMounted() )
      setMaskBits( TransformMask );

   if ( mPhysicsRep )
      mPhysicsRep->setTransform( mat );

   // Accumulation
   if ( isClientObject() && mShapeInstance )
   {
      if ( mShapeInstance->hasAccumulation() ) 
         AccumulationVolume::updateObject(this);
   }

   // Since this is a static it's render transform changes 1
   // to 1 with it's collision transform... no interpolation.
   setRenderTransform(mat);
}

U32 TSStatic::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
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

   if (stream->writeFlag(mask & AdvancedStaticOptionsMask))
   {
      stream->writeString(mShapeName);
      stream->write((U32)mDecalType);

      stream->writeFlag(mAllowPlayerStep);
      stream->writeFlag(mMeshCulling);
      stream->writeFlag(mUseOriginSort);

      stream->write(mRenderNormalScalar);

      stream->write(mForceDetail);

      stream->writeFlag(mPlayAmbient);
   }

   if ( stream->writeFlag(mUseAlphaFade) )  
   {  
      stream->write(mAlphaFadeStart);  
      stream->write(mAlphaFadeEnd);  
      stream->write(mInvertAlphaFade);  
   } 

   if ( mLightPlugin )
      retMask |= mLightPlugin->packUpdate(this, AdvancedStaticOptionsMask, con, mask, stream);

   if( stream->writeFlag( reflectorDesc != NULL ) )
   {
      stream->writeRangedU32( reflectorDesc->getId(), DataBlockObjectIdFirst,  DataBlockObjectIdLast );
   }
   return retMask;
}

void TSStatic::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   if ( stream->readFlag() ) // TransformMask
   {
      MatrixF mat;
      mathRead( *stream, &mat );
      setTransform(mat);
      setRenderTransform(mat);
   }

   if ( stream->readFlag() ) // ScaleMask
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

   if (stream->readFlag()) // AdvancedStaticOptionsMask
   {
      mShapeName = stream->readSTString();

      stream->read((U32*)&mDecalType);

      mAllowPlayerStep = stream->readFlag();
      mMeshCulling = stream->readFlag();
      mUseOriginSort = stream->readFlag();

      stream->read(&mRenderNormalScalar);

      stream->read(&mForceDetail);
      mPlayAmbient = stream->readFlag();
   }

   mUseAlphaFade = stream->readFlag();  
   if (mUseAlphaFade)
   {
      stream->read(&mAlphaFadeStart);  
      stream->read(&mAlphaFadeEnd);  
      stream->read(&mInvertAlphaFade);  
   }

   if ( mLightPlugin )
   {
      mLightPlugin->unpackUpdate(this, con, stream);
   }

   if( stream->readFlag() )
   {
      cubeDescId = stream->readRangedU32( DataBlockObjectIdFirst, DataBlockObjectIdLast );
   }

   if ( isProperlyAdded() )
      _updateShouldTick();
}

//----------------------------------------------------------------------------
bool TSStatic::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
   if ( mCollisionType == None )
      return false;

   if ( !mShapeInstance )
      return false;

   if ( mCollisionType == Bounds )
   {
      F32 fst;
      if (!mObjBox.collideLine(start, end, &fst, &info->normal))
         return false;

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

bool TSStatic::castRayRendered(const Point3F &start, const Point3F &end, RayInfo *info)
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

bool TSStatic::buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F &box, const SphereF &)
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
         dynamic_cast<TSStatic*>(getClientObject())->mShapeInstance->buildPolyList( polyList, dl );
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

void TSStatic::buildConvex(const Box3F& box, Convex* convex)
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

SceneObject* TSStaticPolysoupConvex::smCurObject = NULL;

TSStaticPolysoupConvex::TSStaticPolysoupConvex()
:  box( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f ),
   normal( 0.0f, 0.0f, 0.0f, 0.0f ),
   idx( 0 ),
   mesh( NULL )
{
   mType = TSPolysoupConvexType;

   for ( U32 i = 0; i < 4; ++i )
   {
      verts[i].set( 0.0f, 0.0f, 0.0f );
   }
}

Point3F TSStaticPolysoupConvex::support(const VectorF& vec) const
{
   F32 bestDot = mDot( verts[0], vec );

   const Point3F *bestP = &verts[0];
   for(S32 i=1; i<4; i++)
   {
      F32 newD = mDot(verts[i], vec);
      if(newD > bestDot)
      {
         bestDot = newD;
         bestP = &verts[i];
      }
   }

   return *bestP;
}

Box3F TSStaticPolysoupConvex::getBoundingBox() const
{
   Box3F wbox = box;
   wbox.minExtents.convolve( mObject->getScale() );
   wbox.maxExtents.convolve( mObject->getScale() );
   mObject->getTransform().mul(wbox);
   return wbox;
}

Box3F TSStaticPolysoupConvex::getBoundingBox(const MatrixF& mat, const Point3F& scale) const
{
   AssertISV(false, "TSStaticPolysoupConvex::getBoundingBox(m,p) - Not implemented. -- XEA");
   return box;
}

void TSStaticPolysoupConvex::getPolyList(AbstractPolyList *list)
{
   // Transform the list into object space and set the pointer to the object
   MatrixF i( mObject->getTransform() );
   Point3F iS( mObject->getScale() );
   list->setTransform(&i, iS);
   list->setObject(mObject);

   // Add only the original collision triangle
   S32 base =  list->addPoint(verts[0]);
               list->addPoint(verts[2]);
               list->addPoint(verts[1]);

   list->begin(0, (U32)idx ^ (uintptr_t)mesh);
   list->vertex(base + 2);
   list->vertex(base + 1);
   list->vertex(base + 0);
   list->plane(base + 0, base + 1, base + 2);
   list->end();
}

void TSStaticPolysoupConvex::getFeatures(const MatrixF& mat,const VectorF& n, ConvexFeature* cf)
{
   cf->material = 0;
   cf->object = mObject;

   // For a tetrahedron this is pretty easy... first
   // convert everything into world space.
   Point3F tverts[4];
   mat.mulP(verts[0], &tverts[0]);
   mat.mulP(verts[1], &tverts[1]);
   mat.mulP(verts[2], &tverts[2]);
   mat.mulP(verts[3], &tverts[3]);

   // points...
   S32 firstVert = cf->mVertexList.size();
   cf->mVertexList.increment(); cf->mVertexList.last() = tverts[0];
   cf->mVertexList.increment(); cf->mVertexList.last() = tverts[1];
   cf->mVertexList.increment(); cf->mVertexList.last() = tverts[2];
   cf->mVertexList.increment(); cf->mVertexList.last() = tverts[3];

   //    edges...
   cf->mEdgeList.increment();
   cf->mEdgeList.last().vertex[0] = firstVert+0;
   cf->mEdgeList.last().vertex[1] = firstVert+1;

   cf->mEdgeList.increment();
   cf->mEdgeList.last().vertex[0] = firstVert+1;
   cf->mEdgeList.last().vertex[1] = firstVert+2;

   cf->mEdgeList.increment();
   cf->mEdgeList.last().vertex[0] = firstVert+2;
   cf->mEdgeList.last().vertex[1] = firstVert+0;

   cf->mEdgeList.increment();
   cf->mEdgeList.last().vertex[0] = firstVert+3;
   cf->mEdgeList.last().vertex[1] = firstVert+0;

   cf->mEdgeList.increment();
   cf->mEdgeList.last().vertex[0] = firstVert+3;
   cf->mEdgeList.last().vertex[1] = firstVert+1;

   cf->mEdgeList.increment();
   cf->mEdgeList.last().vertex[0] = firstVert+3;
   cf->mEdgeList.last().vertex[1] = firstVert+2;

   //    triangles...
   cf->mFaceList.increment();
   cf->mFaceList.last().normal = PlaneF(tverts[2], tverts[1], tverts[0]);
   cf->mFaceList.last().vertex[0] = firstVert+2;
   cf->mFaceList.last().vertex[1] = firstVert+1;
   cf->mFaceList.last().vertex[2] = firstVert+0;

   cf->mFaceList.increment();
   cf->mFaceList.last().normal = PlaneF(tverts[1], tverts[0], tverts[3]);
   cf->mFaceList.last().vertex[0] = firstVert+1;
   cf->mFaceList.last().vertex[1] = firstVert+0;
   cf->mFaceList.last().vertex[2] = firstVert+3;

   cf->mFaceList.increment();
   cf->mFaceList.last().normal = PlaneF(tverts[2], tverts[1], tverts[3]);
   cf->mFaceList.last().vertex[0] = firstVert+2;
   cf->mFaceList.last().vertex[1] = firstVert+1;
   cf->mFaceList.last().vertex[2] = firstVert+3;

   cf->mFaceList.increment();
   cf->mFaceList.last().normal = PlaneF(tverts[0], tverts[2], tverts[3]);
   cf->mFaceList.last().vertex[0] = firstVert+0;
   cf->mFaceList.last().vertex[1] = firstVert+2;
   cf->mFaceList.last().vertex[2] = firstVert+3;

   // All done!
}

void TSStatic::onMount( SceneObject *obj, S32 node )
{
   Parent::onMount(obj, node);
   _updateShouldTick();
}

void TSStatic::onUnmount( SceneObject *obj, S32 node )
{
   Parent::onUnmount( obj, node );
   setMaskBits( TransformMask );
   _updateShouldTick();
}

//------------------------------------------------------------------------
//These functions are duplicated in tsStatic and shapeBase.
//They each function a little differently; but achieve the same purpose of gathering
//target names/counts without polluting simObject.

DefineEngineMethod( TSStatic, getTargetName, const char*, ( S32 index ),(0),
   "Get the name of the indexed shape material.\n"
   "@param index index of the material to get (valid range is 0 - getTargetCount()-1).\n"
   "@return the name of the indexed material.\n"
   "@see getTargetCount()\n")
{
	TSStatic *obj = dynamic_cast< TSStatic* > ( object );
	if(obj)
	{
		// Try to use the client object (so we get the reskinned targets in the Material Editor)
		if ((TSStatic*)obj->getClientObject())
			obj = (TSStatic*)obj->getClientObject();

		return obj->getShapeInstance()->getTargetName(index);
	}

	return "";
}

DefineEngineMethod( TSStatic, getTargetCount, S32,(),,
   "Get the number of materials in the shape.\n"
   "@return the number of materials in the shape.\n"
   "@see getTargetName()\n")
{
	TSStatic *obj = dynamic_cast< TSStatic* > ( object );
	if(obj)
	{
		// Try to use the client object (so we get the reskinned targets in the Material Editor)
		if ((TSStatic*)obj->getClientObject())
			obj = (TSStatic*)obj->getClientObject();

		return obj->getShapeInstance()->getTargetCount();
	}

	return -1;
}

// This method is able to change materials per map to with others. The material that is being replaced is being mapped to
// unmapped_mat as a part of this transition

DefineEngineMethod( TSStatic, changeMaterial, void, ( const char* mapTo, Material* oldMat, Material* newMat ),("",NULL,NULL),
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

   TSMaterialList* shapeMaterialList = object->getShape()->materialList;

   // Check the mapTo name exists for this shape
   S32 matIndex = shapeMaterialList->getMaterialNameList().find_next(String(mapTo));
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
   delete shapeMaterialList->mMatInstList[matIndex];
   shapeMaterialList->mMatInstList[matIndex] = newMat->createMatInstance();

   // Finish up preparing the material instances for rendering
   const GFXVertexFormat *flags = getGFXVertexFormat<GFXVertexPNTTB>();
   FeatureSet features = MATMGR->getDefaultFeatures();
   shapeMaterialList->getMaterialInst(matIndex)->init(features, flags);
}

DefineEngineMethod( TSStatic, getModelFile, const char *, (),,
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