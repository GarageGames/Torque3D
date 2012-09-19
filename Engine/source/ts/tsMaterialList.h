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

#ifndef _TSMATERIALLIST_H_
#define _TSMATERIALLIST_H_

#ifndef _MATERIALLIST_H_
#include "materials/materialList.h"
#endif
#ifndef _PATH_H_
#include "core/util/path.h"
#endif


/// Specialized material list for 3space objects.
class TSMaterialList : public MaterialList
{
   typedef MaterialList Parent;

   Vector<U32> mFlags;
   Vector<U32> mReflectanceMaps;
   Vector<U32> mBumpMaps;
   Vector<U32> mDetailMaps;
   Vector<F32> mDetailScales;
   Vector<F32> mReflectionAmounts;

   bool mNamesTransformed;

   void allocate(U32 sz);

  public:

   enum
   {
      S_Wrap             = BIT(0),
      T_Wrap             = BIT(1),
      Translucent        = BIT(2),
      Additive           = BIT(3),
      Subtractive        = BIT(4),
      SelfIlluminating   = BIT(5),
      NeverEnvMap        = BIT(6),
      NoMipMap           = BIT(7),
      MipMap_ZeroBorder  = BIT(8),
      AuxiliaryMap       = BIT(27) | BIT(28) | BIT(29) | BIT(30) | BIT(31) // DEPRECATED
   };

   TSMaterialList(U32 materialCount, const char **materialNames, const U32 * materialFlags,
                  const U32 * reflectanceMaps, const U32 * bumpMaps, const U32 * detailMaps,
                  const F32 * detailScales, const F32 * reflectionAmounts);
   TSMaterialList();
   TSMaterialList(const TSMaterialList*);
   ~TSMaterialList();
   void free();

   U32 getFlags(U32 index);
   void setFlags(U32 index, U32 value);

   bool renameMaterial(U32 index, const String& newName); // use to support reskinning

   /// pre-load only
   void push_back(const String &name, U32 flags,
                  U32 a=0xFFFFFFFF, U32 b=0xFFFFFFFF, U32 c=0xFFFFFFFF,
                  F32 dm=1.0f, F32 em=1.0f);
   void push_back(const char * name, U32 flags, Material* mat);

   /// @name IO
   /// Functions for reading/writing to/from streams
   /// @{

   bool write(Stream &);
   bool read(Stream &);
   /// @}

protected:
   virtual void mapMaterial( U32 index );
};


#endif // _TSMATERIALLIST_H_
