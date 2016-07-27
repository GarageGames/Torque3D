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
#include "gui/buttons/guiBitmapButtonCtrl.h"
#include "core/util/path.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gui/core/guiCanvas.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTextureManager.h"


ImplementEnumType( GuiBitmapMode,
   "Rendering behavior when placing bitmaps in controls.\n\n"
   "@ingroup GuiImages" )
   { GuiBitmapButtonCtrl::BitmapStretched, "Stretched", "Stretch bitmap to fit control extents." },
   { GuiBitmapButtonCtrl::BitmapCentered, "Centered", "Center bitmap in control."  },
EndImplementEnumType;


//=============================================================================
//    GuiBitmapButtonCtrl
//=============================================================================

IMPLEMENT_CONOBJECT(GuiBitmapButtonCtrl);

ConsoleDocClass( GuiBitmapButtonCtrl,
   "@brief A button that renders its various states (mouse over, pushed, etc.) from separate bitmaps.\n\n"
   
   "A bitmapped button is a push button that uses one or more texture images for rendering its individual states.\n\n"
   
   "To find the individual textures associated with the button, a naming scheme is used.  For each state "
   "a suffix is appended to the texture file name given in the GuiBitmapButtonCtrl::bitmap field:\n"
   
   "- \"_n\": Normal state.  This one will be active when no other state applies.\n"
   "- \"_h\": Highlighted state.  This applies when the mouse is hovering over the button.\n"
   "- \"_d\": Depressed state.  This applies when the left mouse button has been clicked on the button but not yet released.\n"
   "- \"_i\": Inactive state.  This applies when the button control has been deactivated (GuiControl::setActive())\n\n"
   
   "If a bitmap for a particular state cannot be found, the default bitmap will be used.  To disable all state-based "
   "bitmap functionality, set useStates to false which will make the control solely render from the bitmap specified "
   "in the bitmap field.\n\n"

   "@section guibitmapbutton_modifiers Per-Modifier Button Actions\n"
   
   "If GuiBitmapButtonCtrl::useModifiers is set to true, per-modifier button actions and textures are enabled.  This functionality "
   "allows to associate different images and different actions with a button depending on which modifiers are pressed "
   "on the keyboard by the user.\n\n"
   
   "When enabled, this functionality alters the texture lookup above by prepending the following strings to the "
   "suffixes listed above:\n"
   
   "- \"\": Default.  No modifier is pressed.\n"
   "- \"_ctrl\": Image to use when CTRL/CMD is down.\n"
   "- \"_alt\": Image to use when ALT is down.\n"
   "- \"_shift\": Image to use when SHIFT is down\n\n"
   
   "When this functionality is enabled, a new set of callbacks is used:\n"
   
   "- onDefaultClick: Button was clicked without a modifier being presssed.\n"
   "- onCtrlClick: Button was clicked with the CTRL/CMD key down.\n"
   "- onAltClick: Button was clicked with the ALT key down.\n"
   "- onShiftClick: Button was clicked with the SHIFT key down.\n\n"
   
   "GuiControl::command or GuiControl::onAction() still work as before when per-modifier functionality is enabled.\n\n"
   
   "Note that modifiers cannot be mixed.  If two or more modifiers are pressed, a single one will take precedence over "
   "the remaining modifiers.  The order of precedence corresponds to the order listed above.\n\n"
   
   "@tsexample\n"
   "// Create an OK button that will trigger an onOk() call on its parent when clicked:\n"
   "%okButton = new GuiBitmapButtonCtrl()\n"
   "{\n"
   "   bitmap = \"art/gui/okButton\";\n"
   "   autoFitExtents = true;\n"
   "   command = \"$ThisControl.getParent().onOk();\";\n"
   "};\n"
   "@endtsexample\n\n"
   
   "@ingroup GuiButtons"
);

IMPLEMENT_CALLBACK( GuiBitmapButtonCtrl, onDefaultClick, void, (), (),
   "Called when per-modifier functionality is enabled and the user clicks on the button without any modifier pressed.\n"
   "@ref guibitmapbutton_modifiers" );
IMPLEMENT_CALLBACK( GuiBitmapButtonCtrl, onCtrlClick, void, (), (),
   "Called when per-modifier functionality is enabled and the user clicks on the button with the CTRL key pressed.\n"
   "@ref guibitmapbutton_modifiers" );
IMPLEMENT_CALLBACK( GuiBitmapButtonCtrl, onAltClick, void, (), (),
   "Called when per-modifier functionality is enabled and the user clicks on the button with the ALT key pressed.\n"
   "@ref guibitmapbutton_modifiers" );
IMPLEMENT_CALLBACK( GuiBitmapButtonCtrl, onShiftClick, void, (), (),
   "Called when per-modifier functionality is enabled and the user clicks on the button with the SHIFT key pressed.\n"
   "@ref guibitmapbutton_modifiers" );

//-----------------------------------------------------------------------------

GuiBitmapButtonCtrl::GuiBitmapButtonCtrl()
{
   mBitmapMode = BitmapStretched;
   mAutoFitExtents = false;
   mUseModifiers = false;
   mUseStates = true;
   setExtent( 140, 30 );
   mMasked = false;
}

//-----------------------------------------------------------------------------

void GuiBitmapButtonCtrl::initPersistFields()
{
   addGroup( "Bitmap" );
   
      addProtectedField( "bitmap", TypeStringFilename, Offset( mBitmapName, GuiBitmapButtonCtrl ),
         &_setBitmap, &defaultProtectedGetFn,
         "Texture file to display on this button.\n"
         "If useStates is false, this will be the file that renders on the control.  Otherwise, this will "
         "specify the default texture name to which the various state and modifier suffixes are appended "
         "to find the per-state and per-modifier (if enabled) textures." );
      addField( "bitmapMode", TYPEID< BitmapMode >(), Offset( mBitmapMode, GuiBitmapButtonCtrl ),
         "Behavior for fitting the bitmap to the control extents.\n"
         "If set to 'Stretched', the bitmap will be stretched both verticall and horizontally to fit inside "
         "the control's extents.\n\n"
         "If set to 'Centered', the bitmap will stay at its original resolution centered in the control's "
         "rectangle (getting clipped if the control is smaller than the texture)." );
      addProtectedField( "autoFitExtents", TypeBool, Offset( mAutoFitExtents, GuiBitmapButtonCtrl ),
         &_setAutoFitExtents, &defaultProtectedGetFn,
         "If true, the control's extents will be set to match the bitmap's extents when setting the bitmap.\n"
         "The bitmap extents will always be taken from the default/normal bitmap (in case the extents of the various "
         "bitmaps do not match up.)" );
      addField( "useModifiers", TypeBool, Offset( mUseModifiers, GuiBitmapButtonCtrl ),
         "If true, per-modifier button functionality is enabled.\n"
         "@ref guibitmapbutton_modifiers" );
      addField( "useStates", TypeBool, Offset( mUseStates, GuiBitmapButtonCtrl ),
         "If true, per-mouse state button functionality is enabled.\n"
         "Defaults to true.\n\n"
         "If you do not use per-state images on this button set this to false to speed up the loading process "
         "by inhibiting searches for the individual images." );
      addField("masked", TypeBool, Offset(mMasked, GuiBitmapButtonCtrl),"Use alpha masking for interaction.");
         
   endGroup( "Bitmap" );
      
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool GuiBitmapButtonCtrl::onWake()
{
   if (! Parent::onWake())
      return false;
      
   setActive( true );
   setBitmap( mBitmapName );
   
   return true;
}

//-----------------------------------------------------------------------------

void GuiBitmapButtonCtrl::onSleep()
{
   if( dStricmp(mBitmapName, "texhandle") != 0 )
      for( U32 i = 0; i < NumModifiers; ++ i )
      {
         mTextures[ i ].mTextureNormal = NULL;
         mTextures[ i ].mTextureHilight = NULL;
         mTextures[ i ].mTextureDepressed = NULL;
         mTextures[ i ].mTextureInactive = NULL;
      }

   Parent::onSleep();
}

//-----------------------------------------------------------------------------

bool GuiBitmapButtonCtrl::_setAutoFitExtents( void *object, const char *index, const char *data )
{
   GuiBitmapButtonCtrl* ctrl = reinterpret_cast< GuiBitmapButtonCtrl* >( object );
   ctrl->setAutoFitExtents( dAtob( data ) );
   return false;
}

//-----------------------------------------------------------------------------

bool GuiBitmapButtonCtrl::_setBitmap( void *object, const char *index, const char *data )
{
   GuiBitmapButtonCtrl* ctrl = reinterpret_cast< GuiBitmapButtonCtrl* >( object );
   ctrl->setBitmap( data );
   return false;
}

//-----------------------------------------------------------------------------

// Legacy method.  Can just assign to bitmap field.
DefineEngineMethod( GuiBitmapButtonCtrl, setBitmap, void, ( const char* path ),,
   "Set the bitmap to show on the button.\n"
   "@param path Path to the texture file in any of the supported formats.\n" )
{
   object->setBitmap( path );
}

//-----------------------------------------------------------------------------

void GuiBitmapButtonCtrl::inspectPostApply()
{
   Parent::inspectPostApply();

   Torque::Path path( mBitmapName );
   const String& fileName = path.getFileName();
   
   if( mUseStates )
   {
      // If the filename points to a single state, automatically
      // cut off the state part.  Makes it easy to select files in
      // the editor without having to go in and manually cut off the
      // state parts all the time.
      
      static String s_n = "_n";
      static String s_d = "_d";
      static String s_h = "_h";
      static String s_i = "_i";
      
      if(    fileName.endsWith( s_n )
          || fileName.endsWith( s_d )
          || fileName.endsWith( s_h )
          || fileName.endsWith( s_i ) )
      {
         path.setFileName( fileName.substr( 0, fileName.length() - 2 ) );
         path.setExtension( String::EmptyString );
      }
   }
   
   setBitmap( path.getFullPath() );

   // if the extent is set to (0,0) in the gui editor and appy hit, this control will
   // set it's extent to be exactly the size of the normal bitmap (if present)

   if ((getWidth() == 0) && (getHeight() == 0) && mTextures[ 0 ].mTextureNormal)
   {
      setExtent( mTextures[ 0 ].mTextureNormal->getWidth(), mTextures[ 0 ].mTextureNormal->getHeight());
   }
}

//-----------------------------------------------------------------------------

void GuiBitmapButtonCtrl::setAutoFitExtents( bool state )
{
   mAutoFitExtents = state;
   if( mAutoFitExtents )
      setBitmap( mBitmapName );
}

//-----------------------------------------------------------------------------

void GuiBitmapButtonCtrl::setBitmap( const String& name )
{
   PROFILE_SCOPE( GuiBitmapButtonCtrl_setBitmap );
   
   mBitmapName = name;
   if( !isAwake() )
      return;

   if( !mBitmapName.isEmpty() )
   {
      if( dStricmp( mBitmapName, "texhandle" ) != 0 )
      {
         const U32 count = mUseModifiers ? NumModifiers : 1;
         for( U32 i = 0; i < count; ++ i )
         {
            static String modifiers[] =
            {
               "",
               "_ctrl",
               "_alt",
               "_shift"
            };
            
            static String s_n = "_n";
            static String s_d = "_d";
            static String s_h = "_h";
            static String s_i = "_i";

            String baseName = mBitmapName;
            if( mUseModifiers )
               baseName += modifiers[ i ];

            mTextures[ i ].mTextureNormal = GFXTexHandle( baseName, &GFXDefaultPersistentProfile, avar("%s() - mTextureNormal (line %d)", __FUNCTION__, __LINE__));
            
            if( mUseStates )
            {
               if( !mTextures[ i ].mTextureNormal )
                  mTextures[ i ].mTextureNormal = GFXTexHandle( baseName + s_n, &GFXDefaultPersistentProfile, avar("%s() - mTextureNormal (line %d)", __FUNCTION__, __LINE__));
               
               mTextures[ i ].mTextureHilight = GFXTexHandle( baseName + s_h, &GFXDefaultPersistentProfile, avar("%s() - mTextureHighlight (line %d)", __FUNCTION__, __LINE__));
               if( !mTextures[ i ].mTextureHilight )
                  mTextures[ i ].mTextureHilight = mTextures[ i ].mTextureNormal;
                  
               mTextures[ i ].mTextureDepressed = GFXTexHandle( baseName + s_d, &GFXDefaultPersistentProfile, avar("%s() - mTextureDepressed (line %d)", __FUNCTION__, __LINE__));
               if( !mTextures[ i ].mTextureDepressed )
                  mTextures[ i ].mTextureDepressed = mTextures[ i ].mTextureHilight;

               mTextures[ i ].mTextureInactive = GFXTexHandle( baseName + s_i, &GFXDefaultPersistentProfile, avar("%s() - mTextureInactive (line %d)", __FUNCTION__, __LINE__));
               if( !mTextures[ i ].mTextureInactive )
                  mTextures[ i ].mTextureInactive = mTextures[ i ].mTextureNormal;
            }

            if( i == 0 && mTextures[ i ].mTextureNormal.isNull() && mTextures[ i ].mTextureHilight.isNull() && mTextures[ i ].mTextureDepressed.isNull() && mTextures[ i ].mTextureInactive.isNull() )
            {
               Con::warnf( "GuiBitmapButtonCtrl::setBitmap - Unable to load texture: %s", mBitmapName.c_str() );
               this->setBitmap( GFXTextureManager::getUnavailableTexturePath() );
               return;
            }
         }
      }
      
      if( mAutoFitExtents && !mTextures[ 0 ].mTextureNormal.isNull() )
         setExtent( mTextures[ 0 ].mTextureNormal.getWidth(), mTextures[ 0 ].mTextureNormal.getHeight() );
   }
   else
   {
      for( U32 i = 0; i < NumModifiers; ++ i )
      {
         mTextures[ i ].mTextureNormal = NULL;
         mTextures[ i ].mTextureHilight = NULL;
         mTextures[ i ].mTextureDepressed = NULL;
         mTextures[ i ].mTextureInactive = NULL;
      }
   }
   
   setUpdate();
}

//-----------------------------------------------------------------------------

void GuiBitmapButtonCtrl::setBitmapHandles(GFXTexHandle normal, GFXTexHandle highlighted, GFXTexHandle depressed, GFXTexHandle inactive)
{
   const U32 count = mUseModifiers ? NumModifiers : 1;
   for( U32 i = 0; i < count; ++ i )
   {
      mTextures[ i ].mTextureNormal = normal;
      mTextures[ i ].mTextureHilight = highlighted;
      mTextures[ i ].mTextureDepressed = depressed;
      mTextures[ i ].mTextureInactive = inactive;

      if (!mTextures[ i ].mTextureHilight)
         mTextures[ i ].mTextureHilight = mTextures[ i ].mTextureNormal;
      if (!mTextures[ i ].mTextureDepressed)
         mTextures[ i ].mTextureDepressed = mTextures[ i ].mTextureHilight;
      if (!mTextures[ i ].mTextureInactive)
         mTextures[ i ].mTextureInactive = mTextures[ i ].mTextureNormal;

      if (mTextures[ i ].mTextureNormal.isNull() && mTextures[ i ].mTextureHilight.isNull() && mTextures[ i ].mTextureDepressed.isNull() && mTextures[ i ].mTextureInactive.isNull())
      {
         Con::warnf("GuiBitmapButtonCtrl::setBitmapHandles() - Invalid texture handles");
         setBitmap( GFXTextureManager::getUnavailableTexturePath() );
         
         return;
      }
   }

   mBitmapName = "texhandle";
}

//------------------------------------------------------------------------------

GuiBitmapButtonCtrl::Modifier GuiBitmapButtonCtrl::getCurrentModifier()
{   
   U8 modifierKeys = Input::getModifierKeys();

   if( modifierKeys & SI_PRIMARY_CTRL )
      return ModifierCtrl;
   else if( modifierKeys & SI_PRIMARY_ALT )
      return ModifierAlt;
   else if( modifierKeys & SI_SHIFT )
      return ModifierShift;
   
   return ModifierNone;
}

//------------------------------------------------------------------------------

GFXTexHandle& GuiBitmapButtonCtrl::getTextureForCurrentState()
{
   U32 index = ModifierNone;
   if( mUseModifiers )
      index = getCurrentModifier();
         
   if( !mUseStates )
   {
      if( mTextures[ index ].mTextureNormal )
         return mTextures[ 0 ].mTextureNormal;
      else
         return mTextures[ index ].mTextureNormal;
   }

   switch( getState() )
   {
      case NORMAL:
         if( !mTextures[ index ].mTextureNormal )
            return mTextures[ 0 ].mTextureNormal;
         else
            return mTextures[ index ].mTextureNormal;
            
      case HILIGHT:
         if( !mTextures[ index ].mTextureHilight )
            return mTextures[ 0 ].mTextureHilight;
         else
            return mTextures[ index ].mTextureHilight;
            
      case DEPRESSED:
         if( !mTextures[ index ].mTextureDepressed )
            return mTextures[ 0 ].mTextureDepressed;
         else
            return mTextures[ index ].mTextureDepressed;
            
      default:
         if( !mTextures[ index ].mTextureInactive )
            return mTextures[ 0 ].mTextureInactive;
         else
            return mTextures[ index ].mTextureInactive;
   }
}

//------------------------------------------------------------------------------

void GuiBitmapButtonCtrl::onAction()
{
   Parent::onAction();
   
   if( mUseModifiers )
   {
      switch( getCurrentModifier() )
      {
         case ModifierNone:
            onDefaultClick_callback();
            break;
         
         case ModifierCtrl:
            onCtrlClick_callback();
            break;
         
         case ModifierAlt:
            onAltClick_callback();
            break;
         
         case ModifierShift:
            onShiftClick_callback();
            break;
            
         default:
            break;
      }
   }
}

//------------------------------------------------------------------------------

void GuiBitmapButtonCtrl::onRender(Point2I offset, const RectI& updateRect)
{
   GFXTexHandle& texture = getTextureForCurrentState();
   if( texture )
   {
      renderButton( texture, offset, updateRect );
      renderChildControls( offset, updateRect );
   }
   else
      Parent::onRender(offset, updateRect);
}

//------------------------------------------------------------------------------

void GuiBitmapButtonCtrl::renderButton( GFXTexHandle &texture, const Point2I &offset, const RectI& updateRect )
{
   GFX->getDrawUtil()->clearBitmapModulation();
   
   switch( mBitmapMode )
   {
      case BitmapStretched:
      {
         RectI rect( offset, getExtent() );
         GFX->getDrawUtil()->drawBitmapStretch( texture, rect );
         break;
      }
         
      case BitmapCentered:
      {
         Point2I p = offset;
         
         p.x += getExtent().x / 2 - texture.getWidth() / 2;
         p.y += getExtent().y / 2 - texture.getHeight() / 2;
         
         GFX->getDrawUtil()->drawBitmap( texture, p );
         break;
      }
   }
}

//=============================================================================
//    GuiBitmapButtonTextCtrl.
//=============================================================================

IMPLEMENT_CONOBJECT( GuiBitmapButtonTextCtrl);

ConsoleDocClass( GuiBitmapButtonTextCtrl,
   "@brief An extension of GuiBitmapButtonCtrl that additionally renders a text label on the bitmapped button.\n\n"
   
   "The text for the label is taken from the GuiButtonBaseCtrl::text property.\n\n"
   
   "For rendering, the label is placed, relative to the control's upper left corner, at the text offset specified in the "
   "control's profile (GuiControlProfile::textOffset) and justified according to the profile's setting (GuiControlProfile::justify).\n\n"
   
   "@see GuiControlProfile::textOffset\n"
   "@see GuiControlProfile::justify\n"
   "@ingroup GuiButtons"
);

//-----------------------------------------------------------------------------

void GuiBitmapButtonTextCtrl::renderButton( GFXTexHandle &texture, const Point2I &offset, const RectI& updateRect )
{
   Parent::renderButton( texture, offset, updateRect );

   Point2I textPos = offset;
   if(mDepressed)
      textPos += Point2I(1,1);

   // Make sure we take the profile's textOffset into account.
   textPos += mProfile->mTextOffset;

   GFX->getDrawUtil()->setBitmapModulation( mProfile->mFontColor );
   renderJustifiedText(textPos, getExtent(), mButtonText);
}

bool GuiBitmapButtonCtrl::pointInControl(const Point2I& parentCoordPoint)
{
   if (mMasked && getTextureForCurrentState())
   {
      ColorI rColor(0, 0, 0, 0);
      GBitmap* bmp;

      const RectI &bounds = getBounds();
      S32 xt = parentCoordPoint.x - bounds.point.x;
      S32 yt = parentCoordPoint.y - bounds.point.y;

      bmp = getTextureForCurrentState().getBitmap();
      if (!bmp)
      {
         setBitmap(mBitmapName);
         bmp = getTextureForCurrentState().getBitmap();
      }

      S32 relativeXRange = this->getExtent().x;
      S32 relativeYRange = this->getExtent().y;
      S32 fileXRange = bmp->getHeight(0);
      S32 fileYRange = bmp->getWidth(0);
      //Con::errorf("xRange:[%i -- %i],  Range:[%i -- %i]  pos:(%i,%i)",relativeXRange,fileXRange,relativeYRange,fileYRange,xt,yt);

      S32 fileX = (xt*fileXRange) / relativeXRange;
      S32 fileY = (yt*fileYRange) / relativeYRange;
      //Con::errorf("Checking %s @ (%i,%i)",this->getName(),fileX,fileY);

      bmp->getColor(fileX, fileY, rColor);

      if (rColor.alpha)
         return true;
      else
         return false;
   }
   else
      return Parent::pointInControl(parentCoordPoint);
}