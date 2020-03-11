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

function HelpDlg::onWake(%this)
{
   HelpFileList.entryCount = 0;
   HelpFileList.clear();
   for(%file = findFirstFile("*.hfl"); %file !$= ""; %file = findNextFile("*.hfl"))
   {
      HelpFileList.fileName[HelpFileList.entryCount] = %file;
      HelpFileList.addRow(HelpFileList.entryCount, fileBase(%file));
      HelpFileList.entryCount++;
   }
   HelpFileList.sortNumerical(0);
   for(%i = 0; %i < HelpFileList.entryCount; %i++)
   {
      %rowId = HelpFileList.getRowId(%i);
      %text = HelpFileList.getRowTextById(%rowId);
      %text = %i + 1 @ ". " @ restWords(%text);
      HelpFileList.setRowById(%rowId, %text);
   }
   HelpFileList.setSelectedRow(0);
}

function HelpFileList::onSelect(%this, %row)
{
   %fo = new FileObject();
   %fo.openForRead(%this.fileName[%row]);
   %text = "";
   while(!%fo.isEOF())
      %text = %text @ %fo.readLine() @ "\n";

   %fo.delete();
   HelpText.setText(%text);
}

function getHelp(%helpName)
{
   Canvas.pushDialog(HelpDlg);
   if(%helpName !$= "")
   {
      %index = HelpFileList.findTextIndex(%helpName);
      HelpFileList.setSelectedRow(%index);
   }
}

function contextHelp()
{
   for(%i = 0; %i < Canvas.getCount(); %i++)
   {
      if(Canvas.getObject(%i).getName() $= HelpDlg)
      {
         Canvas.popDialog(HelpDlg);
         return;
      }
   }
   %content = Canvas.getContent();
   %helpPage = %content.getHelpPage();
   getHelp(%helpPage);
}

function GuiControl::getHelpPage(%this)
{
   return %this.helpPage;
}

function GuiMLTextCtrl::onURL(%this, %url)
{
   gotoWebPage( %url );
}   

