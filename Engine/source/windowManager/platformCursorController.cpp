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
#include "windowManager/platformCursorController.h"

void PlatformCursorController::pushCursor( S32 cursorID )
{
   // Place the new cursor shape onto the stack
   mCursors.increment();

   Cursor_Shape &shape = mCursors.last();
   shape.mCursorType  = Cursor_Shape::TYPE_RESOURCE;
   shape.mCursorID    = cursorID;

   // Now Change the Cursor Shape.
   setCursorShape( shape.mCursorID );
}

void PlatformCursorController::pushCursor( const UTF8 *fileName )
{
   // Place the new cursor shape onto the stack
   mCursors.increment();

   // Store the Details.
   Cursor_Shape &shape = mCursors.last();
   shape.mCursorType  = Cursor_Shape::TYPE_FILE;
   shape.mCursorFile  = String::ToString( "%s", fileName );

   // Now Change the Cursor Shape.
   setCursorShape( shape.mCursorFile.c_str(), true );
}

void PlatformCursorController::popCursor()
{
   // Before poping the stack, make sure we're not trying to remove the last cursor shape
   if ( mCursors.size() <= 1 )
   {
      return;
   }

   // Clear the Last Cursor.
   mCursors.pop_back();

   // Now Change the Cursor Shape.
   setCursorShape( mCursors.last(), true );
}

void PlatformCursorController::refreshCursor()
{
   // Refresh the Cursor Shape.
   setCursorShape( mCursors.last(), false );
}

void PlatformCursorController::setCursorShape( const Cursor_Shape &shape, bool reload )
{
    switch( shape.mCursorType )
    {
        case Cursor_Shape::TYPE_RESOURCE :
            {

                // Set Resource.
                setCursorShape( shape.mCursorID );

            } break;

        case Cursor_Shape::TYPE_FILE :
            {

                // Set File.
                setCursorShape( shape.mCursorFile.c_str(), reload );

            } break;
    }
}
