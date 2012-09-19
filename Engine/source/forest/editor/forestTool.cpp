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

#include "platform/platform.h"
#include "forest/editor/forestTool.h"
#include "forest/editor/forestEditorCtrl.h"

#include "forest/forestDataFile.h"
#include "forest/forestCell.h"

#include "util/undo.h"
#include "math/mMath.h"
#include "math/mathUtils.h"


IMPLEMENT_CONOBJECT( ForestTool );

ConsoleDocClass( ForestTool,
   "@brief Base class for Forest Editor specific tools\n\n"
   "Editor use only.\n\n"
   "@internal"
);

ForestTool::ForestTool()
   : mForest( NULL ),
     mEditor( NULL )
{   
}

ForestTool::~ForestTool()
{
}

void ForestTool::_submitUndo( UndoAction *action )
{
   AssertFatal( action, "ForestTool::_submitUndo() - No undo action!" );

   // Grab the mission editor undo manager.
   UndoManager *undoMan = NULL;
   if ( !Sim::findObject( "EUndoManager", undoMan ) )
   {
      Con::errorf( "ForestTool::_submitUndo() - EUndoManager not found!" );
      return;     
   }

   undoMan->addAction( action );

   mEditor->updateCollision();
}
