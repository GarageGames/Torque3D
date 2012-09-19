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

#ifndef _MATERIALLIST_H_
#define _MATERIALLIST_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif


class Material;
class BaseMatInstance;
class Stream;
class GFXVertexFormat;
class FeatureSet;


class MaterialList
{
public:
   MaterialList();
   MaterialList(U32 materialCount, const char **materialNames);
   virtual ~MaterialList();

   /// Note: this is not to be confused with MaterialList(const MaterialList&).  Copying
   ///  a material list in the middle of it's lifetime is not a good thing, so we force
   ///  it to copy at construction time by restricting the copy syntax to
   ///  ML* pML = new ML(&copy);
   explicit MaterialList(const MaterialList*);

   const Vector<String> &getMaterialNameList() const { return mMaterialNames; }
   const String &getMaterialName(U32 index) const { return mMaterialNames[index]; }
   GFXTextureObject *getDiffuseTexture(U32 index);

   void setTextureLookupPath(const String& path) { mLookupPath = path; }
   void setMaterialName(U32 index, const String& name);

   void set(U32 materialCount, const char **materialNames);
   U32  push_back(const String &filename, Material* = 0);

   virtual void free();
   void clearMatInstList();

   bool empty() const { return mMaterialNames.empty();   }
   U32  size() const { return (U32)mMaterialNames.size(); }

   bool read(Stream &stream);
   bool write(Stream &stream);

   bool readText(Stream &stream, U8 firstByte);
   bool readText(Stream &stream);
   bool writeText(Stream &stream);

   void mapMaterials();

   /// Initialize material instances in material list.
   void initMatInstances(  const FeatureSet &features, 
                           const GFXVertexFormat *vertexFormat );

   /// Return the material instance or NULL if the 
   /// index is out of bounds.
   inline BaseMatInstance* getMaterialInst( U32 index ) const
   {
      return index < mMatInstList.size() ? mMatInstList[index] : NULL;
   }

   void setMaterialInst( BaseMatInstance *matInst, U32 index );

   // Needs to be accessible if were going to freely edit instances
   Vector<BaseMatInstance*> mMatInstList;

protected:

   String               mLookupPath;
   Vector<String>       mMaterialNames;            //!< Material 'mapTo' targets

   virtual void mapMaterial( U32 index );

private:
   enum Constants { BINARY_FILE_VERSION = 1 };
};

#endif // _MATERIALLIST_H_
