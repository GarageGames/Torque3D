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
#include "T3D/physics/physX/pxCloth.h"

#include "console/consoleTypes.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "renderInstance/renderPassManager.h"
#include "lighting/lightQuery.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physx/pxWorld.h"
#include "T3D/physics/physx/pxStream.h"
#include "T3D/physics/physx/pxCasts.h"
#include "gfx/gfxDrawUtil.h"
#include "math/mathIO.h"
#include "core/stream/bitStream.h"
#include "materials/materialManager.h"
#include "materials/baseMatInstance.h"


IMPLEMENT_CO_NETOBJECT_V1( PxCloth );

ConsoleDocClass( PxCloth,
   
   "@brief Rectangular patch of cloth simulated by PhysX.\n\n"  

   "PxCloth is affected by other objects in the simulation but does not itself "
   "affect others, it is essentially a visual effect. Eg, shooting at cloth will "
   "disturb it but will not explode the projectile.\n\n"

   "Be careful with the cloth size and resolution because it can easily become "
   "performance intensive to simulate. A single piece of cloth that is very "
   "large or high resolution is also much more expensive than multiple pieces "
   "that add up to the same number of verts.\n\n"

   "Note that most field docs have been copied from their PhysX counterpart.\n\n"
   
   "@ingroup Physics"
);

enum PxClothAttachment {};
DefineBitfieldType( PxClothAttachment );

ImplementBitfieldType( PxClothAttachment,
   "Soon to be deprecated\n"
   "@internal" )
   { 0, "Bottom Right" },
   { 1, "Bottom Left" },
   { 2, "Top Right" },
   { 3, "Top Left" },
   { 4, "Top Center" },
   { 5, "Bottom Center" },
   { 6, "Right Center" },
   { 7, "Left Center" },
   { 8, "Top Edge" },
   { 9, "Bottom Edge" },
   { 10, "Right Edge" },
   { 11, "Left Edge" }
EndImplementBitfieldType;


PxCloth::PxCloth()
 : mWorld( NULL ),
   mScene( NULL ),
   mMatInst( NULL )
{
   mVertexRenderBuffer = NULL; 
   mIndexRenderBuffer = NULL;

   mMaxVertices = 0;
   mMaxIndices = 0;
  
   mClothMesh = NULL;
   mCloth = NULL;

   mPatchVerts.set( 8, 8 );
   mPatchSize.set( 8.0f, 8.0f );

   mNetFlags.set( Ghostable | ScopeAlways );
   mTypeMask |= StaticObjectType | StaticShapeObjectType;

   mReceiveBuffers.setToDefault();

   mBendingEnabled = false;
   mDampingEnabled = false;
   mTriangleCollisionEnabled = false;
   mSelfCollisionEnabled = false;

   mDensity = 1.0f;
   mThickness = 0.1f;
   mFriction = 0.25f;
   mBendingStiffness = 0.5f;
   mDampingCoefficient = 0.25f;

   mAttachmentMask = 0;
}

PxCloth::~PxCloth()
{
}

bool PxCloth::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   // Cloth is only created on the client.
   if ( isClientObject() )
   {
      mWorld = dynamic_cast<PxWorld*>( PHYSICSMGR->getWorld( "client" ) );

      if ( !mWorld || !mWorld->getScene() )
      {
         Con::errorf( "PxCloth::onAdd() - PhysXWorld not initialized... cloth disabled!" );
         return true;
      }

      mScene = mWorld->getScene();

      mResetXfm = getTransform();

      _createClothPatch();      

      PhysicsPlugin::getPhysicsResetSignal().notify( this, &PxCloth::onPhysicsReset, 1053.0f );
   }

   // On the server we use the static update
   // to setup the bounds of the cloth.
   if ( isServerObject() )
      _updateStaticCloth();

   addToScene();
   
   // Also the server object never ticks.
   if ( isServerObject() )
      setProcessTick( false );

   return true;
}

void PxCloth::onRemove()
{   
   SAFE_DELETE( mMatInst );

   if ( isClientObject() )
   {
      _releaseCloth();
      _releaseMesh();

      PhysicsPlugin::getPhysicsResetSignal().remove( this, &PxCloth::onPhysicsReset );
   }

   removeFromScene();

   Parent::onRemove();
}

void PxCloth::onPhysicsReset( PhysicsResetEvent reset )
{
   // Store the reset transform for later use.
   if ( reset == PhysicsResetEvent_Store )
      mResetXfm = getTransform();

   // Recreate the cloth at the last reset position.
   _recreateCloth( mResetXfm );
}

void PxCloth::initPersistFields()
{
   Parent::initPersistFields();

   addField( "material", TypeMaterialName, Offset( mMaterialName, PxCloth ),
      "@brief Name of the material to render.\n\n" );

   addField( "samples", TypePoint2I, Offset( mPatchVerts, PxCloth ),
      "@brief The number of cloth vertices in width and length.\n\n"
      "At least two verts should be defined.\n\n");

   addField( "size", TypePoint2F, Offset( mPatchSize, PxCloth ),
      "@brief The width and height of the cloth.\n\n" );

   addField( "bending", TypeBool, Offset( mBendingEnabled, PxCloth ),
      "@brief Enables or disables bending resistance.\n\n"
      "Set the bending resistance through PxCloth::bendingStiffness." );

   addField( "damping", TypeBool, Offset( mDampingEnabled, PxCloth ),
      "@brief Enable/disable damping of internal velocities.\n\n" );

   addField( "triangleCollision", TypeBool, Offset( mTriangleCollisionEnabled, PxCloth ),
      "@brief Not supported in current release (according to PhysX docs).\n\n"
	   "Enables or disables collision detection of cloth triangles against the scene. "
	   "If not set, only collisions of cloth particles are detected. If set, "
      "collisions of cloth triangles are detected as well." );

   addField( "selfCollision", TypeBool, Offset( mSelfCollisionEnabled, PxCloth ),
      "@brief Enables or disables self-collision handling within a single piece of cloth.\n\n" );

   addField( "density", TypeF32, Offset( mDensity, PxCloth ),
      "@brief Density of the cloth (Mass per Area).\n\n" );

   addField( "thickness", TypeF32, Offset( mThickness, PxCloth ),
      "@brief Value representing how thick the cloth is.\n\n"
      "The thickness is usually a fraction of the overall extent of the cloth and "
	   "should not be set to a value greater than that. A good value is the maximal "
	   "distance between two adjacent cloth particles in their rest pose. Visual "
	   "artifacts or collision problems may appear if the thickness is too small.\n\n" );

   addField( "friction", TypeF32, Offset( mFriction, PxCloth ),
      "@brief Friction coefficient in the range 0 to 1.\n\n"
	   "Defines the damping of the velocities of cloth particles that are in contact." );

   addField( "bendingStiffness", TypeF32, Offset( mBendingStiffness, PxCloth ),
      "@brief Bending stiffness of the cloth in the range 0 to 1.\n\n" );
   
   addField( "dampingCoefficient", TypeF32, Offset( mDampingCoefficient, PxCloth ),
      "@brief Spring damping of the cloth in the range 0 to 1.\n\n" );

   addField( "attachments", TYPEID< PxClothAttachment >(), Offset( mAttachmentMask, PxCloth ),
      "@brief Optional way to specify cloth verts that will be attached to the world position "
      "it is created at.\n\n" );

   // Cloth doesn't support scale.
   removeField( "scale" );
}

void PxCloth::inspectPostApply()
{
   Parent::inspectPostApply();

   // Must have at least 2 verts.
   mPatchVerts.x = getMax( 2, mPatchVerts.x );
   mPatchVerts.y = getMax( 2, mPatchVerts.y );
   if ( isServerObject() )
      _updateStaticCloth();

   setMaskBits( TransformMask | MaterialMask | ClothMask );
}

U32 PxCloth::packUpdate( NetConnection *conn, U32 mask, BitStream *stream )
{
   U32 retMask = Parent::packUpdate( conn, mask, stream );

   if ( stream->writeFlag( mask & TransformMask ) )
      mathWrite( *stream, getTransform() );

   if ( stream->writeFlag( mask & MaterialMask ) )
      stream->write( mMaterialName );

   if ( stream->writeFlag( mask & ClothMask ) )
   {
      mathWrite( *stream, mPatchVerts );
      mathWrite( *stream, mPatchSize );

      stream->write( mAttachmentMask );

      stream->writeFlag( mBendingEnabled );
      stream->writeFlag( mDampingEnabled );
      stream->writeFlag( mTriangleCollisionEnabled );
      stream->writeFlag( mSelfCollisionEnabled );
      stream->write( mThickness );
      stream->write( mFriction );
      stream->write( mBendingStiffness );
      stream->write( mDampingCoefficient );

      stream->write( mDensity );
   }

   return retMask;
}

void PxCloth::unpackUpdate( NetConnection *conn, BitStream *stream )
{
   Parent::unpackUpdate( conn, stream );

   // TransformMask
   if ( stream->readFlag() )
   {
      MatrixF mat;
      mathRead( *stream, &mat );
      setTransform( mat );
   }

   // MaterialMask
   if ( stream->readFlag() )
   {
      stream->read( &mMaterialName );
      SAFE_DELETE( mMatInst );
   }

   // ClothMask
   if ( stream->readFlag() )
   {
      Point2I patchVerts;
      Point2F patchSize;
      mathRead( *stream, &patchVerts );
      mathRead( *stream, &patchSize );

      if (  patchVerts != mPatchVerts ||
            !patchSize.equal( mPatchSize ) )
      {
         mPatchVerts = patchVerts;
         mPatchSize = patchSize;
         _releaseMesh();
      }

      U32 attachMask;
      stream->read( &attachMask );
      if ( attachMask != mAttachmentMask )
      {
         mAttachmentMask = attachMask;
         _releaseCloth();
      }

      mBendingEnabled = stream->readFlag();
      mDampingEnabled = stream->readFlag();
      mTriangleCollisionEnabled = stream->readFlag();
      mSelfCollisionEnabled = stream->readFlag();
      stream->read( &mThickness );
      stream->read( &mFriction );
      stream->read( &mBendingStiffness );
      stream->read( &mDampingCoefficient );

      F32 density;
      stream->read( &density );
      if ( density != mDensity )
      {
         mDensity = density;
         _releaseCloth();
      }

      if (  isClientObject() && 
            isProperlyAdded() &&
            mWorld &&
            !mCloth )
      {
         _createClothPatch();
      }

      _updateClothProperties();
   }
}

void PxCloth::_recreateCloth( const MatrixF &transform )
{
   if ( !mWorld )
      return;

   mWorld->getPhysicsResults();

   Parent::setTransform( transform );

   _createClothPatch();
}

void PxCloth::setTransform( const MatrixF &mat )
{
   Parent::setTransform( mat );
   setMaskBits( TransformMask );

   // Only need to do this if we're on the server
   // or if we're not currently ticking physics.
   if ( !mWorld || !mWorld->isEnabled() )
      _updateStaticCloth();
}

void PxCloth::setScale( const VectorF &scale )
{
   // Cloth doesn't support scale as it has plenty
   // of complications... sharing meshes, thickness,
   // transform origin, etc.
   return;
}

void PxCloth::prepRenderImage( SceneRenderState *state )
{  
   if ( mIsVBDirty )
      _updateVBIB();

   // Recreate the material if we need to.
   if ( !mMatInst )
      _initMaterial();

   // If we don't have a material instance after the override then 
   // we can skip rendering all together.
   BaseMatInstance *matInst = state->getOverrideMaterial( mMatInst );
   if ( !matInst )
      return;

   MeshRenderInst *ri = state->getRenderPass()->allocInst<MeshRenderInst>();

	// If we need lights then set them up.
   if ( matInst->isForwardLit() )
   {
      LightQuery query;
      query.init( getWorldSphere() );
		query.getLights( ri->lights, 8 );
   }

   ri->projection = state->getRenderPass()->allocSharedXform(RenderPassManager::Projection);
   ri->objectToWorld = &MatrixF::Identity;

   ri->worldToCamera = state->getRenderPass()->allocSharedXform(RenderPassManager::View);   
   ri->type = RenderPassManager::RIT_Mesh;

   ri->primBuff = &mPrimBuffer;
   ri->vertBuff = &mVB;

   ri->matInst = matInst;
   ri->prim = state->getRenderPass()->allocPrim();
   ri->prim->type = GFXTriangleList;
   ri->prim->minIndex = 0;
   ri->prim->startIndex = 0;
   ri->prim->numPrimitives = mNumIndices / 3;

   ri->prim->startVertex = 0;
   ri->prim->numVertices = mNumVertices;

   ri->defaultKey = matInst->getStateHint();
   ri->defaultKey2 = (U32)ri->vertBuff;

   state->getRenderPass()->addInst( ri );
}

void PxCloth::_releaseMesh()
{
   if ( !mClothMesh )
      return;

   _releaseCloth();

   mWorld->releaseClothMesh( *mClothMesh );
   mClothMesh = NULL;

   delete [] mVertexRenderBuffer;
   mVertexRenderBuffer = NULL;
   delete [] mIndexRenderBuffer;
   mIndexRenderBuffer = NULL;
}

void PxCloth::_releaseCloth()
{
   if ( !mCloth )
      return;

   mWorld->releaseCloth( *mCloth );
   mCloth = NULL;
}

void PxCloth::_initClothMesh()
{
   // Make sure we can change the world.
   mWorld->releaseWriteLock();

   _releaseMesh();

   // Must have at least 2 verts.
   mPatchVerts.x = getMax( 2, mPatchVerts.x );
   mPatchVerts.y = getMax( 2, mPatchVerts.y );

   // Generate a uniform cloth patch, 
   // w and h are the width and height, 
   // d is the distance between vertices.  
   mNumVertices = mPatchVerts.x * mPatchVerts.y;
   mNumIndices = (mPatchVerts.x-1) * (mPatchVerts.y-1) * 2;

   NxClothMeshDesc desc;
   desc.numVertices = mNumVertices;    
   desc.numTriangles = mNumIndices;   
   desc.pointStrideBytes = sizeof(NxVec3);    
   desc.triangleStrideBytes = 3*sizeof(NxU32);    
   desc.points = (NxVec3*)dMalloc(sizeof(NxVec3)*desc.numVertices);    
   desc.triangles = (NxU32*)dMalloc(sizeof(NxU32)*desc.numTriangles*3);    
   desc.flags = 0;    

   U32 i,j;    
   NxVec3 *p = (NxVec3*)desc.points;    
   
   F32 patchWidth = mPatchSize.x / (F32)( mPatchVerts.x - 1 );
   F32 patchHeight = mPatchSize.y / (F32)( mPatchVerts.y - 1 );

   for (i = 0; i < mPatchVerts.y; i++) 
   {        
      for (j = 0; j < mPatchVerts.x; j++) 
      {            
         p->set( patchWidth * j, 0.0f, patchHeight * i );     
         p++;
      }    
   }

   NxU32 *id = (NxU32*)desc.triangles;    
   
   for (i = 0; i < mPatchVerts.y-1; i++) 
   {        
      for (j = 0; j < mPatchVerts.x-1; j++) 
      {            
         NxU32 i0 = i * mPatchVerts.x + j;            
         NxU32 i1 = i0 + 1;            
         NxU32 i2 = i0 + mPatchVerts.x;            
         NxU32 i3 = i2 + 1;            
         if ( (j+i) % 2 ) 
         {                
            *id++ = i0; 
            *id++ = i2; 
            *id++ = i1;                
            *id++ = i1; 
            *id++ = i2; 
            *id++ = i3;            
         }            
         else 
         {                
            *id++ = i0; 
            *id++ = i2; 
            *id++ = i3;                
            *id++ = i0; 
            *id++ = i3; 
            *id++ = i1;            
         }        
      }    
   }   
   
   NxCookingInterface *cooker = PxWorld::getCooking();
   cooker->NxInitCooking();

   // Ok... cook the mesh!
   NxCookingParams params;
   params.targetPlatform = PLATFORM_PC;
   params.skinWidth = 0.01f;
   params.hintCollisionSpeed = false;

   cooker->NxSetCookingParams( params );
  
   PxMemStream cooked;	
  
   if ( cooker->NxCookClothMesh( desc, cooked ) )
   {
      cooked.resetPosition();
      mClothMesh = gPhysicsSDK->createClothMesh( cooked );
   }
   
   cooker->NxCloseCooking();
   
   NxVec3 *ppoints = (NxVec3*)desc.points;
   NxU32 *triangs = (NxU32*)desc.triangles;

   dFree( ppoints );
   dFree( triangs );

   if ( mClothMesh )
      _initReceiveBuffers();
}

void PxCloth::_initReceiveBuffers()
{
   // here we setup the buffers through which the SDK returns the dynamic cloth data
   // we reserve more memory for vertices than the initial mesh takes
   // because tearing creates new vertices
   // the SDK only tears cloth as long as there is room in these buffers

   mMaxVertices = 3 * mNumVertices;
   mMaxIndices = 3 * mNumIndices;
   
   // Allocate Render Buffer for Vertices if it hasn't been done before
   mVertexRenderBuffer = new GFXVertexPNTT[mMaxVertices];
   mIndexRenderBuffer = new U16[mMaxIndices];

   mReceiveBuffers.verticesPosBegin = &(mVertexRenderBuffer[0].point);
   mReceiveBuffers.verticesNormalBegin = &(mVertexRenderBuffer[0].normal);
   mReceiveBuffers.verticesPosByteStride = sizeof(GFXVertexPNTT);
   mReceiveBuffers.verticesNormalByteStride = sizeof(GFXVertexPNTT);
   mReceiveBuffers.maxVertices = mMaxVertices;
   mReceiveBuffers.numVerticesPtr = &mNumVertices;

   // the number of triangles is constant, even if the cloth is torn
   mReceiveBuffers.indicesBegin = &mIndexRenderBuffer[0];
   mReceiveBuffers.indicesByteStride = sizeof(NxU16);
   mReceiveBuffers.maxIndices = mMaxIndices;
   mReceiveBuffers.numIndicesPtr = &mNumIndices;

   // Set up texture coords.

   F32 dx = 1.0f / (F32)(mPatchVerts.x-1);
   F32 dy = 1.0f / (F32)(mPatchVerts.y-1);

   F32 *coord = (F32*)&mVertexRenderBuffer[0].texCoord;
   for ( U32 i = 0; i < mPatchVerts.y; i++) 
   {
      for ( U32 j = 0; j < mPatchVerts.x; j++) 
      {
         coord[0] = j*dx;
         coord[1] = i*-dy;
         coord += sizeof( GFXVertexPNTT ) / sizeof( F32 );
      }
   }

   // the parent index information would be needed if we used textured cloth
   //mReceiveBuffers.parentIndicesBegin       = (U32*)malloc(sizeof(U32)*mMaxVertices);
   //mReceiveBuffers.parentIndicesByteStride  = sizeof(U32);
   //mReceiveBuffers.maxParentIndices         = mMaxVertices;
   //mReceiveBuffers.numParentIndicesPtr      = &mNumParentIndices;

   mMeshDirtyFlags = 0;
   mReceiveBuffers.dirtyBufferFlagsPtr = &mMeshDirtyFlags;

   // init the buffers in case we want to draw the mesh 
   // before the SDK as filled in the correct values

   mReceiveBuffers.flags |= NX_MDF_16_BIT_INDICES;
}

bool PxCloth::_createClothPatch()
{
   // Make sure we have a mesh.
   if ( !mClothMesh )
   {
      _initClothMesh();
      if ( !mClothMesh )
         return false;
   }

   // Make sure we can change the world.
   mWorld->releaseWriteLock();

   _releaseCloth();

   NxClothDesc desc;
   desc.globalPose.setRowMajor44( getTransform() );
   desc.thickness = mThickness;
   desc.density = mDensity;
   desc.bendingStiffness = mBendingStiffness;   
   desc.dampingCoefficient = mDampingCoefficient;
   desc.friction = mFriction;
   
   if ( mBendingEnabled )
      desc.flags |= NX_CLF_BENDING;   
   if ( mDampingEnabled )
      desc.flags |= NX_CLF_DAMPING;
   if ( mTriangleCollisionEnabled )
      desc.flags |= NX_CLF_TRIANGLE_COLLISION;
   if ( mSelfCollisionEnabled )
      desc.flags |= NX_CLF_SELFCOLLISION;

   desc.clothMesh = mClothMesh;    
   desc.meshData = mReceiveBuffers;

   if ( !desc.isValid() )
      return false;

   mCloth = mScene->createCloth( desc );
   mIsVBDirty = true;

   _updateStaticCloth();
   _setupAttachments();

   return true;
}

void PxCloth::_updateClothProperties()
{
   if ( !mCloth )
      return;

   mCloth->setThickness( mThickness );
   mCloth->setBendingStiffness( mBendingStiffness );
   mCloth->setDampingCoefficient( mDampingCoefficient );
   mCloth->setFriction( mFriction );

   NxU32 flags = NX_CLF_GRAVITY; // TODO: Expose this?
   if ( mBendingEnabled )
      flags |= NX_CLF_BENDING;   
   if ( mDampingEnabled )
      flags |= NX_CLF_DAMPING;
   if ( mTriangleCollisionEnabled )
      flags |= NX_CLF_TRIANGLE_COLLISION;
   if ( mSelfCollisionEnabled )
      flags |= NX_CLF_SELFCOLLISION;
   mCloth->setFlags( flags );
}

void PxCloth::_initMaterial()
{
   SAFE_DELETE( mMatInst );

   Material *material = NULL;
   if (mMaterialName.isNotEmpty() )
      Sim::findObject( mMaterialName, material );

   if ( material )
      mMatInst = material->createMatInstance();
   else
      mMatInst = MATMGR->createMatInstance( "WarningMaterial" );

   GFXStateBlockDesc desc;
   desc.setCullMode( GFXCullNone );
   mMatInst->addStateBlockDesc( desc );

   mMatInst->init( MATMGR->getDefaultFeatures(), getGFXVertexFormat<GFXVertexPNTT>() );
}

void PxCloth::_updateVBIB()
{
   PROFILE_SCOPE( PxCloth_UpdateVBIB );

   mIsVBDirty = false;

   // Don't set the VB if the vertex count is the same!
   if ( mVB.isNull() || mVB->mNumVerts < mNumVertices )
      mVB.set( GFX, mNumVertices, GFXBufferTypeDynamic );

   GFXVertexPNTT *vert = mVertexRenderBuffer;
   GFXVertexPNTT *secondVert = NULL;

   for ( U32 i = 0; i < mNumVertices; i++ )
   {
      if ( i % (U32)mPatchSize.x == 0 && i != 0 )
      {
         secondVert = vert;
         secondVert--;
         vert->tangent = -(vert->point - secondVert->point);
      }
      else
      {      
         secondVert = vert;
         secondVert++;
         vert->tangent = vert->point - secondVert->point;
      }

      vert->tangent.normalize();
      vert++;
   }

   GFXVertexPNTT *vpPtr = mVB.lock();
   dMemcpy( vpPtr, mVertexRenderBuffer, sizeof( GFXVertexPNTT ) * mNumVertices );
   mVB.unlock();

   if ( mPrimBuffer.isNull() || mPrimBuffer->mIndexCount < mNumIndices )
      mPrimBuffer.set( GFX, mNumIndices, 0, GFXBufferTypeDynamic );

   U16 *pbPtr;
   mPrimBuffer.lock( &pbPtr );
   dMemcpy( pbPtr, mIndexRenderBuffer, sizeof( U16 ) * mNumIndices );
   mPrimBuffer.unlock();
}

void PxCloth::_updateStaticCloth()
{
   // Setup the unsimulated world bounds.
   mObjBox.set(   0, mThickness * -0.5f, 0, 
                  mPatchSize.x, mThickness * 0.5f, mPatchSize.y );
   resetWorldBox();

   // If we don't have render buffers then we're done.
   if ( !mVertexRenderBuffer || !mIndexRenderBuffer )
      return;

   // Make sure the VBs are updated.
   mIsVBDirty = true;

   F32 patchWidth = mPatchSize.x / (F32)(mPatchVerts.x-1);
   F32 patchHeight = mPatchSize.y / (F32)(mPatchVerts.y-1);

   Point3F normal( 0, 1, 0 );
   getTransform().mulV( normal );

   GFXVertexPNTT *vert = mVertexRenderBuffer;

   for (U32 y = 0; y < mPatchVerts.y; y++) 
   {        
      for (U32 x = 0; x < mPatchVerts.x; x++) 
      {            
         vert->point.set( patchWidth * x, 0.0f, patchHeight * y );
         getTransform().mulP( vert->point );
         vert->normal = normal;
         vert++;
      }
   }

   U16 *index = mIndexRenderBuffer;
   mNumIndices = (mPatchVerts.x-1) * (mPatchVerts.y-1) * 6;
   U16 yOffset = mPatchVerts.x;

   for (U32 y = 0; y < mPatchVerts.y-1; y++) 
   {   
      for (U32 x = 0; x < mPatchVerts.x-1; x++) 
      {       
         U16 base = x + ( yOffset * y );

         index[0] = base;
         index[1] = base + 1;
         index[2] = base + 1 + yOffset;

         index[3] = base + 1 + yOffset;
         index[4] = base + yOffset;
         index[5] = base;

         index += 6;
      }
   }
}

void PxCloth::processTick( const Move *move )
{
   // Make sure the cloth is created.
   if ( !mCloth )
      return;

   // TODO: Remove this hack!
   const bool enableWind = Con::getBoolVariable( "$PxCloth::enableWind", false );

   if ( enableWind )
   {
      NxVec3 windVec(   25.0f + NxMath::rand(-5.0f, 5.0f),
			                     NxMath::rand(-5.0f, 5.0f),
			                     NxMath::rand(-5.0f, 5.0f) );

      mCloth->setWindAcceleration( windVec );

      // Wake the cloth!
      mCloth->wakeUp();
   }
   else
      mCloth->setWindAcceleration( NxVec3( 0, 0, 0 ) );

   // Update bounds.
   if ( mWorld->getEnabled() )
   {
      NxBounds3 box;
      mCloth->getWorldBounds( box ); 

      Point3F min = pxCast<Point3F>( box.min );
      Point3F max = pxCast<Point3F>( box.max );

      mWorldBox.set( min, max );
      mObjBox = mWorldBox;

      getWorldTransform().mul( mObjBox );
   }
   else
   {
      mObjBox.set(   0, mThickness * -0.5f, 0, 
                     mPatchSize.x, mThickness * 0.5f, mPatchSize.y );
   }

   resetWorldBox();

   // Update the VB on the next render.
   mIsVBDirty = true;
}

void PxCloth::interpolateTick( F32 delta )
{
   // Nothing to do for now!
}

bool PxCloth::onNewDataBlock( GameBaseData *dptr, bool reload )
{
   return false;
}

void PxCloth::_setupAttachments()
{
   if ( !mCloth || !mWorld )
      return;

   // Set up attachments
   // Bottom right = bit 0
   // Bottom left = bit 1
   // Top right = bit 2
   // Top left = bit 3

   if ( mAttachmentMask & BIT( 0 ) )
      mCloth->attachVertexToGlobalPosition( 0, mCloth->getPosition( 0 ) );
   if ( mAttachmentMask & BIT( 1 ) )
      mCloth->attachVertexToGlobalPosition( mPatchVerts.x-1, mCloth->getPosition( mPatchVerts.x-1 ) );   
   if ( mAttachmentMask & BIT( 2 ) )
      mCloth->attachVertexToGlobalPosition( mPatchVerts.x * mPatchVerts.y - mPatchVerts.x, mCloth->getPosition( mPatchVerts.x * mPatchVerts.y - mPatchVerts.x ) );
   if ( mAttachmentMask & BIT( 3 ) )
      mCloth->attachVertexToGlobalPosition( mPatchVerts.x * mPatchVerts.y - 1, mCloth->getPosition( mPatchVerts.x * mPatchVerts.y - 1 ) );   
   if ( mAttachmentMask & BIT( 4 ) )
      mCloth->attachVertexToGlobalPosition( mPatchVerts.x * mPatchVerts.y - (mPatchVerts.x/2), mCloth->getPosition( mPatchVerts.x * mPatchVerts.y - (mPatchVerts.x/2) ) );
   if ( mAttachmentMask & BIT( 5 ) )
      mCloth->attachVertexToGlobalPosition( (mPatchVerts.x/2), mCloth->getPosition( (mPatchVerts.x/2) ) );
   if ( mAttachmentMask & BIT( 6 ) )
      mCloth->attachVertexToGlobalPosition( mPatchVerts.x * (mPatchVerts.y/2), mCloth->getPosition( mPatchVerts.x * (mPatchVerts.y/2) ) );
   if ( mAttachmentMask & BIT( 7 ) )
      mCloth->attachVertexToGlobalPosition( mPatchVerts.x * (mPatchVerts.y/2) + (mPatchVerts.x-1), mCloth->getPosition( mPatchVerts.x * (mPatchVerts.y/2) + (mPatchVerts.x-1) ) );
   
   if ( mAttachmentMask & BIT( 8 ) )
      for ( U32 i = mPatchVerts.x * mPatchVerts.y - mPatchVerts.x; i < mPatchVerts.x * mPatchVerts.y; i++ )
         mCloth->attachVertexToGlobalPosition( i, mCloth->getPosition( i ) );
   
   if ( mAttachmentMask & BIT( 9 ) )
      for ( U32 i = 0; i < mPatchVerts.x; i++ )
         mCloth->attachVertexToGlobalPosition( i, mCloth->getPosition( i ) );

   if ( mAttachmentMask & BIT( 10 ) )
      for ( U32 i = 0; i < mPatchVerts.x * mPatchVerts.y; i+=mPatchVerts.x )
         mCloth->attachVertexToGlobalPosition( i, mCloth->getPosition( i ) );

   if ( mAttachmentMask & BIT( 11 ) )
      for ( U32 i = mPatchVerts.x-1; i < mPatchVerts.x * mPatchVerts.y; i+=mPatchVerts.x )
         mCloth->attachVertexToGlobalPosition( i, mCloth->getPosition( i ) );
}