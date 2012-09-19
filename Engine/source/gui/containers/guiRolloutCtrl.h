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
#ifndef _GUI_ROLLOUTCTRL_H_
#define _GUI_ROLLOUTCTRL_H_

#ifndef _GUICONTROL_H_
   #include "gui/core/guiControl.h"
#endif
#ifndef _GUISTACKCTRL_H_
   #include "gui/containers/guiStackCtrl.h"
#endif
#ifndef _H_GUIDEFAULTCONTROLRENDER_
   #include "gui/core/guiDefaultControlRender.h"
#endif
#ifndef _GUITICKCTRL_H_
   #include "gui/shiny/guiTickCtrl.h"
#endif


/// A container with an optional header that allows its child control to
/// be collapsed using an animated effet.
class GuiRolloutCtrl : public GuiTickCtrl
{
   public:
   
      typedef GuiControl Parent;

      // Theme Support
      enum
      {
         CollapsedLeft = 0,
         CollapsedCenter,
         CollapsedRight,
         TopLeftHeader,
         TopMidHeader,      
         TopRightHeader,  
         MidPageLeft,
         MidPageCenter,
         MidPageRight,
         BottomLeftHeader, 
         BottomMidHeader,   
         BottomRightHeader,   
         NumBitmaps           ///< Number of bitmaps in this array
      };

   protected:

      /// Label to display on rollout header.
      String mCaption;
      
      RectI mHeader;
      RectI mExpanded;
      RectI mChildRect;
      RectI mMargin;
      bool mIsExpanded;
      bool mIsAnimating;
      bool mCollapsing;
      S32 mAnimateDestHeight;
      S32 mAnimateStep;
      S32 mDefaultHeight;
      
      /// Whether the rollout can be collapsed.
      bool mCanCollapse;
      
      /// Whether to hide the rollout header.
      bool mHideHeader;
      
      /// Whether to automatically collapse sibling rollouts when this one
      /// is expanded.
      bool mAutoCollapseSiblings;

      GuiCursor*  mDefaultCursor;
      GuiCursor*  mVertSizingCursor;

      /// Indicates whether we have a texture to render the tabs with.
      bool mHasTexture;
      
      /// Array of rectangles identifying textures for rollout.
      RectI *mBitmapBounds;

      // Property - "Expanded"
      static bool setExpanded( void *object, const char *index, const char *data )  
      { 
         bool expand = dAtob( data );
         if( expand )
            static_cast<GuiRolloutCtrl*>(object)->instantExpand();         
         else
            static_cast<GuiRolloutCtrl*>(object)->instantCollapse();         
         return false; 
      };
      
      bool _onMouseUp( const GuiEvent& event, bool lockedMouse );
      
      /// @name Callbacks
      /// @{
      DECLARE_CALLBACK( void, onHeaderRightClick, () );

      DECLARE_CALLBACK( void, onExpanded, () );

      DECLARE_CALLBACK( void, onCollapsed, () );
      /// @}

   public:
   
      GuiRolloutCtrl();
      ~GuiRolloutCtrl();

      DECLARE_CONOBJECT(GuiRolloutCtrl);
      DECLARE_CATEGORY( "Gui Containers" );
      DECLARE_DESCRIPTION( "A container that displays a header with a caption on top of its child control\n"
         "that when clicked collapses/expands the control to/from just the header." );

      // Persistence
      static void initPersistFields();

      // Control Events
      bool onWake();
      void addObject(SimObject *obj);
      void removeObject(SimObject *obj);
      virtual void childResized(GuiControl *child);

      // Mouse Events
      virtual void onMouseDown( const GuiEvent& event );
      virtual void onMouseUp( const GuiEvent& event );
      virtual void onRightMouseUp( const GuiEvent& event );
      virtual bool onMouseUpEditor( const GuiEvent& event, Point2I offset );

      // Sizing Helpers
      virtual void calculateHeights();
      virtual bool resize( const Point2I &newPosition, const Point2I &newExtent );
      virtual void sizeToContents();
      inline bool isExpanded() const { return mIsExpanded; }

      // Sizing Animation Functions
      void animateTo( S32 height );
      virtual void processTick();

      void collapse() { animateTo( mHeader.extent.y ); }
      void expand() { animateTo( mExpanded.extent.y ); }
      void instantCollapse();
      void instantExpand();
      void toggleExpanded( bool instant );

      const String& getCaption() const { return mCaption; }
      bool isHeaderHidden() const { return mHideHeader; }
      bool canCollapse() const { return mCanCollapse; }

      void setCaption( const String& str ) { mCaption = str; }
      void setMargin( const RectI& rect ) { mMargin = rect; }
      void setMargin( S32 left, S32 top, S32 right, S32 bottom ) { setMargin( RectI( left, top, right, bottom ) ); }
      
      void setHeaderHidden( bool value ) { mHideHeader = value; }
      void setCanCollapse( bool value ) { mCanCollapse = value; }

      // Control Rendering
      virtual void onRender(Point2I offset, const RectI &updateRect);
      bool onAdd();
};

#endif // _GUI_ROLLOUTCTRL_H_