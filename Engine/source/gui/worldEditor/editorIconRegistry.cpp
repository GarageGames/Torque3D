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
#include "gui/worldEditor/editorIconRegistry.h"

#include "console/console.h"
#include "console/simBase.h"


EditorIconRegistry gEditorIcons;

ConsoleDoc(
   "@class EditorIconRegistry\n"
   "@brief This class is used to find the correct icon file path for different SimObject class types.\n\n"
   "It is typically used by the editors, not intended for actual game development\n\n"
   "@internal"
);

EditorIconRegistry::EditorIconRegistry()
{
}

EditorIconRegistry::~EditorIconRegistry()
{
   clear();
}

void EditorIconRegistry::loadFromPath( const String &path, bool overwrite )
{
   AbstractClassRep *classRep = AbstractClassRep::getClassList();
   while ( classRep )
   {
      String iconFile = String::ToString( "%s%s", path.c_str(), classRep->getClassName() );
      add( classRep->getClassName(), iconFile.c_str(), overwrite );
      classRep = classRep->getNextClass();
   }

   String defaultIconFile = path + "default";

   mDefaultIcon.set( defaultIconFile,
                     &GFXDefaultPersistentProfile, 
                     avar("%s() - mIcons[] (line %d)", 
                     __FUNCTION__, __LINE__) );
}

void EditorIconRegistry::add( const String &className, const String &imageFile, bool overwrite )
{
   // First see if we can load the image.
   GFXTexHandle icon(   imageFile, &GFXDefaultPersistentProfile, 
                        avar("%s() - mIcons[] (line %d)", __FUNCTION__, __LINE__) );
   if ( icon.isNull() )
      return;

   // Look it up in the map.
   StringNoCase key( className );
   IconMap::Iterator iter = mIcons.find( key );
   if ( iter != mIcons.end() )
   {
      if ( !overwrite )
         return;

      iter->value = icon;
   }
   else
      mIcons.insertUnique( key, icon );
}

GFXTexHandle EditorIconRegistry::findIcon( AbstractClassRep *classRep )
{
   while ( classRep )
   {
      StringNoCase key( classRep->getClassName() );
      IconMap::Iterator icon = mIcons.find( key );

      if ( icon != mIcons.end() && icon->value.isValid() )
         return icon->value;

      classRep = classRep->getParentClass();
   }

   return mDefaultIcon;
}

GFXTexHandle EditorIconRegistry::findIcon( const SimObject *object )
{
	if( object == NULL )
		return mDefaultIcon;

   AbstractClassRep *classRep = object->getClassRep();

   return findIcon( classRep );
}   

GFXTexHandle EditorIconRegistry::findIcon( const char *className )
{   
   // On the chance we have this className already in the map,
   // check there first because its a lot faster...
   
   StringNoCase key( className );
   IconMap::Iterator icon = mIcons.find( key );

   if ( icon != mIcons.end() && icon->value.isValid() )
      return icon->value;

   // Well, we could still have an icon for a parent class,
   // so find the AbstractClassRep for the className.
   //
   // Unfortunately the only way to do this is looping through
   // the AbstractClassRep linked list.

   bool found = false;
   AbstractClassRep* pClassRep = AbstractClassRep::getClassList();
   
   while ( pClassRep )
   {
      if ( key.equal( pClassRep->getClassName(), String::NoCase ) )
      {
         found = true;
         break;
      }
      pClassRep = pClassRep->getNextClass();
   }

   if ( !found )
   {
      Con::errorf( "EditorIconRegistry::findIcon, passed className %s was not an AbstractClassRep!", key.c_str() );
      return mDefaultIcon;
   }
   
   // Now do a find by AbstractClassRep recursively up the class tree...
   return findIcon( pClassRep );
}

bool EditorIconRegistry::hasIconNoRecurse( const SimObject *object )
{
   AbstractClassRep *classRep = object->getClassRep();
      
   StringNoCase key( classRep->getClassName() );

   IconMap::Iterator icon = mIcons.find( key );

   return icon != mIcons.end();
}

void EditorIconRegistry::clear()
{
   mIcons.clear();
   mDefaultIcon.free();
}

ConsoleStaticMethod( EditorIconRegistry, add, void, 3, 4, "( String className, String imageFile [, bool overwrite = true] )"
					"@internal")
{
   bool overwrite = true;
   if ( argc > 3 )
      overwrite = dAtob( argv[3] );

   gEditorIcons.add( argv[1], argv[2], overwrite );
}

ConsoleStaticMethod( EditorIconRegistry, loadFromPath, void, 2, 3, "( String imagePath [, bool overwrite = true] )"
					"@internal")
{
   bool overwrite = true;
   if ( argc > 2 )
      overwrite = dAtob( argv[2] );

   gEditorIcons.loadFromPath( argv[1], overwrite );
}

ConsoleStaticMethod( EditorIconRegistry, clear, void, 1, 1, "" 
					"@internal")
{
   gEditorIcons.clear();
}

ConsoleStaticMethod( EditorIconRegistry, findIconByClassName, const char*, 2, 2, "( String className )\n"
   "Returns the file path to the icon file if found." 
   "@internal")
{
   GFXTexHandle icon = gEditorIcons.findIcon( argv[1] );
   if ( icon.isNull() )
      return NULL;

   return icon->mPath;
}

ConsoleStaticMethod( EditorIconRegistry, findIconBySimObject, const char*, 2, 2, "( SimObject )\n"
   "Returns the file path to the icon file if found." 
   "@internal")
{
   SimObject *obj = NULL;
   if ( !Sim::findObject( argv[1], obj ) )
   {
      Con::warnf( "EditorIconRegistry::findIcon, parameter %d was not a SimObject!", argv[1] );
      return NULL;
   }

   GFXTexHandle icon = gEditorIcons.findIcon( obj );
   if ( icon.isNull() )
      return NULL;

   return icon->mPath;
}

