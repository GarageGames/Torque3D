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

#include "gui/containers/guiDragAndDropCtrl.h"
#include "gui/core/guiCanvas.h"
#include "console/engineAPI.h"


IMPLEMENT_CONOBJECT( GuiDragAndDropControl );

ConsoleDocClass( GuiDragAndDropControl,
   "@brief A container control that can be used to implement drag&drop behavior.\n\n"
   
   "GuiDragAndDropControl is a special control that can be used to allow drag&drop behavior to be implemented where "
   "GuiControls may be dragged across the canvas and the dropped on other GuiControls.\n\n"
   
   "To start a drag operation, construct a GuiDragAndDropControl and add the control that should be drag&dropped "
   "as a child to it.  Note that this must be a single child control.  To drag multiple controls, wrap them in a new "
   "GuiControl object as a temporary container.\n\n"
   
   "Then, to initiate the drag, add the GuiDragAndDropControl to the canvas and call startDragging().  You can optionally "
   "supply an offset to better position the GuiDragAndDropControl on the mouse cursor.\n\n"
   
   "As the GuiDragAndDropControl is then moved across the canvas, it will call the onControlDragEnter(), onControlDragExit(), "
   "onControlDragged(), and finally onControlDropped() callbacks on the visible topmost controls that it moves across.  "
   "onControlDropped() is called when the mouse button is released and the drag operation thus finished.\n\n"
   
   "@tsexample\n"
      "// The following example implements drag&drop behavior for GuiSwatchButtonCtrl so that\n"
      "// one color swatch may be dragged over the other to quickly copy its color.\n"
      "//\n"
      "// This code is taken from the stock scripts.\n"
      "\n"
      "//---------------------------------------------------------------------------------------------\n"
      "\n"
      "// With this method, we start the operation when the mouse is click-dragged away from a color swatch.\n"
      "function GuiSwatchButtonCtrl::onMouseDragged( %this )\n"
      "{\n"
      "   // First we construct a new temporary swatch button that becomes the payload for our\n"
      "   // drag operation and give it the properties of the swatch button we want to copy.\n"
      "\n"
      "   %payload = new GuiSwatchButtonCtrl();\n"
      "   %payload.assignFieldsFrom( %this );\n"
      "   %payload.position = \"0 0\";\n"
      "   %payload.dragSourceControl = %this; // Remember where the drag originated from so that we don't copy a color swatch onto itself.\n"
      "\n"
      "   // Calculate the offset of the GuiDragAndDropControl from the mouse cursor.  Here we center\n"
      "   // it on the cursor.\n"
      "\n"
      "   %xOffset = getWord( %payload.extent, 0 ) / 2;\n"
      "   %yOffset = getWord( %payload.extent, 1 ) / 2;\n"
      "\n"
      "   // Compute the initial position of the GuiDragAndDrop control on the cavas based on the current\n"
      "   // mouse cursor position.\n"
      "\n"
      "   %cursorpos = Canvas.getCursorPos();\n"
      "   %xPos = getWord( %cursorpos, 0 ) - %xOffset;\n"
      "   %yPos = getWord( %cursorpos, 1 ) - %yOffset;\n"
      "\n"
      "   // Create the drag control.\n"
      "\n"
      "   %ctrl = new GuiDragAndDropControl()\n"
      "   {\n"
      "      canSaveDynamicFields    = \"0\";\n"
      "      Profile                 = \"GuiSolidDefaultProfile\";\n"
      "      HorizSizing             = \"right\";\n"
      "      VertSizing              = \"bottom\";\n"
      "      Position                = %xPos SPC %yPos;\n"
      "      extent                  = %payload.extent;\n"
      "      MinExtent               = \"4 4\";\n"
      "      canSave                 = \"1\";\n"
      "      Visible                 = \"1\";\n"
      "      hovertime               = \"1000\";\n"
      "\n"
      "      // Let the GuiDragAndDropControl delete itself on mouse-up.  When the drag is aborted,\n"
      "      // this not only deletes the drag control but also our payload.\n"
      "      deleteOnMouseUp         = true;\n"
      "\n"
      "      // To differentiate drags, use the namespace hierarchy to classify them.\n"
      "      // This will allow a color swatch drag to tell itself apart from a file drag, for example.\n"
      "      class                   = \"GuiDragAndDropControlType_ColorSwatch\";\n"
      "   };\n"
      "\n"
      "   // Add the temporary color swatch to the drag control as the payload.\n"
      "   %ctrl.add( %payload );\n"
      "\n"
      "   // Start drag by adding the drag control to the canvas and then calling startDragging().\n"
      "\n"
      "   Canvas.getContent().add( %ctrl );\n"
      "   %ctrl.startDragging( %xOffset, %yOffset );\n"
      "}\n"
      "\n"
      "//---------------------------------------------------------------------------------------------\n"
      "\n"
      "// This method receives the drop when the mouse button is released over a color swatch control\n"
      "// during a drag operation.\n"
      "function GuiSwatchButtonCtrl::onControlDropped( %this, %payload, %position )\n"
      "{\n"
      "   // Make sure this is a color swatch drag operation.\n"
      "   if( !%payload.parentGroup.isInNamespaceHierarchy( \"GuiDragAndDropControlType_ColorSwatch\" ) )\n"
      "      return;\n"
      "\n"
      "   // If dropped on same button whence we came from,\n"
      "   // do nothing.\n"
      "\n"
      "   if( %payload.dragSourceControl == %this )\n"
      "      return;\n"
      "\n"
      "   // If a swatch button control is dropped onto this control,\n"
      "   // copy it's color.\n"
      "\n"
      "   if( %payload.isMemberOfClass( \"GuiSwatchButtonCtrl\" ) )\n"
      "   {\n"
      "      // If the swatch button is part of a color-type inspector field,\n"
      "      // remember the inspector field so we can later set the color\n"
      "      // through it.\n"
      "\n"
      "      if( %this.parentGroup.isMemberOfClass( \"GuiInspectorTypeColorI\" ) )\n"
      "         %this.parentGroup.apply( ColorFloatToInt( %payload.color ) );\n"
      "      else if( %this.parentGroup.isMemberOfClass( \"GuiInspectorTypeColorF\" ) )\n"
      "         %this.parentGroup.apply( %payload.color );\n"
      "      else\n"
      "         %this.setColor( %payload.color );\n"
      "   }\n"
      "}\n"
   "@endtsexample\n\n"
   
   "@see GuiControl::onControlDragEnter\n"
   "@see GuiControl::onControlDragExit\n"
   "@see GuiControl::onControlDragged\n"
   "@see GuiControl::onControlDropped\n\n"
   
   "@ingroup GuiUtil"
);


//-----------------------------------------------------------------------------

void GuiDragAndDropControl::initPersistFields()
{
   addField( "deleteOnMouseUp", TypeBool, Offset( mDeleteOnMouseUp, GuiDragAndDropControl ),
      "If true, the control deletes itself when the left mouse button is released.\n\n"
      "If at this point, the drag&drop control still contains its payload, it will be deleted along with the control." );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

void GuiDragAndDropControl::startDragging( Point2I offset )
{
   GuiCanvas* canvas = getRoot();
   if( !canvas )
   {
      Con::errorf( "GuiDragAndDropControl::startDragging - GuiDragAndDropControl wasn't added to the gui before the drag started." );
      if( mDeleteOnMouseUp )
         deleteObject();
      return;
   }
   
   if( canvas->getMouseLockedControl() )
   {
      GuiEvent event;
      canvas->getMouseLockedControl()->onMouseLeave(event);
      canvas->mouseUnlock( canvas->getMouseLockedControl() );
   }
   canvas->mouseLock(this);
   canvas->setFirstResponder(this);
   mOffset = offset;
   mLastTarget=NULL;
}

//-----------------------------------------------------------------------------

void GuiDragAndDropControl::onMouseDown( const GuiEvent& event )
{
   startDragging( event.mousePoint - getPosition() );
}

//-----------------------------------------------------------------------------

void GuiDragAndDropControl::onMouseDragged( const GuiEvent& event )
{
   setPosition( event.mousePoint - mOffset );
   
   // Allow the control under the drag to react to a potential drop
   GuiControl* enterTarget = findDragTarget( event.mousePoint, "onControlDragEnter" );
   if( mLastTarget != enterTarget )
   {
      if( mLastTarget )
         mLastTarget->onControlDragExit_callback( dynamic_cast< GuiControl* >( at( 0 ) ), getDropPoint() );
      if( enterTarget )
         enterTarget->onControlDragEnter_callback( dynamic_cast< GuiControl* >( at( 0 ) ), getDropPoint() );

      mLastTarget = enterTarget;
   }

   GuiControl* dragTarget = findDragTarget( event.mousePoint, "onControlDragged" );
   if( dragTarget )
      dragTarget->onControlDragged_callback( dynamic_cast< GuiControl* >( at( 0 ) ), getDropPoint() );
}

//-----------------------------------------------------------------------------

void GuiDragAndDropControl::onMouseUp(const GuiEvent& event)
{
   mouseUnlock();

   GuiControl* target = findDragTarget( event.mousePoint, "onControlDropped" );
   if( target )
      target->onControlDropped_callback( dynamic_cast< GuiControl* >( at( 0 ) ), getDropPoint() );

   if( mDeleteOnMouseUp )
      deleteObject();
}

//-----------------------------------------------------------------------------

GuiControl* GuiDragAndDropControl::findDragTarget( Point2I mousePoint, const char* method )
{
   // If there are any children and we have a parent.
   GuiControl* parent = getParent();
   if (size() && parent)
   {
      mVisible = false;
      GuiControl* dropControl = parent->findHitControl(mousePoint);
      mVisible = true;
      while( dropControl )
      {      
         if (dropControl->isMethod(method))
            return dropControl;
         else
            dropControl = dropControl->getParent();
      }
   }
   return NULL;
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( GuiDragAndDropControl, startDragging, void, ( S32 x, S32 y ), ( 0, 0 ),
   "Start the drag operation.\n\n"
   "@param x X coordinate for the mouse pointer offset which the drag control should position itself.\n"
   "@param y Y coordinate for the mouse pointer offset which the drag control should position itself.")
{
   object->startDragging( Point2I( x, y ) );
}
