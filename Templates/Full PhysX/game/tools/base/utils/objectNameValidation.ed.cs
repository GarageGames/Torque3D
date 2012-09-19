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


function Editor::validateObjectName( %name, %mustHaveName )
{
   if( %mustHaveName && %name $= "" )
   {
      MessageBoxOK( "Missing Object Name", "No name given for object.  Please enter a valid object name." );
      return false;
   }
   if( !isValidObjectName( %name ) )
   {
      MessageBoxOK( "Invalid Object Name", "'" @ %name @ "' is not a valid object name." NL
         "" NL
         "Please choose a name that begins with a letter or underscore and is otherwise comprised " @
         "exclusively of letters, digits, and/or underscores."
      );
      return false;
   }
   if( isObject( %name ) )
   {
      %filename = %name.getFilename();
      if ( %filename $= "" )
         %filename = "an unknown file";

      MessageBoxOK( "Invalid Object Name", "Object names must be unique, and there is an " @
         "existing " @ %name.getClassName() @ " object with the name '" @ %name @ "' (defined " @
         "in " @ %filename @ ").  Please choose another name." );
      return false;
   }
   if( isClass( %name ) )
   {
      MessageBoxOK( "Invalid Object Name", "'" @ %name @ "' is the name of an existing TorqueScript " @
         "class.  Please choose another name." );
      return false;
   }

   return true;
}
