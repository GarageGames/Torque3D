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

#ifndef _MATERIALFEATUREDATA_H_
#define _MATERIALFEATUREDATA_H_

#ifndef _UTIL_DELEGATE_H_
#include "core/util/delegate.h"
#endif
#ifndef _FEATURESET_H_
#include "shaderGen/featureSet.h"
#endif


class ProcessedMaterial;


//-----------------------------------------------------------------------------
// MaterialFeatureData, this is basically a series of flags which a material can
// ask for.  Shader processed materials will use the shadergen system to accomplish this,
// FF processed materials will do the best they can.  
//-----------------------------------------------------------------------------
struct MaterialFeatureData
{
public:

   // General feature data for a pass or for other purposes.
   FeatureSet features;

   // This is to give hints to shader creation code.  It contains
   //    all the features that are in a material stage instead of just
   //    the current pass.
   FeatureSet materialFeatures;

public:

   MaterialFeatureData();

   MaterialFeatureData( const MaterialFeatureData &data );

   MaterialFeatureData( const FeatureSet &handle );

   void clear();

   const FeatureSet& codify() const { return features; }

};


///
typedef Delegate< void( ProcessedMaterial *mat,
                        U32 stageNum,
                        MaterialFeatureData &fd, 
                        const FeatureSet &features) > MatFeaturesDelegate;

#endif // _MATERIALFEATUREDATA_H_
