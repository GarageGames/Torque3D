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

#ifndef _GuiGameListOptionsCtrl_H_
#define _GuiGameListOptionsCtrl_H_

#include "gui/controls/guiGameListMenuCtrl.h"

/// \class GuiGameListOptionsCtrl
/// A control for showing pages of options that are gamepad friendly.
class GuiGameListOptionsCtrl : public GuiGameListMenuCtrl
{
   typedef GuiGameListMenuCtrl Parent;

protected:
   /// \struct Row
   /// An extension to the parent's row, adding the ability to keep a collection
   /// of options and track status related to them.
   struct Row : public Parent::Row
   {
      Vector<StringTableEntry>   mOptions;         ///< Collection of options available to display
      S32                        mSelectedOption;  ///< Index into mOptions pointing at the selected option
      bool                       mWrapOptions;     ///< Determines if options should "wrap around" at the ends

      Row() : mSelectedOption(0), mWrapOptions(false)
      {
         VECTOR_SET_ASSOCIATION( mOptions );
      }
   };

public:
   /// Gets the text for the currently selected option of the given row.
   ///
   /// \param rowIndex Index of the row to get the option from.
   /// \return A string representing the text currently displayed as the selected
   /// option on the given row. If there is no such displayed text then the empty
   /// string is returned.
   StringTableEntry getCurrentOption(S32 rowIndex) const;

   /// Attempts to set the given row to the specified selected option. The option
   /// will only be set if the option exists in the control.
   ///
   /// \param rowIndex Index of the row to set an option on.
   /// \param option The option to be made active.
   /// \return True if the row contained the option and was set, false otherwise.
   bool selectOption(S32 rowIndex, StringTableEntry option);

   /// Sets the list of options on the given row.
   ///
   /// \param rowIndex Index of the row to set options on.
   /// \param optionsList A tab separated list of options for the control.
   void setOptions(S32 rowIndex, const char * optionsList);

   /// Adds a row to the control.
   ///
   /// \param label The text to display on the row as a label.
   /// \param optionsList A tab separated list of options for the control.
   /// \param wrapOptions Specify true to allow options to wrap at the ends or
   /// false to prevent wrapping.
   /// \param callback [optional] Name of a script function to use as a callback
   /// when this row is activated. Default NULL means no callback.
   /// \param icon [optional] Index of the icon to use as a marker. Default -1
   /// means no icon will be shown on this row.
   /// \param yPad [optional] An extra amount of height padding before the row.
   /// \param enabled [optional] If this row is initially enabled. Default true.
   void addRow(const char* label, const char* optionsList, bool wrapOptions, const char* callback, S32 icon = -1, S32 yPad = 0, bool enabled = true);

   void onRender(Point2I offset, const RectI &updateRect);

   /// Callback when the mouse button is released.
   ///
   /// \param event A reference to the event that triggered the callback.
   void onMouseUp(const GuiEvent &event);

   /// Callback when a key is pressed.
   ///
   /// \param event The event that triggered this callback.
   bool onKeyDown(const GuiEvent &event);

   /// Callback when a key is repeating.
   ///
   /// \param event The event that triggered this callback.
   bool onKeyRepeat(const GuiEvent &event){ return onKeyDown(event); }

   /// Callback when the gamepad axis is activated.
   ///
   /// \param event A reference to the event that triggered the callback.
   virtual bool onGamepadAxisLeft(const GuiEvent &event);

   /// Callback when the gamepad axis is activated.
   ///
   /// \param event A reference to the event that triggered the callback.
   virtual bool onGamepadAxisRight(const GuiEvent &event);

   GuiGameListOptionsCtrl();
   ~GuiGameListOptionsCtrl();

   DECLARE_CONOBJECT(GuiGameListOptionsCtrl);
   DECLARE_DESCRIPTION( "A control for showing pages of options that are gamepad friendly." );
   
   virtual bool onAdd();

   /// Initializes fields accessible through the console.
   static void initPersistFields();

   static const S32 NO_OPTION = -1; ///< Indicates there is no option

protected:
   /// Checks to make sure our control has a profile of the correct type.
   ///
   /// \return True if the profile is of type GuiGameListOptionsProfile or false
   /// if the profile is of any other type.
   bool hasValidProfile() const;

   /// Enforces the validity of the fields on this control and its profile (if the
   /// profile is valid, see: hasValidProfile).
   void enforceConstraints();

   /// Adds lines around the column divisions to the feedback already provided
   /// in the Parent.
   void onDebugRender(Point2I offset);

private:
   /// Performs a click on the current option row. The x position is used to
   /// determine if the left or right arrow were clicked. If one was clicked, the
   /// option will be changed. If neither was clicked, the option is unaffected.
   /// This method should only be called when there is an actively selected row.
   ///
   /// \param row The row to perform the click on.
   /// \param xPos The x position of the the click, relative to the control.
   void clickOption(Row * row, S32 xPos);

   /// Changes the option on the currently selected row. If there is no row
   /// selected, this method does nothing.
   ///
   /// \param delta The amount to change the option selection by. Typically this
   /// will be 1 or -1.
   void changeOption(S32 delta);

   /// Changes the option on the given row.
   ///
   /// \param row The row to change the option on.
   /// \param delta The amount to change the option selection by. Typically this
   /// will be 1 or -1.
   void changeOption(Row * row, S32 delta);
};

/// \class GuiGameListOptionsProfile
/// A gui profile with additional fields specific to GuiGameListOptionsCtrl.
class GuiGameListOptionsProfile : public GuiGameListMenuProfile
{
   typedef GuiGameListMenuProfile Parent;

public:
   /// Enforces range constraints on all required fields.
   void enforceConstraints();

   GuiGameListOptionsProfile();

   S32   mColumnSplit;  ///< Absolute position of the split between columns
   S32   mRightPad;     ///< Extra padding between the right arrow and the hit area

   DECLARE_CONOBJECT(GuiGameListOptionsProfile);

   /// Initializes fields accessible through the console.
   static void initPersistFields();
};

#endif
