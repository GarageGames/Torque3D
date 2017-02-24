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

function getLoadFilename(%filespec, %callback, %currentFile, %getRelative, %defaultPath)
{
   //If no default path passed in then try to get one from the file
   if(%defaultPath $= "")
   {
      if ( filePath( %currentFile ) !$= "" )
         %defaultPath = filePath(%currentFile);
   }
   
   %dlg = new OpenFileDialog()
   {
      Filters = %filespec;
      DefaultFile = %currentFile;
      DefaultPath = %defaultPath;
      ChangePath = false;
      MustExist = true;
      MultipleFiles = false;
   };
   
   %ok = %dlg.Execute();
   if ( %ok )
   {
      %file = %dlg.FileName;
      if(%getRelative)
         %file = strreplace(%file,getWorkingDirectory() @ "/", "");
      eval(%callback @ "(\"" @ %file @ "\");");
      $Tools::FileDialogs::LastFilePath = filePath( %dlg.FileName );
   }
   
   %dlg.delete();
   
   return %ok;
}

// Opens a choose file dialog with format filters already loaded
// in. This avoids the issue of passing a massive list of format 
// filters into a function as an arguement.
function getLoadFormatFilename(%callback, %currentFile)
{   
   %dlg = new OpenFileDialog()
   {
      Filters = getFormatFilters() @ "(All Files (*.*)|*.*|";
      DefaultFile = %currentFile;
      ChangePath = false;
      MustExist = true;
      MultipleFiles = false;
   };
   
   if ( filePath( %currentFile ) !$= "" )
      %dlg.DefaultPath = filePath(%currentFile);  
      
   if ( %dlg.Execute() )
   {
      eval(%callback @ "(\"" @ %dlg.FileName @ "\");");
      $Tools::FileDialogs::LastFilePath = filePath( %dlg.FileName );
   }
   
   %dlg.delete();
}
