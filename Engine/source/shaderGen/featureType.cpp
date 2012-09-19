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
#include "shaderGen/featureType.h"

#include "shaderGen/featureSet.h"

FeatureTypeVector& FeatureType::_getTypes()
{
   // We create it as a method static so that 
   // its available to other statics regardless
   // of initialization order.
   static FeatureTypeVector theTypes;
   return theTypes;
}

void FeatureType::addDefaultTypes( FeatureSet *outFeatures )
{
   const FeatureTypeVector &types = _getTypes();
   for ( U32 i=0; i < types.size(); i++ )
   {
      if ( types[i]->isDefault() )
         outFeatures->addFeature( *types[i] );
   }   
}

FeatureType::FeatureType( const char *name, U32 group, F32 order, bool isDefault )
   :  mName( name ),
      mGroup( group ),
      mOrder( order ),
      mIsDefault( isDefault )
{
   FeatureTypeVector &types = _getTypes();

   #ifdef TORQUE_DEBUG
      for ( U32 i=0; i < types.size(); i++ )
         AssertFatal( !mName.equal( types[i]->getName() ), "FeatureType - This feature already exists!" );
   #endif

   mId = types.size();
   types.push_back( this );
}
