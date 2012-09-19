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
#include "ts/tsMaterialList.h"

#include "ts/tsShape.h"
#include "materials/matInstance.h"
#include "materials/materialManager.h"


TSMaterialList::TSMaterialList(U32 materialCount,
                               const char **materialNames,
                               const U32 * materialFlags,
                               const U32 * reflectanceMaps,
                               const U32 * bumpMaps,
                               const U32 * detailMaps,
                               const F32 * detailScales,
                               const F32 * reflectionAmounts)
 : MaterialList(materialCount,materialNames),
   mNamesTransformed(false)
{
   VECTOR_SET_ASSOCIATION(mFlags);
   VECTOR_SET_ASSOCIATION(mReflectanceMaps);
   VECTOR_SET_ASSOCIATION(mBumpMaps);
   VECTOR_SET_ASSOCIATION(mDetailMaps);
   VECTOR_SET_ASSOCIATION(mDetailScales);
   VECTOR_SET_ASSOCIATION(mReflectionAmounts);

   allocate(materialCount);

   dMemcpy(mFlags.address(),materialFlags,materialCount*sizeof(U32));
   dMemcpy(mReflectanceMaps.address(),reflectanceMaps,materialCount*sizeof(U32));
   dMemcpy(mBumpMaps.address(),bumpMaps,materialCount*sizeof(U32));
   dMemcpy(mDetailMaps.address(),detailMaps,materialCount*sizeof(U32));
   dMemcpy(mDetailScales.address(),detailScales,materialCount*sizeof(F32));
   dMemcpy(mReflectionAmounts.address(),reflectionAmounts,materialCount*sizeof(F32));
}

TSMaterialList::TSMaterialList()
   : mNamesTransformed(false)
{
   VECTOR_SET_ASSOCIATION(mFlags);
   VECTOR_SET_ASSOCIATION(mReflectanceMaps);
   VECTOR_SET_ASSOCIATION(mBumpMaps);
   VECTOR_SET_ASSOCIATION(mDetailMaps);
   VECTOR_SET_ASSOCIATION(mDetailScales);
   VECTOR_SET_ASSOCIATION(mReflectionAmounts);
}

TSMaterialList::TSMaterialList(const TSMaterialList* pCopy)
   : MaterialList(pCopy)
{
   VECTOR_SET_ASSOCIATION(mFlags);
   VECTOR_SET_ASSOCIATION(mReflectanceMaps);
   VECTOR_SET_ASSOCIATION(mBumpMaps);
   VECTOR_SET_ASSOCIATION(mDetailMaps);
   VECTOR_SET_ASSOCIATION(mDetailScales);
   VECTOR_SET_ASSOCIATION(mReflectionAmounts);

   mFlags             = pCopy->mFlags;
   mReflectanceMaps   = pCopy->mReflectanceMaps;
   mBumpMaps          = pCopy->mBumpMaps;
   mDetailMaps        = pCopy->mDetailMaps;
   mDetailScales      = pCopy->mDetailScales;
   mReflectionAmounts = pCopy->mReflectionAmounts;
   mNamesTransformed  = pCopy->mNamesTransformed;
}

TSMaterialList::~TSMaterialList()
{
   free();
}

void TSMaterialList::free()
{
   // these aren't found on our parent, clear them out here to keep in synch
   mFlags.clear();
   mReflectanceMaps.clear();
   mBumpMaps.clear();
   mDetailMaps.clear();
   mDetailScales.clear();
   mReflectionAmounts.clear();

   Parent::free();
}

void TSMaterialList::push_back(const String &name, U32 flags, U32 rMap, U32 bMap, U32 dMap, F32 dScale, F32 emapAmount)
{
   Parent::push_back(name);
   mFlags.push_back(flags);
   if (rMap==0xFFFFFFFF)
      mReflectanceMaps.push_back(size()-1);
   else
      mReflectanceMaps.push_back(rMap);
   mBumpMaps.push_back(bMap);
   mDetailMaps.push_back(dMap);
   mDetailScales.push_back(dScale);
   mReflectionAmounts.push_back(emapAmount);
}

void TSMaterialList::push_back(const char * name, U32 flags, Material* mat)
{
   Parent::push_back(name, mat);
   mFlags.push_back(flags);
   mReflectanceMaps.push_back(size()-1);
   mBumpMaps.push_back(0xFFFFFFFF);
   mDetailMaps.push_back(0xFFFFFFFF);
   mDetailScales.push_back(1.0f);
   mReflectionAmounts.push_back(1.0f);
}

void TSMaterialList::allocate(U32 sz)
{
   mFlags.setSize(sz);
   mReflectanceMaps.setSize(sz);
   mBumpMaps.setSize(sz);
   mDetailMaps.setSize(sz);
   mDetailScales.setSize(sz);
   mReflectionAmounts.setSize(sz);
}

U32 TSMaterialList::getFlags(U32 index)
{
   AssertFatal(index < size(),"TSMaterialList::getFlags: index out of range");
   return mFlags[index];
}

void TSMaterialList::setFlags(U32 index, U32 value)
{
   AssertFatal(index < size(),"TSMaterialList::getFlags: index out of range");
   mFlags[index] = value;
}

bool TSMaterialList::write(Stream & s)
{
   if (!Parent::write(s))
      return false;

   U32 i;
   for (i=0; i<size(); i++)
      s.write(mFlags[i]);

   for (i=0; i<size(); i++)
      s.write(mReflectanceMaps[i]);

   for (i=0; i<size(); i++)
      s.write(mBumpMaps[i]);

   for (i=0; i<size(); i++)
      s.write(mDetailMaps[i]);

   // MDF - This used to write mLightmaps
   // We never ended up using it but it is
   // still part of the version 25 standard
   if (TSShape::smVersion == 25)
   {
      for (i=0; i<size(); i++)
         s.write(0xFFFFFFFF);
   }
      
   for (i=0; i<size(); i++)
      s.write(mDetailScales[i]);

   for (i=0; i<size(); i++)
      s.write(mReflectionAmounts[i]);

   return (s.getStatus() == Stream::Ok);
}

bool TSMaterialList::read(Stream & s)
{
   if (!Parent::read(s))
      return false;

   allocate(size());

   U32 i;
   if (TSShape::smReadVersion < 2)
   {
      for (i=0; i<size(); i++)
         setFlags(i,S_Wrap|T_Wrap);
   }
   else
   {
      for (i=0; i<size(); i++)
         s.read(&mFlags[i]);
   }

   if (TSShape::smReadVersion < 5)
   {
      for (i=0; i<size(); i++)
      {
         mReflectanceMaps[i] = i;
         mBumpMaps[i] = 0xFFFFFFFF;
         mDetailMaps[i] = 0xFFFFFFFF;
      }
   }
   else
   {
      for (i=0; i<size(); i++)
         s.read(&mReflectanceMaps[i]);
      for (i=0; i<size(); i++)
         s.read(&mBumpMaps[i]);
      for (i=0; i<size(); i++)
         s.read(&mDetailMaps[i]);

      if (TSShape::smReadVersion == 25)
      {
         U32 dummy = 0;

         for (i=0; i<size(); i++)
            s.read(&dummy);
      }
   }

   if (TSShape::smReadVersion > 11)
   {
      for (i=0; i<size(); i++)
         s.read(&mDetailScales[i]);
   }
   else
   {
      for (i=0; i<size(); i++)
         mDetailScales[i] = 1.0f;
   }

   if (TSShape::smReadVersion > 20)
   {
      for (i=0; i<size(); i++)
         s.read(&mReflectionAmounts[i]);
   }
   else
   {
      for (i=0; i<size(); i++)
         mReflectionAmounts[i] = 1.0f;
   }

   if (TSShape::smReadVersion < 16)
   {
      // make sure emapping is off for translucent materials on old shapes
      for (i=0; i<size(); i++)
         if (mFlags[i] & TSMaterialList::Translucent)
            mFlags[i] |= TSMaterialList::NeverEnvMap;
   }

   return (s.getStatus() == Stream::Ok);
}

//--------------------------------------------------------------------------
// Sets the specified material in the list to the specified texture.  also 
// remaps mat instances based on the new texture name.  Returns false if 
// the specified texture is not valid.
//--------------------------------------------------------------------------
bool TSMaterialList::renameMaterial(U32 i, const String& newName)
{
   if (i > size() || newName.isEmpty())
      return false;

   // Check if already using this name
   if (newName.equal(mMaterialNames[i], String::NoCase))
   {
      // same material, return true since we aren't changing it
      return true;
   }

   // Allow the rename if the new name is mapped, or if there is a diffuse texture
   // available (for which a simple Material can be generated in mapMaterial)
   String mappedName = MATMGR->getMapEntry(newName);
   if (mappedName.isEmpty())
   {
      GFXTexHandle texHandle;
      if (mLookupPath.isEmpty())
      {
         texHandle.set( newName, &GFXDefaultStaticDiffuseProfile, avar("%s() - handle (line %d)", __FUNCTION__, __LINE__) );
      }
      else
      {
         String fullPath = String::ToString( "%s/%s", mLookupPath.c_str(), newName.c_str() );
         texHandle.set( fullPath, &GFXDefaultStaticDiffuseProfile, avar("%s() - handle (line %d)", __FUNCTION__, __LINE__) );
      }
      if (!texHandle.isValid())
         return false;
   }

   // change material name
   mMaterialNames[i] = newName;

   // Dump the old mat instance and remap the material.
   if( mMatInstList[ i ] )
      SAFE_DELETE( mMatInstList[ i ] );
   mapMaterial( i );

   return true;
}

void TSMaterialList::mapMaterial( U32 i )
{
   Parent::mapMaterial( i );

   BaseMatInstance* matInst = mMatInstList[i];
   if (matInst && matInst->getMaterial()->isTranslucent())
      mFlags[i] |= Translucent;
}
