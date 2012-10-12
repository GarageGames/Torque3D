//guiBitmapStringCtrl.cpp
//by: Robert Fritzen for Phantom Games Development
//Copyright 2012, Phantom Games Development

/**
Permission is granted by me (Robert Fritzen) for free use and editing by anyone with a copy of Torque 3D.
This control is only meant for use in Torque 3D 1.2. 

I have no specific licensing requirements, I am however not responsible for any issues this may cause, so
use at your own risk? I have yet to encounter any issues, so I think there shouldn't be any problems anyways. :)

DOC: Torque Definitions:
 * setBitmap(string path) - Sets the default image of the control
 * setImageCount(int amount) - Sets the amount of images to string using the default image
 * setCutoffBitmap(string path) - Sets the cutoff image of the control
 * setCutoffPoint(int amount) - Sets the point at which the control will use the other image

 * This control basically acts like a GuiBitmapControl without mWrap turned on. It will
 * automatically re-scale the images based on the size of the control and the number of images
 * that you specify.
**/

#include "platform/platform.h"
#include "gui/controls/guiBitmapStringCtrl.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"


IMPLEMENT_CONOBJECT(GuiBitmapStringCtrl);

ConsoleDocClass( GuiBitmapStringCtrl,
   "@brief A gui control that is used to string two image types.\n\n"
   
   "The images are stretched to the constraints of the control by default.\n\n"
   
   "@ingroup GuiControls"
);

GuiBitmapStringCtrl::GuiBitmapStringCtrl(void)
 : mBitmapName(),
   mCutoffBitmapName(),
	mCutoffPoint(0),
	mImageCount(1)
{	
}

bool GuiBitmapStringCtrl::setBitmapName( void *object, const char *index, const char *data ) {
   static_cast<GuiBitmapStringCtrl *>( object )->setBitmap( data );
   return false;
}

bool GuiBitmapStringCtrl::setCutoffBitmapName( void *object, const char *index, const char *data ) {
   static_cast<GuiBitmapStringCtrl *>( object )->setCutoffBitmap( data );
   return false;
}

void GuiBitmapStringCtrl::initPersistFields() {
   addGroup( "Bitmap" );
   
      addProtectedField( "bitmap", TypeImageFilename, Offset( mBitmapName, GuiBitmapStringCtrl ),
         &setBitmapName, &defaultProtectedGetFn,
         "The bitmap file that is normally stringed (without a cutoff)." );
      addProtectedField( "cutoffBitmap", TypeImageFilename, Offset( mCutoffBitmapName, GuiBitmapStringCtrl ),
         &setCutoffBitmapName, &defaultProtectedGetFn,
         "The bitmap file to string when cutoff is active." );  

   endGroup( "Bitmap" );

   Parent::initPersistFields();
}

bool GuiBitmapStringCtrl::onWake() {
	if(!Parent::onWake()) {
      return false;
   }
   setActive(true);
   setBitmap(mBitmapName);
   setCutoffBitmap(mCutoffBitmapName);
	return true;
}

void GuiBitmapStringCtrl::onSleep() {
	if(!mBitmapName.equal("texhandle", String::NoCase)) {
      mTextureObject = NULL;
   }
	if(!mCutoffBitmapName.equal("texhandle", String::NoCase)) {
	   mCutoffTextureObject = NULL;
	}

   Parent::onSleep();
}

//-------------------------------------
void GuiBitmapStringCtrl::inspectPostApply() {
   Parent::inspectPostApply();

   if ((getExtent().x == 0) && (getExtent().y == 0) && mTextureObject) {
      setExtent( mTextureObject->getWidth(), mTextureObject->getHeight());
   }
}

void GuiBitmapStringCtrl::setBitmap(const char *name, bool resize) {
   mBitmapName = name;
	if(!isAwake()) {
      return;
	}
   if(mBitmapName.isNotEmpty()) {
		if(!mBitmapName.equal("texhandle", String::NoCase)) {
		   mTextureObject.set(mBitmapName, &GFXDefaultGUIProfile, avar("%s() - mTextureObject (line %d)", __FUNCTION__, __LINE__));
		}
      // Resize the control to fit the bitmap
      if (mTextureObject && resize) {
         setExtent(mTextureObject->getWidth(), mTextureObject->getHeight());
         updateSizing();
      }
   }
	else {
      mTextureObject = NULL;
	}

   setUpdate();
}

void GuiBitmapStringCtrl::setCutoffBitmap(const char *name, bool resize) {
   mCutoffBitmapName = name;
	if(!isAwake()) {
      return;
	}
   if(mCutoffBitmapName.isNotEmpty()) {
		if(!mCutoffBitmapName.equal("texhandle", String::NoCase)) {
		   mCutoffTextureObject.set(mCutoffBitmapName, &GFXDefaultGUIProfile, avar("%s() - mTextureObject (line %d)", __FUNCTION__, __LINE__));
		}
      // Resize the control to fit the bitmap
      if (mCutoffTextureObject && resize) {
         setExtent(mCutoffTextureObject->getWidth(), mCutoffTextureObject->getHeight());
         updateSizing();
      }
   }
	else {
      mCutoffTextureObject = NULL;
	}

   setUpdate();
}

void GuiBitmapStringCtrl::updateSizing() {
	if(!getParent()) {
      return;
	}
   // updates our bounds according to our horizSizing and verSizing rules
   RectI fakeBounds(getPosition(), getParent()->getExtent());
   parentResized(fakeBounds, fakeBounds);
}

void GuiBitmapStringCtrl::setBitmapHandle(GFXTexHandle handle, bool resize) {
   mTextureObject = handle;

   mBitmapName = String("texhandle");

   if (resize) {
      setExtent(mTextureObject->getWidth(), mTextureObject->getHeight());
      updateSizing();
   }
}

void GuiBitmapStringCtrl::setCutoffBitmapHandle(GFXTexHandle handle, bool resize) {
   mCutoffTextureObject = handle;

   mCutoffBitmapName = String("texhandle");
}

S32 GuiBitmapStringCtrl::getImageCount() {
   return mImageCount;
}

S32 GuiBitmapStringCtrl::getCutoffPoint() {
   return mCutoffPoint;
}

void GuiBitmapStringCtrl::setImageCounts(S32 imageCount, S32 cutoffPoint) {
   mImageCount = imageCount;
	mCutoffPoint = cutoffPoint;

	//retexture if necessary
	if(mTextureObject || (mCutoffTextureObject && cutoffPoint > 0)) {
	   setUpdate();
	}
}

void GuiBitmapStringCtrl::onRender(Point2I offset, const RectI &updateRect) {
   if (mTextureObject) { //If we only have a mCutoffBitmap, there's a problem... lol
      GFX->getDrawUtil()->clearBitmapModulation();
 		GFXTextureObject* texture = mTextureObject, *cutoff = mTextureObject; //default cutoff here, we can adjust later

		//Definitions
      S32 nCO_count = 0, CO_count = 0;
		RectI dstSize;
		//

		if(mCutoffTextureObject && mCutoffPoint <= mImageCount && mCutoffPoint >= 0) {
		   //define the cutoff, if necessary
         cutoff = mCutoffTextureObject;
			CO_count = mImageCount - mCutoffPoint;
		}
		nCO_count = mImageCount - CO_count;
		//
		if(mImageCount <= 0) {
			Con::errorf("GuiBitmapStringCtrl: Corrected mImageCount for control. (was 0)");
		   mImageCount = 1;
		}
		S32 x_scale = getExtent().x / mImageCount;
		//
		for(int i = 0; i < mImageCount; i++) {
			dstSize.set((offset.x + (x_scale * i)), 
				(offset.y),
				(x_scale),
				(getExtent().y));

			if(i >= mCutoffPoint) {
			   GFX->getDrawUtil()->drawBitmapStretch(cutoff, dstSize, GFXBitmapFlip_None, GFXTextureFilterLinear, false);   
			}
			else {
				GFX->getDrawUtil()->drawBitmapStretch(texture, dstSize, GFXBitmapFlip_None, GFXTextureFilterLinear, false);			
			}
		}
   }

   if (mProfile->mBorder || !mTextureObject) {
      RectI rect(offset.x, offset.y, getExtent().x, getExtent().y);
      GFX->getDrawUtil()->drawRect(rect, mProfile->mBorderColor);
   }

   renderChildControls(offset, updateRect);
}


//"Set the bitmap displayed in the control. Note that it is limited in size, to 256x256."
ConsoleMethod(GuiBitmapStringCtrl, setBitmap, void, 3, 4,
   "( String filename | String filename, bool resize ) Assign the default image to this control.\n\n"
   "@hide") {
   char filename[1024];
   Con::expandScriptFilename(filename, sizeof(filename), argv[2]);
   object->setBitmap(filename, argc > 3 ? dAtob( argv[3] ) : false );
}

ConsoleMethod(GuiBitmapStringCtrl, setCutoffBitmap, void, 3, 4,
   "( String filename | String filename, bool resize ) Assign the cutoff image to this control.\n\n"
   "@hide") {
   char filename[1024];
   Con::expandScriptFilename(filename, sizeof(filename), argv[2]);
   object->setCutoffBitmap(filename, argc > 3 ? dAtob( argv[3] ) : false );
}

//New methods
DefineEngineMethod(GuiBitmapStringCtrl, setCutoffPoint, void, (S32 newPoint),, "Set the cutoff point for this control") {
   //define it
	object->setImageCounts(object->getImageCount(), newPoint);
}

DefineEngineMethod(GuiBitmapStringCtrl, setImageCount, void, (S32 newCount),, "set the image count for this image") {
   //define it
	object->setImageCounts(newCount, object->getCutoffPoint());
}