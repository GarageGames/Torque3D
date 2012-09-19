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
#include "environment/editors/guiRoadEditorCtrl.h"

#include "console/consoleTypes.h"
#include "scene/sceneManager.h"
#include "collision/collision.h"
#include "math/util/frustum.h"
#include "gfx/gfxPrimitiveBuffer.h"
#include "gfx/gfxTextureHandle.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/primBuilder.h"
#include "T3D/gameBase/gameConnection.h"
#include "gui/core/guiCanvas.h"
#include "gui/buttons/guiButtonCtrl.h"
#include "gui/worldEditor/undoActions.h"
#include "materials/materialDefinition.h"

IMPLEMENT_CONOBJECT(GuiRoadEditorCtrl);

ConsoleDocClass( GuiRoadEditorCtrl,
   "@brief GUI tool that makes up the Decal Road Editor\n\n"
   "Editor use only.\n\n"
   "@internal"
);

GuiRoadEditorCtrl::GuiRoadEditorCtrl()
{
	// Each of the mode names directly correlates with the River Editor's
	// tool palette
	mSelectRoadMode = "RoadEditorSelectMode";
	mAddRoadMode = "RoadEditorAddRoadMode";
	mMovePointMode = "RoadEditorMoveMode";
	mScalePointMode = "RoadEditorScaleMode";
	mAddNodeMode = "RoadEditorAddNodeMode";
	mInsertPointMode = "RoadEditorInsertPointMode";
	mRemovePointMode = "RoadEditorRemovePointMode";
	
	mMode = mSelectRoadMode;
   
   mRoadSet = NULL;   
   mSelNode = -1;
   mHoverNode = -1;
   mSelRoad = NULL;
   mHoverRoad = NULL; 
   mAddNodeIdx = 0;

   mDefaultWidth = 10.0f;
   mInsertIdx = -1;

   mStartWidth = -1.0f;
   mStartX = 0;

   mNodeHalfSize.set(4,4);

   mHoverSplineColor.set( 255,0,0,255 );
   mSelectedSplineColor.set( 0,255,0,255 );
   mHoverNodeColor.set( 255,255,255,255 );

   mIsDirty = false;

	mMaterialName = StringTable->insert("DefaultDecalRoadMaterial");
}

GuiRoadEditorCtrl::~GuiRoadEditorCtrl()
{
   // nothing to do
}

void GuiRoadEditorUndoAction::undo()
{
   DecalRoad *road = NULL;
   if ( !Sim::findObject( mObjId, road ) )
      return;

   // Temporarily save the roads current data.
   String materialName = road->mMaterialName;
   F32 textureLength = road->mTextureLength;
   F32 breakAngle = road->mBreakAngle;
   F32 segmentsPerBatch = road->mSegmentsPerBatch;
   Vector<RoadNode> nodes;   
   nodes.merge( road->mNodes );

   // Restore the Road properties saved in the UndoAction
   road->mMaterialName = materialName;
   road->mBreakAngle = breakAngle;
   road->mSegmentsPerBatch = segmentsPerBatch;
   road->mTextureLength = textureLength;
   road->inspectPostApply();

   // Restore the Nodes saved in the UndoAction
   road->mNodes.clear();
   for ( U32 i = 0; i < mNodes.size(); i++ )
   {
      road->_addNode( mNodes[i].point, mNodes[i].width );      
   }

   // Regenerate the road
   road->regenerate();

   // If applicable set the selected road and node
   mRoadEditor->mSelRoad = road;
   mRoadEditor->mSelNode = -1;

   // Now save the previous Road data in this UndoAction
   // since an undo action must become a redo action and vice-versa
   mMaterialName = materialName;
   mBreakAngle = breakAngle;
   mSegmentsPerBatch = segmentsPerBatch;
   mTextureLength = textureLength;
   
   mNodes.clear();
   mNodes.merge( nodes );
}

bool GuiRoadEditorCtrl::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   mRoadSet = DecalRoad::getServerSet();   

   GFXStateBlockDesc desc;      
   desc.setCullMode( GFXCullNone );
   desc.setBlend(false);
   desc.setZReadWrite( false, false );

   mZDisableSB = GFX->createStateBlock(desc);

   return true;
}

void GuiRoadEditorCtrl::initPersistFields()
{
   addField( "DefaultWidth",        TypeF32,    Offset( mDefaultWidth, GuiRoadEditorCtrl ) );
   addField( "HoverSplineColor",    TypeColorI, Offset( mHoverSplineColor, GuiRoadEditorCtrl ) );
   addField( "SelectedSplineColor", TypeColorI, Offset( mSelectedSplineColor, GuiRoadEditorCtrl ) );
   addField( "HoverNodeColor",      TypeColorI, Offset( mHoverNodeColor, GuiRoadEditorCtrl ) );
   addField( "isDirty",             TypeBool,   Offset( mIsDirty, GuiRoadEditorCtrl ) );
	addField( "materialName",			TypeString, Offset( mMaterialName, GuiRoadEditorCtrl ),
      "Default Material used by the Road Editor on road creation." );
   //addField( "MoveNodeCursor", TYPEID< SimObject >(), Offset( mMoveNodeCursor, GuiRoadEditorCtrl) );
   //addField( "AddNodeCursor", TYPEID< SimObject >(), Offset( mAddNodeCursor, GuiRoadEditorCtrl) );
   //addField( "InsertNodeCursor", TYPEID< SimObject >(), Offset( mInsertNodeCursor, GuiRoadEditorCtrl) );
   //addField( "ResizeNodeCursor", TYPEID< SimObject >(), Offset( mResizeNodeCursor, GuiRoadEditorCtrl) );

   Parent::initPersistFields();
}

void GuiRoadEditorCtrl::onSleep()
{
   Parent::onSleep();

   mMode = mSelectRoadMode;   
   mHoverNode = -1;
   mHoverRoad = NULL;
   setSelectedNode(-1);
}

void GuiRoadEditorCtrl::get3DCursor( GuiCursor *&cursor, 
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

void GuiRoadEditorCtrl::on3DMouseDown(const Gui3DMouseEvent & event)
{
   if ( !isFirstResponder() )
      setFirstResponder();

   // Get the clicked terrain position.
   Point3F tPos;
   if ( !getTerrainPos( event, tPos ) )
      return;      

   mouseLock();

   // Find any road / node at the clicked position.
   // TODO: handle overlapping roads/nodes somehow, cycle through them.
   
   DecalRoad *roadPtr = NULL;
   S32 closestNodeIdx = -1;
   F32 closestDist = F32_MAX;
   DecalRoad *closestNodeRoad = NULL;

   // First, find the closest node in any road to the clicked position.
   for ( SimSetIterator iter(mRoadSet); *iter; ++iter )
   {
      roadPtr = static_cast<DecalRoad*>( *iter );
      U32 idx;
      if ( roadPtr->getClosestNode( tPos, idx ) )
      {
         Point3F nodePos = roadPtr->getNodePosition(idx);
         F32 dist = ( nodePos - tPos ).len();
         if ( dist < closestDist )
         {
            closestNodeIdx = idx;
            closestDist = dist;
            closestNodeRoad = roadPtr;
         }
      }
   }

   //
   // Second, determine if the screen-space node rectangle
   // contains the clicked position.

   bool nodeClicked = false;
   S32 clickedNodeIdx = -1;

   if ( closestNodeIdx != -1 )
   {
      Point3F nodePos = closestNodeRoad->getNodePosition( closestNodeIdx );

      Point3F temp;
      project( nodePos, &temp );
      Point2I screenPos( temp.x, temp.y );

      RectI nodeRect( screenPos - mNodeHalfSize, mNodeHalfSize * 2 );
      
      nodeClicked = nodeRect.pointInRect( event.mousePoint );
      if ( nodeClicked )
         clickedNodeIdx = closestNodeIdx;
   }

   //
   // Determine the clickedRoad
   //
   DecalRoad *clickedRoadPtr = NULL;
   U32 insertNodeIdx = 0;

   if ( nodeClicked && (mSelRoad == NULL || closestNodeRoad == mSelRoad) )
   {
      // If a node was clicked, the owning road is always
      // considered the clicked road.
      clickedRoadPtr = closestNodeRoad;
   }
   else
   {
      // check the selected road first
      if ( mSelRoad != NULL && mSelRoad->containsPoint( tPos, &insertNodeIdx ) )
      {
         clickedRoadPtr = mSelRoad;
         nodeClicked = false;
         clickedNodeIdx = -1;
      }
      else
      {
         // Otherwise, we must ask each road if it contains
         // the clicked pos.
         for ( SimSetIterator iter(mRoadSet); *iter; ++iter )
         {
            roadPtr = static_cast<DecalRoad*>( *iter );
            if ( roadPtr->containsPoint( tPos, &insertNodeIdx ) )
            {
               clickedRoadPtr = roadPtr;
               break;            
            }
         }
      }
   }

	// shortcuts
   bool dblClick = ( event.mouseClickCount > 1 );
	if( dblClick )
   { 
		if( mMode == mSelectRoadMode )
		{
			setMode( mAddRoadMode, true );
			return;
		}
		if( mMode == mAddNodeMode )
		{
			// Delete the node attached to the cursor.
			deleteSelectedNode();
			mMode = mAddRoadMode;
			return;
		}
	}

	//this check is here in order to bounce back from deleting a whole road with ctrl+z
	//this check places the editor back into addroadmode
	if ( mMode == mAddNodeMode )
	{
      if ( !mSelRoad )
         mMode = mAddRoadMode;
	}

	if ( mMode == mSelectRoadMode )
	{
      // Did not click on a road or a node.
      if ( !clickedRoadPtr  )
      {
         setSelectedRoad( NULL );
         setSelectedNode( -1 );
         
         return;
      }

      // Clicked on a road that wasn't the currently selected road.
      if ( clickedRoadPtr != mSelRoad )
      {
         setSelectedRoad( clickedRoadPtr );
         setSelectedNode( -1 );
         return;
      }

      // Clicked on a node in the currently selected road that wasn't
      // the currently selected node.
      if ( nodeClicked )
      {
         setSelectedNode( clickedNodeIdx );
         return;
      }

      
      // Clicked a position on the currently selected road
      // that did not contain a node.
      //U32 newNode = clickedRoadPtr->insertNode( tPos, mDefaultWidth, insertNodeIdx );                  
      //setSelectedNode( newNode );
	}
   else if ( mMode == mAddRoadMode )
   {
		if ( nodeClicked && clickedRoadPtr )
      {
			// A double-click on a node in Normal mode means set AddNode mode.  
         if ( clickedNodeIdx == 0 )
         {
				setSelectedRoad( clickedRoadPtr );
				setSelectedNode( clickedNodeIdx );

				mAddNodeIdx = clickedNodeIdx;
            mMode = mAddNodeMode;
            mSelNode = mSelRoad->insertNode( tPos, mDefaultWidth, mAddNodeIdx );
            mIsDirty = true;

				return;
         }
			else if ( clickedNodeIdx == clickedRoadPtr->mNodes.size() - 1 )
         {
				setSelectedRoad( clickedRoadPtr );
				setSelectedNode( clickedNodeIdx );

            mAddNodeIdx = U32_MAX;
            mMode = mAddNodeMode;
            mSelNode = mSelRoad->addNode( tPos, mDefaultWidth );
            mIsDirty = true;
				setSelectedNode( mSelNode );

				return;
         } 
		}

		DecalRoad *newRoad = new DecalRoad;
		

		newRoad->mMaterialName = mMaterialName;

      newRoad->registerObject();

      // Add to MissionGroup                              
      SimGroup *missionGroup;
      if ( !Sim::findObject( "MissionGroup", missionGroup ) )               
         Con::errorf( "GuiDecalRoadEditorCtrl - could not find MissionGroup to add new DecalRoad" );
      else
         missionGroup->addObject( newRoad );               

      newRoad->insertNode( tPos, mDefaultWidth, 0 );
      U32 newNode = newRoad->insertNode( tPos, mDefaultWidth, 1 );

      // Always add to the end of the road, the first node is the start.
      mAddNodeIdx = U32_MAX;
      
      setSelectedRoad( newRoad );      
      setSelectedNode( newNode );

      mMode = mAddNodeMode;

      // Disable the hover node while in addNodeMode, we
      // don't want some random node enlarged.
      mHoverNode = -1;

      // Grab the mission editor undo manager.
      UndoManager *undoMan = NULL;
      if ( !Sim::findObject( "EUndoManager", undoMan ) )
      {
         Con::errorf( "GuiRoadEditorCtrl::on3DMouseDown() - EUndoManager not found!" );
         return;           
      }

      // Create the UndoAction.
      MECreateUndoAction *action = new MECreateUndoAction("Create Road");
      action->addObject( newRoad );
      
      // Submit it.               
      undoMan->addAction( action );
		
		//send a callback to script after were done here if one exists
		if ( isMethod( "onRoadCreation" ) )
         Con::executef( this, "onRoadCreation" );

		return;
   }
	else if ( mMode == mAddNodeMode )
	{
		// Oops the road got deleted, maybe from an undo action?
      // Back to NormalMode.
      if ( mSelRoad )
      {
			// A double-click on a node in Normal mode means set AddNode mode.  
         if ( clickedNodeIdx == 0 )
         {
				submitUndo( "Add Node" );
				mAddNodeIdx = clickedNodeIdx;
            mMode = mAddNodeMode;
            mSelNode = mSelRoad->insertNode( tPos, mDefaultWidth, mAddNodeIdx );
            mIsDirty = true;
				setSelectedNode( mSelNode );

				return;
         }
			else
         {
				if( clickedRoadPtr && clickedNodeIdx == clickedRoadPtr->mNodes.size() - 1 )
				{
					submitUndo( "Add Node" );
					mAddNodeIdx = U32_MAX;
					mMode = mAddNodeMode;
					mSelNode = mSelRoad->addNode( tPos, mDefaultWidth );
               mIsDirty = true;
					setSelectedNode( mSelNode );

					return;
				}
				else
				{
					submitUndo( "Insert Node" );
					// A single-click on empty space while in
					// AddNode mode means insert / add a node.
					//submitUndo( "Add Node" );
					//F32 width = mSelRoad->getNodeWidth( mSelNode );
					U32 newNode = mSelRoad->insertNode( tPos, mDefaultWidth, mAddNodeIdx);
               mIsDirty = true;
					setSelectedNode( newNode );

					return;
				}
         } 
      }
	}
	else if ( mMode == mInsertPointMode  && mSelRoad != NULL)
	{
		if ( clickedRoadPtr == mSelRoad )
      {
			F32 w0 = mSelRoad->getNodeWidth( insertNodeIdx );
         F32 w1 = mSelRoad->getNodeWidth( insertNodeIdx + 1 );               
         F32 width = ( w0 + w1 ) * 0.5f;

         submitUndo( "Insert Node" );
         U32 newNode = mSelRoad->insertNode( tPos, width, insertNodeIdx + 1);  
         mIsDirty = true;
         setSelectedNode( newNode );

			return;
       }
	}
	else if ( mMode == mRemovePointMode  && mSelRoad != NULL)
	{
		if ( nodeClicked && clickedRoadPtr == mSelRoad )
      {
			setSelectedNode( clickedNodeIdx );
         deleteSelectedNode();
         return;
      }
	}
	else if ( mMode == mMovePointMode )
	{
		if ( nodeClicked && clickedRoadPtr == mSelRoad )
      {
			setSelectedNode( clickedNodeIdx );
         return;
      }
	}
	else if ( mMode == mScalePointMode )
	{
		if ( nodeClicked && clickedRoadPtr == mSelRoad )
      {
			setSelectedNode( clickedNodeIdx );
         return;
      }
	}
}

void GuiRoadEditorCtrl::on3DRightMouseDown(const Gui3DMouseEvent & event)
{
   //mIsPanning = true;
}

void GuiRoadEditorCtrl::on3DRightMouseUp(const Gui3DMouseEvent & event)
{
   //mIsPanning = false;
}

void GuiRoadEditorCtrl::on3DMouseUp(const Gui3DMouseEvent & event)
{
   mStartWidth = -1.0f;     
   mSavedDrag = false;
   mouseUnlock();
}

void GuiRoadEditorCtrl::on3DMouseMove(const Gui3DMouseEvent & event)
{
   if ( mSelRoad != NULL && mMode == mAddNodeMode )
   {
      Point3F startPnt = event.pos;
      Point3F endPnt = event.pos + event.vec * 1000.0f;
      RayInfo ri;   
      if ( gServerContainer.castRay(startPnt, endPnt, TerrainObjectType, &ri) )
      {
         mSelRoad->setNodePosition( mSelNode, ri.point );
         mIsDirty = true;
      }

      return;
   }

   // Is cursor hovering over a road?
   if ( mMode == mSelectRoadMode )
   {
      mHoverRoad = NULL;

      Point3F startPnt = event.pos;
      Point3F endPnt = event.pos + event.vec * 1000.0f;

      RayInfo ri;   

      if ( gServerContainer.castRay(startPnt, endPnt, TerrainObjectType, &ri) )
      {         
         DecalRoad *pRoad = NULL;

         for ( SimSetIterator iter(mRoadSet); *iter; ++iter )
         {
            pRoad = static_cast<DecalRoad*>( *iter );

            if ( pRoad->containsPoint( ri.point ) )
            {
               mHoverRoad = pRoad;
               break;
            }
         }      
      }
   }

   // Is cursor hovering over a RoadNode?
   if ( mHoverRoad )
   {      
      DecalRoad *pRoad = mHoverRoad;

      S32 hoverNodeIdx = -1;
      F32 hoverNodeDist = F32_MAX;

      for ( U32 i = 0; i < pRoad->mNodes.size(); i++ )
      {
         const Point3F &nodePos = pRoad->mNodes[i].point;

         Point3F screenPos;
         project( nodePos, &screenPos );
         
         RectI rect( Point2I((S32)screenPos.x,(S32)screenPos.y) - mNodeHalfSize, mNodeHalfSize * 2 );
         
         if ( rect.pointInRect( event.mousePoint ) && screenPos.z < hoverNodeDist )
         {            
            hoverNodeDist = screenPos.z;
            hoverNodeIdx = i;
         }           
      }      

      mHoverNode = hoverNodeIdx;
   }
}

void GuiRoadEditorCtrl::on3DMouseDragged(const Gui3DMouseEvent & event)
{   
   // Drags are only used to transform nodes
   if ( !mSelRoad || mSelNode == -1 ||
      ( mMode != mMovePointMode && mMode != mScalePointMode ) )
      return;

   if ( !mSavedDrag )
   {
      submitUndo( "Modify Node" );
      mSavedDrag = true;
   }

   if ( mMode == mScalePointMode )
   {
      Point3F tPos;
      if ( !getTerrainPos( event, tPos ) )
         return;   

      if ( mStartWidth == -1.0f )
      {
         mStartWidth = mSelRoad->mNodes[mSelNode].width;
         
         mStartX = event.mousePoint.x;
         mStartWorld = tPos;
      }

      S32 deltaScreenX = event.mousePoint.x - mStartX;
      
      F32 worldDist = ( event.pos - mStartWorld ).len();      

      F32 deltaWorldX = ( deltaScreenX * worldDist ) / getWorldToScreenScale().y;

      F32 width = mStartWidth + deltaWorldX;      

      mSelRoad->setNodeWidth( mSelNode, width );
      mIsDirty = true;
   }
   else if( mMode == mMovePointMode ) 
   {    
      Point3F tPos;
      if ( !getTerrainPos( event, tPos ) )
         return; 

      mSelRoad->setNodePosition( mSelNode, tPos );
      mIsDirty = true;
   }   

   Con::executef( this, "onNodeModified", Con::getIntArg(mSelNode) );
}

void GuiRoadEditorCtrl::on3DMouseEnter(const Gui3DMouseEvent & event)
{
   // nothing to do
}

void GuiRoadEditorCtrl::on3DMouseLeave(const Gui3DMouseEvent & event)
{
   // nothing to do
}

bool GuiRoadEditorCtrl::onKeyDown(const GuiEvent& event)
{
	if( event.keyCode == KEY_RETURN && mMode == mAddNodeMode )
   {
		// Delete the node attached to the cursor.
		deleteSelectedNode();
		mMode = mAddRoadMode;
		return true;
	}

	return false;
}

void GuiRoadEditorCtrl::updateGuiInfo()
{
   // nothing to do
}
      
void GuiRoadEditorCtrl::onRender( Point2I offset, const RectI &updateRect )
{
   PROFILE_SCOPE( GuiRoadEditorCtrl_OnRender );

   Parent::onRender( offset, updateRect );
   return;
}
      
void GuiRoadEditorCtrl::renderScene(const RectI & updateRect)
{   
   GFX->setStateBlock( mZDisableSB );

   // Draw the spline based from the client-side road
   // because the serverside spline is not actually reliable...
   // Can be incorrect if the DecalRoad is before the TerrainBlock
   // in the MissionGroup.

   if ( mHoverRoad && mHoverRoad != mSelRoad )
   {      
      DecalRoad *pRoad = (DecalRoad*)mHoverRoad->getClientObject();
      if ( pRoad )
         _drawRoadSpline( pRoad, mHoverSplineColor );
   }

   if ( mSelRoad )
   {
      DecalRoad *pRoad = (DecalRoad*)mSelRoad->getClientObject();
      if ( pRoad )
         _drawRoadSpline( pRoad, mSelectedSplineColor );     
   }
} 

void GuiRoadEditorCtrl::renderGui( Point2I offset, const RectI &updateRect )
{
   // Draw Control nodes for selected and highlighted roads
   if ( mHoverRoad )
      _drawRoadControlNodes( mHoverRoad, mHoverSplineColor );
   if ( mSelRoad )
      _drawRoadControlNodes( mSelRoad, mSelectedSplineColor );

   Parent::renderGui(offset, updateRect);
}

void GuiRoadEditorCtrl::_drawRoadSpline( DecalRoad *road, const ColorI &color )
{
   if ( road->mEdges.size() <= 1 )
      return;
	
   GFXTransformSaver saver;

	if ( DecalRoad::smShowSpline )
	{
		// Render the center-line
		PrimBuild::color( color );
		PrimBuild::begin( GFXLineStrip, road->mEdges.size() );            
		for ( U32 i = 0; i < road->mEdges.size(); i++ )
		{            		      
			PrimBuild::vertex3fv( road->mEdges[i].p1 );		      
		}
		PrimBuild::end();
	}
	
	if ( DecalRoad::smWireframe )
	{
		// Left-side line
		PrimBuild::color3i( 100, 100, 100 );
		PrimBuild::begin( GFXLineStrip, road->mEdges.size() );            
		for ( U32 i = 0; i < road->mEdges.size(); i++ )
		{            		      
			PrimBuild::vertex3fv( road->mEdges[i].p0 );		      
		}
		PrimBuild::end();

		// Right-side line
		PrimBuild::begin( GFXLineStrip, road->mEdges.size() );            
		for ( U32 i = 0; i < road->mEdges.size(); i++ )
		{            		      
			PrimBuild::vertex3fv( road->mEdges[i].p2 );		      
		}
		PrimBuild::end();

		// Cross-sections
		PrimBuild::begin( GFXLineList, road->mEdges.size() * 2 );            
		for ( U32 i = 0; i < road->mEdges.size(); i++ )
		{            		      
			PrimBuild::vertex3fv( road->mEdges[i].p0 );
			PrimBuild::vertex3fv( road->mEdges[i].p2 );		      
		}
		PrimBuild::end();
	}
}

void GuiRoadEditorCtrl::_drawRoadControlNodes( DecalRoad *road, const ColorI &color )
{
   if ( !DecalRoad::smShowSpline )
      return;

   RectI bounds = getBounds();

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   bool isSelected = ( road == mSelRoad );
   bool isHighlighted = ( road == mHoverRoad );

   for ( U32 i = 0; i < road->mNodes.size(); i++ )
   {
      if ( false && isSelected && mSelNode == i  )
         continue;

      const Point3F &wpos = road->mNodes[i].point;

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
         nodeHalfSize += Point2I(2,2);

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
         else if ( i == road->mNodes.size() - 1 )
         {
            theColor.set(255,0,0);
         }         
      }

      drawer->drawRectFill( posi - nodeHalfSize, posi + nodeHalfSize, theColor );
   }
}

bool GuiRoadEditorCtrl::getTerrainPos( const Gui3DMouseEvent & event, Point3F &tpos )
{     
   // Find clicked point on the terrain

   Point3F startPnt = event.pos;
   Point3F endPnt = event.pos + event.vec * 10000.0f;

   RayInfo ri;
   bool hit;         
         
   hit = gServerContainer.castRay(startPnt, endPnt, TerrainObjectType, &ri);    
   tpos = ri.point;
   
   return hit;
}

void GuiRoadEditorCtrl::deleteSelectedNode()
{    
   if ( !mSelRoad || mSelNode == -1 )
      return;
   
   // If the road has only two nodes remaining,
   // delete the whole road.
   if ( mSelRoad->mNodes.size() <= 2 )
   {      
      deleteSelectedRoad();
   }
   else
   {
      // Only submit undo if we weren't in AddMode
      if ( mMode != mAddNodeMode )
         submitUndo( "Delete Node" );

      // Delete the SelectedNode of the SelectedRoad
      mSelRoad->deleteNode(mSelNode);
      mIsDirty = true;

      // We deleted the Node but not the Road (it has nodes left)
      // so decrement the currently selected node.
      if ( mSelRoad->mNodes.size() <= mSelNode )
         mSelNode--;
   }
}

void GuiRoadEditorCtrl::deleteSelectedRoad( bool undoAble )
{
   AssertFatal( mSelRoad != NULL, "GuiRoadEditorCtrl::deleteSelectedRoad() - No road IS selected" );

   // Not undo-able? Just delete it.
   if ( !undoAble )
   {
      DecalRoad *lastRoad = mSelRoad;

      setSelectedRoad(NULL);

      lastRoad->deleteObject();
      mIsDirty = true;

      return;
   }

   // Grab the mission editor undo manager.
   UndoManager *undoMan = NULL;
   if ( !Sim::findObject( "EUndoManager", undoMan ) )
   {
      // Couldn't find it? Well just delete the road.
      Con::errorf( "GuiRoadEditorCtrl::on3DMouseDown() - EUndoManager not found!" );    
      return;
   }
   else
   {
      DecalRoad *lastRoad = mSelRoad;
      setSelectedRoad(NULL);

      // Create the UndoAction.
      MEDeleteUndoAction *action = new MEDeleteUndoAction("Deleted Road");
      action->deleteObject( lastRoad );
      mIsDirty = true;

      // Submit it.               
      undoMan->addAction( action );
   }
}

void GuiRoadEditorCtrl::setMode( String mode, bool sourceShortcut = false )
{
   mMode = mode;

	if( sourceShortcut )
		Con::executef( this, "paletteSync", mode );
}

void GuiRoadEditorCtrl::setSelectedRoad( DecalRoad *road )
{
   mSelRoad = road;

   if ( road != NULL )
      Con::executef( this, "onRoadSelected", road->getIdString() );
   else
      Con::executef( this, "onRoadSelected" );

	if ( mSelRoad != road )
      setSelectedNode(-1);
}

void GuiRoadEditorCtrl::setNodeWidth( F32 width )
{
   if ( mSelRoad && mSelNode != -1 )
   {
      mSelRoad->setNodeWidth( mSelNode, width );
      mIsDirty = true;
   }
}

F32 GuiRoadEditorCtrl::getNodeWidth()
{
   if ( mSelRoad && mSelNode != -1 )
      return mSelRoad->getNodeWidth( mSelNode );

   return 0.0f;   
}

void GuiRoadEditorCtrl::setNodePosition( Point3F pos )
{
   if ( mSelRoad && mSelNode != -1 )
   {
      mSelRoad->setNodePosition( mSelNode, pos );
      mIsDirty = true;
   }
}

Point3F GuiRoadEditorCtrl::getNodePosition()
{
   if ( mSelRoad && mSelNode != -1 )
      return mSelRoad->getNodePosition( mSelNode );

   return Point3F( 0, 0, 0 );   
}

void GuiRoadEditorCtrl::setSelectedNode( S32 node )
{
   //if ( mSelNode == node )
   //   return;

   mSelNode = node;
   
   if ( mSelNode != -1 && mSelRoad != NULL )
      Con::executef( this, "onNodeSelected", Con::getIntArg(mSelNode), Con::getFloatArg(mSelRoad->mNodes[mSelNode].width) );
   else
      Con::executef( this, "onNodeSelected", Con::getIntArg(-1) );
}

void GuiRoadEditorCtrl::submitUndo( const UTF8 *name )
{
   // Grab the mission editor undo manager.
   UndoManager *undoMan = NULL;
   if ( !Sim::findObject( "EUndoManager", undoMan ) )
   {
      Con::errorf( "GuiRoadEditorCtrl::submitUndo() - EUndoManager not found!" );
      return;           
   }

   // Setup the action.
   GuiRoadEditorUndoAction *action = new GuiRoadEditorUndoAction( name );

   action->mObjId = mSelRoad->getId();
   action->mBreakAngle = mSelRoad->mBreakAngle;
   action->mMaterialName = mSelRoad->mMaterialName;
   action->mSegmentsPerBatch = mSelRoad->mSegmentsPerBatch;   
   action->mTextureLength = mSelRoad->mTextureLength;
   action->mRoadEditor = this;

   for( U32 i = 0; i < mSelRoad->mNodes.size(); i++ )
   {
      action->mNodes.push_back( mSelRoad->mNodes[i] );      
   }
      
   undoMan->addAction( action );
}

ConsoleMethod( GuiRoadEditorCtrl, deleteNode, void, 2, 2, "deleteNode()" )
{
   object->deleteSelectedNode();
}

ConsoleMethod( GuiRoadEditorCtrl, getMode, const char*, 2, 2, "" )
{
   return object->getMode();
}

ConsoleMethod( GuiRoadEditorCtrl, setMode, void, 3, 3, "setMode( String mode )" )
{
	String newMode = ( argv[2] );
   object->setMode( newMode );
}

ConsoleMethod( GuiRoadEditorCtrl, getNodeWidth, F32, 2, 2, "" )
{
   return object->getNodeWidth();
}

ConsoleMethod( GuiRoadEditorCtrl, setNodeWidth, void, 3, 3, "" )
{
   object->setNodeWidth( dAtof(argv[2]) );
}

ConsoleMethod( GuiRoadEditorCtrl, getNodePosition, const char*, 2, 2, "" )
{
	char* returnBuffer = Con::getReturnBuffer(256);

	dSprintf(returnBuffer, 256, "%f %f %f",
      object->getNodePosition().x, object->getNodePosition().y, object->getNodePosition().z);

	return returnBuffer;
}

ConsoleMethod( GuiRoadEditorCtrl, setNodePosition, void, 3, 3, "" )
{
	Point3F pos;

	S32 count = dSscanf( argv[2], "%f %f %f", 
		&pos.x, &pos.y, &pos.z);
	
	if ( (count != 3) )
   {
		Con::printf("Failed to parse node information \"px py pz\" from '%s'", argv[3]);
      return;
   }

   object->setNodePosition( pos );
}

ConsoleMethod( GuiRoadEditorCtrl, setSelectedRoad, void, 2, 3, "" )
{
   if ( argc == 2 )
      object->setSelectedRoad(NULL);
   else
   {
      DecalRoad *road = NULL;
      if ( Sim::findObject( argv[2], road ) )
         object->setSelectedRoad(road);
   }
}

ConsoleMethod( GuiRoadEditorCtrl, getSelectedRoad, const char*, 2, 2, "" )
{
   DecalRoad *road = object->getSelectedRoad();
   if ( road )
      return road->getIdString();
   
   return NULL;
}

ConsoleMethod( GuiRoadEditorCtrl, getSelectedNode, S32, 2, 2, "" )
{
   return object->getSelectedNode();
}

ConsoleMethod( GuiRoadEditorCtrl, deleteRoad, void, 2, 2, "" )
{
   object->deleteSelectedRoad();
}
