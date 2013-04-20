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
#include "gui/worldEditor/terrainEditor.h"

#include "core/frameAllocator.h"
#include "core/strings/stringUnit.h"
#include "console/consoleTypes.h"
#include "console/simEvents.h"
#include "sim/netConnection.h"
#include "math/mathUtils.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "gui/core/guiCanvas.h"
#include "gui/worldEditor/terrainActions.h"
#include "terrain/terrMaterial.h"



IMPLEMENT_CONOBJECT(TerrainEditor);

ConsoleDocClass( TerrainEditor,
   "@brief The base Terrain Editor tool\n\n"
   "Editor use only.\n\n"
   "@internal"
);

Selection::Selection() :
   Vector<GridInfo>(__FILE__, __LINE__),
   mName(0),
   mUndoFlags(0),
   mHashListSize(1024)
{
   VECTOR_SET_ASSOCIATION(mHashLists);

   // clear the hash list
   mHashLists.setSize(mHashListSize);
   reset();
}

Selection::~Selection()
{
}

void Selection::reset()
{
   PROFILE_SCOPE( TerrainEditor_Selection_Reset );

   for(U32 i = 0; i < mHashListSize; i++)
      mHashLists[i] = -1;
   clear();
}

bool Selection::validate()
{
   PROFILE_SCOPE( TerrainEditor_Selection_Validate );

   // scan all the hashes and verify that the heads they point to point back to them
   U32 hashesProcessed = 0;
   for(U32 i = 0; i < mHashLists.size(); i++)
   {
      U32 entry = mHashLists[i];
      if(entry == -1)
         continue;
      
      GridInfo info = (*this)[entry];
      U32 hashIndex = getHashIndex(info.mGridPoint.gridPos);
      
      if( entry != mHashLists[hashIndex] )
      {
         AssertFatal(false, "Selection hash lists corrupted");
         return false;
      }
      hashesProcessed++;
   }

   // scan all the entries and verify that anything w/ a prev == -1 is correctly in the hash table
   U32 headsProcessed = 0;
   for(U32 i = 0; i < size(); i++)
   {
      GridInfo info = (*this)[i];
      if(info.mPrev != -1)
         continue;

      U32 hashIndex = getHashIndex(info.mGridPoint.gridPos);

      if(mHashLists[hashIndex] != i)
      {
         AssertFatal(false, "Selection list heads corrupted");       
         return false;
      }
      headsProcessed++;
   }
   AssertFatal(headsProcessed == hashesProcessed, "Selection's number of hashes and number of list heads differ.");
   return true;
}

U32 Selection::getHashIndex(const Point2I & pos)
{
   PROFILE_SCOPE( TerrainEditor_Selection_GetHashIndex );

   Point2F pnt = Point2F((F32)pos.x, (F32)pos.y) + Point2F(1.3f,3.5f);
   return( (U32)(mFloor(mHashLists.size() * mFmod(pnt.len() * 0.618f, 1.0f))) );
}

S32 Selection::lookup(const Point2I & pos)
{
   PROFILE_SCOPE( TerrainEditor_Selection_Lookup );

   U32 index = getHashIndex(pos);

   S32 entry = mHashLists[index];

   while(entry != -1)
   {
      if((*this)[entry].mGridPoint.gridPos == pos)
         return(entry);

      entry = (*this)[entry].mNext;
   }

   return(-1);
}

void Selection::insert(GridInfo info)
{
   PROFILE_SCOPE( TerrainEditor_Selection_Insert );

   //validate();
   // get the index into the hash table
   U32 index = getHashIndex(info.mGridPoint.gridPos);

   // if there is an existing linked list, make it our next
   info.mNext = mHashLists[index];
   info.mPrev = -1;

   // if there is an existing linked list, make us it's prev
   U32 indexOfNewEntry = size();
   if(info.mNext != -1)
      (*this)[info.mNext].mPrev = indexOfNewEntry;

   // the hash table holds the heads of the linked lists. make us the head of this list.
   mHashLists[index] = indexOfNewEntry;

   // copy us into the vector
   push_back(info);
   //validate();
}

bool Selection::remove(const GridInfo &info)
{
   PROFILE_SCOPE( TerrainEditor_Selection_Remove );

   if(size() < 1)
      return false;

   //AssertFatal( validate(), "Selection hashLists corrupted before Selection.remove()");

   U32 hashIndex = getHashIndex(info.mGridPoint.gridPos);
   S32 listHead = mHashLists[hashIndex];
   //AssertFatal(listHead < size(), "A Selection's hash table is corrupt.");

   if(listHead == -1)
      return(false);

   const S32 victimEntry = lookup(info.mGridPoint.gridPos);
   if( victimEntry == -1 )
      return(false);

   const GridInfo victim = (*this)[victimEntry];
   const S32 vicPrev = victim.mPrev;
   const S32 vicNext = victim.mNext;
      
   // remove us from the linked list, if there is one.
   if(vicPrev != -1)
      (*this)[vicPrev].mNext = vicNext;
   if(vicNext != -1)
      (*this)[vicNext].mPrev = vicPrev;
   
   // if we were the head of the list, make our next the new head in the hash table.
   if(vicPrev == -1)
      mHashLists[hashIndex] = vicNext;

   // if we're not the last element in the vector, copy the last element to our position.
   if(victimEntry != size() - 1)
   {
      // copy last into victim, and re-cache next & prev
      const GridInfo lastEntry = last();
      const S32 lastPrev = lastEntry.mPrev;
      const S32 lastNext = lastEntry.mNext;
      (*this)[victimEntry] = lastEntry;
      
      // update the new element's next and prev, to reestablish it in it's linked list.
      if(lastPrev != -1)
         (*this)[lastPrev].mNext = victimEntry;
      if(lastNext != -1)
         (*this)[lastNext].mPrev = victimEntry;

      // if it was the head of it's list, update the hash table with its new position.
      if(lastPrev == -1)
      {
         const U32 lastHash = getHashIndex(lastEntry.mGridPoint.gridPos);
         AssertFatal(mHashLists[lastHash] == size() - 1, "Selection hashLists corrupted during Selection.remove() (oldmsg)");
         mHashLists[lastHash] = victimEntry;
      }
   }
   
   // decrement the vector, we're done here
   pop_back();
   //AssertFatal( validate(), "Selection hashLists corrupted after Selection.remove()");
   return true;
}

bool Selection::add(const GridInfo &info)
{
   PROFILE_SCOPE( TerrainEditor_Selection_Add );

   S32 index = lookup(info.mGridPoint.gridPos);
   if(index != -1)
      return(false);

   insert(info);
   return(true);
}

bool Selection::getInfo(Point2I pos, GridInfo & info)
{
   PROFILE_SCOPE( TerrainEditor_Selection_GetInfo );

   S32 index = lookup(pos);
   if(index == -1)
      return(false);

   info = (*this)[index];
   return(true);
}

bool Selection::setInfo(GridInfo & info)
{
   PROFILE_SCOPE( TerrainEditor_Selection_SetInfo );

   S32 index = lookup(info.mGridPoint.gridPos);
   if(index == -1)
      return(false);

   S32 next = (*this)[index].mNext;
   S32 prev = (*this)[index].mPrev;

   (*this)[index] = info;
   (*this)[index].mNext = next;
   (*this)[index].mPrev = prev;

   return(true);
}

F32 Selection::getAvgHeight()
{
   PROFILE_SCOPE( TerrainEditor_Selection_GetAvgHeight );

   if(!size())
      return(0);

   F32 avg = 0.f;
   for(U32 i = 0; i < size(); i++)
      avg += (*this)[i].mHeight;

   return(avg / size());
}

F32 Selection::getMinHeight()
{
   PROFILE_SCOPE( TerrainEditor_Selection_GetMinHeight );

   if(!size())
      return(0);

   F32 minHeight = (*this)[0].mHeight;
   for(U32 i = 1; i < size(); i++)
      minHeight = getMin(minHeight, (*this)[i].mHeight);

   return minHeight;
}

F32 Selection::getMaxHeight()
{
   PROFILE_SCOPE( TerrainEditor_Selection_GetMaxHeight );

   if(!size())
      return(0);

   F32 maxHeight = (*this)[0].mHeight;
   for(U32 i = 1; i < size(); i++)
      maxHeight = getMax(maxHeight, (*this)[i].mHeight);

   return maxHeight;
}

Brush::Brush(TerrainEditor * editor) :
   mTerrainEditor(editor)
{
   mSize = mTerrainEditor->getBrushSize();
}

const Point2I & Brush::getPosition()
{
   return(mGridPoint.gridPos);
}

const GridPoint & Brush::getGridPoint()
{
   return mGridPoint;
}

void Brush::setPosition(const Point3F & pos)
{
   PROFILE_SCOPE( TerrainEditor_Brush_SetPosition_Point3F );

   mTerrainEditor->worldToGrid(pos, mGridPoint);
   update();
}

void Brush::setPosition(const Point2I & pos)
{
   PROFILE_SCOPE( TerrainEditor_Brush_SetPosition_Point2I );

   mGridPoint.gridPos = pos;
   update();
}

void Brush::update()
{
   PROFILE_SCOPE( TerrainEditor_Brush_update );

   if ( mGridPoint.terrainBlock )
      rebuild();
}

void Brush::render()
{
   PROFILE_SCOPE( TerrainEditor_Brush_Render );

   // Render the brush's outline via the derived brush class.
   _renderOutline();

   // Render the brush's interior grid points.

   const U32 pointCount = mSize.x * mSize.y;
   if ( pointCount == 0 )
      return;

   if ( mRenderList.empty() || empty() )
      return;

   Vector<GFXVertexPCT> pointList;
   pointList.reserve( pointCount );

   for(S32 x = 0; x < mSize.x; x++)
   {
      for(S32 y = 0; y < mSize.y; y++)
      {   
         S32 id = mRenderList[x*mSize.x+y];
         if ( id == -1 )
            continue;

         const GridInfo &gInfo = (*this)[ id ];                        

         Point3F pos;
         mTerrainEditor->gridToWorld( gInfo.mGridPoint.gridPos, pos, gInfo.mGridPoint.terrainBlock );
         
         if ( !mTerrainEditor->project( pos, &pos ) )
            continue;

         pointList.increment();
         GFXVertexPCT &pointInfo = pointList.last();

         pointInfo.point = pos;
         
         pointInfo.color.set( 255, 0, 255, gInfo.mWeight * 255 );      
         
         pointInfo.texCoord.set( 1.0f, 0.0f );
      }
   }

   mTerrainEditor->renderPoints( pointList );
}

void BoxBrush::rebuild()
{
   PROFILE_SCOPE( TerrainEditor_BoxBrush_Rebuild );

   reset();

   const F32 squareSize = mGridPoint.terrainBlock->getSquareSize();

   mRenderList.setSize(mSize.x*mSize.y);
   
   Point3F center( F32(mSize.x - 1) / 2.0f * squareSize, F32(mSize.y - 1) / 2.0f * squareSize, 0.0f );
   
   Filter filter;
   filter.set(1, &mTerrainEditor->mSoftSelectFilter);
   
   const Point3F mousePos = mTerrainEditor->getMousePos();   

   F32 xFactorScale = center.x / ( center.x + 0.5f );
   F32 yFactorScale = center.y / ( center.y + 0.5f );
   
   const F32 softness = mTerrainEditor->getBrushSoftness();
   const F32 pressure = mTerrainEditor->getBrushPressure();

   Point3F posw( 0,0,0 );
   Point2I posg( 0,0 );
   Vector<GridInfo> infos;

   for ( S32 x = 0; x < mSize.x; x++ )
   {
      for(S32 y = 0; y < mSize.y; y++)
      {
         F32 xFactor = 0.0f;
         if ( center.x > 0 )
            xFactor = mAbs( center.x - x ) / center.x * xFactorScale;

         F32 yFactor = 0.0f;
         if ( center.y > 0 )
            yFactor = mAbs( center.y - y ) / center.y * yFactorScale;

         S32 &rl = mRenderList[x*mSize.x+y];
         
         posw.x = mousePos.x + (F32)x * squareSize - center.x;
         posw.y = mousePos.y + (F32)y * squareSize - center.y;
         // round to grid coords
         GridPoint gridPoint = mGridPoint;
         mTerrainEditor->worldToGrid( posw, gridPoint );         

         // Check that the grid point is valid within the terrain.  This assumes
         // that there is no wrap around past the edge of the terrain.
         if(!mTerrainEditor->isPointInTerrain(gridPoint))
         {
            rl = -1;
            continue;
         }
         
         infos.clear();
         mTerrainEditor->getGridInfos( gridPoint, infos );

         for (U32 z = 0; z < infos.size(); z++)
         {
            infos[z].mWeight = pressure *
               mLerp( infos[z].mWeight, filter.getValue(xFactor > yFactor ? xFactor : yFactor), softness );

            push_back(infos[z]);
         }

         rl = size()-1;
      }
   }
}

void BoxBrush::_renderOutline()
{
   F32 squareSize = mGridPoint.terrainBlock->getSquareSize();

   RayInfo ri;
   Point3F start( 0, 0, 5000.0f );
   Point3F end( 0, 0, -5000.0f );
   bool hit;

   Vector<Point3F> pointList;
   pointList.reserve( 64 );

   const ColorI col( 255, 0, 255, 255 );

   const Point3F &mousePos = mTerrainEditor->getMousePos();
   
   static const Point2F offsetArray [5] =
   {
      Point2F( -1, -1 ),
      Point2F( 1, -1 ),
      Point2F( 1, 1 ),
      Point2F( -1, 1 ),
      Point2F( -1, -1 ) // repeat of offset[0]
   };

   // 64 total steps, 4 sides to the box, 16 steps per side.
   // 64 / 4 = 16
   const U32 steps = 16; 
   
   for ( S32 i = 0; i < 4; i++ )
   {            
      const Point2F &offset = offsetArray[i];
      const Point2F &next = offsetArray[i+1];      

      for ( S32 j = 0; j < steps; j++ )
      {
         F32 frac = (F32)j / ( (F32)steps - 1.0f );
         
         Point2F tmp;
         tmp.interpolate( offset, next, frac );         

         start.x = end.x = mousePos.x + tmp.x * squareSize * 0.5f * (F32)mSize.x;
         start.y = end.y = mousePos.y + tmp.y * squareSize * 0.5f * (F32)mSize.y;
               
         hit = gServerContainer.castRay( start, end, TerrainObjectType, &ri );

         if ( hit )
            pointList.push_back( ri.point );
      }
   }   

   mTerrainEditor->drawLineList( pointList, col, 1.0f );  
}

void EllipseBrush::rebuild()
{
   PROFILE_SCOPE( TerrainEditor_EllipseBrush_Rebuild );

   reset();

   const F32 squareSize = mGridPoint.terrainBlock->getSquareSize();

   mRenderList.setSize(mSize.x*mSize.y);
   
   Point3F center( F32(mSize.x - 1) / 2.0f * squareSize, F32(mSize.y - 1) / 2.0f * squareSize, 0.0f );
   
   Filter filter;
   filter.set(1, &mTerrainEditor->mSoftSelectFilter);
   
   const Point3F mousePos = mTerrainEditor->getMousePos();   

   // a point is in a circle if:
   // x^2 + y^2 <= r^2
   // a point is in an ellipse if:
   // (ax)^2 + (by)^2 <= 1
   // where a = 1/halfEllipseWidth and b = 1/halfEllipseHeight

   // for a soft-selected ellipse,
   // the factor is simply the filtered: ((ax)^2 + (by)^2)

   F32 a = 1.0f / (F32(mSize.x) * squareSize * 0.5f);
   F32 b = 1.0f / (F32(mSize.y) * squareSize * 0.5f);
   
   const F32 softness = mTerrainEditor->getBrushSoftness();
   const F32 pressure = mTerrainEditor->getBrushPressure();

   Point3F posw( 0,0,0 );
   Point2I posg( 0,0 );
   Vector<GridInfo> infos;

   for ( S32 x = 0; x < mSize.x; x++ )
   {
      for ( S32 y = 0; y < mSize.y; y++ )
      {
         F32 xp = center.x - x * squareSize;
         F32 yp = center.y - y * squareSize;

         F32 factor = (a * a * xp * xp) + (b * b * yp * yp);
         if ( factor > 1 )
         {
            mRenderList[x*mSize.x+y] = -1;
            continue;
         }

         S32 &rl = mRenderList[x*mSize.x+y];
         
         posw.x = mousePos.x + (F32)x * squareSize - center.x;
         posw.y = mousePos.y + (F32)y * squareSize - center.y;

         // round to grid coords
         GridPoint gridPoint = mGridPoint;         
         mTerrainEditor->worldToGrid( posw, gridPoint );         

         // Check that the grid point is valid within the terrain.  This assumes
         // that there is no wrap around past the edge of the terrain.
         if ( !mTerrainEditor->isPointInTerrain( gridPoint ) )
         {
            rl = -1;
            continue;
         }
         
         infos.clear();
         mTerrainEditor->getGridInfos( gridPoint, infos );

         for ( U32 z = 0; z < infos.size(); z++ )
         {
            infos[z].mWeight = pressure * mLerp( infos[z].mWeight, filter.getValue( factor ), softness ); 
            push_back(infos[z]);
         }

         rl = size()-1;
      }
   }
}

void EllipseBrush::_renderOutline()
{
   F32 squareSize = mGridPoint.terrainBlock->getSquareSize();

   RayInfo ri;
   Point3F start( 0, 0, 5000.0f );
   Point3F end( 0, 0, -5000.0f );
   bool hit;

   Vector<Point3F> pointList;

   ColorI col( 255, 0, 255, 255 );
   
   const U32 steps = 64;

   const Point3F &mousePos = mTerrainEditor->getMousePos();
   
   for ( S32 i = 0; i < steps; i++ )
   {
      F32 radians = (F32)i / (F32)(steps-1) * M_2PI_F;
      VectorF vec(0,1,0);
      MathUtils::vectorRotateZAxis( vec, radians );

      start.x = end.x = mousePos.x + vec.x * squareSize * (F32)mSize.x * 0.5f;
      start.y = end.y = mousePos.y + vec.y * squareSize * (F32)mSize.y * 0.5f;
            
      hit = gServerContainer.castRay( start, end, TerrainObjectType, &ri );

      if ( hit )
         pointList.push_back( ri.point );
   }   

   mTerrainEditor->drawLineList( pointList, col, 1.0f );   
}

SelectionBrush::SelectionBrush(TerrainEditor * editor) :
   Brush(editor)
{
   //... grab the current selection
}

void SelectionBrush::rebuild()
{
   reset();
   //... move the selection
}

void SelectionBrush::render(Vector<GFXVertexPC> & vertexBuffer, S32 & verts, S32 & elems, S32 & prims, const ColorF & inColorFull, const ColorF & inColorNone, const ColorF & outColorFull, const ColorF & outColorNone) const
{
   //... render the selection
}

TerrainEditor::TerrainEditor() :
   mActiveTerrain(0),
   mMousePos(0,0,0),
   mMouseBrush(0),
   mInAction(false),
   mUndoSel(0),
   mGridUpdateMin( S32_MAX, S32_MAX ),
   mGridUpdateMax( 0, 0 ),
   mMaxBrushSize(48,48),
   mNeedsGridUpdate( false ),
   mNeedsMaterialUpdate( false ),
   mMouseDown( false )
{
   VECTOR_SET_ASSOCIATION(mActions);

   //
   resetCurrentSel();

   //
   mBrushPressure = 1.0f;
   mBrushSize.set(1,1);
   mBrushSoftness = 1.0f;
   mBrushChanged = true;
   mMouseBrush = new BoxBrush(this);
   mMouseDownSeq = 0;
   mIsDirty = false;
   mIsMissionDirty = false;
   mPaintIndex = -1;

   // add in all the actions here..
   mActions.push_back(new SelectAction(this));
   mActions.push_back(new DeselectAction(this));
   mActions.push_back(new ClearAction(this));
   mActions.push_back(new SoftSelectAction(this));
   mActions.push_back(new OutlineSelectAction(this));
   mActions.push_back(new PaintMaterialAction(this));
   mActions.push_back(new ClearMaterialsAction(this));
   mActions.push_back(new RaiseHeightAction(this));
   mActions.push_back(new LowerHeightAction(this));
   mActions.push_back(new SetHeightAction(this));
   mActions.push_back(new SetEmptyAction(this));
   mActions.push_back(new ClearEmptyAction(this));
   mActions.push_back(new ScaleHeightAction(this));
   mActions.push_back(new BrushAdjustHeightAction(this));
   mActions.push_back(new AdjustHeightAction(this));
   mActions.push_back(new FlattenHeightAction(this));
   mActions.push_back(new SmoothHeightAction(this));
   mActions.push_back(new SmoothSlopeAction(this));
   mActions.push_back(new PaintNoiseAction(this));
   //mActions.push_back(new ThermalErosionAction(this));


   // set the default action
   mCurrentAction = mActions[0];
   mRenderBrush = mCurrentAction->useMouseBrush();

   // persist data defaults
   mRenderBorder = true;
   mBorderHeight = 10;
   mBorderFillColor.set(0,255,0,20);
   mBorderFrameColor.set(0,255,0,128);
   mBorderLineMode = false;
   mSelectionHidden = false;
   mRenderVertexSelection = false;
   mRenderSolidBrush = false;
   mProcessUsesBrush = false;

   //
   mAdjustHeightVal = 10;
   mSetHeightVal = 100;
   mScaleVal = 1;
   mSmoothFactor = 0.1f;
   mNoiseFactor = 1.0f;
   mMaterialGroup = 0;
   mSoftSelectRadius = 50.f;
   mAdjustHeightMouseScale = 0.1f;

   mSoftSelectDefaultFilter = StringTable->insert("1.000000 0.833333 0.666667 0.500000 0.333333 0.166667 0.000000");
   mSoftSelectFilter = mSoftSelectDefaultFilter;

   mSlopeMinAngle = 0.0f;
   mSlopeMaxAngle = 90.0f;
}

TerrainEditor::~TerrainEditor()
{
   // mouse
   delete mMouseBrush;

   // terrain actions
   U32 i;
   for(i = 0; i < mActions.size(); i++)
      delete mActions[i];

   // undo stuff
   delete mUndoSel;
}

TerrainAction * TerrainEditor::lookupAction(const char * name)
{
   for(U32 i = 0; i < mActions.size(); i++)
      if(!dStricmp(mActions[i]->getName(), name))
         return(mActions[i]);
   return(0);
}

bool TerrainEditor::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   GFXStateBlockDesc desc;
   desc.setZReadWrite( false );
   desc.zWriteEnable = false;
   desc.setCullMode( GFXCullNone );
   desc.setBlend( true, GFXBlendSrcAlpha, GFXBlendDestAlpha );
   mStateBlock = GFX->createStateBlock( desc );

   return true;
}

bool TerrainEditor::onWake()
{
   if ( !Parent::onWake() )
      return false;

   // Push our default cursor on here once.
   GuiCanvas *root = getRoot();
   if ( root )
   {
      S32 currCursor = PlatformCursorController::curArrow;

      PlatformWindow *window = root->getPlatformWindow();
      PlatformCursorController *controller = window->getCursorController();
      controller->pushCursor( currCursor );
   }

   return true;
}

void TerrainEditor::onSleep()
{
   // Pop our default cursor off.
   GuiCanvas *root = getRoot();
   if ( root )
   {
      PlatformWindow *window = root->getPlatformWindow();
      PlatformCursorController *controller = window->getCursorController();
      controller->popCursor();
   }

   Parent::onSleep();
}

void TerrainEditor::get3DCursor( GuiCursor *&cursor, 
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

void TerrainEditor::onDeleteNotify(SimObject * object)
{
   Parent::onDeleteNotify(object);

   if (dynamic_cast<TerrainBlock*>(object) == mActiveTerrain)
      mActiveTerrain = NULL;
}

TerrainBlock* TerrainEditor::getClientTerrain( TerrainBlock *serverTerrain ) const
{
   if ( !serverTerrain )
      serverTerrain = mActiveTerrain;

   return serverTerrain ? dynamic_cast<TerrainBlock*>( serverTerrain->getClientObject() ) : NULL;
}

bool TerrainEditor::isMainTile(const GridPoint & gPoint) const
{
   const S32 blockSize = (S32)gPoint.terrainBlock->getBlockSize();

   Point2I testPos = gPoint.gridPos;
   if (!dStrcmp(getCurrentAction(),"paintMaterial"))
   {
      if (testPos.x == blockSize)
         testPos.x--;
      if (testPos.y == blockSize)
         testPos.y--;
   }

   return (testPos.x >= 0 && testPos.x < blockSize && testPos.y >= 0 && testPos.y < blockSize);
}

TerrainBlock* TerrainEditor::getTerrainUnderWorldPoint(const Point3F & wPos)
{
   PROFILE_SCOPE( TerrainEditor_GetTerrainUnderWorldPoint );

   // Cast a ray straight down from the world position and see which
   // Terrain is the closest to our starting point
   Point3F startPnt = wPos;
   Point3F endPnt = wPos + Point3F(0.0f, 0.0f, -1000.0f);

   S32 blockIndex = -1;
   F32 nearT = 1.0f;

   for (U32 i = 0; i < mTerrainBlocks.size(); i++)
   {
      Point3F tStartPnt, tEndPnt;

      mTerrainBlocks[i]->getWorldTransform().mulP(startPnt, &tStartPnt);
      mTerrainBlocks[i]->getWorldTransform().mulP(endPnt, &tEndPnt);

      RayInfo ri;
      if (mTerrainBlocks[i]->castRayI(tStartPnt, tEndPnt, &ri, true))
      {
         if (ri.t < nearT)
         {
            blockIndex = i;
            nearT = ri.t;
         }
      }
   }

   if (blockIndex > -1)
      return mTerrainBlocks[blockIndex];

   return NULL;
}

bool TerrainEditor::gridToWorld(const GridPoint & gPoint, Point3F & wPos)
{
   PROFILE_SCOPE( TerrainEditor_GridToWorld );

   const MatrixF & mat = gPoint.terrainBlock->getTransform();
   Point3F origin;
   mat.getColumn(3, &origin);

   wPos.x = gPoint.gridPos.x * gPoint.terrainBlock->getSquareSize() + origin.x;
   wPos.y = gPoint.gridPos.y * gPoint.terrainBlock->getSquareSize() + origin.y;
   wPos.z = getGridHeight(gPoint) + origin.z;

   return isMainTile(gPoint);
}

bool TerrainEditor::gridToWorld(const Point2I & gPos, Point3F & wPos, TerrainBlock* terrain)
{
   GridPoint gridPoint;
   gridPoint.gridPos = gPos;
   gridPoint.terrainBlock = terrain;

   return gridToWorld(gridPoint, wPos);
}

bool TerrainEditor::worldToGrid(const Point3F & wPos, GridPoint & gPoint)
{
   PROFILE_SCOPE( TerrainEditor_WorldToGrid );

   // If the grid point TerrainBlock is NULL then find the closest Terrain underneath that
   // point - pad a little upward in case our incoming point already lies exactly on the terrain
   if (!gPoint.terrainBlock)
      gPoint.terrainBlock = getTerrainUnderWorldPoint(wPos + Point3F(0.0f, 0.0f, 0.05f));

   if (gPoint.terrainBlock == NULL)
      return false;

   gPoint.gridPos = gPoint.terrainBlock->getGridPos(wPos);
   return isMainTile(gPoint);
}

bool TerrainEditor::worldToGrid(const Point3F & wPos, Point2I & gPos, TerrainBlock* terrain)
{
   GridPoint gridPoint;
   gridPoint.terrainBlock = terrain;

   bool ret = worldToGrid(wPos, gridPoint);

   gPos = gridPoint.gridPos;

   return ret;
}

bool TerrainEditor::gridToCenter(const Point2I & gPos, Point2I & cPos) const
{
   // TODO: What is this for... megaterrain or tiled terrains?
   cPos.x = gPos.x; // & TerrainBlock::BlockMask;
   cPos.y = gPos.y;// & TerrainBlock::BlockMask;

   //if (gPos.x == TerrainBlock::BlockSize)
   //   cPos.x = gPos.x;
   //if (gPos.y == TerrainBlock::BlockSize)
   //   cPos.y = gPos.y;

   //return isMainTile(gPos);
   return true;
}

//------------------------------------------------------------------------------

//bool TerrainEditor::getGridInfo(const Point3F & wPos, GridInfo & info)
//{
//   Point2I gPos;
//   worldToGrid(wPos, gPos);
//   return getGridInfo(gPos, info);
//}

bool TerrainEditor::getGridInfo(const GridPoint & gPoint, GridInfo & info)
{
   //
   info.mGridPoint = gPoint;
   info.mMaterial = getGridMaterial(gPoint);
   info.mHeight = getGridHeight(gPoint);
   info.mWeight = 1.f;
   info.mPrimarySelect = true;
   info.mMaterialChanged = false;

   Point2I cPos;
   gridToCenter(gPoint.gridPos, cPos);

   return isMainTile(gPoint);
}

bool TerrainEditor::getGridInfo(const Point2I & gPos, GridInfo & info, TerrainBlock* terrain)
{
   GridPoint gridPoint;
   gridPoint.gridPos = gPos;
   gridPoint.terrainBlock = terrain;

   return getGridInfo(gridPoint, info);
}

void TerrainEditor::getGridInfos(const GridPoint & gPoint, Vector<GridInfo>& infos)
{
   PROFILE_SCOPE( TerrainEditor_GetGridInfos );

   // First we test against the brush terrain so that we can
   // favor it (this should be the same as the active terrain)
   bool foundBrush = false;

   GridInfo baseInfo;
   if (getGridInfo(gPoint, baseInfo))
   {
      infos.push_back(baseInfo);

      foundBrush = true;
   }

   // We are going to need the world position to test against
   Point3F wPos;
   gridToWorld(gPoint, wPos);

   // Now loop through our terrain blocks and decide which ones hit the point
   // If we already found a hit against our brush terrain we only add points
   // that are relatively close to the found point
   for (U32 i = 0; i < mTerrainBlocks.size(); i++)
   {
      // Skip if we've already found the point on the brush terrain
      if (foundBrush && mTerrainBlocks[i] == baseInfo.mGridPoint.terrainBlock)
         continue;

      // Get our grid position
      Point2I gPos;
      worldToGrid(wPos, gPos, mTerrainBlocks[i]);

      GridInfo info;
      if (getGridInfo(gPos, info, mTerrainBlocks[i]))
      {
         // Skip adding this if we already found a GridInfo from the brush terrain
         // and the resultant world point isn't equivalent
         if (foundBrush)
         {
            // Convert back to world (since the height can be different)
            // Possibly use getHeight() here?
            Point3F testWorldPt;
            gridToWorld(gPos, testWorldPt, mTerrainBlocks[i]);

            if (mFabs( wPos.z - testWorldPt.z ) > 4.0f )
               continue;
         }

         infos.push_back(info);
      }
   }
}

void TerrainEditor::setGridInfo(const GridInfo & info, bool checkActive)
{
   PROFILE_SCOPE( TerrainEditor_SetGridInfo );

   setGridHeight(info.mGridPoint, info.mHeight);
   setGridMaterial(info.mGridPoint, info.mMaterial);
}

F32 TerrainEditor::getGridHeight(const GridPoint & gPoint)
{
   PROFILE_SCOPE( TerrainEditor_GetGridHeight );

   Point2I cPos;
   gridToCenter( gPoint.gridPos, cPos );
   const TerrainFile *file = gPoint.terrainBlock->getFile();
   return fixedToFloat( file->getHeight( cPos.x, cPos.y ) );
}

void TerrainEditor::gridUpdateComplete( bool materialChanged )
{
   PROFILE_SCOPE( TerrainEditor_GridUpdateComplete );

   // TODO: This updates all terrains and not just the ones
   // that were changed.  We should keep track of the mGridUpdate
   // in world space and transform it into terrain space.

   if(mGridUpdateMin.x <= mGridUpdateMax.x)
   {
      for (U32 i = 0; i < mTerrainBlocks.size(); i++)
      {
         TerrainBlock *clientTerrain = getClientTerrain( mTerrainBlocks[i] );
         if ( materialChanged )
            clientTerrain->updateGridMaterials(mGridUpdateMin, mGridUpdateMax);

         mTerrainBlocks[i]->updateGrid(mGridUpdateMin, mGridUpdateMax);
         clientTerrain->updateGrid(mGridUpdateMin, mGridUpdateMax);
      }
   }

   mGridUpdateMin.set( S32_MAX, S32_MAX );
   mGridUpdateMax.set( 0, 0 );
   mNeedsGridUpdate = false;
}

void TerrainEditor::materialUpdateComplete()
{
   PROFILE_SCOPE( TerrainEditor_MaterialUpdateComplete );

   if(mActiveTerrain && (mGridUpdateMin.x <= mGridUpdateMax.x))
   {
      TerrainBlock * clientTerrain = getClientTerrain(mActiveTerrain);
      clientTerrain->updateGridMaterials(mGridUpdateMin, mGridUpdateMax);
   }
   mGridUpdateMin.set( S32_MAX, S32_MAX );
   mGridUpdateMax.set( 0, 0 );
   mNeedsMaterialUpdate = false;
}

void TerrainEditor::setGridHeight(const GridPoint & gPoint, const F32 height)
{
   PROFILE_SCOPE( TerrainEditor_SetGridHeight );

   Point2I cPos;
   gridToCenter(gPoint.gridPos, cPos);

   mGridUpdateMin.setMin( cPos );
   mGridUpdateMax.setMax( cPos );

   gPoint.terrainBlock->setHeight(cPos, height);
}

U8 TerrainEditor::getGridMaterial( const GridPoint &gPoint ) const
{
   PROFILE_SCOPE( TerrainEditor_GetGridMaterial );

   Point2I cPos;
   gridToCenter( gPoint.gridPos, cPos );
   const TerrainFile *file = gPoint.terrainBlock->getFile();
   return file->getLayerIndex( cPos.x, cPos.y );
}

void TerrainEditor::setGridMaterial( const GridPoint &gPoint, U8 index )
{
   PROFILE_SCOPE( TerrainEditor_SetGridMaterial );

   Point2I cPos;
   gridToCenter( gPoint.gridPos, cPos );
   TerrainFile *file = gPoint.terrainBlock->getFile();

   // If we changed the empty state then we need
   // to do a grid update as well.
   U8 currIndex = file->getLayerIndex( cPos.x, cPos.y );
   if (  ( currIndex == (U8)-1 && index != (U8)-1 ) || 
         ( currIndex != (U8)-1 && index == (U8)-1 ) )
   {
      mGridUpdateMin.setMin( cPos );
      mGridUpdateMax.setMax( cPos );
      mNeedsGridUpdate = true;
   }

   file->setLayerIndex( cPos.x, cPos.y, index );
}

//------------------------------------------------------------------------------

TerrainBlock* TerrainEditor::collide(const Gui3DMouseEvent & evt, Point3F & pos)
{
   PROFILE_SCOPE( TerrainEditor_Collide );

   if (mTerrainBlocks.size() == 0)
      return NULL;

   if ( mMouseDown && !dStrcmp(getCurrentAction(),"paintMaterial") )
   {
      if ( !mActiveTerrain )
         return NULL;

      Point3F tpos, tvec;

      tpos = evt.pos;
      tvec = evt.vec;

      mMousePlane.intersect( evt.pos, evt.vec, &pos ); 

      return mActiveTerrain;
   }

   const U32 mask = TerrainObjectType;

   Point3F start( evt.pos );
   Point3F end( evt.pos + ( evt.vec * 10000.0f ) );

   RayInfo rinfo;
   bool hit = gServerContainer.castRay( start, end, mask, &rinfo );

   if ( !hit )
      return NULL;

   pos = rinfo.point;

   return (TerrainBlock*)(rinfo.object);

   //
   //// call the terrain block's ray collision routine directly
   //Point3F startPnt = event.pos;
   //Point3F endPnt = event.pos + event.vec * 1000.0f;

   //S32 blockIndex = -1;
   //F32 nearT = 1.0f;

   //for (U32 i = 0; i < mTerrainBlocks.size(); i++)
   //{
   //   Point3F tStartPnt, tEndPnt;

   //   mTerrainBlocks[i]->getWorldTransform().mulP(startPnt, &tStartPnt);
   //   mTerrainBlocks[i]->getWorldTransform().mulP(endPnt, &tEndPnt);

   //   RayInfo ri;
   //   if (mTerrainBlocks[i]->castRayI(tStartPnt, tEndPnt, &ri, true))
   //   {
   //      if (ri.t < nearT)
   //      {
   //         blockIndex = i;
   //         nearT = ri.t;
   //      }
   //   }
   //}

   //if (blockIndex > -1)
   //{
   //   pos.interpolate(startPnt, endPnt, nearT);

   //   return mTerrainBlocks[blockIndex];
   //}

   //return NULL;
}

//------------------------------------------------------------------------------

void TerrainEditor::updateGuiInfo()
{
   PROFILE_SCOPE( TerrainEditor_UpdateGuiInfo );

   char buf[128];

   // mouse num grids
   // mouse avg height
   // selection num grids
   // selection avg height
   dSprintf(buf, sizeof(buf), "%d %g %g %g %d %g",
      mMouseBrush->size(), mMouseBrush->getMinHeight(),
      mMouseBrush->getAvgHeight(), mMouseBrush->getMaxHeight(),
      mDefaultSel.size(), mDefaultSel.getAvgHeight());
   Con::executef(this, "onGuiUpdate", buf);

   // If the brush setup has changed send out
   // a notification of that!
   if ( mBrushChanged && isMethod( "onBrushChanged" ) )
   {
      mBrushChanged = false;
      Con::executef( this, "onBrushChanged" );
   }
}

//------------------------------------------------------------------------------

void TerrainEditor::onPreRender()
{
   PROFILE_SCOPE( TerrainEditor_OnPreRender );

   if ( mNeedsGridUpdate )
      gridUpdateComplete( mNeedsMaterialUpdate );
   else if ( mNeedsMaterialUpdate )
      materialUpdateComplete();

   Parent::onPreRender();
}

void TerrainEditor::renderScene(const RectI &)
{
   PROFILE_SCOPE( TerrainEditor_RenderScene );      

   if(mTerrainBlocks.size() == 0)
      return;

   if(!mSelectionHidden)
      renderSelection(mDefaultSel, ColorF::RED, ColorF::GREEN, ColorF::BLUE, ColorF::BLUE, true, false);

   if(mRenderBrush && mMouseBrush->size())
      renderBrush(*mMouseBrush, ColorF::GREEN, ColorF::RED, ColorF::BLUE, ColorF::BLUE, false, true);

   if(mRenderBorder)
      renderBorder();
}

//------------------------------------------------------------------------------

void TerrainEditor::renderGui( Point2I offset, const RectI &updateRect )
{   
   PROFILE_SCOPE( TerrainEditor_RenderGui );

   if ( !mActiveTerrain )
      return;

   // Just in case...
   if ( mMouseBrush->getGridPoint().terrainBlock != mActiveTerrain )
      mMouseBrush->setTerrain( mActiveTerrain );

   mMouseBrush->render();
}

void TerrainEditor::renderPoints( const Vector<GFXVertexPCT> &pointList )
{
   PROFILE_SCOPE( TerrainEditor_RenderPoints );

   const U32 pointCount = pointList.size();
   const U32 vertCount = pointCount * 6;

   GFXStateBlockDesc desc;
   desc.setBlend( true );
   desc.setZReadWrite( false, false );
   GFX->setupGenericShaders();   
   GFX->setStateBlockByDesc( desc );

   U32 vertsLeft = vertCount;
   U32 offset = 0;

   while ( vertsLeft > 0 )
   {
      U32 vertsThisDrawCall = getMin( (U32)vertsLeft, (U32)MAX_DYNAMIC_VERTS );
      vertsLeft -= vertsThisDrawCall;

      GFXVertexBufferHandle<GFXVertexPC> vbuff( GFX, vertsThisDrawCall, GFXBufferTypeVolatile );
      GFXVertexPC *vert = vbuff.lock();

      const U32 loops = vertsThisDrawCall / 6;

      for ( S32 i = 0; i < loops; i++ )
      {
         const GFXVertexPCT &pointInfo = pointList[i + offset];                  
               
         vert[0].color = vert[1].color = vert[2].color = vert[3].color = vert[4].color = vert[5].color = pointInfo.color;
         
         
         const F32 halfSize = pointInfo.texCoord.x * 0.5f;
         const Point3F &pos = pointInfo.point;

         Point3F p0( pos.x - halfSize, pos.y - halfSize, 0.0f );
         Point3F p1( pos.x + halfSize, pos.y - halfSize, 0.0f );
         Point3F p2( pos.x + halfSize, pos.y + halfSize, 0.0f );
         Point3F p3( pos.x - halfSize, pos.y + halfSize, 0.0f );

         vert[0].point = p0;
         vert[1].point = p1;
         vert[2].point = p2;
         
         vert[3].point = p0;
         vert[4].point = p2;
         vert[5].point = p3;

         vert += 6;
      }

      vbuff.unlock();

      GFX->setVertexBuffer( vbuff );

      GFX->drawPrimitive( GFXTriangleList, 0, vertsThisDrawCall / 3 );

      offset += loops;
   }
}


//------------------------------------------------------------------------------

void TerrainEditor::renderSelection( const Selection & sel, const ColorF & inColorFull, const ColorF & inColorNone, const ColorF & outColorFull, const ColorF & outColorNone, bool renderFill, bool renderFrame )
{
   PROFILE_SCOPE( TerrainEditor_RenderSelection );

   // Draw nothing if nothing selected.
   if(sel.size() == 0)
      return;

   Vector<GFXVertexPC> vertexBuffer;
   ColorF color;
   ColorI iColor;

   vertexBuffer.setSize(sel.size() * 5);

   F32 squareSize = ( mActiveTerrain ) ? mActiveTerrain->getSquareSize() : 1;

   // 'RenderVertexSelection' looks really bad so just always use the good one.
   if( false && mRenderVertexSelection)
   {

      for(U32 i = 0; i < sel.size(); i++)
      {
         Point3F wPos;
         bool center = gridToWorld(sel[i].mGridPoint, wPos);

         F32 weight = sel[i].mWeight;

         if(center)
         {
            if ( weight < 0.f || weight > 1.f )
               color = inColorFull;
            else
               color.interpolate( inColorNone, inColorFull, weight );
         }
         else
         {
            if ( weight < 0.f || weight > 1.f)
               color = outColorFull;
            else
               color.interpolate( outColorFull, outColorNone, weight );
         }
         //
         iColor = color;

         GFXVertexPC *verts = &(vertexBuffer[i * 5]);

         verts[0].point = wPos + Point3F(-squareSize, -squareSize, 0);
         verts[0].color = iColor;
         verts[1].point = wPos + Point3F( squareSize, -squareSize, 0);
         verts[1].color = iColor;
         verts[2].point = wPos + Point3F( squareSize,  squareSize, 0);
         verts[2].color = iColor;
         verts[3].point = wPos + Point3F(-squareSize,  squareSize, 0);
         verts[3].color = iColor;
         verts[4].point = verts[0].point;
         verts[4].color = iColor;
      }
   }
   else
   {
      // walk the points in the selection
      for(U32 i = 0; i < sel.size(); i++)
      {
         Point2I gPos = sel[i].mGridPoint.gridPos;

         GFXVertexPC *verts = &(vertexBuffer[i * 5]);

         bool center = gridToWorld(sel[i].mGridPoint, verts[0].point);
         gridToWorld(Point2I(gPos.x + 1, gPos.y), verts[1].point, sel[i].mGridPoint.terrainBlock);
         gridToWorld(Point2I(gPos.x + 1, gPos.y + 1), verts[2].point, sel[i].mGridPoint.terrainBlock);
         gridToWorld(Point2I(gPos.x, gPos.y + 1), verts[3].point, sel[i].mGridPoint.terrainBlock);
         verts[4].point = verts[0].point;

         F32 weight = sel[i].mWeight;

         if( !mRenderSolidBrush )
         {
            if ( center )
            {
               if ( weight < 0.f || weight > 1.f )
                  color = inColorFull;
               else
                  color.interpolate(inColorNone, inColorFull, weight );
            }
            else
            {
               if( weight < 0.f || weight > 1.f )
                  color = outColorFull;
               else
                  color.interpolate(outColorFull, outColorNone, weight );
            }

            iColor = color;
         }
         else
         {
            if ( center )
            {
               iColor = inColorNone;
            }
            else
            {
               iColor = outColorFull;
            }
         }

         verts[0].color = iColor;
         verts[1].color = iColor;
         verts[2].color = iColor;
         verts[3].color = iColor;
         verts[4].color = iColor;
      }
   }

   // Render this bad boy, by stuffing everything into a volatile buffer
   // and rendering...
   GFXVertexBufferHandle<GFXVertexPC> selectionVB(GFX, vertexBuffer.size(), GFXBufferTypeStatic);

   selectionVB.lock(0, vertexBuffer.size());

   // Copy stuff
   dMemcpy((void*)&selectionVB[0], (void*)&vertexBuffer[0], sizeof(GFXVertexPC) * vertexBuffer.size());

   selectionVB.unlock();

   GFX->setupGenericShaders();
   GFX->setStateBlock( mStateBlock );
   GFX->setVertexBuffer(selectionVB);

   if(renderFill)
      for(U32 i=0; i < sel.size(); i++)
         GFX->drawPrimitive( GFXTriangleFan, i*5, 4);

   if(renderFrame)
      for(U32 i=0; i < sel.size(); i++)
         GFX->drawPrimitive( GFXLineStrip , i*5, 4);
}

void TerrainEditor::renderBrush( const Brush & brush, const ColorF & inColorFull, const ColorF & inColorNone, const ColorF & outColorFull, const ColorF & outColorNone, bool renderFill, bool renderFrame )
{  
}

void TerrainEditor::renderBorder()
{
   // TODO: Disabled rendering the terrain borders... it was
   // very annoying getting a fullscreen green tint on things.
   //
   // We should consider killing this all together or coming
   // up with a new technique.
   /*
   Point2I pos(0,0);
   Point2I dir[4] = {
      Point2I(1,0),
      Point2I(0,1),
      Point2I(-1,0),
      Point2I(0,-1)
   };

   GFX->setStateBlock( mStateBlock );
   
   //
   if(mBorderLineMode)
   {
      PrimBuild::color(mBorderFrameColor);
      
      PrimBuild::begin( GFXLineStrip, TerrainBlock::BlockSize * 4 + 1 );
      for(U32 i = 0; i < 4; i++)
      {
         for(U32 j = 0; j < TerrainBlock::BlockSize; j++)
         {
            Point3F wPos;
            gridToWorld(pos, wPos, mActiveTerrain);
            PrimBuild::vertex3fv( wPos );
            pos += dir[i];
         }
      }

      Point3F wPos;
      gridToWorld(Point2I(0,0), wPos, mActiveTerrain);
      PrimBuild::vertex3fv( wPos );
      PrimBuild::end();
   }
   else
   {
      GridSquare * gs = mActiveTerrain->findSquare(TerrainBlock::BlockShift, Point2I(0,0));
      F32 height = F32(gs->maxHeight) * 0.03125f + mBorderHeight;

      const MatrixF & mat = mActiveTerrain->getTransform();
      Point3F pos;
      mat.getColumn(3, &pos);

      Point2F min(pos.x, pos.y);
      Point2F max(pos.x + TerrainBlock::BlockSize * mActiveTerrain->getSquareSize(),
                  pos.y + TerrainBlock::BlockSize * mActiveTerrain->getSquareSize());

      ColorI & a = mBorderFillColor;
      ColorI & b = mBorderFrameColor;

      for(U32 i = 0; i < 2; i++)
      {
         //
         if(i){ PrimBuild::color(a); PrimBuild::begin( GFXTriangleFan, 4 ); } else { PrimBuild::color(b); PrimBuild::begin( GFXLineStrip, 5 ); }

         PrimBuild::vertex3f(min.x, min.y, 0);
         PrimBuild::vertex3f(max.x, min.y, 0);
         PrimBuild::vertex3f(max.x, min.y, height);
         PrimBuild::vertex3f(min.x, min.y, height);
         if(!i) PrimBuild::vertex3f( min.x, min.y, 0.f );
         PrimBuild::end();

         //
         if(i){ PrimBuild::color(a); PrimBuild::begin( GFXTriangleFan, 4 ); } else { PrimBuild::color(b); PrimBuild::begin( GFXLineStrip, 5 ); }
         PrimBuild::vertex3f(min.x, max.y, 0);
         PrimBuild::vertex3f(max.x, max.y, 0);
         PrimBuild::vertex3f(max.x, max.y, height);
         PrimBuild::vertex3f(min.x, max.y, height);
         if(!i) PrimBuild::vertex3f( min.x, min.y, 0.f );
         PrimBuild::end();

         //
         if(i){ PrimBuild::color(a); PrimBuild::begin( GFXTriangleFan, 4 ); } else { PrimBuild::color(b); PrimBuild::begin( GFXLineStrip, 5 ); }
         PrimBuild::vertex3f(min.x, min.y, 0);
         PrimBuild::vertex3f(min.x, max.y, 0);
         PrimBuild::vertex3f(min.x, max.y, height);
         PrimBuild::vertex3f(min.x, min.y, height);
         if(!i) PrimBuild::vertex3f( min.x, min.y, 0.f );
         PrimBuild::end();

         //
         if(i){ PrimBuild::color(a); PrimBuild::begin( GFXTriangleFan, 4 ); } else { PrimBuild::color(b); PrimBuild::begin( GFXLineStrip, 5 ); }
         PrimBuild::vertex3f(max.x, min.y, 0);
         PrimBuild::vertex3f(max.x, max.y, 0);
         PrimBuild::vertex3f(max.x, max.y, height);
         PrimBuild::vertex3f(max.x, min.y, height);
         if(!i) PrimBuild::vertex3f( min.x, min.y, 0.f );
         PrimBuild::end();
      }
   }
   */
}

void TerrainEditor::submitUndo( Selection *sel )
{
   // Grab the mission editor undo manager.
   UndoManager *undoMan = NULL;
   if ( !Sim::findObject( "EUndoManager", undoMan ) )
   {
      Con::errorf( "TerrainEditor::submitUndo() - EUndoManager not found!" );
      return;     
   }

   // Create and submit the action.
   TerrainEditorUndoAction *action = new TerrainEditorUndoAction( "Terrain Editor Action" );
   action->mSel = sel;
   action->mTerrainEditor = this;
   undoMan->addAction( action );
   
   // Mark the editor as dirty!
   setDirty();
}

void TerrainEditor::TerrainEditorUndoAction::undo()
{
   // NOTE: This function also handles TerrainEditorUndoAction::redo().

   bool materialChanged = false;

   for (U32 i = 0; i < mSel->size(); i++)
   {
      // Grab the current grid info for this point.
      GridInfo info;
      mTerrainEditor->getGridInfo( (*mSel)[i].mGridPoint, info );
      info.mMaterialChanged = (*mSel)[i].mMaterialChanged;

      materialChanged |= info.mMaterialChanged;

      // Restore the previous grid info.      
      mTerrainEditor->setGridInfo( (*mSel)[i] );

      // Save the old grid info so we can 
      // restore it later.
      (*mSel)[i] = info;
   }

   // Mark the editor as dirty!
   mTerrainEditor->setDirty();
   mTerrainEditor->gridUpdateComplete( materialChanged );
   mTerrainEditor->mMouseBrush->update();
}

void TerrainEditor::submitMaterialUndo( String actionName )
{
   // Grab the mission editor undo manager.
   UndoManager *undoMan = NULL;
   if ( !Sim::findObject( "EUndoManager", undoMan ) )
   {
      Con::errorf( "TerrainEditor::submitMaterialUndo() - EUndoManager not found!" );
      return;     
   }

   TerrainBlock *terr = getClientTerrain();
   
   // Create and submit the action.
   TerrainMaterialUndoAction *action = new TerrainMaterialUndoAction( actionName );
   action->mTerrain = terr;
   action->mMaterials = terr->getMaterials();
   action->mLayerMap = terr->getLayerMap();
   action->mEditor = this;

   undoMan->addAction( action );
   
   // Mark the editor as dirty!
   setDirty();
}

void TerrainEditor::onMaterialUndo( TerrainBlock *terr )
{
   setDirty();
   scheduleMaterialUpdate();
   setGridUpdateMinMax();

   terr->mDetailsDirty = true;
   terr->mLayerTexDirty = true; 

   Con::executef( this, "onMaterialUndo" );
}

void TerrainEditor::TerrainMaterialUndoAction::undo()
{
   Vector<TerrainMaterial*> tempMaterials = mTerrain->getMaterials();
   Vector<U8> tempLayers = mTerrain->getLayerMap();

   mTerrain->setMaterials(mMaterials);
   mTerrain->setLayerMap(mLayerMap);

   mMaterials = tempMaterials;
   mLayerMap = tempLayers;
      
   mEditor->onMaterialUndo( mTerrain );      
}

void TerrainEditor::TerrainMaterialUndoAction::redo()
{
   undo();
}

class TerrainProcessActionEvent : public SimEvent
{
   U32 mSequence;
public:
   TerrainProcessActionEvent(U32 seq)
   {
      mSequence = seq;
   }
   void process(SimObject *object)
   {
      ((TerrainEditor *) object)->processActionTick(mSequence);
   }
};

void TerrainEditor::processActionTick(U32 sequence)
{
   if(mMouseDownSeq == sequence)
   {
      Sim::postEvent(this, new TerrainProcessActionEvent(mMouseDownSeq), Sim::getCurrentTime() + 30);
      mCurrentAction->process(mMouseBrush, mLastEvent, false, TerrainAction::Update);
   }
}

bool TerrainEditor::onInputEvent(const InputEventInfo & event)
{
   /*
   if (  mRightMousePassThru && 
         event.deviceType == KeyboardDeviceType &&
         event.objType == SI_KEY &&
         event.objInst == KEY_TAB && 
         event.action == SI_MAKE )
   {
      if ( isMethod( "onToggleToolWindows" ) )
         Con::executef( this, "onToggleToolWindows" );
   }
   */
   
   return Parent::onInputEvent( event );
}

void TerrainEditor::on3DMouseDown(const Gui3DMouseEvent & event)
{   
   getRoot()->showCursor( false );

   if(mTerrainBlocks.size() == 0)
      return;

   if (!dStrcmp(getCurrentAction(),"paintMaterial"))
   {
      Point3F pos;
      TerrainBlock* hitTerrain = collide(event, pos);

      if(!hitTerrain)
         return;

      // Set the active terrain
      bool changed = mActiveTerrain != hitTerrain;
      mActiveTerrain = hitTerrain;

      if (changed)
      {
         Con::executef(this, "onActiveTerrainChange", Con::getIntArg(hitTerrain->getId()));
         mMouseBrush->setTerrain(mActiveTerrain);
         //if(mRenderBrush)
            //mCursorVisible = false;
         mMousePos = pos;

         mMouseBrush->setPosition(mMousePos);         

         return;
      }
   }
   else if ((event.modifier & SI_ALT) && !dStrcmp(getCurrentAction(),"setHeight"))
   {
      // Set value to terrain height at mouse position
      GridInfo info;
      getGridInfo(mMouseBrush->getGridPoint(), info);
      mSetHeightVal = info.mHeight;
      mBrushChanged = true;
      return;
   }

   mMousePlane.set( mMousePos, Point3F(0,0,1) );
   mMouseDown = true;

   mSelectionLocked = false;

   mouseLock();
   mMouseDownSeq++;
   mUndoSel = new Selection;
   mCurrentAction->process(mMouseBrush, event, true, TerrainAction::Begin);
   // process on ticks - every 30th of a second.
   Sim::postEvent(this, new TerrainProcessActionEvent(mMouseDownSeq), Sim::getCurrentTime() + 30);
}

void TerrainEditor::on3DMouseMove(const Gui3DMouseEvent & event)
{
   PROFILE_SCOPE( TerrainEditor_On3DMouseMove );

   if(mTerrainBlocks.size() == 0)
      return;

   Point3F pos;
   TerrainBlock* hitTerrain = collide(event, pos);

   if(!hitTerrain)
   {
      mMouseBrush->reset();
   }
   else
   {
      // We do not change the active terrain as the mouse moves when
      // in painting mode.  This is because it causes the material 
      // window to change as you cursor over to it.
      if ( dStrcmp(getCurrentAction(),"paintMaterial") != 0 )
      {
         // Set the active terrain
         bool changed = mActiveTerrain != hitTerrain;
         mActiveTerrain = hitTerrain;

         if (changed)
            Con::executef(this, "onActiveTerrainChange", Con::getIntArg(hitTerrain->getId()));
      }

      mMousePos = pos;

      mMouseBrush->setTerrain(mActiveTerrain);
      mMouseBrush->setPosition(mMousePos);
   }  
}

void TerrainEditor::on3DMouseDragged(const Gui3DMouseEvent & event)
{
   PROFILE_SCOPE( TerrainEditor_On3DMouseDragged );

   if ( mTerrainBlocks.empty() )
      return;

   if ( !isMouseLocked() )
      return;
    
   Point3F pos;

   if ( !mSelectionLocked )
   {
      if ( !collide( event, pos)  )
         mMouseBrush->reset();
   }
   
   // check if the mouse has actually moved in grid space
   bool selChanged = false;
   if ( !mSelectionLocked )
   {
      Point2I gMouse;
      Point2I gLastMouse;
      worldToGrid( pos, gMouse );
      worldToGrid( mMousePos, gLastMouse );

      mMousePos = pos;
      mMouseBrush->setPosition( mMousePos );

      selChanged = gMouse != gLastMouse;
   }

   mCurrentAction->process( mMouseBrush, event, true, TerrainAction::Update );
}

void TerrainEditor::on3DMouseUp(const Gui3DMouseEvent & event)
{
   mMouseDown = false;
   getRoot()->showCursor( true );

   if ( mTerrainBlocks.size() == 0 )
      return;   

   if ( isMouseLocked() )
   {
      mouseUnlock();
      mMouseDownSeq++;
      mCurrentAction->process( mMouseBrush, event, false, TerrainAction::End );

      if ( mUndoSel->size() )
         submitUndo( mUndoSel );
      else
         delete mUndoSel;

      mUndoSel = 0;
      mInAction = false;
   }
}

bool TerrainEditor::onMouseWheelDown( const GuiEvent & event )
{
   if ( event.modifier & SI_PRIMARY_CTRL && event.modifier & SI_SHIFT )
   {
      setBrushPressure( mBrushPressure - 0.1f );
      return true;
   }
   else if ( event.modifier & SI_SHIFT )
   {
      setBrushSoftness( mBrushSoftness + 0.05f );
      return true;
   }
   else if ( event.modifier & SI_PRIMARY_CTRL )
   {
      Point2I newBrush = getBrushSize() - Point2I(1,1);  
      setBrushSize( newBrush.x, newBrush.y );
      return true;
   }
      
   return Parent::onMouseWheelDown( event );
}

bool TerrainEditor::onMouseWheelUp( const GuiEvent & event )
{
   if ( event.modifier & SI_PRIMARY_CTRL && event.modifier & SI_SHIFT )
   {
      setBrushPressure( mBrushPressure + 0.1f );
      return true;
   }
   else if ( event.modifier & SI_SHIFT )
   {
      setBrushSoftness( mBrushSoftness - 0.05f );
      return true;
   }
   else if( event.modifier & SI_PRIMARY_CTRL )
   {
      Point2I newBrush = getBrushSize() + Point2I(1,1);
      setBrushSize( newBrush.x, newBrush.y );
      return true;
   }
   
   return Parent::onMouseWheelUp( event );
}

//------------------------------------------------------------------------------
// any console function which depends on a terrainBlock attached to the editor
// should call this
bool checkTerrainBlock(TerrainEditor * object, const char * funcName)
{
   if(!object->terrainBlockValid())
   {
      Con::errorf(ConsoleLogEntry::Script, "TerrainEditor::%s: not attached to a terrain block!", funcName);
      return(false);
   }
   return(true);
}

void TerrainEditor::attachTerrain(TerrainBlock *terrBlock)
{
   mActiveTerrain = terrBlock;
   mTerrainBlocks.push_back_unique(terrBlock);
}

void TerrainEditor::detachTerrain(TerrainBlock *terrBlock)
{
   if (mActiveTerrain == terrBlock)
      mActiveTerrain = NULL; //do we want to set this to an existing terrain?

   if (mMouseBrush->getGridPoint().terrainBlock == terrBlock)
      mMouseBrush->setTerrain(NULL);

   // reset the brush as its gridinfos may still have references to the old terrain
   mMouseBrush->reset();

   mTerrainBlocks.remove(terrBlock);
}

TerrainBlock* TerrainEditor::getTerrainBlock(S32 index)
{
   if(index < 0 || index >= mTerrainBlocks.size())
      return NULL;

   return mTerrainBlocks[index];
}

void TerrainEditor::getTerrainBlocksMaterialList(Vector<StringTableEntry>& list)
{
   for(S32 i=0; i<mTerrainBlocks.size(); ++i)
   {
      TerrainBlock* tb = mTerrainBlocks[i];
      if(!tb)
         continue;

      for(S32 m=0; m<tb->getMaterialCount(); ++m)
      {
         TerrainMaterial* mat = tb->getMaterial(m);
         if (mat)
            list.push_back_unique(mat->getInternalName());
      }
   }
}

void TerrainEditor::setBrushType( const char *type )
{
   if ( mMouseBrush && dStrcmp( mMouseBrush->getType(), type ) == 0 )
      return;

   if(!dStricmp(type, "box"))
   {
      delete mMouseBrush;
      mMouseBrush = new BoxBrush(this);
      mBrushChanged = true;
   }
   else if(!dStricmp(type, "ellipse"))
   {
      delete mMouseBrush;
      mMouseBrush = new EllipseBrush(this);
      mBrushChanged = true;
   }
   else if(!dStricmp(type, "selection"))
   {
      delete mMouseBrush;
      mMouseBrush = new SelectionBrush(this);
      mBrushChanged = true;
   }
   else {}   
}

const char* TerrainEditor::getBrushType() const
{
   if ( mMouseBrush )
      return mMouseBrush->getType();

   return "";
}

void TerrainEditor::setBrushSize( S32 w, S32 h )
{
   w = mClamp( w, 1, mMaxBrushSize.x );
   h = mClamp( h, 1, mMaxBrushSize.y );

   if ( w == mBrushSize.x && h == mBrushSize.y )
      return;

	mBrushSize.set( w, h );
   mBrushChanged = true;

   if ( mMouseBrush )
   {
   	mMouseBrush->setSize( mBrushSize );

      if ( mMouseBrush->getGridPoint().terrainBlock )
         mMouseBrush->rebuild();
   }
}

void TerrainEditor::setBrushPressure( F32 pressure )
{
   pressure = mClampF( pressure, 0.01f, 1.0f );
   
   if ( mBrushPressure == pressure )
      return;

   mBrushPressure = pressure;
   mBrushChanged = true;

   if ( mMouseBrush && mMouseBrush->getGridPoint().terrainBlock )
      mMouseBrush->rebuild();
}

void TerrainEditor::setBrushSoftness( F32 softness )
{
   softness = mClampF( softness, 0.01f, 1.0f );

   if ( mBrushSoftness == softness )
      return;

   mBrushSoftness = softness;
   mBrushChanged = true;

   if ( mMouseBrush && mMouseBrush->getGridPoint().terrainBlock )
      mMouseBrush->rebuild();
}

const char* TerrainEditor::getBrushPos()
{
   AssertFatal(mMouseBrush!=NULL, "TerrainEditor::getBrushPos: no mouse brush!");

   Point2I pos = mMouseBrush->getPosition();
   char * ret = Con::getReturnBuffer(32);
   dSprintf(ret, 32, "%d %d", pos.x, pos.y);
   return(ret);
}

void TerrainEditor::setBrushPos(Point2I pos)
{
   AssertFatal(mMouseBrush!=NULL, "TerrainEditor::setBrushPos: no mouse brush!");
   mMouseBrush->setPosition(pos);
}

void TerrainEditor::setAction(const char* action)
{
   for(U32 i = 0; i < mActions.size(); i++)
   {
      if(!dStricmp(mActions[i]->getName(), action))
      {
         mCurrentAction = mActions[i];

         //
         mRenderBrush = mCurrentAction->useMouseBrush();
         return;
      }
   }
}

const char* TerrainEditor::getActionName(U32 index)
{
   if(index >= mActions.size())
      return("");
   return(mActions[index]->getName());
}

const char* TerrainEditor::getCurrentAction() const
{
   return(mCurrentAction->getName());
}

S32 TerrainEditor::getNumActions()
{
   return(mActions.size());
}

void TerrainEditor::resetSelWeights(bool clear)
{
   //
   if(!clear)
   {
      for(U32 i = 0; i < mDefaultSel.size(); i++)
      {
         mDefaultSel[i].mPrimarySelect = false;
         mDefaultSel[i].mWeight = 1.f;
      }
      return;
   }

   Selection sel;

   U32 i;
   for(i = 0; i < mDefaultSel.size(); i++)
   {
      if(mDefaultSel[i].mPrimarySelect)
      {
         mDefaultSel[i].mWeight = 1.f;
         sel.add(mDefaultSel[i]);
      }
   }

   mDefaultSel.reset();

   for(i = 0; i < sel.size(); i++)
      mDefaultSel.add(sel[i]);
}

void TerrainEditor::clearSelection()
{
	mDefaultSel.reset();
}

void TerrainEditor::processAction(const char* sAction)
{
   if(!checkTerrainBlock(this, "processAction"))
      return;

   TerrainAction * action = mCurrentAction;
   if (dStrcmp(sAction, "") != 0)
   {
      action = lookupAction(sAction);

      if(!action)
      {
         Con::errorf(ConsoleLogEntry::General, "TerrainEditor::cProcessAction: invalid action name '%s'.", sAction);
         return;
      }
   }

   if(!getCurrentSel()->size() && !mProcessUsesBrush)
      return;

   mUndoSel = new Selection;

   Gui3DMouseEvent event;
   if(mProcessUsesBrush)
      action->process(mMouseBrush, event, true, TerrainAction::Process);
   else
      action->process(getCurrentSel(), event, true, TerrainAction::Process);

   // check if should delete the undo
   if(mUndoSel->size())
      submitUndo( mUndoSel );
   else
      delete mUndoSel;

   mUndoSel = 0;
}

S32 TerrainEditor::getNumTextures()
{
   if(!checkTerrainBlock(this, "getNumTextures"))
      return(0);

   // walk all the possible material lists and count them..
   U32 count = 0;
   for (U32 t = 0; t < mTerrainBlocks.size(); t++)
      count += mTerrainBlocks[t]->getMaterialCount();

   return count;
}

void TerrainEditor::markEmptySquares()
{
   if(!checkTerrainBlock(this, "markEmptySquares"))
      return;
}

void TerrainEditor::mirrorTerrain(S32 mirrorIndex)
{
   if(!checkTerrainBlock(this, "mirrorTerrain"))
      return;

   // TODO!
   /*
   TerrainBlock * terrain = mActiveTerrain;
   setDirty();

   //
   enum {
      top = BIT(0),
      bottom = BIT(1),
      left = BIT(2),
      right = BIT(3)
   };

   U32 sides[8] =
   {
      bottom,
      bottom | left,
      left,
      left | top,
      top,
      top | right,
      right,
      bottom | right
   };

   U32 n = TerrainBlock::BlockSize;
   U32 side = sides[mirrorIndex % 8];
   bool diag = mirrorIndex & 0x01;

   Point2I src((side & right) ? (n - 1) : 0, (side & bottom) ? (n - 1) : 0);
   Point2I dest((side & left) ? (n - 1) : 0, (side & top) ? (n - 1) : 0);
   Point2I origSrc(src);
   Point2I origDest(dest);

   // determine the run length
   U32 minStride = ((side & top) || (side & bottom)) ? n : n / 2;
   U32 majStride = ((side & left) || (side & right)) ? n : n / 2;

   Point2I srcStep((side & right) ? -1 : 1, (side & bottom) ? -1 : 1);
   Point2I destStep((side & left) ? -1 : 1, (side & top) ? -1 : 1);

   //
   U16 * heights = terrain->getHeightAddress(0,0);
   U8 * baseMaterials = terrain->getBaseMaterialAddress(0,0);
   TerrainBlock::Material * materials = terrain->getMaterial(0,0);

   // create an undo selection
   Selection * undo = new Selection;

   // walk through all the positions
   for(U32 i = 0; i < majStride; i++)
   {
      for(U32 j = 0; j < minStride; j++)
      {
         // skip the same position
         if(src != dest)
         {
            U32 si = src.x + (src.y << TerrainBlock::BlockShift);
            U32 di = dest.x + (dest.y << TerrainBlock::BlockShift);

            // add to undo selection
            GridInfo info;
            getGridInfo(dest, info, terrain);
            undo->add(info);

            //... copy info... (height, basematerial, material)
            heights[di] = heights[si];
            baseMaterials[di] = baseMaterials[si];
            materials[di] = materials[si];
         }

         // get to the new position
         src.x += srcStep.x;
         diag ? (dest.y += destStep.y) : (dest.x += destStep.x);
      }

      // get the next position for a run
      src.y += srcStep.y;
      diag ? (dest.x += destStep.x) : (dest.y += destStep.y);

      // reset the minor run
      src.x = origSrc.x;
      diag ? (dest.y = origDest.y) : (dest.x = origDest.x);

      // shorten the run length for diag runs
      if(diag)
         minStride--;
   }

   // rebuild stuff..
   terrain->buildGridMap();
   terrain->rebuildEmptyFlags();
   terrain->packEmptySquares();

   // add undo selection
   submitUndo( undo );
   */
}

bool TerrainEditor::isPointInTerrain( const GridPoint & gPoint)
{
   PROFILE_SCOPE( TerrainEditor_IsPointInTerrain );

   Point2I cPos;
   gridToCenter( gPoint.gridPos, cPos );
   const TerrainFile *file = gPoint.terrainBlock->getFile();
   return file->isPointInTerrain( cPos.x, cPos.y );
}

void TerrainEditor::reorderMaterial( S32 index, S32 orderPos )
{   
   TerrainBlock *terr = getClientTerrain();
   Vector<U8> layerMap = terr->getLayerMap();
   Vector<TerrainMaterial*> materials = terr->getMaterials();

   TerrainMaterial *pMat = materials[index];

   submitMaterialUndo( String::ToString( "Reordered %s Material", terr->getMaterialName(index) ) );

   materials.erase( index );
   materials.insert( orderPos, pMat );

   Vector<U8>::iterator itr = layerMap.begin();
   for ( ; itr != layerMap.end(); itr++ )
   {
      // Was previous material, set to new index.
      if ( *itr == index )
         *itr = orderPos;
      else 
      {
         // We removed a Material prior to this one, bump it down.
         if ( *itr > index )
            (*itr)--;
         // We added a Material prior to this one, bump it up.
         if ( *itr >= orderPos )         
            (*itr)++;
      }
   }

   terr->setMaterials( materials );
   terr->setLayerMap( layerMap );   

   // We didn't really just "undo" but it happens to do everything we
   // need to update the materials and gui.
   onMaterialUndo( terr );
}

//------------------------------------------------------------------------------

ConsoleMethod( TerrainEditor, attachTerrain, void, 2, 3, "(TerrainBlock terrain)")
{
   SimSet * missionGroup = dynamic_cast<SimSet*>(Sim::findObject("MissionGroup"));
   if (!missionGroup)
   {
      Con::errorf(ConsoleLogEntry::Script, "TerrainEditor::attach: no mission group found");
      return;
   }

   VectorPtr<TerrainBlock*> terrains;

   // attach to first found terrainBlock
   if (argc == 2)
   {
      for(SimSetIterator itr(missionGroup); *itr; ++itr)
      {
         TerrainBlock* terrBlock = dynamic_cast<TerrainBlock*>(*itr);

         if (terrBlock)
            terrains.push_back(terrBlock);
      }

      //if (terrains.size() == 0)
      //   Con::errorf(ConsoleLogEntry::Script, "TerrainEditor::attach: no TerrainBlock objects found!");
   }
   else  // attach to named object
   {
      TerrainBlock* terrBlock = dynamic_cast<TerrainBlock*>(Sim::findObject(argv[2]));

      if (terrBlock)
         terrains.push_back(terrBlock);

      if(terrains.size() == 0)
         Con::errorf(ConsoleLogEntry::Script, "TerrainEditor::attach: failed to attach to object '%s'", argv[2]);
   }

   if (terrains.size() > 0)
   {
      for (U32 i = 0; i < terrains.size(); i++)
      {
         if (!terrains[i]->isServerObject())
         {
            terrains[i] = NULL;

            Con::errorf(ConsoleLogEntry::Script, "TerrainEditor::attach: cannot attach to client TerrainBlock");
         }
      }
   }

   for (U32 i = 0; i < terrains.size(); i++)
   {
      if (terrains[i])
	      object->attachTerrain(terrains[i]);
   }
}

ConsoleMethod( TerrainEditor, getTerrainBlockCount, S32, 2, 2, "()")
{
   return object->getTerrainBlockCount();
}

ConsoleMethod( TerrainEditor, getTerrainBlock, S32, 3, 3, "(S32 index)")
{
   TerrainBlock* tb = object->getTerrainBlock(dAtoi(argv[2]));
   if(!tb)
      return 0;
   else
      return tb->getId();
}

ConsoleMethod(TerrainEditor, getTerrainBlocksMaterialList, const char *, 2, 2, "() gets the list of current terrain materials for all terrain blocks.")
{
   Vector<StringTableEntry> list;
   object->getTerrainBlocksMaterialList(list);

   if(list.size() == 0)
      return "";

   // Calculate the size of the return buffer
   S32 size = 0;
   for(U32 i = 0; i < list.size(); ++i)
   {
      size += dStrlen(list[i]);
      ++size;
   }
   ++size;

   // Copy the material names
   char *ret = Con::getReturnBuffer(size);
   ret[0] = 0;
   for(U32 i = 0; i < list.size(); ++i)
   {
      dStrcat( ret, list[i] );
      dStrcat( ret, "\n" );
   }

   return ret;
}

ConsoleMethod( TerrainEditor, setBrushType, void, 3, 3, "(string type)"
              "One of box, ellipse, selection.")
{
	object->setBrushType(argv[2]);
}

ConsoleMethod( TerrainEditor, getBrushType, const char*, 2, 2, "()")
{
   return object->getBrushType();
}

ConsoleMethod( TerrainEditor, setBrushSize, void, 3, 4, "(int w [, int h])")
{
   S32 w = dAtoi(argv[2]);
   S32 h = argc > 3 ? dAtoi(argv[3]) : w;
	object->setBrushSize( w, h );
}

ConsoleMethod( TerrainEditor, getBrushSize, const char*, 2, 2, "()")
{
   Point2I size = object->getBrushSize();

   char * ret = Con::getReturnBuffer(32);
   dSprintf(ret, 32, "%d %d", size.x, size.y);
   return ret;
}

ConsoleMethod( TerrainEditor, setBrushPressure, void, 3, 3, "(float pressure)")
{
   object->setBrushPressure( dAtof( argv[2] ) );
}

ConsoleMethod( TerrainEditor, getBrushPressure, F32, 2, 2, "()")
{
   return object->getBrushPressure();
}

ConsoleMethod( TerrainEditor, setBrushSoftness, void, 3, 3, "(float softness)")
{
   object->setBrushSoftness( dAtof( argv[2] ) );
}

ConsoleMethod( TerrainEditor, getBrushSoftness, F32, 2, 2, "()")
{
   return object->getBrushSoftness();
}

ConsoleMethod( TerrainEditor, getBrushPos, const char*, 2, 2, "Returns a Point2I.")
{
	return object->getBrushPos();
}

ConsoleMethod( TerrainEditor, setBrushPos, void, 3, 4, "(int x, int y)")
{
   //
   Point2I pos;
   if(argc == 3)
      dSscanf(argv[2], "%d %d", &pos.x, &pos.y);
   else
   {
      pos.x = dAtoi(argv[2]);
      pos.y = dAtoi(argv[3]);
   }

   object->setBrushPos(pos);
}

ConsoleMethod( TerrainEditor, setAction, void, 3, 3, "(string action_name)")
{
	object->setAction(argv[2]);
}

ConsoleMethod( TerrainEditor, getActionName, const char*, 3, 3, "(int num)")
{
	return (object->getActionName(dAtoi(argv[2])));
}

ConsoleMethod( TerrainEditor, getNumActions, S32, 2, 2, "")
{
	return(object->getNumActions());
}

ConsoleMethod( TerrainEditor, getCurrentAction, const char*, 2, 2, "")
{
	return object->getCurrentAction();
}

ConsoleMethod( TerrainEditor, resetSelWeights, void, 3, 3, "(bool clear)")
{
	object->resetSelWeights(dAtob(argv[2]));
}

ConsoleMethod( TerrainEditor, clearSelection, void, 2, 2, "")
{
   object->clearSelection();
}

ConsoleMethod( TerrainEditor, processAction, void, 2, 3, "(string action=NULL)")
{
	if(argc == 3)
		object->processAction(argv[2]);
	else object->processAction("");
}

ConsoleMethod( TerrainEditor, getActiveTerrain, S32, 2, 2, "")
{
   S32 ret = 0;

   TerrainBlock* terrain = object->getActiveTerrain();

   if (terrain)
      ret = terrain->getId();

	return ret;
}

ConsoleMethod( TerrainEditor, getNumTextures, S32, 2, 2, "")
{
	return object->getNumTextures();
}

ConsoleMethod( TerrainEditor, markEmptySquares, void, 2, 2, "")
{
	object->markEmptySquares();
}

ConsoleMethod( TerrainEditor, mirrorTerrain, void, 3, 3, "")
{
	object->mirrorTerrain(dAtoi(argv[2]));
}

ConsoleMethod(TerrainEditor, setTerraformOverlay, void, 3, 3, "(bool overlayEnable) - sets the terraformer current heightmap to draw as an overlay over the current terrain.")
{
   // XA: This one needs to be implemented :)
}

ConsoleMethod(TerrainEditor, updateMaterial, bool, 4, 4, 
   "( int index, string matName )\n"
   "Changes the material name at the index." )
{
   TerrainBlock *terr = object->getClientTerrain();
   if ( !terr )
      return false;
   
   U32 index = dAtoi( argv[2] );
   if ( index >= terr->getMaterialCount() )
      return false;

   terr->updateMaterial( index, argv[3] );

   object->setDirty();

   return true;
}

ConsoleMethod(TerrainEditor, addMaterial, S32, 3, 3, 
   "( string matName )\n"
   "Adds a new material." )
{
   TerrainBlock *terr = object->getClientTerrain();
   if ( !terr )
      return false;
   
   terr->addMaterial( argv[2] );

   object->setDirty();

   return true;
}

ConsoleMethod( TerrainEditor, removeMaterial, void, 3, 3, "( int index ) - Remove the material at the given index." )
{
   TerrainBlock *terr = object->getClientTerrain();
   if ( !terr )
      return;
      
   S32 index = dAtoi( argv[ 2 ] );
   if ( index < 0 || index >= terr->getMaterialCount() )
   {
      Con::errorf( "TerrainEditor::removeMaterial - index out of range!" );
      return;
   }

   if ( terr->getMaterialCount() == 1 )
   {
      Con::errorf( "TerrainEditor::removeMaterial - cannot remove material, there is only one!" );
      return;
   }

   const char *matName = terr->getMaterialName( index );

   object->submitMaterialUndo( String::ToString( "Remove TerrainMaterial %s", matName ) );
   
   terr->removeMaterial( index );

   object->setDirty();
   object->scheduleMaterialUpdate();
   object->setGridUpdateMinMax();
}

ConsoleMethod(TerrainEditor, getMaterialCount, S32, 2, 2, 
   "Returns the current material count." )
{
   TerrainBlock *terr = object->getClientTerrain();
   if ( terr )
      return terr->getMaterialCount();

   return 0;
}

ConsoleMethod(TerrainEditor, getMaterials, const char *, 2, 2, "() gets the list of current terrain materials.")
{
   TerrainBlock *terr = object->getClientTerrain();
   if ( !terr )
      return "";

   char *ret = Con::getReturnBuffer(4096);
   ret[0] = 0;
   for(U32 i = 0; i < terr->getMaterialCount(); i++)
   {
      dStrcat( ret, terr->getMaterialName(i) );
      dStrcat( ret, "\n" );
   }

   return ret;
}

ConsoleMethod( TerrainEditor, getMaterialName, const char*, 3, 3, "( int index ) - Returns the name of the material at the given index." )
{
   TerrainBlock *terr = object->getClientTerrain();
   if ( !terr )
      return "";
      
   S32 index = dAtoi( argv[ 2 ] );
   if( index < 0 || index >= terr->getMaterialCount() )
   {
      Con::errorf( "TerrainEditor::getMaterialName - index out of range!" );
      return "";
   }
   
   const char* name = terr->getMaterialName( index );
   return Con::getReturnBuffer( name );
}

ConsoleMethod( TerrainEditor, getMaterialIndex, S32, 3, 3, "( string name ) - Returns the index of the material with the given name or -1." )
{
   TerrainBlock *terr = object->getClientTerrain();
   if ( !terr )
      return -1;
      
   const char* name = argv[ 2 ];
   const U32 count = terr->getMaterialCount();
   
   for( U32 i = 0; i < count; ++ i )
      if( dStricmp( name, terr->getMaterialName( i ) ) == 0 )
         return i;
         
   return -1;
}

ConsoleMethod( TerrainEditor, reorderMaterial, void, 4, 4, "( int index, int order ) "
  "- Reorder material at the given index to the new position, changing the order in which it is rendered / blended." )
{
   object->reorderMaterial( dAtoi( argv[2] ), dAtoi( argv[3] ) );
}

ConsoleMethod(TerrainEditor, getTerrainUnderWorldPoint, S32, 3, 5, "(x/y/z) Gets the terrain block that is located under the given world point.\n"
                                                                           "@param x/y/z The world coordinates (floating point values) you wish to query at. " 
                                                                           "These can be formatted as either a string (\"x y z\") or separately as (x, y, z)\n"
                                                                           "@return Returns the ID of the requested terrain block (0 if not found).\n\n")
{   
   TerrainEditor *tEditor = (TerrainEditor *) object;
   if(tEditor == NULL)
      return 0;
   Point3F pos;
   if(argc == 3)
      dSscanf(argv[2], "%f %f %f", &pos.x, &pos.y, &pos.z);
   else if(argc == 5)
   {
      pos.x = dAtof(argv[2]);
      pos.y = dAtof(argv[3]);
      pos.z = dAtof(argv[4]);
   }

   else
   {
      Con::errorf("TerrainEditor.getTerrainUnderWorldPoint(): Invalid argument count! Valid arguments are either \"x y z\" or x,y,z\n");
      return 0;
   }

   TerrainBlock* terrain = tEditor->getTerrainUnderWorldPoint(pos);
   if(terrain != NULL)
   {
      return terrain->getId();
   }

   return 0;
}

//------------------------------------------------------------------------------

void TerrainEditor::initPersistFields()
{
   addGroup("Misc");	
   addField("isDirty", TypeBool, Offset(mIsDirty, TerrainEditor));
   addField("isMissionDirty", TypeBool, Offset(mIsMissionDirty, TerrainEditor));
   addField("renderBorder", TypeBool, Offset(mRenderBorder, TerrainEditor));                    ///< Not currently used
   addField("borderHeight", TypeF32, Offset(mBorderHeight, TerrainEditor));                     ///< Not currently used
   addField("borderFillColor", TypeColorI, Offset(mBorderFillColor, TerrainEditor));            ///< Not currently used
   addField("borderFrameColor", TypeColorI, Offset(mBorderFrameColor, TerrainEditor));          ///< Not currently used
   addField("borderLineMode", TypeBool, Offset(mBorderLineMode, TerrainEditor));                ///< Not currently used
   addField("selectionHidden", TypeBool, Offset(mSelectionHidden, TerrainEditor));
   addField("renderVertexSelection", TypeBool, Offset(mRenderVertexSelection, TerrainEditor));  ///< Not currently used
   addField("renderSolidBrush", TypeBool, Offset(mRenderSolidBrush, TerrainEditor));
   addField("processUsesBrush", TypeBool, Offset(mProcessUsesBrush, TerrainEditor));
   addField("maxBrushSize", TypePoint2I, Offset(mMaxBrushSize, TerrainEditor));

   // action values...
   addField("adjustHeightVal", TypeF32, Offset(mAdjustHeightVal, TerrainEditor));               ///< RaiseHeightAction and LowerHeightAction
   addField("setHeightVal", TypeF32, Offset(mSetHeightVal, TerrainEditor));                     ///< SetHeightAction
   addField("scaleVal", TypeF32, Offset(mScaleVal, TerrainEditor));                             ///< ScaleHeightAction
   addField("smoothFactor", TypeF32, Offset(mSmoothFactor, TerrainEditor));                     ///< SmoothHeightAction
   addField("noiseFactor", TypeF32, Offset(mNoiseFactor, TerrainEditor));                       ///< PaintNoiseAction
   addField("materialGroup", TypeS32, Offset(mMaterialGroup, TerrainEditor));                   ///< Not currently used
   addField("softSelectRadius", TypeF32, Offset(mSoftSelectRadius, TerrainEditor));             ///< SoftSelectAction
   addField("softSelectFilter", TypeString, Offset(mSoftSelectFilter, TerrainEditor));          ///< SoftSelectAction brush filtering
   addField("softSelectDefaultFilter", TypeString, Offset(mSoftSelectDefaultFilter, TerrainEditor));  ///< SoftSelectAction brush filtering
   addField("adjustHeightMouseScale", TypeF32, Offset(mAdjustHeightMouseScale, TerrainEditor)); ///< Not currently used
   addField("paintIndex", TypeS32, Offset(mPaintIndex, TerrainEditor));                         ///< PaintMaterialAction
   endGroup("Misc");

   Parent::initPersistFields();
}

ConsoleMethod( TerrainEditor, getSlopeLimitMinAngle, F32, 2, 2, 0)
{
   return object->mSlopeMinAngle;
}

ConsoleMethod( TerrainEditor, setSlopeLimitMinAngle, F32, 3, 3, 0)
{
	F32 angle = dAtof( argv[2] );	
	if ( angle < 0.0f )
		angle = 0.0f;
   if ( angle > object->mSlopeMaxAngle )
      angle = object->mSlopeMaxAngle;

	object->mSlopeMinAngle = angle;
	return angle;
}

ConsoleMethod( TerrainEditor, getSlopeLimitMaxAngle, F32, 2, 2, 0)
{
   return object->mSlopeMaxAngle;
}

ConsoleMethod( TerrainEditor, setSlopeLimitMaxAngle, F32, 3, 3, 0)
{
	F32 angle = dAtof( argv[2] );	
	if ( angle > 90.0f )
		angle = 90.0f;
   if ( angle < object->mSlopeMinAngle )
      angle = object->mSlopeMinAngle;
      
	object->mSlopeMaxAngle = angle;
	return angle;
}

//------------------------------------------------------------------------------  
void TerrainEditor::autoMaterialLayer( F32 mMinHeight, F32 mMaxHeight, F32 mMinSlope, F32 mMaxSlope )  
{  
   if (!mActiveTerrain)  
      return;  
  
   S32 mat = getPaintMaterialIndex();  
   if (mat == -1)  
      return;  
  
   mUndoSel = new Selection;  
          
   U32 terrBlocks = mActiveTerrain->getBlockSize();  
   for (U32 y = 0; y < terrBlocks; y++) 
   {  
      for (U32 x = 0; x < terrBlocks; x++) 
      {  
         // get info  
         GridPoint gp;  
         gp.terrainBlock = mActiveTerrain;  
         gp.gridPos.set(x, y);  
  
         GridInfo gi;  
         getGridInfo(gp, gi);  
  
         if (gi.mMaterial == mat)  
            continue;  
  
         Point3F wp;  
         gridToWorld(gp, wp);  
  
         if (!(wp.z >= mMinHeight && wp.z <= mMaxHeight))  
            continue;  
  
         // transform wp to object space  
         Point3F op;  
         mActiveTerrain->getWorldTransform().mulP(wp, &op);  
  
         Point3F norm;  
         mActiveTerrain->getNormal(Point2F(op.x, op.y), &norm, true);  
  
         if (mMinSlope > 0)  
            if (norm.z > mSin(mDegToRad(90.0f - mMinSlope)))  
               continue;  
  
         if (mMaxSlope < 90)  
            if (norm.z < mSin(mDegToRad(90.0f - mMaxSlope)))  
               continue;  
  
         gi.mMaterialChanged = true;  
         mUndoSel->add(gi);  
         gi.mMaterial = mat;  
         setGridInfo(gi);  
      }  
   }  
  
   if(mUndoSel->size())  
      submitUndo( mUndoSel );  
   else  
      delete mUndoSel;  
  
   mUndoSel = 0;  
  
   scheduleMaterialUpdate();     
}  
  
ConsoleMethod( TerrainEditor, autoMaterialLayer, void, 6, 6, "(float minHeight, float maxHeight, float minSlope, float maxSlope)")  
{  
   object->autoMaterialLayer( dAtof(argv[2]), dAtof(argv[3]), dAtof(argv[4]), dAtof(argv[5]) );  
}  
