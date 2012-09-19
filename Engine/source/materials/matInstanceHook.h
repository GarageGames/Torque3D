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

#ifndef _MATINSTANCEHOOK_H_
#define _MATINSTANCEHOOK_H_

#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif


/// The hook type wrapper object
class MatInstanceHookType
{
protected:

   typedef HashTable<String,U32> TypeMap;

   /// Returns the map of all the hook types.  We create
   /// it as a method static so that its available to other
   /// statics regardless of initialization order.
   static inline TypeMap& getTypeMap()
   {
      static TypeMap smTypeMap;
      return smTypeMap;
   }

   /// The hook type index for this type.
   U32 mTypeIndex;

public:

   MatInstanceHookType( const char *type );

   inline MatInstanceHookType( const MatInstanceHookType &type )
      : mTypeIndex( type.mTypeIndex )
   {
   }

   inline operator U32 () const { return mTypeIndex; }
};


/// This class is used to define hook objects attached to
/// material instances and provide a registration system
/// for different hook types.
///
/// @see BaseMatInstance
/// @see MaterialManager
///
class MatInstanceHook
{
public:

   ///
   virtual ~MatInstanceHook() {}

   /// 
   virtual const MatInstanceHookType& getType() const = 0;
};

#endif // _MATINSTANCEHOOK_H_






