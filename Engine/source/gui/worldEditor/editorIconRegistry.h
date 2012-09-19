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

#ifndef _EDITORICONREGISTRY_H_
#define _EDITORICONREGISTRY_H_

#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif
#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif 

class SimObject;
class AbstractClassRep;


/// This class is used to find the correct icon file 
/// path for different SimObject class types.  It is
/// typically used by the editors.
class EditorIconRegistry
{
public:

   EditorIconRegistry();
   ~EditorIconRegistry();

   /// Loops thru all the AbstractClassReps looking for icons in the path.
   void loadFromPath( const String &path, bool overwrite );

   /// Adds a single icon to the registry.
   void add( const String &className, const String &imageFile, bool overwrite );

   /// Clears all the icons from the registry.
   void clear();

   /// Looks up an icon given an AbstractClassRep.
   /// Other findIcon methods redirect to this.
   GFXTexHandle findIcon( AbstractClassRep *classRep );

   /// Looks up an icon given a SimObject.
   GFXTexHandle findIcon( const SimObject *object );   

   /// Looks up an icon given a className.
   GFXTexHandle findIcon( const char *className );

   /// Returns true if an icon is defined this object's class.
   /// Does not recurse up the class hierarchy.
   bool hasIconNoRecurse( const SimObject *object );

protected:
 
   typedef HashTable<StringNoCase,GFXTexHandle> IconMap;
   IconMap mIcons;

   /// The default icon returned when no matching icon is found.
   GFXTexHandle mDefaultIcon;
};

/// The global registry of editor icons.
extern EditorIconRegistry gEditorIcons;

#endif // _EDITORICONREGISTRY_H_