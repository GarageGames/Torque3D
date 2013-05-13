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

//Needed for culling
#include "T3D/gameBase/gameConnection.h"
#include "gui/core/guiCanvas.h"
#include "gui/core/guiControl.h"

//-----------------------------------------------------------------------------
static GuiCanvas* mCanvas;

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

void SceneRenderState::renderObjects( SceneObject** objects, U32 numObjects )
{ 
	// Let the objects batch their stuff.
	//Make sure it's a client render
	if(this->mSceneManager->GetIsClient())
	{
		//Get the connection for camera fov
		GameConnection* connection = GameConnection::getConnectionToServer();
		if( !connection )
		{
			//Client side render with no connection, should we return?
			return;			
		}
		
		//Get the Camera Object
		GameBase* cameraObject = 0;
		cameraObject = connection->getCameraObject();

		//Variables needed for culling
		fOV = cameraObject->getCameraFov();
		if(fOV < 90.0f)
			fOV = 90.0f;
		else
			fOV += 30.0f;

		//Near and far plane distances
		nearDist = this->getNearPlane(); 
		farDist =  this->getFarPlane();
		
		if (!mCanvas)
			mCanvas =dynamic_cast<GuiCanvas*>( Sim::findObject("Canvas"));

		//Equation for getting Near Plane dimensions
		yAdd = tan(fOV/2)*nearDist;
		xAdd = yAdd*(mCanvas->getPlatformWindow()->getVideoMode().resolution.x/mCanvas->getPlatformWindow()->getVideoMode().resolution.y);

		//Far Plane generation
		Point3F pPos, pView;
		MatrixF mTrans;
		cameraObject->getCameraTransform(pPos, &mTrans);
		pView = mTrans.getForwardVector();
		pPos = mTrans.getPosition();

		pCP = pPos + (pView)*farDist;
		plFar.set(pCP, (-1*pView));

		//Near Plane generation
		pCP = pPos + pView*nearDist;
		plNear.set(pCP, pView);

		//Getting the camera's orientation vectors
		right = mTrans.getRightVector();
		up = mTrans.getUpVector();

		//4 corners of near Plane
		pTR = pCP + (right*xAdd) + (up*yAdd);
		pTL = pCP - (right*xAdd) + (up*yAdd);
		pBR = pCP + (right*xAdd) - (up*yAdd);
		pBL = pCP - (right*xAdd) - (up*yAdd);

		//Finally set this to the camera position
		pCP = pPos;

		//Generate the side, top, and bottom planes
		plLeft.set(pCP, pTL, pBL);
		plTop.set(pCP, pTR, pTL);
		plRight.set(pCP, pBR, pTR);
		plBottom.set(pCP, pBL, pBR);

		//Start sorting throughb and prepping to render
		PROFILE_START( SceneRenderState_prepRenderImages );
		for( U32 i = 0; i < numObjects; ++ i )
		{
			//Compare each object
			SceneObject* object = objects[ i ];
			bounds = object->getWorldBox();

			//For each plane, check all 8 corners of the bounding box
			//Early outs if any conditions fail.
			// I.e. if the box is completely behind one plane
			if(plNear.whichSide(bounds) < 0.0f)
				continue;
			if(plFar.whichSide(bounds) < 0.0f)
				continue;
			if(plLeft.whichSide(bounds) < 0.0f)
				continue;
			if(plRight.whichSide(bounds) < 0.0f)
				continue;
			if(plBottom.whichSide(bounds) < 0.0f)
				continue;
			if(plTop.whichSide(bounds) < 0.0f)
				continue;
			
			//Add the object if it is within the view
			object->prepRenderImage( this );
		}
	}

	else
	{
		//Server Render
		PROFILE_START( SceneRenderState_prepRenderImages );
		for( U32 i = 0; i < numObjects; ++ i )
		{
			SceneObject* object = objects[ i ];
			object->prepRenderImage( this );
		}
	}

   PROFILE_END();

   // Render what the objects have batched.

   getRenderPass()->renderPass( this );
}
