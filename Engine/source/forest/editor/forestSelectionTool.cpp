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
#include "forest/editor/forestSelectionTool.h"

#include "forest/forest.h"
#include "forest/editor/forestUndo.h"
#include "forest/editor/forestEditorCtrl.h"

#include "gui/worldEditor/editTSCtrl.h"
#include "gui/worldEditor/gizmo.h"
#include "console/consoleTypes.h"
#include "core/util/tVector.h"
#include "core/util/safeDelete.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/worldEditor/worldEditor.h"
#include "math/mMatrix.h"

template <>
MatrixF Selection<ForestItem>::getOrientation()
{
   if ( size() == 1 )
      return first().getTransform();

   return MatrixF::Identity;
}

template <>
Point3F Selection<ForestItem>::getOrigin()
{
   Point3F centroid( Point3F::Zero );

   if ( empty() )
      return centroid;

   Selection<ForestItem>::iterator itr = begin();

   for ( ; itr != end(); itr++ )
   {
      const MatrixF &mat = itr->getTransform();
      Point3F wPos;
      mat.getColumn( 3, &wPos );

      centroid += wPos;
   }

   centroid /= (F32)size();

   return centroid;
}

template <>
Point3F Selection<ForestItem>::getScale()
{
   if ( size() == 1 )
      return Point3F( first().getScale() );

   return Point3F::One;
}

void ForestItemSelection::offsetObject( ForestItem &object, const Point3F &delta )
{
   if ( !mData )
      return;

   MatrixF newMat( object.getTransform() );
   newMat.displace( delta );

   object = mData->updateItem( object.getKey(), object.getPosition(), object.getData(), newMat, object.getScale() );   
}

void ForestItemSelection::rotateObject( ForestItem &object, const EulerF &delta, const Point3F &origin )
{
   if ( !mData )
      return;

   MatrixF mat = object.getTransform();

   Point3F pos;
   mat.getColumn( 3, &pos );

   MatrixF wMat( mat );
   wMat.inverse();   

   // get offset in obj space
   Point3F offset = pos - origin;

   if ( size() == 1 )
   {
      wMat.mulV(offset);

      MatrixF transform(EulerF::Zero, -offset);
      transform.mul(MatrixF(delta));
      transform.mul(MatrixF(EulerF::Zero, offset));
      mat.mul(transform);
   }
   else
   {
      MatrixF transform( delta );
      Point3F wOffset;
      transform.mulV( offset, &wOffset );

      wMat.mulV( offset );

      transform.set( EulerF::Zero, -offset );

      mat.setColumn( 3, Point3F::Zero );
      wMat.setColumn( 3, Point3F::Zero );

      transform.mul( wMat );
      transform.mul( MatrixF(delta) );
      transform.mul( mat );
      mat.mul( transform );

      mat.normalize();
      mat.setColumn( 3, wOffset + origin );
   }

   object = mData->updateItem( object.getKey(), object.getPosition(), object.getData(), mat, object.getScale() );
}

void ForestItemSelection::scaleObject( ForestItem &object, const Point3F &delta )
{
   if ( !mData )
      return;

   object = mData->updateItem( object.getKey(), object.getPosition(), object.getData(), object.getTransform(), delta.z );
}


IMPLEMENT_CONOBJECT( ForestSelectionTool );

ConsoleDocClass( ForestSelectionTool,
   "@brief Specialized selection tool for picking individual trees in a forest.\n\n"
   "Editor use only.\n\n"
   "@internal"
);

ForestSelectionTool::ForestSelectionTool()
   :  Parent(),
      mCurrAction( NULL ),
      mGizmo( NULL ),
      mGizmoProfile( NULL )
{
   mBounds = Box3F::Invalid;

   mDragRectColor.set(255,255,0);
   mMouseDown = false;
   mDragSelect = false;
}

ForestSelectionTool::~ForestSelectionTool()
{
   SAFE_DELETE( mCurrAction );
}

void ForestSelectionTool::setParentEditor( ForestEditorCtrl *editor )
{
   mEditor = editor;
   mGizmo = editor->getGizmo();
   mGizmoProfile = mGizmo->getProfile();
}

void ForestSelectionTool::_selectItem( const ForestItem &item )
{
   // Make sure its not already selected.
   for ( U32 i=0; i < mSelection.size(); i++ )
   {
      if ( mSelection[i].getKey() == item.getKey() )
         return;
   }

   mSelection.push_back( item );
   mBounds.intersect( item.getWorldBox() );
}

void ForestSelectionTool::deleteSelection()
{
   ForestDeleteUndoAction *action = new ForestDeleteUndoAction( mForest->getData(), mEditor );

   for ( U32 i=0; i < mSelection.size(); i++ )
   {
      const ForestItem &item = mSelection[i];
      action->removeItem( item );
   }

   clearSelection();
   _submitUndo( action );
}

void ForestSelectionTool::clearSelection()
{
   mSelection.clear();
   mBounds = Box3F::Invalid;
}

void ForestSelectionTool::cutSelection()
{

}

void ForestSelectionTool::copySelection()
{

}

void ForestSelectionTool::pasteSelection()
{

}

void ForestSelectionTool::setActiveForest( Forest *forest )
{
   mForest = forest;
   
   if ( forest )
      mSelection.setForestData( forest->getData() );
   else
      mSelection.setForestData( NULL );
}

void ForestSelectionTool::on3DMouseDown( const Gui3DMouseEvent &evt )
{   
   mMouseDown = true;
   mMouseDragged = false;

   mUsingGizmo = !mSelection.empty() && mGizmo->getSelection() != Gizmo::None;

   if ( mUsingGizmo )
   {
      mGizmo->on3DMouseDown( evt );   
      return;
   }   
     
   mDragSelection.clear();
   mDragRect.set( Point2I(evt.mousePoint), Point2I(0,0) );
   mDragStart = evt.mousePoint;

   const bool multiSel = evt.modifier & SI_CTRL;

   if ( !multiSel )
      clearSelection();

   if ( mHoverItem.isValid() )
      _selectItem( mHoverItem );   

   // This should never happen... it should have been 
   // submitted and nulled in on3DMouseUp()!
   //
   // Yeah, unless you had a breakpoint there and on3DMouseUp never fired,
   // in which case this is really annoying.
   //
   //AssertFatal( !mCurrAction, "ForestSelectionTool::on3DMouseDown() - Dangling undo action!" );
}

void ForestSelectionTool::on3DMouseMove( const Gui3DMouseEvent &evt )
{
   // Invalidate the hover item first... we're gonna find a new one.
   mHoverItem.makeInvalid();

   if ( !mForest )
      return;   

   if ( !mSelection.empty() )
      mGizmo->on3DMouseMove( evt );

   RayInfo ri;
   ri.userData = new ForestItem;
   Point3F startPnt = evt.pos;
   Point3F endPnt = evt.pos + evt.vec * 1000.0f;   

   if ( mForest->castRayRendered( startPnt, endPnt, &ri ) )
      mHoverItem = (*(ForestItem*)ri.userData);

   delete static_cast<ForestItem*>(ri.userData);
}

void ForestSelectionTool::on3DMouseDragged( const Gui3DMouseEvent &evt )
{
   mHoverItem.makeInvalid();

   if ( mUsingGizmo )
   {
      mGizmo->on3DMouseDragged( evt );

      const Point3F &deltaRot = mGizmo->getDeltaRot();
      const Point3F &deltaPos = mGizmo->getOffset();
      const Point3F &deltaScale = mGizmo->getDeltaScale();

      if ( deltaRot.isZero() && deltaPos.isZero() && deltaScale.isZero() )
         return;

      // Store the current item states!
      if ( !mCurrAction )
      {
         mCurrAction = new ForestUpdateAction( mForest->getData(), mEditor );
         for ( U32 i=0; i < mSelection.size(); i++ )
         {
            const ForestItem &item = mSelection[i];
            mCurrAction->saveItem( item );
         }
      }

      switch ( mGizmo->getMode() )
      {
      case MoveMode:
         mSelection.offset( deltaPos ); break;
      case RotateMode:
         mSelection.rotate( deltaRot ); break;
      case ScaleMode:         
         mSelection.scale( mGizmo->getScale() ); break;
      default: ;
      }

      return;
   }

   mDragSelect = true;
   mHoverItem.makeInvalid();

   // Doing a drag selection.

   if ( mDragSelect )
   {
      // build the drag selection on the renderScene method - make sure no neg extent!
      mDragRect.point.x = (evt.mousePoint.x < mDragStart.x) ? evt.mousePoint.x : mDragStart.x;
      mDragRect.extent.x = (evt.mousePoint.x > mDragStart.x) ? evt.mousePoint.x - mDragStart.x : mDragStart.x - evt.mousePoint.x;
      mDragRect.point.y = (evt.mousePoint.y < mDragStart.y) ? evt.mousePoint.y : mDragStart.y;
      mDragRect.extent.y = (evt.mousePoint.y > mDragStart.y) ? evt.mousePoint.y - mDragStart.y : mDragStart.y - evt.mousePoint.y;
      return;
   }   
}

void ForestSelectionTool::on3DMouseUp( const Gui3DMouseEvent &evt )
{
   mGizmo->on3DMouseUp( evt );

   mMouseDown = false;

   // If we have an undo action then we must have
   // moved, rotated, or scaled something.
   if ( mCurrAction )
   {
      _submitUndo( mCurrAction );
      mCurrAction = NULL;
      return;
   }

   // If we were performing a drag select, finalize it now.
   if ( mDragSelect )
   {
      mDragSelect = false;

      clearSelection();

      for ( S32 i = 0; i < mDragSelection.size(); i++ )      
         _selectItem( mDragSelection[i] );
      
      mDragSelection.clear();
      
      return;
   }
}

void ForestSelectionTool::onRender3D()
{
   GFXDrawUtil *drawUtil = GFX->getDrawUtil();
   ColorI color( 255, 255, 255, 255 );
   MatrixF treeMat;

   GFXStateBlockDesc desc;
   desc.setBlend( true );
   desc.setZReadWrite( true, false );

   if ( mHoverItem.isValid() )
   {      
      treeMat = mHoverItem.getTransform();
      drawUtil->drawObjectBox( desc, mHoverItem.getSize(), mHoverItem.getWorldBox().getCenter(), treeMat, color );
   }

   if ( !mSelection.empty() )
   {      
      for ( U32 i = 0; i < mSelection.size(); i++ )
      {
         const ForestItem &item = mSelection[i];
         treeMat = item.getTransform();
         drawUtil->drawObjectBox( desc, item.getSize(), item.getWorldBox().getCenter(), treeMat, color );
      }

      mGizmo->set( mSelection.getOrientation(), mSelection.getOrigin(), mSelection.getScale() );

      mGizmo->renderGizmo( mEditor->getLastCameraQuery().cameraMatrix, mEditor->getLastCameraQuery().fov );  
   }
}

static Frustum gDragFrustum;

void ForestSelectionTool::onRender2D()
{
   // Draw drag selection rect.
   if ( mDragSelect && mDragRect.extent.x > 1 && mDragRect.extent.y > 1 )
      GFX->getDrawUtil()->drawRect( mDragRect, mDragRectColor );

   // update what is in the selection
   if ( mDragSelect )
      mDragSelection.clear();

   // Determine selected objects based on the drag box touching
   // a mesh if a drag operation has begun.
   if ( mDragSelect && mDragRect.extent.x > 1 && mDragRect.extent.y > 1 )
   {
      // Build the drag frustum based on the rect
      F32 wwidth;
      F32 wheight;
      F32 aspectRatio = F32(mEditor->getWidth()) / F32(mEditor->getHeight());

      const CameraQuery &lastCameraQuery = mEditor->getLastCameraQuery();
      if(!lastCameraQuery.ortho)
      {
         wheight = lastCameraQuery.nearPlane * mTan(lastCameraQuery.fov / 2);
         wwidth = aspectRatio * wheight;
      }
      else
      {
         wheight = lastCameraQuery.fov;
         wwidth = aspectRatio * wheight;
      }

      F32 hscale = wwidth * 2 / F32(mEditor->getWidth());
      F32 vscale = wheight * 2 / F32(mEditor->getHeight());

      F32 left = (mDragRect.point.x - mEditor->getPosition().x) * hscale - wwidth;
      F32 right = (mDragRect.point.x - mEditor->getPosition().x + mDragRect.extent.x) * hscale - wwidth;
      F32 top = wheight - vscale * (mDragRect.point.y - mEditor->getPosition().y);
      F32 bottom = wheight - vscale * (mDragRect.point.y - mEditor->getPosition().y + mDragRect.extent.y);
      gDragFrustum.set(lastCameraQuery.ortho, left, right, top, bottom, lastCameraQuery.nearPlane, lastCameraQuery.farPlane, lastCameraQuery.cameraMatrix );

      mForest->getData()->getItems( gDragFrustum, &mDragSelection );      
   }
}

bool ForestSelectionTool::updateGuiInfo()
{
   SimObject *statusbar;
   if ( !Sim::findObject( "EditorGuiStatusBar", statusbar ) )
      return false;

   String text( "Forest Editor." );      
   GizmoMode mode = mGizmoProfile->mode;

   if ( mMouseDown && mGizmo->getSelection() != Gizmo::None )
   {
      Point3F delta;
      String qualifier;

      if ( mode == RotateMode )   
         delta = mGizmo->getDeltaTotalRot();         
      else if ( mode == MoveMode )         
         delta = mGizmo->getTotalOffset();         
      else if ( mode == ScaleMode )
         delta = mGizmo->getDeltaTotalScale();            

      if ( mGizmo->getAlignment() == Object && mode != ScaleMode )
      {       
         mSelection.getOrientation().mulV( delta );
      }

      if ( mIsZero( delta.x, 0.0001f ) )
         delta.x = 0.0f;
      if ( mIsZero( delta.y, 0.0001f ) )
         delta.y = 0.0f;
      if ( mIsZero( delta.z, 0.0001f ) )
         delta.z = 0.0f;

      if ( mode == RotateMode )         
      {         
         delta.x = mRadToDeg( delta.x );
         delta.y = mRadToDeg( delta.y );
         delta.z = mRadToDeg( delta.z );
         text = String::ToString( "Delta angle ( x: %4.2f, y: %4.2f, z: %4.2f ).", delta.x, delta.y, delta.z ); 
      }
      else if ( mode == MoveMode )     
         text = String::ToString( "Delta position ( x: %4.2f, y: %4.2f, z: %4.2f ).", delta.x, delta.y, delta.z ); 
      else if ( mode == ScaleMode )
         text = String::ToString( "Delta scale ( x: %4.2f, y: %4.2f, z: %4.2f ).", delta.x, delta.y, delta.z ); 
   }
   else 
   {     
      if ( mode == MoveMode )            
         text = "Move selection.  SHIFT while dragging duplicates objects.";
      else if ( mode == RotateMode )            
         text = "Rotate selection.";
      else if ( mode == ScaleMode )            
         text = "Scale selection.";        
   }   

   Con::executef( statusbar, "setInfo", text.c_str() );

   Con::executef( statusbar, "setSelectionObjectsByCount", Con::getIntArg( mSelection.size() ) );

   return true;
}

void ForestSelectionTool::updateGizmo()
{
   mGizmoProfile->restoreDefaultState();     

   const GizmoMode &mode = mGizmoProfile->mode;

   if ( mode == ScaleMode )   
   {
      mGizmoProfile->flags &= ~GizmoProfile::PlanarHandlesOn;
      mGizmoProfile->allAxesScaleUniform = true;
   }
}

void ForestSelectionTool::onUndoAction()
{
   ForestData *data = mForest->getData();

   // Remove items from our selection that no longer exist.
   for ( S32 i = 0; i < mSelection.size(); i++ )
   {
      const ForestItem &item = data->findItem( mSelection[i].getKey(), mSelection[i].getPosition() );

      if ( item == ForestItem::Invalid )
      {
         mSelection.erase_fast( i );
         i--;
      }
      else      
         mSelection[i] = item;      
   }

   // Recalculate our selection bounds.
   mBounds = Box3F::Invalid;
   for ( S32 i = 0; i < mSelection.size(); i++ )   
      mBounds.intersect( mSelection[i].getWorldBox() );
}

ConsoleMethod( ForestSelectionTool, getSelectionCount, S32, 2, 2, "" )
{
   return object->getSelectionCount();
}

ConsoleMethod( ForestSelectionTool, deleteSelection, void, 2, 2, "" )
{
   object->deleteSelection();
}

ConsoleMethod( ForestSelectionTool, clearSelection, void, 2, 2, "" )
{
   object->clearSelection();
}

ConsoleMethod( ForestSelectionTool, cutSelection, void, 2, 2, "" )
{
   object->cutSelection();
}

ConsoleMethod( ForestSelectionTool, copySelection, void, 2, 2, "" )
{
   object->copySelection();
}

ConsoleMethod( ForestSelectionTool, pasteSelection, void, 2, 2, "" )
{
   object->pasteSelection();
}

