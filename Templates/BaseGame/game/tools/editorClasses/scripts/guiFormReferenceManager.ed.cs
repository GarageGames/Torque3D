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


//-----------------------------------------------------------------------------
// Add Content Reference to RefList
//
// Returns : True or False.
//-----------------------------------------------------------------------------
function GuiFormManager::AddContentReference( %library, %contentName, %control )
{
   // Fetch Content Object.
   %contentObj = GuiFormManager::FindFormContent( %library, %contentName );

   // See if we Found the Library.
   if( %contentObj == 0 || !isObject( %contentObj ) )
   {
      error( "GuiFormManager::AddContentReference - Unable to Find Library by Name or ID!" );
      return false;
   }

   // Validate Ref List.
   if( !isObject( %contentObj.RefList ) )
   {
      error( "GuiFormManager::AddContentReference - Unable to find content RefList!" );
      return false;
   }

   //error("adding ref for object" SPC %control );

   // Add Control Reference.
   %contentObj.RefList.add( %control );

   // Return Success.
   return true;
}

//-----------------------------------------------------------------------------
// Remove Content Reference from RefList
//
// Returns : True or False.
//-----------------------------------------------------------------------------
function GuiFormManager::RemoveContentReference( %library, %contentName, %control )
{
   // Fetch Content Object.
   %contentObj = GuiFormManager::FindFormContent( %library, %contentName );

   // See if we Found the Library.
   if( %contentObj == 0 || !isObject( %contentObj ) )
   {
      error( "GuiFormManager::AddContentReference - Unable to Find Library by Name or ID!" );
      return false;
   }

   // Validate Ref List.
   if( !isObject( %contentObj.RefList ) )
   {
      error( "GuiFormManager::AddContentReference - Unable to find content RefList!" );
      return false;
   }

   //error("removing ref for object" SPC %control );

   // Add Control Reference.
   %contentObj.RefList.remove( %control );
   
   if( %control.isMethod("onFormRemove") )
      %control.onFormRemove();

   // Return Success.
   return true;
}

//-----------------------------------------------------------------------------
// Gets the current number of instances of the specified content that are active
//
// Returns : Number of instances or 0.
//-----------------------------------------------------------------------------
function GuiFormManager::GetContentCount( %library, %contentName )
{
   // Fetch Content Object.
   %contentObj = GuiFormManager::FindFormContent( %library, %contentName );

   // See if we Found the Library.
   if( %contentObj == 0 || !isObject( %contentObj ) )
   {
      error( "GuiFormManager::GetContentCount - Unable to Find Library by Name or ID!" );
      return 0;
   }

   // Validate Ref List.
   if( !isObject( %contentObj.RefList ) )
   {
      error( "GuiFormManager::GetContentCount - Unable to find content RefList!" );
      return 0;
   }

   // Return Count.
   return %contentObj.RefList.getCount();
}
