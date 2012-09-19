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

#ifndef _FEATURETYPE_H_
#define _FEATURETYPE_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif

class FeatureType;
class FeatureSet;


/// A vector of feature types.
typedef Vector<const FeatureType*> FeatureTypeVector;


///
class FeatureType
{
protected:

   /// Returns the map of all the types.
   static FeatureTypeVector& _getTypes();

   /// The feature type name.
   String mName;

   /// A unique feature id value.
   U32 mId;

   /// The group is used to orginize the types.
   U32 mGroup;

   /// The sort order of this feature type 
   /// within its group.
   F32 mOrder;

   ///
   bool mIsDefault;

   ///
   FeatureType( const FeatureType &type );

public:

   /// Adds all the default features types to the set.
   static void addDefaultTypes( FeatureSet *outFeatures );

   /// You should not use this constructor directly.
   /// @see DeclareFeatureType
   /// @see ImplementFeatureType
   FeatureType(   const char *type, 
                  U32 group = -1, 
                  F32 order = -1.0f,
                  bool isDefault = true );

   bool operator !=( const FeatureType &type ) const { return this != &type; }

   bool operator ==( const FeatureType &type ) const { return this == &type; }

   const String& getName() const { return mName; }

   U32 getId() const { return mId; }

   U32 getGroup() const { return mGroup; }

   F32 getOrder() const { return mOrder; }

   bool isDefault() const { return mIsDefault; }

};


/// 
#define DeclareFeatureType( name ) \
   extern const FeatureType name


///
#define ImplementFeatureType( name, group, order, isDefault ) \
   const FeatureType name( #name, group, order, isDefault )

/*
#define ImplementFeatureType( name, group, order ) \
   const FeatureType name( #name, group, order );

#define ImplementFeatureType( name, group ) \
   const FeatureType name( #name, group );

#define ImplementFeatureType( name ) \
   const FeatureType name( #name );
*/

#endif // _FEATURETYPE_H_