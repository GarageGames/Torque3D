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
#include "gui/worldEditor/terrainActions.h"

#include "gui/core/guiCanvas.h"

//------------------------------------------------------------------------------

void SelectAction::process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type)
{
   if(sel == mTerrainEditor->getCurrentSel())
      return;

   if(type == Process)
      return;

   if(selChanged)
   {
      if(event.modifier & SI_MULTISELECT)
      {
         for(U32 i = 0; i < sel->size(); i++)
            mTerrainEditor->getCurrentSel()->remove((*sel)[i]);
      }
      else
      {
         for(U32 i = 0; i < sel->size(); i++)
         {
            GridInfo gInfo;
            if(mTerrainEditor->getCurrentSel()->getInfo((*sel)[i].mGridPoint.gridPos, gInfo))
            {
               if(!gInfo.mPrimarySelect)
                  gInfo.mPrimarySelect = (*sel)[i].mPrimarySelect;

               if(gInfo.mWeight < (*sel)[i].mWeight)
                  gInfo.mWeight = (*sel)[i].mWeight;

               mTerrainEditor->getCurrentSel()->setInfo(gInfo);
            }
            else
               mTerrainEditor->getCurrentSel()->add((*sel)[i]);
         }
      }
   }
}

void DeselectAction::process(Selection * sel, const Gui3DMouseEvent & event, bool selChanged, Type type)
{
   if(sel == mTerrainEditor->getCurrentSel())
      return;

   if(type == Process)
      return;

   if(selChanged)
   {
      for(U32 i = 0; i < sel->size(); i++)
         mTerrainEditor->getCurrentSel()->remove((*sel)[i]);
   }
}

//------------------------------------------------------------------------------

void SoftSelectAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type type)
{
   TerrainBlock *terrBlock = mTerrainEditor->getActiveTerrain();
   if ( !terrBlock )
      return;
      
   // allow process of current selection
   Selection tmpSel;
   if(sel == mTerrainEditor->getCurrentSel())
   {
      tmpSel = *sel;
      sel = &tmpSel;
   }

   if(type == Begin || type == Process)
      mFilter.set(1, &mTerrainEditor->mSoftSelectFilter);

   //
   if(selChanged)
   {
      F32 radius = mTerrainEditor->mSoftSelectRadius;
      if(radius == 0.f)
         return;

      S32 squareSize = terrBlock->getSquareSize();
      U32 offset = U32(radius / F32(squareSize)) + 1;

      for(U32 i = 0; i < sel->size(); i++)
      {
         GridInfo & info = (*sel)[i];

         info.mPrimarySelect = true;
         info.mWeight = mFilter.getValue(0);

         if(!mTerrainEditor->getCurrentSel()->add(info))
            mTerrainEditor->getCurrentSel()->setInfo(info);

         Point2F infoPos((F32)info.mGridPoint.gridPos.x, (F32)info.mGridPoint.gridPos.y);

         //
         for(S32 x = info.mGridPoint.gridPos.x - offset; x < info.mGridPoint.gridPos.x + (offset << 1); x++)
            for(S32 y = info.mGridPoint.gridPos.y - offset; y < info.mGridPoint.gridPos.y + (offset << 1); y++)
            {
               //
               Point2F pos((F32)x, (F32)y);

               F32 dist = Point2F(pos - infoPos).len() * F32(squareSize);

               if(dist > radius)
                  continue;

               F32 weight = mFilter.getValue(dist / radius);

               //
               GridInfo gInfo;
               GridPoint gridPoint = info.mGridPoint;
               gridPoint.gridPos.set(x, y);

               if(mTerrainEditor->getCurrentSel()->getInfo(Point2I(x, y), gInfo))
               {
                  if(gInfo.mPrimarySelect)
                     continue;

                  if(gInfo.mWeight < weight)
                  {
                     gInfo.mWeight = weight;
                     mTerrainEditor->getCurrentSel()->setInfo(gInfo);
                  }
               }
               else
               {
                  Vector<GridInfo> gInfos;
                  mTerrainEditor->getGridInfos(gridPoint, gInfos);

                  for (U32 z = 0; z < gInfos.size(); z++)
                  {
                     gInfos[z].mWeight = weight;
                     gInfos[z].mPrimarySelect = false;
                     mTerrainEditor->getCurrentSel()->add(gInfos[z]);
                  }
               }
            }
      }
   }
}

//------------------------------------------------------------------------------

void OutlineSelectAction::process(Selection * sel, const Gui3DMouseEvent & event, bool, Type type)
{
   TORQUE_UNUSED(sel); TORQUE_UNUSED(event); TORQUE_UNUSED(type);
   switch(type)
   {
      case Begin:
         if(event.modifier & SI_SHIFT)
            break;

         mTerrainEditor->getCurrentSel()->reset();
         break;

      case End:
      case Update:

      default:
         return;
   }

   mLastEvent = event;
}

//------------------------------------------------------------------------------

void PaintMaterialAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   S32 mat = mTerrainEditor->getPaintMaterialIndex();
   if ( !selChanged || mat < 0 )
      return;

   const bool slopeLimit = mTerrainEditor->mSlopeMinAngle > 0.0f || mTerrainEditor->mSlopeMaxAngle < 90.0f;
   const F32 minSlope = mSin( mDegToRad( 90.0f - mTerrainEditor->mSlopeMinAngle ) );
   const F32 maxSlope = mSin( mDegToRad( 90.0f - mTerrainEditor->mSlopeMaxAngle ) );

   const TerrainBlock *terrain = mTerrainEditor->getActiveTerrain();
   const F32 squareSize = terrain->getSquareSize();

   Point2F p;
   Point3F norm;


   for( U32 i = 0; i < sel->size(); i++ )
   {
      GridInfo &inf = (*sel)[i];

      if ( slopeLimit )
      {
         p.x = inf.mGridPoint.gridPos.x * squareSize;
         p.y = inf.mGridPoint.gridPos.y * squareSize;
         if ( !terrain->getNormal( p, &norm, true ) )
            continue;

         if (  norm.z > minSlope ||
               norm.z < maxSlope )
            continue;  
      }

      // If grid is already set to our material, or it is an
      // empty grid spot, then skip painting.
      if ( inf.mMaterial == mat || inf.mMaterial == U8_MAX )
         continue;

      if ( mRandF() > mTerrainEditor->getBrushPressure() )
         continue;

      inf.mMaterialChanged = true;
      mTerrainEditor->getUndoSel()->add(inf);

      // Painting is really simple now... set the one mat index.
      inf.mMaterial = mat;
      mTerrainEditor->setGridInfo(inf, true);
   }

   mTerrainEditor->scheduleMaterialUpdate();
}

//------------------------------------------------------------------------------

void ClearMaterialsAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(selChanged)
   {
      for(U32 i = 0; i < sel->size(); i++)
      {
         GridInfo &inf = (*sel)[i];

         mTerrainEditor->getUndoSel()->add(inf);
         inf.mMaterialChanged = true;

         // Reset to the first texture layer.
         inf.mMaterial = 0; 
         mTerrainEditor->setGridInfo(inf);
      }
      mTerrainEditor->scheduleMaterialUpdate();
   }
}

//------------------------------------------------------------------------------

void RaiseHeightAction::process( Selection *sel, const Gui3DMouseEvent &evt, bool selChanged, Type type )
{
   // ok the raise height action is our "dirt pour" action
   // only works on brushes...

   Brush *brush = dynamic_cast<Brush*>(sel);
   if ( !brush )
      return;

   if ( type == End )   
      return;

   Point2I brushPos = brush->getPosition();
   Point2I brushSize = brush->getSize();
   GridPoint brushGridPoint = brush->getGridPoint();

   Vector<GridInfo> cur; // the height at the brush position
   mTerrainEditor->getGridInfos(brushGridPoint, cur);

   if ( cur.size() == 0 )
      return;

   // we get 30 process actions per second (at least)
   F32 heightAdjust = mTerrainEditor->mAdjustHeightVal / 30;
   // nothing can get higher than the current brush pos adjusted height

   F32 maxHeight = cur[0].mHeight + heightAdjust;

   for ( U32 i = 0; i < sel->size(); i++ )
   {
      mTerrainEditor->getUndoSel()->add((*sel)[i]);
      if ( (*sel)[i].mHeight < maxHeight )
      {
         (*sel)[i].mHeight += heightAdjust * (*sel)[i].mWeight;
         if ( (*sel)[i].mHeight > maxHeight )
            (*sel)[i].mHeight = maxHeight;
      }
      mTerrainEditor->setGridInfo((*sel)[i]);
   }   

   mTerrainEditor->scheduleGridUpdate();  
}

//------------------------------------------------------------------------------

void LowerHeightAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type type)
{
   // ok the lower height action is our "dirt dig" action
   // only works on brushes...

   Brush *brush = dynamic_cast<Brush *>(sel);
   if(!brush)
      return;

   if ( type == End )   
      return;

   Point2I brushPos = brush->getPosition();
   Point2I brushSize = brush->getSize();
   GridPoint brushGridPoint = brush->getGridPoint();

   Vector<GridInfo> cur; // the height at the brush position
   mTerrainEditor->getGridInfos(brushGridPoint, cur);

   if (cur.size() == 0)
      return;

   // we get 30 process actions per second (at least)
   F32 heightAdjust = -mTerrainEditor->mAdjustHeightVal / 30;
   // nothing can get higher than the current brush pos adjusted height

   F32 maxHeight = cur[0].mHeight + heightAdjust;
   if(maxHeight < 0)
      maxHeight = 0;

   for(U32 i = 0; i < sel->size(); i++)
   {
      mTerrainEditor->getUndoSel()->add((*sel)[i]);
      if((*sel)[i].mHeight > maxHeight)
      {
         (*sel)[i].mHeight += heightAdjust * (*sel)[i].mWeight;
         if((*sel)[i].mHeight < maxHeight)
            (*sel)[i].mHeight = maxHeight;
      }
      mTerrainEditor->setGridInfo((*sel)[i]);
   }

   mTerrainEditor->scheduleGridUpdate();   
}

//------------------------------------------------------------------------------

void SetHeightAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(selChanged)
   {
      for(U32 i = 0; i < sel->size(); i++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);
         (*sel)[i].mHeight = mTerrainEditor->mSetHeightVal;
         mTerrainEditor->setGridInfo((*sel)[i]);
      }
      mTerrainEditor->scheduleGridUpdate();
   }
}

//------------------------------------------------------------------------------

void SetEmptyAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if ( !selChanged )
      return;

   mTerrainEditor->setMissionDirty();

   for ( U32 i = 0; i < sel->size(); i++ )
   {
      GridInfo &inf = (*sel)[i];

      // Skip already empty blocks.
      if ( inf.mMaterial == U8_MAX )
         continue;

      // The change flag needs to be set on the undo
      // so that it knows to restore materials.
      inf.mMaterialChanged = true;
      mTerrainEditor->getUndoSel()->add( inf );

      // Set the material to empty.
      inf.mMaterial = -1;
      mTerrainEditor->setGridInfo( inf );
   }

   mTerrainEditor->scheduleGridUpdate();
}

//------------------------------------------------------------------------------

void ClearEmptyAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if ( !selChanged )
      return;

   mTerrainEditor->setMissionDirty();

   for ( U32 i = 0; i < sel->size(); i++ )
   {
      GridInfo &inf = (*sel)[i];

      // Skip if not empty.
      if ( inf.mMaterial != U8_MAX )
         continue;

      // The change flag needs to be set on the undo
      // so that it knows to restore materials.
      inf.mMaterialChanged = true;
      mTerrainEditor->getUndoSel()->add( inf );

      // Set the material
      inf.mMaterial = 0;
      mTerrainEditor->setGridInfo( inf );
   }

   mTerrainEditor->scheduleGridUpdate();
}

//------------------------------------------------------------------------------

void ScaleHeightAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(selChanged)
   {
      for(U32 i = 0; i < sel->size(); i++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);
         (*sel)[i].mHeight *= mTerrainEditor->mScaleVal;
         mTerrainEditor->setGridInfo((*sel)[i]);
      }
      mTerrainEditor->scheduleGridUpdate();
   }
}

void BrushAdjustHeightAction::process(Selection * sel, const Gui3DMouseEvent & event, bool, Type type)
{
   if(type == Process)
      return;

   TerrainBlock *terrBlock = mTerrainEditor->getActiveTerrain();
   if ( !terrBlock )
      return;

   if(type == Begin)
   {
      mTerrainEditor->lockSelection(true);
      mTerrainEditor->getRoot()->mouseLock(mTerrainEditor);

      // the way this works is:
      // construct a plane that goes through the collision point
      // with one axis up the terrain Z, and horizontally parallel to the
      // plane of projection

      // the cross of the camera ffdv and the terrain up vector produces
      // the cross plane vector.

      // all subsequent mouse actions are collided against the plane and the deltaZ
      // from the previous position is used to delta the selection up and down.
      Point3F cameraDir;

      EditTSCtrl::smCamMatrix.getColumn(1, &cameraDir);
      terrBlock->getTransform().getColumn(2, &mTerrainUpVector);

      // ok, get the cross vector for the plane:
      Point3F planeCross;
      mCross(cameraDir, mTerrainUpVector, &planeCross);

      planeCross.normalize();
      Point3F planeNormal;

      Point3F intersectPoint;
      mTerrainEditor->collide(event, intersectPoint);

      mCross(mTerrainUpVector, planeCross, &planeNormal);
      mIntersectionPlane.set(intersectPoint, planeNormal);

      // ok, we have the intersection point...
      // project the collision point onto the up vector of the terrain

      mPreviousZ = mDot(mTerrainUpVector, intersectPoint);

      // add to undo
      // and record the starting heights
      for(U32 i = 0; i < sel->size(); i++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);
         (*sel)[i].mStartHeight = (*sel)[i].mHeight;
      }
   }
   else if(type == Update)
   {
      // ok, collide the ray from the event with the intersection plane:

      Point3F intersectPoint;
      Point3F start = event.pos;
      Point3F end = start + event.vec * 1000;

      F32 t = mIntersectionPlane.intersect(start, end);

      m_point3F_interpolate( start, end, t, intersectPoint);
      F32 currentZ = mDot(mTerrainUpVector, intersectPoint);

      F32 diff = currentZ - mPreviousZ;

      for(U32 i = 0; i < sel->size(); i++)
      {
         (*sel)[i].mHeight = (*sel)[i].mStartHeight + diff * (*sel)[i].mWeight;

         // clamp it
         if((*sel)[i].mHeight < 0.f)
            (*sel)[i].mHeight = 0.f;
         if((*sel)[i].mHeight > 2047.f)
            (*sel)[i].mHeight = 2047.f;

         mTerrainEditor->setGridInfoHeight((*sel)[i]);
      }
      mTerrainEditor->scheduleGridUpdate();
   }
   else if(type == End)
   {
      mTerrainEditor->getRoot()->mouseUnlock(mTerrainEditor);
   }
}

//------------------------------------------------------------------------------

AdjustHeightAction::AdjustHeightAction(TerrainEditor * editor) :
   BrushAdjustHeightAction(editor)
{
   mCursor = 0;
}

void AdjustHeightAction::process(Selection *sel, const Gui3DMouseEvent & event, bool b, Type type)
{
   Selection * curSel = mTerrainEditor->getCurrentSel();
   BrushAdjustHeightAction::process(curSel, event, b, type);
}

//------------------------------------------------------------------------------
// flatten the primary selection then blend in the rest...

void FlattenHeightAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(!sel->size())
      return;

   if(selChanged)
   {
      F32 average = 0.f;

      // get the average height
      U32 cPrimary = 0;
      for(U32 k = 0; k < sel->size(); k++)
         if((*sel)[k].mPrimarySelect)
         {
            cPrimary++;
            average += (*sel)[k].mHeight;
         }

      average /= cPrimary;

      // set it
      for(U32 i = 0; i < sel->size(); i++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);

         //
         if((*sel)[i].mPrimarySelect)
            (*sel)[i].mHeight = average;
         else
         {
            F32 h = average - (*sel)[i].mHeight;
            (*sel)[i].mHeight += (h * (*sel)[i].mWeight);
         }

         mTerrainEditor->setGridInfo((*sel)[i]);
      }
      mTerrainEditor->scheduleGridUpdate();
   }
}

//------------------------------------------------------------------------------

void SmoothHeightAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if(!sel->size())
      return;

   if(selChanged)
   {
      F32 avgHeight = 0.f;
      for(U32 k = 0; k < sel->size(); k++)
      {
         mTerrainEditor->getUndoSel()->add((*sel)[k]);
         avgHeight += (*sel)[k].mHeight;
      }

      avgHeight /= sel->size();

      // clamp the terrain smooth factor...
      if(mTerrainEditor->mSmoothFactor < 0.f)
         mTerrainEditor->mSmoothFactor = 0.f;
      if(mTerrainEditor->mSmoothFactor > 1.f)
         mTerrainEditor->mSmoothFactor = 1.f;

      // linear
      for(U32 i = 0; i < sel->size(); i++)
      {
         (*sel)[i].mHeight += (avgHeight - (*sel)[i].mHeight) * mTerrainEditor->mSmoothFactor * (*sel)[i].mWeight;
         mTerrainEditor->setGridInfo((*sel)[i]);
      }
      mTerrainEditor->scheduleGridUpdate();
   }
}

void SmoothSlopeAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)  
{  
   if(!sel->size())  
      return;  
  
   if(selChanged)  
   {  
      // Perform simple 2d linear regression on x&z and y&z:  
      // b = (Avg(xz) - Avg(x)Avg(z))/(Avg(x^2) - Avg(x)^2)  
      Point2F prod(0.f, 0.f);   // mean of product for covar  
      Point2F avgSqr(0.f, 0.f); // mean sqr of x, y for var  
      Point2F avgPos(0.f, 0.f);  
      F32 avgHeight = 0.f;  
      F32 z;  
      Point2F pos;  
      for(U32 k = 0; k < sel->size(); k++)  
      {  
         mTerrainEditor->getUndoSel()->add((*sel)[k]);  
         pos = Point2F((*sel)[k].mGridPoint.gridPos.x, (*sel)[k].mGridPoint.gridPos.y);  
         z = (*sel)[k].mHeight;  
  
         prod += pos * z;  
         avgSqr += pos * pos;  
         avgPos += pos;  
         avgHeight += z;  
      }  
  
      prod /= sel->size();  
      avgSqr /= sel->size();  
      avgPos /= sel->size();  
      avgHeight /= sel->size();  
  
      Point2F avgSlope = (prod - avgPos*avgHeight)/(avgSqr - avgPos*avgPos);  
  
      F32 goalHeight;  
      for(U32 i = 0; i < sel->size(); i++)  
      {  
         goalHeight = avgHeight + ((*sel)[i].mGridPoint.gridPos.x - avgPos.x)*avgSlope.x +  
            ((*sel)[i].mGridPoint.gridPos.y - avgPos.y)*avgSlope.y;  
         (*sel)[i].mHeight += (goalHeight - (*sel)[i].mHeight) * (*sel)[i].mWeight;  
         mTerrainEditor->setGridInfo((*sel)[i]);  
      }  
      mTerrainEditor->scheduleGridUpdate();  
   }  
}  

void PaintNoiseAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type type)
{
   // If this is the ending
   // mouse down event, then
   // update the noise values.
   if ( type == Begin )
   {
      mNoise.setSeed( Sim::getCurrentTime() );
      mNoise.fBm( &mNoiseData, mNoiseSize, 12, 1.0f, 5.0f );
      mNoise.getMinMax( &mNoiseData, &mMinMaxNoise.x, &mMinMaxNoise.y, mNoiseSize );
    
      mScale = 1.5f / ( mMinMaxNoise.x - mMinMaxNoise.y);
   }

   if( selChanged )
   {
      for( U32 i = 0; i < sel->size(); i++ )
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);

         const Point2I &gridPos = (*sel)[i].mGridPoint.gridPos;

         const F32 noiseVal = mNoiseData[ ( gridPos.x % mNoiseSize ) + 
                                          ( ( gridPos.y % mNoiseSize ) * mNoiseSize ) ];

         (*sel)[i].mHeight += (noiseVal - mMinMaxNoise.y * mScale) * (*sel)[i].mWeight * mTerrainEditor->mNoiseFactor;

         mTerrainEditor->setGridInfo((*sel)[i]);
      }

      mTerrainEditor->scheduleGridUpdate();
   }
}
/*
void ThermalErosionAction::process(Selection * sel, const Gui3DMouseEvent &, bool selChanged, Type)
{
   if( selChanged )
   {
      TerrainBlock *tblock = mTerrainEditor->getActiveTerrain();
      if ( !tblock )
         return;
      
      F32 height = 0;
      F32 maxHeight = 0;
      U32 shift = getBinLog2( TerrainBlock::BlockSize );

      for ( U32 x = 0; x < TerrainBlock::BlockSize; x++ )
      {
         for ( U32 y = 0; y < TerrainBlock::BlockSize; y++ )
         {
            height = fixedToFloat( tblock->getHeight( x, y ) );
            mTerrainHeights[ x + (y << 8)] = height;

            if ( height > maxHeight )
               maxHeight = height;
         }
      }

      //mNoise.erodeThermal( &mTerrainHeights, &mNoiseData, 30.0f, 5.0f, 5, TerrainBlock::BlockSize, tblock->getSquareSize(), maxHeight );
         
      mNoise.erodeHydraulic( &mTerrainHeights, &mNoiseData, 1, TerrainBlock::BlockSize );

      F32 heightDiff = 0;

      for( U32 i = 0; i < sel->size(); i++ )
      {
         mTerrainEditor->getUndoSel()->add((*sel)[i]);

         const Point2I &gridPos = (*sel)[i].mGridPoint.gridPos;
         
         // Need to get the height difference
         // between the current height and the
         // erosion height to properly apply the
         // softness and pressure settings of the brush
         // for this selection.
         heightDiff = (*sel)[i].mHeight - mNoiseData[ gridPos.x + (gridPos.y << shift)];

         (*sel)[i].mHeight -= (heightDiff * (*sel)[i].mWeight);

         mTerrainEditor->setGridInfo((*sel)[i]);
      }

      mTerrainEditor->gridUpdateComplete();
   }
}
*/


IMPLEMENT_CONOBJECT( TerrainSmoothAction );

ConsoleDocClass( TerrainSmoothAction,
   "@brief Terrain action used for leveling varying terrain heights smoothly.\n\n"
   "Editor use only.\n\n"
   "@internal"
);

TerrainSmoothAction::TerrainSmoothAction()
   :  UndoAction( "Terrain Smoothing" )
{
}

void TerrainSmoothAction::initPersistFields()
{
   Parent::initPersistFields();
}

void TerrainSmoothAction::smooth( TerrainBlock *terrain, F32 factor, U32 steps )
{
   AssertFatal( terrain, "TerrainSmoothAction::smooth() - Got null object!" );

   // Store our input parameters.
   mTerrainId = terrain->getId();
   mSteps = steps;
   mFactor = factor;

   // The redo can do the rest.
   redo();
}

ConsoleMethod( TerrainSmoothAction, smooth, void, 5, 5, "( TerrainBlock obj, F32 factor, U32 steps )")
{
   TerrainBlock *terrain = NULL;
   if ( Sim::findObject( argv[2], terrain ) && terrain )
   	object->smooth( terrain, dAtof( argv[3] ), mClamp( dAtoi( argv[4] ), 1, 13 ) );
}

void TerrainSmoothAction::undo()
{
   // First find the terrain from the id.
   TerrainBlock *terrain;
   if ( !Sim::findObject( mTerrainId, terrain ) || !terrain )
      return;

   // Get the terrain file.
   TerrainFile *terrFile = terrain->getFile();

   // Copy our stored heightmap to the file.
   terrFile->setHeightMap( mUnsmoothedHeights, false );

   // Tell the terrain to update itself.
   terrain->updateGrid( Point2I::Zero, Point2I::Max, true );
}

void TerrainSmoothAction::redo()
{
   // First find the terrain from the id.
   TerrainBlock *terrain;
   if ( !Sim::findObject( mTerrainId, terrain ) || !terrain )
      return;

   // Get the terrain file.
   TerrainFile *terrFile = terrain->getFile();

   // First copy the heightmap state.
   mUnsmoothedHeights = terrFile->getHeightMap();

   // Do the smooth.
   terrFile->smooth( mFactor, mSteps, false );

   // Tell the terrain to update itself.
   terrain->updateGrid( Point2I::Zero, Point2I::Max, true );
}