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

#include "gfx/gfxInit.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "gui/core/guiCanvas.h"

#include "gui/editor/guiParticleGraphCtrl.h"

IMPLEMENT_CONOBJECT(GuiParticleGraphCtrl);

ConsoleDocClass( GuiParticleGraphCtrl,
   "@brief Legacy remnant from old Torque particle editor\n\n"
   "Editor use only.\n\n"
   "@internal"
);

GuiParticleGraphCtrl::GuiParticleGraphCtrl()
{

   for(int i = 0; i < MaxPlots; i++)
   {
	   mPlots[i].mGraphColor = ColorF(1.0, 1.0, 1.0);
	   VECTOR_SET_ASSOCIATION(mPlots[i].mGraphData);
	   mPlots[i].mGraphMax.x = 1;
	   mPlots[i].mGraphMax.y = 50;
	   mPlots[i].mGraphMin.x = 0;
	   mPlots[i].mGraphMin.y = 0;
	   mPlots[i].mGraphType = Polyline;
	   mPlots[i].mGraphName = StringTable->insert("");
	   mPlots[i].mHidden = false;
	   mPlots[i].mGraphScale = 0.05f;
   }

   mPlots[0].mGraphColor = ColorF(1.0f, 0.2f, 0.2f);
   mPlots[1].mGraphColor = ColorF(1.0f, 0.5f, 0.5f);
   mPlots[2].mGraphColor = ColorF(0.0f, 1.0f, 0.0f);
   mPlots[3].mGraphColor = ColorF(0.0f, 0.0f, 1.0f);
   mPlots[4].mGraphColor = ColorF(0.0f, 1.0f, 1.0f);
   mPlots[5].mGraphColor = ColorF(0.0f, 0.0f, 0.0f);
   mPlots[6].mGraphColor = ColorF(0.5f, 0.5f, 0.5f);
   mPlots[7].mGraphColor = ColorF(0.5f, 0.0f, 0.0f);
   mPlots[8].mGraphColor = ColorF(0.0f, 0.5f, 0.0f);
   mPlots[9].mGraphColor = ColorF(0.0f, 0.0f, 0.5f);
   mPlots[10].mGraphColor = ColorF(0.0f, 0.5f, 0.5f);
   mPlots[11].mGraphColor = ColorF(0.25f, 0.25f, 0.25f);
   mPlots[12].mGraphColor = ColorF(0.5f, 0.5f, 0.5f);
   mPlots[13].mGraphColor = ColorF(0.5f, 0.0f, 0.0f);
   mPlots[14].mGraphColor = ColorF(0.0f, 0.5f, 0.0f);
   mPlots[15].mGraphColor = ColorF(0.0f, 0.0f, 0.5f);
   mPlots[16].mGraphColor = ColorF(0.0f, 0.5f, 0.5f);
   mPlots[17].mGraphColor = ColorF(0.25f, 0.25f, 0.25f);
   mPlots[18].mGraphColor = ColorF(1.0f, 0.2f, 0.2f);
   mPlots[19].mGraphColor = ColorF(1.0f, 0.5f, 0.5f);
   mPlots[20].mGraphColor = ColorF(0.0f, 1.0f, 0.0f);
   mPlots[21].mGraphColor = ColorF(0.0f, 0.0f, 1.0f);
   mPlots[22].mGraphColor = ColorF(0.0f, 1.0f, 1.0f);
   mPlots[23].mGraphColor = ColorF(0.0f, 0.0f, 0.0f);
   mPlots[24].mGraphColor = ColorF(0.5f, 0.5f, 0.5f);
   mPlots[25].mGraphColor = ColorF(0.5f, 0.0f, 0.0f);
   mPlots[26].mGraphColor = ColorF(0.0f, 0.5f, 0.0f);
   mPlots[27].mGraphColor = ColorF(0.0f, 0.0f, 0.5f);
   mPlots[28].mGraphColor = ColorF(1.0f, 0.0f, 0.0f);
   mPlots[29].mGraphColor = ColorF(0.0f, 1.0f, 0.0f);
   mPlots[30].mGraphColor = ColorF(0.0f, 0.0f, 1.0f);
   mPlots[31].mGraphColor = ColorF(0.5f, 0.0f, 0.0f);

   mVertexClickSize = 6;
   mSelectedPlot = 0;
   mSelectedPoint = -1;
   mOriginalSelectedPoint = -1;
   mLastSelectedPoint = -1;
   mAddedPointIndex = -1;
   mAutoMax = false;
   mAutoRemove = false;
   mRenderAllPoints = false;
   mRenderGraphTooltip = true;
   mPointWasAdded = false;
   mPointXMovementClamped = false;
   mOutlineColor = ColorI(1, 1, 1);
   mCursorPos = Point2I(0, 0);
}

ImplementEnumType( GuiParticleGraphType,
   "\n\n"
   "@ingroup GuiCore" 
   "@internal")
   { GuiParticleGraphCtrl::Bar,        "bar"      },
   { GuiParticleGraphCtrl::Filled,     "filled"   },
   { GuiParticleGraphCtrl::Point,      "point"    },
   { GuiParticleGraphCtrl::Polyline ,  "polyline" },
EndImplementEnumType;

bool GuiParticleGraphCtrl::onWake()
{
   if (! Parent::onWake())
      return false;
   setActive(true);
   return true;
}

void GuiParticleGraphCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   // Fetch Draw Utility.
   GFXDrawUtil* pDrawUtil = GFX->getDrawUtil();

	if (mProfile->mBorder)
	{
		const RectI bounds = getBounds();
		RectI rect(offset.x, offset.y, bounds.extent.x, bounds.extent.y);		
		pDrawUtil->drawRect(rect, mProfile->mBorderColor);
	}

    GuiControlProfile* profile = dynamic_cast<GuiControlProfile*>(Sim::findObject("GuiDefaultProfile"));
    Resource<GFont> font = profile->mFont;
	GFXVideoMode videoMode = GFXInit::getDesktopResolution();

	ColorF color(1.0f, 1.0f, 1.0f, 0.5f);
	pDrawUtil->drawRectFill(updateRect, color);

	for (int k = 0; k < MaxPlots; k++)
	{
		// Nothing to graph
		if ((mPlots[k].mGraphData.size() == 0) || (mPlots[k].mHidden == true))
			continue;
	
        Point2F graphExtent = getGraphExtent(k);

		// Adjust scale to max value + 5% so we can see high value
		F32 ScaleX = (F32(getExtent().x) / (graphExtent.x*(1.00 + (mPlots[k].mGraphScale))));
		F32 ScaleY = (F32(getExtent().y) / (graphExtent.y*(1.00 + (mPlots[k].mGraphScale))));

		if((mPlots[k].mGraphType == Point) || (mPlots[k].mGraphType == Polyline))
		{
         S32 posX;
			S32 posY;
			S32 lastPosX = 0;
			S32 lastPosY = 0;
			Point2F plotPoint;

			S32 size = 32;

			for (S32 sample = 0; sample < mPlots[k].mGraphData.size(); sample++)
			{
				S32 temp;

				temp = (S32)(((F32)getExtent().x / (F32)mPlots[k].mGraphData.size()) * (F32)sample);

				// calculate the point positions
            plotPoint = getPlotPoint(k, sample);

				posX = (S32)((plotPoint.x - mPlots[k].mGraphMin.x) * (ScaleX /(1.00 + mPlots[k].mGraphScale)));
				posY = (getExtent().y) - (S32)((plotPoint.y  - mPlots[k].mGraphMin.y) * ScaleY);

            posX += getExtent().x * (mPlots[k].mGraphScale);
				posY /= (1.00 + (mPlots[k].mGraphScale));

            posX = localToGlobalCoord(Point2I(posX, posY)).x;
				posY = localToGlobalCoord(Point2I(posX, posY)).y;

				// check if this isn't our first loop through, if it is we won't have starting points
            if(sample > 0)
				{
					pDrawUtil->drawLine( lastPosX, lastPosY , posX, posY , mPlots[k].mGraphColor );
				} else
				{
               mPlots[k].mNutList.clear();
				}

            mPlots[k].mNutList.push_back( Point2F(posX, posY) );

				// store the last positions to be the starting points drawn into a line next loop
				lastPosX = posX;
				lastPosY = posY;

            //Con::printf("red %f green %f blue %f", mPlots[k].mGraphColor.red, mPlots[k].mGraphColor.green, mPlots[k].mGraphColor.blue);



				if(mSelectedPoint != -1)
				{
				   mLastSelectedPoint = mSelectedPoint;
				}

            ColorI nutColor (mPlots[k].mGraphColor);

				if(k == mSelectedPlot && sample == mLastSelectedPoint)
            {
               // grab the colors for the nut
               F32 red = mPlots[k].mGraphColor.red;
               F32 green = mPlots[k].mGraphColor.green;
               F32 blue = mPlots[k].mGraphColor.blue;

               // invert the colors for the nut since this is a selected nut
               red = 1.0 - red; 
               green = 1.0 - green;
               blue = 1.0 - blue;
               // nut color

               nutColor = ColorI(ColorF(red, green, blue));
            } 

            // draw the seleciton nut
				drawNut( Point2I(posX, posY), 3, mOutlineColor, nutColor );

				if((mLastSelectedPoint != -1) || (mRenderAllPoints == true))
				{
               if((k == mSelectedPlot && sample == mLastSelectedPoint) || (mRenderAllPoints == true))
				   {
                  char number[32];

					   Point2I comparePos = localToGlobalCoord(Point2I(getPosition().x, getPosition().y));

                  dSprintf(number, 32, "%4.3f %4.3f", plotPoint.x, plotPoint.y);

                  S32 textWidth = (S32)font->getStrWidth((const UTF8*)number);;
				      textWidth /= 2;

				      if((((S32)posX - (textWidth/2)) < comparePos.x) || (((S32)posX - textWidth) <= 0))
				      {
                      posX += (textWidth/1.5);
					  } else if((posX + (textWidth * 1.8)) > (comparePos.x + getExtent().x) || ((posX + textWidth) >= videoMode.resolution.x))
				      {
                      posX -= (textWidth * 1.5);
				      }

				      if((((S32)posY) < comparePos.y) || (((S32)posY - textWidth) <= 0))
				      {
                      posY += 40;
				      } 
				
					  pDrawUtil->setBitmapModulation( profile->mFontColor );
					  pDrawUtil->drawText( font, Point2I(posX, posY + 5) - Point2I(size >> 1, size), number );
					  pDrawUtil->clearBitmapModulation();
				   }
				}
			}
		}
	}

	if(mRenderNextGraphTooltip == true && mRenderGraphTooltip == true)
	{
       char argBuffer[1][32];
   
       dSprintf(argBuffer[0], 32, "%s", getGraphName(mTooltipSelectedPlot));

	   renderGraphTooltip(mCursorPos, argBuffer[0]);
	}
}

S32 GuiParticleGraphCtrl::addPlotPoint(S32 plotID, Point2F v, bool setAdded)
{
   S32 mPlotIndex = 0;
   bool plotChanged = false;
   bool plotAdded = false;

   if(mAutoMax == false)
   {
      if(v.x < mPlots[plotID].mGraphMin.x)
      {
         v.x = mPlots[plotID].mGraphMin.x;
      } 

      if(v.x > mPlots[plotID].mGraphMax.x)
      {
         v.x = mPlots[plotID].mGraphMax.x;
      }

      if(v.y < mPlots[plotID].mGraphMin.y)
      {
         v.y = mPlots[plotID].mGraphMin.y;
      } 

      if(v.y > mPlots[plotID].mGraphMax.y)
      {
         v.y = mPlots[plotID].mGraphMax.y;
      }
   }

   for(S32 i = 0; i < mPlots[plotID].mGraphData.size(); i++)
   {         
	   if(mFabs(v.x - mPlots[plotID].mGraphData[i].x) < 0.001)
	   {
	      if(mAutoRemove == true)
		   {
            changePlotPoint(plotID, i, v);
			   plotChanged = true;
			   mPlotIndex = i;
		   } else
		   {
            mPlotIndex = -1;
		   }
			
		   plotAdded = true;
		  
		   break;
	   } else if(v.x < mPlots[plotID].mGraphData[i].x)
	   {
          insertPlotPoint(plotID, i, v); 
		  plotAdded = true;
		  mPlotIndex = i;
		  break;
	   }
   }

   if(plotAdded == false)
   {
	   mPlots[plotID].mGraphData.push_back( v );
	   mPlotIndex = mPlots[plotID].mGraphData.size() - 1;
   }

   if(mAutoMax == true)
   {
      // Keep record of maximum data value for scaling purposes.
      if(v.y > mPlots[plotID].mGraphMax.y)
	     mPlots[plotID].mGraphMax.y = v.y;

      if(v.x > mPlots[plotID].mGraphMax.x)
	     mPlots[plotID].mGraphMax.x = v.x;
   }

   if(plotChanged == true)
   {
      mPointWasAdded = false;  
   } else if(mPlotIndex != -1 && setAdded)
   {
      mPointWasAdded = true;
	   mAddedPoint = v;
	   mAddedPointIndex = mPlotIndex;
   }

   return mPlotIndex;
}

void GuiParticleGraphCtrl::insertPlotPoint(S32 plotID, S32 i, Point2F v)
{
   //AssertFatal(plotID > -1 && plotID < MaxPlots, "Invalid plot specified!");

   // Add the data and trim the vector...
   mPlots[plotID].mGraphData.insert( i );
   mPlots[plotID].mGraphData[ i ] = v;

   if(mAutoMax == true)
   {
      // Keep record of maximum data value for scaling purposes.
      if(v.y > mPlots[plotID].mGraphMax.y)
	     mPlots[plotID].mGraphMax.y = v.y;

      if(v.x > mPlots[plotID].mGraphMax.x)
	     mPlots[plotID].mGraphMax.x = v.x;
   }

   // Argument Buffer.
   char argBuffer[2][32];
   
   dSprintf(argBuffer[0], 32, "%d", plotID);
   dSprintf(argBuffer[1], 32, "%f %f", v.x, v.y);

   // Call Scripts.
   Con::executef(this, "onPlotPointInserted", argBuffer[0], argBuffer[1]);
}

S32 GuiParticleGraphCtrl::changePlotPoint(S32 plotID, S32 i, Point2F v)
{
   //AssertFatal(plotID > -1 && plotID < MaxPlots, "Invalid plot specified!");

   // Add the data and trim the vector...
   S32 point = removePlotPoint(plotID, i);

   // Argument Buffer.
   char argBuffer[3][32];
   
   dSprintf(argBuffer[0], 32, "%d", mSelectedPlot);
   dSprintf(argBuffer[1], 32, "%d", point);
   dSprintf(argBuffer[2], 32, "%f %f", v.x, v.y);

   // Call Scripts.
   Con::executef(this, "onPlotPointRemoved", argBuffer[0], argBuffer[1], argBuffer[2]);
   
   // call the insert function
   S32 index = addPlotPoint(plotID, v);

   return index;
}

S32 GuiParticleGraphCtrl::removePlotPoint(S32 plotID, S32 i)
{
   if((plotID < MaxPlots && plotID >= 0) && (i < mPlots[plotID].mGraphData.size()))
   {
      Point2F v = mPlots[plotID].mGraphData[i];

      mPlots[plotID].mGraphData.erase(i);

      if(i == mPlots[plotID].mGraphData.size() && mSelectedPlot == plotID && mLastSelectedPoint == i)
      {
         setSelectedPoint(i-1);
      } else if(i < mLastSelectedPoint)
      {
         setSelectedPoint(mLastSelectedPoint-1);
      }
   }

   return i;
}

S32 GuiParticleGraphCtrl::getSelectedPlot()
{
   return mSelectedPlot;
}

S32 GuiParticleGraphCtrl::getSelectedPoint()
{
   return mLastSelectedPoint;
}

bool GuiParticleGraphCtrl::isExistingPoint(S32 plotID, S32 sample)
{
   if (((plotID < 0) || (plotID > MaxPlots)) || ((sample < 0) || (sample > MaxDataPoints)) || (sample >= mPlots[plotID].mGraphData.size()))
   {
      return false;
   } else
   {
      return true;
   }
}

Point2F GuiParticleGraphCtrl::getPlotPoint(S32 plotID, S32 sample)
{
   Point2F val;
   val.x = -1;
   val.y = -1;

   if (((plotID < 0) || (plotID > MaxPlots)) || ((sample < 0) || (sample > MaxDataPoints)))
   {
      return val;
   }

   return mPlots[plotID].mGraphData[sample];
}

S32 GuiParticleGraphCtrl::getPlotIndex(S32 plotID, F32 x, F32 y)
{
   if ((plotID < 0) || (plotID > MaxPlots))
   {
      return -1;
   } 

   F32 compareX = 0.0;
   F32 compareY = 0.0;

   for(S32 i = 0; i < mPlots[plotID].mGraphData.size(); i++)
   {
      compareX = mPlots[plotID].mGraphData[i].x;
      compareY = mPlots[plotID].mGraphData[i].y;

      //
	   //if((x == compareX) && (y == compareY))
      if((mFabs(x - compareX) < 0.001) && (mFabs(y - compareY) < 0.001))
         return i;
   }

   return -1;
}

void GuiParticleGraphCtrl::setGraphType(S32 plotID, const char *graphType)
{
	AssertFatal(plotID > -1 && plotID < MaxPlots, "Invalid plot specified!");
	if(!dStricmp(graphType,"Bar"))
		mPlots[plotID].mGraphType = Bar;
	else if(!dStricmp(graphType,"Filled"))
		mPlots[plotID].mGraphType = Filled;
	else if(!dStricmp(graphType,"Point"))
		mPlots[plotID].mGraphType = Point;
	else if(!dStricmp(graphType,"Polyline"))
		mPlots[plotID].mGraphType = Polyline;
	else AssertWarn(true, "Invalid graph type!");
}

void GuiParticleGraphCtrl::setSelectedPlot(S32 plotID)
{
   mSelectedPlot = plotID;

   // Argument Buffer.
   char argBuffer[32];
   
   dSprintf(argBuffer, 32, "%d", plotID);

   // Call Scripts.
   Con::executef(this, "onSetSelected", argBuffer);
}

void GuiParticleGraphCtrl::setSelectedPoint(S32 point)
{
   if(point != mSelectedPoint && point < mPlots[mSelectedPlot].mGraphData.size() && point >= 0)
   {
      mSelectedPoint = point;
      mLastSelectedPoint = point;
      char argBuffer[32];

      dSprintf(argBuffer, 32, "%d", point);

      // Call Scripts.
      Con::executef(this, "onPlotPointSelected", argBuffer);
   }
}

void GuiParticleGraphCtrl::resetSelectedPoint()
{
   mSelectedPoint = -1;
}

void GuiParticleGraphCtrl::setAutoGraphMax(bool autoMax)
{
   mAutoMax = autoMax;
}

void GuiParticleGraphCtrl::setAutoRemove(bool autoRemove)
{
   mAutoRemove = autoRemove;
}

void GuiParticleGraphCtrl::setGraphHidden(S32 plotID, bool isHidden)
{
   mPlots[plotID].mHidden = isHidden;
}

void GuiParticleGraphCtrl::setRenderAll(bool renderAll)
{
   mRenderAllPoints = renderAll;
}

void GuiParticleGraphCtrl::setPointXMovementClamped(bool clamped)
{
   mPointXMovementClamped = clamped;
}

void GuiParticleGraphCtrl::setRenderGraphTooltip(bool renderGraphTooltip)
{
   mRenderGraphTooltip = renderGraphTooltip;
}


void GuiParticleGraphCtrl::drawNut(const Point2I &nut, S32 size, ColorI &outlineColor, ColorI &nutColor)
{
	// Fetch Draw Utility.
	GFXDrawUtil* pDrawUtil = GFX->getDrawUtil();

   //Con::printf("r %d g %d b %d", nutColor.red, nutColor.green, nutColor.blue);
   S32 NUT_SIZE = size;
   RectI r(nut.x - NUT_SIZE, nut.y - NUT_SIZE, 2 * NUT_SIZE + 1, 2 * NUT_SIZE + 1);
   r.inset(1, 1);
   pDrawUtil->drawRectFill(r, nutColor);
   r.inset(-1, -1);
   pDrawUtil->drawRect(r, outlineColor);
}

bool GuiParticleGraphCtrl::inNut(const Point2I &pt, S32 x, S32 y)
{
   S32 dx = pt.x - x;
   S32 dy = pt.y - y;
   return dx <= mVertexClickSize && dx >= -mVertexClickSize && dy <= mVertexClickSize && dy >= -mVertexClickSize;
}

Point2I GuiParticleGraphCtrl::findHitNut( Point2I hitPoint )
{
   for(S32 i = 0; i < MaxPlots; i++)
   {
	  if ( (mPlots[i].mGraphData.size() == 0) || (mPlots[i].mHidden == true))
         continue;

      for (S32 j = 0 ; j < mPlots[i].mNutList.size(); j++ )
      {
	     if( inNut (Point2I( mPlots[i].mNutList[j].x, mPlots[i].mNutList[j].y), hitPoint.x, hitPoint.y) )
		 {
            mTooltipSelectedPlot = i; 
			return Point2I(i,j);
		 }
	  }
   }

   return Point2I(-1,-1);
}

Point2F GuiParticleGraphCtrl::convertToGraphCoord(S32 plotID, Point2I v)
{
   Point2F resultV;

   v = globalToLocalCoord( v );
   v.y = getExtent().y - v.y;
   resultV = Point2F(v.x, v.y);
   resultV.x /= getExtent().x; 
   resultV.y /= getExtent().y;
   resultV.x *= getGraphExtent(plotID).x;
   resultV.y *= getGraphExtent(plotID).y;
   resultV.x *= 1.00 + (mPlots[plotID].mGraphScale*2);
   resultV.y *= 1.00 + (mPlots[plotID].mGraphScale*2);
   resultV.x -= getGraphExtent(plotID).x * ((mPlots[plotID].mGraphScale));
   resultV.y -= getGraphExtent(plotID).y * ((mPlots[plotID].mGraphScale));
   resultV.x += mPlots[plotID].mGraphMin.x;
   resultV.y += mPlots[plotID].mGraphMin.y;

   if(mAutoMax == false)
   {
      if(resultV.x < mPlots[plotID].mGraphMin.x)
      {
         resultV.x = mPlots[plotID].mGraphMin.x;
      } 

      if(resultV.x > mPlots[plotID].mGraphMax.x)
      {
         resultV.x = mPlots[plotID].mGraphMax.x;
      }

      if(resultV.y < mPlots[plotID].mGraphMin.y)
      {
         resultV.y = mPlots[plotID].mGraphMin.y;
      } 

      if(resultV.y > mPlots[plotID].mGraphMax.y)
      {
         resultV.y = mPlots[plotID].mGraphMax.y;
      }
   }



   return resultV;
}

Point2F GuiParticleGraphCtrl::getGraphExtent(S32 plotID)
{
   Point2F resultV;

   resultV.x = mPlots[plotID].mGraphMax.x - mPlots[plotID].mGraphMin.x;
   resultV.y = mPlots[plotID].mGraphMax.y - mPlots[plotID].mGraphMin.y;

   return resultV;
}

ColorF GuiParticleGraphCtrl::getGraphColor(S32 plotID)
{
   return mPlots[plotID].mGraphColor;
}

Point2F GuiParticleGraphCtrl::getGraphMax(S32 plotID)
{
   return mPlots[plotID].mGraphMax;
}

Point2F GuiParticleGraphCtrl::getGraphMin(S32 plotID)
{
   return mPlots[plotID].mGraphMin;
}

void GuiParticleGraphCtrl::setGraphMin(S32 plotID, Point2F graphMin)
{
   mPlots[plotID].mGraphMin = graphMin;
}

void GuiParticleGraphCtrl::setGraphMax(S32 plotID, Point2F graphMax)
{
   mPlots[plotID].mGraphMax = graphMax;
}

void GuiParticleGraphCtrl::setGraphMinX(S32 plotID, F32 graphMinX)
{
   mPlots[plotID].mGraphMin.x = graphMinX;
}

void GuiParticleGraphCtrl::setGraphMinY(S32 plotID, F32 graphMinY)
{
   mPlots[plotID].mGraphMin.y = graphMinY;
}

void GuiParticleGraphCtrl::setGraphMaxX(S32 plotID, F32 graphMaxX)
{
   mPlots[plotID].mGraphMax.x = graphMaxX;
}

void GuiParticleGraphCtrl::setGraphMaxY(S32 plotID, F32 graphMaxY)
{
   mPlots[plotID].mGraphMax.y = graphMaxY;
}

void GuiParticleGraphCtrl::setGraphName(S32 plotID, StringTableEntry graphName)
{
   mPlots[plotID].mGraphName = StringTable->insert(graphName);
}

StringTableEntry GuiParticleGraphCtrl::getGraphName(S32 plotID)
{
   return mPlots[plotID].mGraphName;
}

void GuiParticleGraphCtrl::onMouseMove(const GuiEvent &event)
{
   mCursorPos = event.mousePoint;
	
   Point2I hitNut = findHitNut(event.mousePoint);

   if( hitNut != Point2I(-1,-1) )
   {
      mRenderNextGraphTooltip = true;
   } else
   {
      mRenderNextGraphTooltip = false;
   }

   // Argument Buffer.
   char argBuffer[32];
   
   dSprintf(argBuffer, 32, "%f %f", convertToGraphCoord(mSelectedPlot, event.mousePoint).x, convertToGraphCoord(mSelectedPlot, event.mousePoint).y);


   // Call Scripts.
   Con::executef(this, "onMouseMove", argBuffer);
}

void GuiParticleGraphCtrl::onMouseDown(const GuiEvent &event)
{
   Point2I hitNut = findHitNut(event.mousePoint);

   if( hitNut != Point2I(-1,-1) )
   {
	  if(event.mouseClickCount == 2)
	  {
        Point2F plotPoint = getPlotPoint(hitNut.x, hitNut.y);
		  Point2F mousePos = convertToGraphCoord(mSelectedPlot, event.mousePoint);
        S32 point = removePlotPoint(hitNut.x, hitNut.y);

        // Argument Buffer.
        char argBuffer[3][32];
   
        dSprintf(argBuffer[0], 32, "%d", mSelectedPlot);
        dSprintf(argBuffer[1], 32, "%d", point);
		  dSprintf(argBuffer[2], 32, "%f %f", plotPoint.x, plotPoint.y);

		 

         // Call Scripts.
         Con::executef(this, "onPlotPointRemoved", argBuffer[0], argBuffer[1], argBuffer[2]);
	  } else
	  {
	     setSelectedPlot(hitNut.x);
        setSelectedPoint(hitNut.y);
	     mOriginalSelectedPoint = hitNut.y;

		  char argBuffer[32];

        dSprintf(argBuffer, 32, "%d", hitNut.y);

        // Call Scripts.
        Con::executef(this, "onPlotPointSelectedMouseDown", argBuffer);
	  }
   } else if( mSelectedPlot != -1 ) 
   {  
	   Point2F mousePos = convertToGraphCoord(mSelectedPlot, event.mousePoint);
	   mLastSelectedPoint = addPlotPoint(mSelectedPlot, mousePos);

	   // Argument Buffer.
      char argBuffer[32];
   
      dSprintf(argBuffer, 32, "%f %f", convertToGraphCoord(mSelectedPlot, event.mousePoint).x, convertToGraphCoord(mSelectedPlot, event.mousePoint).y);

      // Call Scripts.
      Con::executef(this, "onMouseDragged", argBuffer);

	   return;
   }
}

void GuiParticleGraphCtrl::onMouseUp(const GuiEvent &event)
{
   if(mSelectedPoint != -1)
	  mLastSelectedPoint = mSelectedPoint;

   if(mPointWasAdded == true)
   {
      if(mSelectedPoint == -1)
	   {
	     // Argument Buffer.
         char argBuffer[3][32];
   
         dSprintf(argBuffer[0], 32, "%d", mSelectedPlot);
         dSprintf(argBuffer[1], 32, "%f %f", mAddedPoint.x, mAddedPoint.y);
		   dSprintf(argBuffer[2], 32, "%d", mAddedPointIndex);
         
         // Call Scripts.
         Con::executef(this, "onPlotPointAdded", argBuffer[0], argBuffer[1], argBuffer[2]);
	   } else
	   {
         // Argument Buffer.
         char argBuffer[4][32];
   
         dSprintf(argBuffer[0], 32, "%d", mSelectedPlot);
         dSprintf(argBuffer[1], 32, "%f %f", mAddedPoint.x, mAddedPoint.y);
		   dSprintf(argBuffer[2], 32, "%d", mOriginalSelectedPoint);
		   dSprintf(argBuffer[3], 32, "%d", mAddedPointIndex);

         // Call Scripts.
         Con::executef(this, "onPlotPointChangedUp", argBuffer[0], argBuffer[1], argBuffer[2], argBuffer[3]);
	  }
   }

   mPointWasAdded = false;

   mSelectedPoint = -1;
}

void GuiParticleGraphCtrl::onMouseDragged(const GuiEvent &event)
{
   mRenderNextGraphTooltip = false;

   if(mSelectedPoint != -1)
   {
	   Point2F mousePos = convertToGraphCoord(mSelectedPlot, event.mousePoint);

      if(mPointXMovementClamped == true)
	   {
         F32 prevXPos = getPlotPoint(mSelectedPlot, mSelectedPoint).x;
		   if(mousePos.x != prevXPos)
		   {
		      mousePos.x = prevXPos;
		   }
	   }

	   removePlotPoint(mSelectedPlot, mSelectedPoint);
	   S32 point = addPlotPoint(mSelectedPlot, mousePos);
      if(point != -1)
	   {
         setSelectedPoint(point);
		   mLastMousePos = mousePos;

         // Argument Buffer.
         char argBuffer[3][32];
   
         dSprintf(argBuffer[0], 32, "%d", mSelectedPlot);
         dSprintf(argBuffer[1], 32, "%f %f", mAddedPoint.x, mAddedPoint.y);
	      dSprintf(argBuffer[2], 32, "%d", point);

         // Call Scripts.
         Con::executef(this, "onPlotPointChangedMove", argBuffer[0], argBuffer[1], argBuffer[2]);
	   } else
	   {
         point = addPlotPoint(mSelectedPlot, mLastMousePos);
	   }
   }

   // Argument Buffer.
   char argBuffer[32];
   
   dSprintf(argBuffer, 32, "%f %f", convertToGraphCoord(mSelectedPlot, event.mousePoint).x, convertToGraphCoord(mSelectedPlot, event.mousePoint).y);

   // Call Scripts.
   Con::executef(this, "onMouseDragged", argBuffer);
}

void GuiParticleGraphCtrl::onRightMouseDown(const GuiEvent &event)
{
   Point2I hitNut = findHitNut(event.mousePoint);

   if( hitNut != Point2I(-1,-1) )
   {
      Point2F plotPoint = getPlotPoint(hitNut.x, hitNut.y);

      Point2F mousePos = convertToGraphCoord(mSelectedPlot, event.mousePoint);
      S32 point = removePlotPoint(hitNut.x, hitNut.y);

      // Argument Buffer.
      char argBuffer[3][32];
   
      dSprintf(argBuffer[0], 32, "%d", mSelectedPlot);
      dSprintf(argBuffer[1], 32, "%d", point);
	   dSprintf(argBuffer[2], 32, "%f %f", plotPoint.x, plotPoint.y);

      // Call Scripts.
      Con::executef(this, "onPlotPointRemoved", argBuffer[0], argBuffer[1], argBuffer[2]);
   } 
}

void GuiParticleGraphCtrl::onRightMouseUp(const GuiEvent &event)
{

}

void GuiParticleGraphCtrl::onRightMouseDragged(const GuiEvent &event)
{
   Point2I hitNut = findHitNut(event.mousePoint);

   if( hitNut != Point2I(-1,-1) )
   {
      Point2F plotPoint = getPlotPoint(hitNut.x, hitNut.y);

      Point2F mousePos = convertToGraphCoord(mSelectedPlot, event.mousePoint);
      S32 point = removePlotPoint(hitNut.x, hitNut.y);

	   // Argument Buffer.
      char argBuffer[3][32];
   
      dSprintf(argBuffer[0], 32, "%d", mSelectedPlot);
      dSprintf(argBuffer[1], 32, "%d", point);
	   dSprintf(argBuffer[2], 32, "%f %f", plotPoint.x, plotPoint.y);

      // Call Scripts.
      Con::executef(this, "onPlotPointRemoved", argBuffer[0], argBuffer[1], argBuffer[2]);
   } 
}

void GuiParticleGraphCtrl::clearAllGraphs()
{
   for(S32 i = 0;i < MaxPlots;i++)
   {
      clearGraph(i);
   }
}

void GuiParticleGraphCtrl::clearGraph(S32 plotID)
{
   mPlots[plotID].mGraphData.clear();
   mPlots[plotID].mNutList.clear();
}

bool GuiParticleGraphCtrl::renderGraphTooltip(Point2I cursorPos, StringTableEntry tooltip)
{
   // Short Circuit.
   if (!mAwake) 
      return false;
   if ( dStrlen( tooltip ) == 0 )
      return false;

   // Need to have root.
   GuiCanvas *root = getRoot();
   if ( !root )
      return false;

   if (!mTooltipProfile)
      setTooltipProfile( mProfile );

   GFont *font = mTooltipProfile->mFont;

   // Fetch Canvas.
   GuiCanvas *pCanvas = getRoot();

   //Vars used:
   //Screensize (for position check)
   //Offset to get position of cursor
   //textBounds for text extent.
   Point2I screensize = pCanvas->getWindowSize();
   Point2I offset = cursorPos; 
   Point2I textBounds;
   S32 textWidth = font->getStrWidth(tooltip);

   //Offset below cursor image
   offset.y -= root->getCursorExtent().y;
   //Create text bounds.
   textBounds.x = textWidth+8;
   textBounds.y = font->getHeight() + 4;

   // Check position/width to make sure all of the tooltip will be rendered
   // 5 is given as a buffer against the edge
   if (screensize.x < offset.x + textBounds.x + 5)
      offset.x = screensize.x - textBounds.x - 5;

   // And ditto for the height
   if(screensize.y < offset.y + textBounds.y + 5)
      offset.y = cursorPos.y - textBounds.y - 5;

   // Set rectangle for the box, and set the clip rectangle.
   RectI rect(offset, textBounds);
   GFX->setClipRect(rect);

	// Fetch Draw Utility.
	GFXDrawUtil* pDrawUtil = GFX->getDrawUtil();

   // Draw Filler bit, then border on top of that
	pDrawUtil->drawRectFill(rect, ColorI(mTooltipProfile->mFillColor.red, mTooltipProfile->mFillColor.green, mTooltipProfile->mFillColor.blue, 200) );
	pDrawUtil->drawRect( rect, mTooltipProfile->mBorderColor );

   // Draw the text centered in the tool tip box
   pDrawUtil->setBitmapModulation( mTooltipProfile->mFontColor );
   Point2I start;
   start.set( ( textBounds.x - textWidth) / 2, ( textBounds.y - font->getHeight() ) / 2 );
   pDrawUtil->drawText( font, start + offset, tooltip, mProfile->mFontColors );
   pDrawUtil->clearBitmapModulation();

   return true;
}

ConsoleMethod(GuiParticleGraphCtrl, setSelectedPoint, void, 3, 3, "(int point)"
              "Set the selected point on the graph.\n"
			  "@return No return value")
{
   S32 point = dAtoi(argv[2]);
   if(point >= object->mPlots[object->mSelectedPlot].mGraphData.size() || point < 0)
   {
	   Con::errorf("Invalid point to select.");
	   return;
   }
   object->setSelectedPoint( point );
}

ConsoleMethod(GuiParticleGraphCtrl, setSelectedPlot, void, 3, 3, "(int plotID)"
              "Set the selected plot (a.k.a. graph)."
			  "@return No return value" )
{
   S32 plotID = dAtoi(argv[2]);
   if(plotID > object->MaxPlots)
   {
	   Con::errorf("Invalid plotID.");
	   return;
   }
   object->setSelectedPlot( plotID );
}

ConsoleMethod(GuiParticleGraphCtrl, clearGraph, void, 3, 3, "(int plotID)"
              "Clear the graph of the given plot."
			  "@return No return value")
{
   S32 plotID = dAtoi(argv[2]);
   if(plotID > object->MaxPlots)
   {
	   Con::errorf("Invalid plotID.");
	   return;
   }
   object->clearGraph( plotID );
}

ConsoleMethod(GuiParticleGraphCtrl, clearAllGraphs, void, 2, 2, "()"
              "Clear all of the graphs."
			  "@return No return value")
{
   object->clearAllGraphs();
}

ConsoleMethod(GuiParticleGraphCtrl, addPlotPoint, const char*, 5, 6, "(int plotID, float x, float y, bool setAdded = true;)"
              "Add a data point to the given plot."
			  "@return")
{
   S32 plotID = dAtoi(argv[2]);
   S32 pointAdded = 0;
   char *retBuffer = Con::getReturnBuffer(32);

   if(plotID > object->MaxPlots)
   {
	   Con::errorf("Invalid plotID.");
	   dSprintf(retBuffer, 32, "%d", -2);
      return retBuffer;
   }

   if(argc == 5)
   {
      pointAdded = object->addPlotPoint( plotID, Point2F(dAtof(argv[3]), dAtof(argv[4])));
   } else if(argc == 6)
   {
      pointAdded = object->addPlotPoint( plotID, Point2F(dAtof(argv[3]), dAtof(argv[4])), dAtob(argv[5]));
   }

   
   dSprintf(retBuffer, 32, "%d", pointAdded);

   return retBuffer;
}

ConsoleMethod(GuiParticleGraphCtrl, insertPlotPoint, void, 6, 6, "(int plotID, int i, float x, float y)\n"
              "Insert a data point to the given plot and plot position.\n"
			  "@param plotID The plot you want to access\n"
			  "@param i The data point.\n"
			  "@param x,y The plot position.\n"
			  "@return No return value.")
{
   S32 plotID = dAtoi(argv[2]);
   if(plotID > object->MaxPlots)
   {
	   Con::errorf("Invalid plotID.");
	   return;
   }
   object->insertPlotPoint( plotID, dAtoi(argv[3]), Point2F(dAtof(argv[4]), dAtof(argv[5])));
}

ConsoleMethod(GuiParticleGraphCtrl, changePlotPoint, const char*, 6, 6, "(int plotID, int i, float x, float y)"
              "Change a data point to the given plot and plot position.\n"
			  "@param plotID The plot you want to access\n"
			  "@param i The data point.\n"
			  "@param x,y The plot position.\n"
			  "@return No return value.")
{
   S32 plotID = dAtoi(argv[2]);
   if(plotID > object->MaxPlots)
   {
	   Con::errorf("Invalid plotID.");

      char *retBuffer = Con::getReturnBuffer(64);
      const S32 index = -1;
      dSprintf(retBuffer, 64, "%d", index);
      return retBuffer;
   }

   char *retBuffer = Con::getReturnBuffer(64);
   const S32 index = object->changePlotPoint( plotID, dAtoi(argv[3]), Point2F(dAtof(argv[4]), dAtof(argv[5])));
   dSprintf(retBuffer, 64, "%d", index);
   return retBuffer;
}

ConsoleMethod(GuiParticleGraphCtrl, getSelectedPlot, const char*, 2, 2, "() "
              "Gets the selected Plot (a.k.a. graph).\n"
			  "@return The plot's ID.")
{
   char *retBuffer = Con::getReturnBuffer(32);
   const S32 plot = object->getSelectedPlot();
   dSprintf(retBuffer, 32, "%d", plot);
   return retBuffer;
}

ConsoleMethod(GuiParticleGraphCtrl, getSelectedPoint, const char*, 2, 2, "()"
              "Gets the selected Point on the Plot (a.k.a. graph)."
			  "@return The last selected point ID")
{
   char *retBuffer = Con::getReturnBuffer(32);
   const S32 point = object->getSelectedPoint();
   dSprintf(retBuffer, 32, "%d", point);
   return retBuffer;
}

ConsoleMethod(GuiParticleGraphCtrl, isExistingPoint, const char*, 4, 4, "(int plotID, int samples)"
              "@return Returns true or false whether or not the point in the plot passed is an existing point.")
{
   S32 plotID = dAtoi(argv[2]);
   S32 samples = dAtoi(argv[3]);

   if(plotID > object->MaxPlots)
   {
	   Con::errorf("Invalid plotID.");
   }
   if(samples > object->MaxDataPoints)
   {
	   Con::errorf("Invalid sample.");
   }

   char *retBuffer = Con::getReturnBuffer(32);
   const bool isPoint = object->isExistingPoint(plotID, samples);
   dSprintf(retBuffer, 32, "%d", isPoint);
   return retBuffer;
}

ConsoleMethod(GuiParticleGraphCtrl, getPlotPoint, const char*, 4, 4, "(int plotID, int samples)"
              "Get a data point from the plot specified, samples from the start of the graph."
			  "@return The data point ID")
{
   S32 plotID = dAtoi(argv[2]);
   S32 samples = dAtoi(argv[3]);

   if(plotID > object->MaxPlots)
   {
	   Con::errorf("Invalid plotID.");
   }
   if(samples > object->MaxDataPoints)
   {
	   Con::errorf("Invalid sample.");
   }

   char *retBuffer = Con::getReturnBuffer(64);
   const Point2F &pos = object->getPlotPoint(plotID, samples);
   dSprintf(retBuffer, 64, "%f %f", pos.x, pos.y);
   return retBuffer;
}

ConsoleMethod(GuiParticleGraphCtrl, getPlotIndex, const char*, 5, 5, "(int plotID, float x, float y)\n"
              "Gets the index of the point passed on the plotID passed (graph ID).\n"
			  "@param plotID The plot you wish to check.\n"
			  "@param x,y The coordinates of the point to get.\n"
			  "@return Returns the index of the point.\n")
{
   S32 plotID = dAtoi(argv[2]);
   F32 x = dAtof(argv[3]);
   F32 y = dAtof(argv[4]);

   if(plotID > object->MaxPlots)
   {
	   Con::errorf("Invalid plotID.");
   }

   char *retBuffer = Con::getReturnBuffer(32);
   const S32 &index = object->getPlotIndex(plotID, x, y);
   dSprintf(retBuffer, 32, "%d", index);
   return retBuffer;
}

ConsoleMethod(GuiParticleGraphCtrl, getGraphColor, const char*, 3, 3, "(int plotID)"
              "Get the color of the graph passed."
			  "@return Returns the color of the graph as a string of RGB values formatted as \"R G B\"")
{
   S32 plotID = dAtoi(argv[2]);

   if(plotID > object->MaxPlots)
   {
	   Con::errorf("Invalid plotID.");
   }

   char *retBuffer = Con::getReturnBuffer(64);
   const ColorF &color = object->getGraphColor(plotID);
   dSprintf(retBuffer, 64, "%f %f %f", color.red, color.green, color.blue);
   return retBuffer;
}

ConsoleMethod(GuiParticleGraphCtrl, getGraphMin, const char*, 3, 3, "(int plotID) "
              "Get the minimum values of the graph ranges.\n"
			  "@return Returns the minimum of the range formatted as \"x-min y-min\"")
{
   S32 plotID = dAtoi(argv[2]);

   if(plotID > object->MaxPlots)
   {
	   Con::errorf("Invalid plotID.");
   }

   char *retBuffer = Con::getReturnBuffer(64);
   const Point2F graphMin = object->getGraphMin(plotID);
   dSprintf(retBuffer, 64, "%f %f", graphMin.x, graphMin.y);
   return retBuffer;
}

ConsoleMethod(GuiParticleGraphCtrl, getGraphMax, const char*, 3, 3, "(int plotID) "
			  "Get the maximum values of the graph ranges.\n"
			  "@return Returns the maximum of the range formatted as \"x-max y-max\"")
{
   S32 plotID = dAtoi(argv[2]);

   if(plotID > object->MaxPlots)
   {
	   Con::errorf("Invalid plotID.");
   }

   char *retBuffer = Con::getReturnBuffer(64);
   const Point2F graphMax = object->getGraphMax(plotID);
   dSprintf(retBuffer, 64, "%f %f", graphMax.x, graphMax.y);
   return retBuffer;
}

ConsoleMethod(GuiParticleGraphCtrl, getGraphName, const char*, 3, 3, "(int plotID) "
              "Get the name of the graph passed.\n"
			  "@return Returns the name of the plot")
{
   S32 plotID = dAtoi(argv[2]);

   if(plotID > object->MaxPlots)
   {
	   Con::errorf("Invalid plotID.");
   }

   char *retBuffer = Con::getReturnBuffer(64);
   const StringTableEntry graphName = object->getGraphName(plotID);
   dSprintf(retBuffer, 64, "%s", graphName);
   return retBuffer;
}

ConsoleMethod(GuiParticleGraphCtrl, setGraphMin, void, 5, 5, "(int plotID, float minX, float minY) "
			  "Set the min values of the graph of plotID.\n"
			  "@param plotID The plot to modify\n"
			  "@param minX,minY The minimum bound of the value range.\n"
			  "@return No return value.")
{
	S32 plotID = dAtoi(argv[2]);

	if(plotID > object->MaxPlots)
	{
		Con::errorf("Invalid plotID.");
		return;
	}

	object->setGraphMin(dAtoi(argv[2]), Point2F(dAtof(argv[3]), dAtof(argv[4])));
}

ConsoleMethod(GuiParticleGraphCtrl, setGraphMinX, void, 4, 4, "(int plotID, float minX) "
			  "Set the min X value of the graph of plotID.\n"
			  "@param plotID The plot to modify.\n"
			  "@param minX The minimum x value.\n"
			  "@return No return Value.")
{
	S32 plotID = dAtoi(argv[2]);

	if(plotID > object->MaxPlots)
	{
		Con::errorf("Invalid plotID.");
		return;
	}

	object->setGraphMinX(dAtoi(argv[2]), dAtof(argv[3]));
}

ConsoleMethod(GuiParticleGraphCtrl, setGraphMinY, void, 4, 4, "(int plotID, float minY) "
			  "Set the min Y value of the graph of plotID."
			  "@param plotID The plot to modify.\n"
			  "@param minY The minimum y value.\n"
			  "@return No return Value.")
{
	S32 plotID = dAtoi(argv[2]);

	if(plotID > object->MaxPlots)
	{
		Con::errorf("Invalid plotID.");
		return;
	}

	object->setGraphMinY(dAtoi(argv[2]), dAtof(argv[3]));
}

ConsoleMethod(GuiParticleGraphCtrl, setGraphMax, void, 5, 5, "(int plotID, float maxX, float maxY) "
			  "Set the max values of the graph of plotID."
			  "@param plotID The plot to modify\n"
			  "@param maxX,maxY The maximum bound of the value range.\n"
			  "@return No return value.")
{
	S32 plotID = dAtoi(argv[2]);

	if(plotID > object->MaxPlots)
	{
		Con::errorf("Invalid plotID.");
		return;
	}

	object->setGraphMax(dAtoi(argv[2]), Point2F(dAtof(argv[3]), dAtof(argv[4])));
}

ConsoleMethod(GuiParticleGraphCtrl, setGraphMaxX, void, 4, 4, "(int plotID, float maxX)"
			  "Set the max X value of the graph of plotID."
			  "@param plotID The plot to modify.\n"
			  "@param maxX The maximum x value.\n"
			  "@return No return Value.")
{
	S32 plotID = dAtoi(argv[2]);

	if(plotID > object->MaxPlots)
	{
		Con::errorf("Invalid plotID.");
		return;
	}

	object->setGraphMaxX(dAtoi(argv[2]), dAtof(argv[3]));
}

ConsoleMethod(GuiParticleGraphCtrl, setGraphMaxY, void, 4, 4, "(int plotID, float maxY)"
			  "Set the max Y value of the graph of plotID."
			  "@param plotID The plot to modify.\n"
			  "@param maxY The maximum y value.\n"
			  "@return No return Value.")
{
	S32 plotID = dAtoi(argv[2]);

	if(plotID > object->MaxPlots)
	{
		Con::errorf("Invalid plotID.");
		return;
	}

	object->setGraphMaxY(dAtoi(argv[2]), dAtof(argv[3]));
}

ConsoleMethod(GuiParticleGraphCtrl, setGraphHidden, void, 4, 4, "(int plotID, bool isHidden)"
			  "Set whether the graph number passed is hidden or not."
			  "@return No return value.")
{
	S32 plotID = dAtoi(argv[2]);

	if(plotID > object->MaxPlots)
	{
		Con::errorf("Invalid plotID.");
		return;
	}

	object->setGraphHidden(dAtoi(argv[2]), dAtob(argv[3]));
}

ConsoleMethod(GuiParticleGraphCtrl, setAutoGraphMax, void, 3, 3, "(bool autoMax) "
			  "Set whether the max will automatically be set when adding points "
			  "(ie if you add a value over the current max, the max is increased to that value).\n"
			  "@return No return value.")
{
	object->setAutoGraphMax(dAtob(argv[2]));
}

ConsoleMethod(GuiParticleGraphCtrl, setAutoRemove, void, 3, 3, "(bool autoRemove) "
			  "Set whether or not a point should be deleted when you drag another one over it."
			  "@return No return value.")
{
	object->setAutoRemove(dAtob(argv[2]));
}

ConsoleMethod(GuiParticleGraphCtrl, setRenderAll, void, 3, 3, "(bool renderAll)"
			  "Set whether or not a position should be rendered on every point or just the last selected."
			  "@return No return value.")
{
	object->setRenderAll(dAtob(argv[2]));
}

ConsoleMethod(GuiParticleGraphCtrl, setPointXMovementClamped, void, 3, 3, "(bool clamped)"
			  "Set whether the x position of the selected graph point should be clamped"
			  "@return No return value.")
{
	object->setPointXMovementClamped(dAtob(argv[2]));
}

ConsoleMethod(GuiParticleGraphCtrl, setRenderGraphTooltip, void, 3, 3, "(bool renderGraphTooltip)"
			  "Set whether or not to render the graph tooltip."
			  "@return No return value.")
{
	object->setRenderGraphTooltip(dAtob(argv[2]));
}

ConsoleMethod(GuiParticleGraphCtrl, setGraphName, void, 4, 4, "(int plotID, string graphName) "
			  "Set the name of the given plot.\n"
			  "@param plotID The plot to modify.\n"
			  "@param graphName The name to set on the plot.\n"
			  "@return No return value.")
{
	S32 plotID = dAtoi(argv[2]);

	if(plotID > object->MaxPlots)
	{
		Con::errorf("Invalid plotID.");
		return;
	}

	object->setGraphName(dAtoi(argv[2]), argv[3]);
}

ConsoleMethod(GuiParticleGraphCtrl, resetSelectedPoint, void, 2, 2, "()"
			  "This will reset the currently selected point to nothing."
			  "@return No return value.")
{
	object->resetSelectedPoint();
}
