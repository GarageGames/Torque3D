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
#ifndef _GUICOLORPICKER_H_
#define _GUICOLORPICKER_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

//----------------------------
/// GuiColorPickerCtrl
///
/// This control draws a box containing a color specified by mPickColor, 
/// in a way according to one of the PickMode enum's, stored as mDisplayMode.
/// 
/// The color the box represents is stored as mBaseColour (for pPallete, pBlendColorRange), 
/// whilst the color chosen by the box is stored as mPickColor.
///
/// Whenever the control is clicked, it will do one of many things :
///
/// -# If its in pPallete mode, execute the regular "command"
/// -# If its in pBlendColorRange mode, update the selector position. The position will be re-read upon the next render. In addition, a cross will be drawn where the color has been selected from. As with 1, "command" will be executed.
/// -# If its in pHorizColorRange or pVertColorRange mode, it will function in a similar manner to 2, but the selector will resemble a horizontal or vertical bar.
/// -# If its in pHorizAlphaRange or pVertAlphaRange mode, it will also function the same way as 3
/// -# If its in pDropperBackground mode, nothing will happen
///
/// Colours are drawn in different ways according to mDisplayMode:
///
/// -# With pPallete, a box with a blank color, mBaseColor is drawn.
/// -# With pHorizColorRange, a horizontal box with colors blending in the range, mColorRange.
/// -# With pVertColorRange, a vertical box with colors blending in the range, mColorRange.
/// -# With pBlendColorRange, a box, the bottom colors being black, but the top left being white, and the top right being mBaseColor.
/// -# With pHorizAlphaRange, a horizontal box with black blending with an alpha from 0 to 255
/// -# With pVertAlphaRange, a vertical box with black blending with an apha from 0 to 255
/// -# With pDropperBackground, nothing is drawn
class GuiColorPickerCtrl : public GuiControl
{
   typedef GuiControl Parent;
   
  public:
   enum PickMode
   {
     pPallet = 0,                ///< We just have a solid color; We just act like a pallet 
     pHorizColorRange,           ///< We have a range of base colors going horizontally
     pVertColorRange,            ///< We have a range of base colors going vertically
     pHorizColorBrightnessRange, ///< HorizColorRange with brightness
     pVertColorBrightnessRange,  ///< VertColorRange with brightness
     pBlendColorRange,           ///< We have a box which shows a range in brightness of the color
     pHorizAlphaRange,           ///< We have a box which shows a range in alpha going horizontally
     pVertAlphaRange,            ///< We have a box which shows a range in alpha going vertically
     pDropperBackground          ///< The control does not draw anything; Only does something when you click, or move the mouse (when active)
   };
   
   enum SelectorMode
   {
     sHorizontal = 0,            ///< Horizontal selector with small gap
     sVertical,                  ///< Vertical selector with small gap
   };

  protected:
   /// @name Core Rendering functions
   /// @{
   void renderColorBox(RectI &bounds); ///< Function that draws the actual color box
   void drawSelector(RectI &bounds, Point2I &selectorPos, SelectorMode mode); /// < Function that draws the selection indicator
   void drawBlendBox(RectI &bounds, ColorF &c1, ColorF &c2, ColorF &c3, ColorF &c4);
   void drawBlendRangeBox(RectI &bounds, bool vertical, U8 numColors, ColorI *colors);
   /// @}

   /// @name Core Variables
   /// @{
   ColorF mPickColor;		///< Color that has been picked from control
   ColorF mBaseColor;		///< Colour we display (in case of pallet and blend mode)
   PickMode mDisplayMode;	///< Current color display mode of the selector
   bool mUseSRGB;          ///< use sRGB color scale
   
   Point2I mSelectorPos;	///< Current position of the selector
   bool mPositionChanged;	///< Current position has changed since last render?
   bool mMouseOver;		///< Mouse is over?
   bool mMouseDown;		///< Mouse button down?
   bool mActionOnMove;		///< Perform onAction() when position has changed?

   bool mSelectColor;
   ColorF mSetColor;
   GBitmap* mBitmap;

   Point2I findColor(const ColorF & color, const Point2I& offset, const Point2I& resolution, GBitmap& bmp);
   
   S32   mSelectorGap;		///< The half-way "gap" between the selector pos and where the selector is allowed to draw. 

   GFXStateBlockRef mStateBlock;

   static ColorI mColorRange[7]; ///< Color range for pHorizColorRange and pVertColorRange
   /// @}

  public:

   DECLARE_CONOBJECT(GuiColorPickerCtrl);
   DECLARE_CATEGORY( "Gui Editor" );
   
   GuiColorPickerCtrl();
   ~GuiColorPickerCtrl();

   static void initPersistFields();
   void onRender(Point2I offset, const RectI &updateRect);
   bool mShowReticle;       ///< Show reticle on render
   /// @name Color Value Functions
   /// @{
   /// NOTE: setValue only sets baseColor, since setting pickColor wouldn't be useful
   void setValue(ColorF &value) {mBaseColor = value;}
   /// NOTE: getValue() returns baseColor if pallet (since pallet controls can't "pick" colours themselves)
   ColorF getValue() { return mDisplayMode == pPallet ? mBaseColor : mPickColor; }
   const char *getScriptValue();
   void setScriptValue(const char *value);
   void updateColor() {mPositionChanged = true;}
   /// @}

   /// @name Selector Functions
   /// @{
   void setSelectorPos(const Point2I &pos); ///< Set new pos (in local coords)
   void setSelectorPos(const ColorF & color);
   Point2I getSelectorPos() {return mSelectorPos;}
   /// @}

   /// @name Input Events
   /// @{
   void onMouseDown(const GuiEvent &);
   void onMouseUp(const GuiEvent &);
   void onMouseMove(const GuiEvent &event);
   void onMouseDragged(const GuiEvent &event);

   void onMouseEnter(const GuiEvent &);
   void onMouseLeave(const GuiEvent &);
   /// @}
};

typedef GuiColorPickerCtrl::PickMode GuiColorPickMode;
DefineEnumType( GuiColorPickMode );

#endif
