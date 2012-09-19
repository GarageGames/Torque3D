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
#include "forest/forestItem.h"

#include "forest/forestCollision.h"

#include "core/stream/bitStream.h"
#include "console/consoleTypes.h"

IMPLEMENT_CO_DATABLOCK_V1(ForestItemData);

ConsoleDocClass( ForestItemData,
   "@brief Base class for defining a type of ForestItem. It does not implement "
   "loading or rendering of the shapeFile.\n\n"
   "@ingroup Forest"
);

SimSet* ForestItemData::smSet = NULL;


ForestItemData::ForestItemData()
   :  mNeedPreload( true ),
      mShapeFile( NULL ),
      mCollidable( true ),
      mRadius( 1 ),      
      mWindScale( 0.0f ),
      mTrunkBendScale( 0.0f ),
      mWindBranchAmp( 0.0f ),
      mWindDetailAmp( 0.0f ),
      mWindDetailFreq( 0.0f ),
      mMass( 5.0f ),
      mRigidity( 10.0f ),
      mTightnessCoefficient( 0.4f ),
      mDampingCoefficient( 0.7f )      
{
}

void ForestItemData::initPersistFields()
{
   Parent::initPersistFields();

   addGroup( "Media" );
      
      addField( "shapeFile",  TypeShapeFilename, Offset( mShapeFile, ForestItemData ),
         "Shape file for this item type" );

      addField( "collidable",   TypeBool, Offset( mCollidable, ForestItemData ),
         "Can other objects or spacial queries hit items of this type." );

      addField( "radius", TypeF32, Offset( mRadius, ForestItemData ),
         "Radius used during placement to ensure items are not crowded." );

   endGroup( "Media" );

   addGroup( "Wind" );
      
      addField( "mass", TypeF32, Offset( mMass, ForestItemData ),
         "Mass used in calculating spring forces on the trunk. Generally how "
         "springy a plant is." );

      addField( "rigidity", TypeF32, Offset( mRigidity, ForestItemData ),
         "Rigidity used in calculating spring forces on the trunk. How much the plant resists the wind force" );

      addField( "tightnessCoefficient", TypeF32, Offset( mTightnessCoefficient, ForestItemData ),
         "Coefficient used in calculating spring forces on the trunk. "
         "How much the plant resists bending." );

      addField( "dampingCoefficient", TypeF32, Offset( mDampingCoefficient, ForestItemData ),
         "Coefficient used in calculating spring forces on the trunk. "
         "Causes oscillation and forces to decay faster over time." );

      addField( "windScale",   TypeF32, Offset( mWindScale, ForestItemData ),
         "Overall scale to the effect of wind." );
      
      addField( "trunkBendScale",   TypeF32, Offset( mTrunkBendScale, ForestItemData ),
         "Overall bend amount of the tree trunk by wind and impacts." );

      addField( "branchAmp", TypeF32, Offset( mWindBranchAmp, ForestItemData ),
         "Amplitude of the effect on larger branches." );

      addField( "detailAmp", TypeF32, Offset( mWindDetailAmp, ForestItemData ),
         "Amplitude of the winds effect on leafs/fronds." );

      addField( "detailFreq", TypeF32, Offset( mWindDetailFreq, ForestItemData ),
         "Frequency (speed) of the effect on leafs/fronds." );

   endGroup( "Wind" );
}

void ForestItemData::consoleInit()
{
}

SimSet* ForestItemData::getSet()
{
   if ( !smSet )
   {
      if ( Sim::findObject( "ForestItemDataSet", smSet ) )
         return smSet;

      smSet = new SimSet;
      smSet->assignName( "ForestItemDataSet" );
      smSet->registerObject();
		Sim::getRootGroup()->addObject( smSet );
   }

   return smSet;
}

ForestItemData* ForestItemData::find( const char *name )
{
   ForestItemData *result = dynamic_cast<ForestItemData*>( getSet()->findObjectByInternalName( name ) );
   if ( !result )
      Sim::findObject( name, result );

   return result;
}

void ForestItemData::onNameChange( const char *name )
{
   setInternalName( name );
}

bool ForestItemData::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   getSet()->addObject( this );

   return true;
}

void ForestItemData::packData(BitStream* stream)
{ 
   Parent::packData(stream);

   String localName = getInternalName();
   if ( localName.isEmpty() )
      localName = getName();

   stream->write( localName );

   stream->writeString(mShapeFile);
   
   stream->writeFlag( mCollidable );

   stream->write( mRadius );

   stream->write( mMass );
   stream->write( mRigidity );
   stream->write( mTightnessCoefficient );
   stream->write( mDampingCoefficient );

   stream->write( mWindScale );
   stream->write( mTrunkBendScale );
   stream->write( mWindBranchAmp );
   stream->write( mWindDetailAmp );
   stream->write( mWindDetailFreq );
}

void ForestItemData::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);

   String localName;
   stream->read( &localName );
   setInternalName( localName );

   char readBuffer[1024];

   stream->readString(readBuffer);
   mShapeFile = StringTable->insert(readBuffer);
   
   mCollidable = stream->readFlag();

   stream->read( &mRadius );

   stream->read( &mMass );
   stream->read( &mRigidity );
   stream->read( &mTightnessCoefficient );
   stream->read( &mDampingCoefficient );

   stream->read( &mWindScale );
   stream->read( &mTrunkBendScale );
   stream->read( &mWindBranchAmp );
   stream->read( &mWindDetailAmp );
   stream->read( &mWindDetailFreq );
}

const ForestItem ForestItem::Invalid;

ForestItem::ForestItem()
   :  mDataBlock( NULL ),
      mTransform( true ),
      mScale( 0.0f ),
      mWorldBox( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f ),
      mRadius( 0.0f ),
      mKey( 0 )
{
}

ForestItem::~ForestItem()
{
}

void ForestItem::setTransform( const MatrixF &xfm, F32 scale )
{
   mTransform = xfm;
   mScale = scale;

   // Cache the world box to improve culling performance.
   VectorF objScale( mScale, mScale, mScale );
   mWorldBox = getObjBox();
   mWorldBox.minExtents.convolve( objScale );
   mWorldBox.maxExtents.convolve( objScale );
   mTransform.mul( mWorldBox );

   // Generate a radius that encompasses the entire box.
   mRadius = ( mWorldBox.maxExtents - mWorldBox.minExtents ).len() / 2.0f;
}

void ForestItem::setData( ForestItemData *data )
{
   mDataBlock = data;
}


