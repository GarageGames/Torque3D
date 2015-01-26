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
#include "scene/sceneRenderState.h"

#include "renderInstance/renderPassManager.h"
#include "math/util/matrixSet.h"
#include "torqueConfig.h"

//<!-- Scene Culling --!>
#include "console/engineAPI.h"
#include "T3D/gameBase/gameConnection.h"
#include "terrain/terrData.h"

namespace CameraAndFOV
	{
	
	extern   F32 mFarClippingDistance  ;
	
	} // namespace {}
//<!-- Scene Culling --!>
//-----------------------------------------------------------------------------

SceneRenderState::SceneRenderState( SceneManager* sceneManager,
                                    ScenePassType passType,
                                    const SceneCameraState& view,
                                    RenderPassManager* renderPass /* = NULL */,
                                    bool usePostEffects /* = true */ )
   :  mSceneManager( sceneManager ),
      mCullingState( sceneManager, view ),
      mRenderPass( renderPass ? renderPass : sceneManager->getDefaultRenderPass() ),
      mScenePassType( passType ),
      mRenderNonLightmappedMeshes( true ),
      mRenderLightmappedMeshes( true ),
      mUsePostEffects( usePostEffects ),
      mDisableAdvancedLightingBins( false ),
      mRenderArea( view.getFrustum().getBounds() ),
      mAmbientLightColor( sceneManager->getAmbientLightColor() ),
      mSceneRenderStyle( SRS_Standard ),
      mRenderField( 0 )
//<!-- Scene Culling --!>
	  ,fOV(0.0f),
	  yAdd(0.0f),
	  div(0.0f),
	  xAdd(0.0f),
	  posf(0.0f),
	  cameraObject( NULL )
//<!-- Scene Culling --!>
{
   // Setup the default parameters for the screen metrics methods.
   mDiffuseCameraTransform = view.getViewWorldMatrix();

   // The vector eye is the camera vector with its 
   // length normalized to 1 / zFar.
   getCameraTransform().getColumn( 1, &mVectorEye );
   mVectorEye.normalize( 1.0f / getFarPlane() );

   // TODO: What about ortho modes?  Is near plane ok
   // or do i need to remove it... maybe ortho has a near
   // plane of 1 and it just works out?

   const Frustum& frustum = view.getFrustum();
   const RectI& viewport = view.getViewport();

   mWorldToScreenScale.set(   ( frustum.getNearDist() * viewport.extent.x ) / ( frustum.getNearRight() - frustum.getNearLeft() ),
                              ( frustum.getNearDist() * viewport.extent.y ) / ( frustum.getNearTop() - frustum.getNearBottom() ) );

   // Assign shared matrix data to the render pass.

   mRenderPass->assignSharedXform( RenderPassManager::View, view.getWorldViewMatrix() );
   mRenderPass->assignSharedXform( RenderPassManager::Projection, view.getProjectionMatrix() );
}

//-----------------------------------------------------------------------------

SceneRenderState::~SceneRenderState()
{
}

//-----------------------------------------------------------------------------

const MatrixF& SceneRenderState::getWorldViewMatrix() const
{
   return getRenderPass()->getMatrixSet().getWorldToCamera();
}

//-----------------------------------------------------------------------------

const MatrixF& SceneRenderState::getProjectionMatrix() const
{
   return getRenderPass()->getMatrixSet().getCameraToScreen();
}

//-----------------------------------------------------------------------------
//<!-- Scene Culling --!>
   //Box used for temporarily storing the object to compare's bounding box.
   //This object is passed into a helper function "BoxToPlane" for using the
   // half space test on the 6 planes of the view frustum
   Box3F bounds;

   //Camera vectors for easy use.
   //The camera's up, right vector, and forward vector
   Point3F up, right;

   //Near & Far Plane distances from the camera
   F32 nearDist, farDist;

   //The four corners of the near distance plane
   //These corners are used for generating the 4 side planes
   Point3F pTR, pTL, pBR, pBL; 

   //This variable will be used as the camera's position for generating the view frustum sides.
   //It will also be used as the center of the near plane for finding the corners first.
   Point3F pCP; 
//<!-- Scene Culling --!>
	  
void SceneRenderState::renderObjects( SceneObject** objects, U32 numObjects )
{
   // Let the objects batch their stuff.

   PROFILE_START( SceneRenderState_prepRenderImages );
   // Make sure it's a client render

//<!-- Scene Culling --!>
   if(this->mSceneManager->GetIsClient())
   {
      //Get the connection for camera fov
      if (!connection.isValid())
         connection = GameConnection::getConnectionToServer();
      if( !connection )
         return;			

      //If this is the first render of the gameconnection then we want to 
      //prepRender the whole zone to load textures and stuff.
      //After we do the whole zone we then only want to do what is
      //in our culled area.
      if (!connection->didFirstRender)
      {
         for( U32 i = 0; i < numObjects;  i++ )
            {
            SceneObject* object = objects[ i ];
            object->prepRenderImage( this );
            }
      connection->didFirstRender=true;
	   }
	   else
	   {
         cameraObject = connection->getCameraObject();
         //Variables needed for culling
         fOV = cameraObject->getCameraFov();
         fOV = fOV < 90 ? 90.0f : fOV +=30;
        
         //Near and far plane distances
         nearDist = this->getNearPlane(); 
         farDist =  this->getFarPlane();

         //If the override > 0  and the fardist is less than max, then use it.
         if ( CameraAndFOV::mFarClippingDistance > nearDist && CameraAndFOV::mFarClippingDistance < farDist )
            farDist = CameraAndFOV::mFarClippingDistance;
	
         if (!mCanvas.isValid())
            mCanvas =static_cast<GuiCanvas*>( Sim::findObject("Canvas"));//Static cast should be ok, since the canvas is somewhat consistent.

         //Amount to increment to determine corners of near frustum
         //These are based on the resolution of the screen and the distance of
         // the near clipping plane.
      
         //Equation for getting Near Plane dimensions
         yAdd = tan( fOV / 2 ) * nearDist;
         const GFXVideoMode gmode = mCanvas->getPlatformWindow()->getVideoMode();
         div = (F32)gmode.resolution.x / (F32)gmode.resolution.y;
         xAdd = yAdd * div;

         //Far Plane generation
         cameraObject->getCameraTransform(&posf, &mTrans);
         pView = mTrans.getForwardVector();
         pPos = mTrans.getPosition();
 
		   //Far Plane generation
         pCP = pPos + ( pView ) * farDist;
         plFar.set( pCP , ( -1.0f * pView ) );
 
		   //Near Plane generation
         pCP = pPos + pView * nearDist;
         plNear.set ( pCP , pView );
 
         //Getting the camera's orientation vectors
         right = mTrans.getRightVector();
         up = mTrans.getUpVector();

         //4 corners of near Plane
         pTR = pCP + (right * xAdd) + (up * yAdd);
         pTL = pCP - (right * xAdd) + (up * yAdd);
         pBR = pCP + (right * xAdd) - (up * yAdd);
         pBL = pCP - (right * xAdd) - (up * yAdd);
 
         //Finally set this to the camera position
         pCP = pPos;

         //Generate the side, top, and bottom planes
         plLeft.set    ( pCP , pTL , pBL );
         plTop.set     ( pCP , pTR , pTL );
         plRight.set   ( pCP,  pBR , pTR );
         plBottom.set  ( pCP,  pBL , pBR );

         F32 chkVal = -(farDist/2);
         for (int i = 0; i < numObjects; i++)
         {
            SceneObject* object = objects[ i ];
            //Decal Manager's getWorldBox returns something flaky.
            //So let's not cull decals for now.
            //Easiest way is to get the ID, if the ID is zero (DecalManager)
            //then don't check bounds.
            if ((S32)object->getId() !=0 )
            {
               //Always add Terrainblocks.
               if (object->getTypeMask() & TerrainObjectType)
               {
                  object->prepRenderImage( this );
                  continue;
               }
               bounds = object->getWorldBox();
		 
               if ((F32) plNear.whichSide(bounds) < chkVal) continue;
               if ((F32) plFar.whichSide(bounds) < 0.0f) continue;
               if ((F32) plLeft.whichSide(bounds) < 0.0f) continue;
               if ((F32) plRight.whichSide(bounds) < 0.0f) continue;
               if ((F32) plBottom.whichSide(bounds) < chkVal) continue;
               if ((F32) plTop.whichSide(bounds) < chkVal) continue;
            }
		      //Add the object if it is within the view
		      object->prepRenderImage( this );
         }
      }
   }
   else
   {
//<!-- Scene Culling --!>
      for( U32 i = 0; i < numObjects;  i++ )
      {
         SceneObject* object = objects[ i ];
         object->prepRenderImage( this );
      }
//<!-- Scene Culling --!>
   }
//<!-- Scene Culling --!>
   PROFILE_END();
   // Render what the objects have batched.

   getRenderPass()->renderPass( this );
}

//<!-- Scene Culling --!>
DefineConsoleFunction( setFarClippingDistance, void, ( F32 dist ),,
   "Sets the clients far clipping.\n"
   )
{
   CameraAndFOV::mFarClippingDistance=dist;
}

DefineConsoleFunction( getFarClippingDistance, F32,() ,,
   "Gets the clients far clipping.\n"
   )
{
   return CameraAndFOV::mFarClippingDistance;
}
//<!-- Scene Culling --!>
