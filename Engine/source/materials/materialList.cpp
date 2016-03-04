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
#include "materials/materialList.h"

#include "materials/matInstance.h"
#include "materials/materialManager.h"
#include "materials/materialFeatureTypes.h"
#include "materials/processedMaterial.h"
#include "core/volume.h"
#include "console/simSet.h"


MaterialList::MaterialList()
{
   VECTOR_SET_ASSOCIATION(mMatInstList);
   VECTOR_SET_ASSOCIATION(mMaterialNames);
}

MaterialList::MaterialList(const MaterialList* pCopy)
{
   VECTOR_SET_ASSOCIATION(mMatInstList);
   VECTOR_SET_ASSOCIATION(mMaterialNames);

   mMaterialNames.setSize(pCopy->mMaterialNames.size());
   S32 i;
   for (i = 0; i < mMaterialNames.size(); i++)
   {
      mMaterialNames[i] = pCopy->mMaterialNames[i];
   }

   clearMatInstList();
   mMatInstList.setSize(pCopy->size());
   for( i = 0; i < mMatInstList.size(); i++ )
   {
      if( i < pCopy->mMatInstList.size() && pCopy->mMatInstList[i] )
      {
         mMatInstList[i] = pCopy->mMatInstList[i]->getMaterial()->createMatInstance();
      }
      else
      {
         mMatInstList[i] = NULL;
      }
   }
}



MaterialList::MaterialList(U32 materialCount, const char **materialNames)
{
   VECTOR_SET_ASSOCIATION(mMaterialNames);

   set(materialCount, materialNames);
}


//--------------------------------------
void MaterialList::set(U32 materialCount, const char **materialNames)
{
   free();
   mMaterialNames.setSize(materialCount);
   clearMatInstList();
   mMatInstList.setSize(materialCount);
   for(U32 i = 0; i < materialCount; i++)
   {
      mMaterialNames[i] = materialNames[i];
      mMatInstList[i] = NULL;
   }
}


//--------------------------------------
MaterialList::~MaterialList()
{
   free();
}

//--------------------------------------
void MaterialList::setMaterialName(U32 index, const String& name)
{
   if (index < mMaterialNames.size())
      mMaterialNames[index] = name;
}

//--------------------------------------
GFXTextureObject *MaterialList::getDiffuseTexture(U32 index)
{
   AssertFatal(index < (U32) mMatInstList.size(), "MaterialList::getDiffuseTex: index lookup out of range.");

   MatInstance *matInst = dynamic_cast<MatInstance*>(mMatInstList[index]);
   if (matInst && matInst->getProcessedMaterial())
      return matInst->getProcessedMaterial()->getStageTexture(0, MFT_DiffuseMap);
   else
      return NULL;
}

//--------------------------------------
void MaterialList::free()
{
   clearMatInstList();
   mMatInstList.clear();
   mMaterialNames.clear();
}

/*
//--------------------------------------
U32 MaterialList::push_back(GFXTexHandle textureHandle, const String &filename)
{
   mMaterialNames.push_back(filename);
   mMatInstList.push_back(NULL);

   // return the index
   return mMaterialNames.size()-1;
}
*/

//--------------------------------------
U32 MaterialList::push_back(const String &filename, Material* material)
{
   mMaterialNames.push_back(filename);
   mMatInstList.push_back(material ? material->createMatInstance() : NULL);

   // return the index
   return mMaterialNames.size()-1;
}

//--------------------------------------
bool MaterialList::read(Stream &stream)
{
   free();

   // check the stream version
   U8 version;
   if ( stream.read(&version) && version != BINARY_FILE_VERSION)
      return readText(stream,version);

   // how many materials?
   U32 count;
   if ( !stream.read(&count) )
      return false;

   // pre-size the vectors for efficiency
   mMaterialNames.reserve(count);

   // read in the materials
   for (U32 i=0; i<count; i++)
   {
      // Load the bitmap name
      char buffer[256];
      stream.readString(buffer);
      if( !buffer[0] )
      {
         AssertWarn(0, "MaterialList::read: error reading stream");
         return false;
      }

      // Material paths are a legacy of Tribes tools,
      // strip them off...
      char *name = &buffer[dStrlen(buffer)];
      while (name != buffer && name[-1] != '/' && name[-1] != '\\')
         name--;

      // Add it to the list
      mMaterialNames.push_back(name);
      mMatInstList.push_back(NULL);
   }

   return (stream.getStatus() == Stream::Ok);
}

//--------------------------------------
bool MaterialList::write(Stream &stream)
{
   stream.write((U8)BINARY_FILE_VERSION);          // version
   stream.write((U32)mMaterialNames.size());       // material count

   for(S32 i=0; i < mMaterialNames.size(); i++)    // material names
      stream.writeString(mMaterialNames[i]);

   return (stream.getStatus() == Stream::Ok);
}

//--------------------------------------
bool MaterialList::readText(Stream &stream, U8 firstByte)
{
   free();

   if (!firstByte)
      return (stream.getStatus() == Stream::Ok || stream.getStatus() == Stream::EOS);

   char buf[1024];
   buf[0] = firstByte;
   U32 offset = 1;

   for(;;)
   {
      stream.readLine((U8*)(buf+offset), sizeof(buf)-offset);
      if(!buf[0])
         break;
      offset = 0;

      // Material paths are a legacy of Tribes tools,
      // strip them off...
      char *name = &buf[dStrlen(buf)];
      while (name != buf && name[-1] != '/' && name[-1] != '\\')
         name--;

      // Add it to the list
      mMaterialNames.push_back(name);
      mMatInstList.push_back(NULL);
   }

   return (stream.getStatus() == Stream::Ok || stream.getStatus() == Stream::EOS);
}

bool MaterialList::readText(Stream &stream)
{
   U8 firstByte;
   stream.read(&firstByte);
   return readText(stream,firstByte);
}

//--------------------------------------
bool MaterialList::writeText(Stream &stream)
{
   for(S32 i=0; i < mMaterialNames.size(); i++)
      stream.writeLine((U8*)(mMaterialNames[i].c_str()));
   stream.writeLine((U8*)"");

   return (stream.getStatus() == Stream::Ok);
}

//--------------------------------------------------------------------------
// Clear all materials in the mMatInstList member variable
//--------------------------------------------------------------------------
void MaterialList::clearMatInstList()
{
   // clear out old materials.  any non null element of the list should be pointing at deletable memory,
   // although multiple indexes may be pointing at the same memory so we have to be careful (see
   // comment in loop body)
   for (U32 i=0; i<mMatInstList.size(); i++)
   {
      if (mMatInstList[i])
      {
         BaseMatInstance* current = mMatInstList[i];

         // ok, since ts material lists can remap difference indexes to the same object 
         // we need to make sure that we don't delete the same memory twice.  walk the 
         // rest of the list and null out any pointers that match the one we deleted.
         for (U32 j=0; j<mMatInstList.size(); j++)
            if (mMatInstList[j] == current)
               mMatInstList[j] = NULL;

         mMatInstList[i] = NULL;
         delete current;
      }
   }
}

//--------------------------------------------------------------------------
// Map materials - map materials to the textures in the list
//--------------------------------------------------------------------------
void MaterialList::mapMaterials()
{
   mMatInstList.setSize( mMaterialNames.size() );

   for( U32 i=0; i<mMaterialNames.size(); i++ )
      mapMaterial( i );
}

/// Map the material name at the given index to a material instance.
///
/// @note The material instance that is created will <em>not be initialized.</em>

void MaterialList::mapMaterial( U32 i )
{
   AssertFatal( i < size(), "MaterialList::mapMaterialList - index out of bounds" );

   if( mMatInstList[i] != NULL )
      return;

   // lookup a material property entry
   const String &matName = getMaterialName(i);

   // JMQ: this code assumes that all materials have names.
   if( matName.isEmpty() )
   {
      mMatInstList[i] = NULL;
      return;
   }

   String materialName = MATMGR->getMapEntry(matName);

   // IF we didn't find it, then look for a PolyStatic generated Material
   //  [a little cheesy, but we need to allow for user override of generated Materials]
   if ( materialName.isEmpty() )
      materialName = MATMGR->getMapEntry( String::ToString( "polyMat_%s", matName.c_str() ) );

   if ( materialName.isNotEmpty() )
   {
      Material * mat = MATMGR->getMaterialDefinitionByName( materialName );
      mMatInstList[i] = mat ? mat->createMatInstance() : MATMGR->createWarningMatInstance();
   }
   else
   {
      if ( Con::getBoolVariable( "$Materials::createMissing", true ) )
      {
         // No Material found, create new "default" material with just a diffuseMap

         // First see if there is a valid diffuse texture
         GFXTexHandle texHandle;
         if (mLookupPath.isEmpty())
         {
            texHandle.set( mMaterialNames[i], &GFXDefaultStaticDiffuseProfile, avar("%s() - handle (line %d)", __FUNCTION__, __LINE__) );
         }
         else
         {
            // Should we strip off the extension of the path here before trying
            // to load the texture?
            String fullPath = String::ToString( "%s/%s", mLookupPath.c_str(), mMaterialNames[i].c_str() );
            texHandle.set( fullPath, &GFXDefaultStaticDiffuseProfile, avar("%s() - handle (line %d)", __FUNCTION__, __LINE__) );
         }

         if ( texHandle.isValid() )
         {
            String newMatName = Sim::getUniqueName( "DefaultMaterial" );
            Material *newMat = MATMGR->allocateAndRegister( newMatName, mMaterialNames[i] );

            // Flag this as an autogenerated Material
            newMat->mAutoGenerated = true;

            // Overwrite diffuseMap in new material
            newMat->mDiffuseMapFilename[0] = texHandle->mTextureLookupName;

            // Set up some defaults for transparent textures
            if (texHandle->mHasTransparency)
            {
               newMat->mTranslucent = true;
               newMat->mTranslucentBlendOp = Material::LerpAlpha;
               newMat->mTranslucentZWrite = true;
               newMat->mAlphaRef = 20;
            }

            // create a MatInstance for the new material
            mMatInstList[i] = newMat->createMatInstance();

            #ifndef TORQUE_SHIPPING
               Con::warnf( "[MaterialList::mapMaterials] Creating missing material for texture: %s", texHandle->mTextureLookupName.c_str() );
            #endif
         }
         else
         {
            Con::errorf( "[MaterialList::mapMaterials] Unable to find material for texture: %s", mMaterialNames[i].c_str() );
            mMatInstList[i] = MATMGR->createWarningMatInstance();
         }
      }
      else
      {
         mMatInstList[i] = MATMGR->createWarningMatInstance();
      }
   }
}

void MaterialList::initMatInstances(   const FeatureSet &features, 
                                       const GFXVertexFormat *vertexFormat )
{
   for( U32 i=0; i < mMatInstList.size(); i++ )
   {
      BaseMatInstance *matInst = mMatInstList[i];
      if ( !matInst )
         continue;

      if ( !matInst->init( features, vertexFormat ) )
      {
         Con::errorf( "MaterialList::initMatInstances - failed to initialize material instance for '%s'",
            matInst->getMaterial()->getName() );

         // Fall back to warning material.

         SAFE_DELETE( matInst );
         matInst = MATMGR->createMatInstance( "WarningMaterial" );
         matInst->init( MATMGR->getDefaultFeatures(), vertexFormat );
         mMatInstList[ i ] = matInst;
      }
   }

}

void MaterialList::setMaterialInst( BaseMatInstance *matInst, U32 texIndex )
{
   AssertFatal( texIndex < mMatInstList.size(), "MaterialList::setMaterialInst - index out of bounds" );
   mMatInstList[texIndex] = matInst;
}
