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

#include "T3D/examples/renderShapeExample.h"

#include "math/mathIO.h"
#include "sim/netConnection.h"
#include "scene/sceneRenderState.h"
#include "console/consoleTypes.h"
#include "core/resourceManager.h"
#include "core/stream/bitStream.h"
#include "gfx/gfxTransformSaver.h"
#include "renderInstance/renderPassManager.h"
#include "lighting/lightQuery.h"


IMPLEMENT_CO_NETOBJECT_V1(RenderShapeExample);

ConsoleDocClass( RenderShapeExample, 
   "@brief An example scene object which renders a DTS.\n\n"
   "This class implements a basic SceneObject that can exist in the world at a "
   "3D position and render itself. There are several valid ways to render an "
   "object in Torque. This class makes use of the 'TS' (three space) shape "
   "system. TS manages loading the various mesh formats supported by Torque as "
   "well was rendering those meshes (including LOD and animation...though this "
   "example doesn't include any animation over time).\n\n"
   "See the C++ code for implementation details.\n\n"
   "@ingroup Examples\n" );

//-----------------------------------------------------------------------------
// Object setup and teardown
//-----------------------------------------------------------------------------
RenderShapeExample::RenderShapeExample()
{
   // Flag this object so that it will always
   // be sent across the network to clients
   mNetFlags.set( Ghostable | ScopeAlways );

   // Set it as a "static" object.
   mTypeMask |= StaticObjectType | StaticShapeObjectType;

   // Make sure to initialize our TSShapeInstance to NULL
   mShapeInstance = NULL;
}

RenderShapeExample::~RenderShapeExample()
{
}

//-----------------------------------------------------------------------------
// Object Editing
//-----------------------------------------------------------------------------
void RenderShapeExample::initPersistFields()
{
   addGroup( "Rendering" );
   addField( "shapeFile",      TypeStringFilename, Offset( mShapeFile, RenderShapeExample ),
      "The path to the DTS shape file." );
   endGroup( "Rendering" );

   // SceneObject already handles exposing the transform
   Parent::initPersistFields();
}

void RenderShapeExample::inspectPostApply()
{
   Parent::inspectPostApply();

   // Flag the network mask to send the updates
   // to the client object
   setMaskBits( UpdateMask );
}

bool RenderShapeExample::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   // Set up a 1x1x1 bounding box
   mObjBox.set( Point3F( -0.5f, -0.5f, -0.5f ),
                Point3F(  0.5f,  0.5f,  0.5f ) );

   resetWorldBox();

   // Add this object to the scene
   addToScene();

   // Setup the shape.
   createShape();

   return true;
}

void RenderShapeExample::onRemove()
{
   // Remove this object from the scene
   removeFromScene();

   // Remove our TSShapeInstance
   if ( mShapeInstance )
      SAFE_DELETE( mShapeInstance );

   Parent::onRemove();
}

void RenderShapeExample::setTransform(const MatrixF & mat)
{
   // Let SceneObject handle all of the matrix manipulation
   Parent::setTransform( mat );

   // Dirty our network mask so that the new transform gets
   // transmitted to the client object
   setMaskBits( TransformMask );
}

U32 RenderShapeExample::packUpdate( NetConnection *conn, U32 mask, BitStream *stream )
{
   // Allow the Parent to get a crack at writing its info
   U32 retMask = Parent::packUpdate( conn, mask, stream );

   // Write our transform information
   if ( stream->writeFlag( mask & TransformMask ) )
   {
      mathWrite(*stream, getTransform());
      mathWrite(*stream, getScale());
   }

   // Write out any of the updated editable properties
   if ( stream->writeFlag( mask & UpdateMask ) )
   {
      stream->write( mShapeFile );

      // Allow the server object a chance to handle a new shape
      createShape();
   }

   return retMask;
}

void RenderShapeExample::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   // Let the Parent read any info it sent
   Parent::unpackUpdate(conn, stream);

   if ( stream->readFlag() )  // TransformMask
   {
      mathRead(*stream, &mObjToWorld);
      mathRead(*stream, &mObjScale);

      setTransform( mObjToWorld );
   }

   if ( stream->readFlag() )  // UpdateMask
   {
      stream->read( &mShapeFile );

      if ( isProperlyAdded() )
         createShape();
   }
}

//-----------------------------------------------------------------------------
// Object Rendering
//-----------------------------------------------------------------------------
void RenderShapeExample::createShape()
{
   if ( mShapeFile.isEmpty() )
      return;

   // If this is the same shape then no reason to update it
   if ( mShapeInstance && mShapeFile.equal( mShape.getPath().getFullPath(), String::NoCase ) )
      return;

   // Clean up our previous shape
   if ( mShapeInstance )
      SAFE_DELETE( mShapeInstance );
   mShape = NULL;

   // Attempt to get the resource from the ResourceManager
   mShape = ResourceManager::get().load( mShapeFile );

   if ( !mShape )
   {
      Con::errorf( "RenderShapeExample::createShape() - Unable to load shape: %s", mShapeFile.c_str() );
      return;
   }

   // Attempt to preload the Materials for this shape
   if ( isClientObject() && 
        !mShape->preloadMaterialList( mShape.getPath() ) && 
        NetConnection::filesWereDownloaded() )
   {
      mShape = NULL;
      return;
   }

   // Update the bounding box
   mObjBox = mShape->bounds;
   resetWorldBox();
   setRenderTransform(mObjToWorld);

   // Create the TSShapeInstance
   mShapeInstance = new TSShapeInstance( mShape, isClientObject() );
}

void RenderShapeExample::prepRenderImage( SceneRenderState *state )
{
   // Make sure we have a TSShapeInstance
   if ( !mShapeInstance )
      return;

   // Calculate the distance of this object from the camera
   Point3F cameraOffset;
   getRenderTransform().getColumn( 3, &cameraOffset );
   cameraOffset -= state->getDiffuseCameraPosition();
   F32 dist = cameraOffset.len();
   if ( dist < 0.01f )
      dist = 0.01f;

   // Set up the LOD for the shape
   F32 invScale = ( 1.0f / getMax( getMax( mObjScale.x, mObjScale.y ), mObjScale.z ) );

   mShapeInstance->setDetailFromDistance( state, dist * invScale );

   // Make sure we have a valid level of detail
   if ( mShapeInstance->getCurrentDetail() < 0 )
      return;

   // GFXTransformSaver is a handy helper class that restores
   // the current GFX matrices to their original values when
   // it goes out of scope at the end of the function
   GFXTransformSaver saver;

   // Set up our TS render state      
   TSRenderState rdata;
   rdata.setSceneState( state );
   rdata.setFadeOverride( 1.0f );

   // We might have some forward lit materials
   // so pass down a query to gather lights.
   LightQuery query;
   query.init( getWorldSphere() );
   rdata.setLightQuery( &query );

   // Set the world matrix to the objects render transform
   MatrixF mat = getRenderTransform();
   mat.scale( mObjScale );
   GFX->setWorldMatrix( mat );

   // Animate the the shape
   mShapeInstance->animate();

   // Allow the shape to submit the RenderInst(s) for itself
   mShapeInstance->render( rdata );
}