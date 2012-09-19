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

#ifndef _GUIGRAPHCTRL_H_
#define _GUIGRAPHCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

class GuiGraphCtrl : public GuiControl
{
   public:
   
      typedef GuiControl Parent;

      enum Constants
      {
         MaxPlots = 6,
         MaxDataPoints = 200
      };

      enum GraphType {
         Point,
         Polyline,
         Filled,
         Bar
      };

   protected:
   
      F32 mCenterY;
      StringTableEntry mAutoPlot[ MaxPlots ];
      U32 mAutoPlotDelay[ MaxPlots ];
      SimTime mAutoPlotLastDisplay[ MaxPlots ];
      ColorF mGraphColor[ MaxPlots ];
      Vector< F32 > mGraphData[ MaxPlots ];
      F32 mGraphMax[ MaxPlots ];
      GraphType mGraphType[ MaxPlots ];

      GFXStateBlockRef  mSolidSB;
      GFXStateBlockRef  mBlendSB;
      
   public:
      
      GuiGraphCtrl();

      void addDatum( S32 plotID, F32 v );
      F32  getDatum( S32 plotID, S32 samples );
      void addAutoPlot( S32 plotID, const char *variable, S32 update );
      void removeAutoPlot( S32 plotID);
      void setGraphType( S32 plotID, GraphType graphType );
      F32 getMax( S32 plotID ) const { return mGraphMax[ plotID ]; }
      void setMax( S32 plotID, F32 max ) { mGraphMax[ plotID ] = max; }

      // GuiControl.
      virtual void onRender(Point2I offset, const RectI &updateRect);

      DECLARE_CONOBJECT(GuiGraphCtrl);
      DECLARE_CATEGORY( "Gui Other" );
      DECLARE_DESCRIPTION( "A control that allows to plot curve graphs." );
      
      static void initPersistFields();
};

typedef GuiGraphCtrl::GraphType GuiGraphType;
DefineEnumType( GuiGraphType );

#endif
