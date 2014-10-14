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

#ifndef _GUITYPES_H_
#define _GUITYPES_H_

#ifndef _GFONT_H_
#include "gfx/gFont.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _DYNAMIC_CONSOLETYPES_H_
#include "console/dynamicTypes.h"
#endif


#include "gfx/gfxDevice.h"
#include "platform/input/event.h"

class GBitmap;
class SFXTrack;

/// Represents a single GUI event.
///
/// This is passed around to all the relevant controls so they know what's going on.
struct GuiEvent
{
   U16                  ascii;            ///< ascii character code 'a', 'A', 'b', '*', etc (if device==keyboard) - possibly a uchar or something
   U8                   modifier;         ///< SI_LSHIFT, etc
   InputObjectInstances keyCode;          ///< for unprintables, 'tab', 'return', ...
   Point2I              mousePoint;       ///< for mouse events
   U8                   mouseClickCount;  ///< to determine double clicks, etc...
   U8                   mouseAxis;        ///< mousewheel axis (0 == X, 1 == Y)
   F32                  fval;             ///< used for mousewheel events
   
   GuiEvent()
      : ascii( 0 ),
        modifier( 0 ),
        keyCode( KEY_NULL ),
        mousePoint( 0, 0 ),
        mouseClickCount( 0 ),
        mouseAxis( 0 ),
        fval( 0.f ) {}
};

/// Represent a mouse event with a 3D position and vector.
///
/// This event used by the EditTSCtrl derived controls.
struct Gui3DMouseEvent : public GuiEvent
{
   Point3F     vec;
   Point3F     pos;
   
   Gui3DMouseEvent()
      : vec( 0.f, 0.f, 0.f ),
        pos( 0.f, 0.f, 0.f ) {}
};


/// @name Docking Flag
/// @{
/// @brief Docking Options available to all GuiControl subclasses.
namespace Docking
{
   enum DockingType
   {
      dockNone    = BIT(0), ///< Do not align this control to it's parent, let the control specify it's position/extent (default)
      dockClient  = BIT(1), ///< Align this control to the client area available in the parent
      dockTop     = BIT(2), ///< Align this control to the topmost border of it's parent (Width will be parent width)
      dockBottom  = BIT(3), ///< Align this control to the bottommost border of it's parent (Width will be parent width)
      dockLeft    = BIT(4), ///< Align this control to the leftmost border of it's parent (Height will be parent height)
      dockRight   = BIT(5), ///< Align this control to the rightmost border of it's parent (Height will be parent height)
      dockInvalid = BIT(6),     ///< Default NOT specified docking mode, this allows old sizing to takeover when needed by controls
      dockAny     = dockClient | dockTop | dockBottom | dockLeft | dockRight
   };
};

typedef Docking::DockingType GuiDockingType;
DefineEnumType( GuiDockingType );

/// @}


/// @name Margin Padding Structure
/// @{
struct RectSpacingI
{
   S32 left;
   S32 top;
   S32 bottom;
   S32 right;
   RectSpacingI() { left = right = top = bottom = 0; };
   RectSpacingI( S32 in_top, S32 in_bottom, S32 in_left, S32 in_right )
   {
      top = in_top;
      bottom = in_bottom;
      left = in_left;
      right = in_right;
   }
   void setAll( S32 value ) { left = right = top = bottom = value; };
   void set( S32 in_top, S32 in_bottom, S32 in_left, S32 in_right )
   {
      top = in_top;
      bottom = in_bottom;
      left = in_left;
      right = in_right;
   }
   void insetRect( RectI &rectRef )
   {
      // Inset by padding
      rectRef.point.x += left;
      rectRef.point.y += top;
      rectRef.extent.x -= (left + right );
      rectRef.extent.y -= (bottom + top );
   }
   void expandRect( RectI &rectRef )
   {
      // Inset by padding
      rectRef.point.x -= left;
      rectRef.point.y -= top;
      rectRef.extent.x += (left + right );
      rectRef.extent.y += (bottom + top );
   }


};

DECLARE_STRUCT( RectSpacingI );
DefineConsoleType( TypeRectSpacingI, RectSpacingI );
/// @}


/// @name Axis-Aligned Edge Structure
/// @{
///
struct Edge
{
   Point2F normal;  ///< The Normal of this edge
   Point2I position;///< The Position of the edge
   Point2I extent;  ///< The X/Y extents of the edge
   F32     margin;   ///< The Size of the edge

   Edge(): normal(0.f,0.f),
      position(0,0),
      extent(0,0),
      margin(1.f){};
   Edge( const Point2I &inPoint, const Point2F &inNormal )
   {
      normal = inNormal;
      margin = 2.f;

      if( normal.x == 1.f || normal.x == -1.f )  
      {
         // Vertical Edge
         position.x = inPoint.x;
         position.y = 0;

         extent.x = 1;
         extent.y = 1;
      }
      else if( normal.y == 1.f || normal.y == -1.f )
      {
         // Horizontal Edge
         position.y = inPoint.y;
         position.x = 0;

         extent.x = 1;
         extent.y = 1;
      }
      else
         AssertFatal(false,"Edge point constructor cannot construct an Edge without an axis-aligned normal.");
   }

   // Copy Constructor
   Edge( const Edge &inEdge )
   {
      normal   = inEdge.normal;
      position = inEdge.position;
      extent   = inEdge.extent;
      margin   = inEdge.margin;
   }     

   // RectI cast operator overload
   operator const RectI() const
   {
      if( normal.x == 1.f || normal.x == -1.f )  
      {
         // Vertical Edge
         RectI retRect = RectI( position.x, position.y, 1, position.y + extent.y );
         // Expand Rect by Margin along the X Axis
         retRect.inset(-margin,0);

         return retRect;
      }
      else if( normal.y == 1.f || normal.y == -1.f )
      {
         // Horizontal Edge
         RectI retRect =  RectI( position.x, position.y , position.x + extent.x,  1 );
         // Expand Rect by Margin along the Y Axis
         retRect.inset(0,-margin);
         return retRect;
      }

      // CodeReview this code only deals with axis-aligned edges [6/8/2007 justind]
      AssertFatal(false,"Edge cast operator cannot construct a Rect from an Edge that is not axis-aligned.");
      return RectI( 0,0,0,0 );
   }

   inline bool hit( const Edge &inEdge ) const
   {
      const RectI thisRect = *this;
      const RectI thatRect = inEdge;

      return thisRect.overlaps( thatRect );
   }
};
/// @}


struct EdgeRectI
{
   Edge left;
   Edge top;
   Edge bottom;
   Edge right;

   EdgeRectI(){ }

   EdgeRectI( const RectI &inRect, F32 inMargin )
   {
      // Left Edge 
      left.normal    = Point2F( -1.f, 0.f );
      left.position.x= inRect.point.x;
      left.position.y= 0;
      left.extent    = Point2I(inRect.point.y, inRect.point.y + inRect.extent.y);
      left.margin    = inMargin;

      // Right Edge
      right.normal     = Point2F( 1.f, 0.f );
      right.position.x = inRect.point.x + inRect.extent.x;
      right.position.y = 0;
      right.extent     = Point2I(inRect.point.y, inRect.point.y + inRect.extent.y);
      right.margin     = inMargin;

      // Top Edge
      top.normal   = Point2F( 0.f, 1.f );
      top.position.y = inRect.point.y;
      top.position.x = 0;
      top.extent   = Point2I(inRect.point.x + inRect.extent.x, inRect.point.x);
      top.margin   = inMargin;

      // Bottom Edge
      bottom.normal   = Point2F( 0.f, -1.f );
      bottom.position.y= inRect.point.y + inRect.extent.y;
      bottom.position.x=0;
      bottom.extent   = Point2I(inRect.point.x + inRect.extent.x, inRect.point.x);
      bottom.margin   = inMargin;
   }

   // Copy constructor
   EdgeRectI( const EdgeRectI &inEdgeRect )
   {
      left     = inEdgeRect.left;
      right    = inEdgeRect.right;
      top      = inEdgeRect.top;
      bottom   = inEdgeRect.bottom;
   }
};


/// Represents the Sizing Options for a GuiControl
struct ControlSizing
{
   ControlSizing()
   {
      mDocking = Docking::dockInvalid;
      mPadding.setAll( 0 );
      mInternalPadding.setAll( 0 );

      // Default anchors to full top/left
      mAnchorBottom  = false;
      mAnchorLeft    = true;
      mAnchorTop     = true;
      mAnchorRight   = false;
   };

   S32   mDocking; ///< Docking Flag

   RectSpacingI mPadding; ///< Padding for each side of the control to have as spacing between other controls
   ///  For example 1,1,1,1 would mean one pixel at least of spacing between this control and the
   ///  one next to it.  
   RectSpacingI mInternalPadding; ///< Interior Spacing of the control


   /// @name Anchoring
   /// @{
   /// @brief Anchors are applied to @b ONLY controls that are children of any derivative of a
   /// GuiContainer control.  Anchors are applied when a parent is resized and a child
   /// element should be resized to accommodate the new parent extent
   ///
   /// Anchors are specified as true or false and control whether a certain edge of a control
   /// will be locked to a certain edge of a parent, when the parent resizes.  Anchors are specified
   /// as a Mask and therefore you may lock any number of edges to a parent container and when the parent
   /// is resized, any locked edges on a control will remain the same distance from the parent edge it
   /// is locked to, after the resize happens.  
   ///
   bool mAnchorTop;     ///< Anchor to the Top edge of the parent when created
   bool mAnchorBottom;  ///< Anchor to the Bottom edge of the parent when created
   bool mAnchorLeft;    ///< Anchor to the Left edge of the parent when created
   bool mAnchorRight;   ///< Anchor to the Right edge of the parent when created
   /// @}

};

class GuiCursor : public SimObject
{
private:
   typedef SimObject Parent;
   StringTableEntry mBitmapName;

   Point2I mHotSpot;
   Point2F mRenderOffset;
   Point2I mExtent;
   GFXTexHandle mTextureObject;

public:
   Point2I getHotSpot() { return mHotSpot; }
   Point2I getExtent() { return mExtent; }

   DECLARE_CONOBJECT(GuiCursor);
   GuiCursor(void);
   ~GuiCursor(void);
   static void initPersistFields();

   bool onAdd(void);
   void onRemove();
   void render(const Point2I &pos);
};

/// A GuiControlProfile is used by every GuiObject and is akin to a
/// datablock. It is used to control information that does not change
/// or is unlikely to change during execution of a program. It is also
/// a level of abstraction between script and GUI control so that you can
/// use the same control, say a button, and have it look completly different
/// just with a different profile.
class GuiControlProfile : public SimObject
{
private:
   typedef SimObject Parent;

public:
   static StringTableEntry  sFontCacheDirectory;   ///< Directory where Torque will store font *.uft files.

   U32  mUseCount;                                 ///< Total number of controls currently referencing this profile.
   U32  mLoadCount;                                ///< Number of controls in woken state using this profile; resources for the profile are loaded when this is >0.
   bool mTabable;                                  ///< True if this object is accessable from using the tab key

   bool mCanKeyFocus;                              ///< True if the object can be given keyboard focus (in other words, made a first responder @see GuiControl)
   bool mModal;                                    ///< True if this is a Modeless dialog meaning it will pass input through instead of taking it all

   bool mOpaque;                                   ///< True if this object is not translucent, and should draw a fill
   ColorI mFillColor;                              ///< Fill color, this is used to fill the bounds of the control if it is opaque
   ColorI mFillColorHL;                            ///< This is used instead of mFillColor if the object is highlighted
   ColorI mFillColorNA;                            ///< This is used instead of mFillColor if the object is not active or disabled
   ColorI mFillColorSEL;                           ///< This is used instead of mFillColor if the object is selected

   S32 mBorder;                                    ///< For most controls, if mBorder is > 0 a border will be drawn, some controls use this to draw different types of borders however @see guiDefaultControlRender.cc
   S32 mBorderThickness;                           ///< Border thickness
   ColorI mBorderColor;                            ///< Border color, used to draw a border around the bounds if border is enabled
   ColorI mBorderColorHL;                          ///< Used instead of mBorderColor when the object is highlighted
   ColorI mBorderColorNA;                          ///< Used instead of mBorderColor when the object is not active or disabled

   ColorI mBevelColorHL;                          ///< Used for the high-light part of the bevel
   ColorI mBevelColorLL;                          ///< Used for the low-light part of the bevel

   // font members
   StringTableEntry  mFontType;                    ///< Font face name for the control
   S32               mFontSize;                    ///< Font size for the control
   enum {
      BaseColor = 0,
      ColorHL,
      ColorNA,
      ColorSEL,
      ColorUser0,
      ColorUser1,
      ColorUser2,
      ColorUser3,
      ColorUser4,
      ColorUser5,
   };
   ColorI  mFontColors[10];                        ///< Array of font colors used for drawText with escape characters for changing color mid-string
   ColorI& mFontColor;                             ///< Main font color
   ColorI& mFontColorHL;                           ///< Highlighted font color
   ColorI& mFontColorNA;                           ///< Font color when object is not active/disabled
   ColorI& mFontColorSEL;                          ///< Font color when object/text is selected
   FontCharset mFontCharset;                       ///< Font character set

   Resource<GFont>   mFont;                        ///< Font resource

   enum AlignmentType
   {
      LeftJustify,
      RightJustify,
      CenterJustify,
      TopJustify,
      BottomJustify
   };

   AlignmentType mAlignment;                       ///< Horizontal text alignment
   bool mAutoSizeWidth;                            ///< Auto-size the width-bounds of the control to fit it's contents
   bool mAutoSizeHeight;                           ///< Auto-size the height-bounds of the control to fit it's contents
   bool mReturnTab;                                ///< Used in GuiTextEditCtrl to specify if a tab-event should be simulated when return is pressed.
   bool mNumbersOnly;                              ///< For text controls, true if this should only accept numerical data
   bool mMouseOverSelected;                        ///< True if this object should be "selected" while the mouse is over it
   ColorI mCursorColor;                            ///< Color for the blinking cursor in text fields (for example)
	   
	Point2I mTextOffset;                            ///< Text offset for the control

   // bitmap members
   StringTableEntry mBitmapName;                   ///< Bitmap file name for the bitmap of the control
   bool mUseBitmapArray;                           ///< Flag to use the bitmap array or to fallback to non-array rendering
   GFXTexHandle mTextureObject;
   Vector<RectI> mBitmapArrayRects;                ///< Used for controls which use an array of bitmaps such as checkboxes

   // sound members
   SimObjectPtr< SFXTrack > mSoundButtonDown;                   ///< Sound played when the object is "down" ie a button is pushed
   SimObjectPtr< SFXTrack > mSoundButtonOver;                   ///< Sound played when the mouse is over the object

   StringTableEntry mChildrenProfileName;       ///< The name of the profile to use for the children controls

   /// Returns our children profile (and finds the profile if it hasn't been set yet)
   GuiControlProfile* getChildrenProfile();
   
   /// Category name for editing in the Gui Editor.
   String mCategory;

    /// Sets the children profile for this profile
    ///
    /// @see GuiControlProfile
    /// @param   prof   Tooltip profile to apply
    void setChildrenProfile(GuiControlProfile *prof);
protected:
   GuiControlProfile* mChildrenProfile;         ///< Profile used with children controls (such as the scroll bar on a popup menu) when defined.

   static bool protectedSetBitmap( void *object, const char *index, const char *data );
   static bool protectedSetSoundButtonDown( void* object, const char* index, const char* data );
   static bool protectedSetSoundButtonOver( void* object, const char* index, const char* data );
   static const char* protectedGetSoundButtonDown( void* object, const char* data );
   static const char* protectedGetSoundButtonOver( void* object, const char* data );

public:
   DECLARE_CONOBJECT(GuiControlProfile);
   GuiControlProfile();
   ~GuiControlProfile();
   static void initPersistFields();
   
   bool onAdd();

   void onStaticModified(const char* slotName, const char* newValue = NULL );

   /// Called when mProfileForChildren is deleted
    virtual void onDeleteNotify(SimObject *object);

   /// This method creates an array of bitmaps from one single bitmap with
   /// separator color. The separator color is whatever color is in pixel 0,0
   /// of the bitmap. For an example see darkWindow.png and some of the other
   /// UI textures. It returns the number of bitmaps in the array it created
   /// It also stores the sizes in the mBitmapArrayRects vector.
   S32 constructBitmapArray();
   
   /// This method returns the ith bitmap array rect, first ensuring that i is a
   /// valid index into mBitmapArrayRects. If the vector is empty, we call
   /// constructBitmapArray() automatically. If it is still empty, we return a 
   /// zeroed RectI.
   RectI getBitmapArrayRect(U32 i);

   ///
   bool isInUse() const { return ( mUseCount != 0 ); }
   
   void incUseCount() { mUseCount ++; }
   void decUseCount() { if( mUseCount > 0 ) mUseCount --; }

   void incLoadCount();
   void decLoadCount();
   
   bool loadFont();

   void setBitmapHandle(GFXTexHandle handle); 
};

typedef GuiControlProfile::AlignmentType GuiAlignmentType;
DefineEnumType( GuiAlignmentType );

typedef FontCharset GuiFontCharset;
DefineEnumType( GuiFontCharset );

GFX_DeclareTextureProfile(GFXGuiCursorProfile);
GFX_DeclareTextureProfile(GFXDefaultGUIProfile);

#endif //_GUITYPES_H
