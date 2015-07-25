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

//----------------------------------------------------------------------------
// Enter Chat Message Hud
//----------------------------------------------------------------------------

//------------------------------------------------------------------------------

function MessageHud::open(%this)
{
   %offset = 6;

   if(%this.isVisible())
      return;

   if(%this.isTeamMsg)
      %text = "TEAM:";
   else
      %text = "GLOBAL:";

   MessageHud_Text.setValue(%text);
   
   %windowPos = "0 " @ ( getWord( outerChatHud.position, 1 ) + getWord( outerChatHud.extent, 1 ) + 1 );
   %windowExt = getWord( OuterChatHud.extent, 0 ) @ " " @ getWord( MessageHud_Frame.extent, 1 );
   
   %textExtent = getWord(MessageHud_Text.extent, 0) + 14;
   %ctrlExtent = getWord(MessageHud_Frame.extent, 0);

   Canvas.pushDialog(%this);
   
   messageHud_Frame.position = %windowPos;
   messageHud_Frame.extent = %windowExt;
   MessageHud_Edit.position = setWord(MessageHud_Edit.position, 0, %textExtent + %offset);
   MessageHud_Edit.extent = setWord(MessageHud_Edit.extent, 0, %ctrlExtent - %textExtent - (2 * %offset));

   %this.setVisible(true);
   deactivateKeyboard();
   MessageHud_Edit.makeFirstResponder(true);
}

//------------------------------------------------------------------------------

function MessageHud::close(%this)
{
   if(!%this.isVisible())
      return;
      
   Canvas.popDialog(%this);
   %this.setVisible(false);
   if ( $enableDirectInput )
      activateKeyboard();
   MessageHud_Edit.setValue("");
}


//------------------------------------------------------------------------------

function MessageHud::toggleState(%this)
{
   if(%this.isVisible())
      %this.close();
   else
      %this.open();
}

//------------------------------------------------------------------------------

function MessageHud_Edit::onEscape(%this)
{
   MessageHud.close();
}

//------------------------------------------------------------------------------

function MessageHud_Edit::eval(%this)
{
   %text = collapseEscape(trim(%this.getValue()));

   if(%text !$= "")
   {
      if(MessageHud.isTeamMsg)
         commandToServer('teamMessageSent', %text);
      else
         commandToServer('messageSent', %text);
   }

   MessageHud.close();
}

   
//----------------------------------------------------------------------------
// MessageHud key handlers

function toggleMessageHud(%make)
{
   if(%make)
   {
      MessageHud.isTeamMsg = false;
      MessageHud.toggleState();
   }
}

function teamMessageHud(%make)
{
   if(%make)
   {
      MessageHud.isTeamMsg = true;
      MessageHud.toggleState();
   }
}
