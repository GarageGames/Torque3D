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
#ifndef _FEATUREMGR_H_
#define _FEATUREMGR_H_

#ifndef _TSINGLETON_H_
#include "core/util/tSingleton.h"
#endif 
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif 

class FeatureType;
class ShaderFeature;

/// Used by the feature manager.
struct FeatureInfo
{
   const FeatureType *type;
   ShaderFeature *feature;
};


///
class FeatureMgr
{
protected:

   bool mNeedsSort;

   typedef Vector<FeatureInfo> FeatureInfoVector;

   FeatureInfoVector mFeatures;

   static S32 QSORT_CALLBACK _featureInfoCompare( const FeatureInfo *a, const FeatureInfo *b );

public:
   
   FeatureMgr();
   ~FeatureMgr();

   /// Returns the count of registered features.
   U32 getFeatureCount() const { return mFeatures.size(); }

   /// Returns the feature info at the index.
   const FeatureInfo& getAt( U32 index );

   /// 
   ShaderFeature* getByType( const FeatureType &type );

   // Allows other systems to add features.  index is 
   // the enum in GFXMaterialFeatureData.
   void registerFeature(   const FeatureType &type, 
                           ShaderFeature *feature );

   // Unregister a feature.
   void unregisterFeature( const FeatureType &type );


   /// Removes all features.
   void unregisterAll();

   // For ManagedSingleton.
   static const char* getSingletonName() { return "FeatureMgr"; }   
};

// Helper for accessing the feature manager singleton.
#define FEATUREMGR ManagedSingleton<FeatureMgr>::instance()

#endif // FEATUREMGR