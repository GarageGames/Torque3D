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

function ESettingsWindow::startup( %this )
{
   ESettingsWindowTabBook.selectPage( 0 );
   ESettingsWindowList.setSelectedById( 0 );
}

function ESettingsWindow::onWake( %this )
{
}

function ESettingsWindow::hideDialog( %this )
{
   %this.setVisible(false);
}

function ESettingsWindow::ToggleVisibility()
{
   if ( ESettingsWindow.visible  )
   {
      ESettingsWindow.setVisible(false);
   }
   else
   {
      ESettingsWindow.setVisible(true);
      ESettingsWindow.selectWindow();
      ESettingsWindow.setCollapseGroup(false);
   }
}

function ESettingsWindow::addTabPage( %this, %page )
{
   ESettingsWindowTabBook.add( %page );
   ESettingsWindowList.addRow( ESettingsWindowTabBook.getSelectedPage(), %page.text );
   ESettingsWindowList.sort(0);
}

//-----------------------------------------------------------------------------

function ESettingsWindowList::onSelect( %this, %id, %text )
{
   ESettingsWindowTabBook.selectPage( %id );
}

//-----------------------------------------------------------------------------
// Standard settings GUI classes.  Editors may define their own methods of
// working with settings and are not required to use these.
//-----------------------------------------------------------------------------

function ESettingsWindowCheckbox::onWake( %this )
{
   %this.setStateOn( EditorSettings.value( %this.editorSettingsValue ));
}

function ESettingsWindowCheckbox::onClick( %this )
{
   EditorSettings.setValue( %this.editorSettingsValue, %this.getValue() );
   eval(%this.editorSettingsRead);
}

//-----------------------------------------------------------------------------

function ESettingsWindowTextEdit::onWake( %this )
{
   %this.setText( EditorSettings.value( %this.editorSettingsValue ));
}

function ESettingsWindowTextEdit::onValidate( %this )
{
   EditorSettings.setValue( %this.editorSettingsValue, %this.getValue() );
   eval(%this.editorSettingsRead);
}

function ESettingsWindowTextEdit::onGainFirstResponder( %this )
{
   %this.selectAllText();
}

//-----------------------------------------------------------------------------

function ESettingsWindowColor::apply( %this, %color )
{
   EditorSettings.setValue( %this.editorSettingsValue, %color );
   eval(%this.editorSettingsRead);

   %this.findObjectByInternalName("ColorEdit", true).setText( %color);
   %this.findObjectByInternalName("ColorButton", true).color = ColorIntToFloat( %color );
}

function ESettingsWindowColorEdit::onWake( %this )
{
   %this.setText( EditorSettings.value( %this.getParent().editorSettingsValue ));
}

function ESettingsWindowColorEdit::onValidate( %this )
{
   %this.getParent().apply( %this.getValue() );
}

function ESettingsWindowColorEdit::onGainFirstResponder( %this )
{
   %this.selectAllText();
}

function ESettingsWindowColorButton::onWake( %this )
{
   %this.color = ColorIntToFloat( EditorSettings.value( %this.getParent().editorSettingsValue ) );
}

function ESettingsWindowColorButton::onClick( %this )
{
   getColorI( ColorFloatToInt( %this.color ), %this.getId() @ ".apply", %this.getRoot() );
   //EditorSettings.setValue( %this.editorSettingsValue, %this.getValue() );
   //eval(%this.editorSettingsRead);
}

function ESettingsWindowColorButton::apply( %this, %color )
{
   %this.getParent().apply(%color);
   echo("ESettingsWindowColorButton::apply(): " @ %color);
}
