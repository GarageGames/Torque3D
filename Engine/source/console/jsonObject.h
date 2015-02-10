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

#ifndef _JSONOBJECT_H_
#define _JSONOBJECT_H_

#ifndef _ARRAYOBJECT_H_
#include "console/arrayObject.h"
#endif

// This class was written by Nathan Bowhay


/// A data structure holding indexed sequences of key/value pairs with value type for script use to write to JSON.
class JSONObject : public ArrayObject
{
   typedef ArrayObject Parent;

protected:
   ///Ignore key's and treat it like an array
   bool mIsArray;

public:
  
   JSONObject();

   ///Enumeration of data type for the array
   enum DataType {
	   NUMBER,
	   STRING,
	   BOOLEAN,
	   ARRAY,
	   OBJECT,
	   NULL_TYPE,
	   UNDEFINED
   };

   ///Get the type of the value
   ///@arg val String value to check the type of
   ///@return DataType of the string passed in
   const DataType getValueType( const String& val ) const;

   /// Returns string JSON representation of this object
   /// @return JSON string for this object
   char* toJSON() const;

   // SimObject
   DECLARE_CONOBJECT( JSONObject );
   DECLARE_CATEGORY( "Core" );
   DECLARE_DESCRIPTION( "An object storing an indexed sequence of key/value pairs with type and JSON writing." );
   
   static void initPersistFields();
};

#endif // _JSONOBJECT_H_