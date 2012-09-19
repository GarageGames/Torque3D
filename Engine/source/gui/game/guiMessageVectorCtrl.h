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

#ifndef _GUIMESSAGEVECTORCTRL_H_
#define _GUIMESSAGEVECTORCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif
#ifndef _MESSAGEVECTOR_H_
#include "gui/utility/messageVector.h"
#endif

/// Render a MessageVector (chat HUD)
///
/// This renders messages from a MessageVector; the important thing
/// here is that the MessageVector holds all the messages we care
/// about, while we can destroy and create these GUI controls as
/// needed. (For instance, Tribes 2 used seperate GuiMessageVectorCtrl
/// controls in the different HUD modes.)
class GuiMessageVectorCtrl : public GuiControl
{
   typedef GuiControl Parent;

   //-------------------------------------- Public interfaces...
  public:
   struct SpecialMarkers {
      struct Special {
         S32 specialType;
         S32 start;
         S32 end;
      };

      U32      numSpecials;
      Special* specials;
   };

   GuiMessageVectorCtrl();
   ~GuiMessageVectorCtrl();

   bool isAttached() const;
   bool attach(MessageVector*);
   void detach();

   // Gui control overrides
  protected:
   bool onAdd();
   void onRemove();

   bool onWake();
   void onSleep();
   void onRender(Point2I offset, const RectI &updateRect);
   void inspectPostApply();
   void parentResized(const RectI& oldParentRect, const RectI& newParentRect);

   void onMouseUp(const GuiEvent &event);
   void onMouseDown(const GuiEvent &event);
//   void onMouseMove(const GuiEvent &event);

   // Overrideables
  protected:
   virtual void lineInserted(const U32);
   virtual void lineDeleted(const U32);
   virtual void vectorDeleted();

   MessageVector* mMessageVector;

   // Font resource
  protected:
   U32             mMinSensibleWidth;

   U32             mLineSpacingPixels;
   U32             mLineContinuationIndent;

   StringTableEntry mAllowedMatches[16];
   ColorI           mSpecialColor;

   U32   mMaxColorIndex;

   bool mMouseDown;
   S32  mMouseSpecialLine;
   S32  mMouseSpecialRef;

   /// Derived classes must keep this set of variables consistent if they
   ///  use this classes onRender.  Note that this has the fairly large
   ///  disadvantage of requiring lots of ugly, tiny mem allocs.  A rewrite
   ///  to a more memory friendly version might be a good idea...
   struct LineWrapping {
      struct SEPair {
         S32 start;
         S32 end;
      };

      U32     numLines;
      SEPair* startEndPairs;     /// start[linex] = startEndPairs[linex*2+0]
                                 /// end[linex]   = startEndPairs[linex*2+1]
                                 /// if end < start, line is empty...
   };

   struct TextElement {
      TextElement* nextInLine;
      TextElement* nextPhysicalLine;

      S32 specialReference;  /// if (!= -1), indicates a special reference

      S32 start;
      S32 end;
   };
   struct LineElement {
      U32 physicalLineStart;

      TextElement* headLineElements;
   };

   Vector<LineWrapping>   mLineWrappings;
   Vector<SpecialMarkers> mSpecialMarkers;
   Vector<LineElement>    mLineElements;

   void createLineWrapping(LineWrapping&, const char*);
   void createSpecialMarkers(SpecialMarkers&, const char*);
   void createLineElement(LineElement&, LineWrapping&, SpecialMarkers&);

   void findSpecialFromCoord(const Point2I&, S32*, S32*);

  public:
   void callbackRouter(const MessageVector::MessageCode, const U32);

  public:
   
   DECLARE_CONOBJECT(GuiMessageVectorCtrl);
   DECLARE_CATEGORY( "Gui Game" );
   DECLARE_DESCRIPTION( "A chat HUD control that displays messages from a MessageVector." );
   
   static void initPersistFields();
};

#endif  // _H_GUIMESSAGEVECTORCTRL_
