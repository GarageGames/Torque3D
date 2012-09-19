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
#include "decalDataFile.h"

#include "math/mathIO.h"
#include "core/tAlgorithm.h"
#include "core/stream/fileStream.h"

#include "T3D/decal/decalManager.h"
#include "T3D/decal/decalData.h"


template<>
void* Resource<DecalDataFile>::create( const Torque::Path &path )
{
   FileStream stream;
   stream.open( path.getFullPath(), Torque::FS::File::Read );
   if ( stream.getStatus() != Stream::Ok )
      return NULL;

   DecalDataFile *file = new DecalDataFile();
   if ( !file->read( stream ) )
   {
      delete file;
      return NULL;
   }

   return file;
}

template<> 
ResourceBase::Signature Resource<DecalDataFile>::signature()
{
   return MakeFourCC('d','e','c','f');
}


//-----------------------------------------------------------------------------

DecalDataFile::DecalDataFile()
   : mIsDirty( false ),
     mSphereWithLastInsertion( NULL )
{
   VECTOR_SET_ASSOCIATION( mSphereList );
}

//-----------------------------------------------------------------------------

DecalDataFile::~DecalDataFile()
{
   clear();
}

//-----------------------------------------------------------------------------

void DecalDataFile::clear()
{
   for ( U32 i=0; i < mSphereList.size(); i++ )
      delete mSphereList[i];

   mSphereList.clear();
   mSphereWithLastInsertion = NULL;
   mChunker.freeBlocks();

   mIsDirty = true;
}

//-----------------------------------------------------------------------------

bool DecalDataFile::write( Stream& stream )
{
   // Write our identifier... so we have a better
   // idea if we're reading pure garbage.
   // This identifier stands for "Torque Decal Data File".
   stream.write( 4, "TDDF" );

   // Now the version number.
   stream.write( (U8)FILE_VERSION );

   Vector<DecalInstance*> allDecals;

   // Gather all DecalInstances that should be saved.
   for ( U32 i = 0; i < mSphereList.size(); i++ )  
   {      
      Vector<DecalInstance*>::const_iterator item = mSphereList[i]->mItems.begin();
      for ( ; item != mSphereList[i]->mItems.end(); item++ )
      {
         if ( (*item)->mFlags & SaveDecal )
            allDecals.push_back( (*item) );
      }
   }

   // Gather all the DecalData datablocks used.
   Vector<const DecalData*> allDatablocks;
   for ( U32 i = 0; i < allDecals.size(); i++ )
      allDatablocks.push_back_unique( allDecals[i]->mDataBlock );

   // Write out datablock lookupNames.
   U32 count = allDatablocks.size();
   stream.write( count );
   for ( U32 i = 0; i < count; i++ )
      stream.write( allDatablocks[i]->lookupName );

   Vector<const DecalData*>::iterator dataIter;

   // Write out the DecalInstance list.
   count = allDecals.size();
   stream.write( count );
   for ( U32 i = 0; i < count; i++ )
   {
      DecalInstance *inst = allDecals[i];

      dataIter = find( allDatablocks.begin(), allDatablocks.end(), inst->mDataBlock );
      U8 dataIndex = dataIter - allDatablocks.begin();
      
      stream.write( dataIndex );
      mathWrite( stream, inst->mPosition );
      mathWrite( stream, inst->mNormal );
      mathWrite( stream, inst->mTangent );
		stream.write( inst->mTextureRectIdx );   
      stream.write( inst->mSize );
      stream.write( inst->mRenderPriority );
   }

   // Clear the dirty flag.
   mIsDirty = false;

   return true;
}

//-----------------------------------------------------------------------------

bool DecalDataFile::read( Stream &stream )
{
   // NOTE: we are shortcutting by just saving out the DecalInst and 
   // using regular addDecal methods to add them, which will end up
   // generating the DecalSphere(s) in the process.
   // It would be more efficient however to just save out all the data
   // and read it all in with no calculation required.

   // Read our identifier... so we know we're 
   // not reading in pure garbage.
   char id[4] = { 0 };
   stream.read( 4, id );
   if ( dMemcmp( id, "TDDF", 4 ) != 0 )
   {
      Con::errorf( "DecalDataFile::read() - This is not a Decal file!" );
      return false;
   }

   // Empty ourselves before we really begin reading.
   clear();

   // Now the version number.
   U8 version;
   stream.read( &version );
   if ( version != (U8)FILE_VERSION )
   {
      Con::errorf( "DecalDataFile::read() - file versions do not match!" );
      Con::errorf( "You must manually delete the old .decals file before continuing!" );
      return false;
   }

   // Read in the lookupNames of the DecalData datablocks and recover the datablock.   
   Vector<DecalData*> allDatablocks;
   U32 count;
   stream.read( &count );
   allDatablocks.setSize( count );
   for ( U32 i = 0; i < count; i++ )
   {      
      String lookupName;      
      stream.read( &lookupName );
      
      DecalData *data = DecalData::findDatablock( lookupName );

      if ( !data )
      {
			char name[512];
			dSprintf(name, 512, "%s_missing", lookupName.c_str());

			DecalData *stubCheck = DecalData::findDatablock( name );
			if( !stubCheck )
			{
				data = new DecalData;
				data->lookupName = name;
				data->registerObject(name);
				Sim::getRootGroup()->addObject( data );
				data->materialName = "WarningMaterial";
				data->material = dynamic_cast<Material*>(Sim::findObject("WarningMaterial"));
			
				Con::errorf( "DecalDataFile::read() - DecalData %s does not exist! Temporarily created %s_missing.", lookupName.c_str() );
			}
      }
		
		allDatablocks[ i ] = data;
   }

   U8 dataIndex;   
   DecalData *data;

   // Now read all the DecalInstance(s).
   stream.read( &count );
   for ( U32 i = 0; i < count; i++ )
   {           
      DecalInstance *inst = _allocateInstance();

      stream.read( &dataIndex );
      mathRead( stream, &inst->mPosition );
      mathRead( stream, &inst->mNormal );
      mathRead( stream, &inst->mTangent );
		stream.read( &inst->mTextureRectIdx );
      stream.read( &inst->mSize );
      stream.read( &inst->mRenderPriority );

      inst->mVisibility = 1.0f;
      inst->mFlags = PermanentDecal | SaveDecal | ClipDecal;
      inst->mCreateTime = Sim::getCurrentTime();
      inst->mVerts = NULL;
      inst->mIndices = NULL;
      inst->mVertCount = 0;
      inst->mIndxCount = 0;

      data = allDatablocks[ dataIndex ];

      if ( data )          
      {         
         inst->mDataBlock = data;

         _addDecalToSpheres( inst );
			
			// onload set instances should get added to the appropriate vec
			inst->mId = gDecalManager->mDecalInstanceVec.size();
			gDecalManager->mDecalInstanceVec.push_back(inst);
      }
      else
      {
         _freeInstance( inst );
         Con::errorf( "DecalDataFile::read - cannot find DecalData for DecalInstance read from disk." );
      }
   }

   // Clear the dirty flag.
   mIsDirty = false;

   return true;
}

//-----------------------------------------------------------------------------

DecalInstance* DecalDataFile::addDecal( const Point3F& pos, const Point3F& normal, const Point3F& tangent,
                                        DecalData* decalData, F32 decalScale, S32 decalTexIndex, U8 flags )
{
   DecalInstance* newDecal = _allocateInstance();

   newDecal->mRenderPriority = 0;
   newDecal->mCustomTex = NULL;
   newDecal->mId = -1;

   newDecal->mPosition = pos;
   newDecal->mNormal = normal;
   newDecal->mTangent = tangent;

   newDecal->mSize = decalData->size * decalScale;

   newDecal->mDataBlock = decalData;

   S32 frame = newDecal->mDataBlock->frame;
   // randomize the frame if the flag is set. this number is used directly below us
   // when calculating render coords
   if ( decalData->randomize )
      frame = gRandGen.randI();

   frame %= getMax( decalData->texCoordCount, 0 ) + 1;

   newDecal->mTextureRectIdx = frame;

   newDecal->mVisibility = 1.0f;

   newDecal->mLastAlpha = -1.0f;

   newDecal->mCreateTime = Sim::getCurrentTime();

   newDecal->mVerts = NULL;
   newDecal->mIndices = NULL;
   newDecal->mVertCount = 0;
   newDecal->mIndxCount = 0;

   newDecal->mFlags = flags;
   newDecal->mFlags |= ClipDecal;

   _addDecalToSpheres( newDecal );

   return newDecal;
}

//-----------------------------------------------------------------------------

void DecalDataFile::_addDecalToSpheres( DecalInstance* inst )
{
   // First try the sphere we have last inserted an item into, if there is one.
   // Given a good spatial locality of insertions, this is a reasonable first
   // guess as a good candidate.

   if( mSphereWithLastInsertion && mSphereWithLastInsertion->tryAddItem( inst ) )
      return;

   // Otherwise, go through the list and try to find an existing sphere that meets
   // our tolerances.

   for( U32 i = 0; i < mSphereList.size(); i++ )
   {
      DecalSphere* sphere = mSphereList[i];
      
      if( sphere == mSphereWithLastInsertion )
         continue;

      if( sphere->tryAddItem( inst ) )
      {
         mSphereWithLastInsertion = sphere;
         return;
      }
   }

   // Didn't find a suitable existing sphere, so create a new one.

   DecalSphere* sphere = new DecalSphere( inst->mPosition, inst->mSize / 2.f );

   mSphereList.push_back( sphere );
   mSphereWithLastInsertion = sphere;

   sphere->mItems.push_back( inst );
}

//-----------------------------------------------------------------------------

void DecalDataFile::removeDecal( DecalInstance *inst )
{
   if( !_removeDecalFromSpheres( inst ) )
      return;

   _freeInstance( inst );
}

//-----------------------------------------------------------------------------

bool DecalDataFile::_removeDecalFromSpheres( DecalInstance *inst )
{
   for( U32 i = 0; i < mSphereList.size(); i++ )
   {
      DecalSphere* sphere = mSphereList[ i ];
      Vector< DecalInstance* > &items = sphere->mItems;

      // Try to remove the instance from the list of this sphere.
      // If that fails, the instance doesn't belong to this sphere
      // so continue.

      if( !items.remove( inst ) )
         continue;

      // If the sphere is now empty, remove it.  Otherwise, update
      // it's bounds.

      if( items.empty() )
      {
         if( mSphereWithLastInsertion == sphere )
            mSphereWithLastInsertion = NULL;

         delete sphere;      
         mSphereList.erase( i );
      }
      else
         sphere->updateWorldSphere();

      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------

void DecalDataFile::notifyDecalModified( DecalInstance *inst )
{
   _removeDecalFromSpheres( inst );
   _addDecalToSpheres( inst );
}
