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
#include "environment/editors/guiRiverEditorCtrl.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "environment/river.h"
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
#include "T3D/prefab.h"

IMPLEMENT_CONOBJECT(GuiRiverEditorCtrl);

ConsoleDocClass( GuiRiverEditorCtrl,
   "@brief GUI tool that makes up the River Editor\n\n"
   "Editor use only.\n\n"
   "@internal"
);

GuiRiverEditorCtrl::GuiRiverEditorCtrl()
 : mDefaultWidth( 10.0f ),
   mDefaultDepth( 5.0f ),
   mDefaultNormal( 0, 0, 1 )
{   
	// Each of the mode names directly correlates with the River Editor's
	// tool palette
   mSelectRiverMode = "RiverEditorSelectMode";
	mAddRiverMode = "RiverEditorAddRiverMode";
	mMovePointMode = "RiverEditorMoveMode";
	mRotatePointMode = "RiverEditorRotateMode";
	mScalePointMode = "RiverEditorScaleMode";
	mAddNodeMode = "RiverEditorAddNodeMode";
	mInsertPointMode = "RiverEditorInsertPointMode";
	mRemovePointMode = "RiverEditorRemovePointMode";
   
	mMode = mSelectRiverMode;
   
   mRiverSet = NULL;   
   mSelNode = -1;
   mSelRiver = NULL;
   mHoverRiver = NULL;
   mAddNodeIdx = 0;
   mHoverNode = -1;

   mInsertIdx = -1;

   mStartWidth = -1.0f;
   mStartHeight = -1.0f;
   mStartX = 0;

   mIsDirty = false;

   mNodeHalfSize.set(4,4);

   mNodeSphereRadius = 15.0f;
   mNodeSphereFillColor.set( 15,15,100,145 );
   mNodeSphereLineColor.set( 25,25,25,0 );
   mHoverSplineColor.set( 255,0,0,255 );
   mSelectedSplineColor.set( 0,255,0,255 );
   mHoverNodeColor.set( 255,255,255,255 );

   mStartDragMousePoint = InvalidMousePoint;
   //mMoveNodeCursor = NULL;
   //mAddNodeCursor = NULL;
   //mInsertNodeCursor = NULL;
   //mResizeNodeCursor = NULL;
}

GuiRiverEditorCtrl::~GuiRiverEditorCtrl()
{
   // nothing to do
}

void GuiRiverEditorUndoAction::undo()
{
   River *river = NULL;
   if ( !Sim::findObject( mObjId, river ) )
      return;

   // Temporarily save the Rivers current data.
   F32 metersPerSeg = river->mMetersPerSegment;
   Vector<RiverNode> nodes;   
   nodes.merge( river->mNodes );

   // Restore the River properties saved in the UndoAction
   river->mMetersPerSegment = mMetersPerSegment;

   // Restore the Nodes saved in the UndoAction
   river->mNodes.clear();
   for ( U32 i = 0; i < mNodes.size(); i++ )
   {
      river->_addNode( mNodes[i].point, mNodes[i].width, mNodes[i].depth, mNodes[i].normal );      
   }

   // Regenerate the River
   river->regenerate();

   // If applicable set the selected River and node
   mRiverEditor->mSelRiver = river;
   mRiverEditor->mSelNode = -1;

   // Now save the previous River data in this UndoAction
   // since an undo action must become a redo action and vice-versa
   mMetersPerSegment = metersPerSeg;
   mNodes.clear();
   mNodes.merge( nodes );
}

bool GuiRiverEditorCtrl::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   mRiverSet = River::getServerSet();

   GFXStateBlockDesc desc;
   desc.fillMode = GFXFillSolid;      
   desc.setBlend( false );
   desc.setZReadWrite( false, false );
   desc.setCullMode( GFXCullNone );   

   mZDisableSB = GFX->createStateBlock(desc);

   desc.setZReadWrite( true, true );
   mZEnableSB = GFX->createStateBlock(desc);

   SceneManager::getPreRenderSignal().notify( this, &GuiRiverEditorCtrl::_prepRenderImage );

   return true;
}

void GuiRiverEditorCtrl::initPersistFields()
{
   addField( "DefaultWidth",        TypeF32,    Offset( mDefaultWidth, GuiRiverEditorCtrl ) );
	addField( "DefaultDepth",        TypeF32,    Offset( mDefaultDepth, GuiRiverEditorCtrl ) );
	addField( "DefaultNormal",       TypePoint3F,Offset( mDefaultNormal, GuiRiverEditorCtrl ) );
   addField( "HoverSplineColor",    TypeColorI, Offset( mHoverSplineColor, GuiRiverEditorCtrl ) );
   addField( "SelectedSplineColor", TypeColorI, Offset( mSelectedSplineColor, GuiRiverEditorCtrl ) );
   addField( "HoverNodeColor",      TypeColorI, Offset( mHoverNodeColor, GuiRiverEditorCtrl ) );
   addField( "isDirty",             TypeBool,   Offset( mIsDirty, GuiRiverEditorCtrl ) );
   //addField( "MoveNodeCursor", TYPEID< SimObject >(), Offset( mMoveNodeCursor, GuiRiverEditorCtrl) );
   //addField( "AddNodeCursor", TYPEID< SimObject >(), Offset( mAddNodeCursor, GuiRiverEditorCtrl) );
   //addField( "InsertNodeCursor", TYPEID< SimObject >(), Offset( mInsertNodeCursor, GuiRiverEditorCtrl) );
   //addField( "ResizeNodeCursor", TYPEID< SimObject >(), Offset( mResizeNodeCursor, GuiRiverEditorCtrl) );

   Parent::initPersistFields();
}

void GuiRiverEditorCtrl::onSleep()
{
   Parent::onSleep();

   mMode = mSelectRiverMode;  
   mHoverNode = -1;
   mHoverRiver = NULL;
   setSelectedNode(-1);
   //mSelRiver = NULL;
   //mSelNode = -1;
}

void GuiRiverEditorCtrl::get3DCursor( GuiCursor *&cursor, 
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

void GuiRiverEditorCtrl::on3DMouseDown(const Gui3DMouseEvent & event)
{
   _process3DMouseDown( event );

   mGizmo->on3DMouseDown( event );

   if ( !isFirstResponder() )
      setFirstResponder();
}

void GuiRiverEditorCtrl::_process3DMouseDown( const Gui3DMouseEvent& event )
{
	// Get the raycast collision position
   Point3F tPos;
   if ( !getStaticPos( event, tPos ) )
		return;  
		
   mouseLock();

   // Construct a LineSegment from the camera position to 1000 meters away in
   // the direction clicked.
   // If that segment hits the terrain, truncate the ray to only be that length.

   // We will use a LineSegment/Sphere intersection test to determine if a RiverNode
   // was clicked.   

   Point3F startPnt = event.pos;
   Point3F endPnt = event.pos + event.vec * 1000.0f;

   RayInfo ri;   

   if ( gServerContainer.castRay(startPnt, endPnt, StaticShapeObjectType, &ri) )
      endPnt = ri.point;

   River *riverPtr = NULL;
   River *clickedRiverPtr = NULL;

   // Did we click on a river? check current selection first
   U32 insertNodeIdx = -1;
   Point3F collisionPnt;
   if ( mSelRiver != NULL && mSelRiver->collideRay( event.pos, event.vec, &insertNodeIdx, &collisionPnt ) )
   {
      clickedRiverPtr = mSelRiver;
   }
   else
   {
      for ( SimSetIterator iter(mRiverSet); *iter; ++iter )
      {
         riverPtr = static_cast<River*>( *iter );

         // Do not select or edit a River within a Prefab.
         if ( Prefab::getPrefabByChild(riverPtr) )
            continue;

         if ( riverPtr->collideRay( event.pos, event.vec, &insertNodeIdx, &collisionPnt ) )
         {
            clickedRiverPtr = riverPtr;
            break;
         }
      }
   }

   // Did we click on a riverNode?
   bool nodeClicked = false;   
   S32 clickedNodeIdx = -1;
   F32 clickedNodeDist = mNodeSphereRadius;

   // If we clicked on the currently selected river, only scan its nodes
   if ( mSelRiver != NULL && clickedRiverPtr == mSelRiver )
   {
      for ( U32 i = 0; i < mSelRiver->mNodes.size(); i++ )
      {
         const Point3F &nodePos = mSelRiver->mNodes[i].point;

         Point3F screenPos;
         project( nodePos, &screenPos );

         F32 dist = ( event.mousePoint - Point2I(screenPos.x, screenPos.y) ).len();
         if ( dist < clickedNodeDist )
         {
            clickedNodeDist = dist;
            clickedNodeIdx = i;
            insertNodeIdx = i;
            nodeClicked = true;
         }
      }
   }
   else
   {
      for ( SimSetIterator iter(mRiverSet); *iter; ++iter )
      {
         riverPtr = static_cast<River*>( *iter );

         // Do not select or edit a River within a Prefab.
         if ( Prefab::getPrefabByChild(riverPtr) )
            continue;
         
         for ( U32 i = 0; i < riverPtr->mNodes.size(); i++ )
         {
            const Point3F &nodePos = riverPtr->mNodes[i].point;

            Point3F screenPos;
            project( nodePos, &screenPos );

            F32 dist = ( event.mousePoint - Point2I(screenPos.x, screenPos.y) ).len();
            if ( dist < clickedNodeDist )
            {
               // we found a hit!
               clickedNodeDist = dist;
               clickedNodeIdx = i;
               insertNodeIdx = i;
               nodeClicked = true;
               clickedRiverPtr = riverPtr;
            }
         }
      }
   }
	
	// shortcuts
	bool dblClick = ( event.mouseClickCount > 1 );
	if( dblClick )
   { 
		if( mMode == mSelectRiverMode )
		{
			setMode( mAddRiverMode, true );
			return;
		}
		if( mMode == mAddNodeMode )
		{
			// Delete the node attached to the cursor.
			deleteSelectedNode();
			mMode = mAddRiverMode;
			return;
		}
	}

	//this check is here in order to bounce back from deleting a whole road with ctrl+z
	//this check places the editor back into addrivermode
	if ( mMode == mAddNodeMode )
	{
      if ( !mSelRiver )
         mMode = mAddRiverMode;
	}

	if ( mMode == mSelectRiverMode )
	{
      // Did not click on a River or a node.
      if ( !clickedRiverPtr  )
      {
         setSelectedRiver( NULL );
         setSelectedNode( -1 );
         
         return;
      }

      // Clicked on a River that wasn't the currently selected River.
      if ( clickedRiverPtr != mSelRiver )
      {
         setSelectedRiver( clickedRiverPtr );
         setSelectedNode( clickedNodeIdx );
         return;
      }

     // Clicked on a node in the currently selected River that wasn't
      // the currently selected node.
      if ( nodeClicked )
      {
         setSelectedNode( clickedNodeIdx );
         return;
      }
	}
   else if ( mMode == mAddRiverMode )
   {
		if ( nodeClicked )
      {
			// A double-click on a node in Normal mode means set AddNode mode.  
         if ( clickedNodeIdx == 0 )
         {
				setSelectedRiver( clickedRiverPtr );
				setSelectedNode( clickedNodeIdx );

				mAddNodeIdx = clickedNodeIdx;
            mMode = mAddNodeMode; 

            mSelNode = mSelRiver->insertNode( tPos, mDefaultWidth, mDefaultDepth, mDefaultNormal, mAddNodeIdx );
            mIsDirty = true;

				return;
         }
			else if ( clickedNodeIdx == clickedRiverPtr->mNodes.size() - 1 )
         {
				setSelectedRiver( clickedRiverPtr );
				setSelectedNode( clickedNodeIdx );

            mAddNodeIdx = U32_MAX;
				mMode = mAddNodeMode;

            mSelNode = mSelRiver->addNode( tPos, mDefaultWidth, mDefaultDepth, mDefaultNormal);
            mIsDirty = true;
				setSelectedNode( mSelNode );

				return;
         } 
		}

		if ( !isMethod( "createRiver" ) )
      {
			Con::errorf( "GuiRiverEditorCtrl::on3DMouseDown - createRiver method does not exist." );
         return;
      }

      const char *res = Con::executef( this, "createRiver" );

      River *newRiver;
      if ( !Sim::findObject( res, newRiver ) )
      {
         Con::errorf( "GuiRiverEditorCtrl::on3DMouseDown - createRiver method did not return a river object." );
         return;
      }                

      // Add to MissionGroup                              
      SimGroup *missionGroup;
      if ( !Sim::findObject( "MissionGroup", missionGroup ) )               
         Con::errorf( "GuiRiverEditorCtrl - could not find MissionGroup to add new River" );
      else
         missionGroup->addObject( newRiver );

      Point3F pos( endPnt );
      pos.z += mDefaultDepth * 0.5f;

      newRiver->insertNode( pos, mDefaultWidth, mDefaultDepth, mDefaultNormal, 0 );
      U32 newNode = newRiver->insertNode( pos, mDefaultWidth, mDefaultDepth, mDefaultNormal, 1 );

      // Always add to the end of the road, the first node is the start.
      mAddNodeIdx = U32_MAX;
      
      setSelectedRiver( newRiver );      
      setSelectedNode( newNode );

      mMode = mAddNodeMode;

      // Disable the hover node while in addNodeMode, we
      // don't want some random node enlarged.
      mHoverNode = -1;

      // Grab the mission editor undo manager.
      UndoManager *undoMan = NULL;
      if ( !Sim::findObject( "EUndoManager", undoMan ) )
      {
         Con::errorf( "GuiMeshRoadEditorCtrl::on3DMouseDown() - EUndoManager not found!" );
         return;           
      }

      // Create the UndoAction.
      MECreateUndoAction *action = new MECreateUndoAction("Create MeshRoad");
      action->addObject( newRiver );

      // Submit it.               
      undoMan->addAction( action );

		return;
   }
	else if ( mMode == mAddNodeMode )
	{
		// Oops the road got deleted, maybe from an undo action?
      // Back to NormalMode.
      if ( mSelRiver )
      {
			// A double-click on a node in Normal mode means set AddNode mode.  
         if ( clickedNodeIdx == 0 )
         {
				submitUndo( "Add Node" );
				mAddNodeIdx = clickedNodeIdx;
            mMode = mAddNodeMode;
            mSelNode = mSelRiver->insertNode( tPos, mDefaultWidth, mDefaultDepth, mDefaultNormal, mAddNodeIdx );
            mIsDirty = true;
				setSelectedNode( mSelNode );

				return;
         }
			else
         {
				if( clickedRiverPtr && clickedNodeIdx == clickedRiverPtr->mNodes.size() - 1 )
				{
					submitUndo( "Add Node" );
					mAddNodeIdx = U32_MAX;
					mMode = mAddNodeMode;
					U32 newNode = mSelRiver->addNode( tPos, mDefaultWidth, mDefaultDepth, mDefaultNormal);  
               mIsDirty = true;
					setSelectedNode( newNode );

					return;
				}
				else
				{
					submitUndo( "Insert Node" );
					// A single-click on empty space while in
					// AddNode mode means insert / add a node.
					//submitUndo( "Add Node" );
					//F32 width = mSelRiver->getNodeWidth( mSelNode );
					U32 newNode = mSelRiver->insertNode( tPos, mDefaultWidth, mDefaultDepth, mDefaultNormal, mAddNodeIdx);
               mIsDirty = true;
					setSelectedNode( newNode );

					return;
				}
			}
		}
	}
	else if ( mMode == mInsertPointMode && mSelRiver != NULL )
	{
		if ( clickedRiverPtr == mSelRiver )
      {
			// NOTE: I guess we have to determine the if the clicked ray intersects a road but not a specific node...
         // in order to handle inserting nodes in the same way as for DecalRoad

         U32 prevNodeIdx = insertNodeIdx;
         U32 nextNodeIdx = ( prevNodeIdx + 1 > mSelRiver->mNodes.size() - 1 ) ? prevNodeIdx : prevNodeIdx + 1;

         const RiverNode &prevNode = mSelRiver->mNodes[prevNodeIdx];
         const RiverNode &nextNode = mSelRiver->mNodes[nextNodeIdx];

         F32 width = ( prevNode.width + nextNode.width ) * 0.5f;
         F32 depth = ( prevNode.depth + nextNode.depth ) * 0.5f;
         Point3F normal = ( prevNode.normal + nextNode.normal ) * 0.5f;
         normal.normalize();

         submitUndo( "Insert Node" );
         U32 newNode = mSelRiver->insertNode( collisionPnt, width, depth, normal, insertNodeIdx + 1 );
         mIsDirty = true;
         setSelectedNode( newNode );

			return;
       }
	}
	else if ( mMode == mRemovePointMode && mSelRiver != NULL )
	{
		if ( nodeClicked && clickedRiverPtr == mSelRiver )
      {
			setSelectedNode( clickedNodeIdx );
         deleteSelectedNode();
         return;
      }
	}
	else if ( mMode == mMovePointMode )
	{
		if ( nodeClicked && clickedRiverPtr == mSelRiver )
      {
			setSelectedNode( clickedNodeIdx );
         return;
      }
	}
	else if ( mMode == mScalePointMode )
	{
		if ( nodeClicked && clickedRiverPtr == mSelRiver )
      {
			setSelectedNode( clickedNodeIdx );
         return;
      }
	}
	else if ( mMode == mRotatePointMode )
	{
		if ( nodeClicked && clickedRiverPtr == mSelRiver )
      {
			setSelectedNode( clickedNodeIdx );
         return;
      }
	}
}

void GuiRiverEditorCtrl::on3DRightMouseDown(const Gui3DMouseEvent & event)
{
   //mIsPanning = true;
}

void GuiRiverEditorCtrl::on3DRightMouseUp(const Gui3DMouseEvent & event)
{
   //mIsPanning = false;
}

void GuiRiverEditorCtrl::on3DMouseUp(const Gui3DMouseEvent & event)
{
   // Keep the Gizmo up to date.
   mGizmo->on3DMouseUp( event );

   mStartWidth = -1.0f;     
   mStartHeight = -1.0f;
   mSavedDrag = false;

   mouseUnlock();
}

void GuiRiverEditorCtrl::on3DMouseMove(const Gui3DMouseEvent & event)
{
   if ( mSelRiver != NULL && mMode == mAddNodeMode )
   {
      Point3F pos;
      if ( getStaticPos( event, pos ) )         
      {
         pos.z += mSelRiver->getNodeDepth(mSelNode) * 0.5f;
         mSelRiver->setNodePosition( mSelNode, pos );
         mIsDirty = true;
      }

      return;
   }

   if ( mSelRiver != NULL && mSelNode != -1 )
      mGizmo->on3DMouseMove( event );

   // Is cursor hovering over a river?
   if ( mMode == mSelectRiverMode )
   {
      mHoverRiver = NULL;

      Point3F startPnt = event.pos;
      Point3F endPnt = event.pos + event.vec * 1000.0f;

      RayInfo ri;   

      if ( gServerContainer.castRay(startPnt, endPnt, StaticShapeObjectType, &ri) )
         endPnt = ri.point;

      River *pRiver = NULL;

      for ( SimSetIterator iter(mRiverSet); *iter; ++iter )
      {
         pRiver = static_cast<River*>( *iter );

         // Do not select or edit a River within a Prefab.
         if ( Prefab::getPrefabByChild(pRiver) )
            continue;

         if ( pRiver->collideRay( event.pos, event.vec ) )
         {
            mHoverRiver = pRiver;
            break;
         }
      }      
   }

   // Is cursor hovering over a RiverNode?
   if ( mHoverRiver )
   {      
      River *pRiver = mHoverRiver;

      S32 hoverNodeIdx = -1;
      F32 hoverNodeDist = mNodeSphereRadius;

      //for ( SimSetIterator iter(mRiverSet); *iter; ++iter )
      //{
      //   River *pRiver = static_cast<River*>( *iter );

         for ( U32 i = 0; i < pRiver->mNodes.size(); i++ )
         {
            const Point3F &nodePos = pRiver->mNodes[i].point;

            Point3F screenPos;
            project( nodePos, &screenPos );

            F32 dist = ( event.mousePoint - Point2I(screenPos.x, screenPos.y) ).len();
            if ( dist < hoverNodeDist )
            {
               // we found a hit!
               hoverNodeDist = dist;
               hoverNodeIdx = i;
            }           
         }      
      //}  

      mHoverNode = hoverNodeIdx;
   }
}

void GuiRiverEditorCtrl::on3DMouseDragged(const Gui3DMouseEvent & event)
{
   // Drags are only used to transform nodes
   if ( !mSelRiver || mSelNode == -1 ||
      ( mMode != mMovePointMode && mMode != mScalePointMode && mMode != mRotatePointMode ) )
      return;

   // If we haven't already saved,
   // save an undo action to get back to this state,
   // before we make any modifications to the selected node.
   if ( !mSavedDrag )
   {
      submitUndo( "Modify Node" );
      mSavedDrag = true;
   }

   // Let the gizmo handle the drag, eg, modify its transforms
   mGizmo->on3DMouseDragged( event );
   if ( mGizmo->isDirty() )
   {
      Point3F pos = mGizmo->getPosition();
      Point3F scale = mGizmo->getScale();      
      const MatrixF &mat = mGizmo->getTransform();
      VectorF normal;
      mat.getColumn( 2, &normal );

      mSelRiver->setNode( pos, scale.x, scale.z, normal, mSelNode );
      mIsDirty = true;
   }
   Con::executef( this, "onNodeModified", Con::getIntArg(mSelNode) );	
   /*
   // If we are just starting a new drag,
   // we need to save the starting screen position of the mouse,
   // and the starting position of the selected node.
   if ( mStartDragMousePoint == InvalidMousePoint )
   {
      mStartDragMousePoint = event.mousePoint;
      mStartDragNodePos = mSelRiver->getNodePosition( mSelNode );
   }

   MathUtils::Line clickLine;
   clickLine.p = event.pos;
   clickLine.d = event.vec;

   MathUtils::Line axisLine;
   axisLine.p = mStartDragNodePos;
   axisLine.d = mGizmo.selectionToAxisVector( mGizmoSelection );

   MathUtils::LineSegment segment;

   MathUtils::mShortestSegmentBetweenLines( clickLine, axisLine, segment );

   // Segment.p1 is the closest point on the axis line, 
   // We want to put the selected gizmo handle at that point,
   // So calculate the offset from the handle to the centerPoint to
   // determine the gizmo's position.
   mSelRiver->setNodePosition( mSelNode, segment.p1 );
   */

   /*
   // Convert the delta (dragged mouse distance) from screen space
   // into world space.
   Point2I deltaScreen = event.mousePoint - mStartDragMousePoint;

   F32 worldDist = ( event.pos - mStartDragNodePos ).len();      
   
   Point2F deltaWorld;
   deltaWorld.x = GFX->unprojectRadius( worldDist, deltaScreen.x );
   deltaWorld.y = GFX->unprojectRadius( worldDist, deltaScreen.y );

   // Now modify the selected node depending on the kind of operation we are doing.
   if ( mGizmoSelection == Gizmo::Axis_X )
   {
      Point3F newPos = mStartDragNodePos;
      newPos.x += deltaWorld.x;      
      mSelRiver->setNodePosition( mSelNode, newPos );
   }
   else if ( mGizmoSelection == Gizmo::Axis_Y )
   {
      Point3F newPos = mStartDragNodePos;
      newPos.y += deltaWorld.x;      
      mSelRiver->setNodePosition( mSelNode, newPos );
   }
   else if ( mGizmoSelection == Gizmo::Axis_Z )
   {
      Point3F newPos = mStartDragNodePos;
      newPos.z += deltaWorld.y;      
      mSelRiver->setNodePosition( mSelNode, newPos );
   }
   */

   /*
   F32 height = mStartHeight + deltaWorldX;    
   Con::printf( "height = %g", height );

   mSelRiver->setNodeHeight( mSelNode, height );

   Con::executef( this, "onNodeHeightModified", Con::getFloatArg(height) );


   if ( event.modifier & SI_PRIMARY_CTRL )
   {
      //Point3F tPos;
      //if ( !getStaticPos( event, tPos ) )
      //   return;  

      if ( mStartHeight == -1.0f )
      {
         mStartHeight = mSelRiver->mNodes[mSelNode].point.z;

         mStartX = event.mousePoint.x;
         mStartWorld = mSelRiver->mNodes[mSelNode].point;
      }

      S32 deltaScreenX = event.mousePoint.x - mStartX;

      F32 worldDist = ( event.pos - mStartWorld ).len();      

      F32 deltaWorldX = GFX->unprojectRadius( worldDist, deltaScreenX );

      F32 height = mStartHeight + deltaWorldX;    
      Con::printf( "height = %g", height );

      mSelRiver->setNodeHeight( mSelNode, height );

      Con::executef( this, "onNodeHeightModified", Con::getFloatArg(height) );
   }
   else if ( event.modifier & SI_SHIFT )
   {
      Point3F tPos;
      if ( !getStaticPos( event, tPos ) )
         return;   

      if ( mStartWidth == -1.0f )
      {
         mStartWidth = mSelRiver->mNodes[mSelNode].width;
         
         mStartX = event.mousePoint.x;
         mStartWorld = tPos;
      }

      S32 deltaScreenX = event.mousePoint.x - mStartX;
      
      F32 worldDist = ( event.pos - mStartWorld ).len();      

      F32 deltaWorldX = GFX->unprojectRadius( worldDist, deltaScreenX );

      F32 width = mStartWidth + deltaWorldX;      

      mSelRiver->setNodeWidth( mSelNode, width );

      Con::executef( this, "onNodeWidthModified", Con::getFloatArg(width) );
   }
   else
   {    
      Point3F tPos;
      if ( !getStaticPos( event, tPos ) )
         return; 
      else if ( mGizmoSelection == Gizmo::Axis_Y )
      {
         Point3F newPos = mStartDragNodePos;
         newPos.y += deltaWorld.x;      
         mSelRiver->setNodePosition( mSelNode, newPos );
      }
      mSelRiver->setNodePosition( mSelNode, tPos );
   }   
   */
}

void GuiRiverEditorCtrl::on3DMouseEnter(const Gui3DMouseEvent & event)
{
   // nothing to do
}

void GuiRiverEditorCtrl::on3DMouseLeave(const Gui3DMouseEvent & event)
{
   // nothing to do
}

bool GuiRiverEditorCtrl::onKeyDown(const GuiEvent& event)
{
	if( event.keyCode == KEY_RETURN && mMode == mAddNodeMode )
   {
		// Delete the node attached to the cursor.
		deleteSelectedNode();
		mMode = mAddRiverMode;
		return true;
	}

	return false;
}

void GuiRiverEditorCtrl::updateGuiInfo()
{
   // nothing to do
}
      
void GuiRiverEditorCtrl::onRender( Point2I offset, const RectI &updateRect )
{
   PROFILE_SCOPE( GuiRiverEditorCtrl_OnRender );

   Parent::onRender( offset, updateRect );
   return;
}
      
void GuiRiverEditorCtrl::renderScene(const RectI & updateRect)
{
   //GFXDrawUtil *drawer = GFX->getDrawUtil();            

   GFX->setStateBlock( mZDisableSB );

   // get the projected size...
   GameConnection* connection = GameConnection::getConnectionToServer();
   if(!connection)
      return;

   // Grab the camera's transform
   MatrixF mat;
   connection->getControlCameraTransform(0, &mat);

   // Get the camera position
   Point3F camPos;
   mat.getColumn(3,&camPos);

   if ( mHoverRiver && mHoverRiver != mSelRiver )
   {
      _drawRiverSpline( mHoverRiver, mHoverSplineColor );
   }

   if ( mSelRiver )
   {
      _drawRiverSpline( mSelRiver, mSelectedSplineColor );            

      // Render Gizmo for selected node if were in either of the three transform modes
      if ( mSelNode != -1 && ( mMode == mMovePointMode || mMode == mScalePointMode || mMode == mRotatePointMode ) )
      {
			if( mMode == mMovePointMode )
			{
				mGizmo->getProfile()->mode = MoveMode;
			}
			else if( mMode == mScalePointMode )
			{
				mGizmo->getProfile()->mode = ScaleMode;
			}
			else if( mMode == mRotatePointMode )
			{
				mGizmo->getProfile()->mode = RotateMode;
			}

         const RiverNode &node = mSelRiver->mNodes[mSelNode];

         MatrixF objMat = mSelRiver->getNodeTransform(mSelNode);      
         Point3F objScale( node.width, 1.0f, node.depth );
         Point3F worldPos = node.point;

         mGizmo->set( objMat, worldPos, objScale );

         mGizmo->renderGizmo( mLastCameraQuery.cameraMatrix, mLastCameraQuery.fov );
			
			// Render Gizmo text
			//mGizmo->renderText( mSaveViewport, mSaveModelview, mSaveProjection ); 
      }    
   }

   // Now draw all the 2d stuff!
   GFX->setClipRect(updateRect); 

   // Draw Control nodes for selecting and highlighted rivers
   if ( mHoverRiver )
      _drawRiverControlNodes( mHoverRiver, mHoverSplineColor );
   if ( mSelRiver )
      _drawRiverControlNodes( mSelRiver, mSelectedSplineColor );
} 

void GuiRiverEditorCtrl::_drawRiverSpline( River *river, const ColorI &color )
{
   if ( river->mSlices.size() <= 1 )
      return;

	if ( River::smShowSpline )
	{
		// Render the River center-line
		PrimBuild::color( color );
		PrimBuild::begin( GFXLineStrip, river->mSlices.size() );            
		for ( U32 i = 0; i < river->mSlices.size(); i++ )
		{            		      
			PrimBuild::vertex3fv( river->mSlices[i].p1 );		      
		}
		PrimBuild::end();
	}
   
	if ( River::smWireframe )
	{
		// Left-side line
		PrimBuild::color3i( 100, 100, 100 );
		PrimBuild::begin( GFXLineStrip, river->mSlices.size() );            
		for ( U32 i = 0; i < river->mSlices.size(); i++ )
		{            		      
			PrimBuild::vertex3fv( river->mSlices[i].p0 );		      
		}
		PrimBuild::end();

		// Right-side line
		PrimBuild::begin( GFXLineStrip, river->mSlices.size() );            
		for ( U32 i = 0; i < river->mSlices.size(); i++ )
		{            		      
			PrimBuild::vertex3fv( river->mSlices[i].p2 );		      
		}
		PrimBuild::end();

		// Cross-sections
		PrimBuild::begin( GFXLineList, river->mSlices.size() * 2 );            
		for ( U32 i = 0; i < river->mSlices.size(); i++ )
		{            		      
			PrimBuild::vertex3fv( river->mSlices[i].p0 );
			PrimBuild::vertex3fv( river->mSlices[i].p2 );		      
		}
		PrimBuild::end();
	}
   // Segment 
}

void GuiRiverEditorCtrl::_drawRiverControlNodes( River *river, const ColorI &color )
{
   if ( !River::smShowSpline )
      return;

   RectI bounds = getBounds();

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   bool isSelected = ( river == mSelRiver );
   bool isHighlighted = ( river == mHoverRiver );

   for ( U32 i = 0; i < river->mNodes.size(); i++ )
   {
      if ( false && isSelected && mSelNode == i  )
         continue;

      const Point3F &wpos = river->mNodes[i].point;

      Point3F spos;
      project( wpos, &spos );                  

      if ( spos.z > 1.0f )
         continue;

      Point2I posi;
      posi.x = spos.x;
      posi.y = spos.y;

      if ( !bounds.pointInRect( posi ) )
         continue;

      ColorI theColor = color;
      Point2I nodeHalfSize = mNodeHalfSize;

      if ( isHighlighted && mHoverNode == i )
      {
         //theColor = mHoverNodeColor;
         nodeHalfSize += Point2I(2,2);
      }

      if ( isSelected )
      {   
         if ( mSelNode == i )
         {
            theColor.set(0,0,255);
         }
         else if ( i == 0 )
         {
            theColor.set(0,255,0);
         }
         else if ( i == river->mNodes.size() - 1 )
         {
            theColor.set(255,0,0);
         }         
      }

      drawer->drawRectFill( posi - nodeHalfSize, posi + nodeHalfSize, theColor );
   }
}

bool GuiRiverEditorCtrl::getStaticPos( const Gui3DMouseEvent & event, Point3F &tpos )
{     
   // Find clicked point on the terrain

   Point3F startPnt = event.pos;
   Point3F endPnt = event.pos + event.vec * 1000.0f;

   RayInfo ri;
   bool hit;         
         
   hit = gServerContainer.castRay(startPnt, endPnt, StaticShapeObjectType, &ri);    
   tpos = ri.point;
   
   return hit;
}

void GuiRiverEditorCtrl::deleteSelectedNode()
{    
   if ( !mSelRiver || mSelNode == -1 )
      return;
   
   // If the River has only two nodes remaining,
   // delete the whole River.
   if ( mSelRiver->mNodes.size() <= 2 )
   {      
      deleteSelectedRiver( mMode != mAddNodeMode );
   }
   else
   {
      if ( mMode != mAddNodeMode )
         submitUndo( "Delete Node" );

      // Delete the SelectedNode of the SelectedRiver
      mSelRiver->deleteNode(mSelNode);
      mIsDirty = true;

      // We deleted the Node but not the River (it has nodes left)
      // so decrement the currently selected node.
      if ( mSelRiver->mNodes.size() <= mSelNode )
         setSelectedNode( mSelNode - 1 );
      else
      {
         // force gizmo to update to the selected nodes position
         // the index didn't change but the node it refers to did.
         U32 i = mSelNode;
         mSelNode = -1;
         setSelectedNode( i );
      }
   }

   // If you were in addNodeMode, 
   // deleting a node should ends it.
   //mMode = smNormalMode;
}

void GuiRiverEditorCtrl::deleteSelectedRiver( bool undoAble )
{
   AssertFatal( mSelRiver != NULL, "GuiRiverEditorCtrl::deleteSelectedRiver() - No River IS selected" );

   // Not undoAble? Just delete it.
   if ( !undoAble )
   {
      mSelRiver->deleteObject();
      mIsDirty = true;
      Con::executef( this, "onRiverSelected" );
      mSelNode = -1;

      return;
   }

   // Grab the mission editor undo manager.
   UndoManager *undoMan = NULL;
   if ( !Sim::findObject( "EUndoManager", undoMan ) )
   {
      // Couldn't find it? Well just delete the River.
      Con::errorf( "GuiRiverEditorCtrl::on3DMouseDown() - EUndoManager not found!" );    
      return;
   }
   else
   {
      // Create the UndoAction.
      MEDeleteUndoAction *action = new MEDeleteUndoAction("Deleted River");
      action->deleteObject( mSelRiver );
      mIsDirty = true;

      // Submit it.               
      undoMan->addAction( action );
   }

   // ScriptCallback with 'NULL' parameter for no River currently selected.
   Con::executef( this, "onRiverSelected" );

   // Clear the SelectedNode (it has been deleted along with the River).  
	setSelectedNode( -1 );
   mSelNode = -1;

   // SelectedRiver is a SimObjectPtr and will be NULL automatically.
}

void GuiRiverEditorCtrl::setMode( String mode, bool sourceShortcut = false )
{
   mMode = mode;

	if( sourceShortcut )
		Con::executef( this, "paletteSync", mode );
}

void GuiRiverEditorCtrl::setSelectedRiver( River *river )
{
   mSelRiver = river;

   if ( mSelRiver != NULL )
      Con::executef( this, "onRiverSelected", river->getIdString() );
   else
      Con::executef( this, "onRiverSelected" );

	if ( mSelRiver != river )
      setSelectedNode(-1);
}

void GuiRiverEditorCtrl::setNodeWidth( F32 width )
{
   if ( mSelRiver && mSelNode != -1 )
   {
      mSelRiver->setNodeWidth( mSelNode, width );
      mIsDirty = true;
   }
}

F32 GuiRiverEditorCtrl::getNodeWidth()
{
   if ( mSelRiver && mSelNode != -1 )
      return mSelRiver->getNodeWidth( mSelNode );

   return 0.0f;   
}

void GuiRiverEditorCtrl::setNodeDepth(F32 depth)
{
   if ( mSelRiver && mSelNode != -1 )
   {
      mSelRiver->setNodeDepth( mSelNode, depth );
      mIsDirty = true;
   }
}

F32 GuiRiverEditorCtrl::getNodeDepth()
{
   if ( mSelRiver && mSelNode != -1 )
      return mSelRiver->getNodeDepth( mSelNode );

   return 0.0f;
}

void GuiRiverEditorCtrl::setNodePosition(const Point3F& pos)
{
   if ( mSelRiver && mSelNode != -1 )
   {
      mSelRiver->setNodePosition( mSelNode, pos );
      mIsDirty = true;
   }
}

Point3F GuiRiverEditorCtrl::getNodePosition()
{
   if ( mSelRiver && mSelNode != -1 )
      return mSelRiver->getNodePosition( mSelNode );

   return Point3F( 0, 0, 0 );   
}

void GuiRiverEditorCtrl::setNodeNormal( const VectorF &normal )
{
   if ( mSelRiver && mSelNode != -1 )
   {
      mSelRiver->setNodeNormal( mSelNode, normal );
      mIsDirty = true;
   }
}

VectorF GuiRiverEditorCtrl::getNodeNormal()
{
   if ( mSelRiver && mSelNode != -1 )
      return mSelRiver->getNodeNormal( mSelNode );

   return VectorF::Zero;
}

void GuiRiverEditorCtrl::setSelectedNode( S32 node )
{
   //if ( mSelNode == node )
   //   return;

   mSelNode = node;
   if ( mSelNode != -1 )
   {
      const RiverNode &node = mSelRiver->mNodes[mSelNode];

      MatrixF objMat = mSelRiver->getNodeTransform(mSelNode);      
      Point3F objScale( node.width, 1.0f, node.depth );
      Point3F worldPos = node.point;

      mGizmo->set( objMat, worldPos, objScale );
   }
   
   if ( mSelNode != -1 )
      Con::executef( this, "onNodeSelected", Con::getIntArg(mSelNode) );
   else
      Con::executef( this, "onNodeSelected", Con::getIntArg(-1) );
}

void GuiRiverEditorCtrl::submitUndo( const UTF8 *name )
{
   // Grab the mission editor undo manager.
   UndoManager *undoMan = NULL;
   if ( !Sim::findObject( "EUndoManager", undoMan ) )
   {
      Con::errorf( "GuiRiverEditorCtrl::submitUndo() - EUndoManager not found!" );
      return;           
   }

   // Setup the action.
   GuiRiverEditorUndoAction *action = new GuiRiverEditorUndoAction( name );

   action->mObjId = mSelRiver->getId();
   action->mMetersPerSegment = mSelRiver->mMetersPerSegment;
   action->mSegmentsPerBatch = mSelRiver->mSegmentsPerBatch;   
   action->mRiverEditor = this;

   for( U32 i = 0; i < mSelRiver->mNodes.size(); i++ )
   {
      action->mNodes.push_back( mSelRiver->mNodes[i] );      
   }
      
   undoMan->addAction( action );
}

void GuiRiverEditorCtrl::_prepRenderImage( SceneManager* sceneGraph, const SceneRenderState* state )
{
   if ( isAwake() && River::smEditorOpen && mSelRiver )
   {
      ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->type = RenderPassManager::RIT_Editor;
      ri->renderDelegate.bind( this, &GuiRiverEditorCtrl::_renderSelectedRiver );
      ri->defaultKey = 100;
      state->getRenderPass()->addInst( ri );
   }
}

void GuiRiverEditorCtrl::_renderSelectedRiver( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *matInst )
{
   if ( !mSelRiver || !River::smEditorOpen)
      return;

   GFXTransformSaver saver;

   GFX->setStateBlock( mZEnableSB );

   if ( River::smShowWalls && mSelRiver->mSlices.size() > 1 )
   {
      Point3F offset(0,0,1);

      // Render the River volume      
      PrimBuild::begin( GFXTriangleList, 18 * mSelRiver->mSlices.size() - 1 );

      for ( U32 i = 0; i < mSelRiver->mSlices.size() - 1; i++ )
      {
         const RiverSlice &slice = mSelRiver->mSlices[i];
         const RiverSlice &nextSlice = mSelRiver->mSlices[i+1];

         // Top face
         //drawer->drawQuad( slice.p0, nextSlice.p0, nextSlice.p2, slice.p2, colorRed, true );
         //PrimBuild::color3i( 0, 0, 255 );
         //PrimBuild::vertex3fv( slice.p0 );
         //PrimBuild::vertex3fv( nextSlice.p0 );
         //PrimBuild::vertex3fv( nextSlice.p2 );
         //PrimBuild::vertex3fv( slice.p0 );
         //PrimBuild::vertex3fv( nextSlice.p2 );
         //PrimBuild::vertex3fv( slice.p2 );         

         // Bottom face
         PrimBuild::color3i( 0, 255, 0 );
         PrimBuild::vertex3fv( slice.pb0 );
         PrimBuild::vertex3fv( nextSlice.pb0 );
         PrimBuild::vertex3fv( nextSlice.pb2 );
         PrimBuild::vertex3fv( slice.pb0 );
         PrimBuild::vertex3fv( nextSlice.pb2 );         
         PrimBuild::vertex3fv( slice.pb2 );         

         // Left face
         PrimBuild::color3i( 255, 0, 0 );
         PrimBuild::vertex3fv( slice.pb0 );
         PrimBuild::vertex3fv( nextSlice.pb0 );         
         PrimBuild::vertex3fv( nextSlice.p0 );
         PrimBuild::vertex3fv( slice.pb0 );
         PrimBuild::vertex3fv( nextSlice.p0 );
         PrimBuild::vertex3fv( slice.p0 );         

         // Right face
         PrimBuild::color3i( 255, 0, 0 );
         PrimBuild::vertex3fv( slice.p2 );         
         PrimBuild::vertex3fv( nextSlice.p2 );
         PrimBuild::vertex3fv( nextSlice.pb2 );
         PrimBuild::vertex3fv( slice.p2 );    
         PrimBuild::vertex3fv( nextSlice.pb2 );
         PrimBuild::vertex3fv( slice.pb2 );         
      }

      PrimBuild::end();
   }
}

DefineConsoleMethod( GuiRiverEditorCtrl, deleteNode, void, (), , "deleteNode()" )
{
   object->deleteSelectedNode();
}

DefineConsoleMethod( GuiRiverEditorCtrl, getMode, const char*, (), , "" )
{
   return object->getMode();
}

DefineConsoleMethod( GuiRiverEditorCtrl, setMode, void, ( const char * mode ), , "setMode( String mode )" )
{
   String newMode = ( mode );
   object->setMode( newMode );
}

DefineConsoleMethod( GuiRiverEditorCtrl, getNodeWidth, F32, (), , "" )
{
   return object->getNodeWidth();
}

DefineConsoleMethod( GuiRiverEditorCtrl, setNodeWidth, void, ( F32 width ), , "" )
{
   object->setNodeWidth( width );
}

DefineConsoleMethod( GuiRiverEditorCtrl, getNodeDepth, F32, (), , "" )
{
   return object->getNodeDepth();
}

DefineConsoleMethod( GuiRiverEditorCtrl, setNodeDepth, void, ( F32 depth ), , "" )
{
   object->setNodeDepth( depth );
}

DefineConsoleMethod( GuiRiverEditorCtrl, getNodePosition, Point3F, (), , "" )
{

	return  object->getNodePosition();
}

DefineConsoleMethod( GuiRiverEditorCtrl, setNodePosition, void, (Point3F pos), , "" )
{
   object->setNodePosition( pos );
}

DefineConsoleMethod( GuiRiverEditorCtrl, getNodeNormal, Point3F, (), , "" )
{

	return object->getNodeNormal();
}

DefineConsoleMethod( GuiRiverEditorCtrl, setNodeNormal, void, (Point3F normal), , "" )
{

   object->setNodeNormal( normal );
}

DefineConsoleMethod( GuiRiverEditorCtrl, setSelectedRiver, void, (const char * objName), (""), "" )
{
   if (dStrcmp( objName,"" )==0)
      object->setSelectedRiver(NULL);
   else
   {
      River *river = NULL;
      if ( Sim::findObject( objName, river ) )
         object->setSelectedRiver(river);
   }
}

DefineConsoleMethod( GuiRiverEditorCtrl, getSelectedRiver, S32, (), , "" )
{
   River *river = object->getSelectedRiver();
   if ( !river )
      return NULL;

   return river->getId();
}

DefineConsoleMethod( GuiRiverEditorCtrl, regenerate, void, (), , "" )
{
   River *river = object->getSelectedRiver();
   if ( river )
      river->regenerate();
}
