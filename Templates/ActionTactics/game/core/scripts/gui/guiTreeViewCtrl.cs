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

function GuiTreeViewCtrl::onDefineIcons( %this )
{
   %icons = "core/art/gui/images/treeview/default:" @
            "core/art/gui/images/treeview/simgroup:" @
            "core/art/gui/images/treeview/simgroup_closed:" @
            "core/art/gui/images/treeview/simgroup_selected:" @
            "core/art/gui/images/treeview/simgroup_selected_closed:" @      
            "core/art/gui/images/treeview/hidden:" @      
            "core/art/gui/images/treeview/shll_icon_passworded_hi:" @
            "core/art/gui/images/treeview/shll_icon_passworded:" @      
            "core/art/gui/images/treeview/default";
              
   %this.buildIconTable(%icons);   
}

function GuiTreeViewCtrl::handleRenameObject( %this, %name, %obj )
{
   %inspector = GuiInspector::findByObject( %obj );
   
   if( isObject( %inspector ) )   
   {
      %field = ( %this.renameInternal ) ? "internalName" : "name";      
      %inspector.setObjectField( %field, %name );
      return true;
   }
   
   return false;   
}
