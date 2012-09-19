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

#ifndef _GUICONTAINER_H_
#define _GUICONTAINER_H_

#ifndef _GUICONTROL_H_
   #include "gui/core/guiControl.h"
#endif


/// Base class for controls that act as containers to other controls.
///
/// @addtogroup gui_container_group Containers
///
/// @ingroup gui_group Gui System
/// @{
class  GuiContainer : public GuiControl
{
   public:
   
      typedef GuiControl Parent;
   
      enum 
      {
         updateSelf = BIT(1),
         updateParent = BIT(2),
         updateNone = 0
      };

   protected:

      S32 mUpdateLayout; ///< Layout Update Mask
      ControlSizing mSizingOptions; ///< Control Sizing Options
      S32 mValidDockingMask;
      
   public:
   
      DECLARE_CONOBJECT(GuiContainer);
      DECLARE_CATEGORY( "Gui Containers" );

      GuiContainer();
      virtual ~GuiContainer();

      static void initPersistFields();

      /// @name Container Sizing
      /// @{

      /// Returns the Mask of valid docking modes supported by this container
      inline S32 getValidDockingMask() { return mValidDockingMask; };

      /// Docking Accessors
      inline S32 getDocking() { return mSizingOptions.mDocking; };
      virtual void setDocking( S32 docking );

      /// Docking Protected Field Setter
      static bool setDockingField( void *object, const char *index, const char *data )
      {
         GuiContainer *pContainer = static_cast<GuiContainer*>(object);
         pContainer->setUpdateLayout( updateParent );
         return true;
      }

      inline bool getAnchorTop() const { return mSizingOptions.mAnchorTop; }
      inline bool getAnchorBottom() const { return mSizingOptions.mAnchorBottom; }
      inline bool getAnchorLeft() const { return mSizingOptions.mAnchorLeft; }
      inline bool getAnchorRight() const { return mSizingOptions.mAnchorRight; }
      inline void setAnchorTop(bool val) { mSizingOptions.mAnchorTop = val; }
      inline void setAnchorBottom(bool val) { mSizingOptions.mAnchorBottom = val; }
      inline void setAnchorLeft(bool val) { mSizingOptions.mAnchorLeft = val; }
      inline void setAnchorRight(bool val) { mSizingOptions.mAnchorRight = val; }

      ControlSizing getSizingOptions() const { return mSizingOptions; }
      void setSizingOptions(ControlSizing val) { mSizingOptions = val; }

      /// @}   

      /// @name Sizing Constraints
      /// @{
      virtual const RectI getClientRect();
      /// @}

      /// @name Control Layout Methods
      /// @{

      /// Called when the Layout for a Container needs to be updated because of a resize call or a call to setUpdateLayout
      /// @param   clientRect The Client Rectangle that is available for this Container to layout it's children in
      virtual bool layoutControls( RectI &clientRect );

      /// Set the layout flag to Dirty on a Container, triggering an update to it's layout on the next onPreRender call.
      ///  @attention This can be called without regard to whether the flag is already set, as setting it
      ///    does not actually cause an update, but rather tells the container it should update the next
      ///    chance it gets
      /// @param   updateType   A Mask that indicates how the layout should be updated.
      inline void setUpdateLayout( S32 updateType = updateSelf ) { mUpdateLayout |= updateType; };

      /// @}

      /// @name Container Sizing Methods
      /// @{

      /// Dock a Control with the given docking mode inside the given client rect.
      /// @attention The clientRect passed in will be modified by the docking of 
      ///    the control.  It will return the rect that remains after the docking operation.
      virtual bool dockControl( GuiContainer *control, S32 dockingMode, RectI &clientRect );
      
      /// Update a Controls Anchor based on a delta sizing of it's parents extent
      /// This function should return true if the control was changed in size or position at all
      virtual bool anchorControl( GuiControl *control, const Point2I &deltaParentExtent );
      
      /// @}

      /// @name GuiControl Inherited
      /// @{
      
      virtual void onChildAdded(GuiControl* control);
      virtual void onChildRemoved(GuiControl* control);
      virtual bool resize( const Point2I &newPosition, const Point2I &newExtent );
      virtual void childResized(GuiControl *child);
      virtual void addObject(SimObject *obj);
      virtual void removeObject(SimObject *obj);
      virtual bool reOrder(SimObject* obj, SimObject* target);
      virtual void onPreRender();
      
      /// GuiContainer deals with parentResized calls differently than GuiControl.  It will
      /// update the layout for all of it's non-docked child controls.  parentResized calls
      /// on the child controls will be handled by their default functions, but for our
      /// purposes we want at least our immediate children to use the anchors that they have
      /// set on themselves. - JDD [9/20/2006]
      virtual void parentResized(const RectI &oldParentRect, const RectI &newParentRect);
      
      /// @}
};
/// @}
#endif
