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
#ifndef _GUIGRADIENTCTRL_H_
#define _GUIGRADIENTCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

#ifndef _GUISWATCHBUTTONCTRL_H_
#include "gui/buttons/guiSwatchButtonCtrl.h"
#endif

class GuiGradientSwatchCtrl : public GuiSwatchButtonCtrl
{
private:		
	typedef GuiSwatchButtonCtrl Parent;
private:
	Point2I mMouseDownPosition;
	RectI mOrigBounds;
public:
	DECLARE_CONOBJECT(GuiGradientSwatchCtrl);
	DECLARE_CALLBACK( void, onMouseDown, ());
	DECLARE_CALLBACK( void, onDoubleClick, ());
	GuiGradientSwatchCtrl();
	void onMouseDown(const GuiEvent &);
	void onRightMouseDown(const GuiEvent &);
   void onMouseDragged(const GuiEvent &event);
	void onRender(Point2I offset, const RectI &updateRect);
	bool onWake();
protected:
	StringTableEntry  mColorFunction;
};
//----------------------------
/// GuiGradientCtrl

class GuiGradientCtrl : public GuiControl
{
   typedef GuiControl Parent;
   
public:
   enum PickMode
   {
		pHorizColorRange,		///< We have a range of base colors going horizontally
		pHorizAlphaRange,		///< We have a box which shows a range in alpha going horizontally
   };
   
   enum SelectorMode
   {
		sHorizontal = 0,		///< Horizontal selector with small gap
		sVertical,			///< Vertical selector with small gap
   };
	
	struct ColorRange
	{
		GuiGradientSwatchCtrl* swatch;
		S32 pos;
		ColorF color;
	};

	Vector<ColorRange> mColorRange;
	Vector<ColorRange> mAlphaRange;
	S32 mSwatchFactor;
	RectI mBlendRangeBox;

private:
   
   /// @name Core Rendering functions
   /// @{
   void renderColorBox(RectI &bounds);			///< Function that draws the actual color box
   //void drawSelector(RectI &bounds, Point2I &selectorPos, SelectorMode mode);	///< Function that draws the selection indicator
   void drawBlendRangeBox(RectI &bounds, bool vertical, Vector<ColorRange> colorRange);
   /// @}

   /// @name Core Variables
   /// @{
   ColorF mPickColor;		///< Color that has been picked from control
   ColorF mBaseColor;		///< Colour we display (in case of pallet and blend mode)
   PickMode mDisplayMode;	///< Current color display mode of the selector
	PickMode mSaveDisplayMode;

   bool mPositionChanged;	///< Current position has changed since last render?
   bool mMouseOver;		///< Mouse is over?
   bool mMouseDown;		///< Mouse button down?
   bool mActionOnMove;		///< Perform onAction() when position has changed?

   GFXStateBlockRef mStateBlock;
	
	ColorF colorWhite;
	ColorF colorWhiteBlend;
	ColorF colorBlack;
	ColorF colorAlpha;
	ColorF colorAlphaW;
   /// @}
	String mColorFunction;
	
public:   
   
   DECLARE_CONOBJECT(GuiGradientCtrl);
   DECLARE_CATEGORY( "Gui Editor" );
   
   GuiGradientCtrl();

   static void initPersistFields();
   void onRender(Point2I offset, const RectI &updateRect);
   bool mShowReticle;       ///< Show reticle on render
   /// @name Color Value Functions
   /// @{
   /// NOTE: setValue only sets baseColor, since setting pickColor wouldn't be useful
   void setValue(ColorF &value) {mBaseColor = value;}
   /// NOTE: getValue() returns baseColor if pallet (since pallet controls can't "pick" colours themselves)
   ColorF getValue() {return mPickColor;}
   void updateColor() {mPositionChanged = true;}
   /// @}
   
   /// @name Input Events
   /// @{
   void onMouseDown(const GuiEvent &);
   void onMouseUp(const GuiEvent &);

   void onMouseEnter(const GuiEvent &);
   void onMouseLeave(const GuiEvent &);
   /// @}

	void addColorRange(ColorI color);
	void setupDefaultRange();

	bool onAdd();
	void inspectPreApply();
	void inspectPostApply();
	void reInitSwatches( GuiGradientCtrl::PickMode );
   void addColorRange(Point2I pos, const ColorF& color);
	void removeColorRange( GuiGradientSwatchCtrl* swatch );
	void sortColorRange();

	PickMode getDisplayMode() { return mDisplayMode; }
};

typedef GuiGradientCtrl::PickMode GuiGradientPickMode;
DefineEnumType( GuiGradientPickMode );

#endif
