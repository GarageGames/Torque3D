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
#include "shaderGen/featureSet.h"

#include "shaderGen/featureType.h"
#include "platform/profiler.h"
#include "core/util/hashFunction.h"


const FeatureSet FeatureSet::EmptySet;


S32 QSORT_CALLBACK FeatureSet::_typeCmp( const FeatureInfo* a, const FeatureInfo* b )
{
   if ( a->type->getGroup() < b->type->getGroup() )
      return -1;
   else if ( a->type->getGroup() > b->type->getGroup() )
      return 1;
   else if ( a->index < b->index )
      return -1;
   else if ( a->index > b->index )
      return 1;
   else if ( a->type->getOrder() < b->type->getOrder() )
      return -1;
   else if ( a->type->getOrder() > b->type->getOrder() )
      return 1;
   else
      return 0;
}

void FeatureSet::_rebuildDesc()
{
   PROFILE_SCOPE( FeatureSet_RebuildDesc );

   // First get the features in the proper order.
   mFeatures.sort( _typeCmp );

   String desc;

   for ( U32 i=0; i < mFeatures.size(); i++ )
      desc +=  String::ToString( "%s,%d\n", 
               mFeatures[i].type->getName().c_str(),
               mFeatures[i].index );

   // By interning the description we have only
   // one instance in the system and we get fast
   // pointer compares for equality.
   mDescription = desc.intern();
}

const FeatureType& FeatureSet::getAt( U32 index, S32 *outIndex ) const 
{
   // We want to make sure we access the features in the
   // correct order.  By asking for the description we ensure
   // the feature set is properly sorted.
   getDescription();

   if ( outIndex )
      *outIndex = mFeatures[index].index;

   return *mFeatures[index].type; 
}

void FeatureSet::clear()
{
   mDescription.clear();
   mFeatures.clear();
}

FeatureSet& FeatureSet::operator =( const FeatureSet &h )
{
   clear();
   merge( h );
   return *this;
}

bool FeatureSet::hasFeature( const FeatureType &type, S32 index ) const
{
   PROFILE_SCOPE(FeatureSet_hasFeature);

   for ( U32 i = 0; i < mFeatures.size(); i++)
   {
      if (  mFeatures[i].type == &type &&
            ( index < 0 || mFeatures[i].index == index ) )
         return true;
   }

   return false;
}

void FeatureSet::setFeature( const FeatureType &type, bool set, S32 index )
{
   for ( U32 i=0; i < mFeatures.size(); i++ )
   {
      const FeatureInfo &info = mFeatures[i];
      if ( info.type == &type && info.index == index )
      {
         if ( set )
            return;
         else
         {
            mFeatures.erase_fast( i );
            mDescription.clear();
            return;
         }
      }
   }

   if ( !set )
      return;

   FeatureInfo info;
   info.type = &type;
   info.index = index;
   mFeatures.push_back( info );

   mDescription.clear();
}

void FeatureSet::addFeature( const FeatureType &type, S32 index )
{
   for ( U32 i=0; i < mFeatures.size(); i++ )
   {
      const FeatureInfo &info = mFeatures[i];
      if (  info.type == &type && 
            info.index == index )
         return;
   }

   FeatureInfo info;
   info.type = &type;
   info.index = index;
   mFeatures.push_back( info );

   mDescription.clear();
}

void FeatureSet::removeFeature( const FeatureType &type )
{
   for ( U32 i=0; i < mFeatures.size(); i++ )
   {
      const FeatureInfo &info = mFeatures[i];
      if ( info.type == &type )
      {
         mFeatures.erase_fast( i );
         mDescription.clear();
         return;
      }
   }
}

U32 FeatureSet::getNextFeatureIndex( const FeatureType &type, S32 index ) const
{
   for ( U32 i=0; i < mFeatures.size(); i++ )
   {
      const FeatureInfo &info = mFeatures[i];
      if ( info.type == &type && info.index > index )
         return i;
   }

   return -1;
}

void FeatureSet::filter( const FeatureSet &features )
{
   PROFILE_SCOPE( FeatureSet_Filter );

   for ( U32 i=0; i < mFeatures.size(); )
   {
      if ( !features.hasFeature( *mFeatures[i].type ) )
         mFeatures.erase_fast( i );
      else
         i++;
   }

   mDescription.clear();
}

void FeatureSet::exclude( const FeatureSet &features )
{
   PROFILE_SCOPE( FeatureSet_Exclude );

   for ( U32 i=0; i < features.mFeatures.size(); i++ )
      removeFeature( *features.mFeatures[i].type );

   mDescription.clear();
}

void FeatureSet::merge( const FeatureSet &features )
{
   PROFILE_SCOPE( FeatureSet_Merge );

   if ( mFeatures.empty() )
   {
      mFeatures.merge( features.mFeatures );
      mDescription = features.mDescription;
      return;
   }

   for ( U32 i=0; i < features.mFeatures.size(); i++ )
      addFeature( *features.mFeatures[i].type, 
                  features.mFeatures[i].index );
}

