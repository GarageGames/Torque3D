//guiBitmapStringCtrl.h
//by: Robert Fritzen for Phantom Games Development
//Copyright 2012, Phantom Games Development
// * This basically strings together images in a linear fashion.
// It automatically re-sizes them based on the control and you can specifiy
// a cut off point to where it switches to another image.

// Permission is hereby granted by me (Robert Fritzen) for public use under
// Torque 3D 1.2. Do note that I am not responsible for any issues adding this may
// cause to your project. I do ask however, that if you find any problems to please
// correct them and provide the changes to the community.

#ifndef _GUIBITMAPSTRINGCTRL_H_
#define _GUIBITMAPSTRINGCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

#include <string>

class GuiBitmapStringCtrl : public GuiControl {
   public:
   
      typedef GuiControl Parent;

   protected:
      String mBitmapName;
		String mCutoffBitmapName;
      
      /// Loaded texture.
      GFXTexHandle mTextureObject;
		GFXTexHandle mCutoffTextureObject;

		S32 mCutoffPoint; //the cutoff point number
		S32 mImageCount;  //the amount of images to string

      static bool setBitmapName( void *object, const char *index, const char *data );
      static const char *getBitmapName( void *obj, const char *data );
      static bool setCutoffBitmapName( void *object, const char *index, const char *data );
      static const char *getCutoffBitmapName( void *obj, const char *data );

   public:
      
      GuiBitmapStringCtrl();
      static void initPersistFields();

      void setBitmap(const char *name,bool resize = false);
      void setBitmapHandle(GFXTexHandle handle, bool resize = false);
      void setCutoffBitmap(const char *name,bool resize = false);
      void setCutoffBitmapHandle(GFXTexHandle handle, bool resize = false);

      // GuiControl.
      bool onWake();
      void onSleep();
      void inspectPostApply();

      void updateSizing();

      void onRender(Point2I offset, const RectI &updateRect);

      void setImageCounts(S32 imageCount, S32 cutoffPoint);
		S32 getImageCount();
		S32 getCutoffPoint();

      DECLARE_CONOBJECT( GuiBitmapStringCtrl );
      DECLARE_CATEGORY( "Gui Images" );
      DECLARE_DESCRIPTION( "A control that strings together two images based on a position number." );
};

#endif
