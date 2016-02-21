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

#include "gui/editor/inspector/variableInspector.h"
#include "gui/editor/inspector/variableGroup.h"
#include "console/engineAPI.h"

GuiVariableInspector::GuiVariableInspector()
{
}

GuiVariableInspector::~GuiVariableInspector()
{
}

IMPLEMENT_CONOBJECT(GuiVariableInspector);

ConsoleDocClass( GuiVariableInspector,
   "@brief GUI dedicated to variable viewing/manipulation\n\n"
   "Mostly used in console system, internal use only.\n\n"
   "@internal"
);

void GuiVariableInspector::loadVars( String searchStr )
{     
   clearGroups();

   GuiInspectorVariableGroup *group = new GuiInspectorVariableGroup();

   group->setHeaderHidden( true );
   group->setCanCollapse( false );
   group->mParent = this;
   group->setCaption( "Global Variables" );
   group->mSearchString = searchStr;

   group->registerObject();
   mGroups.push_back( group );
   addObject( group );
 
   //group->inspectGroup();
}

DefineConsoleMethod( GuiVariableInspector, loadVars, void, ( const char * searchString ), , "loadVars( searchString )" )
{
   object->loadVars( searchString );
}