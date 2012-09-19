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

#include "gui/controls/guiMLTextCtrl.h"
#include "gui/containers/guiScrollCtrl.h"
#include "console/consoleTypes.h"
#include "gui/core/guiCanvas.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "core/frameAllocator.h"
#include "core/strings/unicode.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxTypes.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT( GuiMLTextCtrl );

ConsoleDocClass( GuiMLTextCtrl,
   "@brief A text control that uses the Gui Markup Language ('ML') tags to dynamically change the text.\n\n"

   "Example of dynamic changes include colors, styles, and/or hyperlinks. These changes can occur without "
   "having to use separate text controls with separate text profiles.\n\n"

   "@tsexample\n"
   "new GuiMLTextCtrl(CenterPrintText)\n"
   "{\n"
   "    lineSpacing = \"2\";\n"
   "    allowColorChars = \"0\";\n"
   "    maxChars = \"-1\";\n"
   "    deniedSound = \"DeniedSoundProfile\";\n"
   "    text = \"The Text for This Control.\";\n"
   "    useURLMouseCursor = \"true\";\n"
   "    //Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@see GuiControl\n\n"

   "@ingroup GuiCore\n"
);


IMPLEMENT_CALLBACK( GuiMLTextCtrl, onURL, void, ( const char* url ),( url ),
   "@brief Called whenever a URL was clicked on within the control.\n\n"
   "@param url The URL address that was clicked on.\n"
   "@tsexample\n"
   "// A URL address was clicked on in the control, causing the callback to occur.\n"
   "GuiMLTextCtrl::onUrl(%this,%url)\n"
   "	{\n"
   "		// Code to run whenever a URL was clicked on\n"
   "	}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

IMPLEMENT_CALLBACK( GuiMLTextCtrl, onResize, void, ( const char* width, const char* maxY ),( width, maxY ),
   "@brief Called whenever the control size changes.\n\n"
   "@param width The new width value for the control\n"
   "@param maxY The current maximum allowed Y value for the control\n\n"
   "@tsexample\n"
   "// Control size changed, causing the callback to occur.\n"
   "GuiMLTextCtrl::onResize(%this,%width,%maxY)\n"
   "	{\n"
   "		// Code to call when the control size changes\n"
   "	}\n"
   "@endtsexample\n\n"
   "@see GuiControl\n\n"
);

GFX_ImplementTextureProfile(GFXMLTextureProfile,
                            GFXTextureProfile::DiffuseMap, 
                            GFXTextureProfile::PreserveSize |
                            GFXTextureProfile::Static, 
                            GFXTextureProfile::None);

const U32 GuiMLTextCtrl::csmTextBufferGrowthSize = 1024;

DefineEngineMethod( GuiMLTextCtrl, setText, void, (const char* text),,
   "@brief Set the text contained in the control.\n\n"
   "@param text The text to display in the control.\n"
   "@tsexample\n"
   "// Define the text to display\n"
   "%text = \"Nifty Control Text\";\n\n"
   "// Set the text displayed within the control\n"
   "%thisGuiMLTextCtrl.setText(%text);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setText(text, dStrlen(text));
}

DefineEngineMethod( GuiMLTextCtrl, getText, const char*, (),,
   "@brief Returns the text from the control, including TorqueML characters.\n\n"
   "@tsexample\n"
   "// Get the text displayed in the control\n"
   "%controlText = %thisGuiMLTextCtrl.getText();\n"
   "@endtsexample\n\n"
   "@return Text string displayed in the control, including any TorqueML characters.\n\n"
   "@see GuiControl")
{
   return( object->getTextContent() );
}

DefineEngineMethod( GuiMLTextCtrl, addText, void, ( const char* text, bool reformat),,
   "@brief Appends the text in the control with additional text. Also .\n\n"
   "@param text New text to append to the existing text.\n"
   "@param reformat If true, the control will also be visually reset.\n"
   "@tsexample\n"
   "// Define new text to add\n"
   "%text = \"New Text to Add\";\n\n"
   "// Set reformat boolean\n"
   "%reformat = \"true\";\n\n"
   "// Inform the control to add the new text\n"
   "%thisGuiMLTextCtrl.addText(%text,%reformat);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->addText(text, dStrlen(text), reformat);
}

DefineEngineMethod( GuiMLTextCtrl, setCursorPosition, bool, (int newPos),,
   "@brief Change the text cursor's position to a new defined offset within the text in the control.\n\n"
   "@param newPos Offset to place cursor.\n"
   "@tsexample\n"
   "// Define cursor offset position\n"
   "%position = \"23\";\n\n"
   "// Inform the GuiMLTextCtrl object to move the cursor to the new position.\n"
   "%thisGuiMLTextCtrl.setCursorPosition(%position);\n"
   "@endtsexample\n\n"
   "@return Returns true if the cursor position moved, or false if the position was not changed.\n\n"
   "@see GuiControl")
{
   return object->setCursorPosition(newPos);
}

DefineEngineMethod( GuiMLTextCtrl, scrollToTag, void, (int tagID),,
   "@brief Scroll down to a specified tag.\n\n"
   "Detailed description\n\n"
   "@param tagID TagID to scroll the control to\n"
   "@tsexample\n"
   "// Define the TagID we want to scroll the control to\n"
   "%tagId = \"4\";\n\n"
   "// Inform the GuiMLTextCtrl to scroll to the defined TagID\n"
   "%thisGuiMLTextCtrl.scrollToTag(%tagId);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->scrollToTag( tagID );
}

DefineEngineMethod( GuiMLTextCtrl, scrollToTop, void, ( S32 param1, S32 param2),,
   "@brief Scroll to the top of the text.\n\n"
   "@tsexample\n"
   "// Inform GuiMLTextCtrl object to scroll to its top\n"
   "%thisGuiMLTextCtrl.scrollToTop();\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->scrollToTop();
}

DefineEngineMethod( GuiMLTextCtrl, scrollToBottom, void, (),,
   "@brief Scroll to the bottom of the text.\n\n"
   "@tsexample\n"
   "// Inform GuiMLTextCtrl object to scroll to its bottom\n"
   "%thisGuiMLTextCtrl.scrollToBottom();\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->scrollToBottom();
}

DefineEngineFunction(StripMLControlChars, const char*, (const char* inString),,
					 "@brief Strip TorqueML control characters from the specified string, returning a 'clean' version.\n\n"
					 "@param inString String to strip TorqueML control characters from.\n"
					 "@tsexample\n"
					 "// Define the string to strip TorqueML control characters from\n"
					 "%string = \"<font:Arial:24>How Now <color:c43c12>Brown <color:000000>Cow\";\n\n"
					 "// Request the stripped version of the string\n"
					 "%strippedString = StripMLControlChars(%string);\n"
					 "@endtsexample\n\n"
					 "@return Version of the inputted string with all TorqueML characters removed.\n\n"
					 "@see References\n\n"
					 "@ingroup GuiCore")
{
   return GuiMLTextCtrl::stripControlChars(inString);
}

DefineEngineMethod( GuiMLTextCtrl, forceReflow, void, (),,
   "@brief Forces the text control to reflow the text after new text is added, possibly resizing the control.\n\n"
   "@tsexample\n"
   "// Define new text to add\n"
   "%newText = \"BACON!\";\n\n"
   "// Add the new text to the control\n"
   "%thisGuiMLTextCtrl.addText(%newText);\n\n"
   "// Inform the GuiMLTextCtrl object to force a reflow to ensure the added text fits properly.\n"
   "%thisGuiMLTextCtrl.forceReflow();\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   if(!object->isAwake())
      Con::errorf("GuiMLTextCtrl::forceReflow can only be called on visible controls.");
   else
      object->reflow();
}

//--------------------------------------------------------------------------
GuiMLTextCtrl::GuiMLTextCtrl()
: mTabStops( NULL ), 
  mTabStopCount( 0 ), 
  mCurTabStop( 0 ), 
  mCurStyle( NULL ), 
  mCurLMargin( 0 ), 
  mCurRMargin( 100 ), 
  mCurJustify( LeftJustify ), 
  mCurDiv( 0 ),
  mCurY( 0 ), 
  mCurClipX( 0 ), 
  mLineAtoms( NULL ), 
  mLineAtomPtr( &mLineAtoms ), 
  mLineList( NULL ),
  mLineInsert( &mLineList ), 
  mScanPos( 0 ),
  mCurX( 0 ),
  mMaxY( 0 ),
  mCurURL( NULL ),
  mLineStart( 0 ),
  mVertMoveAnchor( 0 ),
  mVertMoveAnchorValid( false ),
  mSelectionAnchor( 0 ),
  mIsEditCtrl( false ),
  mCursorPosition( false ),
  mMaxBufferSize( -1 ),
  mInitialText( StringTable->insert("") ),
  mSelectionActive( false ),
  mSelectionStart( 0 ),
  mSelectionEnd( 0 ),
  mLineSpacingPixels( 2 ),
  mAllowColorChars( false ),
  mUseURLMouseCursor( false ),
  mBitmapList( 0 ),
  mBitmapRefList( 0 ),
  mDirty( true ),  
  mTagList( NULL ),
  mHitURL( 0 ),
  mAlpha( 1.0f ),
  mFontList( NULL )
{   
   mActive = true;
   //mInitialText = StringTable->insert("");
   Sim::findObject("InputDeniedSound", mDeniedSound);
}

//--------------------------------------------------------------------------
GuiMLTextCtrl::~GuiMLTextCtrl()
{
   mCursorPosition     = 0;

   mSelectionActive = false;
   mSelectionStart  = 0;
   mSelectionEnd    = 0;
   freeResources();
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::initPersistFields()
{
   addGroup( "Text" );
   
      addField("lineSpacing",       TypeS32,    Offset(mLineSpacingPixels, GuiMLTextCtrl), "The number of blank pixels to place between each line.\n");
      addField("allowColorChars",   TypeBool,   Offset(mAllowColorChars,   GuiMLTextCtrl), "If true, the control will allow characters to have unique colors.");
      addField("maxChars",          TypeS32,    Offset(mMaxBufferSize,     GuiMLTextCtrl), "Maximum number of characters that the control will display.");
      addField("deniedSound",       TypeSFXTrackName, Offset(mDeniedSound, GuiMLTextCtrl), "If the text will not fit in the control, the deniedSound is played.");
      addField("text",              TypeCaseString,  Offset( mInitialText, GuiMLTextCtrl ), "Text to display in this control.");
      addField("useURLMouseCursor", TypeBool,   Offset(mUseURLMouseCursor,   GuiMLTextCtrl), "If true, the mouse cursor will turn into a hand cursor while over a link in the text.\n"
																							 "This is dependant on the markup language used by the GuiMLTextCtrl\n");
   
   endGroup( "Text" );
   
   Parent::initPersistFields();
}

DefineEngineMethod( GuiMLTextCtrl, setAlpha, void, (F32 alphaVal),,
   "@brief Sets the alpha value of the control.\n\n"
   "@param alphaVal n - 1.0 floating value for the alpha\n"
   "@tsexample\n"
   "// Define the alphe value\n"
   "%alphaVal = \"0.5\";\n\n"
   "// Inform the control to update its alpha value.\n"
   "%thisGuiMLTextCtrl.setAlpha(%alphaVal);\n"
   "@endtsexample\n\n"
   "@see GuiControl")
{
   object->setAlpha(alphaVal);
}

//--------------------------------------------------------------------------

bool GuiMLTextCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if (!mTextBuffer.length() && mInitialText[0] != 0)
      setText(mInitialText, dStrlen(mInitialText)+1);
   return true;
}

//--------------------------------------------------------------------------
bool GuiMLTextCtrl::onWake()
{
   if (Parent::onWake() == false)
      return false;

   mDirty = true;
   return true;
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::onPreRender()
{
   if(mDirty)
      reflow();
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::drawAtomText(bool sel, U32 start, U32 end, Atom *atom, Line *line, Point2I offset)
{
   GFont *font = atom->style->font->fontRes;
   U32 xOff = 0;
   if(start != atom->textStart)
   {
      const UTF16* buff = mTextBuffer.getPtr() + atom->textStart;
      xOff += font->getStrNWidth(buff, start - atom->textStart);
   }

   Point2I drawPoint(offset.x + atom->xStart + xOff, offset.y + atom->yStart);

   ColorI color;
   if(atom->url)
   { 
      if(atom->url->mouseDown)
         color = atom->style->linkColorHL;
      else
         color = atom->style->linkColor;
   }
   else
      color = atom->style->color;
            
   const UTF16* tmp = mTextBuffer.getPtr() + start;
   U32 tmpLen = end-start;

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   if(!sel)
   {
      if(atom->style->shadowOffset.x || atom->style->shadowOffset.y)
      {
         ColorI shadowColor = atom->style->shadowColor;
         shadowColor.alpha = (S32)(mAlpha * shadowColor.alpha);
         drawer->setBitmapModulation(shadowColor);
         drawer->drawTextN(font, drawPoint + atom->style->shadowOffset, 
            tmp, tmpLen, mAllowColorChars ? mProfile->mFontColors : NULL);
      }

      color.alpha = (S32)(mAlpha * color.alpha);
      drawer->setBitmapModulation(color);
      drawer->drawTextN(font, drawPoint, tmp, end-start, mAllowColorChars ? mProfile->mFontColors : NULL);

      //if the atom was "clipped", see if we need to draw a "..." at the end
      if (atom->isClipped)
      {
         Point2I p2 = drawPoint;
         p2.x += font->getStrNWidthPrecise(tmp, tmpLen);
         drawer->drawTextN(font, p2, "...", 3, mAllowColorChars ? mProfile->mFontColors : NULL);
      }
   }
   else
   {
      RectI rect;
      rect.point.x = drawPoint.x;
      rect.point.y = line->y + offset.y;
      rect.extent.x = font->getStrNWidth(tmp, tmpLen) + 1;
      rect.extent.y = line->height + 1;
      
      drawer->drawRectFill(rect, mProfile->mFillColorHL);
      drawer->setBitmapModulation( mProfile->mFontColorHL );  // over-ride atom color:
      drawer->drawTextN(font, drawPoint, tmp, tmpLen, mAllowColorChars ? mProfile->mFontColors : NULL);

      //if the atom was "clipped", see if we need to draw a "..." at the end
      if (atom->isClipped)
      {
         Point2I p2 = drawPoint;
         p2.x += font->getStrNWidthPrecise(tmp, end - atom->textStart);
         drawer->drawTextN(font, p2, "...", 3, mAllowColorChars ? mProfile->mFontColors : NULL);
      }
   }

   if(atom->url && !atom->url->noUnderline)
   {
      drawPoint.y += atom->baseLine + 2;
      Point2I p2 = drawPoint;
      p2.x += font->getStrNWidthPrecise(tmp, end - atom->textStart);
      drawer->drawLine(drawPoint, p2, color);
   }
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::onRender(Point2I offset, const RectI& updateRect)
{
   Parent::onRender(offset, updateRect);

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   // draw all the bitmaps
   for(BitmapRef *walk = mBitmapRefList; walk; walk = walk->next)
   {
      RectI screenBounds = *walk;
      screenBounds.point += offset;
      if(!screenBounds.overlaps(updateRect))
         continue;

      drawer->clearBitmapModulation();
      drawer->drawBitmap(walk->bitmap->bitmapObject, screenBounds.point);
      //GFX->drawRectFill(screenBounds, mProfile->mFillColor);
   }

   offset += mProfile->mTextOffset;

   // draw all the text and dividerStyles
   for(Line *lwalk = mLineList; lwalk; lwalk = lwalk->next)
   {
      RectI lineRect(offset.x, offset.y + lwalk->y, getWidth(), lwalk->height);

      if(!lineRect.overlaps(updateRect))
         continue;

      if(lwalk->divStyle)
         drawer->drawRectFill(lineRect, mProfile->mFillColorHL);

      for(Atom *awalk = lwalk->atomList; awalk; awalk = awalk->next)
      {
         if(!mSelectionActive || mSelectionEnd < awalk->textStart || mSelectionStart >= awalk->textStart + awalk->len)
            drawAtomText(false, awalk->textStart, awalk->textStart + awalk->len, awalk, lwalk, offset);
         else
         {
            U32 selectionStart = getMax(awalk->textStart, mSelectionStart);
            U32 selectionEnd = getMin(awalk->textStart + awalk->len, mSelectionEnd + 1);

            // draw some unselected text
            if(selectionStart > awalk->textStart)
               drawAtomText(false, awalk->textStart, selectionStart, awalk, lwalk, offset);

            // draw the selection
            drawAtomText(true, selectionStart, selectionEnd, awalk, lwalk, offset);

            if(selectionEnd < awalk->textStart + awalk->len)
               drawAtomText(false, selectionEnd, awalk->textStart + awalk->len, awalk, lwalk, offset);
         }
      }
   }

   drawer->clearBitmapModulation();
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::freeLineBuffers()
{
   mViewChunker.freeBlocks();
   mLineList = NULL;
   mBitmapRefList = NULL;
   mTagList = NULL;
   mHitURL = 0;
   mDirty = true;
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::freeResources()
{
   for(Font* walk = mFontList; walk; walk = walk->next)
   {
      walk->fontRes = NULL;
      delete[] walk->faceName;
   }

   for(Bitmap* bwalk = mBitmapList; bwalk; bwalk = bwalk->next)
      bwalk->bitmapObject = 0;

   mFontList = NULL;
   mBitmapList = NULL;
   mResourceChunker.freeBlocks();
   mDirty = true;

   freeLineBuffers();
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::onSleep()
{
   freeResources();
   Parent::onSleep();
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::inspectPostApply()
{
   Parent::inspectPostApply();

   setText(mInitialText, dStrlen(mInitialText));

   if (mLineSpacingPixels < 0)
      mLineSpacingPixels = 0;
}


//--------------------------------------------------------------------------
void GuiMLTextCtrl::parentResized(const RectI& oldParentRect, const RectI& newParentRect)
{
   Parent::parentResized(oldParentRect, newParentRect);
   mDirty = true;
}

//--------------------------------------------------------------------------
U32 GuiMLTextCtrl::getNumChars() const
{
   return mTextBuffer.length();
}

//--------------------------------------------------------------------------
U32 GuiMLTextCtrl::getText(char* pBuffer, const U32 bufferSize) const
{
   mTextBuffer.getCopy8(pBuffer, bufferSize);

   return getMin(mTextBuffer.length(), bufferSize);
}

//--------------------------------------------------------------------------
const char* GuiMLTextCtrl::getTextContent()
{
   if ( mTextBuffer.length() > 0 )
   {
      UTF8* returnString = Con::getReturnBuffer( mTextBuffer.getUTF8BufferSizeEstimate() );
      mTextBuffer.getCopy8(returnString, mTextBuffer.getUTF8BufferSizeEstimate() );
      return returnString;
   }

   return( "" );
}

//--------------------------------------------------------------------------
const char *GuiMLTextCtrl::getScriptValue()
{
   return getTextContent();
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::setScriptValue(const char *newText)
{
   setText(newText, dStrlen(newText));
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::setText(const char* textBuffer, const U32 numChars)
{
   U32 chars = numChars;
   if(numChars >= mMaxBufferSize)
      chars = mMaxBufferSize;

   // leaving this usage because we StringBuffer.set((UTF8*)) cannot take a numChars arg.
   // perhaps it should? -paxorr
   FrameTemp<UTF8> tmp(chars+1);
   dStrncpy(tmp, textBuffer, chars);
   tmp[chars] = 0;

   mTextBuffer.set(tmp);

   //after setting text, always set the cursor to the beginning
   setCursorPosition(0);
   clearSelection();
   mDirty = true;
   scrollToTop();
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::addText(const char* textBuffer, const U32 numChars, bool reformat)
{
   if(numChars >= mMaxBufferSize)
      return;

   FrameTemp<UTF8> tmp(numChars+1);
   dStrncpy(tmp, textBuffer, numChars);
   tmp[numChars] = 0;

   mTextBuffer.append(tmp);

   //after setting text, always set the cursor to the beginning
   if (reformat)
   {
      setCursorPosition(0);
      clearSelection();
      mDirty = true;
      scrollToTop();
   }
}

//--------------------------------------------------------------------------
bool GuiMLTextCtrl::setCursorPosition(const S32 newPosition)
{
   if (newPosition < 0) 
   {
      mCursorPosition = 0;
      return true;
   }
   else if (newPosition >= mTextBuffer.length()) 
   {
      mCursorPosition = mTextBuffer.length();
      return true;
   }
   else 
   {
      mCursorPosition = newPosition;
      return false;
   }
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::ensureCursorOnScreen()
{
   // If our parent isn't a scroll control, or we're not the only control
   //  in the content region, bail...
   GuiControl* pParent = getParent();
	GuiScrollCtrl *sc = dynamic_cast<GuiScrollCtrl*>(pParent);
	if(!sc)
		return;

   // Ok.  Now we know that our parent is a scroll control.  Let's find the
   //  top of the cursor, and it's bottom.  We can then scroll the parent control
   //  if appropriate...

   Point2I cursorTopP, cursorBottomP;
   ColorI color;
   getCursorPositionAndColor(cursorTopP, cursorBottomP, color);

	sc->scrollRectVisible(RectI(cursorTopP.x, cursorTopP.y, 1, cursorBottomP.y - cursorTopP.y));
}

//--------------------------------------
void GuiMLTextCtrl::getCursorPositionAndColor(Point2I &cursorTop, Point2I &cursorBottom, ColorI &color)
{
   S32 x = 0;
   S32 y = 0;
   S32 height = mProfile->mFont->getHeight();
   color = mProfile->mCursorColor;
   for(Line *walk = mLineList; walk; walk = walk->next)
   {
      if ((mCursorPosition <= walk->textStart + walk->len) || (walk->next == NULL))
      {
         // it's in the atoms on this line...
         y = walk->y;
         height = walk->height;

         for(Atom *awalk = walk->atomList; awalk; awalk = awalk->next)
         {

            if(mCursorPosition < awalk->textStart)
            {
               x = awalk->xStart;
               goto done;
            }

            if(mCursorPosition > awalk->textStart + awalk->len)
            {
               x = awalk->xStart + awalk->width;
               continue;
            }

            // it's in the text block...
            x = awalk->xStart;
            GFont *font = awalk->style->font->fontRes;

            const UTF16* buff = mTextBuffer.getPtr() + awalk->textStart;
            x += font->getStrNWidth(buff, mCursorPosition - awalk->textStart);// - 1);

            color = awalk->style->color;
            goto done;
         }

         //if it's within this walk's width, but we didn't find an atom, leave the cursor at the beginning of the line...
         goto done;
      }
   }
done:
   cursorTop.set(x, y);
   cursorBottom.set(x, y + height);
}

//--------------------------------------------------------------------------
// Keyboard events...
bool GuiMLTextCtrl::onKeyDown(const GuiEvent& event)
{
   //only cut/copy work with this control...
   if (event.modifier & SI_COPYPASTE)
   {
      switch(event.keyCode)
      {
         //copy
         case KEY_C:
         {
            //make sure we actually have something selected
            if (mSelectionActive)
            {
               copyToClipboard(mSelectionStart, mSelectionEnd);
               setUpdate();
            }
            return true;
         }
         
         default:
            break;
      }
   }

   // Otherwise, let the parent have the event...
   return Parent::onKeyDown(event);
}

//--------------------------------------------------------------------------
// Mousing events...
void GuiMLTextCtrl::onMouseDown(const GuiEvent& event)
{
   if(!mActive)
      return;

   Atom *hitAtom = findHitAtom(globalToLocalCoord(event.mousePoint));
   if(hitAtom && !mIsEditCtrl)
   {
      mouseLock();
      mHitURL = hitAtom->url;
      if (mHitURL)
         mHitURL->mouseDown = true;
   }

   setFirstResponder();
   mouseLock();

   mSelectionActive = false;
   mSelectionAnchor        = getTextPosition(globalToLocalCoord(event.mousePoint));
   mSelectionAnchorDropped = event.mousePoint;
   if (mSelectionAnchor < 0)
      mSelectionAnchor = 0;
   else if (mSelectionAnchor >= mTextBuffer.length())
      mSelectionAnchor = mTextBuffer.length();

   mVertMoveAnchorValid = false;

   setUpdate();
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::onMouseDragged(const GuiEvent& event)
{
   if (!mActive || (getRoot()->getMouseLockedControl() != this))
      return;

   Atom *hitAtom = findHitAtom(globalToLocalCoord(event.mousePoint));
   bool down = false;

   //note mHitURL can't be set unless this is (!mIsEditCtrl)
   if(hitAtom && hitAtom->url == mHitURL)
      down = true;

   if(mHitURL && down != mHitURL->mouseDown)
      mHitURL->mouseDown = down;

   if (!mHitURL)
   {
      S32 newSelection = 0;
      newSelection = getTextPosition(globalToLocalCoord(event.mousePoint));
      if (newSelection < 0)
         newSelection = 0;
      else if (newSelection > mTextBuffer.length())
         newSelection = mTextBuffer.length();

      if (newSelection == mSelectionAnchor) 
      {
         mSelectionActive = false;
      }
      else if (newSelection > mSelectionAnchor) 
      {
         mSelectionActive = true;
         mSelectionStart  = mSelectionAnchor;
         mSelectionEnd    = newSelection - 1;
      }
      else 
      {
         mSelectionStart  = newSelection;
         mSelectionEnd    = mSelectionAnchor - 1;
         mSelectionActive = true;
      }
      setCursorPosition(newSelection);
      mDirty = true;
   }

   setUpdate();
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::onMouseUp(const GuiEvent& event)
{
   if (!mActive || (getRoot()->getMouseLockedControl() != this))
      return;

   mouseUnlock();

   //see if we've clicked on a URL
   Atom *hitAtom = findHitAtom(globalToLocalCoord(event.mousePoint));
   if (mHitURL && hitAtom && hitAtom->url == mHitURL && mHitURL->mouseDown)
   {
      mHitURL->mouseDown = false;

      // Convert URL from UTF16 to UTF8.
      UTF8* url = mTextBuffer.createSubstring8(mHitURL->textStart, mHitURL->len);
	  onURL_callback(url);

      delete[] url;
      mHitURL = NULL;

      setUpdate();
      return;
   }

   //else, update our selection
   else
   {
      if ((event.mousePoint - mSelectionAnchorDropped).len() < 3)
         mSelectionActive = false;

      setCursorPosition(getTextPosition(globalToLocalCoord(event.mousePoint)));
      mVertMoveAnchorValid = false;
      setUpdate();
   }
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent)
{
   if(!mUseURLMouseCursor)
   {
      Parent::getCursor(cursor, showCursor, lastGuiEvent);
      return;
   }

   GuiCanvas *pRoot = getRoot();
   if( !pRoot )
      return;

   PlatformWindow *pWindow = pRoot->getPlatformWindow();
   AssertFatal(pWindow != NULL,"GuiControl without owning platform window!  This should not be possible.");
   PlatformCursorController *pController = pWindow->getCursorController();
   AssertFatal(pController != NULL,"PlatformWindow without an owned CursorController!");

   Atom *hitAtom = findHitAtom(globalToLocalCoord(lastGuiEvent.mousePoint));
   if(hitAtom && !mIsEditCtrl && hitAtom->url)
   {
      if(pRoot->mCursorChanged != PlatformCursorController::curHand)
      {
         // We've already changed the cursor, so set it back before we change it again.
         if(pRoot->mCursorChanged != -1)
            pController->popCursor();

         // Now change the cursor shape
         pController->pushCursor(PlatformCursorController::curHand);
         pRoot->mCursorChanged = PlatformCursorController::curHand;

      }
   }
   else if(pRoot->mCursorChanged != -1)
   {
         // Restore the cursor we changed
         pController->popCursor();
         pRoot->mCursorChanged = -1;
   }
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::insertChars(const char* inputChars,
                                const U32   numInputChars,
                                const U32   position)
{
   AssertFatal(isSelectionActive() == false, "GuiMLTextCtrl::insertChars: don't use this function when there's a selection active!");
   AssertFatal(position <= mTextBuffer.length(), "GuiMLTextCtrl::insertChars: can't insert outside of current text!");

   //make sure the text will fit...
   S32 numCharsToInsert = numInputChars;
   if (mMaxBufferSize > 0 && mTextBuffer.length() + numInputChars + 1 > mMaxBufferSize)
      numCharsToInsert = mMaxBufferSize - mTextBuffer.length() - 1;
   if (numCharsToInsert <= 0)
   {
      // Play the "Denied" sound:
      if ( numInputChars > 0 && mDeniedSound )
         SFX->playOnce(mDeniedSound);

      return;
   }

   mTextBuffer.insert(position, inputChars );

   if (mCursorPosition >= position) 
   {
      // Cursor was at or after the inserted text, move forward...
      mCursorPosition += numCharsToInsert;
   }

   AssertFatal(mCursorPosition <= mTextBuffer.length(), "GuiMLTextCtrl::insertChars: bad cursor position");
   mDirty = true;
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::deleteChars(const U32 rangeStart,
                                const U32 rangeEnd)
{
   AssertFatal(isSelectionActive() == false, "GuiMLTextCtrl::deleteChars: don't use this function when there's a selection active");
   AssertFatal(rangeStart <= mTextBuffer.length() && rangeEnd <= mTextBuffer.length(),
               avar("GuiMLTextCtrl::deleteChars: can't delete outside of current text (%d, %d, %d)",
                    rangeStart, rangeEnd, mTextBuffer.length()));
   AssertFatal(rangeStart <= rangeEnd, "GuiMLTextCtrl::deleteChars: invalid delete range");

   // Currently deleting text doesn't resize the text buffer, perhaps this should
   //  change?
   mTextBuffer.cut(rangeStart, rangeEnd - rangeStart);

   if (mCursorPosition <= rangeStart) 
   {
      // Cursor placed before deleted text, ignore
   }
   else if (mCursorPosition > rangeStart && mCursorPosition <= rangeEnd) 
   {
      // Cursor inside deleted text, set to start of range
      mCursorPosition = rangeStart;
   }
   else 
   {
      // Cursor after deleted text, decrement by number of chars deleted
      mCursorPosition -= (rangeEnd - rangeStart) + 1;
   }

   AssertFatal(mCursorPosition <= mTextBuffer.length(), "GuiMLTextCtrl::deleteChars: bad cursor position");
   mDirty = true;
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::copyToClipboard(const U32 rangeStart, const U32 rangeEnd)
{
   AssertFatal(rangeStart < mTextBuffer.length() && rangeEnd < mTextBuffer.length(),
               avar("GuiMLTextCtrl::copyToClipboard: can't copy outside of current text (%d, %d, %d)",
                    rangeStart, rangeEnd, mTextBuffer.length()));
   AssertFatal(rangeStart <= rangeEnd, "GuiMLTextCtrl::copyToClipboard: invalid copy range");

   //copy the selection to the clipboard

   UTF8* selection = mTextBuffer.createSubstring8(rangeStart, rangeEnd-rangeStart+1);
   Platform::setClipboard(selection);
   delete[] selection;
}

//--------------------------------------------------------------------------
bool GuiMLTextCtrl::isSelectionActive() const
{
   return mSelectionActive;
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::clearSelection()
{
   mSelectionActive = false;
   mSelectionStart  = 0;
   mSelectionEnd    = 0;
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::scrollToTag( U32 id )
{
   // If the parent control is not a GuiScrollContentCtrl, then this call is invalid:
   GuiScrollCtrl *pappy = dynamic_cast<GuiScrollCtrl*>(getParent());
   if ( !pappy )
      return;

   // Find the indicated tag:
   LineTag* tag = NULL;
   for ( tag = mTagList; tag; tag = tag->next )
   {
      if ( tag->id == id )
         break;
   }

   if ( !tag )
   {
      Con::warnf( ConsoleLogEntry::General, "GuiMLTextCtrl::scrollToTag - tag id %d not found!", id );
      return;
   }
	pappy->scrollRectVisible(RectI(0, tag->y, 1, 1));
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::scrollToTop()
{
   // If the parent control is not a GuiScrollContentCtrl, then this call is invalid:
   GuiScrollCtrl *pappy = dynamic_cast<GuiScrollCtrl*>(getParent());
   if ( !pappy )
      return;
	pappy->scrollRectVisible(RectI(0,0,0,0));
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::scrollToBottom()
{
   // If the parent control is not a GuiScrollContentCtrl, then this call is invalid:
   GuiScrollCtrl *pappy = dynamic_cast<GuiScrollCtrl*>(getParent());
   if ( !pappy )
      return;

   // Figure bounds for the bottom left corner
   RectI cornerBounds (0, getPosition().y + getExtent().y, 1, 1);
   pappy->scrollRectVisible(cornerBounds);
}

//--------------------------------------------------------------------------
GuiMLTextCtrl::Atom *GuiMLTextCtrl::findHitAtom(const Point2I localCoords)
{
   AssertFatal(mAwake, "Can't get the text position of a sleeping control.");
   if(mDirty)
      reflow();
   for(Line *walk = mLineList; walk; walk = walk->next)
   {
      if(localCoords.y < walk->y)
         return NULL;

      if(localCoords.y >= walk->y && localCoords.y < walk->y + walk->height)
      {
         for(Atom *awalk = walk->atomList; awalk; awalk = awalk->next)
         {
            if(localCoords.x < awalk->xStart)
               return NULL;
            if(localCoords.x >= awalk->xStart + awalk->width)
               continue;
            return awalk;
         }
      }
   }
   return NULL;
}

//--------------------------------------------------------------------------
S32 GuiMLTextCtrl::getTextPosition(const Point2I& localCoords)
{
   AssertFatal(mAwake, "Can't get the text position of a sleeping control.");
   if(mDirty)
      reflow();

   for(Line *walk = mLineList; walk; walk = walk->next)
   {
      if((S32)localCoords.y < (S32)walk->y)
         return walk->textStart;

      if(localCoords.y >= walk->y && localCoords.y < walk->y + walk->height)
      {
         for(Atom *awalk = walk->atomList; awalk; awalk = awalk->next)
         {
            if(localCoords.x < awalk->xStart)
               return awalk->textStart;
            if(localCoords.x >= awalk->xStart + awalk->width)
               continue;
            // it's in the text block...
            GFont *font = awalk->style->font->fontRes;

            const UTF16 *tmp16 = mTextBuffer.getPtr() + awalk->textStart;
            U32 bp = font->getBreakPos(tmp16, awalk->len, localCoords.x - awalk->xStart, false);
            return awalk->textStart + bp;
         }
         return walk->textStart + walk->len;
      }
   }
   return mTextBuffer.length() - 1;
}

//--------------------------------------------------------------------------
GuiMLTextCtrl::Font *GuiMLTextCtrl::allocFont(const char *faceName, U32 faceNameLen, U32 size)
{
   // check if it's in the font list currently:
   for(Font *walk = mFontList; walk; walk = walk->next)
      if(faceNameLen == walk->faceNameLen &&
         !dStrncmp(walk->faceName, faceName, faceNameLen) &&
         size == walk->size)
         return walk;

   // Create!
   Font *ret;
   ret = constructInPlace((Font *) mResourceChunker.alloc(sizeof(Font)));
   ret->faceName = new char[faceNameLen+1];
   dStrncpy(ret->faceName, faceName, faceNameLen);
   ret->faceName[faceNameLen] = '\0';
   ret->faceNameLen = faceNameLen;
   ret->size = size;
   ret->next = mFontList;
   ret->fontRes = GFont::create(ret->faceName, size, GuiControlProfile::sFontCacheDirectory);
   if(ret->fontRes != NULL)
   {
      ret->next = mFontList;
      mFontList = ret;
      return ret;
   }
   return NULL;
}

//--------------------------------------------------------------------------
GuiMLTextCtrl::Bitmap *GuiMLTextCtrl::allocBitmap(const char *bitmapName, U32 bitmapNameLen)
{
   for(Bitmap *walk = mBitmapList; walk; walk = walk->next)
      if(bitmapNameLen == walk->bitmapNameLen &&
         !dStrncmp(walk->bitmapName, bitmapName, bitmapNameLen))
         return walk;

   Bitmap *ret = constructInPlace((Bitmap *) mResourceChunker.alloc(sizeof(Bitmap)));
   const U32 BitmapNameSize = sizeof(ret->bitmapName);
   dStrncpy(ret->bitmapName, bitmapName, getMin(bitmapNameLen,BitmapNameSize));
   if (bitmapNameLen < BitmapNameSize)
      ret->bitmapName[bitmapNameLen] = 0;
   else
      ret->bitmapName[BitmapNameSize - 1] = 0;
   ret->bitmapNameLen = bitmapNameLen;
      
   ret->bitmapObject.set(ret->bitmapName, &GFXMLTextureProfile, avar("%s() - ret->bitmapObject (line %d)", __FUNCTION__, __LINE__));
   //ret->bitmapObject.set(bitmapName, &GFXMLTextureProfile);
   if( bool(ret->bitmapObject) )
   {
      ret->next = mBitmapList;
      mBitmapList = ret;
      return ret;
   }
   return NULL;
}

//--------------------------------------------------------------------------
GuiMLTextCtrl::LineTag *GuiMLTextCtrl::allocLineTag(U32 id)
{
   for ( LineTag* walk = mTagList; walk; walk = walk->next )
   {
      if ( walk->id == id )
      {
         Con::warnf( ConsoleLogEntry::General, "GuiMLTextCtrl - can't add duplicate line tags!" );
         return( NULL );
      }
   }
   LineTag* newTag = (LineTag*) mViewChunker.alloc( sizeof( LineTag ) );
   newTag->id = id;
   newTag->y = mCurY;
   newTag->next = mTagList;
   mTagList = newTag;

   return( newTag );
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::emitNewLine(U32 textStart)
{
   //clear any clipping
   mCurClipX = 0;

   Line *l = (Line *) mViewChunker.alloc(sizeof(Line));
   l->height = mCurStyle->font->fontRes->getHeight();
   l->y = mCurY;
   l->textStart = mLineStart;
   l->len = textStart - l->textStart;
   mLineStart = textStart;
   l->atomList = mLineAtoms;
   l->next = 0;
   l->divStyle = mCurDiv;
   *mLineInsert = l;
   mLineInsert = &(l->next);
   mCurX = mCurLMargin;
   mCurTabStop = 0;

   if(mLineAtoms)
   {
      // scan through the atoms in the line, get the largest height
      U32 maxBaseLine = 0;
      U32 maxDescent = 0;
      Atom* walk;

      for(walk = mLineAtoms; walk; walk = walk->next)
      {
         if(walk->baseLine > maxBaseLine)
            maxBaseLine = walk->baseLine;
         if(walk->descent > maxDescent)
            maxDescent = walk->descent;
         if(!walk->next)
         {
            l->len = walk->textStart + walk->len - l->textStart;
            mLineStart = walk->textStart + walk->len;
         }
      }
      l->height = maxBaseLine + maxDescent;

      for(walk = mLineAtoms; walk; walk = walk->next)
         walk->yStart = mCurY + maxBaseLine - walk->baseLine;
   }
   mCurY += l->height;
   mLineAtoms = NULL;
   mLineAtomPtr = &mLineAtoms;

   // clear out the blocker list
   BitmapRef **blockList = &mBlockList;
   while(*blockList)
   {
      BitmapRef *blk = *blockList;
      if(blk->point.y + blk->extent.y <= mCurY)
         *blockList = blk->nextBlocker;
      else
         blockList = &(blk->nextBlocker);
   }
   if(mCurY > mMaxY)
      mMaxY = mCurY;
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::emitBitmapToken(GuiMLTextCtrl::Bitmap *bmp, U32 textStart, bool bitmapBreak)
{
   if(mCurRMargin <= mCurLMargin)
      return;
   if(mCurRMargin - mCurLMargin < bmp->bitmapObject->getWidth())
      return;

   BitmapRef *ref = (BitmapRef *) mViewChunker.alloc(sizeof(BitmapRef));
   ref->bitmap = bmp;
   ref->next = mBitmapRefList;
   mBitmapRefList = ref;

   // now we gotta insert it into the blocker list and figure out where it's spos to go...
   ref->extent.x = bmp->bitmapObject->getBitmapWidth();
   ref->extent.y = bmp->bitmapObject->getBitmapHeight();

   // find the first space in the blocker list that will fit this thats > curLMargin

   while(bitmapBreak && mBlockList != &mSentinel)
      emitNewLine(textStart);

   for(;;)
   {
      // loop til we find a line that fits...
      // we'll have to emitLine repeatedly to clear out the block lists...

      BitmapRef **walk = &mBlockList;
      U32 minx = mCurX;
      U32 maxx = mCurRMargin;

      while(*walk)
      {
         BitmapRef *blk = *walk;

         if(blk->point.x > minx)
         {
            U32 right = maxx;
            if(blk->point.x < right)
               right = blk->point.x;
            U32 width = right - minx;

            if(right > minx && width >= ref->extent.x) // we've found the spot...
            {
               // insert it:
               U32 x = minx;
               if(mCurJustify == CenterJustify)
                  x += (width - ref->extent.x) >> 1;
               else if(mCurJustify == RightJustify)
                  x += width - ref->extent.x;
               ref->point.x = x;
               ref->point.y = mCurY;
               ref->nextBlocker = blk;
               *walk = ref;
               if(ref->point.y + ref->extent.y > mMaxY)
                  mMaxY = ref->point.y + ref->extent.y;

               return;
            }
         }
         if(minx < blk->point.x + blk->extent.x)
            minx = blk->point.x + blk->extent.x;
         // move on to the next blocker...
         walk = &(blk->nextBlocker);
      }
      // go to the next line...
      emitNewLine(textStart);
   }
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::emitTextToken(U32 textStart, U32 len)
{
   if(mCurRMargin <= mCurLMargin)
      return;

   GFont *font = mCurStyle->font->fontRes;
   Atom *a = (Atom *) mViewChunker.alloc(sizeof(Atom));
   a->url = mCurURL;

   a->style = mCurStyle;
   mCurStyle->used = true;

   a->baseLine = font->getBaseline();
   a->descent = font->getDescent();
   a->textStart = textStart;
   a->len = len;
   a->isClipped = false;
   a->next = NULL;
   *mEmitAtomPtr = a;
   mEmitAtomPtr = &(a->next);
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::processEmitAtoms()
{
   Atom *atomList = mEmitAtoms;
   mEmitAtoms = NULL;
   mEmitAtomPtr = &mEmitAtoms;

   bool bailout = false;

   while(atomList)
   {
      // split the tokenlist by space
      // first find the first space that the text can go into:
      BitmapRef *br = mBlockList;
      //bool bailout = false; // Scoping error here? Moved up one scope. -pw
      Atom *list = atomList;

      while(br && atomList)
      {
         // if the blocker is before the current x, ignore it.
         if(br->point.x + br->extent.x <= mCurX)
         {
            br = br->nextBlocker;
            continue;
         }
         // if cur x is in the middle of the blocker
         // advance cur x to right edge of blocker.
         if(mCurX >= br->point.x)
         {
            mCurX = br->point.x + br->extent.x;
            br = br->nextBlocker;
            continue;
         }
         // get the remaining width
         U32 right = br->point.x;
         if(right > mCurRMargin)
            right = mCurRMargin;

         //if we're clipping text, readjust
         if (mCurClipX > 0 && right > mCurClipX)
            right = mCurClipX;

         // if there's no room, break to the next line...
         if(right <= mCurX)
            break;

         // we've got some space:
         U32 width = right - mCurX;
         atomList = splitAtomListEmit(atomList, width);
         if(atomList) // there's more, so advance cur x
         {
            mCurX = br->point.x + br->extent.x;
            br = br->nextBlocker;
         }
      }
      if(mBlockList == &mSentinel && atomList == list)
      {
         if(bailout)
            return;
         else
            bailout = true;
      }
      // is there more to process for the next line?
      if(atomList)
         emitNewLine(mScanPos);
   }
}

//--------------------------------------------------------------------------
GuiMLTextCtrl::Atom *GuiMLTextCtrl::splitAtomListEmit(Atom *list, U32 width)
{
   U32 totalWidth = 0;
   Atom *emitList = 0;
   Atom **emitPtr = &emitList;

   bool adjustClipAtom = false;
   Atom *clipAtom = NULL;
   bool emitted = false;

   while(list)
   {
      GFont *font = list->style->font->fontRes;
      U32 breakPos;

      const UTF16 *tmp16 = mTextBuffer.getPtr() + list->textStart;

      //if we're clipping the text, we don't break within an atom, we adjust the atom to only render
      //the portion of text that does fit, and to ignore the rest...
      if (mCurClipX > 0)
      {
         //find out how many character's fit within the given width
         breakPos = font->getBreakPos(tmp16, list->len, width - totalWidth, false);

         //if there isn't room for even the first character...
         if (breakPos == 0)
         {
            //set the atom's len and width to prevent it from being drawn
            list->len = 0;
            list->width = 0;
            adjustClipAtom = true;
         }

         //if our text doesn't fit within the clip region, add a "..."
         else if (breakPos != list->len)
         {
            U32 etcWidth = font->getStrNWidthPrecise("...", 3);
            breakPos = font->getBreakPos(tmp16, list->len, width - totalWidth - etcWidth, false);

            //again, if there isn't even room for a single character before the "...."
            if (breakPos == 0)
            {
               //set the atom's len and width to prevent it from being drawn
               list->len = 0;
               list->width = 0;
               adjustClipAtom = true;
            }
            else
            {
               //set the char len to the break pos, and the rest of the characters in this atom will be ignored
               list->len = breakPos;
               list->width = width - totalWidth;

               //mark this one as clipped
               list->isClipped = true;
               clipAtom = NULL;
            }
         }
      
         //otherwise no need to treat this atom any differently..
         else
         {
            //set the atom width == to the string length
            list->width = font->getStrNWidthPrecise(tmp16, breakPos);

            //set the pointer to the last atom that fit within the clip region
            clipAtom = list;
         }
      }
      else
      {
         breakPos = font->getBreakPos(tmp16, list->len, width - totalWidth, true);
         if(breakPos == 0 || (breakPos < list->len && mTextBuffer.getChar(list->textStart + breakPos - 1)!= ' ' && emitted))
            break;

         //set the atom width == to the string length
         list->width = font->getStrNWidthPrecise(tmp16, breakPos);
      }

      //update the total width
      totalWidth += list->width;
      
      // see if this is the last atom that will fit:
      Atom *emit = list;
      
      *emitPtr = emit;
      emitPtr = &(emit->next);
      emitted = true;

      //if we're clipping, don't split the atom, otherwise, see if it needs to be split
      if(!list->isClipped && breakPos != list->len)
      {
         Atom *a = (Atom *) mViewChunker.alloc(sizeof(Atom));
         a->url = list->url;
         a->textStart = list->textStart + breakPos;
         a->len = list->len - breakPos;
         a->next = list->next;
         a->baseLine = list->baseLine;
         a->descent = list->descent;
         a->style = list->style;
         a->isClipped = false;
         
         list = a;
         emit->len = breakPos;
         break;
      }
      list = list->next;
      if(totalWidth > width)
         break;
   }

   //if we had to completely clip an atom(s), the last (partially) visible atom should be modified to include a "..."
   if (adjustClipAtom && clipAtom)
   {
      GFont *font = clipAtom->style->font->fontRes;
      U32 breakPos;

      U32 etcWidth = font->getStrNWidthPrecise("...", 3);

      const UTF16 *tmp16 = mTextBuffer.getPtr() + clipAtom->textStart;

      breakPos = font->getBreakPos(tmp16, clipAtom->len, clipAtom->width - etcWidth, false);
      if (breakPos != 0)
      {
         clipAtom->isClipped = true;
         clipAtom->len = breakPos;
      }
   }

   // terminate the emit list:
   *emitPtr = 0;
   // now emit it:
   // going from mCurX to mCurX + width:
   if(mCurJustify == CenterJustify)
   {
      if ( width > totalWidth )
         mCurX += (width - totalWidth) >> 1;
   }
   else if(mCurJustify == RightJustify)
   {
      if ( width > totalWidth )
         mCurX += width - totalWidth;
   }


   while(emitList)
   {
      emitList->xStart = mCurX;
      mCurX += emitList->width;
      Atom *temp = emitList->next;
      *mLineAtomPtr = emitList;
      emitList->next = 0;
      mLineAtomPtr = &(emitList->next);
      emitList = temp;
   }
   
   return list;
}

//--------------------------------------------------------------------------
static bool scanforchar(const char *str, U32 &idx, char c)
{
   U32 startidx = idx;
   while(str[idx] != c && str[idx] && str[idx] != ':' && str[idx] != '>' && str[idx] != '\n')
      idx++;
   return str[idx] == c && startidx != idx;
}

//--------------------------------------------------------------------------
static bool scanforURL(const char *str, U32 &idx, char c)
{
   U32 startidx = idx;
   while(str[idx] != c && str[idx] && str[idx] != '>' && str[idx] != '\n')
      idx++;
   return str[idx] == c && startidx != idx;
}

//--------------------------------------------------------------------------
static S32 getHexVal(char c)
{
   if(c >= '0' && c <= '9')
      return c - '0';
   else if(c >= 'A' && c <= 'Z')
      return c - 'A' + 10;
   else if(c >= 'a' && c <= 'z')
      return c - 'a' + 10;
   return -1;
}

//--------------------------------------------------------------------------
GuiMLTextCtrl::Style *GuiMLTextCtrl::allocStyle(GuiMLTextCtrl::Style *style)
{
   Style *ret = (Style *) mViewChunker.alloc(sizeof(Style));
   ret->used = false;
   if(style)
   {
      ret->font = style->font;
      ret->color = style->color;
      ret->linkColor = style->linkColor;
      ret->linkColorHL = style->linkColorHL;
      ret->shadowColor = style->shadowColor;
      ret->shadowOffset = style->shadowOffset;
      ret->next = style->next;
   }
   else
   {
      ret->font = 0;
      ret->next = 0;
   }
   return ret;
}

//--------------------------------------------------------------------------
void GuiMLTextCtrl::reflow()
{
   AssertFatal(mAwake, "Can't reflow a sleeping control.");
   freeLineBuffers();
   mDirty = false;
   mScanPos = 0;

   mLineList = NULL;
   mLineInsert = &mLineList;

   mCurStyle = allocStyle(NULL);
   mCurStyle->font = allocFont((char *) mProfile->mFontType, dStrlen(mProfile->mFontType), mProfile->mFontSize);
   if(!mCurStyle->font)
      return;
   mCurStyle->color = mProfile->mFontColor;
   mCurStyle->shadowColor = mProfile->mFontColor;
   mCurStyle->shadowOffset.set(0,0);
   mCurStyle->linkColor = mProfile->mFontColors[GuiControlProfile::ColorUser0];
   mCurStyle->linkColorHL = mProfile->mFontColors[GuiControlProfile::ColorUser1];

   U32 width = getWidth();

   mCurLMargin = 0;
   mCurRMargin = width;
   mCurJustify = LeftJustify;
   mCurDiv = 0;
   mCurY = 0;
   mCurX = 0;
   mCurClipX = 0;
   mLineAtoms = NULL;
   mLineAtomPtr = &mLineAtoms;

   mSentinel.point.x = width;
   mSentinel.point.y = 0;
   mSentinel.extent.x = 0;
   mSentinel.extent.y = 0x7FFFFF;
   mSentinel.nextBlocker = NULL;
   mLineStart = 0;
   mEmitAtoms = 0;
   mMaxY = 0;
   mEmitAtomPtr = &mEmitAtoms;

   mBlockList = &mSentinel;

   Font *nextFont;
   LineTag *nextTag;
   mTabStops = 0;
   mCurTabStop = 0;
   mTabStopCount = 0;
   mCurURL = 0;
   Style *newStyle;

   U32 textStart;
   U32 len;
   U32 idx;
   U32 sizidx;

   for(;;)
   {
      UTF16 curChar = mTextBuffer.getChar(mScanPos);

      if(!curChar)
         break;

      if(curChar == '\n')
      {
         textStart = mScanPos;
         len = 1;
         mScanPos++;
         processEmitAtoms();
         emitNewLine(textStart);
         mCurDiv = 0;
         continue;
      }

      if(curChar == '\t')
      {
         textStart = mScanPos;
         len = 1;
         mScanPos++;
         processEmitAtoms();
         if(mTabStopCount)
         {
            if(mCurTabStop < mTabStopCount)
            {
               if(mCurX < mTabStops[mCurTabStop])
                  mCurX = mTabStops[mCurTabStop];
            }
            mCurTabStop++;
         }
         continue;
      }

      if(curChar == '<')
      {
         // it's probably some kind of tag:

         // Get a pointer into the utf8 version of the buffer,
         // because we're still scanning text in in utf8 mode.
         const UTF8 *str = mTextBuffer.getPtr8();
         str = getNthCodepoint(str, mScanPos);

         //  And go!

         if(!dStrnicmp(str + 1, "br>", 3))
         {
            mScanPos += 4;
            len = 4;
            textStart = mScanPos + 4;
            processEmitAtoms();
            emitNewLine(textStart);
            mCurDiv = 0;
            continue;
         }

         if(!dStrnicmp(str + 1, "font:", 5))
         {
            // scan for the second colon...
            // at each level it should drop out to the text case below...
            idx = 6;
            if(!scanforchar(str, idx, ':'))
               goto textemit;

            sizidx = idx + 1;
            if(!scanforchar(str, sizidx, '>'))
               goto textemit;

            U32 size = dAtoi(str + idx + 1);
            if(!size || size > 64)
               goto textemit;
            textStart = mScanPos + 6;
            len = idx - 6;

            mScanPos += sizidx + 1;

            nextFont = allocFont(str + 6, len, size);
            if(nextFont)
            {
               if(mCurStyle->used)
                  mCurStyle = allocStyle(mCurStyle);
               mCurStyle->font = nextFont;
            }
            continue;
         }

         if ( !dStrnicmp( str + 1, "tag:", 4 ) )
         {
            idx = 5;
            if ( !scanforchar( str, idx, '>' ) )
               goto textemit;
            U32 tagId = dAtoi( str + 5 );
            nextTag = allocLineTag( tagId );

            mScanPos += idx + 1;
            continue;
         }

         if(!dStrnicmp(str +1, "color:", 6))
         {
            idx = 7;
            if(!scanforchar(str, idx, '>'))
               goto textemit;
            if(idx != 13 && idx != 15)
               goto textemit;
            ColorI color;

            color.red = getHexVal(str[7]) * 16 + getHexVal(str[8]);
            color.green = getHexVal(str[9]) * 16 + getHexVal(str[10]);
            color.blue = getHexVal(str[11]) * 16 + getHexVal(str[12]);
            if(idx == 15)
               color.alpha = getHexVal(str[13]) * 16 + getHexVal(str[14]);
            else
               color.alpha = 255;
            mScanPos += idx + 1;

            if(mCurStyle->used)
               mCurStyle = allocStyle(mCurStyle);
            mCurStyle->color = color;

            continue;
         }

         if(!dStrnicmp(str +1, "shadowcolor:", 12))
         {
            idx = 13;
            if(!scanforchar(str, idx, '>'))
               goto textemit;
            if(idx != 19 && idx != 21)
               goto textemit;
            ColorI color;

            color.red   = getHexVal(str[13]) * 16 + getHexVal(str[14]);
            color.green = getHexVal(str[15]) * 16 + getHexVal(str[16]);
            color.blue  = getHexVal(str[17]) * 16 + getHexVal(str[18]);
            if(idx == 21)
               color.alpha = getHexVal(str[19]) * 16 + getHexVal(str[20]);
            else
               color.alpha = 255;
            mScanPos += idx + 1;

            if(mCurStyle->used)
               mCurStyle = allocStyle(mCurStyle);
            mCurStyle->shadowColor = color;

            continue;
         }

         if(!dStrnicmp(str +1, "linkcolor:", 10))
         {
            idx = 11;
            if(!scanforchar(str, idx, '>'))
               goto textemit;
            if(idx != 17 && idx != 19)
               goto textemit;
            ColorI color;

            color.red   = getHexVal(str[11]) * 16 + getHexVal(str[12]);
            color.green = getHexVal(str[13]) * 16 + getHexVal(str[14]);
            color.blue  = getHexVal(str[15]) * 16 + getHexVal(str[16]);
            if(idx == 19)
               color.alpha = getHexVal(str[17]) * 16 + getHexVal(str[18]);
            else
               color.alpha = 255;
            mScanPos += idx + 1;

            if(mCurStyle->used)
               mCurStyle = allocStyle(mCurStyle);
            mCurStyle->linkColor = color;

            continue;
         }

         if(!dStrnicmp(str +1, "linkcolorhl:", 12))
         {
            idx = 13;
            if(!scanforchar(str, idx, '>'))
               goto textemit;
            if(idx != 19 && idx != 21)
               goto textemit;
            ColorI color;

            color.red = getHexVal(str[13]) * 16 + getHexVal(str[14]);
            color.green = getHexVal(str[15]) * 16 + getHexVal(str[16]);
            color.blue = getHexVal(str[17]) * 16 + getHexVal(str[18]);
            if(idx == 21)
               color.alpha = getHexVal(str[19]) * 16 + getHexVal(str[20]);
            else
               color.alpha = 255;
            mScanPos += idx + 1;

            if(mCurStyle->used)
               mCurStyle = allocStyle(mCurStyle);
            mCurStyle->linkColorHL = color;

            continue;
         }
         if(!dStrnicmp(str +1, "shadow:", 7))
         {
            idx = 8;
            if(!scanforchar(str, idx, ':'))
               goto textemit;
            U32 yidx = idx + 1;
            if(!scanforchar(str, yidx, '>'))
               goto textemit;
            mScanPos += yidx + 1;
            Point2I offset;
            offset.x = dAtoi(str + 8);
            offset.y = dAtoi(str + idx + 1);
            if(mCurStyle->used)
               mCurStyle = allocStyle(mCurStyle);
            mCurStyle->shadowOffset = offset;
            continue;
         }
         if(!dStrnicmp(str +1, "bitmap", 6))
         {
            S32 start = 8;
            bool bitBrk = false;
            if(str[7] == 'k' && str[8] == ':')
            {
               bitBrk = true;
               start = 9;
            }
            else if(str[7] != ':')
               goto textemit;

            idx = start;
            if(!scanforchar(str, idx, '>'))
               goto textemit;
            textStart = mScanPos + start;
            len = idx - start;

            mScanPos += idx + 1;

            processEmitAtoms();
            Bitmap *bmp;
            bmp = allocBitmap(str + 8, len);
            if(bmp)
               emitBitmapToken(bmp, textStart, bitBrk);
            continue;
         }

         if(!dStrnicmp(str +1, "spush>", 6))
         {
            mScanPos += 7;
            newStyle = allocStyle(mCurStyle); // copy out all the attributes...
            newStyle->next = mCurStyle;
            mCurStyle = newStyle;
            continue;
         }

         if(!dStrnicmp(str +1, "spop>", 5))
         {
            mScanPos += 6;
            if(mCurStyle->next)
               mCurStyle = mCurStyle->next;
            continue;
         }

         if(!dStrnicmp(str +1, "sbreak>", 7))
         {
            mScanPos += 8;
            processEmitAtoms();
            while(mBlockList != &mSentinel)
               emitNewLine(mScanPos);
            continue;
         }

         if(!dStrnicmp(str +1, "just:left>", 10))
         {
            processEmitAtoms();
            mCurJustify = LeftJustify;
            mScanPos += 11;
            continue;
         }

         if(!dStrnicmp(str +1, "just:right>", 11))
         {
            processEmitAtoms();
            mCurJustify = RightJustify;
            mScanPos += 12;
            continue;
         }

         if(!dStrnicmp(str +1, "just:center>", 12))
         {
            processEmitAtoms();
            mCurJustify = CenterJustify;
            mScanPos += 13;
            continue;
         }

         if(!dStrnicmp(str +1, "a:", 2))
         {
            idx = 3;
            if(!scanforURL(str, idx, '>'))
               goto textemit;

            mCurURL = (URL *) mViewChunker.alloc(sizeof(URL));
            mCurURL->mouseDown = false;
            mCurURL->textStart = mScanPos + 3;
            mCurURL->len = idx - 3;
            mCurURL->noUnderline = false;

            //if the URL is a "gamelink", don't underline...
            if (!dStrnicmp(str + 3, "gamelink", 8))
               mCurURL->noUnderline = true;

            mScanPos += idx + 1;
            continue;
         }

         if(!dStrnicmp(str+1, "/a>", 3))
         {
            mCurURL = NULL;
            mScanPos += 4;
            continue;
         }

         U32 margin;

         if(!dStrnicmp(str + 1, "lmargin%:", 9))
         {
            idx = 10;
            if(!scanforchar(str, idx, '>'))
               goto textemit;
            margin = (getWidth() * dAtoi(str + 10)) / 100;
            mScanPos += idx + 1;
            goto setleftmargin;
         }

         if(!dStrnicmp(str + 1, "lmargin:", 8))
         {
            idx = 9;
            if(!scanforchar(str, idx, '>'))
               goto textemit;
            margin = dAtoi(str + 9);
            mScanPos += idx + 1;
setleftmargin:
            processEmitAtoms();
            U32 oldLMargin;
            oldLMargin = mCurLMargin;
            mCurLMargin = margin;
            if(mCurLMargin >= width)
               mCurLMargin = width - 1;
            if(mCurX == oldLMargin)
               mCurX = mCurLMargin;
            if(mCurX < mCurLMargin)
               mCurX = mCurLMargin;
            continue;
         }

         if(!dStrnicmp(str + 1, "rmargin%:", 9))
         {
            idx = 10;
            if(!scanforchar(str, idx, '>'))
               goto textemit;
            margin = (getWidth() * dAtoi(str + 10)) / 100;
            mScanPos += idx + 1;
            goto setrightmargin;
         }

         if(!dStrnicmp(str + 1, "rmargin:", 8))
         {
            idx = 9;
            if(!scanforchar(str, idx, '>'))
               goto textemit;
            margin = dAtoi(str + 9);
            mScanPos += idx + 1;
setrightmargin:
            processEmitAtoms();
            mCurRMargin = margin;
            if(mCurLMargin >= width)
               mCurLMargin = width;
            if (mCurClipX > mCurRMargin)
               mCurClipX = mCurRMargin;
            continue;
         }

         if(!dStrnicmp(str + 1, "clip:", 5))
         {
            U32 clipWidth = 0;
            idx = 6;
            if(!scanforchar(str, idx, '>'))
               goto textemit;
            clipWidth = dAtoi(str + 6);
            mScanPos += idx + 1;
            processEmitAtoms();
            if (clipWidth > 0)
               mCurClipX = mCurX + clipWidth;
            else
               mCurClipX = 0;
            if(mCurClipX > mCurRMargin)
               mCurClipX = mCurRMargin;
            continue;
         }

         if(!dStrnicmp(str + 1, "/clip>", 6))
         {
            processEmitAtoms();
            mCurClipX = 0;
            mScanPos += 7;
            continue;
         }

         if(!dStrnicmp(str + 1, "div:", 4))
         {
            idx = 5;
            if(!scanforchar(str, idx, '>'))
               goto textemit;
            mScanPos += idx + 1;
            mCurDiv = dAtoi(str + 5);
            continue;
         }

         if(!dStrnicmp(str + 1, "tab:", 4))
         {
            idx = 5;
            if(!scanforchar(str, idx, '>'))
               goto textemit;
            // scan for tab stops...
            mTabStopCount = 1;
            idx = 5;
            while(scanforchar(str, idx, ','))
            {
               idx++;
               mTabStopCount++;
            }
            idx = 5;
            mTabStops = (U32 *) mViewChunker.alloc(sizeof(U32) * mTabStopCount);
            mTabStops[0] = dAtoi(str + idx);
            U32 i = 1;

            while(scanforchar(str, idx, ','))
            {
               idx++;
               mTabStops[i] = dAtoi(str + idx);
               i++;
            }
            mScanPos += idx + 1;
            continue;
         }
      }

      // default case:
textemit:
      textStart = mScanPos;
      idx = 1;
      while(mTextBuffer.getChar(mScanPos+idx) != '\t' && mTextBuffer.getChar(mScanPos+idx) != '<' && mTextBuffer.getChar(mScanPos+idx) != '\n' && mTextBuffer.getChar(mScanPos+idx))
         idx++;
      len = idx;
      mScanPos += idx;
      emitTextToken(textStart, len);
   }
   processEmitAtoms();
   emitNewLine(mScanPos);
   setHeight( mMaxY );
   onResize_callback(Con::getIntArg( getWidth() ), Con::getIntArg( mMaxY ) );
	
   //make sure the cursor is still visible - this handles if we're a child of a scroll ctrl...
   ensureCursorOnScreen();
}

//-----------------------------------------------------------------------------
char* GuiMLTextCtrl::stripControlChars(const char *inString)
{
   if (! bool(inString))
      return NULL;
   U32 maxBufLength = 64;
   char *strippedBuffer = Con::getReturnBuffer(maxBufLength);
   char *stripBufPtr = &strippedBuffer[0];
   const char *bufPtr = (char *) inString;
   U32 idx, sizidx;

   for(;;)
   {
      //if we've reached the end of the string, or run out of room in the stripped Buffer...
      if(*bufPtr == '\0' || (U32(stripBufPtr - strippedBuffer) >= maxBufLength - 1))
         break;

      if (*bufPtr == '\n')
      {
         U32 walked;
         oneUTF8toUTF32(bufPtr,&walked);
         bufPtr += walked; 
         continue;
      }
      if(*bufPtr == '\t')
      {
         U32 walked;
         oneUTF8toUTF32(bufPtr,&walked);
         bufPtr += walked;
         continue;
      }
      if(*bufPtr < 0x20 && *bufPtr >= 0)
      {
         U32 walked;
         oneUTF8toUTF32(bufPtr,&walked);
         bufPtr += walked;
         continue;
      }

      if(*bufPtr == '<')
      {
         // it's probably some kind of tag:
         if(!dStrnicmp(bufPtr + 1, "font:", 5))
         {
            // scan for the second colon...
            // at each level it should drop out to the text case below...
            idx = 6;
            if(!scanforchar((char*)bufPtr, idx, ':'))
               goto textemit;

            sizidx = idx + 1;
            if(!scanforchar((char*)bufPtr, sizidx, '>'))
               goto textemit;

            bufPtr += sizidx + 1;
            continue;
         }

         if (!dStrnicmp(bufPtr + 1, "tag:", 4 ))
         {
            idx = 5;
            if ( !scanforchar((char*)bufPtr, idx, '>' ))
               goto textemit;

            bufPtr += idx + 1;
            continue;
         }

         if(!dStrnicmp(bufPtr + 1, "color:", 6))
         {
            idx = 7;
            if(!scanforchar((char*)bufPtr, idx, '>'))
               goto textemit;
            if(idx != 13)
               goto textemit;

            bufPtr += 14;
            continue;
         }

         if(!dStrnicmp(bufPtr +1, "bitmap:", 7))
         {
            idx = 8;
            if(!scanforchar((char*)bufPtr, idx, '>'))
               goto textemit;

            bufPtr += idx + 1;
            continue;
         }

         if(!dStrnicmp(bufPtr +1, "spush>", 6))
         {
            bufPtr += 7;
            continue;
         }

         if(!dStrnicmp(bufPtr +1, "spop>", 5))
         {
            bufPtr += 6;
            continue;
         }

         if(!dStrnicmp(bufPtr +1, "sbreak>", 7))
         {
            bufPtr += 8;
            continue;
         }

         if(!dStrnicmp(bufPtr +1, "just:left>", 10))
         {
            bufPtr += 11;
            continue;
         }

         if(!dStrnicmp(bufPtr +1, "just:right>", 11))
         {
            bufPtr += 12;
            continue;
         }

         if(!dStrnicmp(bufPtr +1, "just:center>", 12))
         {
            bufPtr += 13;
            continue;
         }

         if(!dStrnicmp(bufPtr +1, "a:", 2))
         {
            idx = 3;
            if(!scanforchar((char*)bufPtr, idx, '>'))
               goto textemit;

            bufPtr += idx + 1;
            continue;
         }

         if(!dStrnicmp(bufPtr+1, "/a>", 3))
         {
            bufPtr += 4;
            continue;
         }

         if(!dStrnicmp(bufPtr + 1, "lmargin%:", 9))
         {
            idx = 10;
            if(!scanforchar((char*)bufPtr, idx, '>'))
               goto textemit;
            bufPtr += idx + 1;
            goto setleftmargin;
         }

         if(!dStrnicmp(bufPtr + 1, "lmargin:", 8))
         {
            idx = 9;
            if(!scanforchar((char*)bufPtr, idx, '>'))
               goto textemit;
            bufPtr += idx + 1;
setleftmargin:
            continue;
         }

         if(!dStrnicmp(bufPtr + 1, "rmargin%:", 9))
         {
            idx = 10;
            if(!scanforchar((char*)bufPtr, idx, '>'))
               goto textemit;
            bufPtr += idx + 1;
            goto setrightmargin;
         }

         if(!dStrnicmp(bufPtr + 1, "rmargin:", 8))
         {
            idx = 9;
            if(!scanforchar((char*)bufPtr, idx, '>'))
               goto textemit;
            bufPtr += idx + 1;
setrightmargin:
            continue;
         }

         if(!dStrnicmp(bufPtr + 1, "clip:", 5))
         {
            idx = 6;
            if(!scanforchar((char*)bufPtr, idx, '>'))
               goto textemit;
            bufPtr += idx + 1;
            continue;
         }

         if(!dStrnicmp(bufPtr + 1, "/clip>", 6))
         {
            bufPtr += 7;
            continue;
         }

         if(!dStrnicmp(bufPtr + 1, "div:", 4))
         {
            idx = 5;
            if(!scanforchar((char*)bufPtr, idx, '>'))
               goto textemit;
            bufPtr += idx + 1;
            continue;
         }

         if(!dStrnicmp(bufPtr + 1, "tab:", 4))
         {
            idx = 5;
            if(!scanforchar((char*)bufPtr, idx, '>'))
               goto textemit;
            bufPtr += idx + 1;
            continue;
         }
      }

      // default case:
textemit:
      *stripBufPtr++ = *bufPtr++;
      while(*bufPtr != '\t' && *bufPtr != '<' && *bufPtr != '\n' && (*bufPtr >= 0x20 || *bufPtr < 0))
         *stripBufPtr++ = *bufPtr++;
   }

   //we're finished - terminate the string
   *stripBufPtr = '\0';
   return strippedBuffer;
}

//--------------------------------------------------------------------------

bool GuiMLTextCtrl::resize( const Point2I& newPosition, const Point2I& newExtent )
{
   if( Parent::resize( newPosition, newExtent ) )
   {
      mDirty = true;
      return true;
   }
   
   return false;
}
