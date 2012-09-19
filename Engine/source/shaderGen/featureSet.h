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

#ifndef _FEATURESET_H_
#define _FEATURESET_H_

#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

class FeatureType;


//
class FeatureSet
{
protected:

   struct FeatureInfo
   {      
      const FeatureType* type;
      S32 index;
   };

   /// The list of featurs.   
   Vector<FeatureInfo> mFeatures;

   /// A string representation of all the 
   /// features used for comparisons.
   String mDescription;

   ///
   static S32 _typeCmp( const FeatureInfo* a, const FeatureInfo *b );

   ///
   void _rebuildDesc();

public:

   FeatureSet()
   {
   }

   FeatureSet( const FeatureSet &h )
      :  mFeatures( h.mFeatures ),
         mDescription( h.mDescription ) 
   {
   }

   FeatureSet& operator =( const FeatureSet &h );

   /// Equality operators.
   inline bool operator != ( const FeatureSet &h ) const { return h.getDescription() != getDescription(); }
   inline bool operator == ( const FeatureSet &h ) const { return h.getDescription() == getDescription(); }

   bool operator []( const FeatureType &type ) const { return hasFeature( type ); }

   /// Returns true if the feature set is empty.
   bool isEmpty() const { return mFeatures.empty(); }

   /// Returns true if the feature set is not empty.
   bool isNotEmpty() const { return !mFeatures.empty(); }

   /// Return the description string which uniquely identifies this feature set.
   const String& getDescription() const;

   /// Returns the feature count.
   U32 getCount() const { return mFeatures.size(); }

   /// Returns the feature at the index and optionally
   /// the feature index when it was added.
   const FeatureType& getAt( U32 index, S32 *outIndex = NULL ) const;

   /// Returns true if this handle has this feature.
   bool hasFeature( const FeatureType &type, S32 index = -1 ) const;

   /// 
   void setFeature( const FeatureType &type, bool set, S32 index = -1 );

   /// 
   void addFeature( const FeatureType &type, S32 index = -1 );

   /// 
   void removeFeature( const FeatureType &type );

   ///
   U32 getNextFeatureIndex( const FeatureType &type, S32 index ) const;

   /// Removes features that are not in the input set.
   void filter( const FeatureSet &features );

   /// Removes features that are in the input set.
   void exclude( const FeatureSet &features );

   ///
   void merge( const FeatureSet &features );

   /// Clears all features.
   void clear();

   /// Default empty feature set.
   static const FeatureSet EmptySet;

};


inline const String& FeatureSet::getDescription() const
{
   // Update the description if its empty and we have features.
   if ( mDescription.isEmpty() && !mFeatures.empty() )
      const_cast<FeatureSet*>(this)->_rebuildDesc();
   
   return mDescription;
}

#endif // _FEATURESET_H_
