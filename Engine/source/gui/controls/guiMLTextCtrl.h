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

#ifndef _GUIMLTEXTCTRL_H_
#define _GUIMLTEXTCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif

#ifndef _STRINGBUFFER_H_
#include "core/stringBuffer.h"
#endif

class GFont;
class SFXTrack;

GFX_DeclareTextureProfile(GFXMLTextureProfile);
class GuiMLTextCtrl : public GuiControl
{
   typedef GuiControl Parent;

   //-------------------------------------- Public interfaces...
  public:
   enum Justification
   {
      LeftJustify,
      RightJustify,
      CenterJustify,
   };

   struct Font {
      char *faceName;
      U32 faceNameLen;
      U32 size;
      Resource<GFont> fontRes;
      Font *next;
   };

   struct Bitmap {
      char bitmapName[1024];
      U32 bitmapNameLen;
      GFXTexHandle bitmapObject;
      Bitmap *next;
   };

   struct URL
   {
      bool mouseDown;
      U32 textStart;
      U32 len;
      bool noUnderline;
   };

   struct Style
   {
      ColorI color;
      ColorI shadowColor;
      ColorI linkColor;
      ColorI linkColorHL;
      Point2I shadowOffset;
      Font *font;
      bool used;
      Style *next;
   };

   struct Atom
   {
      U32 textStart;
      U32 len;
      U32 xStart;
      U32 yStart;
      U32 width;
      U32 baseLine;
      U32 descent;
      Style *style;
      bool isClipped;

      URL *url;
      Atom *next;
   };

   struct Line {
      U32 y;
      U32 height;
      U32 divStyle;
      U32 textStart;
      U32 len;
      Atom *atomList;
      Line *next;
   };

   struct BitmapRef : public RectI
   {
      BitmapRef *nextBlocker;
      U32 textStart;
      U32 len;
      Bitmap *bitmap;
      BitmapRef *next;
   };

   struct LineTag {
      U32 id;
      S32 y;
      LineTag *next;
   };

   GuiMLTextCtrl();
   ~GuiMLTextCtrl();

   DECLARE_CALLBACK( void, onURL, (const char* url));
   DECLARE_CALLBACK( void, onResize, ( S32 width, S32 maxY ));

   // Text retrieval functions
   U32 getNumChars() const;
   U32 getText(char* pBuffer, const U32 bufferSize) const;
   U32 getWrappedText(char* pBuffer, const U32 bufferSize) const;
   const char* getTextContent();
   void insertChars(const char* inputChars,
                    const U32   numInputChars,
                    const U32   position);

   // Text substitution functions
   void setText(const char* textBuffer, const U32 numChars);
   void addText(const char* textBuffer, const U32 numChars, bool reformat);

   void setAlpha(F32 alpha) { mAlpha = alpha;}

   bool setCursorPosition(const S32);
   void ensureCursorOnScreen();

   // Scroll functions
   void scrollToTag( U32 id );
   void scrollToTop();
   void scrollToBottom();

   virtual void reflow();

   DECLARE_CONOBJECT(GuiMLTextCtrl);
   DECLARE_CATEGORY( "Gui Text" );
   DECLARE_DESCRIPTION( "A control that displays multiple lines of text." );
   
   static void initPersistFields();

   void setScriptValue(const char *value);
   const char *getScriptValue();

   static char *stripControlChars(const char *inString);

   //-------------------------------------- Protected Structures and constants
  protected:
   bool mIsEditCtrl;

   U32 *mTabStops;
   U32 mTabStopCount;
   U32 mCurTabStop;

   F32 mAlpha;

   DataChunker mViewChunker;
   DataChunker mResourceChunker;
   Line *mLineList;
   Bitmap *mBitmapList;
   BitmapRef *mBitmapRefList;
   Font *mFontList;
   LineTag *mTagList;
   bool mDirty;
   Style *mCurStyle;

   U32 mCurLMargin;
   U32 mCurRMargin;
   U32 mCurJustify;
   U32 mCurDiv;
   U32 mCurY;
   U32 mCurClipX;
   Atom *mLineAtoms;
   Atom **mLineAtomPtr;

   Atom *mEmitAtoms;
   Atom **mEmitAtomPtr;

   BitmapRef mSentinel;
   Line **mLineInsert;
   BitmapRef *mBlockList;
   U32 mScanPos;
   U32 mCurX;
   U32 mMaxY;
   URL *mCurURL;

   URL *mHitURL;

   void freeLineBuffers();
   void freeResources();

   Bitmap *allocBitmap(const char *bitmapName, U32 bitmapNameLen);
   Font *allocFont(const char *faceName, U32 faceNameLen, U32 size);
   LineTag *allocLineTag(U32 id);
   void emitNewLine(U32 textStart);
   Atom *buildTextAtom(U32 start, U32 len, U32 left, U32 right, URL *url);
   void emitTextToken(U32 textStart, U32 len);
   void emitBitmapToken(Bitmap *bmp, U32 textStart, bool bitmapBreak);
   void processEmitAtoms();
   Atom *splitAtomListEmit(Atom *list, U32 width);
   void drawAtomText(bool sel, U32 start, U32 end, Atom *atom, Line *line, Point2I offset);
   Atom *findHitAtom(const Point2I localCoords);
   Style *allocStyle(Style *style);

   static const U32 csmTextBufferGrowthSize;

   //-------------------------------------- Data...
  protected:
   // Cursor position should always be <= mCurrTextSize
   U32  mCursorPosition;

   // Actual text data. The line buffer is rebuilt from the linear text
   //  given a specific width.  TextBuffer is /not/ \0 terminated
   StringBuffer mTextBuffer;
   U32   mLineStart;
	S32	mMaxBufferSize;
   StringTableEntry mInitialText;

   // Selection information
   bool mSelectionActive;
   U32  mSelectionStart;
   U32  mSelectionEnd;

   U32  mVertMoveAnchor;
   bool mVertMoveAnchorValid;

   S32     mSelectionAnchor;
   Point2I mSelectionAnchorDropped;

   // Font resource
   Resource<GFont> mFont;

   // Console settable parameters
   U32 mLineSpacingPixels;
   bool mAllowColorChars;
   bool mUseURLMouseCursor;

   // Too many chars sound:
   SFXTrack*  mDeniedSound;

   //-------------------------------------- Protected interface
  protected:
   // Inserting and deleting character blocks...
   void deleteChars(const U32 rangeStart,
                    const U32 rangeEnd);
   void copyToClipboard(const U32 rangeStart,
                     const U32 rangeEnd);

   // Selection maintainence
   bool isSelectionActive() const;
   void clearSelection();

   // Pixel -> text position mappings
   S32 getTextPosition(const Point2I& localPosition);

	// Gui control overrides
   bool onWake();
   void onSleep();
   void onPreRender();
   void onRender(Point2I offset, const RectI &updateRect);
   void getCursorPositionAndColor(Point2I &cursorTop, Point2I &cursorBottom, ColorI &color);
   void inspectPostApply();
   void parentResized(const RectI& oldParentRect, const RectI& newParentRect);
   bool onKeyDown(const GuiEvent& event);
   void onMouseDown(const GuiEvent&);
   void onMouseDragged(const GuiEvent&);
   void onMouseUp(const GuiEvent&);

   virtual void getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent);

  public:
   // Gui control overrides
   bool onAdd();

   void setSelectionStart( U32 start ) { clearSelection(); mSelectionStart = start; };
   void setSelectionEnd( U32 end ) { mSelectionEnd = end;};
   void setSelectionActive(bool active) { mSelectionActive = active; };
   S32 getCursorPosition()  { return( mCursorPosition ); }

   virtual bool resize(const Point2I &newPosition, const Point2I &newExtent);
};

#endif  // _H_GUIMLTEXTCTRL_
