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

#include "guiGameListOptionsCtrl.h"
#include "gfx/gfxDrawUtil.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "core/strings/stringUnit.h"

//-----------------------------------------------------------------------------
// GuiGameListOptionsCtrl
//-----------------------------------------------------------------------------

GuiGameListOptionsCtrl::GuiGameListOptionsCtrl()
{
}

GuiGameListOptionsCtrl::~GuiGameListOptionsCtrl()
{
}

bool GuiGameListOptionsCtrl::onAdd()
{
   if( !Parent::onAdd() )
      return false;
      
   if( !hasValidProfile() )
   {
      GuiGameListOptionsProfile* profile;
      if( !Sim::findObject( "DefaultOptionsMenuProfile", profile ) )
      {
         Con::errorf( "GuiGameListOptionsCtrl: %s can't be created with a profile of type %s. Please create it with a profile of type GuiGameListOptionsProfile.",
            getName(), mProfile->getClassName() );
         return false;
      }
      else
         Con::warnf( "GuiGameListOptionsCtrl: substituted non-GuiGameListOptionsProfile in %s for DefaultOptionsMenuProfile", getName() );
         
      setControlProfile( profile );
   }
      
   return true;
}

void GuiGameListOptionsCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   Parent::onRender(offset, updateRect);
   GuiGameListOptionsProfile * profile = (GuiGameListOptionsProfile *) mProfile;

   F32 xScale = (float) getWidth() / profile->getRowWidth();

   S32 rowHeight = profile->getRowHeight();

   bool profileHasArrows = profile->hasArrows();
   Point2I arrowExtent;
   S32 arrowOffsetY(0);
   if (profileHasArrows)
   {
      arrowExtent = profile->getArrowExtent();

      // icon is centered vertically
      arrowOffsetY = (rowHeight - arrowExtent.y) >> 1;
   }

   GFXDrawUtil *drawer = GFX->getDrawUtil();

   Point2I currentOffset = offset;
   Point2I arrowOffset;
   S32 columnSplit = profile->mColumnSplit * xScale;
   S32 iconIndex;
   for (Vector<Parent::Row *>::iterator row = mRows.begin(); row < mRows.end(); ++row)
   {
      Row * myRow = (Row *) *row;
      if (row != mRows.begin())
      {
         // rows other than the first can have padding above them
         currentOffset.y += myRow->mHeightPad;
         currentOffset.y += rowHeight;
      }

      bool hasOptions = (myRow->mOptions.size() > 0) && myRow->mSelectedOption > -1;
      if (hasOptions)
      {
         bool isRowSelected = (getSelected() != NO_ROW) && (row == &mRows[getSelected()]);
         bool isRowHighlighted = (getHighlighted() != NO_ROW) ? ((row == &mRows[getHighlighted()]) && ((*row)->mEnabled)) : false;
         if (profileHasArrows)
         {
            // render the left arrow
            bool arrowOnL = (isRowSelected || isRowHighlighted) && (myRow->mWrapOptions || (myRow->mSelectedOption > 0));
            iconIndex = (arrowOnL) ? Profile::TEX_L_ARROW_ON : Profile::TEX_L_ARROW_OFF;
            arrowOffset.x = currentOffset.x + columnSplit;
            arrowOffset.y = currentOffset.y + arrowOffsetY;

            drawer->clearBitmapModulation();
            drawer->drawBitmapStretchSR(profile->mTextureObject, RectI(arrowOffset, arrowExtent), profile->getBitmapArrayRect((U32)iconIndex));

            // render the right arrow
            bool arrowOnR = (isRowSelected || isRowHighlighted) && (myRow->mWrapOptions || (myRow->mSelectedOption < myRow->mOptions.size() - 1));
            iconIndex = (arrowOnR) ? Profile::TEX_R_ARROW_ON : Profile::TEX_R_ARROW_OFF;
            arrowOffset.x = currentOffset.x + (profile->mHitAreaLowerRight.x - profile->mRightPad) * xScale - arrowExtent.x;
            arrowOffset.y = currentOffset.y + arrowOffsetY;

            drawer->clearBitmapModulation();
            drawer->drawBitmapStretchSR(profile->mTextureObject, RectI(arrowOffset, arrowExtent), profile->getBitmapArrayRect((U32)iconIndex));
         }

         // get the appropriate font color
         ColorI fontColor;
         if (! myRow->mEnabled)
         {
            fontColor = profile->mFontColorNA;
         }
         else if (isRowSelected)
         {
            fontColor = profile->mFontColorSEL;
         }
         else if (isRowHighlighted)
         {
            fontColor = profile->mFontColorHL;
         }
         else
         {
            fontColor = profile->mFontColor;
         }

         // calculate text to be at the center between the arrows
         GFont * font = profile->mFont;
         StringTableEntry text = myRow->mOptions[myRow->mSelectedOption];
         S32 textWidth = font->getStrWidth(text);
         S32 columnWidth = profile->mHitAreaLowerRight.x * xScale - profile->mRightPad - columnSplit;
         S32 columnCenter = columnSplit + (columnWidth >> 1);
         S32 textStartX = columnCenter - (textWidth >> 1);
         Point2I textOffset(textStartX, 0);

         // render the option text itself
         Point2I textExtent(columnWidth, rowHeight);
         drawer->setBitmapModulation(fontColor);
         renderJustifiedText(currentOffset + textOffset, textExtent, text);
      }
   }
}

void GuiGameListOptionsCtrl::onDebugRender(Point2I offset)
{
   Parent::onDebugRender(offset);
   GuiGameListOptionsProfile * profile = (GuiGameListOptionsProfile *) mProfile;

   F32 xScale = (float) getWidth() / profile->getRowWidth();

   ColorI column1Color(255, 255, 0); // yellow
   ColorI column2Color(0, 255, 0); // green
   Point2I shrinker(-1, -1);

   S32 rowHeight = profile->getRowHeight();
   Point2I currentOffset(offset);
   Point2I rowExtent(getExtent().x, rowHeight);
   Point2I hitAreaExtent(profile->getHitAreaExtent());
   hitAreaExtent.x *= xScale;
   hitAreaExtent += shrinker;
   Point2I column1Extent((profile->mColumnSplit - profile->mHitAreaUpperLeft.x) * xScale - 1, hitAreaExtent.y);
   Point2I column2Extent(hitAreaExtent.x - column1Extent.x - 1, hitAreaExtent.y);
   Point2I hitAreaOffset = profile->mHitAreaUpperLeft;
   hitAreaOffset.x *= xScale;

   RectI borderRect;
   Point2I upperLeft;
   for (Vector<Parent::Row *>::iterator row = mRows.begin(); row < mRows.end(); ++row)
   {
      // set the top of the current row
      if (row != mRows.begin())
      {
         // rows other than the first can have padding above them
         currentOffset.y += (*row)->mHeightPad;
         currentOffset.y += rowHeight;
      }

      // draw the box around column 1
      upperLeft = currentOffset + hitAreaOffset;
      borderRect.point = upperLeft;
      borderRect.extent = column1Extent;
      GFX->getDrawUtil()->drawRect(borderRect, column1Color);

      // draw the box around column 2
      upperLeft.x += column1Extent.x + 1;
      borderRect.point = upperLeft;
      borderRect.extent = column2Extent;
      GFX->getDrawUtil()->drawRect(borderRect, column2Color);
   }
}

void GuiGameListOptionsCtrl::addRow(const char* label, const char* optionsList, bool wrapOptions, const char* callback, S32 icon, S32 yPad, bool enabled)
{
   static StringTableEntry DELIM = StringTable->insert("\t", true);
   Row * row = new Row();
   Vector<StringTableEntry> options( __FILE__, __LINE__ );
   S32 count = StringUnit::getUnitCount(optionsList, DELIM);
   for (int i = 0; i < count; ++i)
   {
      const char * option = StringUnit::getUnit(optionsList, i, DELIM);
      options.push_back(StringTable->insert(option, true));
   }
   row->mOptions = options;
   bool hasOptions = row->mOptions.size() > 0;
   row->mSelectedOption = (hasOptions) ? 0 : NO_OPTION;
   row->mWrapOptions = wrapOptions;
   Parent::addRow(row, label, callback, icon, yPad, true, (hasOptions) ? enabled : false);
}

void GuiGameListOptionsCtrl::setOptions(S32 rowIndex, const char * optionsList)
{
   static StringTableEntry DELIM = StringTable->insert("\t", true);

   if (! isValidRowIndex(rowIndex))
   {
      return;
   }

   Row * row = (Row *)mRows[rowIndex];

   S32 count = StringUnit::getUnitCount(optionsList, DELIM);
   row->mOptions.setSize(count);
   for (int i = 0; i < count; ++i)
   {
      const char * option = StringUnit::getUnit(optionsList, i, DELIM);
      row->mOptions[i] = StringTable->insert(option, true);
   }

   if (row->mSelectedOption >= row->mOptions.size())
   {
      row->mSelectedOption = row->mOptions.size() - 1;
   }
}

bool GuiGameListOptionsCtrl::hasValidProfile() const
{
   GuiGameListOptionsProfile * profile = dynamic_cast<GuiGameListOptionsProfile *>(mProfile);
   return profile;
}

void GuiGameListOptionsCtrl::enforceConstraints()
{
   Parent::enforceConstraints();
}

void GuiGameListOptionsCtrl::onMouseUp(const GuiEvent &event)
{
   S32 hitRow = getRow(event.mousePoint);
   if ((hitRow != NO_ROW) && isRowEnabled(hitRow) && (hitRow == getSelected()))
   {
      S32 xPos = globalToLocalCoord(event.mousePoint).x;
      clickOption((Row *) mRows[getSelected()], xPos);
   }
}

void GuiGameListOptionsCtrl::clickOption(Row * row, S32 xPos)
{
   GuiGameListOptionsProfile * profile = (GuiGameListOptionsProfile *) mProfile;
   if (! profile->hasArrows())
   {
      return;
   }

   F32 xScale = (float) getWidth() / profile->getRowWidth();

   S32 bitmapArrowWidth = mProfile->getBitmapArrayRect(Profile::TEX_FIRST_ARROW).extent.x;

   S32 leftArrowX1 = profile->mColumnSplit * xScale;
   S32 leftArrowX2 = leftArrowX1 + bitmapArrowWidth;

   S32 rightArrowX2 = (profile->mHitAreaLowerRight.x - profile->mRightPad) * xScale;
   S32 rightArrowX1 = rightArrowX2 - bitmapArrowWidth;

   if ((leftArrowX1 <= xPos) && (xPos <= leftArrowX2))
   {
      changeOption(row, -1);
   }
   else if ((rightArrowX1 <= xPos) && (xPos <= rightArrowX2))
   {
      changeOption(row, 1);
   }
}

bool GuiGameListOptionsCtrl::onKeyDown(const GuiEvent &event)
{
   switch (event.keyCode)
   {
      case KEY_LEFT:
         changeOption(-1);
         return true;

      case KEY_RIGHT:
         changeOption(1);
         return true;
      
      default:
         break;
   }

   return Parent::onKeyDown(event);
}

bool GuiGameListOptionsCtrl::onGamepadAxisLeft( const GuiEvent &event )
{
   changeOption(-1);
   return true;
}

bool GuiGameListOptionsCtrl::onGamepadAxisRight( const GuiEvent &event )
{
   changeOption(1);
   return true;
}

void GuiGameListOptionsCtrl::changeOption(S32 delta)
{
   if (getSelected() != NO_ROW)
   {
      Row * row = (Row *) mRows[getSelected()];
      changeOption(row, delta);
   }
}

void GuiGameListOptionsCtrl::changeOption(Row * row, S32 delta)
{
   S32 optionCount = row->mOptions.size();

   S32 newSelection = row->mSelectedOption + delta;
   if (optionCount == 0)
   {
      newSelection = NO_OPTION;
   }
   else if (! row->mWrapOptions)
   {
      newSelection = mClamp(newSelection, 0, optionCount - 1);
   }
   else if (newSelection < 0)
   {
      newSelection = optionCount - 1;
   }
   else if (newSelection >= optionCount)
   {
      newSelection = 0;
   }
   row->mSelectedOption = newSelection;

   static StringTableEntry LEFT = StringTable->insert("LEFT", true);
   static StringTableEntry RIGHT = StringTable->insert("RIGHT", true);

   if (row->mScriptCallback != NULL)
   {
      setThisControl();
      StringTableEntry direction = NULL;
      if (delta < 0)
      {
         direction = LEFT;
      }
      else if (delta > 0)
      {
         direction = RIGHT;
      }
      if ((direction != NULL) && (Con::isFunction(row->mScriptCallback)))
      {
         Con::executef(row->mScriptCallback, direction);
      }
   }
}

StringTableEntry GuiGameListOptionsCtrl::getCurrentOption(S32 rowIndex) const
{
   if (isValidRowIndex(rowIndex))
   {
      Row * row = (Row *) mRows[rowIndex];
      if (row->mSelectedOption != NO_OPTION)
      {
         return row->mOptions[row->mSelectedOption];
      }
   }
   return StringTable->insert("", false);
}

bool GuiGameListOptionsCtrl::selectOption(S32 rowIndex, const char * theOption)
{
   if (! isValidRowIndex(rowIndex))
   {
      return false;
   }

   Row * row = (Row *) mRows[rowIndex];

   for (Vector<StringTableEntry>::iterator anOption = row->mOptions.begin(); anOption < row->mOptions.end(); ++anOption)
   {
      if (dStrcmp(*anOption, theOption) == 0)
      {
         S32 newIndex = anOption - row->mOptions.begin();
         row->mSelectedOption = newIndex;
         return true;
      }
   }

   return false;
}

//-----------------------------------------------------------------------------
// Console stuff (GuiGameListOptionsCtrl)
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiGameListOptionsCtrl);

ConsoleDocClass( GuiGameListOptionsCtrl,
   "@brief A control for showing pages of options that are gamepad friendly.\n\n"

   "Each row in this control allows the selection of one value from a set of "
   "options using the keyboard, gamepad or mouse. The row is rendered as 2 "
   "columns: the first column contains the row label, the second column "
   "contains left and right arrows (for mouse picking) and the currently "
   "selected value.\n\n"

   "@see GuiGameListOptionsProfile\n\n"

   "@ingroup GuiGame"
);

void GuiGameListOptionsCtrl::initPersistFields()
{
   Parent::initPersistFields();
}

DefineEngineMethod( GuiGameListOptionsCtrl, addRow, void,
   ( const char* label, const char* options, bool wrapOptions, const char* callback, S32 icon, S32 yPad, bool enabled ),
   ( -1, 0, true ),
   "Add a row to the list control.\n\n"
   "@param label The text to display on the row as a label.\n"
   "@param options A tab separated list of options.\n"
   "@param wrapOptions Specify true to allow options to wrap at each end or false to prevent wrapping.\n"
   "@param callback Name of a script function to use as a callback when this row is activated.\n"
   "@param icon [optional] Index of the icon to use as a marker.\n"
   "@param yPad [optional] An extra amount of height padding before the row. Does nothing on the first row.\n"
   "@param enabled [optional] If this row is initially enabled." )
{
   object->addRow( label, options, wrapOptions, callback, icon, yPad, enabled );
}

DefineEngineMethod( GuiGameListOptionsCtrl, getCurrentOption, const char *, ( S32 row ),,
   "Gets the text for the currently selected option of the given row.\n\n"
   "@param row Index of the row to get the option from.\n"
   "@return A string representing the text currently displayed as the selected option on the given row. If there is no such displayed text then the empty string is returned." )
{
   return object->getCurrentOption( row );
}

DefineEngineMethod( GuiGameListOptionsCtrl, selectOption, bool, ( S32 row, const char* option ),,
   "Set the row's current option to the one specified\n\n"
   "@param row Index of the row to set an option on.\n"
   "@param option The option to be made active.\n"
   "@return True if the row contained the option and was set, false otherwise." )
{
   return object->selectOption( row, option );
}

DefineEngineMethod( GuiGameListOptionsCtrl, setOptions, void, ( S32 row, const char* optionsList ),,
   "Sets the list of options on the given row.\n\n"
   "@param row Index of the row to set options on."
   "@param optionsList A tab separated list of options for the control." )
{
   object->setOptions( row, optionsList );
}

//-----------------------------------------------------------------------------
// GuiGameListOptionsProfile
//-----------------------------------------------------------------------------

GuiGameListOptionsProfile::GuiGameListOptionsProfile()
: mColumnSplit(0),
  mRightPad(0)
{
}

void GuiGameListOptionsProfile::enforceConstraints()
{
   Parent::enforceConstraints();
   
   if( mHitAreaUpperLeft.x > mColumnSplit || mColumnSplit > mHitAreaLowerRight.x )
      Con::errorf( "GuiGameListOptionsProfile: You can't create %s with a ColumnSplit outside the hit area. You set the split to %d. Please change the ColumnSplit to be in the range [%d, %d].",
         getName(), mColumnSplit, mHitAreaUpperLeft.x, mHitAreaLowerRight.x);

   mColumnSplit = mClamp(mColumnSplit, mHitAreaUpperLeft.x, mHitAreaLowerRight.x);
}

//-----------------------------------------------------------------------------
// Console stuff (GuiGameListOptionsProfile)
//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiGameListOptionsProfile);

ConsoleDocClass( GuiGameListOptionsProfile,
   "@brief A GuiControlProfile with additional fields specific to GuiGameListOptionsCtrl.\n\n"

   "@tsexample\n"
   "new GuiGameListOptionsProfile()\n"
   "{\n"
   "   columnSplit = \"100\";\n"
   "   rightPad = \"4\";\n"
   "   //Properties not specific to this control have been omitted from this example.\n"
   "};\n"
   "@endtsexample\n\n"

   "@ingroup GuiGame"
);

void GuiGameListOptionsProfile::initPersistFields()
{
   addField( "columnSplit", TypeS32, Offset(mColumnSplit, GuiGameListOptionsProfile),
      "Padding between the leftmost edge of the control, and the row's left arrow." );

   addField( "rightPad", TypeS32, Offset(mRightPad, GuiGameListOptionsProfile),
      "Padding between the rightmost edge of the control and the row's right arrow." );

   Parent::initPersistFields();
}
