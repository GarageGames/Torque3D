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
#include "gui/game/guiMessageVectorCtrl.h"

#include "gui/utility/messageVector.h"
#include "console/consoleTypes.h"
#include "gui/containers/guiScrollCtrl.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiMessageVectorCtrl);

ConsoleDocClass( GuiMessageVectorCtrl,
	"@brief A chat HUD control that displays messages from a MessageVector.\n\n"

	"This renders messages from a MessageVector; the important thing "
	"here is that the MessageVector holds all the messages we care "
	"about, while we can destroy and create these GUI controls as "
	"needed.\n\n"

	"@tsexample\n"
	"// Declare ChatHud, which is what will display the actual chat from a MessageVector\n"
	"new GuiMessageVectorCtrl(ChatHud) {\n"
    "   profile = \"ChatHudMessageProfile\";\n"
    "   horizSizing = \"width\";\n"
    "   vertSizing = \"height\";\n"
    "   position = \"1 1\";\n"
    "   extent = \"252 16\";\n"
    "   minExtent = \"8 8\";\n"
    "   visible = \"1\";\n"
    "   helpTag = \"0\";\n"
    "   lineSpacing = \"0\";\n"
    "   lineContinuedIndex = \"10\";\n"
    "   matchColor = \"0 0 255 255\";\n"
    "   maxColorIndex = \"5\";\n"
    "};\n\n"
	"// All messages are stored in this HudMessageVector, the actual\n"
	"// MainChatHud only displays the contents of this vector.\n"
	"new MessageVector(HudMessageVector);\n\n"
	"// Attach the MessageVector to the chat control\n"
	"chatHud.attach(HudMessageVector);\n"
	"@endtsexample\n\n"

	"@see MessageVector for more details on how this is used\n"

	"@ingroup GuiUtil\n");


//-------------------------------------- Console functions
DefineEngineMethod( GuiMessageVectorCtrl, attach, bool, ( MessageVector* item),,
	   "@brief Push a line onto the back of the list.\n\n"

	   "@param item The GUI element being pushed into the control\n\n"

	   "@tsexample\n"
	   "// All messages are stored in this HudMessageVector, the actual\n"
	   "// MainChatHud only displays the contents of this vector.\n"
	   "new MessageVector(HudMessageVector);\n\n"
	   "// Attach the MessageVector to the chat control\n"
	   "chatHud.attach(HudMessageVector);\n"
	   "@endtsexample\n\n"

	   "@return Value")
{
	if (item == NULL)
	{
		Con::errorf(ConsoleLogEntry::General, "Could not find MessageVector: %s", item);
		return false;
	}
	
	return object->attach(item);
}

//ConsoleMethod(GuiMessageVectorCtrl, attach, bool, 3, 3, "(MessageVector item)"
//              "Make this gui control display messages from the specified MessageVector")
//{
//   MessageVector* pMV = NULL;
//   Sim::findObject(argv[2], pMV);
//   if (pMV == NULL) {
//      Con::errorf(ConsoleLogEntry::General, "Could not find MessageVector: %s", argv[2]);
//      return false;
//   }
//
//   return object->attach(pMV);
//}

DefineEngineMethod( GuiMessageVectorCtrl, detach, void, (),,
				   "@brief Stop listing messages from the MessageVector previously attached to, if any.\n\n"

				   "Detailed description\n\n"

				   "@param param Description\n\n"

				   "@tsexample\n"
				   "// Deatch the MessageVector from HudMessageVector\n"
				   "// HudMessageVector will no longer render the text\n"
				   "chatHud.detach();\n"
				   "@endtsexample\n\n")
{
	if (object->isAttached() == false)
	{
		Con::warnf(ConsoleLogEntry::General, "GuiMessageVectorCtrl: double detach");
		return;
	}

	object->detach();
}

//ConsoleMethod(GuiMessageVectorCtrl, detach, void, 2, 2, "()"
//              "Stop listing messages from the MessageVector previously attached to, if any.")
//{
//   if (object->isAttached() == false) {
//      Con::warnf(ConsoleLogEntry::General, "GuiMessageVectorCtrl: double detach");
//      return;
//   }
//
//   object->detach();
//}

struct TempLineBreak
{
   S32 start;
   S32 end;
};

//--------------------------------------------------------------------------
// Callback for messageVector
void sMVCtrlCallback(void *                           spectatorKey,
                     const MessageVector::MessageCode code,
                     const U32                        argument)
{
   GuiMessageVectorCtrl* pMVC = reinterpret_cast<GuiMessageVectorCtrl*>(spectatorKey);
   pMVC->callbackRouter(code, argument);
}


//--------------------------------------------------------------------------
GuiMessageVectorCtrl::GuiMessageVectorCtrl()
{
   VECTOR_SET_ASSOCIATION(mLineWrappings);
   VECTOR_SET_ASSOCIATION(mSpecialMarkers);
   VECTOR_SET_ASSOCIATION(mLineElements);

   mMessageVector = NULL;
   mLineSpacingPixels = 0;
   mLineContinuationIndent = 10;

   mMouseDown      = false;
   mMouseSpecialLine = -1;
   mMouseSpecialRef  = -1;

   for (U32 i = 0; i < 16; i++)
      mAllowedMatches[i] = "";
   mSpecialColor.set(0, 0, 255);

   mMaxColorIndex = 9;
}


//--------------------------------------------------------------------------
GuiMessageVectorCtrl::~GuiMessageVectorCtrl()
{
   AssertFatal(mLineWrappings.size() == 0, "Error, line wrappings not properly cleared!");
   AssertFatal(mSpecialMarkers.size() == 0, "Error, special markers not properly cleared!");
   AssertFatal(mLineElements.size() == 0, "Error, line elements not properly cleared!");
}


//--------------------------------------------------------------------------
void GuiMessageVectorCtrl::initPersistFields()
{
   addField("lineSpacing",        TypeS32,    Offset(mLineSpacingPixels,      GuiMessageVectorCtrl));
   addField("lineContinuedIndex", TypeS32,    Offset(mLineContinuationIndent, GuiMessageVectorCtrl));
   addField("allowedMatches",     TypeString, Offset(mAllowedMatches,         GuiMessageVectorCtrl), 16);
   addField("matchColor",         TypeColorI, Offset(mSpecialColor,           GuiMessageVectorCtrl));
   addField("maxColorIndex",      TypeS32,    Offset(mMaxColorIndex,          GuiMessageVectorCtrl));
   Parent::initPersistFields();
}


bool GuiMessageVectorCtrl::onAdd()
{
   return Parent::onAdd();
}


void GuiMessageVectorCtrl::onRemove()
{
   Parent::onRemove();
}


//--------------------------------------------------------------------------
bool GuiMessageVectorCtrl::isAttached() const
{
   return (mMessageVector != NULL);
}


//--------------------------------------------------------------------------
bool GuiMessageVectorCtrl::attach(MessageVector* newAttachment)
{
   AssertFatal(newAttachment, "No attachment!");
   if (newAttachment == NULL || !isAwake())
      return false;

   if (isAttached()) {
      Con::warnf(ConsoleLogEntry::General, "GuiMessageVectorCtrl::attach: overriding attachment");
      detach();
   }
   AssertFatal(mLineWrappings.size() == 0, "Error, line wrappings not properly cleared!");

   mMessageVector = newAttachment;
   mMessageVector->registerSpectator(sMVCtrlCallback, this);

   return true;
}

//--------------------------------------------------------------------------
void GuiMessageVectorCtrl::detach()
{
   if (isAttached() == false) {
      Con::warnf(ConsoleLogEntry::General, "GuiMessageVectorCtrl::detach: not attached!");
      return;
   }

   mMessageVector->unregisterSpectator(this);
   mMessageVector = NULL;
   AssertFatal(mLineWrappings.size() == 0, "Error, line wrappings not properly cleared!");
}


//--------------------------------------------------------------------------
void GuiMessageVectorCtrl::lineInserted(const U32 arg)
{
   AssertFatal(mMessageVector != NULL, "Should not be here unless we're attached!");

   GuiScrollCtrl* pScroll = dynamic_cast<GuiScrollCtrl*>(getParent());
   bool fullyScrolled = pScroll->isScrolledToBottom();

   mSpecialMarkers.insert(arg);
   createSpecialMarkers(mSpecialMarkers[arg], mMessageVector->getLine(arg).message);

   mLineWrappings.insert(arg);
   createLineWrapping(mLineWrappings[arg], mMessageVector->getLine(arg).message);

   mLineElements.insert(arg);
   createLineElement(mLineElements[arg], mLineWrappings[arg], mSpecialMarkers[arg]);

   U32 numLines = 0;
   for (U32 i = 0; i < mLineWrappings.size(); i++) {
      // We need to rebuild the physicalLineStart markers at the same time as
      //  we find out how many of them are left...
      mLineElements[i].physicalLineStart = numLines;

      numLines += mLineWrappings[i].numLines;
   }

   Point2I newExtent = getExtent();
   newExtent.y = (mProfile->mFont->getHeight() + mLineSpacingPixels) * getMax(numLines, U32(1));
   setExtent(newExtent);
   if(fullyScrolled)
      pScroll->scrollTo(0, 0x7FFFFFFF);
}


void GuiMessageVectorCtrl::lineDeleted(const U32 arg)
{
   AssertFatal(mMessageVector != NULL, "Should not be here unless we're attached!");
   AssertFatal(arg < mLineWrappings.size(), "Error, out of bounds line deleted!");

   // It's a somewhat involved process to delete the lineelements...
   LineElement& rElement = mLineElements[arg];

   TextElement* walk = rElement.headLineElements;
   while (walk != NULL) {
      TextElement* lineWalk = walk->nextInLine;
      while (lineWalk != NULL) {
         TextElement* temp = lineWalk;
         lineWalk = lineWalk->nextPhysicalLine;
         delete temp;
      }

      TextElement* temp = walk;
      walk = walk->nextPhysicalLine;
      delete temp;
   }
   rElement.headLineElements = NULL;
   mLineElements.erase(arg);

   delete [] mLineWrappings[arg].startEndPairs;
   mLineWrappings.erase(arg);

   delete [] mSpecialMarkers[arg].specials;
   mSpecialMarkers.erase(arg);

   U32 numLines = 0;
   for (U32 i = 0; i < mLineWrappings.size(); i++) {
      // We need to rebuild the physicalLineStart markers at the same time as
      //  we find out how many of them are left...
      mLineElements[i].physicalLineStart = numLines;

      numLines += mLineWrappings[i].numLines;
   }

   U32 newHeight = (mProfile->mFont->getHeight() + mLineSpacingPixels) * getMax(numLines, U32(1));
   resize(getPosition(), Point2I(getWidth(), newHeight));
}


void GuiMessageVectorCtrl::vectorDeleted()
{
   AssertFatal(mMessageVector != NULL, "Should not be here unless we're attached!");
   AssertFatal(mLineWrappings.size() == 0, "Error, line wrappings not properly cleared out!");

   mMessageVector = NULL;
   U32 newHeight = mProfile->mFont->getHeight() + mLineSpacingPixels;
   resize(getPosition(), Point2I(getWidth(), newHeight));
}


void GuiMessageVectorCtrl::callbackRouter(const MessageVector::MessageCode code,
                                          const U32                        arg)
{
   switch (code) {
      case MessageVector::LineInserted:
         lineInserted(arg);
         break;
      case MessageVector::LineDeleted:
         lineDeleted(arg);
         break;
      case MessageVector::VectorDeletion:
         vectorDeleted();
         break;
   }
}


//--------------------------------------------------------------------------
void GuiMessageVectorCtrl::createSpecialMarkers(SpecialMarkers& rSpecial, const char* string)
{
   // The first thing we need to do is create a version of the string with no uppercase
   //  chars for matching...

   String pLCCopyStr = String::ToLower( string );
   const char* pLCCopy = pLCCopyStr.c_str();

   Vector<TempLineBreak> tempSpecials(__FILE__, __LINE__);
   Vector<S32>           tempTypes(__FILE__, __LINE__);

   const char* pCurr = pLCCopy;
   while (pCurr[0] != '\0') {
      const char* pMinMatch = &pLCCopy[dStrlen(string)];
      U32 minMatchType = 0xFFFFFFFF;
      AssertFatal(pMinMatch[0] == '\0', "Error, bad positioning of sentry...");

      // Find the earliest match
      for (U32 i = 0; i < 16; i++) {
         if (mAllowedMatches[i][0] == '\0')
            continue;

         const char* pMatch = dStrstr(pCurr, mAllowedMatches[i]);
         if (pMatch != NULL && pMatch < pMinMatch) {
            pMinMatch = pMatch;
            minMatchType = i;
         }
      }

      if (pMinMatch[0] != '\0') {
         AssertFatal(minMatchType != 0xFFFFFFFF, "Hm, that's bad");
         // Found a match => now find the end
         U32 start = pMinMatch - pLCCopy;
         U32 j;
         for (j = 1; pLCCopy[start + j] != '\0'; j++) {
            if (pLCCopy[start + j] == '\n' ||
                pLCCopy[start + j] == ' '  ||
                pLCCopy[start + j] == '\t')
               break;
         }
         AssertFatal(j > 0, "Error, j must be > 0 at this point!");
         U32 end = start + j - 1;

         tempSpecials.increment();
         tempSpecials.last().start = start;
         tempSpecials.last().end   = end;
         tempTypes.push_back(minMatchType);

         pCurr  = &pLCCopy[end + 1];
      } else {
         // No match.  This will cause the while loop to terminate...
         pCurr = pMinMatch;
      }
   }

   if ((rSpecial.numSpecials = tempSpecials.size()) != 0) {
      rSpecial.specials = new SpecialMarkers::Special[tempSpecials.size()];
      for (U32 i = 0; i < tempSpecials.size(); i++) {
         rSpecial.specials[i].start       = tempSpecials[i].start;
         rSpecial.specials[i].end         = tempSpecials[i].end;
         rSpecial.specials[i].specialType = tempTypes[i];
      }
   } else {
      rSpecial.specials = NULL;
   }
}


//--------------------------------------------------------------------------
void GuiMessageVectorCtrl::createLineWrapping(LineWrapping& rWrapping, const char* string)
{
   Vector<TempLineBreak> tempBreaks(__FILE__, __LINE__);

   U32 i;
   U32 currStart = 0;
   U32 length = dStrlen(string);
   if (length != 0) {
      for (i = 0; i < length; i++) {
         if (string[i] == '\n') {
            tempBreaks.increment();
            tempBreaks.last().start = currStart;
            tempBreaks.last().end   = i-1;
            currStart = i+1;
         } else if (i == length - 1) {
            tempBreaks.increment();
            tempBreaks.last().start = currStart;
            tempBreaks.last().end   = i;
            currStart = i+1;
         }
      }
   } else {
      tempBreaks.increment();
      tempBreaks.last().start = 0;
      tempBreaks.last().end   = -1;
   }

   U32 splitWidth = getWidth();
   U32 currLine = 0;
   while (currLine < tempBreaks.size()) {
      TempLineBreak& rLine = tempBreaks[currLine];
      if (rLine.start >= rLine.end) {
         if (currLine == 0)
            splitWidth -= mLineContinuationIndent;
         currLine++;
         continue;
      }

      // Ok, there's some actual text in this line.  How long is it?
      U32 baseLength = mProfile->mFont->getStrNWidthPrecise((const UTF8 *)&string[rLine.start], rLine.end-rLine.start+1);
      if (baseLength > splitWidth) {
         // DMMNOTE: Replace with bin search eventually
         U32 currPos = 0;
         U32 breakPos = 0;
         for (currPos = 0; currPos < rLine.end-rLine.start+1; currPos++) {
            U32 currLength = mProfile->mFont->getStrNWidthPrecise((const UTF8 *)&string[rLine.start], currPos+1);
            if (currLength > splitWidth) 
            {
               // Make sure that the currPos has advanced, then set the breakPoint.
               breakPos = currPos != 0 ? currPos - 1 : 0;
               break;
            }
         }
         if (currPos == rLine.end-rLine.start+1) {
            AssertFatal(false, "Error, if the line must be broken, the position must be before this point!");
            currLine++;
            continue;
         }

         // Ok, the character at breakPos is the last valid char we can render.  We
         //  want to scan back to the first whitespace character (which, in the bounds
         //  of the line, is guaranteed to be a space or a tab).
         U32 originalBreak = breakPos;
         while (true) {
            if (string[rLine.start + breakPos] == ' ' || string[rLine.start + breakPos] == '\t') {
               break;
            } else {
               AssertFatal(string[rLine.start + breakPos] != '\n',
                           "Bad characters in line range...");
               if (breakPos == 0) {
                  breakPos = originalBreak;
                  break;
               }
               breakPos--;
            }
         }

         // Ok, everything up to and including breakPos is in the currentLine.  Insert
         //  a new line at this point, and put everything after in that line.
         S32 oldStart = rLine.start;
         S32 oldEnd   = rLine.end;
         rLine.end    = rLine.start + breakPos;

         // Note that rLine is NOTNOTNOTNOT valid after this point!
         tempBreaks.insert(currLine+1);
         tempBreaks[currLine+1].start = oldStart + breakPos + 1;
         tempBreaks[currLine+1].end   = oldEnd;
      }

      if (currLine == 0)
         splitWidth -= mLineContinuationIndent;
      currLine++;
   }

   rWrapping.numLines = tempBreaks.size();
   rWrapping.startEndPairs = new LineWrapping::SEPair[tempBreaks.size()];
   for (i = 0; i < tempBreaks.size(); i++) {
      rWrapping.startEndPairs[i].start = tempBreaks[i].start;
      rWrapping.startEndPairs[i].end   = tempBreaks[i].end;
   }
}

//--------------------------------------------------------------------------
void GuiMessageVectorCtrl::createLineElement(LineElement&    rElement,
                                             LineWrapping&   rWrapping,
                                             SpecialMarkers& rSpecial)
{
   // First, do a straighforward translation of the wrapping...
   TextElement** ppWalk = &rElement.headLineElements;
   for (U32 i = 0; i < rWrapping.numLines; i++) {
      *ppWalk = new TextElement;

      (*ppWalk)->nextInLine       = NULL;
      (*ppWalk)->nextPhysicalLine = NULL;
      (*ppWalk)->specialReference = -1;

      (*ppWalk)->start = rWrapping.startEndPairs[i].start;
      (*ppWalk)->end   = rWrapping.startEndPairs[i].end;

      ppWalk = &((*ppWalk)->nextPhysicalLine);
   }

   if (rSpecial.numSpecials != 0) {
      // Ok.  Now, walk down the lines, and split the elements into their constituent parts...
      //
      TextElement* walk = rElement.headLineElements;
      while (walk) {
         TextElement* walkAcross = walk;
         while (walkAcross) {
            S32 specialMatch = -1;
            for (U32 i = 0; i < rSpecial.numSpecials; i++) {
               if (walkAcross->start <= rSpecial.specials[i].end &&
                   walkAcross->end   >= rSpecial.specials[i].start) {
                  specialMatch = i;
                  break;
               }
            }

            if (specialMatch != -1) {
               // We have a match here.  Break it into the appropriate number of segments.
               //  this will vary depending on how the overlap is occuring...
               if (walkAcross->start >= rSpecial.specials[specialMatch].start) {
                  if (walkAcross->end <= rSpecial.specials[specialMatch].end) {
                     // The whole thing is part of the special
                     AssertFatal(walkAcross->nextInLine == NULL, "Bad assumption!");
                     walkAcross->specialReference = specialMatch;
                  } else {
                     // The first part is in the special, the tail is out
                     AssertFatal(walkAcross->nextInLine == NULL, "Bad assumption!");
                     walkAcross->nextInLine = new TextElement;
                     walkAcross->nextInLine->nextInLine       = NULL;
                     walkAcross->nextInLine->nextPhysicalLine = NULL;
                     walkAcross->nextInLine->specialReference = -1;

                     walkAcross->specialReference  = specialMatch;
                     walkAcross->nextInLine->end   = walkAcross->end;
                     walkAcross->end               = rSpecial.specials[specialMatch].end;
                     walkAcross->nextInLine->start = rSpecial.specials[specialMatch].end + 1;

                     AssertFatal(walkAcross->end >= walkAcross->start && walkAcross->nextInLine->end >= walkAcross->nextInLine->start, "Bad textelements generated!");
                  }

                  walkAcross = walkAcross->nextInLine;
               } else {
                  if (walkAcross->end <= rSpecial.specials[specialMatch].end) {
                     // The first part is out of the special, the second part in.
                     AssertFatal(walkAcross->nextInLine == NULL, "Bad assumption!");
                     walkAcross->nextInLine = new TextElement;
                     walkAcross->nextInLine->nextInLine       = NULL;
                     walkAcross->nextInLine->nextPhysicalLine = NULL;
                     walkAcross->nextInLine->specialReference = specialMatch;

                     walkAcross->specialReference  = -1;
                     walkAcross->nextInLine->end   = walkAcross->end;
                     walkAcross->end               = rSpecial.specials[specialMatch].start - 1;
                     walkAcross->nextInLine->start = rSpecial.specials[specialMatch].start;

                     AssertFatal(walkAcross->end >= walkAcross->start && walkAcross->nextInLine->end >= walkAcross->nextInLine->start, "Bad textelements generated!");
                     walkAcross = walkAcross->nextInLine;
                  } else {
                     // First out, middle in, last out.  Oy.
                     AssertFatal(walkAcross->nextInLine == NULL, "Bad assumption!");
                     walkAcross->nextInLine = new TextElement;
                     walkAcross->nextInLine->nextInLine       = NULL;
                     walkAcross->nextInLine->nextPhysicalLine = NULL;
                     walkAcross->nextInLine->specialReference = specialMatch;

                     walkAcross->nextInLine->nextInLine                   = new TextElement;
                     walkAcross->nextInLine->nextInLine->nextInLine       = NULL;
                     walkAcross->nextInLine->nextInLine->nextPhysicalLine = NULL;
                     walkAcross->nextInLine->nextInLine->specialReference = -1;

                     walkAcross->nextInLine->start = rSpecial.specials[specialMatch].start;
                     walkAcross->nextInLine->end   = rSpecial.specials[specialMatch].end;
                     walkAcross->nextInLine->nextInLine->start = rSpecial.specials[specialMatch].end+1;
                     walkAcross->nextInLine->nextInLine->end   = walkAcross->end;
                     walkAcross->end = walkAcross->nextInLine->start - 1;
                     AssertFatal((walkAcross->end >= walkAcross->start &&
                                  walkAcross->nextInLine->end >= walkAcross->nextInLine->start &&
                                  walkAcross->nextInLine->nextInLine->end >= walkAcross->nextInLine->nextInLine->start),
                                 "Bad textelements generated!");
                     walkAcross = walkAcross->nextInLine->nextInLine;
                  }
               }
            } else {
               walkAcross = walkAcross->nextInLine;
            }
         }

         walk = walk->nextPhysicalLine;
      }
   }
}


//--------------------------------------------------------------------------
bool GuiMessageVectorCtrl::onWake()
{
   if (Parent::onWake() == false)
      return false;

   if (bool(mProfile->mFont) == false)
      return false;

   mMinSensibleWidth = 1;

   for (U32 i = 0; i < 256; i++) {
      if (mProfile->mFont->isValidChar(U8(i))) {
         if (mProfile->mFont->getCharWidth(U8(i)) > mMinSensibleWidth)
            mMinSensibleWidth = mProfile->mFont->getCharWidth(U8(i));
      }
   }

   AssertFatal(mLineWrappings.size() == 0, "Error, line wrappings not properly cleared!");
   return true;
}


//--------------------------------------------------------------------------
void GuiMessageVectorCtrl::onSleep()
{
   if (isAttached())
      detach();

   Parent::onSleep();
}


//--------------------------------------------------------------------------
void GuiMessageVectorCtrl::onRender(Point2I      offset,
                                    const RectI& updateRect)
{
   GFXDrawUtil *drawer = GFX->getDrawUtil();

   Parent::onRender(offset, updateRect);
   if (isAttached()) {
      U32 linePixels = mProfile->mFont->getHeight() + mLineSpacingPixels;
      U32 currLine   = 0;
      ColorI lastColor = mProfile->mFontColor;
      for (U32 i = 0; i < mMessageVector->getNumLines(); i++) {

         TextElement* pElement = mLineElements[i].headLineElements;
         while (pElement != NULL) {
            Point2I localStart(pElement == mLineElements[i].headLineElements ? 0 : mLineContinuationIndent, currLine * linePixels);

            Point2I globalCheck  = localToGlobalCoord(localStart);
            U32 globalRangeStart = globalCheck.y;
            U32 globalRangeEnd   = globalCheck.y + mProfile->mFont->getHeight();
            if (globalRangeStart > updateRect.point.y + updateRect.extent.y ||
                globalRangeEnd   < updateRect.point.y) {
               currLine++;
               pElement = pElement->nextPhysicalLine;
               continue;
            }

            TextElement* walkAcross = pElement;
            while (walkAcross) {
               if (walkAcross->start > walkAcross->end)
                  break;

               Point2I globalStart  = localToGlobalCoord(localStart);

               U32 strWidth;
               if (walkAcross->specialReference == -1) {
                  drawer->setBitmapModulation(lastColor);
                  drawer->setTextAnchorColor(mProfile->mFontColor);
                  strWidth = drawer->drawTextN(mProfile->mFont, globalStart, &mMessageVector->getLine(i).message[walkAcross->start],
                                          walkAcross->end - walkAcross->start + 1, mProfile->mFontColors, mMaxColorIndex);
                  drawer->getBitmapModulation(&lastColor);	// in case an embedded color tag changed it
               } else {
                  drawer->getBitmapModulation( &lastColor );
                  drawer->setBitmapModulation(mSpecialColor);
                  drawer->setTextAnchorColor(mProfile->mFontColor);
                  strWidth = drawer->drawTextN(mProfile->mFont, globalStart, &mMessageVector->getLine(i).message[walkAcross->start],
                                          walkAcross->end - walkAcross->start + 1);

                  // in case we have 2 in a row...
                  drawer->setBitmapModulation(lastColor);
               }

               // drawTextN returns the rightmost X coord, so subtract leftmost coord to get the width
               strWidth -= globalStart.x;

               if (walkAcross->specialReference != -1) {
                  Point2I lineStart = localStart;
                  Point2I lineEnd   = localStart;
                  lineStart.y += mProfile->mFont->getBaseline() + 1;
                  lineEnd.x += strWidth;
                  lineEnd.y += mProfile->mFont->getBaseline() + 1;

                  drawer->drawLine(localToGlobalCoord(lineStart),
                              localToGlobalCoord(lineEnd),
                              mSpecialColor);
               }

               localStart.x += strWidth;
               walkAcross = walkAcross->nextInLine;
            }

            currLine++;
            pElement = pElement->nextPhysicalLine;
         }
      }
      drawer->clearBitmapModulation();
   }
}


//--------------------------------------------------------------------------
void GuiMessageVectorCtrl::inspectPostApply()
{
   Parent::inspectPostApply();
}


//--------------------------------------------------------------------------
void GuiMessageVectorCtrl::parentResized(const RectI& oldParentRect, const RectI& newParentRect)
{
   Parent::parentResized(oldParentRect, newParentRect);

   // If we have a MesssageVector, detach/reattach so we can reflow the text.
   if (mMessageVector)
   {
      MessageVector *reflowme = mMessageVector;

      detach();
      attach(reflowme);
   }
}

//--------------------------------------------------------------------------
void GuiMessageVectorCtrl::findSpecialFromCoord(const Point2I& point, S32* specialLine, S32* specialRef)
{
   if (mLineElements.size() == 0) {
      *specialLine = -1;
      *specialRef  = -1;
      return;
   }

   U32 linePixels = mProfile->mFont->getHeight() + mLineSpacingPixels;
   
   if ((point.x < 0 || point.x >= getWidth()) ||
       (point.y < 0 || point.y >= getHeight())) {
      *specialLine = -1;
      *specialRef  = -1;
      return;
   }

   // Ok, we have real work to do here.  Let's determine the physical line that it's on...
   U32 physLine = point.y / linePixels;
   AssertFatal(physLine >= 0, "Bad physical line!");

   // And now we find the LineElement that contains that physicalLine...
   U32 elemIndex;
   for (elemIndex = 0; elemIndex < mLineElements.size(); elemIndex++) {
      if (mLineElements[elemIndex].physicalLineStart > physLine) {
         // We've passed it.
         AssertFatal(elemIndex != 0, "Error, bad elemIndex, check assumptions.");
         elemIndex--;
         break;
      }
   }
   if (elemIndex == mLineElements.size()) {
      // On the last line...
      elemIndex = mLineElements.size() - 1;
   }

   TextElement* line = mLineElements[elemIndex].headLineElements;
   for (U32 i = 0; i < physLine - mLineElements[elemIndex].physicalLineStart; i++)
   {
      if (line->nextPhysicalLine == NULL)
      {
         *specialLine = -1;
         *specialRef  = -1;
         return;
      }
      line = line->nextPhysicalLine;
      AssertFatal(line != NULL, "Error, moved too far!");
   }

   // Ok, line represents the current line.  We now need to find out which textelement
   //  the points x coord falls in.
   U32 currX = 0;
   if ((physLine - mLineElements[elemIndex].physicalLineStart) != 0) {
      currX = mLineContinuationIndent;
      // First, if this isn't the first line in this wrapping, we have to make sure
      //  that the coord isn't in the margin...
      if (point.x < mLineContinuationIndent) {
         *specialLine = -1;
         *specialRef  = -1;
         return;
      }
   }
   if (line->start > line->end) {
      // Empty line special case...
      *specialLine = -1;
      *specialRef  = -1;
      return;
   }

   while (line) {
      U32 newX = currX + mProfile->mFont->getStrNWidth((const UTF8 *)mMessageVector->getLine(elemIndex).message,
                                                       line->end - line->start + 1);
      if (point.x < newX) {
         // That's the one!
         *specialLine = elemIndex;
         *specialRef  = line->specialReference;
         return;
      }

      currX = newX;
      line = line->nextInLine;
   }

   // Off to the right.  Aw...
   *specialLine = -1;
   *specialRef  = -1;
}


void GuiMessageVectorCtrl::onMouseDown(const GuiEvent& event)
{
   Parent::onMouseDown(event);
   mouseUnlock();

   mMouseDown = true;

   // Find the special we are in, if any...
   findSpecialFromCoord(globalToLocalCoord(event.mousePoint),
                        &mMouseSpecialLine, &mMouseSpecialRef);
}

void GuiMessageVectorCtrl::onMouseUp(const GuiEvent& event)
{
   Parent::onMouseUp(event);
   mouseUnlock();

   // Is this an up from a dragged click?
   if (mMouseDown == false)
      return;

   // Find the special we are in, if any...

   S32 currSpecialLine;
   S32 currSpecialRef;
   findSpecialFromCoord(globalToLocalCoord(event.mousePoint), &currSpecialLine, &currSpecialRef);

   if (currSpecialRef != -1 &&
       (currSpecialLine == mMouseSpecialLine &&
        currSpecialRef  == mMouseSpecialRef)) {
      // Execute the callback
      SpecialMarkers& rSpecial = mSpecialMarkers[currSpecialLine];
      S32 specialStart = rSpecial.specials[currSpecialRef].start;
      S32 specialEnd   = rSpecial.specials[currSpecialRef].end;

      char* copyURL = new char[specialEnd - specialStart + 2];
      dStrncpy(copyURL, &mMessageVector->getLine(currSpecialLine).message[specialStart], specialEnd - specialStart + 1);
      copyURL[specialEnd - specialStart + 1] = '\0';

      Con::executef(this, "urlClickCallback", copyURL);
      delete [] copyURL;
   }

   mMouseDown      = false;
   mMouseSpecialLine = -1;
   mMouseSpecialRef  = -1;
}

