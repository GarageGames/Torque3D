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

#ifndef _GUIIMAGELIST_H_
#define _GUIIMAGELIST_H_

#include "console/simDatablock.h"

#ifndef _GUITYPES_H_
#include "gui/core/guiTypes.h"
#endif


class GuiImageList : public SimObject
{
  private:
   typedef SimObject Parent;

  public:
   typedef struct tag_TextureEntry
   {
      StringTableEntry TexturePath;
      GFXTexHandle Handle;
      U32 id;
   }TextureEntry,*PTextureEntry;
   
   Vector<PTextureEntry> mTextures;

  protected:


    U32 mUniqueId;
	
  public:
   GuiImageList();
   
   DECLARE_CONOBJECT(GuiImageList);
   
   static void initPersistFields();

   // Image managing functions
   bool Clear();
   inline U32 Count() { return (U32)mTextures.size(); };
   U32 Insert( const char* texturePath , GFXTextureProfile *Type = &GFXDefaultGUIProfile  );

   bool FreeTextureEntry( U32 Index );
   bool FreeTextureEntry( PTextureEntry Entry );

   GFXTexHandle GetTextureHandle( U32 Index );
   GFXTexHandle GetTextureHandle( const char* TexturePath );

   const char * GetTexturePath( U32 Index );

   U32 IndexFromId ( U32 Id );
   U32 IndexFromPath ( const char* Path );

};

#endif //_GUIIMAGELISTCTRL_H_
