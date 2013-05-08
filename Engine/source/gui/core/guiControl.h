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

#ifndef _GUICONTROL_H_
#define _GUICONTROL_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _MRECT_H_
#include "math/mRect.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _GUITYPES_H_ 
#include "gui/core/guiTypes.h"
#endif
#ifndef _UTIL_DELEGATE_H_
#include "core/util/delegate.h"
#endif
#ifndef _LANG_H_
#include "i18n/lang.h"
#endif

class GuiCanvas;
class GuiEditCtrl;
class GuiWindowCtrl;


DECLARE_SCOPE( GuiAPI );


/// A delegate used in tool tip rendering.
///
/// @param   hoverPos    position to display the tip near
/// @param   cursorPos   the actual position of the cursor when the delegate is called
/// @param   tipText     optional alternate tip to be rendered
/// @return  Returns true if the tooltip was rendered.
///
/// @see GuiControl::mRenderTooltipDelegate
typedef Delegate<bool( const Point2I &hoverPos, const Point2I &cursorPos, const char *tipText )> RenderTooltipDelegate; 

/// @defgroup gui_group Gui System
/// The GUI system in Torque provides a powerful way of creating
/// WYSIWYG User Interfaces for your Game or Application written
/// in Torque.  
///
/// The GUI Provides a range of different controls that you may use
/// to arrange and layout your GUI's, including Buttons, Lists, Bitmaps
/// Windows, Containers, and HUD elements.
///
/// The Base Control Class GuiControl provides a basis upon which to 
/// write GuiControl's that may be specific to your particular type
/// of game.  


/// @addtogroup gui_core_group Core
/// @section GuiControl_Intro Introduction
///
/// GuiControl is the base class for GUI controls in Torque. It provides these
/// basic areas of functionality:
///      - Inherits from SimGroup, so that controls can have children.
///      - Interfacing with a GuiControlProfile.
///      - An abstraction from the details of handling user input
///        and so forth, providing friendly hooks like onMouseEnter(), onMouseMove(),
///        and onMouseLeave(), onKeyDown(), and so forth.
///      - An abstraction from the details of rendering and resizing.
///      - Helper functions to manipulate the mouse (mouseLock and
///        mouseUnlock), and convert coordinates (localToGlobalCoord() and
///        globalToLocalCoord()).
///
/// @ref GUI has an overview of the GUI system.
///
///
/// @ingroup gui_group Gui System
/// @{
class GuiControl : public SimGroup
{
   public:
   
      typedef SimGroup Parent;
      
      friend class GuiWindowCtrl; // mCollapseGroupVec
      friend class GuiCanvas;
      friend class GuiEditCtrl;
      friend class GuiDragAndDropControl; // drag callbacks
      
      /// Additional write flags for GuiControls.
      enum
      {
         NoCheckParentCanSave = BIT( 31 ),   ///< Don't inherit mCanSave=false from parents.
      };
      
      enum horizSizingOptions
      {
         horizResizeRight = 0,   ///< fixed on the left and width
         horizResizeWidth,       ///< fixed on the left and right
         horizResizeLeft,        ///< fixed on the right and width
         horizResizeCenter,
         horizResizeRelative,     ///< resize relative
         horizResizeWindowRelative ///< resize window relative
      };
      enum vertSizingOptions
      {
         vertResizeBottom = 0,   ///< fixed on the top and in height
         vertResizeHeight,       ///< fixed on the top and bottom
         vertResizeTop,          ///< fixed in height and on the bottom
         vertResizeCenter,
         vertResizeRelative,      ///< resize relative
         vertResizeWindowRelative ///< resize window relative
      };
      
   private:
   
      SimGroup               *mAddGroup;   ///< The internal name of a SimGroup child of the global GuiGroup in which to organize this gui on creation
      RectI                   mBounds;     ///< The internal bounds of this control
      
   protected:
   
      GuiControlProfile* mProfile;         ///< The profile for this gui (data settings that are likely to be shared by multiple guis)
      GuiControlProfile* mTooltipProfile;  ///< The profile for any tooltips
      
      /// @name Control State
      /// @{   
      
      static bool setProfileProt( void *object, const char *index, const char *data );
      static bool setTooltipProfileProt( void *object, const char *index, const char *data );
      
      S32      mTipHoverTime;
      
      /// Delegate called to render a tooltip for this control.
      /// By default this will be set to defaultTooltipRender.
      RenderTooltipDelegate mRenderTooltipDelegate;
      
      /// The default tooltip rendering function.
      /// @see RenderTooltipDelegate
      bool defaultTooltipRender( const Point2I &hoverPos, const Point2I &cursorPos, const char* tipText = NULL );
      
      bool    mVisible;
      bool    mActive;
      bool    mAwake;
      bool    mSetFirstResponder;
      bool    mIsContainer; ///< if true, then the GuiEditor can drag other controls into this one.
      bool    mCanResize;
      bool    mCanHit;
      
      S32     mLayer;
      Point2I mMinExtent;
      StringTableEntry mLangTableName;
      LangTable *mLangTable;
      
      bool mNotifyChildrenResized;
      
      // Contains array of windows located inside GuiControl
      typedef Vector< Vector< GuiWindowCtrl *> > CollapseGroupVec;
      CollapseGroupVec mCollapseGroupVec;
      
      static bool smDesignTime; ///< static GuiControl boolean that specifies if the GUI Editor is active
      /// @}
      
      /// @name Design Time Editor Access
      /// @{
      static GuiEditCtrl *smEditorHandle; ///< static GuiEditCtrl pointer that gives controls access to editor-NULL if editor is closed
      /// @}
      
      /// @name Keyboard Input
      /// @{
      GuiControl *mFirstResponder;
      static GuiControl *smPrevResponder;
      static GuiControl *smCurResponder;
      /// @}
      
      /// @name Control State
      /// @{
      
      S32 mHorizSizing;      ///< Set from horizSizingOptions.
      S32 mVertSizing;       ///< Set from vertSizingOptions.
      
      StringTableEntry mAcceleratorKey;
      StringTableEntry mConsoleVariable;
      
      String mConsoleCommand;
      String mAltConsoleCommand;
      
      String mTooltip;
      
      /// @}
      
      /// @name Console
      /// The console variable collection of functions allows a console variable to be bound to the GUI control.
      ///
      /// This allows, say, an edit field to be bound to '$foo'. The value of the console
      /// variable '$foo' would then be equal to the text inside the text field. Changing
      /// either changes the other.
      /// @{
      
      /// $ThisControl variable for callback execution.
      static GuiControl* smThisControl;
      
      /// Set $ThisControl and evaluate the given script code.
      const char* evaluate( const char* str );
      
      /// Sets the value of the console variable bound to this control
      /// @param   value   String value to assign to control's console variable
      void setVariable(const char *value);
      
      /// Sets the value of the console variable bound to this control
      /// @param   value   Integer value to assign to control's console variable
      void setIntVariable(S32 value);
      
      /// Sets the value of the console variable bound to this control
      /// @param   value   Float value to assign to control's console variable
      void setFloatVariable(F32 value);
      
      const char* getVariable(); ///< Returns value of control's bound variable as a string
      S32 getIntVariable();      ///< Returns value of control's bound variable as a integer
      F32 getFloatVariable();    ///< Returns value of control's bound variable as a float
      
      GFXStateBlockRef mDefaultGuiSB;
      
      /// @name Callbacks
      /// @{
      
      DECLARE_CALLBACK( void, onAdd, () );
      DECLARE_CALLBACK( void, onRemove, () );
      
      DECLARE_CALLBACK( void, onWake, () );
      DECLARE_CALLBACK( void, onSleep, () );
      
      DECLARE_CALLBACK( void, onLoseFirstResponder, () );
      DECLARE_CALLBACK( void, onGainFirstResponder, () );
      
      DECLARE_CALLBACK( void, onAction, () );
      DECLARE_CALLBACK( void, onVisible, ( bool state ) );
      DECLARE_CALLBACK( void, onActive, ( bool state ) );
      
      DECLARE_CALLBACK( void, onDialogPush, () );
      DECLARE_CALLBACK( void, onDialogPop, () );
      
      DECLARE_CALLBACK( void, onControlDragEnter, ( GuiControl* control, const Point2I& dropPoint ) );
      DECLARE_CALLBACK( void, onControlDragExit, ( GuiControl* control, const Point2I& dropPoint ) );
      DECLARE_CALLBACK( void, onControlDragged, ( GuiControl* control, const Point2I& dropPoint ) );
      DECLARE_CALLBACK( void, onControlDropped, ( GuiControl* control, const Point2I& dropPoint ) );
            
      /// @}
      
   public:
   
      /// Set the name of the console variable which this GuiObject is bound to
      /// @param   variable   Variable name
      void setConsoleVariable(const char *variable);
      
      /// Set the name of the console function bound to, such as a script function
      /// a button calls when clicked.
      /// @param   newCmd   Console function to attach to this GuiControl
      void setConsoleCommand( const String& newCmd );
      const char * getConsoleCommand(); ///< Returns the name of the function bound to this GuiControl
      LangTable *getGUILangTable(void);
      const UTF8 *getGUIString(S32 id);
      
      /// @}
      
      /// @name Callbacks
      /// @{
      /// Executes a console command, and returns the result.
      ///
      /// The global console variable $ThisControl is set to the id of the calling
      /// control. WARNING: because multiple controls may set $ThisControl, at any time,
      /// the value of $ThisControl should be stored in a local variable by the
      /// callback code. The use of the $ThisControl variable is not thread safe.
      
      /// Executes mConsoleCommand, and returns the result.
      const char* execConsoleCallback();
      /// Executes mAltConsoleCommand, and returns the result.
      const char* execAltConsoleCallback();
      /// @}
      
      static bool _setVisible( void *object, const char *index, const char *data ) { static_cast<GuiControl*>(object)->setVisible( dAtob( data ) ); return false; };
      static bool _setActive( void *object, const char *index, const char *data ) { static_cast<GuiControl*>(object)->setActive( dAtob( data ) ); return false; };
            
      /// @name Editor
      /// These functions are used by the GUI Editor
      /// @{
      
      /// Sets the size of the GuiControl
      /// @param   horz   Width of the control
      /// @param   vert   Height of the control
      void setSizing(S32 horz, S32 vert);
      
      ///   Overrides Parent Serialization to allow specific controls to not be saved (Dynamic Controls, etc)
      void write(Stream &stream, U32 tabStop, U32 flags);
      
      /// Returns boolean as to whether any parent of this control has the 'no serialization' flag set.
      bool getCanSaveParent();
            
      /// @}
      
      /// @name Initialization
      /// @{
      
      DECLARE_CONOBJECT(GuiControl);
      DECLARE_CATEGORY( "Gui Core" );
      DECLARE_DESCRIPTION( "Base class for GUI controls. Can also be used as a generic container." );
      
      GuiControl();
      virtual ~GuiControl();
      virtual bool processArguments(S32 argc, const char **argv);
      
      static void initPersistFields();
      static void consoleInit();
      
      /// @}
      
      /// @name Accessors
      /// @{
      
      inline const Point2I&   getPosition() const { return mBounds.point; } ///< Returns position of the control
      inline const Point2I&   getExtent() const { return mBounds.extent; } ///< Returns extents of the control
      inline const RectI     getBounds()const { return mBounds; }           ///< Returns the bounds of the control
      inline const RectI     getGlobalBounds() ///< Returns the bounds of this object, in global coordinates 
      {
         RectI retRect = getBounds();
         retRect.point = localToGlobalCoord( Point2I(0,0) );
         
         return retRect;
      };
      virtual Point2I   getMinExtent() const { return mMinExtent; } ///< Returns minimum size the control can be
      virtual void      setMinExtent( const Point2I &newMinExtent ) { mMinExtent = newMinExtent; };
      inline const S32        getLeft() const { return mBounds.point.x; } ///< Returns the X position of the control
      inline const S32        getTop() const { return mBounds.point.y; } ///< Returns the Y position of the control
      inline const S32        getWidth() const { return mBounds.extent.x; } ///< Returns the width of the control
      inline const S32        getHeight() const { return mBounds.extent.y; } ///< Returns the height of the control
      
      inline const S32        getHorizSizing() const { return mHorizSizing; }
      inline const S32        getVertSizing() const { return mVertSizing; }
      
      /// @}
      
      /// @name Flags
      /// @{
      
      /// Sets the visibility of the control
      /// @param   value   True if object should be visible
      virtual void setVisible(bool value);
      inline bool isVisible() const { return mVisible; } ///< Returns true if the object is visible
      virtual bool isHidden() const { return !isVisible(); }
      virtual void setHidden( bool state ) { setVisible( !state ); }
      
      void setCanHit( bool value ) { mCanHit = value; }
      
      /// Sets the status of this control as active and responding or inactive
      /// @param   value   True if this is active
      virtual void setActive(bool value);
      bool isActive() { return mActive; } ///< Returns true if this control is active
      
      bool isAwake() { return mAwake; } ///< Returns true if this control is awake
      
      /// @}
      
      /// Get information about the size of a scroll line.
      ///
      /// @param   rowHeight   The height, in pixels, of a row
      /// @param   columnWidth The width, in pixels, of a column
      virtual void getScrollLineSizes(U32 *rowHeight, U32 *columnWidth);
      
      /// Get information about the cursor.
      /// @param   cursor   Cursor information will be stored here
      /// @param   showCursor Will be set to true if the cursor is visible
      /// @param   lastGuiEvent GuiEvent containing cursor position and modifier keys (ie ctrl, shift, alt etc)
      virtual void getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent);
      
      /// @name Children
      /// @{
      
      /// Adds an object as a child of this object.
      /// @param   obj   New child object of this control
      void addObject(SimObject *obj);
      
      /// Removes a child object from this control.
      /// @param   obj Object to remove from this control
      void removeObject(SimObject *obj);
      
      GuiControl *getParent();  ///< Returns the control which owns this one.
      GuiCanvas *getRoot();     ///< Returns the root canvas of this control.
      
      virtual bool acceptsAsChild( SimObject* object ) const;
      
      virtual void onGroupRemove();
      
      /// @}
      
      /// @name Coordinates
      /// @{
      
      /// Translates local coordinates (wrt this object) into global coordinates
      ///
      /// @param   src   Local coordinates to translate
      Point2I localToGlobalCoord(const Point2I &src);
      
      /// Returns global coordinates translated into local space
      ///
      /// @param   src   Global coordinates to translate
      Point2I globalToLocalCoord(const Point2I &src);
      /// @}
      
      /// @name Resizing
      /// @{
      
      /// Changes the size and/or position of this control
      /// @param   newPosition   New position of this control
      /// @param   newExtent   New size of this control
      virtual bool resize(const Point2I &newPosition, const Point2I &newExtent);
      
      /// Changes the position of this control
      /// @param   newPosition   New position of this control
      virtual bool setPosition( const Point2I &newPosition );
      inline  void setPosition( const S32 x, const S32 y ) { setPosition(Point2I(x,y)); }
      
      /// Changes the size of this control
      /// @param   newExtent   New size of this control
      virtual bool setExtent( const Point2I &newExtent );
      inline  void setExtent( const S32 width, const S32 height) { setExtent(Point2I(width, height)); }
      
      /// Changes the bounds of this control
      /// @param   newBounds   New bounds of this control
      virtual bool setBounds( const RectI &newBounds );
      inline  void setBounds( const S32 left,  const S32 top,
                             const S32 width, const S32 height) { setBounds(RectI(left, top, width, height)); }
      
      /// Changes the X position of this control
      /// @param   newXPosition   New X Position of this control
      virtual void setLeft( S32 newLeft );
      
      /// Changes the Y position of this control
      /// @param   newYPosition   New Y Position of this control
      virtual void setTop( S32 newTop );
      
      /// Changes the width of this control
      /// @param   newWidth   New width of this control
      virtual void setWidth( S32 newWidth );
      
      /// Changes the height of this control
      /// @param   newHeight   New Height of this control
      virtual void setHeight( S32 newHeight );
      
      /// Called when a child control of the object is resized
      /// @param   child   Child object
      virtual void childResized(GuiControl *child);
      
      /// Called when this objects parent is resized
      /// @param   oldParentRect   The old rectangle of the parent object
      /// @param   newParentRect   The new rectangle of the parent object
      virtual void parentResized(const RectI &oldParentRect, const RectI &newParentRect);
      /// @}
      
      /// @name Rendering
      /// @{
      
      /// Called when this control is to render itself
      /// @param   offset   The location this control is to begin rendering
      /// @param   updateRect   The screen area this control has drawing access to
      virtual void onRender(Point2I offset, const RectI &updateRect);
      
      /// Called when this control should render its children
      /// @param   offset   The location this control is to begin rendering
      /// @param   updateRect   The screen area this control has drawing access to
      void renderChildControls(Point2I offset, const RectI &updateRect);
      
      /// Sets the area (local coordinates) this control wants refreshed each frame
      /// @param   pos   UpperLeft point on rectangle of refresh area
      /// @param   ext   Extent of update rect
      void setUpdateRegion(Point2I pos, Point2I ext);
      
      /// Sets the update area of the control to encompass the whole control
      virtual void setUpdate();
      /// @}
      
      //child hierarchy calls
      void awaken();          ///< Called when this control and its children have been wired up.
      void sleep();           ///< Called when this control is no more.
      void preRender();       ///< Pre-render this control and all its children.
      
      /// @name Events
      ///
      /// If you subclass these, make sure to call the Parent::'s versions.
      ///
      /// @{
      
      /// Called when this object is asked to wake up returns true if it's actually awake at the end
      virtual bool onWake();
      
      /// Called when this object is asked to sleep
      virtual void onSleep();
      
      /// Do special pre-render processing
      virtual void onPreRender();
      
      /// Called when this object is removed
      virtual void onRemove();
      
      /// Called when one of this objects children is removed
      virtual void onChildRemoved( GuiControl *child );
      
      /// Called when this object is added to the scene
      virtual bool onAdd();
      
      /// Called when the mProfile or mToolTipProfile is deleted
      virtual void onDeleteNotify(SimObject *object);
      
      /// Called when this object has a new child
      virtual void onChildAdded( GuiControl *child );
      
      /// @}
      
      /// @name Console
      /// @{
      
      /// Returns the value of the variable bound to this object
      virtual const char *getScriptValue();
      
      /// Sets the value of the variable bound to this object
      virtual void setScriptValue(const char *value);
      /// @}
      
      /// @name Input (Keyboard/Mouse)
      /// @{
      
      /// This function will return true if the provided coordinates (wrt parent object) are
      /// within the bounds of this control
      /// @param   parentCoordPoint   Coordinates to test
      virtual bool pointInControl(const Point2I& parentCoordPoint);
      
      /// Returns true if the global cursor is inside this control
      bool cursorInControl();
      
      /// Returns the control which the provided point is under, with layering
      /// @param   pt   Point to test
      /// @param   initialLayer  Layer of gui objects to begin the search
      virtual GuiControl* findHitControl(const Point2I &pt, S32 initialLayer = -1 );
      
      enum EHitTestFlags
      {
         HIT_FullBoxOnly            = BIT( 0 ),    ///< Hit only counts if all of a control's bounds are within the hit rectangle.
         HIT_ParentPreventsChildHit = BIT( 1 ),    ///< A positive hit test on a parent control will prevent hit tests on children.
         HIT_AddParentHits          = BIT( 2 ),    ///< Parent's that get hit should be added regardless of whether any of their children get hit, too.
         HIT_NoCanHitNoRecurse      = BIT( 3 ),    ///< A hit-disabled control will not recurse into children.
      };
      
      ///
      virtual bool findHitControls( const RectI& rect, Vector< GuiControl* >& outResult, U32 flags = 0, S32 initialLayer = -1, U32 depth = 0 );
      
      /// Lock the mouse within the provided control
      /// @param   lockingControl   Control to lock the mouse within
      void mouseLock(GuiControl *lockingControl);
      
      /// Turn on mouse locking with last used lock control
      void mouseLock();
      
      /// Unlock the mouse
      void mouseUnlock();
      
      /// Returns true if the mouse is locked
      bool isMouseLocked();
      /// @}
      
      
      /// General input handler.
      virtual bool onInputEvent(const InputEventInfo &event);
      
      /// @name Mouse Events
      /// These functions are called when the input event which is
      /// in the name of the function occurs.
      /// @{
      virtual void onMouseUp(const GuiEvent &event);
      virtual void onMouseDown(const GuiEvent &event);
      virtual void onMouseMove(const GuiEvent &event);
      virtual void onMouseDragged(const GuiEvent &event);
      virtual void onMouseEnter(const GuiEvent &event);
      virtual void onMouseLeave(const GuiEvent &event);
      
      virtual bool onMouseWheelUp(const GuiEvent &event);
      virtual bool onMouseWheelDown(const GuiEvent &event);
      
      virtual void onRightMouseDown(const GuiEvent &event);
      virtual void onRightMouseUp(const GuiEvent &event);
      virtual void onRightMouseDragged(const GuiEvent &event);
      
      virtual void onMiddleMouseDown(const GuiEvent &event);
      virtual void onMiddleMouseUp(const GuiEvent &event);
      virtual void onMiddleMouseDragged(const GuiEvent &event);
      /// @}
      
      /// @name Gamepad Events
      /// These functions are called when the input event which is in the name of
      /// the function occurs.
      /// @{
      virtual bool onGamepadButtonDown(const GuiEvent &event);  ///< Default behavior is call-through to onKeyDown
      virtual bool onGamepadButtonUp(const GuiEvent &event);    ///< Default behavior is call-through to onKeyUp
      virtual bool onGamepadAxisUp(const GuiEvent &event);
      virtual bool onGamepadAxisDown(const GuiEvent &event);
      virtual bool onGamepadAxisLeft(const GuiEvent &event);
      virtual bool onGamepadAxisRight(const GuiEvent &event);
      virtual bool onGamepadTrigger(const GuiEvent &event);
      /// @}
      
      /// @name Editor Mouse Events
      ///
      /// These functions are called when the input event which is
      /// in the name of the function occurs.  Conversely from normal
      /// mouse events, these have a boolean return value that, if 
      /// they return true, the editor will NOT act on them or be able
      /// to respond to this particular event.
      ///
      /// This is particularly useful for when writing controls so that
      /// they may become aware of the editor and allow customization
      /// of their data or appearance as if they were actually in use.
      /// For example, the GuiTabBookCtrl catches on mouse down to select
      /// a tab and NOT let the editor do any instant group manipulation.
      /// 
      /// @{
      
      /// Called when a mouseDown event occurs on a control and the GUI editor is active
      /// @param   event   the GuiEvent which caused the call to this function
      /// @param   offset   the offset which is representative of the units x and y that the editor takes up on screen
      virtual bool onMouseDownEditor(const GuiEvent &event, Point2I offset) { return false; };
      
      /// Called when a mouseUp event occurs on a control and the GUI editor is active
      /// @param   event   the GuiEvent which caused the call to this function
      /// @param   offset   the offset which is representative of the units x and y that the editor takes up on screen
      virtual bool onMouseUpEditor(const GuiEvent &event, Point2I offset) { return false; };
      
      /// Called when a rightMouseDown event occurs on a control and the GUI editor is active
      /// @param   event   the GuiEvent which caused the call to this function
      /// @param   offset   the offset which is representative of the units x and y that the editor takes up on screen
      virtual bool onRightMouseDownEditor(const GuiEvent &event, Point2I offset) { return false; };
      
      /// Called when a mouseDragged event occurs on a control and the GUI editor is active
      /// @param   event   the GuiEvent which caused the call to this function
      /// @param   offset   the offset which is representative of the units x and y that the editor takes up on screen
      virtual bool onMouseDraggedEditor(const GuiEvent &event, Point2I offset) { return false; };
      
      /// @}
      
      /// @name Tabs
      /// @{
      
      /// Find the first tab-accessible child of this control
      virtual GuiControl* findFirstTabable();
      
      /// Find the last tab-accessible child of this control
      /// @param   firstCall   Set to true to clear the global previous responder
      virtual GuiControl* findLastTabable(bool firstCall = true);
      
      /// Find previous tab-accessible control with respect to the provided one
      /// @param   curResponder   Current control
      /// @param   firstCall   Set to true to clear the global previous responder
      virtual GuiControl* findPrevTabable(GuiControl *curResponder, bool firstCall = true);
      
      /// Find next tab-accessible control with regards to the provided control.
      ///
      /// @param   curResponder   Current control
      /// @param   firstCall   Set to true to clear the global current responder
      virtual GuiControl* findNextTabable(GuiControl *curResponder, bool firstCall = true);
      /// @}
      
      /// Returns true if the provided control is a child (grandchild, or great-grandchild) of this one.
      ///
      /// @param   child   Control to test
      virtual bool controlIsChild(GuiControl *child);
      
      /// @name First Responder
      /// A first responder is the control which reacts first, in it's responder chain, to keyboard events
      /// The responder chain is set for each parent and so there is only one first responder amongst it's
      /// children.
      /// @{
      
      /// Sets the first responder for child controls
      /// @param   firstResponder   First responder for this chain
      virtual void setFirstResponder(GuiControl *firstResponder);
      
      /// Sets up this control to be the first in it's group to respond to an input event
      /// @param   value   True if this should be a first responder
      virtual void makeFirstResponder(bool value);
      
      /// Returns true if this control is a first responder
      bool isFirstResponder();
      
      /// Sets this object to be a first responder
      virtual void setFirstResponder();
      
      /// Clears the first responder for this chain
      void clearFirstResponder();
      
      /// Returns the first responder for this chain
      GuiControl *getFirstResponder() { return mFirstResponder; }
      
      /// Occurs when the control gains first-responder status.
      virtual void onGainFirstResponder();
      
      /// Occurs when the control loses first-responder status.
      virtual void onLoseFirstResponder();
      /// @}
      
      /// @name Keyboard Events
      /// @{
      
      /// Adds the accelerator key for this object to the canvas
      void addAcceleratorKey();
      
      /// Adds this control's accelerator key to the accelerator map, and
      /// recursively tells all children to do the same.
      virtual void buildAcceleratorMap();
      
      /// Occurs when the accelerator key for this control is pressed
      ///
      /// @param   index   Index in the accelerator map of the key
      virtual void acceleratorKeyPress(U32 index);
      
      /// Occurs when the accelerator key for this control is released
      ///
      /// @param   index   Index in the accelerator map of the key
      virtual void acceleratorKeyRelease(U32 index);
      
      /// Happens when a key is depressed
      /// @param   event   Event descriptor (which contains the key)
      virtual bool onKeyDown(const GuiEvent &event);
      
      /// Happens when a key is released
      /// @param   event   Event descriptor (which contains the key)
      virtual bool onKeyUp(const GuiEvent &event);
      
      /// Happens when a key is held down, resulting in repeated keystrokes.
      /// @param   event   Event descriptor (which contains the key)
      virtual bool onKeyRepeat(const GuiEvent &event);
      /// @}
      
      /// Return the delegate used to render tooltips on this control.
      RenderTooltipDelegate& getRenderTooltipDelegate() { return mRenderTooltipDelegate; }
      const RenderTooltipDelegate& getRenderTooltipDelegate() const { return mRenderTooltipDelegate; }
      
      /// Returns our tooltip profile (and finds the profile if it hasn't been set yet)
      GuiControlProfile* getTooltipProfile() { return mTooltipProfile; }
      
      /// Sets the tooltip profile for this control.
      ///
      /// @see GuiControlProfile
      /// @param   prof   Tooltip profile to apply
      void setTooltipProfile(GuiControlProfile *prof);
      
      /// Returns our profile (and finds the profile if it hasn't been set yet)
      GuiControlProfile* getControlProfile() { return mProfile; }
      
      /// Sets the control profile for this control.
      ///
      /// @see GuiControlProfile
      /// @param   prof   Control profile to apply
      void setControlProfile(GuiControlProfile *prof);
      
      /// Occurs when this control performs its "action"
      virtual void onAction();
      
      /// @name Peer Messaging
      /// Used to send a message to other GUIControls which are children of the same parent.
      ///
      /// This is mostly used by radio controls.
      /// @{
      void messageSiblings(S32 message);                      ///< Send a message to all siblings
      virtual void onMessage(GuiControl *sender, S32 msg);    ///< Receive a message from another control
      /// @}
      
      /// @name Canvas Events
      /// Functions called by the canvas
      /// @{
      
      /// Called if this object is a dialog, when it is added to the visible layers
      virtual void onDialogPush();
      
      /// Called if this object is a dialog, when it is removed from the visible layers
      virtual void onDialogPop();
      /// @}
      
      /// Renders justified text using the profile.
      ///
      /// @note This should move into the graphics library at some point
      void renderJustifiedText(Point2I offset, Point2I extent, const char *text);
      
      /// Returns text clipped to fit within a pixel width. The clipping 
      /// occurs on the right side and "..." is appended.  It returns width
      /// of the final clipped text in pixels.
      U32 clipText( String &inOutText, U32 width ) const;
      
      void inspectPostApply();
      void inspectPreApply();
};

typedef GuiControl::horizSizingOptions GuiHorizontalSizing;
typedef GuiControl::vertSizingOptions GuiVerticalSizing;

DefineEnumType( GuiHorizontalSizing );
DefineEnumType( GuiVerticalSizing );

/// @}

#endif
