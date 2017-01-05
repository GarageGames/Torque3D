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
#include "gui/core/guiControl.h"

#include "console/consoleTypes.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"
#include "console/codeBlock.h"
#include "gfx/bitmap/gBitmap.h"
#include "sim/actionMap.h"
#include "gui/core/guiCanvas.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gui/editor/guiEditCtrl.h"
#include "gfx/gfxDrawUtil.h"


//#define DEBUG_SPEW


IMPLEMENT_CONOBJECT( GuiControl );

ConsoleDocClass( GuiControl,
   "@brief Base class for all Gui control objects.\n\n"
   
   "GuiControl is the basis for the Gui system.  It represents an individual control that can be placed on the canvas and with which "
   "the mouse and keyboard can potentially interact with.\n\n"
   
   "@section GuiControl_Hierarchy Control Hierarchies\n"
   
   "GuiControls are arranged in a hierarchy.  All children of a control are placed in their parent's coordinate space, i.e. their "
   "coordinates are relative to the upper left corner of their immediate parent.  When a control is moved, all its child controls "
   "are moved along with it.\n\n"
   
   "Since GuiControl's are SimGroups, hierarchy also implies ownership.  This means that if a control is destroyed, all its children "
   "are destroyed along with it.  It also means that a given control can only be part of a single GuiControl hierarchy.  When adding a "
   "control to another control, it will automatically be reparented from another control it may have previously been parented to.\n\n"
   
   "@section GuiControl_Layout Layout System\n"
   
   "GuiControls have a two-dimensional position and are rectangular in shape.\n\n"
   
   "@section GuiControl_Events Event System\n"
   
   "@section GuiControl_Profiles Control Profiles\n"
   
   "Common data accessed by GuiControls is stored in so-called \"Control Profiles.\"  This includes font, color, and texture information. "
   "By pooling this data in shared objects, the appearance of any number of controls can be changed quickly and easily by modifying "
   "only the shared profile object.\n\n"
   
   "If not explicitly assigned a profile, a control will by default look for a profile object that matches its class name.  This means "
   "that the class GuiMyCtrl, for example, will look for a profile called 'GuiMyProfile'.  If this profile cannot be found, the control "
   "will fall back to GuiDefaultProfile which must be defined in any case for the Gui system to work.\n\n"
   
   "In addition to its primary profile, a control may be assigned a second profile called 'tooltipProfile' that will be used to render "
   "tooltip popups for the control.\n\n"
   
   "@section GuiControl_Actions Triggered Actions\n"
   
   "@section GuiControl_FirstResponders First Responders\n"
   
   "At any time, a single control can be what is called the \"first responder\" on the GuiCanvas is placed on.  This control "
   "will be the first control to receive keyboard events not bound in the global ActionMap.  If the first responder choses to "
   "handle a particular keyboard event, \n\n"
   
   "@section GuiControl_Waking Waking and Sleeping\n"
   
   "@section GuiControl_VisibleActive Visibility and Activeness\n"
   "By default, a GuiControl is active which means that it\n\n"
   
   "@see GuiCanvas\n"
   "@see GuiControlProfile\n"
   "@ingroup GuiCore\n"
);

IMPLEMENT_CALLBACK( GuiControl, onAdd, void, (), (),
   "Called when the control object is registered with the system after the control has been created." );
IMPLEMENT_CALLBACK( GuiControl, onRemove, void, (), (),
   "Called when the control object is removed from the system before it is deleted." );
IMPLEMENT_CALLBACK( GuiControl, onWake, void, (), (),
   "Called when the control is woken up.\n"
   "@ref GuiControl_Waking" );
IMPLEMENT_CALLBACK( GuiControl, onSleep, void, (), (),
   "Called when the control is put to sleep.\n"
   "@ref GuiControl_Waking" );
IMPLEMENT_CALLBACK( GuiControl, onGainFirstResponder, void, (), (),
   "Called when the control gains first responder status on the GuiCanvas.\n"
   "@see setFirstResponder\n"
   "@see makeFirstResponder\n"
   "@see isFirstResponder\n"
   "@ref GuiControl_FirstResponders" );
IMPLEMENT_CALLBACK( GuiControl, onLoseFirstResponder, void, (), (),
   "Called when the control loses first responder status on the GuiCanvas.\n"
   "@see setFirstResponder\n"
   "@see makeFirstResponder\n"
   "@see isFirstResponder\n"
   "@ref GuiControl_FirstResponders" );
IMPLEMENT_CALLBACK( GuiControl, onAction, void, (), (),
   "Called when the control's associated action is triggered and no 'command' is defined for the control.\n"
   "@ref GuiControl_Actions" );
IMPLEMENT_CALLBACK( GuiControl, onVisible, void, ( bool state ), ( state ),
   "Called when the control changes its visibility state, i.e. when going from visible to invisible or vice versa.\n"
   "@param state The new visibility state.\n"
   "@see isVisible\n"
   "@see setVisible\n"
   "@ref GuiControl_VisibleActive" );
IMPLEMENT_CALLBACK( GuiControl, onActive, void, ( bool state ), ( state ),
   "Called when the control changes its activeness state, i.e. when going from active to inactive or vice versa.\n"
   "@param stat The new activeness state.\n"
   "@see isActive\n"
   "@see setActive\n"
   "@ref GuiControl_VisibleActive" );
IMPLEMENT_CALLBACK( GuiControl, onDialogPush, void, (), (),
   "Called when the control is pushed as a dialog onto the canvas.\n"
   "@see GuiCanvas::pushDialog" );
IMPLEMENT_CALLBACK( GuiControl, onDialogPop, void, (), (),
   "Called when the control is removed as a dialog from the canvas.\n"
   "@see GuiCanvas::popDialog" );
IMPLEMENT_CALLBACK( GuiControl, onControlDragEnter, void, ( GuiControl* control, const Point2I& dropPoint ), ( control, dropPoint ),
   "Called when a drag&drop operation through GuiDragAndDropControl has entered the control.  This is only called for "
   "topmost visible controls as the GuiDragAndDropControl moves over them.\n\n"
   "@param control The payload of the drag operation.\n"
   "@param dropPoint The point at which the payload would be dropped if it were released now.  Relative to the canvas." );
IMPLEMENT_CALLBACK( GuiControl, onControlDragExit, void, ( GuiControl* control, const Point2I& dropPoint ), ( control, dropPoint ),
   "Called when a drag&drop operation through GuiDragAndDropControl has exited the control and moved over a different control.  This is only called for "
   "topmost visible controls as the GuiDragAndDropControl moves off of them.\n\n"
   "@param control The payload of the drag operation.\n"
   "@param dropPoint The point at which the payload would be dropped if it were released now.  Relative to the canvas." );
IMPLEMENT_CALLBACK( GuiControl, onControlDragged, void, ( GuiControl* control, const Point2I& dropPoint ), ( control, dropPoint ),
   "Called when a drag&drop operation through GuiDragAndDropControl is moving across the control after it has entered it.  This is only called for "
   "topmost visible controls as the GuiDragAndDropControl moves across them.\n\n"
   "@param control The payload of the drag operation.\n"
   "@param dropPoint The point at which the payload would be dropped if it were released now.  Relative to the canvas." );
IMPLEMENT_CALLBACK( GuiControl, onControlDropped, void, ( GuiControl* control, const Point2I& dropPoint ), ( control, dropPoint ),
   "Called when a drag&drop operation through GuiDragAndDropControl has completed and is dropping its payload onto the control.  "
   "This is only called for topmost visible controls as the GuiDragAndDropControl drops its payload on them.\n\n"
   "@param control The control that is being dropped onto this control.\n"
   "@param dropPoint The point at which the control is being dropped.  Relative to the canvas." );


GuiControl *GuiControl::smPrevResponder = NULL;
GuiControl *GuiControl::smCurResponder = NULL;
GuiEditCtrl*GuiControl::smEditorHandle = NULL;
bool        GuiControl::smDesignTime = false;
GuiControl* GuiControl::smThisControl;

IMPLEMENT_SCOPE( GuiAPI, Gui,, "" );

ImplementEnumType( GuiHorizontalSizing,
   "Horizontal sizing behavior of a GuiControl.\n\n"
   "@ingroup GuiCore" )
	{ GuiControl::horizResizeRight,           "right"     },
	{ GuiControl::horizResizeWidth,           "width"     },
	{ GuiControl::horizResizeLeft,            "left"      },
   { GuiControl::horizResizeCenter,          "center"    },
   { GuiControl::horizResizeRelative,        "relative"  },
   { GuiControl::horizResizeAspectLeft,      "aspectLeft" },
   { GuiControl::horizResizeAspectRight,     "aspectRight" },
   { GuiControl::horizResizeAspectCenter,    "aspectCenter" },
	{ GuiControl::horizResizeWindowRelative,  "windowRelative"  }
EndImplementEnumType;

ImplementEnumType( GuiVerticalSizing,
   "Vertical sizing behavior of a GuiControl.\n\n"
   "@ingroup GuiCore" )
	{ GuiControl::vertResizeBottom,           "bottom"     },
	{ GuiControl::vertResizeHeight,           "height"     },
	{ GuiControl::vertResizeTop,              "top"        },
   { GuiControl::vertResizeCenter,           "center"     },
   { GuiControl::vertResizeRelative,         "relative"   },
   { GuiControl::vertResizeAspectTop,        "aspectTop"  },
   { GuiControl::vertResizeAspectBottom,     "aspectBottom" },
   { GuiControl::vertResizeAspectCenter,     "aspectCenter" },
	{ GuiControl::vertResizeWindowRelative,   "windowRelative"   }
EndImplementEnumType;

//-----------------------------------------------------------------------------

GuiControl::GuiControl() : mAddGroup( NULL ),
                           mBounds(0,0,64,64),
                           mProfile(NULL),
                           mTooltipProfile(NULL),
                           mTipHoverTime(1000),
                           mVisible(true),
                           mActive(true),
                           mAwake(false),
                           mIsContainer(false),
                           mCanResize(true),
                           mCanHit( true ),
                           mLayer(0),
                           mMinExtent(8,2),
                           mLangTable(NULL),
                           mFirstResponder(NULL),
                           mHorizSizing(horizResizeRight),
                           mVertSizing(vertResizeBottom)
{
   mConsoleVariable     = StringTable->EmptyString();
   mAcceleratorKey      = StringTable->EmptyString();
   mLangTableName       = StringTable->EmptyString();
   
   mTooltip = StringTable->EmptyString();
   mRenderTooltipDelegate.bind( this, &GuiControl::defaultTooltipRender );

   mCanSaveFieldDictionary = false;
   mNotifyChildrenResized = true;
}

//-----------------------------------------------------------------------------

GuiControl::~GuiControl()
{
}

//-----------------------------------------------------------------------------

void GuiControl::consoleInit()
{
   Con::addVariable( "$ThisControl", TYPEID< GuiControl >(), &smThisControl,
      "The control for which a command is currently being evaluated.  Only set during 'command' "
      "and altCommand callbacks to the control for which the command or altCommand is invoked.\n"
	  "@ingroup GuiCore");
}

//-----------------------------------------------------------------------------

void GuiControl::initPersistFields()
{
   addGroup( "Layout" );
   
      addField("position",          TypePoint2I,      Offset(mBounds.point, GuiControl),
         "The position relative to the parent control." );
      addField("extent",            TypePoint2I,      Offset(mBounds.extent, GuiControl),
         "The width and height of the control." );
      addField("minExtent",         TypePoint2I,      Offset(mMinExtent, GuiControl),
         "The minimum width and height of the control. The control will not be resized smaller than this." );
      addField("horizSizing",       TYPEID< horizSizingOptions >(),         Offset(mHorizSizing, GuiControl),
         "The horizontal resizing behavior." );
      addField("vertSizing",        TYPEID< vertSizingOptions >(),         Offset(mVertSizing, GuiControl),
         "The vertical resizing behavior." );

   endGroup( "Layout" );

   addGroup( "Control");

      addProtectedField("profile",  TYPEID< GuiControlProfile >(),   Offset(mProfile, GuiControl), &setProfileProt, &defaultProtectedGetFn,
         "The control profile that determines fill styles, font settings, etc." );

      addProtectedField( "visible", TypeBool,         Offset(mVisible, GuiControl), &_setVisible, &defaultProtectedGetFn,
         "Whether the control is visible or hidden." );
      addProtectedField( "active",  TypeBool,         Offset( mActive, GuiControl ), &_setActive, &defaultProtectedGetFn,
         "Whether the control is enabled for user interaction." );

      addDeprecatedField("modal");
      addDeprecatedField("setFirstResponder");

      addField("variable",          TypeString,       Offset(mConsoleVariable, GuiControl),
         "Name of the variable to which the value of this control will be synchronized." );
      addField("command",           TypeCommand,   Offset(mConsoleCommand, GuiControl),
         "Command to execute on the primary action of the control.\n\n"
         "@note Within this script snippet, the control on which the #command is being executed is bound to "
            "the global variable $ThisControl." );
      addField("altCommand",        TypeCommand,   Offset(mAltConsoleCommand, GuiControl),
         "Command to execute on the secondary action of the control.\n\n"
         "@note Within this script snippet, the control on which the #altCommand is being executed is bound to "
            "the global variable $ThisControl." );
      addField("accelerator",       TypeString,       Offset(mAcceleratorKey, GuiControl),
         "Key combination that triggers the control's primary action when the control is on the canvas." );

   endGroup( "Control" );	
   
   addGroup( "ToolTip" );
      addProtectedField("tooltipProfile", TYPEID< GuiControlProfile >(), Offset(mTooltipProfile, GuiControl), &setTooltipProfileProt, &defaultProtectedGetFn,
         "Control profile to use when rendering tooltips for this control." );
      addField("tooltip",           TypeRealString,   Offset(mTooltip, GuiControl),
         "String to show in tooltip for this control." );
      addField("hovertime",         TypeS32,          Offset(mTipHoverTime, GuiControl),
         "Time for mouse to hover over control until tooltip is shown (in milliseconds)." );
   endGroup( "ToolTip" );

   addGroup( "Editing" );
      addField("isContainer",       TypeBool,      Offset(mIsContainer, GuiControl),
         "If true, the control may contain child controls." );
   endGroup( "Editing" );

   addGroup( "Localization" );
      addField("langTableMod",      TypeString,       Offset(mLangTableName, GuiControl),
         "Name of string table to use for lookup of internationalized text." );
   endGroup( "Localization" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool GuiControl::processArguments(S32 argc, ConsoleValueRef *argv)
{
   // argv[0] - The GuiGroup to add this control to when it's created.  
   //           this is an optional parameter that may be specified at
   //           object creation time to organize a gui control into a 
   //           subgroup of GuiControls and is useful in helping the 
   //           gui editor group and sort the existent gui's in the Sim.

   // Specified group?
   if( argc == 1 )
   {
      StringTableEntry steIntName = StringTable->insert(argv[0]);
      mAddGroup = dynamic_cast<SimGroup*>(Sim::getGuiGroup()->findObjectByInternalName( steIntName ));
      if( mAddGroup == NULL )
      {
         mAddGroup = new SimGroup();
         if( mAddGroup->registerObject() )
         {
            mAddGroup->setInternalName( steIntName );
            Sim::getGuiGroup()->addObject( mAddGroup );
         }
         else
         {
            SAFE_DELETE( mAddGroup );
            return false;
         }
      }
      mAddGroup->addObject(this);
   }
   return true;
}

//-----------------------------------------------------------------------------

void GuiControl::awaken()
{
   PROFILE_SCOPE( GuiControl_awaken );
   
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[GuiControl] Waking up %i:%s (%s:%s) awake=%s",
      getId(), getClassName(), getName(), getInternalName(),
      mAwake ? "true" : "false" );
   #endif

   if( mAwake )
      return;
      
   // Wake up visible children.
      
   for( GuiControl::iterator iter = begin(); iter != end(); ++ iter )
   {
      GuiControl* ctrl = static_cast< GuiControl* >( *iter );
      
      if( ctrl->isVisible() && !ctrl->isAwake() )
         ctrl->awaken();
   }

   if( !mAwake && !onWake() )
   {
      Con::errorf( ConsoleLogEntry::General, "GuiControl::awaken: failed onWake for obj: %i:%s (%s)",
         getId(), getClassName(), getName() );
      mAwake = false;
   }
}

//-----------------------------------------------------------------------------

void GuiControl::sleep()
{
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[GuiControl] Putting to sleep %i:%s (%s:%s) awake=%s",
      getId(), getClassName(), getName(), getInternalName(),
      mAwake ? "true" : "false" );
   #endif

   if( !mAwake )
      return;

   iterator i;
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      if( ctrl->isAwake() )
         ctrl->sleep();
   }

   if( mAwake )
      onSleep();
}

//=============================================================================
//    Rendering.
//=============================================================================
// MARK: ---- Rendering ----

//-----------------------------------------------------------------------------

void GuiControl::preRender()
{
   if( !mAwake )
      return;

   iterator i;
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->preRender();
   }
   onPreRender();
}

//-----------------------------------------------------------------------------

void GuiControl::onPreRender()
{
   // do nothing.
}

//-----------------------------------------------------------------------------

void GuiControl::onRender(Point2I offset, const RectI &updateRect)
{
   RectI ctrlRect(offset, getExtent());

   //if opaque, fill the update rect with the fill color
   if ( mProfile->mOpaque )
      GFX->getDrawUtil()->drawRectFill(ctrlRect, mProfile->mFillColor);

   //if there's a border, draw the border
   if ( mProfile->mBorder )
      renderBorder(ctrlRect, mProfile);

   // Render Children
   renderChildControls(offset, updateRect);
}

//-----------------------------------------------------------------------------

bool GuiControl::defaultTooltipRender( const Point2I &hoverPos, const Point2I &cursorPos, const char* tipText )
{
   // Short Circuit.
   if (!mAwake) 
      return false;
      
   if ( dStrlen( mTooltip ) == 0 && ( tipText == NULL || dStrlen( tipText ) == 0 ) )
      return false;

   String renderTip( mTooltip );
   if ( tipText != NULL )
      renderTip = tipText;
   
   // Need to have root.
   GuiCanvas *root = getRoot();
   if ( !root )
      return false;

   GFont *font = mTooltipProfile->mFont;

   GFXDrawUtil* drawUtil = GFX->getDrawUtil();

   // Support for multi-line tooltip text...

   Vector<U32> startLineOffsets, lineLengths;

   font->wrapString( renderTip, U32_MAX, startLineOffsets, lineLengths );

   // The width is the longest line.
   U32 tipWidth = 0;
   for ( U32 i = 0; i < lineLengths.size(); i++ )
   {
      U32 width = font->getStrNWidth( renderTip.c_str() + startLineOffsets[i], lineLengths[i] );

      if ( width > tipWidth )
         tipWidth = width;
   }

   // The height is the number of lines times the height of the font.
   U32 tipHeight = lineLengths.size() * font->getHeight(); 

   // Vars used:
   // Screensize (for position check)
   // Offset to get position of cursor
   // textBounds for text extent.
   Point2I screensize = getRoot()->getWindowSize();
   Point2I offset = hoverPos; 
   Point2I textBounds;   

   // Offset below cursor image
   offset.y += 20; // TODO: Attempt to fix?: root->getCursorExtent().y;
   
   // Create text bounds...

   // Pixels above/below the text
   const U32 vMargin = 2;
   // Pixels left/right of the text
   const U32 hMargin = 4;

   textBounds.x = tipWidth + hMargin * 2;
   textBounds.y = tipHeight + vMargin * 2;

   // Check position/width to make sure all of the tooltip will be rendered
   // 5 is given as a buffer against the edge
   if ( screensize.x < offset.x + textBounds.x + 5 )
      offset.x = screensize.x - textBounds.x - 5;

   // And ditto for the height
   if ( screensize.y < offset.y + textBounds.y + 5 )
      offset.y = hoverPos.y - textBounds.y - 5;
   
   RectI oldClip = GFX->getClipRect();

   // Set rectangle for the box, and set the clip rectangle.
   RectI rect( offset, textBounds );
   GFX->setClipRect( rect );

   // Draw Filler bit, then border on top of that
   drawUtil->drawRectFill( rect, mTooltipProfile->mFillColor );
   drawUtil->drawRect( rect, mTooltipProfile->mBorderColor );

   // Draw the text centered in the tool tip box...

   drawUtil->setBitmapModulation( mTooltipProfile->mFontColor );

   for ( U32 i = 0; i < lineLengths.size(); i++ )
   {      
      Point2I start( hMargin, vMargin + i * font->getHeight() );       
      const UTF8 *line = renderTip.c_str() + startLineOffsets[i];
      U32 lineLen = lineLengths[i];

      drawUtil->drawTextN( font, start + offset, line, lineLen, mProfile->mFontColors );
   }

   GFX->setClipRect( oldClip );

   return true;
}

//-----------------------------------------------------------------------------

void GuiControl::renderChildControls(Point2I offset, const RectI &updateRect)
{
   // Save the current clip rect 
   // so we can restore it at the end of this method.
   RectI savedClipRect = GFX->getClipRect();

   // offset is the upper-left corner of this control in screen coordinates
   // updateRect is the intersection rectangle in screen coords of the control
   // hierarchy.  This can be set as the clip rectangle in most cases.
   RectI clipRect = updateRect;

   iterator i;
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      if (ctrl->mVisible)
      {
         Point2I childPosition = offset + ctrl->getPosition();
         RectI childClip(childPosition, ctrl->getExtent() + Point2I(1,1));

         if (childClip.intersect(clipRect))
         {
            GFX->setClipRect( childClip );
            GFX->setStateBlock(mDefaultGuiSB);
            ctrl->onRender(childPosition, childClip);
         }
      }
   }

   // Restore the clip rect to what it was at the start
   // of this method.
   GFX->setClipRect( savedClipRect );
}

//-----------------------------------------------------------------------------

void GuiControl::setUpdateRegion(Point2I pos, Point2I ext)
{
   Point2I upos = localToGlobalCoord(pos);
   GuiCanvas *root = getRoot();
   if (root)
   {
      root->addUpdateRegion(upos, ext);
   }
}

//-----------------------------------------------------------------------------

void GuiControl::setUpdate()
{
   setUpdateRegion(Point2I(0,0), getExtent());
}

//-----------------------------------------------------------------------------

void GuiControl::renderJustifiedText(Point2I offset, Point2I extent, const char *text)
{
   GFont *font = mProfile->mFont;
   if(!font)
      return;
   S32 textWidth = font->getStrWidthPrecise((const UTF8*)text);
   U32 textHeight = font->getHeight();

   Point2I start( 0, 0 );

   // Align horizontal.
   
   if( textWidth > extent.x )
   {
      // If the text is longer then the box size, (it'll get clipped) so
      // force Left Justify
      start.x = 0;
   }
   else
      switch( mProfile->mAlignment )
      {
         case GuiControlProfile::RightJustify:
            start.x = extent.x - textWidth;
            break;
            
         case GuiControlProfile::TopJustify:
         case GuiControlProfile::BottomJustify:
         case GuiControlProfile::CenterJustify:
            start.x = ( extent.x - textWidth) / 2;
            break;
            
         case GuiControlProfile::LeftJustify:
            start.x = 0;
            break;
      }
   
   // Align vertical.

   if( textHeight > extent.y )
   {
      start.y = 0 - ( textHeight - extent.y ) / 2;
   }
   else
      switch( mProfile->mAlignment )
      {
         case GuiControlProfile::TopJustify:
            start.y = 0;
            break;
            
         case GuiControlProfile::BottomJustify:
            start.y = extent.y - textHeight - 1;
            break;
            
         default:
            // Center vertical.
            start.y = ( extent.y - font->getHeight() ) / 2;
            break;
      }

   GFX->getDrawUtil()->drawText( font, start + offset, text, mProfile->mFontColors );
}

//=============================================================================
//    Events.
//=============================================================================
// MARK: ---- Events ----

//-----------------------------------------------------------------------------

bool GuiControl::onAdd()
{
   // Let Parent Do Work.
   if ( !Parent::onAdd() )
      return false;

   // Grab the classname of this object
   const char *cName = getClassName();

   // if we're a pure GuiControl, then we're a container by default.
   if ( dStrcmp( "GuiControl", cName ) == 0 )
      mIsContainer = true;

   // Add to root group.
   if ( mAddGroup == NULL )
      mAddGroup = Sim::getGuiGroup();
   mAddGroup->addObject(this);

   // If we don't have a profile we must assign one now.
   // Try assigning one based on the control's class name...
   if ( !mProfile )
   {      
      String name = getClassName();

      if ( name.isNotEmpty() )
      {
         U32 pos = name.find( "Ctrl" );
         
         if ( pos != -1 )
            name.replace( pos, 4, "Profile" );
         else
            name += "Profile";
            
         GuiControlProfile *profile = NULL;
         if ( Sim::findObject( name, profile ) )
            setControlProfile( profile ); 
      }
   }

   // Try assigning the default profile...
   if ( !mProfile )
   {
      GuiControlProfile *profile = NULL;
      Sim::findObject( "GuiDefaultProfile", profile );

      AssertISV( profile != NULL, avar("GuiControl::onAdd() unable to find specified profile and GuiDefaultProfile does not exist!") );

      setControlProfile( profile );
   }

   // We must also assign a valid TooltipProfile...
   if ( !mTooltipProfile )
   {
      GuiControlProfile *profile = NULL;
      Sim::findObject( "GuiTooltipProfile", profile );

      AssertISV( profile != NULL, avar("GuiControl::onAdd() unable to find specified tooltip profile and GuiTooltipProfile does not exist!") );

      setTooltipProfile( profile );
   }

   // Notify Script.
   onAdd_callback();

   GFXStateBlockDesc d;

   d.cullDefined = true;
   d.cullMode = GFXCullNone;
   d.zDefined = true;
   d.zEnable = false;

   mDefaultGuiSB = GFX->createStateBlock( d );

   // Return Success.
   return true;
}

//-----------------------------------------------------------------------------

void GuiControl::onRemove()
{
   // If control is still awake, put it to sleep first.  Otherwise, we'll see an
   // onSleep() call triggered when we are removed from our parent.
   
   if( mAwake )
      sleep();
      
   // Only invoke script callbacks if they can be received
   onRemove_callback();

   if ( mProfile )
   {
      mProfile->decUseCount();
      mProfile = NULL;
   }

   if ( mTooltipProfile )
   {
      mTooltipProfile->decUseCount();   
      mTooltipProfile = NULL;
   }

   clearFirstResponder();

   Parent::onRemove();
}

//-----------------------------------------------------------------------------

void GuiControl::onDeleteNotify(SimObject *object)
{
   if( object == mProfile )
   {
      GuiControlProfile* profile;
      Sim::findObject( "GuiDefaultProfile", profile );

      if ( profile == mProfile )     
         mProfile = NULL;
      else
         setControlProfile( profile );
   }
   if (object == mTooltipProfile)
   {
      GuiControlProfile* profile;
      Sim::findObject( "GuiDefaultProfile", profile );

      if ( profile == mTooltipProfile )     
         mTooltipProfile = NULL;
      else
         setTooltipProfile( profile );
   }
}

//-----------------------------------------------------------------------------

bool GuiControl::onWake()
{
   PROFILE_SCOPE( GuiControl_onWake );
   
   AssertFatal( !mAwake, "GuiControl::onWake() - control is already awake" );
   if( mAwake )
      return true;

   // [tom, 4/18/2005] Cause mLangTable to be refreshed in case it was changed
   mLangTable = NULL;

   //set the flag
   mAwake = true;

   //set the layer
   GuiCanvas *root = getRoot();
   GuiControl *parent = getParent();
   if (parent && parent != root)
      mLayer = parent->mLayer;

   //make sure the first responder exists
   if (! mFirstResponder)
      mFirstResponder = findFirstTabable();

   //increment the profile
   mProfile->incLoadCount();
   mTooltipProfile->incLoadCount();

   // Only invoke script callbacks if we have a namespace in which to do so
   // This will suppress warnings
   onWake_callback();
   
   return true;
}

//-----------------------------------------------------------------------------

void GuiControl::onSleep()
{
   AssertFatal( mAwake, "GuiControl::onSleep() - control is already asleep" );
   if( !mAwake )
      return;

   clearFirstResponder();
   mouseUnlock();

   // Only invoke script callbacks if we have a namespace in which to do so
   // This will suppress warnings
   onSleep_callback();

   //decrement the profile reference
   mProfile->decLoadCount();
   mTooltipProfile->decLoadCount();

   // Set Flag
   mAwake = false;
}

//-----------------------------------------------------------------------------

void GuiControl::onChildAdded( GuiControl *child )
{
   // Base class does not make use of this
}

//-----------------------------------------------------------------------------

void GuiControl::onChildRemoved( GuiControl *child )
{
   // Base does nothing with this
}

//-----------------------------------------------------------------------------

void GuiControl::onGroupRemove()
{
   // If we have a first responder in our hierarchy,
   // make sure to kill it off.
   
   if( mFirstResponder )
      mFirstResponder->clearFirstResponder();
   else
   {
      GuiCanvas* root = getRoot();
      if( root )
      {
         GuiControl* firstResponder = root->getFirstResponder();
         if( firstResponder && firstResponder->isChildOfGroup( this ) )
            firstResponder->clearFirstResponder();
      }
   }
   
   // If we are awake, put us to sleep.
   
   if( isAwake() )
      sleep();
}

//-----------------------------------------------------------------------------

bool GuiControl::onInputEvent(const InputEventInfo &event)
{
	// Do nothing by default...
   return( false );
}

//-----------------------------------------------------------------------------

bool GuiControl::onKeyDown(const GuiEvent &event)
{
   //pass the event to the parent
   GuiControl *parent = getParent();
   if (parent)
      return parent->onKeyDown(event);
   else
      return false;
}

//-----------------------------------------------------------------------------

bool GuiControl::onKeyRepeat(const GuiEvent &event)
{
   // default to just another key down.
   return onKeyDown(event);
}

//-----------------------------------------------------------------------------

bool GuiControl::onKeyUp(const GuiEvent &event)
{
   //pass the event to the parent
   GuiControl *parent = getParent();
   if (parent)
      return parent->onKeyUp(event);
   else
      return false;
}

//-----------------------------------------------------------------------------

void GuiControl::onMouseUp(const GuiEvent &event)
{
}

//-----------------------------------------------------------------------------

void GuiControl::onMouseDown(const GuiEvent &event)
{
	if ( !mVisible || !mAwake )
      return;
	
	execConsoleCallback();
}

//-----------------------------------------------------------------------------

void GuiControl::onMouseMove(const GuiEvent &event)
{
   //if this control is a dead end, make sure the event stops here
   if ( !mVisible || !mAwake )
      return;

   //pass the event to the parent
   GuiControl *parent = getParent();
   if ( parent )
      parent->onMouseMove( event );
}

//-----------------------------------------------------------------------------

void GuiControl::onMouseDragged(const GuiEvent &event)
{
}

//-----------------------------------------------------------------------------

void GuiControl::onMouseEnter(const GuiEvent &event)
{
}

//-----------------------------------------------------------------------------

void GuiControl::onMouseLeave(const GuiEvent &event)
{
}

//-----------------------------------------------------------------------------

bool GuiControl::onMouseWheelUp( const GuiEvent &event )
{
   //if this control is a dead end, make sure the event stops here
   if ( !mVisible || !mAwake )
      return true;

   //pass the event to the parent
   GuiControl *parent = getParent();
   if ( parent )
      return parent->onMouseWheelUp( event );
   else
      return false;
}

//-----------------------------------------------------------------------------

bool GuiControl::onMouseWheelDown( const GuiEvent &event )
{
   //if this control is a dead end, make sure the event stops here
   if ( !mVisible || !mAwake )
      return true;

   //pass the event to the parent
   GuiControl *parent = getParent();
   if ( parent )
      return parent->onMouseWheelDown( event );
   else
      return false;
}

//-----------------------------------------------------------------------------

void GuiControl::onRightMouseDown(const GuiEvent &)
{
}

//-----------------------------------------------------------------------------

void GuiControl::onRightMouseUp(const GuiEvent &)
{
}

//-----------------------------------------------------------------------------

void GuiControl::onRightMouseDragged(const GuiEvent &)
{
}

//-----------------------------------------------------------------------------

void GuiControl::onMiddleMouseDown(const GuiEvent &)
{
}

//-----------------------------------------------------------------------------

void GuiControl::onMiddleMouseUp(const GuiEvent &)
{
}

//-----------------------------------------------------------------------------

void GuiControl::onMiddleMouseDragged(const GuiEvent &)
{
}

//-----------------------------------------------------------------------------

bool GuiControl::onGamepadButtonDown(const GuiEvent &event)
{
   return onKeyDown(event);
}

//-----------------------------------------------------------------------------

bool GuiControl::onGamepadButtonUp(const GuiEvent &event)
{
   return onKeyUp(event);
}

//-----------------------------------------------------------------------------

bool GuiControl::onGamepadAxisUp(const GuiEvent &event)
{
   //pass the event to the parent
   GuiControl *parent = getParent();
   if (parent)
   {
      return parent->onGamepadAxisUp(event);
   }
   else
   {
      return false;
   }
}

//-----------------------------------------------------------------------------

bool GuiControl::onGamepadAxisDown(const GuiEvent &event)
{
   //pass the event to the parent
   GuiControl *parent = getParent();
   if (parent)
   {
      return parent->onGamepadAxisDown(event);
   }
   else
   {
      return false;
   }
}

//-----------------------------------------------------------------------------

bool GuiControl::onGamepadAxisLeft(const GuiEvent &event)
{
   //pass the event to the parent
   GuiControl *parent = getParent();
   if (parent)
   {
      return parent->onGamepadAxisLeft(event);
   }
   else
   {
      return false;
   }
}

//-----------------------------------------------------------------------------

bool GuiControl::onGamepadAxisRight(const GuiEvent &event)
{
   //pass the event to the parent
   GuiControl *parent = getParent();
   if (parent)
   {
      return parent->onGamepadAxisRight(event);
   }
   else
   {
      return false;
   }
}

//-----------------------------------------------------------------------------

bool GuiControl::onGamepadTrigger(const GuiEvent &event)
{
   //pass the event to the parent
   GuiControl *parent = getParent();
   if (parent)
   {
      return parent->onGamepadTrigger(event);
   }
   else
   {
      return false;
   }
}

//-----------------------------------------------------------------------------

void GuiControl::onAction()
{
   if (! mActive)
      return;

   //execute the console command
   if( mConsoleCommand.isNotEmpty() )
   {
      execConsoleCallback();
   }
   else
      onAction_callback();
}

//-----------------------------------------------------------------------------

void GuiControl::onMessage( GuiControl* , S32 )
{
}

//-----------------------------------------------------------------------------

void GuiControl::onDialogPush()
{
   // Notify Script.
   onDialogPush_callback();
}

//-----------------------------------------------------------------------------

void GuiControl::onDialogPop()
{
   // Notify Script.
   onDialogPop_callback();
}

//-----------------------------------------------------------------------------

void GuiControl::inspectPreApply()
{
}

//-----------------------------------------------------------------------------

void GuiControl::inspectPostApply()
{
   // To not require every control to hook onto inspectPostApply to make
   // all changed properties take effect, just put controls through a fake
   // sleep&wake sequence.  This should generally get most property changes
   // to show.
   //
   // Don't do this with the canvas.
   
   if( mAwake && !dynamic_cast< GuiCanvas* >( this ) )
   {
      bool isContainer = mIsContainer;

      onSleep();
      onWake();
      
      mIsContainer = isContainer;
   }
}

//=============================================================================
//    Layout.
//=============================================================================
// MARK: ---- Layout ----

//-----------------------------------------------------------------------------

void GuiControl::setSizing(S32 horz, S32 vert)
{
	mHorizSizing = horz;
	mVertSizing = vert;
}

//-----------------------------------------------------------------------------

bool GuiControl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
   const Point2I minExtent = getMinExtent();
   Point2I actualNewExtent = Point2I(getMax(minExtent.x, newExtent.x),
      getMax(minExtent.y, newExtent.y));

   // only do the child control resizing stuff if you really need to.
   const RectI bounds = getBounds();
   
   // If we didn't size anything, return false to indicate such
   bool extentChanged = (actualNewExtent != bounds.extent);
   bool positionChanged = (newPosition != bounds.point);
   if (!extentChanged && !positionChanged ) 
      return false;

   // Update Position
   if ( positionChanged )
      mBounds.point = newPosition;

   // Update Extent
   if( extentChanged )
   {
      //call set update both before and after
      setUpdate();

      mBounds.extent = actualNewExtent;

      // Obey the flag!
      // Could be set if we are resizing in response to a child resizing!
      if ( mNotifyChildrenResized )
      {
         iterator i;
         for(i = begin(); i != end(); i++)
         {
            GuiControl *ctrl = static_cast<GuiControl *>(*i);
            ctrl->parentResized(RectI(bounds.point, bounds.extent), RectI(newPosition,actualNewExtent));
         }
      }

      GuiControl *parent = getParent();
      if (parent)
         parent->childResized(this);
      setUpdate();
   }
   
   // We sized something
   if( extentChanged )
      return true;

   // Note : We treat a repositioning as no sizing happening
   //  because parent's should really not need to know when they
   //  have moved, as it should not affect any child sizing since
   //  all child bounds are relative to the parent's 0,0
   return false;

}

//-----------------------------------------------------------------------------

bool GuiControl::setPosition( const Point2I &newPosition )
{
   return resize( newPosition, mBounds.extent );
}

//-----------------------------------------------------------------------------

bool GuiControl::setExtent( const Point2I &newExtent )
{
   return resize( mBounds.point, newExtent );
}

//-----------------------------------------------------------------------------

bool GuiControl::setBounds( const RectI &newBounds )
{
   return resize( newBounds.point, newBounds.extent );
}

//-----------------------------------------------------------------------------

void GuiControl::setLeft( S32 newLeft )
{
   resize( Point2I( newLeft, mBounds.point.y), mBounds.extent );
}

//-----------------------------------------------------------------------------

void GuiControl::setTop( S32 newTop )
{
   resize( Point2I( mBounds.point.x, newTop ), mBounds.extent );
}

//-----------------------------------------------------------------------------

void GuiControl::setWidth( S32 newWidth )
{
   resize( mBounds.point, Point2I( newWidth, mBounds.extent.y ) );
}

//-----------------------------------------------------------------------------

void GuiControl::setHeight( S32 newHeight )
{
   resize( mBounds.point, Point2I( mBounds.extent.x, newHeight ) );
}

//-----------------------------------------------------------------------------

void GuiControl::childResized( GuiControl* )
{
}

//-----------------------------------------------------------------------------

void GuiControl::parentResized(const RectI &oldParentRect, const RectI &newParentRect)
{
   Point2I newPosition = getPosition();
   Point2I newExtent = getExtent();

	S32 deltaX = newParentRect.extent.x - oldParentRect.extent.x;
 	S32 deltaY = newParentRect.extent.y - oldParentRect.extent.y;

	if (mHorizSizing == horizResizeCenter)
	   newPosition.x = (newParentRect.extent.x - getWidth()) >> 1;
	else if (mHorizSizing == horizResizeWidth)
		newExtent.x += deltaX;
	else if (mHorizSizing == horizResizeLeft)
      newPosition.x += deltaX;
   else if (mHorizSizing == horizResizeRelative && oldParentRect.extent.x != 0)
   {
      S32 newLeft = mRoundToNearest( ( F32( newPosition.x ) / F32( oldParentRect.extent.x ) ) * F32( newParentRect.extent.x ) );
      S32 newWidth = mRoundToNearest( ( F32( newExtent.x ) / F32( oldParentRect.extent.x ) ) * F32( newParentRect.extent.x ) );

      newPosition.x = newLeft;
      newExtent.x = newWidth;
   }
   else if (mHorizSizing == horizResizeAspectLeft && oldParentRect.extent.x != 0)
   {
      S32 newLeft = mRoundToNearest((F32(newPosition.x) / F32(oldParentRect.extent.x)) * F32(newParentRect.extent.x));
      S32 newWidth = mRoundToNearest((F32(newExtent.x) / F32(oldParentRect.extent.y)) * F32(newParentRect.extent.y));

      newPosition.x = newLeft;
      newExtent.x = newWidth;
   }
   else if (mHorizSizing == horizResizeAspectRight && oldParentRect.extent.x != 0)
   {
      S32 newLeft = mRoundToNearest((F32(newPosition.x) / F32(oldParentRect.extent.x)) * F32(newParentRect.extent.x));
      S32 newWidth = mRoundToNearest((F32(newExtent.x) / F32(oldParentRect.extent.y)) * F32(newParentRect.extent.y)); //origional aspect ratio corrected width
      S32 rWidth = mRoundToNearest((F32(newExtent.x) / F32(oldParentRect.extent.x)) * F32(newParentRect.extent.x)); //parent aspect ratio relative width

      S32 offset = rWidth - newWidth; // account for change in relative width
      newLeft += offset;
      newPosition.x = newLeft;
      newExtent.x = newWidth;
   }
   else if (mHorizSizing == horizResizeAspectCenter && oldParentRect.extent.x != 0)
   {
      S32 newLeft = mRoundToNearest((F32(newPosition.x) / F32(oldParentRect.extent.x)) * F32(newParentRect.extent.x));
      S32 newWidth = mRoundToNearest((F32(newExtent.x) / F32(oldParentRect.extent.y)) * F32(newParentRect.extent.y)); //origional aspect ratio corrected width
      S32 rWidth = mRoundToNearest((F32(newExtent.x) / F32(oldParentRect.extent.x)) * F32(newParentRect.extent.x)); //parent aspect ratio relative width

      S32 offset = rWidth - newWidth; // account for change in relative width
      newLeft += offset/2;
      newPosition.x = newLeft;
      newExtent.x = newWidth;
   }

	if (mVertSizing == vertResizeCenter)
	   newPosition.y = (newParentRect.extent.y - getHeight()) >> 1;
	else if (mVertSizing == vertResizeHeight)
		newExtent.y += deltaY;
	else if (mVertSizing == vertResizeTop)
      newPosition.y += deltaY;
   else if(mVertSizing == vertResizeRelative && oldParentRect.extent.y != 0)
   {
      S32 newTop = mRoundToNearest( ( F32( newPosition.y ) / F32( oldParentRect.extent.y ) ) * F32( newParentRect.extent.y ) );
      S32 newHeight = mRoundToNearest( ( F32( newExtent.y ) / F32( oldParentRect.extent.y ) ) * F32( newParentRect.extent.y ) );

      newPosition.y = newTop;
      newExtent.y = newHeight;
   }
   else if (mVertSizing == vertResizeAspectTop && oldParentRect.extent.y != 0)
   {
      S32 newTop = mRoundToNearest((F32(newPosition.y) / F32(oldParentRect.extent.y)) * F32(newParentRect.extent.y));
      S32 newHeight = mRoundToNearest((F32(newExtent.y) / F32(oldParentRect.extent.x)) * F32(newParentRect.extent.x));

      newPosition.y = newTop;
      newExtent.y = newHeight;
   }
   else if (mVertSizing == vertResizeAspectBottom && oldParentRect.extent.y != 0)
   {
      S32 newTop = mRoundToNearest((F32(newPosition.y) / F32(oldParentRect.extent.y)) * F32(newParentRect.extent.y));
      S32 newHeight = mRoundToNearest((F32(newExtent.y) / F32(oldParentRect.extent.x)) * F32(newParentRect.extent.x)); //origional aspect ratio corrected hieght
      S32 rHeight = mRoundToNearest((F32(newExtent.y) / F32(oldParentRect.extent.y)) * F32(newParentRect.extent.y)); //parent aspect ratio relative hieght

      S32 offset = rHeight - newHeight; // account for change in relative hieght
      newTop += offset;
      newPosition.y = newTop;
      newExtent.y = newHeight;
   }
   else if (mVertSizing == vertResizeAspectCenter && oldParentRect.extent.y != 0)
   {
      S32 newTop = mRoundToNearest((F32(newPosition.y) / F32(oldParentRect.extent.y)) * F32(newParentRect.extent.y));
      S32 newHeight = mRoundToNearest((F32(newExtent.y) / F32(oldParentRect.extent.x)) * F32(newParentRect.extent.x)); //origional aspect ratio corrected hieght
      S32 rHeight = mRoundToNearest((F32(newExtent.y) / F32(oldParentRect.extent.y)) * F32(newParentRect.extent.y)); //parent aspect ratio relative hieght

      S32 offset = rHeight - newHeight; // account for change in relative hieght
      newTop += offset / 2;
      newPosition.y = newTop;
      newExtent.y = newHeight;
   }

   // Resizing Re factor [9/18/2006]
   // Only resize if our minExtent is satisfied with it.
   Point2I minExtent = getMinExtent();
   if( newExtent.x >= minExtent.x && newExtent.y >= minExtent.y )
      resize(newPosition, newExtent);
}

//-----------------------------------------------------------------------------

Point2I GuiControl::localToGlobalCoord(const Point2I &src)
{
   Point2I ret = src;
   ret += getPosition();
   GuiControl *walk = getParent();
   while(walk)
   {
      ret += walk->getPosition();
      walk = walk->getParent();
   }
   return ret;
}

//-----------------------------------------------------------------------------

Point2I GuiControl::globalToLocalCoord(const Point2I &src)
{
   Point2I ret = src;
   ret -= getPosition();
   GuiControl *walk = getParent();
   while(walk)
   {
      ret -= walk->getPosition();
      walk = walk->getParent();
   }
   return ret;
}

//-----------------------------------------------------------------------------

bool GuiControl::cursorInControl()
{
   GuiCanvas *root = getRoot();
   if (! root) return false;

   Point2I pt = root->getCursorPos();
   pt = root->getPlatformWindow() ? root->getPlatformWindow()->screenToClient(pt) : pt;
   Point2I extent = getExtent();
   Point2I offset = localToGlobalCoord(Point2I(0, 0));
   if (pt.x >= offset.x && pt.y >= offset.y &&
      pt.x < offset.x + extent.x && pt.y < offset.y + extent.y)
   {
      return true;
   }
   else
   {
      return false;
   }
}

//-----------------------------------------------------------------------------

bool GuiControl::pointInControl(const Point2I& parentCoordPoint)
{
   const RectI &bounds = getBounds();
   S32 xt = parentCoordPoint.x - bounds.point.x;
   S32 yt = parentCoordPoint.y - bounds.point.y;
   return xt >= 0 && yt >= 0 && xt < bounds.extent.x && yt < bounds.extent.y;
}

//=============================================================================
//    Properties.
//=============================================================================
// MARK: ---- Properties ----

//-----------------------------------------------------------------------------

void GuiControl::setTooltipProfile( GuiControlProfile *prof )
{
   AssertFatal( prof, "GuiControl::setTooltipProfile: invalid profile" );

   if ( prof == mTooltipProfile )
      return;

   bool skipAwaken = false;

   if ( mTooltipProfile == NULL )
      skipAwaken = true;
      
   if( mTooltipProfile )
      mTooltipProfile->decUseCount();

   if ( mAwake && mTooltipProfile )
      mTooltipProfile->decLoadCount();

   // Clear the delete notification we previously set up
   if ( mTooltipProfile )
      clearNotify( mTooltipProfile );

   mTooltipProfile = prof;
   mTooltipProfile->incUseCount();
   if ( mAwake )
      mTooltipProfile->incLoadCount();

   // Make sure that the new profile will notify us when it is deleted
   if ( mTooltipProfile )
      deleteNotify( mTooltipProfile );

   // force an update when the profile is changed
   if ( mAwake && !skipAwaken )
   {
      sleep();
      
      if( !Sim::isShuttingDown() )
         awaken();
   }
}

//-----------------------------------------------------------------------------

void GuiControl::setControlProfile( GuiControlProfile *prof )
{
   AssertFatal( prof, "GuiControl::setControlProfile: invalid profile" );

   if ( prof == mProfile )
      return;

   bool skipAwaken = false;

   if ( mProfile == NULL )
      skipAwaken = true;
      
   if( mProfile )
      mProfile->decUseCount();

   if ( mAwake && mProfile )
      mProfile->decLoadCount();

   // Clear the delete notification we previously set up
   if ( mProfile )
      clearNotify( mProfile );

   mProfile = prof;
   mProfile->incUseCount();
   if ( mAwake )
      mProfile->incLoadCount();

   // Make sure that the new profile will notify us when it is deleted
   if ( mProfile )
      deleteNotify( mProfile );

   // force an update when the profile is changed
   if ( mAwake && !skipAwaken )
   {
      sleep();
      
      if( !Sim::isShuttingDown() )
         awaken();
   }
}

//-----------------------------------------------------------------------------

bool GuiControl::setProfileProt( void *object, const char *index, const char *data )
{
   GuiControl* ctrl = static_cast<GuiControl*>( object );   
   GuiControlProfile *prof = dynamic_cast<GuiControlProfile*>( Sim::findObject( data ) );
   if ( prof == NULL )
      return false;   
               
   // filter through our setter, for consistency
   ctrl->setControlProfile( prof );

   // ask the console not to set the data, because we've already set it
   return false;
}

//-----------------------------------------------------------------------------

bool GuiControl::setTooltipProfileProt( void *object, const char *index, const char *data )
{
   GuiControl* ctrl = static_cast<GuiControl*>( object );   
   GuiControlProfile *prof = dynamic_cast<GuiControlProfile*>( Sim::findObject( data ) );
   if ( prof == NULL )
      return false;   

   // filter through our setter, for consistency
   ctrl->setTooltipProfile( prof );

   // ask the console not to set the data, because we've already set it
   return false;
}

//-----------------------------------------------------------------------------


const char *GuiControl::getScriptValue()
{
   return NULL;
}

//-----------------------------------------------------------------------------

void GuiControl::setScriptValue( const char* )
{
}

//-----------------------------------------------------------------------------

void GuiControl::setConsoleVariable(const char *variable)
{
   if (variable)
   {
      mConsoleVariable = StringTable->insert(variable);
   }
   else
   {
      mConsoleVariable = StringTable->EmptyString();
   }
}
  
//-----------------------------------------------------------------------------

void GuiControl::setConsoleCommand( const String& newCmd )
{
   mConsoleCommand = newCmd;
}

//-----------------------------------------------------------------------------

const char * GuiControl::getConsoleCommand()
{
	return mConsoleCommand;
}

//-----------------------------------------------------------------------------

void GuiControl::setVariable(const char *value)
{
   if (mConsoleVariable[0])
      Con::setVariable(mConsoleVariable, value);
}

//-----------------------------------------------------------------------------

void GuiControl::setIntVariable(S32 value)
{
   if (mConsoleVariable[0])
      Con::setIntVariable(mConsoleVariable, value);
}

//-----------------------------------------------------------------------------

void GuiControl::setFloatVariable(F32 value)
{
   if (mConsoleVariable[0])
      Con::setFloatVariable(mConsoleVariable, value);
}

//-----------------------------------------------------------------------------

const char * GuiControl::getVariable()
{
   if (mConsoleVariable[0])
      return Con::getVariable(mConsoleVariable);
   else return NULL;
}

//-----------------------------------------------------------------------------

S32 GuiControl::getIntVariable()
{
   if (mConsoleVariable[0])
      return Con::getIntVariable(mConsoleVariable);
   else return 0;
}

//-----------------------------------------------------------------------------

F32 GuiControl::getFloatVariable()
{
   if (mConsoleVariable[0])
      return Con::getFloatVariable(mConsoleVariable);
   else return 0.0f;
}

//-----------------------------------------------------------------------------

void GuiControl::setVisible(bool value)
{
	mVisible = value;

   setUpdate();

   for( iterator i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->clearFirstResponder();
	}

	GuiControl *parent = getParent();
	if( parent )
   {
	   parent->childResized( this );
      
      // If parent is visible and awake and this control has just
      // become visible but was sleeping, wake it up now.
      
      if(    parent->isVisible() && parent->isAwake()
          && this->isVisible() && !this->isAwake() )
         awaken();
   }
   
   if( getNamespace() ) // May be called during construction.
      onVisible_callback( value );
}

//-----------------------------------------------------------------------------

void GuiControl::setActive( bool value )
{
   if( mActive == value )
      return;
      
   mActive = value;

   if ( !mActive )
      clearFirstResponder();

   if ( mVisible && mAwake )
      setUpdate();
      
   if( getNamespace() ) // May be called during construction.
      onActive_callback( value );
      
   // Pass activation on to children.
      
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      GuiControl* child = dynamic_cast< GuiControl* >( *iter );
      if( child )
         child->setActive( value );
   }
}

//=============================================================================
//    Persistence.
//=============================================================================
// MARK: ---- Persistence ----

//-----------------------------------------------------------------------------

bool GuiControl::getCanSaveParent()
{
   GuiControl *walk = this;
   while(walk)
   {
      if(!walk->getCanSave())
         return false;

      walk = walk->getParent();
   }

   return true;
}

//-----------------------------------------------------------------------------

void GuiControl::write(Stream &stream, U32 tabStop, U32 flags)
{
   //note: this will return false if either we, or any of our parents, are non-save controls
   bool bCanSave	= ( flags & IgnoreCanSave ) || ( flags & NoCheckParentCanSave && getCanSave() ) || getCanSaveParent();
   
   if (bCanSave && mAddGroup)
   {
      StringTableEntry steName = mAddGroup->getInternalName();

      if ((steName != NULL) && (steName != StringTable->insert("null")) && getName())
      {
         MutexHandle handle;
         handle.lock(mMutex);

         // export selected only?
         if ((flags & SelectedOnly) && !isSelected())
         {
            for (U32 i = 0; i < size(); i++)
               (*this)[i]->write(stream, tabStop, flags);

            return;

         }

         stream.writeTabs(tabStop);
         char buffer[1024];
         dSprintf(buffer, sizeof(buffer), "new %s(%s,%s) {\r\n", getClassName(), getName() ? getName() : "", mAddGroup->getInternalName());
         stream.write(dStrlen(buffer), buffer);
         writeFields(stream, tabStop + 1);

         if (size())
         {
            stream.write(2, "\r\n");
            for (U32 i = 0; i < size(); i++)
               (*this)[i]->write(stream, tabStop + 1, flags);
         }

         stream.writeTabs(tabStop);
         stream.write(4, "};\r\n");

         return;
      }
   }
   
   if (bCanSave)
      Parent::write( stream, tabStop, flags );
}

//=============================================================================
//    Hierarchies.
//=============================================================================
// MARK: ---- Hierarchies ----

//-----------------------------------------------------------------------------

void GuiControl::addObject(SimObject *object)
{
   GuiControl *ctrl = dynamic_cast<GuiControl *>(object);
   if(object->getGroup() == this)
      return;

   AssertFatal( ctrl, "GuiControl::addObject() - cannot add non-GuiControl as child of GuiControl" );

	Parent::addObject(object);

   AssertFatal(!ctrl->isAwake(), "GuiControl::addObject: object is already awake before add");
   if( mAwake )
      ctrl->awaken();

  // If we are a child, notify our parent that we've been removed
  GuiControl *parent = ctrl->getParent();
  if( parent )
     parent->onChildAdded( ctrl );
}

//-----------------------------------------------------------------------------

void GuiControl::removeObject(SimObject *object)
{
   GuiControl* ctrl = static_cast< GuiControl* >( object );
   if( mAwake && ctrl->isAwake() )
      ctrl->sleep();

   onChildRemoved( ctrl );

   Parent::removeObject(object);
}

//-----------------------------------------------------------------------------

GuiControl *GuiControl::getParent()
{
	SimObject *obj = getGroup();
	if (GuiControl* gui = dynamic_cast<GuiControl*>(obj))
      return gui;
   return 0;
}

//-----------------------------------------------------------------------------

GuiCanvas *GuiControl::getRoot()
{
   GuiControl *root = NULL;
	GuiControl *parent = getParent();
   while (parent)
   {
      root = parent;
      parent = parent->getParent();
   }
   if (root)
      return dynamic_cast<GuiCanvas*>(root);
   else
      return NULL;
}

//-----------------------------------------------------------------------------

bool GuiControl::acceptsAsChild( SimObject* object ) const
{
   return ( dynamic_cast< GuiControl* >( object ) != NULL );
}

//-----------------------------------------------------------------------------

GuiControl* GuiControl::findHitControl(const Point2I &pt, S32 initialLayer)
{
   iterator i = end(); // find in z order (last to first)

   while (i != begin())
   {
      i--;
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      if (initialLayer >= 0 && ctrl->mLayer > initialLayer)
      {
         continue;
      }

      else if (ctrl->mVisible && ctrl->mCanHit && ctrl->pointInControl(pt))
      {
         Point2I ptemp = pt - ctrl->getPosition();
         GuiControl *hitCtrl = ctrl->findHitControl(ptemp);

         if ( hitCtrl->mProfile->mModal )
            return hitCtrl;
      }
   }

   if( mCanHit )
      return this;
   return NULL;
}

//-----------------------------------------------------------------------------

bool GuiControl::findHitControls( const RectI& rect, Vector< GuiControl* >& outResult, U32 flags, S32 initialLayer, U32 depth )
{
   if( !mVisible )
      return false;
   else if( !mCanHit && flags & HIT_NoCanHitNoRecurse )
      return false;
      
   // Check for hit.  If not full-box, always counts.
      
   bool isHit = mVisible;
   if( flags & HIT_FullBoxOnly )
   {
      RectI rectInParentSpace = rect;
      rectInParentSpace.point += getPosition();
      
      isHit &= rectInParentSpace.contains( getBounds() );
   }
   else
      isHit &= mCanHit;
      
   // If we have a hit and should not recurse into children,
   // return us.
   
   if( isHit && flags & HIT_ParentPreventsChildHit && depth > 0 )
   {
      outResult.push_back( this );
      return true;
   }
   
   // Check child controls.
   
   bool haveFoundChild = false;
   iterator i = end();
   
   while( i != begin() )
   {
      i --;
      
      GuiControl* ctrl = static_cast< GuiControl* >( *i );
      if( initialLayer >= 0 && ctrl->mLayer > initialLayer )
         continue;

      if( ctrl->getBounds().overlaps( rect ) )
      {
         RectI transposedRect = rect;
         transposedRect.point -= ctrl->getPosition();
         
         if( ctrl->findHitControls( transposedRect, outResult, flags, -1, depth + 1 ) )
            haveFoundChild = true;
      }
   }
   
   if( ( !haveFoundChild || flags & HIT_AddParentHits ) && isHit )
   {
      outResult.push_back( this );
      return true;
   }

   return haveFoundChild;
}

//-----------------------------------------------------------------------------

GuiControl* GuiControl::findFirstTabable()
{
   // No tabbing if the control is disabled or hidden.
   if ( !mAwake || !mVisible )
      return NULL;

   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findFirstTabable();
      if (tabCtrl)
      {
         mFirstResponder = tabCtrl;
         return tabCtrl;
      }
   }

   //nothing was found, therefore, see if this ctrl is tabable
   return ( mProfile != NULL ) ? ( ( mProfile->mTabable && mAwake && mVisible ) ? this : NULL ) : NULL;
}

//-----------------------------------------------------------------------------

GuiControl* GuiControl::findLastTabable(bool firstCall)
{
   // No tabbing if the control is disabled or hidden.
   if ( !mAwake || !mVisible )
      return NULL;

   //if this is the first call, clear the global
   if (firstCall)
      smPrevResponder = NULL;

   //if this control is tabable, set the global
   if (mProfile->mTabable)
      smPrevResponder = this;

   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->findLastTabable(false);
   }

   //after the entire tree has been traversed, return the last responder found
   mFirstResponder = smPrevResponder;
   return smPrevResponder;
}

//-----------------------------------------------------------------------------

GuiControl *GuiControl::findNextTabable(GuiControl *curResponder, bool firstCall)
{
   // No tabbing if the control is disabled or hidden.
   if ( !mAwake || !mVisible )
      return NULL;

   //if this is the first call, clear the global
   if (firstCall)
      smCurResponder = NULL;

   //first find the current responder
   if (curResponder == this)
      smCurResponder = this;

   //if the first responder has been found, return the very next *tabable* control
   else if ( smCurResponder && mProfile->mTabable && mAwake && mVisible && mActive )
      return( this );

   //loop through, checking each child to see if it is the one that follows the firstResponder
   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findNextTabable(curResponder, false);
      if (tabCtrl) break;
   }
   mFirstResponder = tabCtrl;
   return tabCtrl;
}

//-----------------------------------------------------------------------------

GuiControl *GuiControl::findPrevTabable(GuiControl *curResponder, bool firstCall)
{
   // No tabbing if the control is disabled or hidden.
   if ( !mAwake || !mVisible )
      return NULL;

   if (firstCall)
      smPrevResponder = NULL;

   //if this is the current reponder, return the previous one
   if (curResponder == this)
      return smPrevResponder;

   //else if this is a responder, store it in case the next found is the current responder
   else if ( mProfile->mTabable && mAwake && mVisible && mActive )
      smPrevResponder = this;

   //loop through, checking each child to see if it is the one that follows the firstResponder
   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findPrevTabable(curResponder, false);
      if (tabCtrl) break;
   }
   mFirstResponder = tabCtrl;
   return tabCtrl;
}

//-----------------------------------------------------------------------------

bool GuiControl::controlIsChild( GuiControl* child )
{
   for( iterator i = begin(); i != end(); ++ i )
   {
      GuiControl* ctrl = static_cast< GuiControl* >( *i );
      if( ctrl == child || ctrl->controlIsChild( child ) )
         return true;
   }

   return false;
}

//=============================================================================
//    Event Handling.
//=============================================================================
// MARK: ---- Event Handling ----

//-----------------------------------------------------------------------------

bool GuiControl::isFirstResponder()
{
   GuiCanvas *root = getRoot();
   return root && root->getFirstResponder() == this;
}

//-----------------------------------------------------------------------------

void GuiControl::makeFirstResponder(bool value)
{
   if ( value )
      //setFirstResponder(this);
      setFirstResponder();
   else
      clearFirstResponder();
}

//-----------------------------------------------------------------------------

void GuiControl::setFirstResponder( GuiControl* firstResponder )
{
   // If the control cannot have keyboard focus, refuse
   // to make it first responder.
   
   if( firstResponder && !firstResponder->mProfile->mCanKeyFocus )
      return;
      
   mFirstResponder = firstResponder;
   
   if( getParent() )
      getParent()->setFirstResponder( firstResponder );
}

//-----------------------------------------------------------------------------

void GuiControl::setFirstResponder()
{
	if( mAwake && mVisible )
	{
	   GuiControl *parent = getParent();
	   if ( mProfile->mCanKeyFocus == true && parent != NULL )
         parent->setFirstResponder( this );
	}
}

//-----------------------------------------------------------------------------

void GuiControl::clearFirstResponder()
{
   if( !getParent() )
      return;
      
   if( isFirstResponder() )
      getParent()->setFirstResponder( NULL );
   else
      for( GuiControl* parent = this; parent != NULL; parent = parent->getParent() )
         if( parent->mFirstResponder == this )
            parent->mFirstResponder = NULL;
}

//-----------------------------------------------------------------------------

void GuiControl::onLoseFirstResponder()
{
	// Since many controls have visual cues when they are the firstResponder...
	setUpdate();

   onLoseFirstResponder_callback();
}

//-----------------------------------------------------------------------------

void GuiControl::onGainFirstResponder()
{
   // Since many controls have visual cues when they are the firstResponder...
   this->setUpdate();

   onGainFirstResponder_callback();
}

//-----------------------------------------------------------------------------

void GuiControl::buildAcceleratorMap()
{
   //add my own accel key
   addAcceleratorKey();

   //add all my childrens keys
   iterator i;
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->buildAcceleratorMap();
   }
}

//-----------------------------------------------------------------------------

void GuiControl::addAcceleratorKey()
{
   //see if we have an accelerator
   if( mAcceleratorKey == StringTable->EmptyString() )
      return;

   EventDescriptor accelEvent;
   ActionMap::createEventDescriptor(mAcceleratorKey, &accelEvent);

   //now we have a modifier, and a key, add them to the canvas
   GuiCanvas *root = getRoot();
   if (root)
      root->addAcceleratorKey(this, 0, accelEvent.eventCode, accelEvent.flags);
}

//-----------------------------------------------------------------------------

void GuiControl::acceleratorKeyPress( U32 )
{
   onAction();
}

//-----------------------------------------------------------------------------

void GuiControl::acceleratorKeyRelease( U32 )
{
   //do nothing
}

//-----------------------------------------------------------------------------

bool GuiControl::isMouseLocked()
{
   GuiCanvas *root = getRoot();
   return root ? root->getMouseLockedControl() == this : false;
}

//-----------------------------------------------------------------------------

void GuiControl::mouseLock(GuiControl *lockingControl)
{
   GuiCanvas *root = getRoot();
   if (root)
      root->mouseLock(lockingControl);
}

//-----------------------------------------------------------------------------

void GuiControl::mouseLock()
{
   GuiCanvas *root = getRoot();
   if (root)
      root->mouseLock(this);
}

//-----------------------------------------------------------------------------

void GuiControl::mouseUnlock()
{
   GuiCanvas *root = getRoot();
   if (root)
      root->mouseUnlock(this);
}

//=============================================================================
//    Misc.
//=============================================================================
// MARK: ---- Misc ----

//-----------------------------------------------------------------------------

LangTable * GuiControl::getGUILangTable()
{
	if(mLangTable)
		return mLangTable;

	if(mLangTableName && *mLangTableName)
	{
		mLangTable = (LangTable *)getModLangTable((const UTF8*)mLangTableName);
		return mLangTable;
	}

	GuiControl *parent = getParent();
	if(parent)
		return parent->getGUILangTable();

	return NULL;
}

//-----------------------------------------------------------------------------

const UTF8 * GuiControl::getGUIString(S32 id)
{
	LangTable *lt = getGUILangTable();
	if(lt)
		return lt->getString(id);

	return NULL;
}

//-----------------------------------------------------------------------------

void GuiControl::messageSiblings(S32 message)
{
   GuiControl *parent = getParent();
   if (! parent) return;
   GuiControl::iterator i;
   for(i = parent->begin(); i != parent->end(); i++)
   {
      GuiControl *ctrl = dynamic_cast<GuiControl *>(*i);
      if (ctrl != this)
         ctrl->onMessage(this, message);
   }
}

//-----------------------------------------------------------------------------

void GuiControl::getScrollLineSizes(U32 *rowHeight, U32 *columnWidth)
{
	// default to 10 pixels in y, 30 pixels in x
	*columnWidth = 30;
	*rowHeight = 30;
}

//-----------------------------------------------------------------------------

U32 GuiControl::clipText( String &text, U32 clipWidth ) const
{
   PROFILE_SCOPE( GuiControl_clipText );

   U32 textWidth = mProfile->mFont->getStrWidthPrecise( text );

   if ( textWidth <= clipWidth )         
      return textWidth;   

   // Start removing characters from the end of the string
   // until the string width plus the elipsesWidth is less
   // than clipWidth...

   // Note this would be more efficient without calling 
   // getStrWidthPrecise each loop iteration. eg. get the 
   // length of each char, store in a temporary U32 array,
   // and then remove the number we need from the end all at once.

   String temp;

   while ( text.isNotEmpty() )
   {
      text.erase( text.length() - 1, 1 );
      temp = text;
      temp += "...";
      textWidth = mProfile->mFont->getStrWidthPrecise( temp );

      if ( textWidth <= clipWidth )
      {
         text = temp;
         return textWidth;
      }
   }

   // Uh, not even the ellipses will fit in the passed width.
   // Text should be an ellipses string now, 
   // which is the right thing to do in this case.

   return 0;
}

//-----------------------------------------------------------------------------

void GuiControl::getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent)
{
#ifdef _XBOX
   return;
#endif

   TORQUE_UNUSED(lastGuiEvent);

   if( !getRoot() )
      return;

   if(getRoot()->mCursorChanged != -1 && !isMouseLocked())
   {
      // We've already changed the cursor, 
      // so set it back before we change it again.

      PlatformWindow *pWindow = static_cast<GuiCanvas*>(getRoot())->getPlatformWindow();
      if (!pWindow)
         return;
      PlatformCursorController *pController = pWindow->getCursorController();
      AssertFatal(pController != NULL,"PlatformWindow without an owned CursorController!");

      pController->popCursor();

      // We haven't changed it
      getRoot()->mCursorChanged = -1;
   }
}

//-----------------------------------------------------------------------------

const char* GuiControl::evaluate( const char* str )
{
   smThisControl = this;
   const char* result = Con::evaluate(str, false);
   smThisControl = NULL;

   return result;
}

//-----------------------------------------------------------------------------

const char* GuiControl::execConsoleCallback()
{
   if( mConsoleCommand.isNotEmpty() )
      return evaluate( mConsoleCommand );

   return "";
}

//-----------------------------------------------------------------------------

const char* GuiControl::execAltConsoleCallback()
{
   if( mAltConsoleCommand.isNotEmpty() )
      return evaluate( mAltConsoleCommand );

   return "";
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, findHitControl, GuiControl*, ( S32 x, S32 y ),,
   "Find the topmost child control located at the given coordinates.\n"
   "@note Only children that are both visible and have the 'modal' flag set in their profile will be considered in the search."
   "@param x The X coordinate in the control's own coordinate space.\n"
   "@param y The Y coordinate in the control's own coordinate space.\n"
   "@return The topmost child control at the given coordintes or the control on which the method was called if no matching child could be found.\n"
   "@see GuiControlProfile::modal\n"
   "@see findHitControls" )
{
   return object->findHitControl( Point2I( x, y ) );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, findHitControls, const char*, ( S32 x, S32 y, S32 width, S32 height ),,
   "Find all visible child controls that intersect with the given rectangle.\n"
   "@note Invisible child controls will not be included in the search.\n"
   "@param x The X coordinate of the rectangle's upper left corner in the control's own coordinate space.\n"
   "@param y The Y coordinate of the rectangle's upper left corner in the control's own coordinate space.\n"
   "@param width The width of the search rectangle in pixels.\n"
   "@param height The height of the search rectangle in pixels.\n"
   "@return A space-separated list of the IDs of all visible control objects intersecting the given rectangle.\n\n"
   "@tsexample\n"
   "// Lock all controls in the rectangle at x=10 and y=10 and the extent width=100 and height=100.\n"
   "foreach$( %ctrl in %this.findHitControls( 10, 10, 100, 100 ) )\n"
   "   %ctrl.setLocked( true );\n"
   "@endtsexample\n"
   "@see findHitControl" )
{
   // Find hit controls.
   
   RectI bounds( x, y, width, height );
   Vector< GuiControl* > controls;
   
   if( !object->findHitControls( bounds, controls ) )
      return "";
      
   // Create vector string.

   bool isFirst = true;
   StringBuilder str;
   for( U32 i = 0, num = controls.size(); i < num; ++ i )
   {
      if( !isFirst )
         str.append( ' ' );
         
      str.append( controls[ i ]->getIdString() );
      isFirst = false;
   }
   String s = str.end();
   
   // Return result.

   if ( s.compare( object->getIdString() ) == 0 )
      return "";
   
   char* buffer = Con::getReturnBuffer( s.size() );
   dStrcpy( buffer, s.c_str() );
   
   return buffer;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, controlIsChild, bool, ( GuiControl* control ),,
   "Test whether the given control is a direct or indirect child to this control.\n"
   "@param control The potential child control.\n"
   "@return True if the given control is a direct or indirect child to this control." )
{
   if( !control )
      return false;
      
   return object->controlIsChild( control );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, isFirstResponder, bool, (),,
   "Test whether the control is the current first responder.\n"
   "@return True if the control is the current first responder.\n"
   "@see makeFirstResponder\n"
   "@see setFirstResponder\n"
   "@ref GuiControl_FirstResponders" )
{
   return object->isFirstResponder();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, setFirstResponder, void, (),,
   "Make this control the current first responder.\n"
   "@note Only controls with a profile that has canKeyFocus enabled are able to become first responders.\n"
   "@see GuiControlProfile::canKeyFocus\n"
   "@see isFirstResponder\n"
   "@ref GuiControl_FirstResponders" )
{
   object->setFirstResponder();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, getFirstResponder, GuiControl*, (),,
   "Get the first responder set on this GuiControl tree.\n"
   "@return The first responder set on the control's subtree.\n"
   "@see isFirstResponder\n"
   "@see makeFirstResponder\n"
   "@see setFirstResponder\n"
   "@ref GuiControl_FirstResponders" )
{
   return object->getFirstResponder();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, clearFirstResponder, void, ( bool ignored ), ( false ),
   "Clear this control from being the first responder in its hierarchy chain.\n"
   "@param ignored Ignored.  Supported for backwards-compatibility.\n" )
{
   object->clearFirstResponder();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, pointInControl, bool, ( S32 x, S32 y ),,
   "Test whether the given point lies within the rectangle of the control.\n"
   "@param x X coordinate of the point in parent-relative coordinates.\n"
   "@param y Y coordinate of the point in parent-relative coordinates.\n"
   "@return True if the point is within the control, false if not.\n"
   "@see getExtent\n"
   "@see getPosition\n" )
{
   return object->pointInControl( Point2I( x, y ) );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, addGuiControl, void, ( GuiControl* control ),,
   "Add the given control as a child to this control.\n"
   "This is synonymous to calling SimGroup::addObject.\n"
   "@param control The control to add as a child.\n"
   "@note The control will retain its current position and size.\n"
   "@see SimGroup::addObject\n"
   "@ref GuiControl_Hierarchy\n" )
{
   if( control )
      object->addObject( control );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, getRoot, GuiCanvas*, (),,
   "Get the canvas on which the control is placed.\n"
   "@return The canvas on which the control's hierarchy is currently placed or 0 if the control is not currently placed on a GuiCanvas.\n"
   "@see GuiControl_Hierarchy\n" )
{
   return object->getRoot();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, getParent, GuiControl*, (),,
   "Get the immediate parent control of the control.\n"
   "@return The immediate parent GuiControl or 0 if the control is not parented to a GuiControl.\n" )
{
   return object->getParent();
}

//-----------------------------------------------------------------------------
DefineEngineMethod( GuiControl, isMouseLocked, bool, (),,
   "Indicates if the mouse is locked in this control.\n"
   "@return True if the mouse is currently locked.\n" )
{
   return object->isMouseLocked();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, setValue, void, ( const char* value ),,
   "Set the value associated with the control.\n"
   "@param value The new value for the control.\n" )
{
   object->setScriptValue( value );
}

DefineEngineMethod( GuiControl, getValue, const char*, (),,
   "Get the value associated with the control.\n"
   "@return value for the control.\n" )
{
   return object->getScriptValue();
}

DefineEngineMethod( GuiControl, makeFirstResponder, void, ( bool isFirst ),,
   "Make this control the first responder.\n"
   "@param isFirst True to make first responder, false to not.\n" )
{
   //object->makeFirstResponder(dAtob(argv[2]));
   object->makeFirstResponder(isFirst);
}

DefineEngineMethod( GuiControl, isActive, bool, (),,
   "Check if this control is active or not.\n"
   "@return True if it's active, false if not.\n" )
{
   return object->isActive();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, setActive, void, ( bool state ), ( true ),
   "Set the control as active or inactive."
   "@param state True to set the control as active, false to set it as inactive.")
{
   object->setActive( state );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, isVisible, bool, (),,
   "Test whether the control is currently set to be visible.\n"
   "@return True if the control is currently set to be visible."
   "@note This method does not tell anything about whether the control is actually visible to "
      "the user at the moment.\n\n"
   "@ref GuiControl_VisibleActive" )
{
   return object->isVisible();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, setVisible, void, ( bool state ), ( true ),
   "Set whether the control is visible or not.\n"
   "@param state The new visiblity flag state for the control.\n"
   "@ref GuiControl_VisibleActive" )
{
   object->setVisible( state );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, isAwake, bool, (),,
   "Test whether the control is currently awake.\n"
   "If a control is awake it means that it is part of the GuiControl hierarchy of a GuiCanvas.\n"
   "@return True if the control is awake."
   "@ref GuiControl_Waking" )
{
   return object->isAwake();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, setProfile, void, ( GuiControlProfile* profile ),,
   "Set the control profile for the control to use.\n"
   "The profile used by a control determines a great part of its behavior and appearance.\n"
   "@param profile The new profile the control should use.\n"
   "@ref GuiControl_Profiles" )
{
   if( !profile )
      return;
      
   object->setControlProfile( profile );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, resize, void, ( S32 x, S32 y, S32 width, S32 height ),,
   "Resize and reposition the control using the give coordinates and dimensions.  Child controls "
   "will resize according to their layout behaviors.\n"
   "@param x The new X coordinate of the control in its parent's coordinate space.\n"
   "@param y The new Y coordinate of the control in its parent's coordinate space.\n"
   "@param width The new width to which the control should be resized.\n"
   "@param height The new height to which the control should be resized." )
{
   Point2I newPos( x, y );
   Point2I newExt( width, height );
   object->resize(newPos, newExt);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, getPosition, Point2I, (),,
   "Get the control's current position relative to its parent.\n"
   "@return The coordinate of the control in its parent's coordinate space." )
{
   return object->getPosition();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, getCenter, Point2I, (),,
   "Get the coordinate of the control's center point relative to its parent.\n"
   "@return The coordinate of the control's center point in parent-relative coordinates." )
{
   const Point2I pos = object->getPosition();
   const Point2I ext = object->getExtent();
   Point2I center( pos.x + ext.x / 2, pos.y + ext.y / 2 );
   
   return center;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, setCenter, void, ( S32 x, S32 y ),,
   "Set the control's position by its center point.\n"
   "@param x The X coordinate of the new center point of the control relative to the control's parent.\n"
   "@param y The Y coordinate of the new center point of the control relative to the control's parent." )
{
   const Point2I ext = object->getExtent();
   Point2I newpos( x - ext.x / 2, y - ext.y / 2 );
   object->setPosition( newpos );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, getGlobalCenter, Point2I, (),,
   "Get the coordinate of the control's center point in coordinates relative to the root control in its control hierarchy.\n"
   "@Return the center coordinate of the control in root-relative coordinates.\n" )
{
   const Point2I tl( 0, 0 );
   Point2I pos = object->localToGlobalCoord( tl );
   const Point2I ext = object->getExtent();
   Point2I center( pos.x + ext.x / 2, pos.y + ext.y / 2 );
   
   return center;
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, getGlobalPosition, Point2I, (),,
   "Get the position of the control relative to the root of the GuiControl hierarchy it is contained in.\n"
   "@return The control's current position in root-relative coordinates." )
{
   const Point2I pos(0,0);
   return object->localToGlobalCoord(pos);
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, setPositionGlobal, void, ( S32 x, S32 y ),,
   "Set position of the control relative to the root of the GuiControl hierarchy it is contained in.\n"
   "@param x The new X coordinate of the control relative to the root's upper left corner.\n"
   "@param y The new Y coordinate of the control relative to the root's upper left corner." )
{
   //see if we can turn the x/y into ints directly, 
   Point2I lPosOffset	=	object->globalToLocalCoord( Point2I( x, y ) );
   
   lPosOffset += object->getPosition();
   
   object->setPosition( lPosOffset );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, setPosition, void, ( S32 x, S32 y ),,
   "Position the control in the local space of the parent control.\n"
   "@param x The new X coordinate of the control relative to its parent's upper left corner.\n"
   "@param y The new Y coordinate of the control relative to its parent's upper left corner." )
{
   object->setPosition( Point2I( x, y ) );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, getExtent, Point2I, (),,
   "Get the width and height of the control.\n"
   "@return A point structure containing the width of the control in x and the height in y." )
{
   return object->getExtent();
}

//-----------------------------------------------------------------------------

static ConsoleDocFragment _sGuiControlSetExtent1(
   "@brief Resize the control to the given dimensions.\n\n"
   "Child controls will resize according to their layout settings.\n"
   "@param width The new width of the control in pixels.\n"
   "@param height The new height of the control in pixels.",
   "GuiControl", // The class to place the method in; use NULL for functions.
   "void setExtent( S32 width, S32 height );" ); // The definition string.
static ConsoleDocFragment _sGuiControlSetExtent2(
   "@brief Resize the control to the given dimensions.\n\n"
   "Child controls with resize according to their layout settings.\n"
   "@param p The new ( width, height ) extents of the control.",
   "GuiControl", // The class to place the method in; use NULL for functions.
   "void setExtent( Point2I p );" ); // The definition string.

DefineConsoleMethod( GuiControl, setExtent, void, ( const char* extOrX, const char* y ), (""),
   "( Point2I p | int x, int y ) Set the width and height of the control.\n\n"
   "@hide" )
{
   Point2I extent;
   if(!String::isEmpty(extOrX) && String::isEmpty(y))
      dSscanf(extOrX, "%d %d", &extent.x, &extent.y);
   else if(!String::isEmpty(extOrX) && !String::isEmpty(y))
   {
      extent.x = dAtoi(extOrX);
      extent.y = dAtoi(y);
   }
   object->setExtent( extent );
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, getMinExtent, Point2I, (),,
   "Get the minimum allowed size of the control.\n"
   "@return The minimum size to which the control can be shrunk.\n"
   "@see minExtent" )
{
   return object->getMinExtent();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiControl, getAspect, F32, (),,
   "Get the aspect ratio of the control's extents.\n"
   "@return The width of the control divided by its height.\n"
   "@see getExtent" )
{
   const Point2I &ext = object->getExtent();
   return (F32)ext.x / (F32)ext.y;
}
