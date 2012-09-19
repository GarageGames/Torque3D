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
#include "forest/editor/forestBrushTool.h"

#include "forest/forest.h"
#include "forest/editor/forestUndo.h"
#include "forest/editor/forestBrushElement.h"
#include "forest/editor/forestEditorCtrl.h"

#include "gui/worldEditor/editTSCtrl.h"
#include "console/consoleTypes.h"
#include "core/util/tVector.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/core/guiCanvas.h"
#include "gfx/primBuilder.h"
#include "gui/controls/guiTreeViewCtrl.h"
#include "core/strings/stringUnit.h"
#include "math/mRandomDeck.h"
#include "math/mRandomSet.h"


bool ForestBrushTool::protectedSetSize( void *object, const char *index, const char *data )
{
   ForestBrushTool *tool = static_cast<ForestBrushTool*>( object );
   F32 val = dAtof(data);
   tool->setSize( val );

   return false;
}

bool ForestBrushTool::protectedSetPressure( void *object, const char *index, const char *data )
{
   ForestBrushTool *tool = static_cast<ForestBrushTool*>( object );
   F32 val = dAtof(data);
   tool->setPressure( val );

   return false;
}

bool ForestBrushTool::protectedSetHardness( void *object, const char *index, const char *data )
{
   ForestBrushTool *tool = static_cast<ForestBrushTool*>( object );
   F32 val = dAtof(data);
   tool->setHardness( val );

   return false;
}

ImplementEnumType( ForestBrushMode,
   "Active brush mode type.\n"
   "@internal\n\n")
   { ForestBrushTool::Paint, "Paint", "Creates Items based on the Elements you have selected.\n" },
   { ForestBrushTool::Erase, "Erase", "Erases Items of any Mesh type.\n" },
   { ForestBrushTool::EraseSelected, "EraseSelected", "Erases items of a specific type.\n"  },
EndImplementEnumType;


ForestBrushTool::ForestBrushTool()
 : mSize( 5.0f ),
   mPressure( 0.1f ),
   mHardness( 1.0f ),
   mDrawBrush( false ),
   mCurrAction( NULL ),
   mStrokeEvent( 0 ),
   mBrushDown( false ),
   mColor( ColorI::WHITE ),
   mMode( Paint ),
   mRandom( Platform::getRealMilliseconds() + 1 )
{	
}

ForestBrushTool::~ForestBrushTool()
{
}

IMPLEMENT_CONOBJECT( ForestBrushTool );

ConsoleDocClass( ForestBrushTool,
   "@brief Defines the brush properties when painting trees in Forest Editor\n\n"
   "Editor use only.\n\n"
   "@internal"
);

void ForestBrushTool::initPersistFields()
{  
   addGroup( "ForestBrushTool" );
      
      addField( "mode", TYPEID< BrushMode >(), Offset( mMode, ForestBrushTool) );
      
      addProtectedField( "size", TypeF32, Offset( mSize, ForestBrushTool ), 
         &protectedSetSize, &defaultProtectedGetFn, "Brush Size" );

      addProtectedField( "pressure", TypeF32, Offset( mPressure, ForestBrushTool ), 
         &protectedSetPressure, &defaultProtectedGetFn, "Brush Pressure" );

      addProtectedField( "hardness", TypeF32, Offset( mHardness, ForestBrushTool ), 
         &protectedSetHardness, &defaultProtectedGetFn, "Brush Hardness" );

   endGroup( "ForestBrushTool" );

   Parent::initPersistFields();
}

bool ForestBrushTool::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   return true;
}

void ForestBrushTool::onRemove()
{	
   Parent::onRemove();
}

void ForestBrushTool::on3DMouseDown( const Gui3DMouseEvent &evt )
{   
   Con::executef( this, "onMouseDown" );

   if ( !_updateBrushPoint( evt ) || !mForest )
      return;

   mBrushDown = true;   

   mEditor->getRoot()->showCursor( false );

   _collectElements();
   _onStroke();

   return;
}

void ForestBrushTool::on3DMouseUp( const Gui3DMouseEvent &evt )
{
   _updateBrushPoint( evt );
   Sim::cancelEvent( mStrokeEvent );
   mBrushDown = false;  

   mEditor->getRoot()->showCursor( true );

   if ( mCurrAction )
   {
      _submitUndo( mCurrAction );
      mCurrAction = NULL;
   }

   //mElements.clear();
}

void ForestBrushTool::on3DMouseMove( const Gui3DMouseEvent &evt )
{
   _updateBrushPoint( evt );   
}

void ForestBrushTool::on3DMouseDragged( const Gui3DMouseEvent &evt )
{
   _updateBrushPoint( evt );

   if ( mBrushDown && !Sim::isEventPending( mStrokeEvent ) )         
      mStrokeEvent = Sim::postEvent( this, new ForestBrushToolEvent(), Sim::getCurrentTime() + 250 );   
}

bool ForestBrushTool::onMouseWheel( const GuiEvent &evt )
{
   if ( evt.modifier & SI_PRIMARY_CTRL )
      setPressure( mPressure + evt.fval * ( 0.05f / 120.0f ) );
   else if ( evt.modifier & SI_SHIFT )
      setHardness( mHardness + evt.fval * ( 0.05f / 120.0f ) );
   else
      setSize( mSize + evt.fval * ( 1.0f / 120.0f ) );

   return true;
}

void ForestBrushTool::onRender3D( )
{      
}

void ForestBrushTool::onRender2D()
{
   if ( !mDrawBrush )
      return;

   if ( !mEditor->getRoot()->isCursorON() && !mBrushDown )
      return;   
   
   RayInfo ri;
   Point3F start( 0, 0, mLastBrushPoint.z + mSize );
   Point3F end( 0, 0, mLastBrushPoint.z - mSize );

   Vector<Point3F> pointList;

   const U32 steps = 32;

   if ( mForest )
      mForest->disableCollision();

   for ( S32 i = 0; i < steps; i++ )
   {
      F32 radians = (F32)i / (F32)(steps-1) * M_2PI_F;
      VectorF vec(0,1,0);
      MathUtils::vectorRotateZAxis( vec, radians );

      end.x = start.x = mLastBrushPoint.x + vec.x * mSize;
      end.y = start.y = mLastBrushPoint.y + vec.y * mSize;

      bool hit = gServerContainer.castRay( start, end, TerrainObjectType | StaticShapeObjectType, &ri );

      if ( hit )
         pointList.push_back( ri.point );
   }

   if ( mForest )
      mForest->enableCollision();

   if ( pointList.empty() )
      return;

   ColorI brushColor( ColorI::WHITE );

   if ( mMode == Paint )
      brushColor = ColorI::BLUE;
   else if ( mMode == Erase )
      brushColor = ColorI::RED;
   else if ( mMode == EraseSelected )
      brushColor.set( 150, 0, 0 );

   if ( mMode == Paint || mMode == EraseSelected )
   {
      if ( mElements.empty() )
      {
         brushColor.set( 140, 140, 140 );
      }
   }
   
   mEditor->drawLineList( pointList, brushColor, 1.5f );   
}

void ForestBrushTool::onActivated( const Gui3DMouseEvent &lastEvent )
{
   _updateBrushPoint( lastEvent );

   Con::executef( this, "onActivated" );
}

void ForestBrushTool::onDeactivated()
{
   Con::executef( this, "onDeactivated" );
}

bool ForestBrushTool::updateGuiInfo()
{
   GuiTextCtrl *statusbar;
   Sim::findObject( "EWorldEditorStatusBarInfo", statusbar );

   GuiTextCtrl *selectionBar;
   Sim::findObject( "EWorldEditorStatusBarSelection", selectionBar );

   String text;

   if ( mMode == Paint )
      text = "Forest Editor ( Paint Tool ) - This brush creates Items based on the Elements you have selected.";
   else if ( mMode == Erase )
      text = "Forest Editor ( Erase Tool ) - This brush erases Items of any Mesh type.";
   else if ( mMode == EraseSelected )   
      text = "Forest Editor ( Erase Selected ) - This brush erases Items based on the Elements you have selected.";      
   
   if ( statusbar )
      statusbar->setText( text );

   if ( mMode == Paint || mMode == EraseSelected )
      text = String::ToString( "%i elements selected", mElements.size() );
   else
      text = "";

   if ( selectionBar )
      selectionBar->setText( text );

   return true;
}

void ForestBrushTool::setSize( F32 val )
{
   mSize = mClampF( val, 0.0f, 150.0f );
   Con::executef( this, "syncBrushToolbar" );
}

void ForestBrushTool::setPressure( F32 val )
{   
   mPressure = mClampF( val, 0.0f, 1.0f );
   Con::executef( this, "syncBrushToolbar" );
}

void ForestBrushTool::setHardness( F32 val )
{
   mHardness = mClampF( val, 0.0f, 1.0f );
   Con::executef( this, "syncBrushToolbar" );
}

void ForestBrushTool::_onStroke()
{
   if ( !mForest || !mBrushDown )
      return;

   _action( mLastBrushPoint );
}

void ForestBrushTool::_action( const Point3F &point )
{
   if ( mMode == Paint )
      _paint( point );
   else if ( mMode == Erase || mMode == EraseSelected )
      _erase( point );
}

inline F32 mCircleArea( F32 radius )
{
   return radius * radius * M_PI_F;   
}

void ForestBrushTool::_paint( const Point3F &point )
{
   AssertFatal( mForest, "ForestBrushTool::_paint() - Can't paint without a Forest!" );

   if ( mElements.empty() )
      return;

   // Iterators, pointers, and temporaries.
   ForestBrushElement *pElement;
   ForestItemData *pData;
   F32 radius, area;
   
   // How much area do we have to fill with trees ( within the brush ).
   F32 fillArea = mCircleArea( mSize );

   // Scale that down by pressure, this is how much area we want to fill.
   fillArea *= mPressure;

   // Create an MRandomSet we can get items we are painting out of with
   // the desired distribution.
   // Also grab the smallest and largest radius elements while we are looping.
   
   ForestBrushElement *smallestElement, *largestElement;
   smallestElement = largestElement = mElements[0];

   MRandomSet<ForestBrushElement*> randElementSet(&mRandom);

   for ( S32 i = 0; i < mElements.size(); i++ )
   {
      pElement = mElements[i];

      if ( pElement->mData->mRadius > largestElement->mData->mRadius )
         largestElement = pElement;
      if ( pElement->mData->mRadius < smallestElement->mData->mRadius )
         smallestElement = pElement;

      randElementSet.add( pElement, pElement->mProbability );
   }
   
   // Pull elements from the random set until we would theoretically fill
   // the desired area.

   F32 areaLeft = fillArea;
   F32 scaleFactor, sink, randRot, worldCoordZ, slope;
   Point2F worldCoords;
   Point3F normalVector, right;
   Point2F temp;

   const S32 MaxTries = 5;


   while ( areaLeft > 0.0f )
   {
      pElement = randElementSet.get();
      pData = pElement->mData;

      scaleFactor =  mLerp( pElement->mScaleMin, pElement->mScaleMax, mClampF( mPow( mRandom.randF(), pElement->mScaleExponent ), 0.0f, 1.0f ) );
      radius = getMax( pData->mRadius * scaleFactor, 0.1f );
      area = mCircleArea( radius );

      areaLeft -= area * 5.0f; // fudge value

      // No room left we are done.
      //if ( areaLeft < 0.0f )
      //   break;

      // We have area left to fill...
      
      const F32 rotRange = mDegToRad( pElement->mRotationRange );

      // Get a random sink value.
      sink = mRandom.randF( pElement->mSinkMin, pElement->mSinkMax ) * scaleFactor;

      // Get a random rotation.
      randRot = mRandom.randF( 0.0f, rotRange );

      // Look for a place within the brush area to place this item.
      // We may have to try several times or give and go onto the next.

      S32 i = 0;
      for ( ; i < MaxTries; i++ )
      {
         // Pick some randoms for placement.
         worldCoords = MathUtils::randomPointInCircle( mSize ) + point.asPoint2F();                  

         // Look for the ground at this position.
         if ( !getGroundAt( Point3F( worldCoords.x, worldCoords.y , point.z ), 
                                     &worldCoordZ, 
                                     &normalVector ) )
         {
            continue;
         }

         // Does this pass our slope and elevation limits.
         right.set( normalVector.x, normalVector.y, 0 );
         right.normalizeSafe();
         slope = mRadToDeg( mDot( right, normalVector ) );

         if ( worldCoordZ < pElement->mElevationMin ||
              worldCoordZ > pElement->mElevationMax ||
              slope < pElement->mSlopeMin ||
              slope > pElement->mSlopeMax )
         {
            continue;
         }

         // Are we up against another tree?
         if ( mForest->getData()->getItems( worldCoords, radius, NULL ) > 0 )
            continue;

         // If the trunk radius is set then we need to sink
         // the tree into the ground a bit to hide the bottom.
         if ( pElement->mSinkRadius > 0.0f )
         {
            // Items that are not aligned to the ground surface
            // get sunken down to hide the bottom of their trunks.

            // sunk down a bit to hide their bottoms on slopes.
            normalVector.z = 0;
            normalVector.normalizeSafe();
            normalVector *= sink;
            temp = worldCoords + normalVector.asPoint2F();
            getGroundAt( Point3F( temp.x, temp.y, point.z ),
                         &worldCoordZ, 
                         NULL );
         }

         worldCoordZ -= sink;

         // Create a new undo action if this is a new stroke.
         ForestCreateUndoAction *action = dynamic_cast<ForestCreateUndoAction*>( mCurrAction );
         if ( !action )
         {
            action = new ForestCreateUndoAction( mForest->getData(), mEditor );
            mCurrAction = action;
         }

         //Con::printf( "worldCoords = %g, %g, %g", worldCoords.x, worldCoords.y, worldCoordZ );

         // Let the action manage adding it to the forest.
         action->addItem( pData,
                          Point3F( worldCoords.x, worldCoords.y, worldCoordZ ),
                          randRot,
                          scaleFactor );
         break;
      }
   }      
}

void ForestBrushTool::_erase( const Point3F &point )
{
   AssertFatal( mForest, "ForestBrushTool::_erase() - Can't erase without a Forest!" );

   // First grab all the forest items around the point.
   ForestItemVector trees;
   if ( mForest->getData()->getItems( point.asPoint2F(), mSize, &trees ) == 0 )
      return;

   if ( mMode == EraseSelected )
   {
      for ( U32 i = 0; i < trees.size(); i++ )
      {
         const ForestItem &tree = trees[i];

         if ( !mDatablocks.contains( tree.getData() ) )    
         {
            trees.erase_fast( i );
            i--;
         }
      }
   }

   if ( trees.empty() )
      return;

   // Number of trees to erase depending on pressure.
   S32 eraseCount = getMax( (S32)mCeil( (F32)trees.size() * mPressure ), 0 );
   
   // Initialize an MRandomDeck with trees under the brush.
   MRandomDeck<ForestItem> deck(&mRandom);
   deck.addToPile( trees );
   deck.shuffle();

   ForestItem currentTree;
   
   // Draw eraseCount number of trees from MRandomDeck, adding them to our erase action.
   for ( U32 i = 0; i < eraseCount; i++ )
   {
      deck.draw(&currentTree);

      // Create a new undo action if this is a new stroke.
      ForestDeleteUndoAction *action = dynamic_cast<ForestDeleteUndoAction*>( mCurrAction );
      if ( !action )
      {
         action = new ForestDeleteUndoAction( mForest->getData(), mEditor );
         mCurrAction = action;
      }

      action->removeItem( currentTree );      
   }
}

bool ForestBrushTool::_updateBrushPoint( const Gui3DMouseEvent &event_ )
{
   // Do a raycast for terrain... thats the placement center.
   const U32 mask = TerrainObjectType | StaticShapeObjectType; // TODO: Make an option!

   Point3F start( event_.pos );
   Point3F end( event_.pos + ( event_.vec * 10000.0f ) );

   if ( mForest )
      mForest->disableCollision();

   RayInfo rinfo;
   mDrawBrush = gServerContainer.castRay( start, end, mask, &rinfo );

   if ( mForest )
      mForest->enableCollision();

   if ( mDrawBrush )
   {
      mLastBrushPoint = rinfo.point;
      mLastBrushNormal = rinfo.normal;
   }

   return mDrawBrush;
}

bool findSelectedElements( ForestBrushElement *obj )
{   
   if ( obj->isSelectedRecursive() )
      return true;

   return false;
}

void ForestBrushTool::_collectElements()
{
   mElements.clear();

   // Get the selected objects from the tree view.
   // These can be a combination of ForestBrush(s) and ForestBrushElement(s).

   GuiTreeViewCtrl *brushTree;
   if ( !Sim::findObject( "ForestEditBrushTree", brushTree ) )
      return;
      
   const char* objectIdList = Con::executef( brushTree, "getSelectedObjectList" );

   // Collect those objects in a vector and mark them as selected.

   Vector<SimObject*> objectList;
   SimObject *simobj;

   S32 wordCount = StringUnit::getUnitCount( objectIdList, " " );
   
   for ( S32 i = 0; i < wordCount; i++ )
   {
      const char* word = StringUnit::getUnit( objectIdList, i, " " );
      
      if ( Sim::findObject( word, simobj ) )
      {
         objectList.push_back( simobj );
         simobj->setSelected(true);
      }
   }

   // Find all ForestBrushElements that are directly or indirectly selected.

   SimGroup *brushGroup = ForestBrush::getGroup();
   brushGroup->findObjectByCallback( findSelectedElements, mElements );

   // We just needed to flag these objects as selected for the benefit of our
   // findSelectedElements callback, we can now mark them un-selected again.

   for ( S32 i = 0; i < objectList.size(); i++ )   
      objectList[i]->setSelected(false);   

   // If we are in Paint or EraseSelected mode we filter out elements with
   // a non-positive probability. 
   
   if ( mMode == Paint || mMode == EraseSelected )
   {
      for ( S32 i = 0; i < mElements.size(); i++ )   
      {
         if ( mElements[i]->mProbability <= 0.0f )
         {
            mElements.erase_fast(i);
            i--;
         }
      }
   }

   // Filter out elements with NULL datablocks and collect all unique datablocks
   // in a vector.

   mDatablocks.clear();
   for ( S32 i = 0; i < mElements.size(); i++ )   
   {
      if ( mElements[i]->mData == NULL )
      {
         mElements.erase_fast(i);
         i--;
         continue;
      }

      mDatablocks.push_back_unique( mElements[i]->mData );   
   }
}

bool ForestBrushTool::getGroundAt( const Point3F &worldPt, float *zValueOut, VectorF *normalOut )
{
   const U32 mask = TerrainObjectType | StaticShapeObjectType;

   Point3F start( worldPt.x, worldPt.y, worldPt.z + mSize );
   Point3F end( worldPt.x, worldPt.y, worldPt.z - mSize );

   if ( mForest )
      mForest->disableCollision();

   // Do a cast ray at this point from the top to 
   // the bottom of our brush radius.

   RayInfo rinfo;   
   bool hit = gServerContainer.castRay( start, end, mask, &rinfo );

   if ( mForest )
      mForest->enableCollision();

   if ( !hit )
      return false;

   if (zValueOut)
      *zValueOut = rinfo.point.z;

   if (normalOut)
      *normalOut = rinfo.normal;

   return true;
}

ConsoleMethod( ForestBrushTool, collectElements, void, 2, 2, "" )
{
   object->collectElements();
}