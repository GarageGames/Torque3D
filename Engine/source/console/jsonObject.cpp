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
#include "console/jsonObject.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "math/mMathFn.h"

IMPLEMENT_CONOBJECT(JSONObject);

ConsoleDocClass( JSONObject,
   "@brief Data structure for storing indexed sequences of key/value pairs with value type.\n\n"

   "This is a powerful JSON class providing PHP style JSON in TorqueScript.\n\n"

   "The following features are supported:<ul>\n"
   "<li>array pointers: this allows you to move forwards or backwards through "
   "the array as if it was a list, including jumping to the start or end.</li>\n"
   "<li>sorting: the array can be sorted in either alphabetic or numeric mode, "
   "on the key or the value, and in ascending or descending order</li>\n"
   "<li>add/remove elements: elements can be pushed/popped from the start or "
   "end of the array, or can be inserted/erased from anywhere in the middle</li>\n"
   "<li>removal of duplicates: remove duplicate keys or duplicate values</li>\n"
   "<li>searching: search the array and return the index of a particular key or "
   "value</li>\n"
   "<li>counting: count the number of instances of a particular value or key in "
   "the array, as well as the total number of elements</li>\n"
   "<li>writing to JSON: write out the JSON/array object as a string</li>\n"
   "<li>advanced features: array append, array crop and array duplicate</li>\n"
   "</ul>\n\n"

   "JSON element keys and values can be strings, numbers, boolean, array, object, null, or undefined\n\n"

   "@ingroup Scripting"
);

//-----------------------------------------------------------------------------

JSONObject::JSONObject()
   : mIsArray(false)
{
}

//-----------------------------------------------------------------------------

void JSONObject::initPersistFields()
{
   addField( "isArray",    TypeBool,   Offset( mIsArray, JSONObject ), 
      "Ignore the keys and treat it as a plain array.\n");

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

const JSONObject::DataType JSONObject::getValueType( const String& val ) const
{
   JSONObject* jsonObj = NULL;
   if(val.isEmpty())
   {
      return DataType::UNDEFINED;
   }
   else if (!val.equal("0", String::NoCase) && Sim::findObject(val, jsonObj))
   {
      return jsonObj->mIsArray ? DataType::ARRAY : DataType::OBJECT;
   }
   else if(val.equal("null", String::NoCase))
   {
      return DataType::NULL_TYPE;
   }
   else if(val.equal("true", String::NoCase) || val.equal("false", String::NoCase))
   {
      return DataType::BOOLEAN;
   }
   /* This code relies on some other math functions defined in mMathFn
   in many cases a string will work just as good, so I removed this, but someone may want it.
   else if(isInt(val.c_str()) || isFloat(val.c_str()))
   {
   return DataType::NUMBER;
   }
   */

   return DataType::STRING;
}

char* JSONObject::toJSON() const
{
   //Put the start of the object/array tag
   String temp = (mIsArray) ? "[" : "{";

   for ( U32 i = 0; i < mArray.size(); i++ )
   {
      //Get the key value pair
      const String& key = mArray[i].key;
      const String& val = mArray[i].value;

      //If not the first then add a comma to separate items
      if(i > 0)
         temp += ", ";

      //If we are not an array add the key to the string
      if(!mIsArray)
         temp += "\"" + key + "\": ";

      //Check the type and output the correct string
      const DataType type = getValueType(val);
      switch(type)
      {
         case DataType::NULL_TYPE:
         case DataType::BOOLEAN:
            temp += String::ToLower(val);
            break;
         case DataType::NUMBER:
            temp += val;
            break;
         case DataType::ARRAY:
         case DataType::OBJECT:
            {
               JSONObject* jsonObj = NULL;
               if(Sim::findObject(val, jsonObj))
               {
                  temp += jsonObj->toJSON();
               }
            }
            break;
         default:
            temp += "\"" + val + "\"";
      }
   }

   //End the object/array
   temp += (mIsArray) ? "]" : "}";

   //Return our new string
   return Con::getReturnBuffer(temp);
}

//=============================================================================
//    Console Methods.
//=============================================================================

DefineEngineMethod(JSONObject, getValueType, const char*, (S32 index),, "Get JSON type as a string")
{
   const String val = object->getValueFromIndex(index);
   switch(object->getValueType(val))
   {
   case JSONObject::DataType::ARRAY:
      return "Array";
   case JSONObject::DataType::BOOLEAN:
      return "Boolean";
   case JSONObject::DataType::NULL_TYPE:
      return "Null";
   case JSONObject::DataType::NUMBER:
      return "Number";
   case JSONObject::DataType::OBJECT:
      return "Object";
   case JSONObject::DataType::STRING:
      return "String";
   default:
      return "Undefined";
   }
}

DefineEngineMethod(JSONObject, toJSON, const char*, (),, "Get JSON string for this object")
{
   return object->toJSON();
}