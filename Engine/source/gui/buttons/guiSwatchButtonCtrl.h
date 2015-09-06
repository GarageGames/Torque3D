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

#ifndef _GUISWATCHBUTTONCTRL_H_
#define _GUISWATCHBUTTONCTRL_H_

#ifndef _GUIBUTTONBASECTRL_H_
   #include "gui/buttons/guiButtonBaseCtrl.h"
#endif


/// A color swatch button.
///
class GuiSwatchButtonCtrl : public GuiButtonBaseCtrl
{
   public:
      
      typedef GuiButtonBaseCtrl Parent;

   protected:
      
      /// The color to display on the button.
      ColorF mSwatchColor;
      
      /// Bitmap used for mGrid
      String mGridBitmap;

      /// Background texture that will show through with transparent colors.
      GFXTexHandle mGrid;
      
   public:

      GuiSwatchButtonCtrl();

      /// Return the color displayed in the swatch.
      ColorF getColor() { return mSwatchColor; }

      /// Set the color to display in the swatch.
      void setColor( const ColorF &color ) { mSwatchColor = color; }

      // GuiButtonBaseCtrl
      virtual bool onWake();
      virtual void onRender(Point2I offset, const RectI &updateRect);

      static void initPersistFields();

      DECLARE_CONOBJECT( GuiSwatchButtonCtrl );
      DECLARE_DESCRIPTION( "A color swatch button." );
};

#endif // _GUISWATCHBUTTONCTRL_H_
