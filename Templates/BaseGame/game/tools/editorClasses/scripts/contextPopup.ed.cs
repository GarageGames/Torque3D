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


/// @class ContextPopup
/// @brief Create a Popup Dialog that closes itself when signaled by an event.
///
/// ContextPopup is a support class that offers a simple way of displaying
/// reusable context sensitive GuiControl popups.  These dialogs are created
/// and shown to the user when the <b>show</b> method is used. 
///
/// Once a Popup is shown it will be dismissed if it meets one of a few 
///   criteria.
///
///  1. A user clicks anywhere outside the bounds of the GuiControl, specified by
///     the 'dialog' field on the object.
///  2. Time Passes of (n)Milliseconds, specifed by the 'timeout' field on 
///     the object.
///
/// For example, if you wished to create a context dialog with a dialog you held in 
/// a local variable named %myDialog you would create a new script object as such.
/// 
/// 
/// @code
/// %MyContextPopup = new ScriptObject() 
/// {
///    class     = ContextPopup;
///    superClass= MyCallbackNamespace; // Only Necessary when you want to perform logic pre/post showing
///    dialog    = %%myDialog.getID();
///    delay     = 500; // Pop the Popup after 500 Milliseconds
/// };
/// @endcode
/// 
/// Now, if you wanted to show the dialog %%myDialog and have it dismissed when anything in the
/// background is clicked, simply call the following.
/// 
///
/// @code
/// %MyContextPopup.show( %positionX, %positionY );
/// @endcode 
/// 
/// If you need to know more than show the dialog and hide it when clicked or time passes, ContextPopup
/// Provides callback methods that you may override for doing intermediate processing on a dialog
/// that is to be shown or is being hidden.  For example, in the above script we created a Context Dialog Container
/// called @%myContextDialog with a superClass of <b>MyCallbackNamespace</b>.  If we wanted to hide the cursor when
/// the dialog was shown, and show it when the dialog was hidden, we could implement the following functions.
/// 
/// @code
/// function MyCallbackNamespace::onContextActivate( %%this ) 
/// {
///     // Hide Cursor Here
/// }
/// function MyCallbackNamespace::onContextDeactivate( %%this )
/// {
///     // Show Cursor Here
/// }
/// @endcode
///
/// @field GuiControl Dialog The GuiControl dialog to be shown when the context dialog is activated

function ContextDialogContainer::onAdd(%this)
{
   // Add to our cleanup group.
   $EditorClassesGroup.add( %this );  
   
   %this.base = new GuiButtonBaseCtrl()
   {
      profile = ToolsGuiTransparentProfile;
      class = ContextDialogWatcher;
      parent = %this;
      modal = true;
   };
   
   // Flag not active.
   %this.isPushed = false;
   
   // Add to our cleanup group.
   $EditorClassesGroup.add( %this.base );
   
   return true;

}

function ContextDialogContainer::onRemove(%this)
{
   %this.Hide();
   
   if( isObject( %this.base ) ) 
      %this.base.delete();
}



//-----------------------------------------------------------------------------
/// (SimID this, int positionX, int positionY)
/// Shows the GuiControl specified in the Dialog field at the coordinates passed
/// to this function. If no coordinates are passed to this function, the Dialog control
/// is shown using it's current position.
/// 
/// @param this The ContextDialogContainer object
/// @param positionX The X Position in Global Screen Coordinates to display the dialog
/// @param positionY The Y Position in Global Screen Coordinates to display the dialog
/// @param delay Optional delay before this popup is hidden that overrides that specified at construction time
///
//-----------------------------------------------------------------------------   
function ContextDialogContainer::Show( %this, %positionX, %positionY, %delay )
{
   if( %this.isPushed == true ) 
      return true;
      
   if( !isObject( %this.Dialog ) )
      return false;

   // Store old parent.
   %this.oldParent = %this.dialog.getParent();
         
   // Set new parent.
   %this.base.add( %this.Dialog );
   
   if( %positionX !$= "" && %positionY !$= "" )
      %this.Dialog.setPositionGlobal( %positionX, %positionY );
   
   Canvas.pushDialog( %this.base, 99 );
   
   // Setup Delay Schedule   
   if( isEventPending( %this.popSchedule ) )
      cancel( %this.popSchedule );
   if( %delay !$= "" )
      %this.popSchedule = %this.schedule( %delay, hide );
   else if( %this.Delay !$= "" )
      %this.popSchedule = %this.schedule( %this.Delay, hide );
   
}

//-----------------------------------------------------------------------------
/// (SimID this)
/// Hides the GuiControl specified in the Dialog field if shown. This function
/// is provided merely for more flexibility in when your dialog is shown.  If you
/// do not call this function, it will be called when the dialog is dismissed by
/// a background click.
/// 
/// @param this The ContextDialogContainer object
///
//-----------------------------------------------------------------------------   
function ContextDialogContainer::Hide( %this )
{
   if( %this.isPushed == true )
      Canvas.popDialog( %this.base );
      
   // Restore Old Parent;
   if( isObject( %this.Dialog ) && isObject( %this.oldParent ) )
      %this.oldParent.add( %this.Dialog );
}



// ContextDialogWatcher Class - JDD
// CDW is a namespace link for the context background button to catch
//     event information and pass it back to the main class.
//
// onClick it will dismiss the parent
// onDialogPop it will cleanup and notify user of deactivation
// onDialogPush it will initialize state information and notify user of activation
function ContextDialogWatcher::onClick( %this )
{
   if( isObject( %this.parent ) )
      %this.parent.hide();
}

function ContextDialogWatcher::onDialogPop( %this )
{
   if( !isObject( %this.parent ) )
      return;

   %this.parent.isPushed = false;

   if( %this.parent.isMethod( "onContextDeactivate" ) )
      %this.parent.onContextDeactivate();
}

function ContextDialogWatcher::onDialogPush( %this )
{
   if( !isObject( %this.parent ) )
      return;
   
   %this.parent.isPushed = true;
   
   if( %this.parent.isMethod( "onContextActivate" ) )
      %this.parent.onContextActivate();

}