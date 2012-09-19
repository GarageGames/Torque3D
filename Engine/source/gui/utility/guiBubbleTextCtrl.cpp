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

#include "gui/utility/guiBubbleTextCtrl.h"
#include "gui/core/guiCanvas.h"

IMPLEMENT_CONOBJECT(GuiBubbleTextCtrl);

ConsoleDocClass( GuiBubbleTextCtrl,
   "@brief A single-line text control that displays its text in a multi-line popup when clicked.\n\n"

   "This control acts like a GuiTextCtrl (and inherits from it), when clicked it creates a GuiMLTextCtrl "
   "roughly where you clicked with the same text in it.  This allows you to have a single line text control "
   "which upon clicking will display the entire text contained in a multi-line format.\n\n"

   "@tsexample\n"
   "new GuiBubbleTextCtrl(BubbleTextGUI)\n"
   "{\n"
   "   text = \"This is the first sentence.  This second sentence can be sized outside of the default single "
               "line view, upon clicking this will be displayed in a multi-line format.\";\n"
   "};\n"
   "@endtsexample\n\n"

   "@see GuiTextCtrl\n"
   "@see GuiMLTextCtrl\n\n"

   "@ingroup GuiControls\n"
);

//------------------------------------------------------------------------------
void GuiBubbleTextCtrl::popBubble()
{
	// Release the mouse:
	mInAction = false;
	mouseUnlock();

	// Pop the dialog
	getRoot()->popDialogControl(mDlg);

	// Kill the popup
	mDlg->removeObject(mPopup);
	mPopup->removeObject(mMLText);
	mMLText->deleteObject();
	mPopup->deleteObject();
	mDlg->deleteObject();
}

//------------------------------------------------------------------------------
void GuiBubbleTextCtrl::onMouseDown(const GuiEvent &event)
{
	if (mInAction)
	{
		popBubble();

		return;
	}

	mDlg = new GuiControl();
   AssertFatal(mDlg, "Failed to create the GuiControl for the BubbleTextCtrl");
   mDlg->setDataField( StringTable->insert("profile"), NULL, "GuiModelessDialogProfile");
   mDlg->setField("horizSizing", "width");
	mDlg->setField("vertSizing", "height");
	mDlg->setField("extent", "640 480");

   mPopup = new GuiControl();
   AssertFatal(mPopup, "Failed to create the GuiControl for the BubbleTextCtrl");
   mPopup->setDataField( StringTable->insert("profile"), NULL, "GuiBubblePopupProfile");

   mMLText = new GuiMLTextCtrl();
   AssertFatal(mMLText, "Failed to create the GuiMLTextCtrl for the BubbleTextCtrl");
	mMLText->setDataField( StringTable->insert("profile"), NULL, "GuiBubbleTextProfile");
	mMLText->setField("position", "2 2");
	mMLText->setField("extent", "296 51");	
	
	mMLText->setText((char*)mText,dStrlen(mText));

	mMLText->registerObject();
	mPopup->registerObject();
	mDlg->registerObject();

	mPopup->addObject(mMLText);
	mDlg->addObject(mPopup);

	mPopup->resize(event.mousePoint,Point2I(300,55));

	getRoot()->pushDialogControl(mDlg,0);
	mouseLock();

	mInAction = true;
}
