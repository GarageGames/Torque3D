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
#include "gui/worldEditor/guiConvexShapeEditorCtrl.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "T3D/convexShape.h"
#include "renderInstance/renderPassManager.h"
#include "collision/collision.h"
#include "math/util/frustum.h"
#include "math/mathUtils.h"
#include "gfx/gfxPrimitiveBuffer.h"
#include "gfx/gfxTextureHandle.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "gui/core/guiCanvas.h"
#include "gui/buttons/guiButtonCtrl.h"
#include "gui/worldEditor/undoActions.h"
#include "T3D/gameBase/gameConnection.h"
#include "gfx/sim/debugDraw.h"
#include "collision/optimizedPolyList.h"
#include "core/volume.h"
#include "gui/worldEditor/worldEditor.h"
#include "T3D/prefab.h"

IMPLEMENT_CONOBJECT( GuiConvexEditorCtrl );

ConsoleDocClass( GuiConvexEditorCtrl,
   "@brief The base class for the sketch tool\n\n"
   "Editor use only.\n\n"
   "@internal"
);


GuiConvexEditorCtrl::GuiConvexEditorCtrl()
 : mIsDirty( false ),
   mFaceHL( -1 ),
   mFaceSEL( -1 ),
   mFaceSavedXfm( true ),
   mSavedUndo( false ),
   mDragging( false ),
   mGizmoMatOffset( Point3F::Zero ),
   mPivotPos( Point3F::Zero ),
   mUsingPivot( false ),
   mSettingPivot( false ),
   mActiveTool( NULL ),
   mCreateTool( NULL ),
   mMouseDown( false ),
   mUndoManager( NULL ),
   mLastUndo( NULL ),
   mHasCopied( false ),
   mSavedGizmoFlags( -1 ),
   mCtrlDown( false )
{   
	mMaterialName = StringTable->insert("Grid512_OrangeLines_Mat");
}

GuiConvexEditorCtrl::~GuiConvexEditorCtrl()
{
}

bool GuiConvexEditorCtrl::onAdd()
{
   if ( !Parent::onAdd() )
      return false;   

   SceneManager::getPreRenderSignal().notify( this, &GuiConvexEditorCtrl::_prepRenderImage );

   mCreateTool = new ConvexEditorCreateTool( this );

   return true;
}

void GuiConvexEditorCtrl::onRemove()
{
   SceneManager::getPreRenderSignal().remove( this, &GuiConvexEditorCtrl::_prepRenderImage );

   SAFE_DELETE( mCreateTool );

   Parent::onRemove();
}

void GuiConvexEditorCtrl::initPersistFields()
{   
   addField( "isDirty", TypeBool, Offset( mIsDirty, GuiConvexEditorCtrl ) );
	addField( "materialName", TypeString, Offset(mMaterialName, GuiConvexEditorCtrl) );

   Parent::initPersistFields();
}

bool GuiConvexEditorCtrl::onWake()
{
   if ( !Parent::onWake() )
      return false;

   SimGroup *missionGroup;
   if ( !Sim::findObject( "MissionGroup", missionGroup ) )
      return true;

   SimGroup::iterator itr = missionGroup->begin();
   for ( ; itr != missionGroup->end(); itr++ )
   {
      if ( dStrcmp( (*itr)->getClassName(), "ConvexShape" ) == 0 )
      {
         mConvexSEL = static_cast<ConvexShape*>( *itr );
         mGizmo->set( mConvexSEL->getTransform(), mConvexSEL->getPosition(), mConvexSEL->getScale() );
         return true;
      }
   }

   return true;   
}

void GuiConvexEditorCtrl::onSleep()
{
   Parent::onSleep();

   mConvexSEL = NULL;
   mConvexHL = NULL;   
}

void GuiConvexEditorCtrl::setVisible( bool val )
{
   //ConvexShape::smRenderEdges = value;

   if ( isProperlyAdded() )
   {
      if ( !val )
      {         
         mFaceHL = -1;
         mConvexHL = NULL;			

         setSelection( NULL, -1 );

         if ( mSavedGizmoFlags != -1 )
         {
            mGizmoProfile->flags = mSavedGizmoFlags;
            mSavedGizmoFlags = -1;
         }
      }
      else
      {
			mConvexHL = NULL;			
			mFaceHL = -1;

         setSelection( NULL, -1 );

			WorldEditor *wedit;
			if ( Sim::findObject( "EWorldEditor", wedit ) )
			{
				S32 count = wedit->getSelectionSize();
				for ( S32 i = 0; i < count; i++ )
				{
					S32 objId = wedit->getSelectObject(i);
					ConvexShape *pShape;
					if ( Sim::findObject( objId, pShape ) )
					{
						mConvexSEL = pShape;						
						wedit->clearSelection();
						wedit->selectObject( String::ToString("%i",objId) );
						break;
					}
				}
			}
         updateGizmoPos();
         mSavedGizmoFlags = mGizmoProfile->flags;
      }
   }

   Parent::setVisible( val );
}

void GuiConvexEditorCtrl::on3DMouseDown(const Gui3DMouseEvent & event)
{
   mouseLock();   

   mMouseDown = true;

   if ( event.modifier & SI_ALT )
   {
      setActiveTool( mCreateTool );
      mActiveTool->on3DMouseDown( event );
      return;
   }

   if ( mConvexSEL && isShapeValid( mConvexSEL ) )      
      mLastValidShape = mConvexSEL->mSurfaces;  

   if ( mConvexSEL &&
        mFaceSEL != -1 &&
        mGizmo->getMode() == RotateMode &&
        mGizmo->getSelection() == Gizmo::Centroid )
   {      
      mSettingPivot = true;      
      mSavedPivotPos = mGizmo->getPosition();
      setPivotPos( mConvexSEL, mFaceSEL, event );
      updateGizmoPos();
      return;
   }

   mGizmo->on3DMouseDown( event );   
}

void GuiConvexEditorCtrl::on3DRightMouseDown(const Gui3DMouseEvent & event)
{
   return;

	/*
   if ( mConvexSEL && mFaceSEL != -1 && mFaceSEL == mFaceHL )
   {
      _submitUndo( "Split ConvexShape face." );

      const MatrixF &surf = mConvexSEL->mSurfaces[mFaceSEL];

      MatrixF newSurf( surf );

      MatrixF rotMat( EulerF( 0.0f, mDegToRad( 2.0f ), 0.0f ) );

      newSurf *= rotMat;           

      mConvexSEL->mSurfaces.insert( mFaceSEL+1, newSurf );
   }
	*/
}

void GuiConvexEditorCtrl::on3DRightMouseUp(const Gui3DMouseEvent & event)
{
   //ConvexShape *hitShape;
   //S32 hitFace;
   //bool hit = _cursorCast( event, &hitShape, &hitFace );
   //Con::printf( hit ? "HIT" : "MISS" );
}

void GuiConvexEditorCtrl::on3DMouseUp(const Gui3DMouseEvent & event)
{
   mouseUnlock();

   mMouseDown = false;

   mHasCopied = false;
   mHasGeometry = false;   

   if ( mActiveTool )
   {
      ConvexEditorTool::EventResult result = mActiveTool->on3DMouseUp( event );

      if ( result == ConvexEditorTool::Done )      
         setActiveTool( NULL );         
      
      return;
   }

   if ( !mSettingPivot && !mDragging && ( mGizmo->getSelection() == Gizmo::None || !mConvexSEL ) )
   {
      if ( mConvexSEL != mConvexHL )
      {         
         setSelection( mConvexHL, -1 );
      }
      else
      {
         if ( mFaceSEL != mFaceHL )         
            setSelection( mConvexSEL, mFaceHL );         
         else
            setSelection( mConvexSEL, -1 );
      }

      mUsingPivot = false;
   }

   mSettingPivot = false;
   mSavedPivotPos = mGizmo->getPosition();
   mSavedUndo = false;   

   mGizmo->on3DMouseUp( event );

   if ( mDragging )
   {
      mDragging = false;

      if ( mConvexSEL )
      {         
         Vector< U32 > removedPlanes;
         mConvexSEL->cullEmptyPlanes( &removedPlanes );

         // If a face has been removed we need to validate / remap
         // our selected and highlighted faces.
         if ( !removedPlanes.empty() )
         {
            S32 prevFaceHL = mFaceHL;
            S32 prevFaceSEL = mFaceSEL;

            if ( removedPlanes.contains( mFaceHL ) )
               prevFaceHL = mFaceHL = -1;
            if ( removedPlanes.contains( mFaceSEL ) )
               prevFaceSEL = mFaceSEL = -1;
            
            for ( S32 i = 0; i < removedPlanes.size(); i++ )
            {
               if ( (S32)removedPlanes[i] < prevFaceSEL )
                  mFaceSEL--;               
               if ( (S32)removedPlanes[i] < prevFaceHL )
                  mFaceHL--;     
            }        

            setSelection( mConvexSEL, mFaceSEL );

            // We need to reindex faces.
            updateShape( mConvexSEL );
         }
      }
   }

   updateGizmoPos();   
}

void GuiConvexEditorCtrl::on3DMouseMove(const Gui3DMouseEvent & event)
{
   if ( mActiveTool )
   {
      // If we have an active tool pass this event to it.
      // If it handled it, consume the event.
      if ( mActiveTool->on3DMouseMove( event ) )
         return;
   }

   ConvexShape *hitShape = NULL;
   S32 hitFace = -1;
   
   _cursorCast( event, &hitShape, &hitFace );

   if ( !mConvexSEL )
   {
      mConvexHL = hitShape;
      mFaceHL = -1;
   }
   else
   {
      if ( mConvexSEL == hitShape )
      {
         mConvexHL = hitShape;        
         mFaceHL = hitFace;
      }
      else
      {
         // Mousing over a shape that is not the one currently selected.

         if ( mFaceSEL != -1 )
         {
            mFaceHL = -1;
         }
         else
         {
            mConvexHL = hitShape;
            mFaceHL = -1;
         } 
      }
   }

   if ( mConvexSEL )
      mGizmo->on3DMouseMove( event );
}

void GuiConvexEditorCtrl::on3DMouseDragged(const Gui3DMouseEvent & event)
{      
   if ( mActiveTool )
   {
      // If we have an active tool pass this event to it.
      // If it handled it, consume the event.
      if ( mActiveTool->on3DMouseDragged( event ) )
         return;
   }

   //mGizmoProfile->rotateScalar = 0.55f;
   //mGizmoProfile->scaleScalar = 0.55f;

   if ( !mConvexSEL )
      return;

   if ( mGizmo->getMode() == RotateMode &&
        mGizmo->getSelection() == Gizmo::Centroid )
   {            
      setPivotPos( mConvexSEL, mFaceSEL, event );      
      mDragging = true;
      return;
   }

   mGizmo->on3DMouseDragged( event );
      
   if ( event.modifier & SI_SHIFT && 
       ( mGizmo->getMode() == MoveMode || mGizmo->getMode() == RotateMode ) &&
        !mHasCopied )
   {
      if ( mFaceSEL != -1 )
      {
         ConvexShape *newShape = mCreateTool->extrudeShapeFromFace( mConvexSEL, mFaceSEL );
         //newShape->_updateGeometry();

         submitUndo( CreateShape, newShape );
         setSelection( newShape, 0 );         
         updateGizmoPos();

         mGizmo->on3DMouseDown( event );

         mHasCopied = true;
         mSavedUndo = true;
      }
      else
      {
         ConvexShape *newShape = new ConvexShape();
         newShape->setTransform( mConvexSEL->getTransform() );
         newShape->setScale( mConvexSEL->getScale() );
         newShape->mSurfaces.clear();
         newShape->mSurfaces.merge( mConvexSEL->mSurfaces );
         
         setupShape( newShape );

         submitUndo( CreateShape, newShape );

         setSelection( newShape, -1 );

         updateGizmoPos();

         mHasCopied = true;
         mSavedUndo = true;
      }

      return;
   }

   if ( mGizmo->getMode() == RotateMode &&
        event.modifier & SI_CTRL &&
        !mHasCopied &&
        mFaceSEL != -1 )
   {
      // Can must verify that splitting the face at the current angle 
      // ( of the gizmo ) will generate a valid shape.  If not enough rotation
      // has occurred we will have two faces that are coplanar and must wait
      // until later in the drag to perform the split.

      //AssertFatal( isShapeValid( mConvexSEL ), "Shape was already invalid at beginning of split operation." );

      if ( !isShapeValid( mConvexSEL ) )
         return;

      mLastValidShape = mConvexSEL->mSurfaces;

      Point3F rot = mGizmo->getDeltaTotalRot();
      rot.normalize();
      rot *= mDegToRad( 10.0f );

      MatrixF rotMat( (EulerF)rot );

      MatrixF worldToObj( mConvexSEL->getTransform() );
      worldToObj.scale( mConvexSEL->getScale() );
      worldToObj.inverse();      

      mConvexSEL->mSurfaces.increment();
      MatrixF &newSurf = mConvexSEL->mSurfaces.last();
      newSurf = mConvexSEL->mSurfaces[mFaceSEL] * rotMat;
      
      //worldToObj.mul( mGizmo->getTransform() );
      //Point3F pos( mPivotPos );
      //worldToObj.mulP( pos );
      //newSurf.setPosition( pos );

      updateShape( mConvexSEL );

      if ( !isShapeValid( mConvexSEL ) )
      {
         mConvexSEL->mSurfaces = mLastValidShape;
         updateShape( mConvexSEL );
      }
      else
      {
         mHasCopied = true;
         mSavedUndo = true;

         mLastValidShape = mConvexSEL->mSurfaces;

         submitUndo( ModifyShape, mConvexSEL );           

         setSelection( mConvexSEL, mConvexSEL->mSurfaces.size() - 1 );

         updateGizmoPos();
      }      
      
      return;
   }

   // If we are dragging, but no gizmo selection...
   // Then treat this like a regular mouse move, update the highlighted
   // convex/face under the cursor and handle onMouseUp as we normally would
   // to change the selection.
   if ( mGizmo->getSelection() == Gizmo::None )
   {
      ConvexShape *hitShape = NULL;
      S32 hitFace = -1;

      _cursorCast( event, &hitShape, &hitFace );
      mFaceHL = hitFace;
      mConvexHL = hitShape;      

      return;
   }

   mDragging = true;

   // Manipulating a face.

   if ( mFaceSEL != -1 )
   {
      if ( !mSavedUndo )
      {
         mSavedUndo = true;
         submitUndo( ModifyShape, mConvexSEL );
      }      

      if ( mGizmo->getMode() == ScaleMode )
      {
         scaleFace( mConvexSEL, mFaceSEL, mGizmo->getScale() );
      }
      else
      {
         // Why does this have to be so ugly.
         if ( mGizmo->getMode() == RotateMode || 
              ( mGizmo->getMode() == MoveMode  && 
                ( event.modifier & SI_CTRL  ||
                  ( mGizmo->getSelection() == Gizmo::Axis_Z && mHasCopied ) 
                )
              )
            )
         {
            const MatrixF &gMat = mGizmo->getTransform();      
            MatrixF surfMat;
            surfMat.mul( mConvexSEL->mWorldToObj, gMat );

            MatrixF worldToObj ( mConvexSEL->getTransform() );
            worldToObj.scale( mConvexSEL->getScale() );
            worldToObj.inverse();

            Point3F newPos;            
            newPos = gMat.getPosition();      

            worldToObj.mulP( newPos );
            surfMat.setPosition( newPos );
            
            // Clear out floating point errors.
            cleanMatrix( surfMat );

            mConvexSEL->mSurfaces[mFaceSEL] = surfMat;

            updateShape( mConvexSEL, mFaceSEL );         
         }
         else
         {
            // Translating a face in x/y/z

            translateFace( mConvexSEL, mFaceSEL, mGizmo->getTotalOffset() );
         }
      }

      if ( isShapeValid( mConvexSEL ) )          
      {
         AssertFatal( mConvexSEL->mSurfaces.size() > mFaceSEL, "mFaceSEL out of range." );
         mLastValidShape = mConvexSEL->mSurfaces; 
      }
      else
      {
         AssertFatal( mLastValidShape.size() > mFaceSEL, "mFaceSEL out of range." );
         mConvexSEL->mSurfaces = mLastValidShape;
         updateShape( mConvexSEL );
      }

      return;
   }

   // Manipulating a whole Convex.

   if ( !mSavedUndo )
   {
      mSavedUndo = true;
      submitUndo( ModifyShape, mConvexSEL );
   }

   if ( mGizmo->getMode() == MoveMode )
   {
      mConvexSEL->setPosition( mGizmo->getPosition() );
   }
   else if ( mGizmo->getMode() == RotateMode )
   {   
      mConvexSEL->setTransform( mGizmo->getTransform() );      
   }
   else
   {
      mConvexSEL->setScale( mGizmo->getScale() );
   }   

   if ( mConvexSEL->getClientObject() )
   {
      ConvexShape *clientObj = static_cast< ConvexShape* >( mConvexSEL->getClientObject() );
      clientObj->setTransform( mConvexSEL->getTransform() );
      clientObj->setScale( mConvexSEL->getScale() );
   }      
}

void GuiConvexEditorCtrl::on3DMouseEnter(const Gui3DMouseEvent & event)
{

}

void GuiConvexEditorCtrl::on3DMouseLeave(const Gui3DMouseEvent & event)
{

}

bool GuiConvexEditorCtrl::onKeyDown( const GuiEvent &evt )
{
   bool handled = false;

   switch ( evt.keyCode )
   {
   case KEY_ESCAPE:
      handled = handleEscape();      
      break;   
   case KEY_A:
      if ( evt.modifier & SI_ALT )
      {
         GizmoAlignment align = mGizmo->getProfile()->alignment;
         if ( align == World )
            mGizmo->getProfile()->alignment = Object;
         else
            mGizmo->getProfile()->alignment = World;
         handled = true;
      }
      break;
   case KEY_LCONTROL:
      //mCtrlDown = true;
      break;  
   default:
      break;
   }
   
   return handled;
}

bool GuiConvexEditorCtrl::onKeyUp( const GuiEvent &evt )
{
   bool handled = false;

   switch ( evt.keyCode )
   {      
   case KEY_LCONTROL:
      //mCtrlDown = false;
      break;   
   default:
      break;
   }

   return handled;
}

void GuiConvexEditorCtrl::get3DCursor( GuiCursor *&cursor, 
                                       bool &visible, 
                                       const Gui3DMouseEvent &event_ )
{
   //cursor = mAddNodeCursor;
   //visible = false;

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

void GuiConvexEditorCtrl::updateGizmo()
{
   mGizmoProfile->restoreDefaultState();
   
   const GizmoMode &mode = mGizmoProfile->mode;
   S32 &flags = mGizmoProfile->flags;
   GizmoAlignment &align = mGizmoProfile->alignment;

   U8 keys = Input::getModifierKeys();

   mCtrlDown = keys & ( SI_LCTRL | SI_LSHIFT );

   bool altDown = keys & ( SI_LALT );

   if ( altDown )
   {
      flags = 0;
      return;
   }

   if ( mFaceSEL != -1 )
   {
      align = Object;    
      flags |= GizmoProfile::CanRotateUniform;
      flags &= ~GizmoProfile::CanRotateScreen;
   }
   else
   {
      flags &= ~GizmoProfile::CanRotateUniform;
      flags |= GizmoProfile::CanRotateScreen;
   }

   if ( mFaceSEL != -1 && mode == ScaleMode )
      flags &= ~GizmoProfile::CanScaleZ;
   else
      flags |= GizmoProfile::CanScaleZ;         

   if ( mFaceSEL != -1 && mode == MoveMode )
   {
      if ( mCtrlDown )      
         flags &= ~( GizmoProfile::CanTranslateX | GizmoProfile::CanTranslateY | GizmoProfile::PlanarHandlesOn );      
      else      
         flags |= ( GizmoProfile::CanTranslateX | GizmoProfile::CanTranslateY | GizmoProfile::PlanarHandlesOn );      
   }
}

void GuiConvexEditorCtrl::renderScene(const RectI & updateRect)
{      
	// Synch selected ConvexShape with the WorldEditor.

	WorldEditor *wedit;
	if ( Sim::findObject( "EWorldEditor", wedit) )
	{
		S32 count = wedit->getSelectionSize();

		if ( !mConvexSEL && count != 0 )
			wedit->clearSelection();
		else if ( mConvexSEL && count != 1 )
		{
			wedit->clearSelection();
			wedit->selectObject( mConvexSEL->getIdString() );
		}
		else if ( mConvexSEL && count == 1 )
		{
			if ( wedit->getSelectObject(0) != mConvexSEL->getId() )
			{
				wedit->clearSelection();
				wedit->selectObject( mConvexSEL->getIdString() );
			}
		}
	}   

   // Update status bar text.

   SimObject *statusbar;
   if ( Sim::findObject( "EditorGuiStatusBar", statusbar ) )
   {
      String text( "Sketch Tool." );      
      GizmoMode mode = mGizmo->getMode();

      if ( mMouseDown && mGizmo->getSelection() != Gizmo::None && mConvexSEL )
      {
         Point3F delta;
         String qualifier;

         if ( mode == RotateMode )   
         {
            if ( mSettingPivot )            
               delta = mGizmo->getPosition() - mSavedPivotPos;
            else
               delta = mGizmo->getDeltaTotalRot();         
         }
         else if ( mode == MoveMode )         
            delta = mGizmo->getTotalOffset();         
         else if ( mode == ScaleMode )
            delta = mGizmo->getDeltaTotalScale();            
         
         if ( mGizmo->getAlignment() == Object && 
              mode != ScaleMode )
         {            
            mConvexSEL->mWorldToObj.mulV( delta );            
            if ( mFaceSEL != -1 && mode != RotateMode )
            {
               MatrixF objToSurf( mConvexSEL->mSurfaces[ mFaceSEL ] );
               objToSurf.scale( mConvexSEL->getScale() );
               objToSurf.inverse();
               objToSurf.mulV( delta );
            }
         }

         if ( mIsZero( delta.x, 0.0001f ) )
            delta.x = 0.0f;
         if ( mIsZero( delta.y, 0.0001f ) )
            delta.y = 0.0f;
         if ( mIsZero( delta.z, 0.0001f ) )
            delta.z = 0.0f;
         
         if ( mode == RotateMode )         
         {
            if ( mSettingPivot )            
               text = String::ToString( "Delta position ( x: %4.2f, y: %4.2f, z: %4.2f ).", delta.x, delta.y, delta.z ); 
            else
            {
               delta.x = mRadToDeg( delta.x );
               delta.y = mRadToDeg( delta.y );
               delta.z = mRadToDeg( delta.z );
               text = String::ToString( "Delta angle ( x: %4.2f, y: %4.2f, z: %4.2f ).", delta.x, delta.y, delta.z ); 
            }
         }
         else if ( mode == MoveMode )     
            text = String::ToString( "Delta position ( x: %4.2f, y: %4.2f, z: %4.2f ).", delta.x, delta.y, delta.z ); 
         else if ( mode == ScaleMode )
            text = String::ToString( "Delta scale ( x: %4.2f, y: %4.2f, z: %4.2f ).", delta.x, delta.y, delta.z ); 
      }
      else 
      {     
         if ( !mConvexSEL )
            text = "Sketch Tool.  ALT + Click-Drag to create a new ConvexShape.";
         else if ( mFaceSEL == -1 )
         {
            if ( mode == MoveMode )            
               text = "Move selection.  SHIFT while dragging duplicates objects.";
            else if ( mode == RotateMode )            
               text = "Rotate selection.";
            else if ( mode == ScaleMode )            
               text = "Scale selection.";        
         }
         else 
         {
            if ( mode == MoveMode )            
               text = "Move face.  SHIFT while beginning a drag EXTRUDES a new convex. Press CTRL for alternate translation mode.";
            else if ( mode == RotateMode )            
               text = "Rotate face.  Gizmo/Pivot is draggable. CTRL while dragging splits/folds a new face. SHIFT while dragging extrudes a new convex.";
            else if ( mode == ScaleMode )            
            text = "Scale face.";
         }
      }
   
      // Issue a warning in the status bar
      // if this convex has an excessive number of surfaces...
      if ( mConvexSEL && mConvexSEL->getSurfaces().size() > ConvexShape::smMaxSurfaces )
      {
          text = "WARNING: Reduce the number of surfaces on the selected ConvexShape, only the first 100 will be saved!";
      }

      Con::executef( statusbar, "setInfo", text.c_str() );

	Con::executef( statusbar, "setSelectionObjectsByCount", Con::getIntArg( mConvexSEL == NULL ? 0 : 1 ) );
   }   

   if ( mActiveTool )
      mActiveTool->renderScene( updateRect );

   ColorI colorHL( 255, 50, 255, 255 );
   ColorI colorSEL( 255, 50, 255, 255 );
   ColorI colorNA( 255, 255, 255, 100 );

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   if ( mConvexSEL && !mDragging )
   {
      if ( mFaceSEL == -1 )
      {
         GFXStateBlockDesc desc;
         desc.setBlend( true );
         desc.setZReadWrite( true, true );

         Box3F objBox = mConvexSEL->getObjBox();
         objBox.scale( mConvexSEL->getScale() );

         const MatrixF &objMat = mConvexSEL->getTransform();

         Point3F boxPos = objBox.getCenter();
         objMat.mulP( boxPos );
         
         drawer->drawObjectBox( desc, objBox.getExtents(), boxPos, objMat, ColorI::WHITE );
      }
      else
      {
         mConvexSEL->renderFaceEdges( -1, colorNA );     

         drawFacePlane( mConvexSEL, mFaceSEL );         
      }

      if ( mConvexHL == mConvexSEL &&
           mFaceHL != -1 && 
           mFaceHL != mFaceSEL && 
           mGizmo->getSelection() == Gizmo::None )
      {
         mConvexSEL->renderFaceEdges( mFaceHL, colorHL );
      }
   }

   if ( mConvexHL && mConvexHL != mConvexSEL )
   {
      mConvexHL->renderFaceEdges( -1 );      
   }

   if ( mGizmo->getMode() != RotateMode && mUsingPivot )
   {
      mUsingPivot = false;
      updateGizmoPos();
   }

   F32 gizmoAlpha = 1.0f;
	if ( !mConvexSEL )
		gizmoAlpha = 0.0f;

   if ( mMouseDown && mGizmo->getSelection() != Gizmo::None && mConvexSEL )
   {
      if ( mSettingPivot )
         gizmoAlpha = 1.0f;
      else
         gizmoAlpha = 0.0f;
   }

   DebugDrawer::get()->render();

   {
      GFXTransformSaver saver;
      // Now draw all the 2d stuff!
      GFX->setClipRect(updateRect); 

      if ( mConvexSEL && mFaceSEL != -1 )
      {      
         Vector< Point3F > lineList;
         mConvexSEL->getSurfaceLineList( mFaceSEL, lineList );

         MatrixF objToWorld( mConvexSEL->getTransform() );
         objToWorld.scale( mConvexSEL->getScale() );      

         for ( S32 i = 0; i < lineList.size(); i++ )     
            objToWorld.mulP( lineList[i] );			

         for ( S32 i = 0; i < lineList.size() - 1; i++ )
         {
			   Point3F p0( lineList[i] );
			   Point3F p1( lineList[i+1] );

			   drawLine( p0, p1, colorSEL, 3.0f );
         }
	   }

      if ( gizmoAlpha == 1.0f )
      {
         if ( mGizmoProfile->mode != NoneMode )
            mGizmo->renderText( mSaveViewport, mSaveModelview, mSaveProjection );   	
      }

      if ( mActiveTool )
         mActiveTool->render2D();
   }

   if ( gizmoAlpha == 1.0f )   
      mGizmo->renderGizmo( mLastCameraQuery.cameraMatrix, mLastCameraQuery.fov );
} 

void GuiConvexEditorCtrl::drawFacePlane( ConvexShape *shape, S32 faceId )
{
   // Build a vb of the face points ( in world space ) scaled outward in
   // the surface space in x/y with uv coords.

   /*
   Vector< Point3F > points;
   Vector< Point2F > coords;

   shape->getSurfaceTriangles( faceId, &points, &coords, false );

   if ( points.empty() )
      return;

   GFXVertexBufferHandle< GFXVertexPCT > vb;
   vb.set( GFX, points.size(), GFXBufferTypeVolatile );
   GFXVertexPCT *vert = vb.lock();

   for ( S32 i = 0; i < points.size(); i++ )
   {
      vert->point = points[i];
      vert->color.set( 255, 255, 255, 200 );
      vert->texCoord = coords[i];
      vert++;
   }

   vb.unlock();

   GFXTransformSaver saver;
   MatrixF renderMat( shape->getTransform() );
   renderMat.scale( shape->getScale() );
   GFX->multWorld( renderMat );

   GFXStateBlockDesc desc;
   desc.setBlend( true );
   desc.setCullMode( GFXCullNone );
   desc.setZReadWrite( true, false );
   desc.samplersDefined = true;
   desc.samplers[0] = GFXSamplerStateDesc::getWrapLinear();
   GFX->setStateBlockByDesc( desc );

   GFX->setVertexBuffer( vb );

   GFXTexHandle tex( "core/art/grids/512_transp", &GFXDefaultStaticDiffuseProfile, "ConvexEditor_grid" );
   GFX->setTexture( 0, tex );
   GFX->setupGenericShaders();
   GFX->drawPrimitive( GFXTriangleList, 0, points.size() / 3 );
   */
}


void GuiConvexEditorCtrl::scaleFace( ConvexShape *shape, S32 faceId, Point3F scale )
{
   if ( !mHasGeometry )
   {
      mHasGeometry = true;
      
      mSavedGeometry = shape->mGeometry;
      mSavedSurfaces = shape->mSurfaces;      
   }
   else
   {
      shape->mGeometry = mSavedGeometry;
      shape->mSurfaces = mSavedSurfaces;
   }

   if ( shape->mGeometry.faces.size() <= faceId )
      return;
   
   ConvexShape::Face &face = shape->mGeometry.faces[faceId];

   Vector< Point3F > &pointList = shape->mGeometry.points;

   AssertFatal( shape->mSurfaces[ face.id ].isAffine(), "ConvexShapeEditor - surface not affine." );
      
   Point3F projScale;
   scale.z = 1.0f;

   const MatrixF &surfToObj = shape->mSurfaces[ face.id ];
   MatrixF objToSurf( surfToObj );
   objToSurf.inverse();

   for ( S32 i = 0; i < face.points.size(); i++ )
   {                  
      Point3F &pnt = pointList[ face.points[i] ];   

      objToSurf.mulP( pnt );
      pnt *= scale;
      surfToObj.mulP( pnt );
   }

   updateModifiedFace( shape, faceId );
}

void GuiConvexEditorCtrl::translateFace( ConvexShape *shape, S32 faceId, const Point3F &displace )
{
   if ( !mHasGeometry )
   {
      mHasGeometry = true;

      mSavedGeometry = shape->mGeometry;
      mSavedSurfaces = shape->mSurfaces;      
   }
   else
   {
      shape->mGeometry = mSavedGeometry;
      shape->mSurfaces = mSavedSurfaces;
   }

   if ( shape->mGeometry.faces.size() <= faceId )
      return;

   ConvexShape::Face &face = shape->mGeometry.faces[faceId];

   Vector< Point3F > &pointList = shape->mGeometry.points;

   AssertFatal( shape->mSurfaces[ face.id ].isAffine(), "ConvexShapeEditor - surface not affine." );

   // Transform displacement into object space.    
   MatrixF worldToObj( shape->getTransform() );
   worldToObj.scale( shape->getScale() );
   worldToObj.inverse();

   Point3F displaceOS;
   worldToObj.mulV( displace, &displaceOS );

   for ( S32 i = 0; i < face.points.size(); i++ )
   {                  
      Point3F &pnt = pointList[ face.points[i] ];   
      pnt += displaceOS;      
   }

   updateModifiedFace( shape, faceId );
}

void GuiConvexEditorCtrl::updateModifiedFace( ConvexShape *shape, S32 faceId )
{
   if ( shape->mGeometry.faces.size() <= faceId )
      return;

   ConvexShape::Face &face = shape->mGeometry.faces[faceId];

   Vector< Point3F > &pointList = shape->mGeometry.points;

   Vector< ConvexShape::Face > &faceList = shape->mGeometry.faces;

   for ( S32 i = 0; i < faceList.size(); i++ )
   {
      ConvexShape::Face &curFace = faceList[i];      
      MatrixF &curSurface = shape->mSurfaces[ curFace.id ];

      U32 curPntCount = curFace.points.size();

      if ( curPntCount < 3 )
         continue;

      // Does this face use any of the points which we have modified?
      // Collect them in correct winding order.

      S32 pId0 = -1;

      for ( S32 j = 0; j < curFace.winding.size(); j++ )
      {
         if ( face.points.contains( curFace.points[ curFace.winding[ j ] ] ) )
         {
            pId0 = j;
            break;
         }
      }         

      if ( pId0 == -1 )
         continue;

      S32 pId1 = -1, pId2 = -1;

      pId1 = ( pId0 + 1 ) % curFace.winding.size();
      pId2 = ( pId0 + 2 ) % curFace.winding.size();

      const Point3F &p0 = pointList[ curFace.points[ curFace.winding[ pId0 ] ] ];
      const Point3F &p1 = pointList[ curFace.points[ curFace.winding[ pId1 ] ] ];
      const Point3F &p2 = pointList[ curFace.points[ curFace.winding[ pId2 ] ] ];

      PlaneF newPlane( p0, p1, p2 );
      Point3F uvec = newPlane.getNormal();
      Point3F fvec = curSurface.getForwardVector();
      Point3F rvec = curSurface.getRightVector();

      F32 dt0 = mDot( uvec, fvec );
      F32 dt1 = mDot( uvec, rvec );

      if ( mFabs( dt0 ) < mFabs( dt1 ) )
      {
         rvec = mCross( fvec, uvec );
         rvec.normalizeSafe();
         fvec = mCross( uvec, rvec );
         fvec.normalizeSafe();
      }
      else
      {
         fvec = mCross( uvec, rvec );
         fvec.normalizeSafe();
         rvec = mCross( fvec, uvec );
         rvec.normalizeSafe();
      }

      curSurface.setColumn( 0, rvec );
      curSurface.setColumn( 1, fvec );
      curSurface.setColumn( 2, uvec );   
      curSurface.setPosition( newPlane.getPosition() );
   }

   updateShape( shape );
}

bool GuiConvexEditorCtrl::isShapeValid( ConvexShape *shape )
{
   // Test for no-geometry.
   if ( shape->mGeometry.points.empty() )
      return false;

   const Vector<Point3F> &pointList = shape->mGeometry.points;
   const Vector<ConvexShape::Face> &faceList = shape->mGeometry.faces;

   // Test that all points are shared by at least 3 faces.

   for ( S32 i = 0; i < pointList.size(); i++ )
   {
      U32 counter = 0;

      for ( S32 j = 0; j < faceList.size(); j++ )
      {
         if ( faceList[j].points.contains( i ) )
            counter++;
      }

      if ( counter < 3 )
         return false;
   }

   // Test for co-planar faces.
   for ( S32 i = 0; i < shape->mPlanes.size(); i++ )
   {
      for ( S32 j = i + 1; j < shape->mPlanes.size(); j++ )
      {
         F32 d = mDot( shape->mPlanes[i], shape->mPlanes[j] );
         if ( d > 0.999f )         
            return false;         
      }
   }

   // Test for faces with zero or negative area.
   for ( S32 i = 0; i < shape->mGeometry.faces.size(); i++ )
   {
      if ( shape->mGeometry.faces[i].area < 0.0f )
         return false;

      if ( shape->mGeometry.faces[i].triangles.empty() )
         return false;
   }

   return true;
}

void GuiConvexEditorCtrl::setupShape( ConvexShape *shape )
{
   shape->setField( "material", mMaterialName );
   shape->registerObject();
   updateShape( shape );

   SimGroup *group;
   if ( Sim::findObject( "missionGroup", group ) )
      group->addObject( shape );
}

void GuiConvexEditorCtrl::updateShape( ConvexShape *shape, S32 offsetFace )
{
   shape->_updateGeometry( true );

   /*
   if ( offsetFace != -1 )
   {
      shape->mSurfaces[ offsetFace ].setPosition( mPivotPos );
   }*/

   synchClientObject( shape );
}

void GuiConvexEditorCtrl::synchClientObject( const ConvexShape *serverConvex )
{
   if ( serverConvex->getClientObject() )
   {
      ConvexShape *clientConvex = static_cast< ConvexShape* >( serverConvex->getClientObject() );
      clientConvex->setScale( serverConvex->getScale() );
      clientConvex->setTransform( serverConvex->getTransform() );
      clientConvex->mSurfaces.clear();
      clientConvex->mSurfaces.merge( serverConvex->mSurfaces );
      clientConvex->_updateGeometry(true);
   }
}

void GuiConvexEditorCtrl::updateGizmoPos()
{
   if ( mConvexSEL )
   {
      if ( mFaceSEL != -1 )
      {
         MatrixF surfMat = mConvexSEL->getSurfaceWorldMat( mFaceSEL );  

         MatrixF objToWorld( mConvexSEL->getTransform() );
         objToWorld.scale( mConvexSEL->getScale() );

         Point3F gizmoPos(0,0,0);

         if ( mUsingPivot )
         {
            gizmoPos = mPivotPos;
         }
         else
         {
            Point3F faceCenterPnt = mConvexSEL->mSurfaces[ mFaceSEL ].getPosition();
            objToWorld.mulP( faceCenterPnt );

            mGizmoMatOffset = surfMat.getPosition() - faceCenterPnt;

            gizmoPos = faceCenterPnt;
         }

         mGizmo->set( surfMat, gizmoPos, Point3F::One );        
      }
      else
      {
         mGizmoMatOffset = Point3F::Zero;
         mGizmo->set( mConvexSEL->getTransform(), mConvexSEL->getPosition(), mConvexSEL->getScale() ); 
      }
   }   
}

bool GuiConvexEditorCtrl::setActiveTool( ConvexEditorTool *tool )
{   
   if ( mActiveTool == tool )
      return false;

   ConvexEditorTool *prevTool = mActiveTool;
   ConvexEditorTool *newTool = tool;

   if ( prevTool )
      prevTool->onDeactivated( newTool );

   mActiveTool = newTool;

   if ( newTool )
      newTool->onActivated( prevTool );

   return true;
}

bool GuiConvexEditorCtrl::handleEscape()
{
   if ( mActiveTool )
   {
      mActiveTool->onDeactivated( NULL );
      mActiveTool = NULL;

      return true;
   }

   if ( mFaceSEL != -1 )
   {
      setSelection( mConvexSEL, -1 );
      return true;
   }

   if ( mConvexSEL )
   {         
      setSelection( NULL, -1 );
      return true;
   }

   return false;
}

bool GuiConvexEditorCtrl::handleDelete()
{
   if ( mActiveTool )
   {
      mActiveTool->onDeactivated( NULL );
      mActiveTool = NULL;
   }

   if ( mConvexSEL )
   {
      if ( mFaceSEL != -1 )
      {
         submitUndo( ModifyShape, mConvexSEL );

         mConvexSEL->mSurfaces.erase_fast( mFaceSEL );
         updateShape( mConvexSEL );

         if ( !isShapeValid( mConvexSEL ) )
         {
            S32 selFace = mFaceSEL;
            mLastUndo->undo();
            mFaceSEL = selFace;
            updateShape( mConvexSEL );
            updateGizmoPos();
         }
         else
         {
            setSelection( mConvexSEL, -1 );
         }
      }
      else
      {
         // Grab the mission editor undo manager.
         UndoManager *undoMan = NULL;
         if ( !Sim::findObject( "EUndoManager", undoMan ) )
         {            
            Con::errorf( "GuiConvexEditorCtrl::on3DMouseDown() - EUndoManager not found!" );    
         }
         else
         {
            // Create the UndoAction.
            MEDeleteUndoAction *action = new MEDeleteUndoAction("Deleted ConvexShape");
            action->deleteObject( mConvexSEL );
            mIsDirty = true;
            
            mFaceHL = -1; 

            setSelection( NULL, -1 );

            // Submit it.               
            undoMan->addAction( action );
         }
      }
   }

   return true;
}

bool GuiConvexEditorCtrl::hasSelection() const
{
   return mConvexSEL != NULL;   
}

void GuiConvexEditorCtrl::clearSelection()
{
   mFaceHL = -1;
   mConvexHL = NULL;
   setSelection( NULL, -1 );
}

void GuiConvexEditorCtrl::handleDeselect()
{
   if ( mActiveTool )
   {
      mActiveTool->onDeactivated( NULL );
      mActiveTool = NULL;
   }

   mFaceHL = -1;
   mConvexHL = NULL;
   setSelection( NULL, -1 );
}

void GuiConvexEditorCtrl::setSelection( ConvexShape *shape, S32 faceId )
{
   mFaceSEL = faceId;
   mConvexSEL = shape;
   updateGizmoPos();

   Con::executef( this, "onSelectionChanged", shape ? shape->getIdString() : "", Con::getIntArg(faceId) );
}

void GuiConvexEditorCtrl::_prepRenderImage( SceneManager* sceneGraph, const SceneRenderState* state )
{   
   if ( !isAwake() )
      return;

   /*
   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   ri->type = RenderPassManager::RIT_Editor;
   ri->renderDelegate.bind( this, &GuiConvexEditorCtrl::_renderObject );
   ri->defaultKey = 100;
   state->getRenderPass()->addInst( ri );   
   */
}

void GuiConvexEditorCtrl::_renderObject( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *matInst )
{  
}

void GuiConvexEditorCtrl::submitUndo( UndoType type, ConvexShape *shape )
{
   Vector< ConvexShape* > shapes;
   shapes.push_back( shape );
   submitUndo( type, shapes );
}

void GuiConvexEditorCtrl::submitUndo( UndoType type, const Vector<ConvexShape*> &shapes )
{   
   // Grab the mission editor undo manager.
   Sim::findObject( "EUndoManager", mUndoManager );   
   
   if ( !mUndoManager )   
   {
      Con::errorf( "GuiConvexEditorCtrl::submitUndo() - EUndoManager not found!" );
      return;           
   }

   if ( type == ModifyShape )
   {
      // Setup the action.
      GuiConvexEditorUndoAction *action = new GuiConvexEditorUndoAction( "Modified a ConvexShape" );

      ConvexShape *shape = shapes.first();

      action->mObjId = shape->getId();   
      action->mEditor = this;   
      action->mSavedObjToWorld = shape->getTransform();
      action->mSavedScale = shape->getScale();
      action->mSavedSurfaces.merge( shape->mSurfaces );             
      action->mUndoManager = mUndoManager;

      mUndoManager->addAction( action );

      mLastUndo = action;
   }
   else if ( type == CreateShape )
   {
      MECreateUndoAction *action = new MECreateUndoAction( "Create ConvexShape" );

      for ( S32 i = 0; i < shapes.size(); i++ )
         action->addObject( shapes[i] );
         
      mUndoManager->addAction( action );
      
      mLastUndo = action;
   }
   else if ( type == DeleteShape )
   {
      MEDeleteUndoAction *action = new MEDeleteUndoAction( "Deleted ConvexShape" );

      for ( S32 i = 0; i < shapes.size(); i++ )
         action->deleteObject( shapes[i] );         

      mUndoManager->addAction( action );

      mLastUndo = action;
   }
   else if ( type == HollowShape )
   {
      CompoundUndoAction *action = new CompoundUndoAction( "Hollow ConvexShape" );

      MECreateUndoAction *createAction = new MECreateUndoAction();
      MEDeleteUndoAction *deleteAction = new MEDeleteUndoAction();

      deleteAction->deleteObject( shapes.first() );
      
      for ( S32 i = 1; i < shapes.size(); i++ )      
         createAction->addObject( shapes[i] );
      
      action->addAction( deleteAction );
      action->addAction( createAction );

      mUndoManager->addAction( action );

      mLastUndo = action;
   }

	mIsDirty = true;
}

bool GuiConvexEditorCtrl::_cursorCastCallback( RayInfo* ri )
{
   // Reject anything that's not a ConvexShape.
   return dynamic_cast< ConvexShape* >( ri->object );
}

bool GuiConvexEditorCtrl::_cursorCast( const Gui3DMouseEvent &event, ConvexShape **hitShape, S32 *hitFace )
{
   RayInfo ri;
   
   if ( gServerContainer.castRay( event.pos, event.pos + event.vec * 10000.0f, StaticShapeObjectType, &ri, &GuiConvexEditorCtrl::_cursorCastCallback ) &&
        dynamic_cast< ConvexShape* >( ri.object ) )
   {
      // Do not select or edit ConvexShapes that are within a Prefab.
      if ( Prefab::getPrefabByChild( ri.object ) )
         return false;

      *hitShape = static_cast< ConvexShape* >( ri.object );
      *hitFace = ri.face;
      mLastRayInfo = ri;

      return true;
   }

   return false;
}

void GuiConvexEditorCtrl::setPivotPos( ConvexShape *shape, S32 faceId, const Gui3DMouseEvent &event )
{
   PlaneF plane;
   mTransformPlane( shape->getTransform(), shape->getScale(), shape->mPlanes[ faceId ], &plane );

   Point3F start( event.pos );
   Point3F end( start + event.vec * 10000.0f );

   F32 t = plane.intersect( start, end );

   if ( t >= 0.0f && t <= 1.0f )
   {
      Point3F hitPos;
      hitPos.interpolate( start, end, t );

      mPivotPos = hitPos;
      mUsingPivot = true;

      MatrixF worldToObj( shape->getTransform() );
      worldToObj.scale( shape->getScale() );
      worldToObj.inverse();

      Point3F objPivotPos( mPivotPos );
      worldToObj.mulP( objPivotPos );

      updateGizmoPos();
   }
}

void GuiConvexEditorCtrl::cleanMatrix( MatrixF &mat )
{
   if ( mat.isAffine() )
      return;

   VectorF col0 = mat.getColumn3F(0);
   VectorF col1 = mat.getColumn3F(1);
   VectorF col2 = mat.getColumn3F(2);

   col0.normalize();
   col1.normalize();
   col2.normalize();

   col2 = mCross( col0, col1 );
   col2.normalize();
   col1 = mCross( col2, col0 );
   col1.normalize();
   col0 = mCross( col1, col2 );
   col0.normalize();

   mat.setColumn(0,col0);
   mat.setColumn(1,col1);
   mat.setColumn(2,col2);

   AssertFatal( mat.isAffine(), "GuiConvexEditorCtrl::cleanMatrix, non-affine matrix" );
}

S32 GuiConvexEditorCtrl::getEdgeByPoints( ConvexShape *shape, S32 faceId, S32 p0, S32 p1 )
{
   const ConvexShape::Face &face = shape->mGeometry.faces[faceId];

   for ( S32 i = 0; i < face.edges.size(); i++ )
   {
      const ConvexShape::Edge &edge = face.edges[i];

      if ( edge.p0 != p0 && edge.p0 != p1 )
         continue;
      if ( edge.p1 != p0 && edge.p1 != p1 )
         continue;

      return i;      
   }

   return -1;
}

bool GuiConvexEditorCtrl::getEdgesTouchingPoint( ConvexShape *shape, S32 faceId, S32 pId, Vector< U32 > &edgeIdxList, S32 excludeEdge )
{
   const ConvexShape::Face &face = shape->mGeometry.faces[faceId];   
   const Vector< ConvexShape::Edge > &edgeList = face.edges;

   for ( S32 i = 0; i < edgeList.size(); i++ )
   {
      if ( i == excludeEdge )
         continue;

      const ConvexShape::Edge &curEdge = edgeList[i];

      if ( curEdge.p0 == pId || curEdge.p1 == pId )      
         edgeIdxList.push_back(i);
   }

   return !edgeIdxList.empty();
}

void GuiConvexEditorUndoAction::undo()
{
   ConvexShape *object = NULL;
   if ( !Sim::findObject( mObjId, object ) )
      return;

   // Temporarily save the ConvexShape current data.   
   Vector< MatrixF > tempSurfaces;   
   tempSurfaces.merge( object->mSurfaces );
   MatrixF tempObjToWorld( object->getTransform() );
   Point3F tempScale( object->getScale() );

   // Restore the Object to the UndoAction state.
   object->mSurfaces.clear();
   object->mSurfaces.merge( mSavedSurfaces );   
   object->setScale( mSavedScale );
   object->setTransform( mSavedObjToWorld );

   // Regenerate the ConvexShape and synch the client object.
   object->_updateGeometry();
   GuiConvexEditorCtrl::synchClientObject( object );

   // If applicable set the selected ConvexShape and face
   // on the editor.   
   mEditor->setSelection( object, -1 );
   mEditor->updateGizmoPos();

   // Now save the previous ConvexShape data in this UndoAction
   // since an undo action must become a redo action and vice-versa
   
   mSavedObjToWorld = tempObjToWorld;
   mSavedScale = tempScale;
   mSavedSurfaces.clear();
   mSavedSurfaces.merge( tempSurfaces );   
}

ConvexEditorCreateTool::ConvexEditorCreateTool( GuiConvexEditorCtrl *editor )
 : Parent( editor ),
   mStage( -1 ),
   mNewConvex( NULL )
{
}

void ConvexEditorCreateTool::onActivated( ConvexEditorTool *prevTool )
{
   mEditor->clearSelection();
   mStage = -1;
   mNewConvex = NULL;
}

void ConvexEditorCreateTool::onDeactivated( ConvexEditorTool *newTool )
{
   if ( mNewConvex )
      mNewConvex->deleteObject();

   mStage = -1;
   mNewConvex = NULL;
   mEditor->mouseUnlock();
}

ConvexEditorTool::EventResult ConvexEditorCreateTool::on3DMouseDown( const Gui3DMouseEvent &event )
{
   if ( mStage == -1 )
   {
      mEditor->setFirstResponder();
      mEditor->mouseLock();

      Point3F start( event.pos );
      Point3F end( event.pos + event.vec * 10000.0f );      
      RayInfo ri;
      
      bool hit = gServerContainer.castRay( event.pos, end, STATIC_COLLISION_TYPEMASK, &ri );

      MatrixF objMat( true );

      // Calculate the orientation matrix of the new ConvexShape
      // based on what has been clicked.

      if ( !hit )
      {
         objMat.setPosition( event.pos + event.vec * 100.0f );      
      }
      else
      {
         if ( dynamic_cast< ConvexShape* >( ri.object ) )
         {
            ConvexShape *hitShape = static_cast< ConvexShape* >( ri.object );
            objMat = hitShape->getSurfaceWorldMat( ri.face );
            objMat.setPosition( ri.point );
         }
         else
         {
            Point3F rvec;
            Point3F fvec( mEditor->getCameraMat().getForwardVector() );
            Point3F uvec( ri.normal );

            rvec = mCross( fvec, uvec );

            if ( rvec.isZero() )
            {
               fvec = mEditor->getCameraMat().getRightVector();
               rvec = mCross( fvec, uvec );
            }

            rvec.normalizeSafe();
            fvec = mCross( uvec, rvec );
            fvec.normalizeSafe();
            uvec = mCross( rvec, fvec );
            uvec.normalizeSafe();

            objMat.setColumn( 0, rvec );
            objMat.setColumn( 1, fvec );
            objMat.setColumn( 2, uvec );

            objMat.setPosition( ri.point );
         }
      }

      mNewConvex = new ConvexShape();                       

      mNewConvex->setTransform( objMat );   
		
		mNewConvex->setField( "material", Parent::mEditor->mMaterialName );
		
      mNewConvex->registerObject();
      mPlaneSizes.set( 0.1f, 0.1f, 0.1f );
      mNewConvex->resizePlanes( mPlaneSizes );
      mEditor->updateShape( mNewConvex );
      
      mTransform = objMat;     

      mCreatePlane.set( objMat.getPosition(), objMat.getUpVector() );
   }
   else if ( mStage == 0 )
   {
      // Handle this on mouseUp
   }
   
   return Handled;
}

ConvexEditorTool::EventResult ConvexEditorCreateTool::on3DMouseUp( const Gui3DMouseEvent &event )
{
   if ( mNewConvex && mStage == -1 )
   {
      mStage = 0;      

      mCreatePlane = PlaneF( mNewConvex->getPosition(), mNewConvex->getTransform().getForwardVector() );

      mTransform.setPosition( mNewConvex->getPosition() );      

      return Handled;
   }
   else if ( mStage == 0 )
   {
      SimGroup *mg;
      Sim::findObject( "MissionGroup", mg );

      mg->addObject( mNewConvex );

      mStage = -1;

      // Grab the mission editor undo manager.
      UndoManager *undoMan = NULL;
      if ( !Sim::findObject( "EUndoManager", undoMan ) )
      {
         Con::errorf( "ConvexEditorCreateTool::on3DMouseDown() - EUndoManager not found!" );
         mNewConvex = NULL;
         return Failed;           
      }

      // Create the UndoAction.
      MECreateUndoAction *action = new MECreateUndoAction("Create ConvexShape");
      action->addObject( mNewConvex );

      // Submit it.               
      undoMan->addAction( action );

      mEditor->setField( "isDirty", "1" );

      mEditor->setSelection( mNewConvex, -1 );      

      mNewConvex = NULL;

      mEditor->mouseUnlock();

      return Done;
   }

   return Done;
}

ConvexEditorTool::EventResult ConvexEditorCreateTool::on3DMouseMove( const Gui3DMouseEvent &event )
{
   if ( mStage == 0 )
   {
      Point3F start( event.pos );
      Point3F end( start + event.vec * 10000.0f );
      
      F32 t = mCreatePlane.intersect( start, end );

      Point3F hitPos;

      if ( t < 0.0f || t > 1.0f )
         return Handled;

      hitPos.interpolate( start, end, t );      

      MatrixF worldToObj( mTransform );
      worldToObj.inverse();
      worldToObj.mulP( hitPos );

      F32 delta = ( hitPos.z );

      mPlaneSizes.z = getMax( 0.1f, delta );

      mNewConvex->resizePlanes( mPlaneSizes );

      mEditor->updateShape( mNewConvex );

      Point3F pos( mTransform.getPosition() );
      pos += mPlaneSizes.z * 0.5f * mTransform.getUpVector();
      mNewConvex->setPosition( pos );
   }

   return Handled;
}

ConvexEditorTool::EventResult ConvexEditorCreateTool::on3DMouseDragged( const Gui3DMouseEvent &event )
{
   if ( !mNewConvex || mStage != -1 )
      return Handled;

   Point3F start( event.pos );
   Point3F end( event.pos + event.vec * 10000.0f );

   F32 t = mCreatePlane.intersect( start, end );

   if ( t < 0.0f || t > 1.0f )
      return Handled;

   Point3F hitPos;
   hitPos.interpolate( start, end, t );
   
   MatrixF xfm( mTransform );
   xfm.inverse();      
   xfm.mulP( hitPos);      
   
   Point3F scale;
   scale.x = getMax( mFabs( hitPos.x ), 0.1f );
   scale.y = getMax( mFabs( hitPos.y ), 0.1f );
   scale.z = 0.1f;

   mNewConvex->resizePlanes( scale );
   mPlaneSizes = scale;
   mEditor->updateShape( mNewConvex );   

   Point3F pos( mTransform.getPosition() );
   pos += mTransform.getRightVector() * hitPos.x * 0.5f;
   pos += mTransform.getForwardVector() * hitPos.y * 0.5f;

   mNewConvex->setPosition( pos );

   return Handled;
}

void ConvexEditorCreateTool::renderScene( const RectI &updateRect )
{
  
}

ConvexShape* ConvexEditorCreateTool::extrudeShapeFromFace( ConvexShape *inShape, S32 inFaceId )
{
   ConvexShape::Geometry &inShapeGeometry = inShape->getGeometry();
   ConvexShape::Face &inFace = inShapeGeometry.faces[inFaceId];
   Vector< Point3F > &inShapePointList = inShapeGeometry.points;
   Vector< MatrixF > &inShapeSurfaces = inShape->getSurfaces();
   
   S32 shapeFaceCount = inFace.edges.size() + 2;   
   
   MatrixF inShapeToWorld( inShape->getTransform() );
   inShapeToWorld.scale( inShape->getScale() );
   //MatrixF inWorldToShape( inShapeToWorld );
   //inWorldToShape.inverse();

   MatrixF shapeToWorld;
   shapeToWorld.mul( inShape->getTransform(), inShapeSurfaces[inFaceId] );
   Point3F tmp( inShapeSurfaces[inFaceId].getPosition() );
   inShapeToWorld.mulP( tmp );
   shapeToWorld.setPosition( tmp );
   MatrixF worldToShape( shapeToWorld );
   worldToShape.inverse();

   MatrixF inShapeToNewShape;
   inShapeToNewShape.mul( inShapeToWorld, worldToShape );   

   ConvexShape *newShape = new ConvexShape;   
   newShape->setTransform( shapeToWorld );   

   Vector< MatrixF > &shapeSurfaces = newShape->getSurfaces();
   shapeSurfaces.setSize( shapeFaceCount );
   //shapeSurfaces.setSize( 2 );

   const Point3F &shapePos = shapeToWorld.getPosition();
   
   shapeSurfaces[0].identity();
   shapeSurfaces[1].identity();   
   shapeSurfaces[1].setColumn( 0, -shapeSurfaces[1].getColumn3F(0) );
   shapeSurfaces[1].setColumn( 2, -shapeSurfaces[1].getColumn3F(2) );

   for ( S32 i = 0; i < inFace.winding.size(); i++ )
   {      
      Point3F p0 = inShapePointList[ inFace.points[ inFace.winding[ i ] ] ];
      Point3F p1;
      
      if ( i+1 < inFace.winding.size() )
         p1 = inShapePointList[ inFace.points[ inFace.winding[ i+1 ] ] ];
      else
         p1 = inShapePointList[ inFace.points[ inFace.winding[ 0 ] ] ];

      inShapeToWorld.mulP( p0 );
      inShapeToWorld.mulP( p1 );

      Point3F newPos = MathUtils::mClosestPointOnSegment( p0, p1, shapePos );      

      Point3F rvec = p0 - p1;
      rvec.normalizeSafe();

      Point3F fvec = shapeToWorld.getUpVector();

      Point3F uvec = mCross( rvec, fvec );      

      if ( i + 2 >= shapeSurfaces.size() )
         continue;
      
      //F32 dt = mDot( shapeToWorld.getUpVector(), rvec );
      //AssertFatal( mIsZero( dt ), "bad" );
      
      MatrixF &surf = shapeSurfaces[i+2];
      surf.identity();
      surf.setColumn( 0, rvec );
      surf.setColumn( 1, fvec );
      surf.setColumn( 2, uvec );
      surf.setPosition( newPos );

      surf.mulL( worldToShape );      
   }

	newShape->setField( "material", Parent::mEditor->mMaterialName );

   newShape->registerObject();
   mEditor->updateShape( newShape );

   SimGroup *group;
   if ( Sim::findObject( "missionGroup", group ) )
      group->addObject( newShape );

   return newShape;
}

void GuiConvexEditorCtrl::hollowShape( ConvexShape *shape, F32 thickness )
{
   // Create a new Convex for each face of the original shape.
   // This is the same as an extrude from face operation going inward by the thickness
   // for every face.

   Vector< ConvexShape* > convexList;

   for ( S32 i = 0; i < shape->mGeometry.faces.size(); i++ )
   {
      ConvexShape *faceShape = mCreateTool->extrudeShapeFromFace( shape, i );
      MatrixF &inwardFace = faceShape->mSurfaces[1];
      //MatrixF &outwardFace = faceShape->mSurfaces[0];      

      Point3F invec = inwardFace.getUpVector();

      inwardFace.setPosition( inwardFace.getPosition() + invec * thickness );

      updateShape( faceShape );

      convexList.push_back( faceShape );
   }

   convexList.push_front( shape );
   submitUndo( HollowShape, convexList );   
}

void GuiConvexEditorCtrl::hollowSelection()
{
   if ( mConvexSEL )
   {
      hollowShape( mConvexSEL, 0.15f );
      setSelection( NULL, -1 );      
   }
}

void GuiConvexEditorCtrl::recenterSelection()
{
   if ( mConvexSEL )    
   {
      recenterShape( mConvexSEL );   
      updateGizmoPos();
   }
}

void GuiConvexEditorCtrl::recenterShape( ConvexShape *shape )
{
   submitUndo( ModifyShape, shape );
   shape->recenter();
   synchClientObject( shape );
}

void GuiConvexEditorCtrl::dropSelectionAtScreenCenter()
{
   // This code copied from WorldEditor.
   // All the dropping code would be moved to somewhere common, but its not.

   if ( !mConvexSEL )
      return;

   // Calculate the center of the screen (in global screen coordinates)
   Point2I offset = localToGlobalCoord(Point2I(0,0));
   Point3F sp(F32(offset.x + F32(getExtent().x / 2)), F32(offset.y + (getExtent().y / 2)), 1.0f);

   // Calculate the view distance to fit the selection
   // within the camera's view.
   const Box3F bounds = mConvexSEL->getWorldBox();
   F32 radius = bounds.len()*0.5f;
   F32 viewdist = calculateViewDistance(radius);

   // Be careful of infinite sized objects, or just large ones in general.
   if(viewdist > 100.0f )
      viewdist = 100.0f;

   // Position the selection   
   mConvexSEL->setPosition( smCamPos + smCamMatrix.getForwardVector() * viewdist );

   synchClientObject( mConvexSEL );

   updateGizmoPos();
}

void GuiConvexEditorCtrl::splitSelectedFace()
{
   if ( !mConvexSEL || mFaceSEL == -1 )
      return;

   if ( !isShapeValid( mConvexSEL ) )
      return;

   mLastValidShape = mConvexSEL->mSurfaces;

   const F32 radians = mDegToRad( 15.0f );
   Point3F rot( 0, 0, 0 );
   MatrixF rotMat( true );

   mConvexSEL->mSurfaces.increment();
   MatrixF &dstMat = mConvexSEL->mSurfaces.last();   
   const MatrixF &srcMat = mConvexSEL->mSurfaces[mFaceSEL];

   for ( S32 i = 0; i < 6; i++ )
   {
      F32 sign = i > 2 ? -1.0f : 1.0f;
      U32 idx = i % 3;

      rot.zero();
      rot[idx] = sign * radians;
      rotMat.set( (EulerF)rot );

      dstMat = srcMat * rotMat;

      updateShape( mConvexSEL );

      if ( isShapeValid( mConvexSEL ) )
      {
         mSavedSurfaces = mConvexSEL->mSurfaces;
         mConvexSEL->mSurfaces = mLastValidShape;

         submitUndo( ModifyShape, mConvexSEL );  

         mConvexSEL->mSurfaces = mSavedSurfaces;
         mLastValidShape = mSavedSurfaces;

         setSelection( mConvexSEL, mConvexSEL->mSurfaces.size() - 1 );

         return;
      }      
   }

   mConvexSEL->mSurfaces = mLastValidShape;
   updateShape( mConvexSEL );
   updateGizmoPos();
}

DefineConsoleMethod( GuiConvexEditorCtrl, hollowSelection, void, (), , "" )
{
   object->hollowSelection();
}

DefineConsoleMethod( GuiConvexEditorCtrl, recenterSelection, void, (), , "" )
{
   object->recenterSelection();
}

DefineConsoleMethod( GuiConvexEditorCtrl, hasSelection, S32, (), , "" )
{
   return object->hasSelection();
}

DefineConsoleMethod( GuiConvexEditorCtrl, handleDelete, void, (), , "" )
{
   object->handleDelete();
}

DefineConsoleMethod( GuiConvexEditorCtrl, handleDeselect, void, (), , "" )
{
   object->handleDeselect();
}

DefineConsoleMethod( GuiConvexEditorCtrl, dropSelectionAtScreenCenter, void, (), , "" )
{
   object->dropSelectionAtScreenCenter();
}

DefineConsoleMethod( GuiConvexEditorCtrl, selectConvex, void, (ConvexShape *convex), , "( ConvexShape )" )
{
if (convex)
      object->setSelection( convex, -1 );
}

DefineConsoleMethod( GuiConvexEditorCtrl, splitSelectedFace, void, (), , "" )
{
   object->splitSelectedFace();
}