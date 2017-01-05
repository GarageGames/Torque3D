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

#ifndef TORQUE_TGB_ONLY

#include "guiDecalEditorCtrl.h"
#include "platform/platform.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "scene/sceneManager.h"
#include "collision/collision.h"
#include "math/util/frustum.h"
#include "gfx/gfxPrimitiveBuffer.h"
#include "gfx/gfxTextureHandle.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/core/guiCanvas.h"
#include "gui/buttons/guiButtonCtrl.h"
#include "gui/worldEditor/gizmo.h"
#include "T3D/decal/decalManager.h"
#include "T3D/decal/decalInstance.h"
#include "gui/worldEditor/undoActions.h"

IMPLEMENT_CONOBJECT(GuiDecalEditorCtrl);

ConsoleDocClass( GuiDecalEditorCtrl,
   "@brief The base class for the Decal Editor tool\n\n"
   "Editor use only.\n\n"
   "@internal"
);

bool GuiDecalEditorCtrl::smRenderDecalPixelSize = false;

GuiDecalEditorCtrl::GuiDecalEditorCtrl()
{   
   mSELDecal = NULL;
   mHLDecal = NULL;
   mCurrentDecalData = NULL;
	mMode = "AddDecalMode";
   mPerformedDragCopy = false;
}

GuiDecalEditorCtrl::~GuiDecalEditorCtrl()
{
   // nothing to do
}

bool GuiDecalEditorCtrl::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   return true;
}

void GuiDecalEditorCtrl::initPersistFields()
{
   addField( "currentDecalData", TYPEID< DecalData >(), Offset( mCurrentDecalData, GuiDecalEditorCtrl ) );

   Parent::initPersistFields();
}

void GuiDecalEditorCtrl::consoleInit()
{
   Con::addVariable( "$DecalEditor::renderPixelSize", TypeBool, &smRenderDecalPixelSize, 
      "Set true to render the pixel size as on overlay on the selected decal instance. "
      "This is the value used to fade distant decals and is intended to help the user adjust "
      "the values of DecalData::pixelSizeStartFade and pixelSizeEndFade.\n\n"
	  "@internal" );

   Parent::consoleInit();
}

void GuiDecalEditorCtrl::onEditorDisable()
{
   // Tools are not deleted/recreated between missions, but decals instances
   // ARE. So we must release any references.
   mSELDecal = NULL;
   mHLDecal = NULL;
}

bool GuiDecalEditorCtrl::onWake()
{
   if ( !Parent::onWake() )
      return false;
	
	

   return true;
}

void GuiDecalEditorCtrl::onSleep()
{
   Parent::onSleep();   
}

void GuiDecalEditorCtrl::get3DCursor( GuiCursor *&cursor, 
                                       bool &visible, 
                                       const Gui3DMouseEvent &event_ )
{
   cursor = NULL;
   visible = false;

   GuiCanvas *root = getRoot();
   if ( !root )
      return;

   S32 currCursor = PlatformCursorController::curArrow;

   if ( root->mCursorChanged == currCursor )
      return;

   PlatformWindow *window = root->getPlatformWindow();
   PlatformCursorController *controller = window->getCursorController();
   
   // We've already changed the cursor, 
   // so set it back before we change it again.
   if( root->mCursorChanged != -1)
      controller->popCursor();

   // Now change the cursor shape
   controller->pushCursor(currCursor);
   root->mCursorChanged = currCursor;   
}

void GuiDecalEditorCtrl::on3DMouseDown(const Gui3DMouseEvent & event)
{
   mPerformedDragCopy = false;

   if ( !isFirstResponder() )
      setFirstResponder();
	
	bool dblClick = ( event.mouseClickCount > 1 );

	// Gather selected decal information 
   RayInfo ri;
   bool hit = getRayInfo( event, &ri );

   Point3F start = event.pos;
   Point3F end = start + event.vec * 3000.0f; // use visible distance here??

   DecalInstance *pDecal = gDecalManager->raycast( start, end );
	
	if( mMode.compare("AddDecalMode") != 0 )
	{
		if ( mSELDecal )
		{
			// If our click hit the gizmo we are done.			
			if ( mGizmo->getSelection() != Gizmo::None )
			{
            mGizmo->on3DMouseDown( event );

				char returnBuffer[256];
				dSprintf(returnBuffer, sizeof(returnBuffer), "%f %f %f %f %f %f %f", 
				mSELDecal->mPosition.x, mSELDecal->mPosition.y, mSELDecal->mPosition.z, 
				mSELDecal->mTangent.x, mSELDecal->mTangent.y, mSELDecal->mTangent.z,
				mSELDecal->mSize);

				Con::executef( this, "prepGizmoTransform", Con::getIntArg(mSELDecal->mId), returnBuffer );

				return;
			}
		}

		if ( mHLDecal && pDecal == mHLDecal )
		{
			mHLDecal = NULL;            
			selectDecal( pDecal );   

			if ( isMethod( "onSelectInstance" ) )
			{
				char idBuf[512];
				dSprintf(idBuf, 512, "%i", pDecal->mId);
				Con::executef( this, "onSelectInstance", String(idBuf).c_str(), pDecal->mDataBlock->lookupName.c_str() );
			}

			return;
		}
		else if ( hit && !pDecal)
		{
			if ( dblClick )
				setMode( String("AddDecalMode"), true );

			return;
		}
	}
	else
	{
		// If we accidently hit a decal, then bail(probably an accident). If the use hits the decal twice,
		// then boot them into selection mode and select the decal.
		if ( mHLDecal && pDecal == mHLDecal )
		{
			if ( dblClick )
			{
				mHLDecal = NULL;            
				selectDecal( pDecal );   

				if ( isMethod( "onSelectInstance" ) )
				{
					char idBuf[512];
					dSprintf(idBuf, 512, "%i", pDecal->mId);
					Con::executef( this, "onSelectInstance", String(idBuf).c_str(), pDecal->mDataBlock->lookupName.c_str() );
				}
				setMode( String("SelectDecalMode"), true );
			}
			return;	
		}

		if ( hit && mCurrentDecalData ) // Create a new decal...
		{
			U8 flags = PermanentDecal | SaveDecal;

			DecalInstance *decalInst = gDecalManager->addDecal( ri.point, ri.normal, 0.0f, mCurrentDecalData, 1.0f, -1, flags );      
	      
			if ( decalInst )  
			{
				// Give the decal an id
				decalInst->mId = gDecalManager->mDecalInstanceVec.size();
				gDecalManager->mDecalInstanceVec.push_back(decalInst);

				selectDecal( decalInst );
				
				// Grab the mission editor undo manager.
				UndoManager *undoMan = NULL;
				if ( !Sim::findObject( "EUndoManager", undoMan ) )
				{
					Con::errorf( "GuiMeshRoadEditorCtrl::on3DMouseDown() - EUndoManager not found!" );
					return;           
				}

				// Create the UndoAction.
				DICreateUndoAction *action = new DICreateUndoAction("Create Decal");
				action->addDecal( *decalInst );
				
				action->mEditor = this;
				// Submit it.               
				undoMan->addAction( action );

				if ( isMethod( "onCreateInstance" ) )
				{
					char buffer[512];
					dSprintf(buffer, 512, "%i", decalInst->mId);
					Con::executef( this, "onCreateInstance", buffer, decalInst->mDataBlock->lookupName.c_str());
				}
			}

			return;
		}
	}

   if ( !mSELDecal )
      return;
}

void GuiDecalEditorCtrl::on3DRightMouseDown(const Gui3DMouseEvent & event)
{
   //mIsPanning = true;
   //mGizmo->on3DRightMouseDown( event );
}

void GuiDecalEditorCtrl::on3DRightMouseUp(const Gui3DMouseEvent & event)
{
   //mIsPanning = false;
//   mGizmo->on3DRightMouseUp( event );
}

void GuiDecalEditorCtrl::on3DMouseUp(const Gui3DMouseEvent & event)
{
   if ( mSELDecal )
	{
		if ( mGizmo->isDirty() )
		{
			char returnBuffer[256];
			dSprintf(returnBuffer, sizeof(returnBuffer), "%f %f %f %f %f %f %f", 
			mSELDecal->mPosition.x, mSELDecal->mPosition.y, mSELDecal->mPosition.z, 
			mSELDecal->mTangent.x, mSELDecal->mTangent.y, mSELDecal->mTangent.z,
			mSELDecal->mSize);

			Con::executef( this, "completeGizmoTransform", Con::getIntArg(mSELDecal->mId), returnBuffer );

			mGizmo->markClean();
		}

      mGizmo->on3DMouseUp( event );
	}
}

void GuiDecalEditorCtrl::on3DMouseMove(const Gui3DMouseEvent & event)
{
   if ( mSELDecal )
      mGizmo->on3DMouseMove( event );

   RayInfo ri;
   if ( !getRayInfo( event, &ri ) )
      return; 

   Point3F start = event.pos;
   Point3F end = start + event.vec * 3000.0f; // use visible distance here??

   DecalInstance *pDecal = gDecalManager->raycast( start, end );

   if ( pDecal && pDecal != mSELDecal )
      mHLDecal = pDecal;   
   else if ( !pDecal )
      mHLDecal = NULL;
}

void GuiDecalEditorCtrl::on3DMouseDragged(const Gui3DMouseEvent & event)
{ 
   if ( !mSELDecal )
      return;

   // Doing a drag copy of the decal?
   if ( event.modifier & SI_SHIFT && !mPerformedDragCopy )
   {
      mPerformedDragCopy = true;

		DecalInstance *newDecal = gDecalManager->addDecal(    mSELDecal->mPosition, 
                                                            mSELDecal->mNormal, 
                                                            0.0f, 
                                                            mSELDecal->mDataBlock, 
                                                            1.0f, 
                                                            -1, 
                                                            PermanentDecal | SaveDecal );

      newDecal->mTangent = mSELDecal->mTangent;
      newDecal->mSize = mSELDecal->mSize;
      newDecal->mTextureRectIdx = mSELDecal->mTextureRectIdx;

      // TODO: This is crazy... we should move this sort of tracking
      // inside of the decal manager... IdDecal flag maybe or just a
      // byproduct of PermanentDecal?
      //
		newDecal->mId = gDecalManager->mDecalInstanceVec.size();
		gDecalManager->mDecalInstanceVec.push_back( newDecal );

		selectDecal( newDecal );
			
		// Grab the mission editor undo manager.
		UndoManager *undoMan = NULL;
		if ( Sim::findObject( "EUndoManager", undoMan ) )
		{
			// Create the UndoAction.
			DICreateUndoAction *action = new DICreateUndoAction("Create Decal");
			action->addDecal( *mSELDecal );
			action->mEditor = this;
			undoMan->addAction( action );

			if ( isMethod( "onCreateInstance" ) )
			{
				char buffer[512];
				dSprintf( buffer, 512, "%i", mSELDecal->mId );
				Con::executef( this, "onCreateInstance", buffer, mSELDecal->mDataBlock->lookupName.c_str());
			}
		}
   }

   // Update the Gizmo.
   if (mGizmo->getSelection() != Gizmo::None)
   {
      mGizmo->on3DMouseDragged( event );

      // Pull out the Gizmo transform
      // and position.
      const MatrixF &gizmoMat = mGizmo->getTransform();
      const Point3F &gizmoPos = gizmoMat.getPosition();
      
      // Get the new projection vector.
      VectorF upVec, rightVec;
      gizmoMat.getColumn( 0, &rightVec );
      gizmoMat.getColumn( 2, &upVec );

      const Point3F &scale = mGizmo->getScale();

      // Assign the appropriate changed value back to the decal.
      if ( mGizmo->getMode() == ScaleMode )
      {
         // Save old size.
         const F32 oldSize = mSELDecal->mSize;

         // Set new size.
         mSELDecal->mSize = ( scale.x + scale.y ) * 0.5f;

         // See if the decal properly clips/projects at this size.  If not,
         // stick to the old size.
         mSELEdgeVerts.clear();
         if ( !gDecalManager->clipDecal( mSELDecal, &mSELEdgeVerts ) )
            mSELDecal->mSize = oldSize;
      }
      else if ( mGizmo->getMode() == MoveMode )
         mSELDecal->mPosition = gizmoPos;
      else if ( mGizmo->getMode() == RotateMode )
      {
         mSELDecal->mNormal = upVec;
         mSELDecal->mTangent = rightVec;
      }

      gDecalManager->notifyDecalModified( mSELDecal );

	   Con::executef( this, "syncNodeDetails" );
   }
}

void GuiDecalEditorCtrl::on3DMouseEnter(const Gui3DMouseEvent & event)
{
   // nothing to do
}

void GuiDecalEditorCtrl::on3DMouseLeave(const Gui3DMouseEvent & event)
{
   // nothing to do
}

void GuiDecalEditorCtrl::updateGuiInfo()
{
   // nothing to do
}
      
void GuiDecalEditorCtrl::onRender( Point2I offset, const RectI &updateRect )
{
   Parent::onRender( offset, updateRect );
}

void GuiDecalEditorCtrl::renderGui( Point2I offset, const RectI &updateRect )
{
   Parent::renderGui( offset, updateRect );

   PROFILE_SCOPE( GuiDecalEditorCtrl_renderGui );

   // Show the pixelSize of the selected decal as a text overlay.
   if ( smRenderDecalPixelSize && mSELDecal != NULL )
   {
      const F32 pixelSize = mSELDecal->calcPixelSize( mSaveViewport.extent.y, mLastCameraQuery.cameraMatrix.getPosition(), mSaveWorldToScreenScale.y );
      
      // Find position onscreen to render the text.
      Point3F screenPos;
      bool onScreen = project( mSELDecal->mPosition, &screenPos );

      if ( onScreen )
      {
         // It is extremely annoying to require the GuiProfile to have a font
         // or to create one within the decal editor for only this single use,
         // so we instead rely on the fact that we already have a Gizmo, that
         // all Gizmo's have a GizmoProfile, and that GizmoProfile has a font.
         GFont *font = mGizmo->getProfile()->font;

         // Might as well use some colors defined in GizmoProfile too instead
         // of just hardcoding it here.
         const ColorI bgColor = mGizmo->getProfile()->inActiveColor;
         const ColorI textColor = mGizmo->getProfile()->activeColor;

         // Note: This mostly mirrors the way WorldEditor renders popupText for
         // the gizmo during a drag operation, consider unifying this into a utility method.

         char buf[256];
         dSprintf( buf, 256, "%0.3f", pixelSize );

         const U32 width = font->getStrWidth((const UTF8 *)buf);;
         const Point2I posi( (U32)screenPos.x, (U32)screenPos.y + 12 );   
         const Point2I minPt(posi.x - width / 2 - 2, posi.y - 1);
         const Point2I maxPt(posi.x + width / 2 + 2, posi.y + font->getHeight() + 1);

         GFXDrawUtil *drawer = GFX->getDrawUtil();
         drawer->drawRectFill( minPt, maxPt, bgColor );
	      GFX->getDrawUtil()->setBitmapModulation( textColor );
         GFX->getDrawUtil()->drawText( mProfile->mFont, Point2I( posi.x - width / 2, posi.y ), buf );
      }      
   }
}

void GuiDecalEditorCtrl::renderScene(const RectI & updateRect)
{
   PROFILE_SCOPE( GuiDecalEditorCtrl_renderScene );

   GFXTransformSaver saver;
   
   ColorI hlColor(0,255,0,255);
   ColorI regColor(255,0,0,255);
   ColorI selColor(0,0,255,255);
   ColorI color;
   
   GFXDrawUtil *drawUtil = GFX->getDrawUtil();   

   GFXStateBlockDesc desc;
   desc.setBlend( true );
   desc.setZReadWrite( true, false );

   // Draw 3D stuff here.   
   if ( mSELDecal )
   {
      mGizmo->renderGizmo( mLastCameraQuery.cameraMatrix, mLastCameraQuery.fov );

      mSELEdgeVerts.clear();
      if ( gDecalManager->clipDecal( mSELDecal, &mSELEdgeVerts ) )
         _renderDecalEdge( mSELEdgeVerts, ColorI( 255, 255, 255, 255 ) );

      const F32 &decalSize = mSELDecal->mSize;
      Point3F boxSize( decalSize, decalSize, decalSize );

      MatrixF worldMat( true );
      mSELDecal->getWorldMatrix( &worldMat, true );   

      drawUtil->drawObjectBox( desc, boxSize, mSELDecal->mPosition, worldMat, ColorI( 255, 255, 255, 255 ) );
   }

   if ( mHLDecal )
   {
      mHLEdgeVerts.clear();
      if ( gDecalManager->clipDecal( mHLDecal, &mHLEdgeVerts ) )
         _renderDecalEdge( mHLEdgeVerts, ColorI( 255, 255, 255, 255 ) );

      const F32 &decalSize = mHLDecal->mSize;
      Point3F boxSize( decalSize, decalSize, decalSize );

      MatrixF worldMat( true );
      mHLDecal->getWorldMatrix( &worldMat, true );  

      drawUtil->drawObjectBox( desc, boxSize, mHLDecal->mPosition, worldMat, ColorI( 255, 255, 255, 255 ) );
   }
} 

void GuiDecalEditorCtrl::forceRedraw( DecalInstance * decalInstance )
{ 
   // This should be redundant because the decal is already reclipped
   // on each frame. Also it is not possible execute rendering code like
   // this in response to UI events from script.
   /*
	if ( !decalInstance )
		return;

	GFXDrawUtil *drawUtil = GFX->getDrawUtil();  
	GFXStateBlockDesc desc;
   desc.setBlend( true );
   desc.setZReadWrite( true, false );

   Vector<Point3F> verts;
   verts.clear();
   if ( gDecalManager->clipDecal( decalInstance, &verts ) )
   if ( gDecalManager->clipDecal( decalInstance, &verts ) )
      _renderDecalEdge( verts, ColorI( 255, 255, 255, 255 ) );

   const F32 &decalSize = decalInstance->mSize;
   Point3F boxSize( decalSize, decalSize, decalSize );

   MatrixF worldMat( true );
   decalInstance->getWorldMatrix( &worldMat, true );   

   drawUtil->drawObjectBox( desc, boxSize, decalInstance->mPosition, worldMat, ColorI( 255, 255, 255, 255 ) );
   */
}

void GuiDecalEditorCtrl::_renderDecalEdge( const Vector<Point3F> &verts, const ColorI &color )
{
   U32 vertCount = verts.size();

   GFXTransformSaver saver;

   PrimBuild::color( color );

   Point3F endPt( 0, 0, 0 );
   for ( U32 i = 0; i < vertCount; i++ )
   {
      const Point3F &vert = verts[i];
      if ( i + 1 < vertCount )
         endPt = verts[i + 1];
      else
         break;

      PrimBuild::begin( GFXLineList, 2 );

      PrimBuild::vertex3f( vert.x, vert.y, vert.z );
      PrimBuild::vertex3f( endPt.x, endPt.y, endPt.z );

      PrimBuild::end();
   }
}

bool GuiDecalEditorCtrl::getRayInfo( const Gui3DMouseEvent & event, RayInfo *rInfo )
{       
   Point3F startPnt = event.pos;
   Point3F endPnt = event.pos + event.vec * 3000.0f;

   bool hit;         
         
   hit = gServerContainer.castRayRendered( startPnt, endPnt, STATIC_COLLISION_TYPEMASK, rInfo );    
   
   return hit;
}

void GuiDecalEditorCtrl::selectDecal( DecalInstance *decalInst )
{
   // If id is zero or invalid we set the selected decal to null
   // which is correct.
   mSELDecal = decalInst;

   if ( decalInst )
      setGizmoFocus( decalInst );
}

void GuiDecalEditorCtrl::deleteSelectedDecal()
{
   if ( !mSELDecal )
      return;
	
	// Grab the mission editor undo manager.
	UndoManager *undoMan = NULL;
	if ( !Sim::findObject( "EUndoManager", undoMan ) )
	{
		Con::errorf( "GuiMeshRoadEditorCtrl::on3DMouseDown() - EUndoManager not found!" );
		return;           
	}

	// Create the UndoAction.
	DIDeleteUndoAction *action = new DIDeleteUndoAction("Delete Decal");
	action->deleteDecal( *mSELDecal );
	
	action->mEditor = this;
	// Submit it.               
	undoMan->addAction( action );
	
	if ( isMethod( "onDeleteInstance" ) )
	{
		char buffer[512];
		dSprintf(buffer, 512, "%i", mSELDecal->mId);
		Con::executef( this, "onDeleteInstance", String(buffer).c_str(), mSELDecal->mDataBlock->lookupName.c_str() );
	}

   gDecalManager->removeDecal( mSELDecal );
   mSELDecal = NULL;
}

void GuiDecalEditorCtrl::deleteDecalDatablock( String lookupName )
{
	DecalData * datablock = dynamic_cast<DecalData*> ( Sim::findObject(lookupName.c_str()) );
	if( !datablock )
		return;

	// Grab the mission editor undo manager.
	UndoManager *undoMan = NULL;
	if ( !Sim::findObject( "EUndoManager", undoMan ) )
	{
		Con::errorf( "GuiMeshRoadEditorCtrl::on3DMouseDown() - EUndoManager not found!" );
		return;           
	}

	// Create the UndoAction.
	DBDeleteUndoAction *action = new DBDeleteUndoAction("Delete Decal Datablock");
	action->mEditor = this;
	action->mDatablockId = datablock->getId();
	
	Vector<DecalInstance*> mDecalQueue;
	Vector<DecalInstance *>::iterator iter;
	mDecalQueue.clear();
	const Vector<DecalSphere*> &grid = gDecalManager->getDecalDataFile()->getSphereList();

	for ( U32 i = 0; i < grid.size(); i++ )
	{
		const DecalSphere *decalSphere = grid[i];
		mDecalQueue.merge( decalSphere->mItems );
	}

	for ( iter = mDecalQueue.begin();iter != mDecalQueue.end();iter++ )
	{	
		if( !(*iter) )
			continue;

		if( (*iter)->mDataBlock->lookupName.compare( lookupName ) == 0 )
		{
			if( (*iter)->mId != -1 )
			{
				//make sure to call onDeleteInstance as well
				if ( isMethod( "onDeleteInstance" ) )
				{
					char buffer[512];
					dSprintf(buffer, 512, "%i", (*iter)->mId);
					Con::executef( this, "onDeleteInstance", String(buffer).c_str(), (*iter)->mDataBlock->lookupName.c_str() );
				}
				
				action->deleteDecal( *(*iter) );
				
				if( mSELDecal == (*iter) )
					mSELDecal = NULL;

				if( mHLDecal == (*iter) )
					mHLDecal = NULL;
			}
			gDecalManager->removeDecal( (*iter) );
		}
	}
	
	undoMan->addAction( action );

	mCurrentDecalData = NULL;
}

void GuiDecalEditorCtrl::retargetDecalDatablock( String dbFrom, String dbTo )
{
	DecalData * ptrFrom = dynamic_cast<DecalData*> ( Sim::findObject(dbFrom.c_str()) );
	DecalData * ptrTo = dynamic_cast<DecalData*> ( Sim::findObject(dbTo.c_str()) );
	
	if( !ptrFrom || !ptrTo )
		return;
	
	// Grab the mission editor undo manager.
	UndoManager *undoMan = NULL;
	if ( !Sim::findObject( "EUndoManager", undoMan ) )
	{
		Con::errorf( "GuiMeshRoadEditorCtrl::on3DMouseDown() - EUndoManager not found!" );
		return;           
	}

	// Create the UndoAction.
	DBRetargetUndoAction *action = new DBRetargetUndoAction("Retarget Decal Datablock");
	action->mEditor = this;
	action->mDBFromId = ptrFrom->getId();
	action->mDBToId = ptrTo->getId();

	Vector<DecalInstance*> mDecalQueue;
	Vector<DecalInstance *>::iterator iter;
	mDecalQueue.clear();
	const Vector<DecalSphere*> &grid = gDecalManager->getDecalDataFile()->getSphereList();
	for ( U32 i = 0; i < grid.size(); i++ )
	{
		const DecalSphere *decalSphere = grid[i];
		mDecalQueue.merge( decalSphere->mItems );
	}

	for ( iter = mDecalQueue.begin();iter != mDecalQueue.end();iter++ )
	{	
		if( !(*iter) )
			continue;

		if( (*iter)->mDataBlock->lookupName.compare( dbFrom ) == 0 )
		{
			if( (*iter)->mId != -1 )
			{
				action->retargetDecal((*iter));	
				(*iter)->mDataBlock = ptrTo;
				forceRedraw((*iter));
			}
		}
	}

	undoMan->addAction( action );
}

void GuiDecalEditorCtrl::setMode( String mode, bool sourceShortcut = false )
{
	if( mode.compare("SelectDecalMode") == 0)
		mGizmo->getProfile()->mode = NoneMode;
	else if( mode.compare("AddDecalMode") == 0)
		mGizmo->getProfile()->mode = NoneMode;
	else if( mode.compare("MoveDecalMode") == 0)
		mGizmo->getProfile()->mode = MoveMode;
	else if( mode.compare("RotateDecalMode") == 0)
		mGizmo->getProfile()->mode = RotateMode;
	else if( mode.compare("ScaleDecalMode") == 0)
		mGizmo->getProfile()->mode = ScaleMode;
	
	mMode = mode;

	if( sourceShortcut )
		Con::executef( this, "paletteSync", mMode );
}

DefineConsoleMethod( GuiDecalEditorCtrl, deleteSelectedDecal, void, (), , "deleteSelectedDecal()" )
{
   object->deleteSelectedDecal();
}

DefineConsoleMethod( GuiDecalEditorCtrl, deleteDecalDatablock, void, ( const char * datablock ), , "deleteSelectedDecalDatablock( String datablock )" )
{
	String lookupName( datablock );
	if( lookupName == String::EmptyString )
		return;
	
	object->deleteDecalDatablock( lookupName );
}

DefineConsoleMethod( GuiDecalEditorCtrl, setMode, void, ( String newMode ), , "setMode( String mode )()" )
{
	object->setMode( newMode );
}

DefineConsoleMethod( GuiDecalEditorCtrl, getMode, const char*, (), , "getMode()" )
{
	return object->mMode;
}

DefineConsoleMethod( GuiDecalEditorCtrl, getDecalCount, S32, (), , "getDecalCount()" )
{
	return gDecalManager->mDecalInstanceVec.size();
}

DefineConsoleMethod( GuiDecalEditorCtrl, getDecalTransform, const char*, ( U32 id ), , "getDecalTransform()" )
{
   DecalInstance *decalInstance = gDecalManager->mDecalInstanceVec[id];

   if( decalInstance == NULL )
      return "";

   static const U32 bufSize = 256;
   char* returnBuffer = Con::getReturnBuffer(bufSize);
   returnBuffer[0] = 0;

   if ( decalInstance )
   {
	   dSprintf(returnBuffer, bufSize, "%f %f %f %f %f %f %f",
         decalInstance->mPosition.x, decalInstance->mPosition.y, decalInstance->mPosition.z, 
		   decalInstance->mTangent.x, decalInstance->mTangent.y, decalInstance->mTangent.z,
		   decalInstance->mSize);
   }

	return returnBuffer;
}

DefineConsoleMethod( GuiDecalEditorCtrl, getDecalLookupName, const char*, ( U32 id ), , "getDecalLookupName( S32 )()" )
{
	DecalInstance *decalInstance = gDecalManager->mDecalInstanceVec[id];
	if( decalInstance == NULL )
		return "invalid";

	return decalInstance->mDataBlock->lookupName;
}

DefineConsoleMethod( GuiDecalEditorCtrl, selectDecal, void, ( U32 id ), , "selectDecal( S32 )()" )
{
	DecalInstance *decalInstance = gDecalManager->mDecalInstanceVec[id];
	if( decalInstance == NULL )
		return;

	object->selectDecal( decalInstance );
}

DefineConsoleMethod( GuiDecalEditorCtrl, editDecalDetails, void, ( U32 id, Point3F pos, Point3F tan,F32 size ), , "editDecalDetails( S32 )()" )
{
	DecalInstance *decalInstance = gDecalManager->mDecalInstanceVec[id];
	if( decalInstance == NULL )
		return;


   decalInstance->mPosition = pos;
	decalInstance->mTangent = tan;
	decalInstance->mSize = size;
	
	if ( decalInstance == object->mSELDecal )
		object->setGizmoFocus( decalInstance );

	object->forceRedraw( decalInstance );

	gDecalManager->notifyDecalModified( decalInstance );
}

DefineConsoleMethod( GuiDecalEditorCtrl, getSelectionCount, S32, (), , "" )
{
   if ( object->mSELDecal != NULL )
      return 1;
   return 0;
}

DefineConsoleMethod( GuiDecalEditorCtrl, retargetDecalDatablock, void, ( const char * dbFrom, const char * dbTo ), , "" )
{
   if( dStrcmp( dbFrom, "" ) != 0 && dStrcmp( dbTo, "" ) != 0 )
		object->retargetDecalDatablock( dbFrom, dbTo );
}

void GuiDecalEditorCtrl::setGizmoFocus( DecalInstance * decalInstance )
{
	const F32 &size = decalInstance->mSize;
   MatrixF worldMat( true );
   decalInstance->getWorldMatrix( &worldMat, true );
   worldMat.setPosition( Point3F( 0, 0, 0 ) );
   mGizmo->set( worldMat, decalInstance->mPosition, Point3F( size, size, size ) );
}

//Decal Instance Create Undo Actions
IMPLEMENT_CONOBJECT( DICreateUndoAction );

ConsoleDocClass( DICreateUndoAction,
				"@brief Decal Instance Create Undo Actions\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

DICreateUndoAction::DICreateUndoAction( const UTF8* actionName )
   :  UndoAction( actionName ), mEditor(0), mDatablockId(0)
{
}

DICreateUndoAction::~DICreateUndoAction()
{
}

void DICreateUndoAction::initPersistFields()
{
   Parent::initPersistFields();
}

void DICreateUndoAction::addDecal(const DecalInstance& decal)
{
	mDecalInstance = decal;
	mDatablockId = decal.mDataBlock->getId();
}

void DICreateUndoAction::undo()
{
	Vector<DecalInstance *>::iterator iter;
   for(iter = gDecalManager->mDecalInstanceVec.begin();iter != gDecalManager->mDecalInstanceVec.end();iter++)
	{
		if( !(*iter) )
			continue;
		
		if( (*iter)->mId != mDecalInstance.mId )
			continue;

		if ( mEditor->isMethod( "onDeleteInstance" ) )
		{
			char buffer[512];
			dSprintf(buffer, 512, "%i", (*iter)->mId);
			Con::executef( mEditor, "onDeleteInstance", String(buffer).c_str(), (*iter)->mDataBlock->lookupName.c_str() );
		}
		
		// Decal manager handles clearing the vector if the decal contains a valid id
		if( mEditor->mSELDecal == (*iter) )
			mEditor->mSELDecal = NULL;

		if( mEditor->mHLDecal == (*iter) )
			mEditor->mHLDecal = NULL;

		gDecalManager->removeDecal( (*iter) );
		break;
	}
}

void DICreateUndoAction::redo()
{
	//Reinstate the valid datablock pointer	
	mDecalInstance.mDataBlock = dynamic_cast<DecalData *>( Sim::findObject( mDatablockId ) );

	DecalInstance * decal = gDecalManager->addDecal( mDecalInstance.mPosition, 
		mDecalInstance.mNormal, 
		mDecalInstance.mTangent, 
		mDecalInstance.mDataBlock,
		( mDecalInstance.mSize / mDecalInstance.mDataBlock->size ), 
		mDecalInstance.mTextureRectIdx, 
		mDecalInstance.mFlags );
	
	decal->mId = mDecalInstance.mId;

	// Override the rectIdx regardless of random decision in addDecal
	decal->mTextureRectIdx = mDecalInstance.mTextureRectIdx;
	
	// We take care of filling in the vector space that was once there
	gDecalManager->mDecalInstanceVec[decal->mId] = decal;

	if ( mEditor->isMethod( "onCreateInstance" ) )
	{
		char buffer[512];
		dSprintf(buffer, 512, "%i", decal->mId);
		Con::executef( mEditor, "onCreateInstance", buffer, decal->mDataBlock->lookupName.c_str());
	}
	mEditor->selectDecal( decal );
}

//Decal Instance Delete Undo Actions
IMPLEMENT_CONOBJECT( DIDeleteUndoAction );

ConsoleDocClass( DIDeleteUndoAction,
				"@brief Decal Instance Delete Undo Actions\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

DIDeleteUndoAction::DIDeleteUndoAction( const UTF8 *actionName )
   :  UndoAction( actionName ), mEditor(0), mDatablockId(0)
{
}

DIDeleteUndoAction::~DIDeleteUndoAction()
{
}

void DIDeleteUndoAction::initPersistFields()
{
   Parent::initPersistFields();
}

void DIDeleteUndoAction::deleteDecal(const DecalInstance& decal)
{
	mDecalInstance = decal;
	mDatablockId = decal.mDataBlock->getId();
}

void DIDeleteUndoAction::undo()
{
	//Reinstate the valid datablock pointer	
	mDecalInstance.mDataBlock = dynamic_cast<DecalData *>( Sim::findObject( mDatablockId ) );

	DecalInstance * decal = gDecalManager->addDecal( mDecalInstance.mPosition, 
		mDecalInstance.mNormal, 
		mDecalInstance.mTangent, 
		mDecalInstance.mDataBlock,
		( mDecalInstance.mSize / mDecalInstance.mDataBlock->size ), 
		mDecalInstance.mTextureRectIdx, 
		mDecalInstance.mFlags );
	
	decal->mId = mDecalInstance.mId;

	// Override the rectIdx regardless of random decision in addDecal
	decal->mTextureRectIdx = mDecalInstance.mTextureRectIdx;
	
	// We take care of filling in the vector space that was once there
	gDecalManager->mDecalInstanceVec[decal->mId] = decal;

	if ( mEditor->isMethod( "onCreateInstance" ) )
	{
		char buffer[512];
		dSprintf(buffer, 512, "%i", decal->mId);
		Con::executef( mEditor, "onCreateInstance", buffer, decal->mDataBlock->lookupName.c_str());
	}
	mEditor->selectDecal( decal );
}

void DIDeleteUndoAction::redo()
{
	Vector<DecalInstance *>::iterator iter;
   for(iter = gDecalManager->mDecalInstanceVec.begin();iter != gDecalManager->mDecalInstanceVec.end();iter++)
	{
		if( !(*iter) )
			continue;
		
		if( (*iter)->mId != mDecalInstance.mId )
			continue;

		if ( mEditor->isMethod( "onDeleteInstance" ) )
		{
			char buffer[512];
			dSprintf(buffer, 512, "%i", (*iter)->mId);
			Con::executef( mEditor, "onDeleteInstance", String(buffer).c_str(), (*iter)->mDataBlock->lookupName.c_str() );
		}
		
		// Decal manager handles clearing the vector if the decal contains a valid id
		if( mEditor->mSELDecal == (*iter) )
			mEditor->mSELDecal = NULL;

		if( mEditor->mHLDecal == (*iter) )
			mEditor->mHLDecal = NULL;

		gDecalManager->removeDecal( (*iter) );
		break;
	}
}

//Decal Datablock Delete Undo Actions
IMPLEMENT_CONOBJECT( DBDeleteUndoAction );

ConsoleDocClass( DBDeleteUndoAction,
				"@brief Decal Datablock Delete Undo Actions\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

DBDeleteUndoAction::DBDeleteUndoAction( const UTF8 *actionName )
   :  UndoAction( actionName ), mEditor(0), mDatablockId(0)
{
}

DBDeleteUndoAction::~DBDeleteUndoAction()
{
}

void DBDeleteUndoAction::initPersistFields()
{
   Parent::initPersistFields();
}

void DBDeleteUndoAction::deleteDecal(const DecalInstance& decal)
{
	mDecalInstanceVec.increment();
   mDecalInstanceVec.last() = decal;
}

void DBDeleteUndoAction::undo()
{
	DecalData * datablock = dynamic_cast<DecalData *>( Sim::findObject( mDatablockId ) );
	if ( mEditor->isMethod( "undoDeleteDecalDatablock" ) )
			Con::executef( mEditor, "undoDeleteDecalDatablock", datablock->lookupName.c_str());

	// Create and restore the decal instances
	for ( S32 i= mDecalInstanceVec.size()-1; i >= 0; i-- )
   {
		DecalInstance vecInstance = mDecalInstanceVec[i];

		//Reinstate the valid datablock pointer		
		vecInstance.mDataBlock = datablock;

		DecalInstance * decalInstance = gDecalManager->addDecal( vecInstance.mPosition, 
		vecInstance.mNormal, 
		vecInstance.mTangent, 
		vecInstance.mDataBlock,
		( vecInstance.mSize / vecInstance.mDataBlock->size ), 
		vecInstance.mTextureRectIdx, 
		vecInstance.mFlags );
	
		decalInstance->mId = vecInstance.mId;

		// Override the rectIdx regardless of random decision in addDecal
		decalInstance->mTextureRectIdx = vecInstance.mTextureRectIdx;
	
		// We take care of filling in the vector space that was once there
		gDecalManager->mDecalInstanceVec[decalInstance->mId] = decalInstance;

		if ( mEditor->isMethod( "onCreateInstance" ) )
		{
			char buffer[512];
			dSprintf(buffer, 512, "%i", decalInstance->mId);
			Con::executef( mEditor, "onCreateInstance", buffer, decalInstance->mDataBlock->lookupName.c_str());
		}
	}
	
}

void DBDeleteUndoAction::redo()
{
	for ( S32 i=0; i < mDecalInstanceVec.size(); i++ )
   {
		DecalInstance vecInstance = mDecalInstanceVec[i];

		Vector<DecalInstance *>::iterator iter;
		for(iter = gDecalManager->mDecalInstanceVec.begin();iter != gDecalManager->mDecalInstanceVec.end();iter++)
		{
			DecalInstance * decalInstance = (*iter);
			if( !decalInstance )
				continue;
			
			if( decalInstance->mId != vecInstance.mId )
				continue;

			if ( mEditor->isMethod( "onDeleteInstance" ) )
			{
				char buffer[512];
				dSprintf(buffer, 512, "%i", decalInstance->mId);
				Con::executef( mEditor, "onDeleteInstance", String(buffer).c_str(), decalInstance->mDataBlock->lookupName.c_str() );
			}
			
			// Decal manager handles clearing the vector if the decal contains a valid id
			if( mEditor->mSELDecal == decalInstance )
				mEditor->mSELDecal = NULL;

			if( mEditor->mHLDecal == decalInstance )
				mEditor->mHLDecal = NULL;

			gDecalManager->removeDecal( decalInstance );
			break;
		}
	}
	
	DecalData * datablock = dynamic_cast<DecalData *>( Sim::findObject( mDatablockId ) );
	if ( mEditor->isMethod( "redoDeleteDecalDatablock" ) )
		Con::executef( mEditor, "redoDeleteDecalDatablock", datablock->lookupName.c_str());
}

//------------------------------
//Decal Datablock Retarget Undo Actions
IMPLEMENT_CONOBJECT( DBRetargetUndoAction );

ConsoleDocClass( DBRetargetUndoAction,
				"@brief Decal Datablock Retarget Undo Actions\n\n"
				"Not intended for game development, for editors or internal use only.\n\n "
				"@internal");

DBRetargetUndoAction::DBRetargetUndoAction( const UTF8 *actionName )
   :  UndoAction( actionName ), mEditor(0), mDBFromId(0), mDBToId(0)
{
}

DBRetargetUndoAction::~DBRetargetUndoAction()
{
}

void DBRetargetUndoAction::initPersistFields()
{
   Parent::initPersistFields();
}

void DBRetargetUndoAction::retargetDecal( DecalInstance* decal )
{
	mDecalInstanceVec.increment();
   mDecalInstanceVec.last() = decal;
}

void DBRetargetUndoAction::undo()
{
	DecalData * ptrFrom = dynamic_cast<DecalData*> ( Sim::findObject(mDBFromId) );
	
	if( !ptrFrom )
		return;

	Vector<DecalInstance *>::iterator iter;
	for(iter = mDecalInstanceVec.begin();iter != mDecalInstanceVec.end();iter++)
   {
		(*iter)->mDataBlock = ptrFrom;
		mEditor->forceRedraw((*iter));
	}
	if ( mEditor->isMethod( "rebuildInstanceTree" ) )
			Con::executef( mEditor, "rebuildInstanceTree" );
}

void DBRetargetUndoAction::redo()
{
	DecalData * ptrTo = dynamic_cast<DecalData*> ( Sim::findObject(mDBToId) );
	
	if( !ptrTo )
		return;

	Vector<DecalInstance *>::iterator iter;
	for(iter = mDecalInstanceVec.begin();iter != mDecalInstanceVec.end();iter++)
   {
		(*iter)->mDataBlock = ptrTo;
		mEditor->forceRedraw((*iter));
	}
	
	if ( mEditor->isMethod( "rebuildInstanceTree" ) )
		Con::executef( mEditor, "rebuildInstanceTree" );
}
#endif
