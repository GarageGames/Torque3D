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
#include "gui/editor/guiGraphCtrl.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/primBuilder.h"


IMPLEMENT_CONOBJECT( GuiGraphCtrl );

ConsoleDocClass( GuiGraphCtrl,
   "@brief A control that plots one or more curves in a chart.\n\n"

   "Up to 6 individual curves can be plotted in the graph.  Each plotted curve can have its own display style including its "
   "own charting style (#plotType) and color (#plotColor).\n\n"
   
   "The data points on each curve can be added in one of two ways:\n\n"
   
   "- Manually by calling addDatum().  This causes new data points to be added to the left end of the plotting curve.\n"
   "- Automatically by letting the graph plot the values of a variable over time.  This is achieved by calling addAutoPlot "
      "and specifying the variable and update frequency.\n\n"
      
   "@tsexample\n"
      "// Create a graph that plots a red polyline graph of the FPS counter in a 1 second (1000 milliseconds) interval.\n"
      "new GuiGraphCtrl( FPSGraph )\n"
      "{\n"
      "   plotType[ 0 ] = \"PolyLine\";\n"
      "   plotColor[ 0 ] = \"1 0 0\";\n"
      "   plotVariable[ 0 ] = \"fps::real\";\n"
      "   plotInterval[ 0 ] = 1000;\n"
      "};\n"
   "@endtsexample\n\n"
      
   "@note Each curve has a maximum number of 200 data points it can have at any one time.  Adding more data points to a curve "
      "will cause older data points to be removed.\n\n"
   
   "@ingroup GuiValues"
);

ImplementEnumType( GuiGraphType,
   "The charting style of a single plotting curve in a GuiGraphCtrl.\n\n"
   "@ingroup GuiValues" )
   { GuiGraphCtrl::Bar, "Bar", "Plot the curve as a bar chart." },
   { GuiGraphCtrl::Filled, "Filled", "Plot a filled poly graph that connects the data points on the curve." },
   { GuiGraphCtrl::Point, "Point", "Plot each data point on the curve as a single dot." },
   { GuiGraphCtrl::Polyline , "PolyLine", "Plot straight lines through the data points of the curve." },
EndImplementEnumType;


//-----------------------------------------------------------------------------

GuiGraphCtrl::GuiGraphCtrl()
   : mCenterY( 1.f )
{
   dMemset( mAutoPlot, 0, sizeof( mAutoPlot ) );
   dMemset( mAutoPlotDelay, 0, sizeof( mAutoPlotDelay ) );
   dMemset( mGraphMax, 0, sizeof( mGraphMax ) );
   
   for( S32 i = 0; i < MaxPlots; ++ i )
   {
	   mGraphType[ i ] = Polyline;

	   VECTOR_SET_ASSOCIATION( mGraphData[ i ] );
   }

   AssertWarn( MaxPlots == 6, "Only 6 plot colors initialized.  Update following code if you change MaxPlots." );

   mGraphColor[ 0 ] = ColorF( 1.0, 1.0, 1.0 );
   mGraphColor[ 1 ] = ColorF( 1.0, 0.0, 0.0 );
   mGraphColor[ 2 ] = ColorF( 0.0, 1.0, 0.0 );
   mGraphColor[ 3 ] = ColorF( 0.0, 0.0, 1.0 );
   mGraphColor[ 4 ] = ColorF( 0.0, 1.0, 1.0 );
   mGraphColor[ 5 ] = ColorF( 0.0, 0.0, 0.0 );
}

//-----------------------------------------------------------------------------

void GuiGraphCtrl::initPersistFields()
{
   addGroup( "Graph" );
   
      addField( "centerY", TypeF32, Offset( mCenterY, GuiGraphCtrl ),
         "Ratio of where to place the center coordinate of the graph on the Y axis. 0.5=middle height of control.\n\n"
         "This allows to account for graphs that have only positive or only negative data points, for example." );
      
      addField( "plotColor", TypeColorF, Offset( mGraphColor, GuiGraphCtrl ), MaxPlots,
         "Color to use for the plotting curves in the graph." );
      
      addField( "plotType", TYPEID< GuiGraphType >(), Offset( mGraphType, GuiGraphCtrl ), MaxPlots,
         "Charting type of the plotting curves." );
                  
      addField( "plotVariable", TypeString, Offset( mAutoPlot, GuiGraphCtrl ), MaxPlots,
         "Name of the variable to automatically plot on the curves.  If empty, auto-plotting "
         "is disabled for the respective curve." );
            
      addField( "plotInterval", TypeS32, Offset( mAutoPlotDelay, GuiGraphCtrl ), MaxPlots,
         "Interval between auto-plots of #plotVariable for the respective curve (in milliseconds)." );
   
   endGroup( "Graph" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

void GuiGraphCtrl::onRender(Point2I offset, const RectI &updateRect)
{   
   if (mBlendSB.isNull())
   {
      GFXStateBlockDesc desc;

      desc.setBlend(true, GFXBlendSrcColor, GFXBlendInvSrcColor);
      mBlendSB = GFX->createStateBlock(desc);

      desc.setBlend(false, GFXBlendOne, GFXBlendZero);
      mSolidSB = GFX->createStateBlock(desc);

   }

   GFX->setStateBlock( mBlendSB );
   
	GFX->getDrawUtil()->drawRectFill( updateRect, mProfile->mFillColor );
   
   GFX->setStateBlock( mSolidSB );

   const Point2I globalPos = getGlobalBounds().point;
   const F32 midPointY = F32( globalPos.y ) + F32( getExtent().y ) * mCenterY;

	for( S32 k = 0; k < MaxPlots; ++ k )
	{
		// Check if there is an autoplot and the proper amount of time has passed, if so add datum.
		if( mAutoPlot[ k ] && mAutoPlotDelay[ k ] < (Sim::getCurrentTime() - mAutoPlotLastDisplay[ k ] ) )
		{
         mAutoPlotLastDisplay[ k ] = Sim::getCurrentTime();
         addDatum( k, Con::getFloatVariable( mAutoPlot[ k ] ) );
		}

		// Nothing to graph
		if( mGraphData[ k ].size() == 0 )
			continue;

		// Adjust scale to max value + 5% so we can see high values
		F32 Scale = F32( getExtent().y ) / F32( mGraphMax[ k ] * 1.05 );
      
      const S32 numSamples = mGraphData[ k ].size();

      switch( mGraphType[ k ] )
      {
         case Bar:
         {
            F32 prevOffset = 0;

            for( S32 sample = 0; sample < numSamples; ++ sample )
            {                  
               PrimBuild::begin( GFXTriangleStrip, 4 );
               PrimBuild::color( mGraphColor[ k ] );

               F32 offset = F32( getExtent().x ) / F32( MaxDataPoints ) * F32( sample + 1 );

               PrimBuild::vertex2f( globalPos.x + prevOffset,
                  midPointY - ( getDatum( k, sample ) * Scale ) );

               PrimBuild::vertex2f( globalPos.x + offset,
                  midPointY - ( getDatum( k, sample ) * Scale ) );

               PrimBuild::vertex2f( globalPos.x + offset,
                  midPointY );

               PrimBuild::vertex2f( globalPos.x + prevOffset,
                  midPointY );

               prevOffset = offset;

               PrimBuild::end();
            }

            break;
         }
         
         case Filled:
         {
            PrimBuild::begin( GFXTriangleStrip, numSamples * 2 ); // Max size correct? -pw
            PrimBuild::color( mGraphColor[ k ] );

            for( S32 sample = 0; sample < numSamples; ++ sample )
            {
               F32 offset = F32( getExtent().x ) / F32( MaxDataPoints - 1 ) * F32( sample );

               PrimBuild::vertex2f( globalPos.x + offset,
                  midPointY );

               PrimBuild::vertex2f( globalPos.x + offset,
                  midPointY - ( getDatum( k, sample ) * Scale ) );
            }

            PrimBuild::end();
            break;
         }

         case Point:
         case Polyline:
         {
            if( mGraphType[ k ] == Point )
               PrimBuild::begin( GFXPointList, numSamples ); // Max size correct? -pw
            else
               PrimBuild::begin( GFXLineStrip, numSamples );

            PrimBuild::color( mGraphColor[ k ] );

            for( S32 sample = 0; sample < numSamples; ++ sample )
            {
               F32 offset = F32( getExtent().x ) / F32( MaxDataPoints - 1 ) * F32( sample );

               PrimBuild::vertex2f( globalPos.x + offset,
                  midPointY - ( getDatum( k, sample ) * Scale ) );
            }

            PrimBuild::end();
            break;
         }
      }
	}

	if( mProfile->mBorder )
	{
		RectI rect( offset.x, offset.y, getWidth(), getHeight() );
		GFX->getDrawUtil()->drawRect( rect, mProfile->mBorderColor );
	}
   
   renderChildControls( offset, updateRect );
}

//-----------------------------------------------------------------------------

void GuiGraphCtrl::addDatum(S32 plotID, F32 v)
{
   AssertFatal(plotID > -1 && plotID < MaxPlots, "Invalid plot specified!");

   // Add the data and trim the vector...
   mGraphData[ plotID ].push_front( v );

   if( mGraphData[ plotID ].size() > MaxDataPoints )
      mGraphData[ plotID ].pop_back();

   // Keep record of maximum data value for scaling purposes.
   if( v > mGraphMax[ plotID ] )
	   mGraphMax[ plotID ] = v;
}

//-----------------------------------------------------------------------------

F32 GuiGraphCtrl::getDatum( S32 plotID, S32 sample)
{
   AssertFatal(plotID > -1 && plotID < MaxPlots, "Invalid plot specified!");
   AssertFatal(sample > -1 && sample < MaxDataPoints, "Invalid sample specified!");
   return mGraphData[ plotID ][sample];
}

//-----------------------------------------------------------------------------

void GuiGraphCtrl::addAutoPlot( S32 plotID, const char* variable, S32 update )
{
   mAutoPlot[ plotID ] = StringTable->insert( variable );
   mAutoPlotDelay[ plotID ] = update;
}

//-----------------------------------------------------------------------------

void GuiGraphCtrl::removeAutoPlot( S32 plotID )
{
   mAutoPlot[ plotID ] = NULL;
}

//-----------------------------------------------------------------------------

void GuiGraphCtrl::setGraphType( S32 plotID, GraphType graphType )
{
	AssertFatal( plotID > -1 && plotID < MaxPlots, "Invalid plot specified!" );
   mGraphType[ plotID ] = graphType;
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiGraphCtrl, addDatum, void, ( S32 plotId, F32 value ),,
   "Add a data point to the plot's curve.\n\n"
   "@param plotId Index of the plotting curve to which to add the data point.  Must be 0<=plotId<6.\n"
   "@param value Value of the data point to add to the curve.\n\n"
   "@note Data values are added to the @b left end of the plotting curve.\n\n"
   "@note A maximum number of 200 data points can be added to any single plotting curve at any one time.  If "
      "this limit is exceeded, data points on the right end of the curve are culled." )
{
   if( plotId > object->MaxPlots )
   {
	   Con::errorf( "GuiGraphCtrl::addDatum - 'plotId' out of range: %i", plotId );
	   return;
   }
   
   object->addDatum( plotId, value );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiGraphCtrl, getDatum, F32, ( S32 plotId, S32 index ),,
   "Get a data point on the given plotting curve.\n\n"
   "@param plotId Index of the plotting curve from which to fetch the data point.  Must be 0<=plotId<6.\n"
   "@param index Index of the data point on the curve.\n"
   "@return The value of the data point or -1 if @a plotId or @a index are out of range." )
{
   if( plotId > object->MaxPlots )
   {
	   Con::errorf( "GuiGraphCtrl::getDatum - 'plotId' out of range: %i", plotId );
	   return -1.f;
   }
   
   if( index > object->MaxDataPoints)
   {
	   Con::errorf( "GuiGraphCtrl::getDatum - Data point index out of range: %i", index );
	   return -1.f;
   }

   return object->getDatum( plotId, index );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiGraphCtrl, addAutoPlot, void, ( S32 plotId, const char* variable, S32 updateFrequency ),,
   "Sets up the given plotting curve to automatically plot the value of the @a variable with a "
   "frequency of @a updateFrequency.\n"
   "@param plotId Index of the plotting curve.  Must be 0<=plotId<6.\n"
   "@param variable Name of the global variable.\n"
   "@param updateFrequency Frequency with which to add new data points to the plotting curve (in milliseconds).\n"
   "@tsexample\n"
   "// Plot FPS counter at 1 second intervals.\n"
   "%graph.addAutoPlot( 0, \"fps::real\", 1000 );\n"
   "@endtsexample" )
{
   if( plotId > object->MaxPlots )
   {
	   Con::errorf( "GuiGraphCtrl::removeAutoPlot - 'plotId' out of range: %i", plotId );
	   return;
   }

   object->addAutoPlot( plotId, variable, updateFrequency );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiGraphCtrl, removeAutoPlot, void, ( S32 plotId ),,
   "Stop automatic variable plotting for the given curve.\n"
   "@param plotId Index of the plotting curve.  Must be 0<=plotId<6.\n" )
{
   if( plotId > object->MaxPlots )
   {
	   Con::errorf( "GuiGraphCtrl::removeAutoPlot - 'plotId' out of range: %i", plotId );
	   return;
   }

   object->removeAutoPlot( plotId );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiGraphCtrl, setGraphType, void, ( S32 plotId, GuiGraphType graphType ),,
   "Change the charting type of the given plotting curve.\n"
   "@param plotId Index of the plotting curve.  Must be 0<=plotId<6.\n"
   "@param graphType Charting type to use for the curve.\n"
   "@note Instead of calling this method, you can directly assign to #plotType." )
{
	if( plotId > object->MaxPlots )
	{
	   Con::errorf( "GuiGraphCtrl::setGraphType - 'plotId' out of range: %i", plotId );
		return;
	}

	object->setGraphType( plotId, graphType );
}

//-----------------------------------------------------------------------------

ConsoleMethod( GuiGraphCtrl, matchScale, void, 3, GuiGraphCtrl::MaxPlots + 2, "( int plotID1, int plotID2, ... ) "
   "Set the scale of all specified plots to the maximum scale among them.\n\n"
   "@param plotID1 Index of plotting curve.\n"
   "@param plotID2 Index of plotting curve." )
{   
   F32 max = 0;
	for( S32 i = 0; i < ( argc - 2 ); ++ i )
	{
      S32 plotID = dAtoi( argv[ 2 + i ] );
		if( plotID > object->MaxPlots )
		{
         Con::errorf( "GuiGraphCtrl::matchScale - Plot ID out of range: %i", plotID );
			return;
		}
      
      max = getMax( object->getMax( plotID ), max );
	}
   
   for( S32 i = 0; i < ( argc - 2 ); ++ i )
		object->setMax( dAtoi( argv[ 2 + i ] ), max );
}
