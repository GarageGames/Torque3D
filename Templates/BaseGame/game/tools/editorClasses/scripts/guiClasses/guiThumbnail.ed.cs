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

// This file merely contains the base functionality for creating your own
// 'subclassed' script namkespaces that define the visual appearance of 
// a thumbnail for a guiThumnailPopup list.
//
// All border creation and callback click functionality is also defined in
// this file and may be overriden in your namespaces provided that you 
// properly invoke the Parent::onMethodName( %parameterList ) to all this
// base namespace to do it's dependent processing.

//function GuiDefaultThumbnail::onAdd( %this )
//{
   //// Nothing Here.
//}
//
//function GuiDefaultThumbnail::onRemove( %this )
//{
   //// Nothing Here.
//}

//-----------------------------------------------------------------------------
// Object Browser Item Default Behaviors
//-----------------------------------------------------------------------------
function GuiDefaultThumbnail::onClick( %this )
{
   // Store data and hide the dialog.
   if( isObject( %this.base ) )
   {
      %this.base.item = %this;
      %this.base.Hide();
   }
}

function GuiDefaultThumbnail::onRightClick( %this )
{
   // Nothing Here.   
}

function GuiDefaultThumbnail::onMouseLeave( %this )
{
   // Nothing Here.
}

function ObjectBrowserItem::onMouseEnter( %this )
{
   // Nothing Here.
}

function GuiDefaultThumbnail::onDoubleClick( %this )
{
   // By Default if the base funcitonality is called
   // in onClick, we will never get here.  However, if
   // you want to override this functionality, simply 
   // override onClick and don't call the parent.
}
