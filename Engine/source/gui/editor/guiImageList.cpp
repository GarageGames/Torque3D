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
#include "console/consoleTypes.h"
#include "console/console.h"
#include "gfx/gfxDevice.h"
#include "gui/editor/guiImageList.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiImageList);

ConsoleDocClass( GuiImageList,
   "@brief GUI control which displays a list of images.\n\n"
   "Used to be a part of an old editor system for previous Torque systems. "
   "Doesn't appear to be used anymore, will most likely be deprecated.\n\n"
   "@ingroup GuiCore\n"
   "@internal");

GuiImageList::GuiImageList()
{
  VECTOR_SET_ASSOCIATION(mTextures);
  mTextures.clear();
  mUniqueId = 0;
}

U32 GuiImageList::Insert( const char* texturePath, GFXTextureProfile *Type )
{
  TextureEntry *t = new TextureEntry;

  t->TexturePath = StringTable->insert(texturePath);
  if ( *t->TexturePath ) 
  {
    t->Handle = GFXTexHandle(t->TexturePath, Type, avar("%s() - t->Handle (line %d)", __FUNCTION__, __LINE__));

    if ( t->Handle )
    {
      t->id = ++mUniqueId;

      mTextures.push_back( t );

      return t->id;

    }
  }

  // Free Texture Entry.
  delete t;

  // Return Failure.
  return -1;

}

bool GuiImageList::Clear()
{
  while ( mTextures.size() )
    FreeTextureEntry( mTextures[0] );
  mTextures.clear();

  mUniqueId = 0;

  return true;
}

bool GuiImageList::FreeTextureEntry( U32 Index )
{
  U32 Id = IndexFromId( Index );
  if ( Id != -1 )
     return FreeTextureEntry( mTextures[ Id ] );
  else
    return false;
}

bool GuiImageList::FreeTextureEntry( PTextureEntry Entry )
{
  if ( ! Entry )
    return false;

  U32 id = IndexFromId( Entry->id );

  delete Entry;

  mTextures.erase ( id );

  return true;
}

U32 GuiImageList::IndexFromId ( U32 Id )
{
  if ( !mTextures.size() ) return -1;
  Vector<PTextureEntry>::iterator i = mTextures.begin();
  U32 j = 0;
  for ( ; i != mTextures.end(); i++ )
  {
    if ( i )
    {
    if ( (*i)->id == Id )
      return j;
    j++;
    }
  }

  return -1;
}

U32 GuiImageList::IndexFromPath ( const char* Path )
{
  if ( !mTextures.size() ) return -1;
  Vector<PTextureEntry>::iterator i = mTextures.begin();
  for ( ; i != mTextures.end(); i++ )
  {
    if ( dStricmp( Path, (*i)->TexturePath ) == 0 )
      return (*i)->id;
  }

  return -1;
}

void GuiImageList::initPersistFields()
{
  Parent::initPersistFields();
}

DefineEngineMethod( GuiImageList, getImage, const char*, (int index),,
   "@brief Get a path to the texture at the specified index.\n\n"
   "@param index Index of the image in the list.\n"
   "@tsexample\n"
   "// Define the image index/n"
   "%index = \"5\";\n\n"
   "// Request the image path location from the control.\n"
   "%imagePath = %thisGuiImageList.getImage(%index);\n"
   "@endtsexample\n\n"
   "@return File path to the image map for the specified index.\n\n"
   "@see SimObject")
{
  return object->GetTexturePath(index);
}

DefineEngineMethod(GuiImageList, clear, bool, (),,
   "@brief Clears the imagelist\n\n"
   "@tsexample\n"
   "// Inform the GuiImageList control to clear itself.\n"
   "%isFinished = %thisGuiImageList.clear();\n"
   "@endtsexample\n\n"
   "@return Returns true when finished.\n\n"
   "@see SimObject")
{
  return object->Clear();
}

DefineEngineMethod( GuiImageList, count, S32, (),,
   "@brief Gets the number of images in the list.\n\n"
   "@tsexample\n"
   "// Request the number of images from the GuiImageList control.\n"
   "%imageCount = %thisGuiImageList.count();\n"
   "@endtsexample\n\n"
   "@return Number of images in the control.\n\n"
   "@see SimObject")
{
  return object->Count();
}

DefineEngineMethod( GuiImageList, remove, bool, (S32 index),,
   "@brief Removes an image from the list by index.\n\n"
   "@param index Image index to remove.\n"
   "@tsexample\n"
   "// Define the image index.\n"
   "%imageIndex = \"4\";\n\n"
   "// Inform the GuiImageList control to remove the image at the defined index.\n"
   "%wasSuccessful = %thisGuiImageList.remove(%imageIndex);\n"
   "@endtsexample\n\n"
   "@return True if the operation was successful, false if it was not.\n\n"
   "@see SimObject")
{
  return object->FreeTextureEntry( index );
}

DefineEngineMethod( GuiImageList, getIndex, S32, (const char* imagePath),,
   "@brief Retrieves the imageindex of a specified texture in the list.\n\n"
   "@param imagePath Imagemap including filepath of image to search for\n"
   "@tsexample\n"
   "// Define the imagemap to search for\n"
   "%imagePath = \"./game/client/data/images/thisImage\";\n\n"
   "// Request the index entry for the defined imagemap\n"
   "%imageIndex = %thisGuiImageList.getIndex(%imagePath);\n"
   "@endtsexample\n\n"
   "@return Index of the imagemap matching the defined image path.\n\n"
   "@see SimObject")
{
  return object->IndexFromPath( imagePath );
}

DefineEngineMethod(GuiImageList, insert, S32, (const char* imagePath),,
   "@brief Insert an image into imagelist- returns the image index or -1 for failure.\n\n"
   "@param imagePath Imagemap, with path, to add to the list.\n"
   "@tsexample\n"
   "// Define the imagemap to add to the list\n"
   "%imagePath = \"./game/client/data/images/thisImage\";\n\n"
   "// Request the GuiImageList control to add the defined image to its list.\n"
   "%imageIndex = %thisGuiImageList.insert(%imagePath);\n"
   "@endtsexample\n\n"
   "@return The index of the newly inserted imagemap, or -1 if the insertion failed.\n\n"
   "@see SimObject")
{
  return object->Insert( imagePath );
}

GFXTexHandle GuiImageList::GetTextureHandle( U32 Index )
{
  U32 ItemIndex = IndexFromId(Index);
  if ( ItemIndex != -1 )
    return mTextures[ItemIndex]->Handle;
  else
    return NULL;

}
GFXTexHandle GuiImageList::GetTextureHandle( const char* TexturePath )
{
  Vector<PTextureEntry>::iterator i = mTextures.begin();
  for ( ; i != mTextures.end(); i++ )
  {
    if ( dStricmp( TexturePath, (*i)->TexturePath ) == 0 )
      return (*i)->Handle;
  }

  return NULL;
}


const char *GuiImageList::GetTexturePath( U32 Index )
{
  U32 ItemIndex = IndexFromId(Index);
  if ( ItemIndex != -1 )
    return mTextures[ItemIndex]->TexturePath;
  else
    return "";
}
