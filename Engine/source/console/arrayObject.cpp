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
#include "console/arrayObject.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "math/mMathFn.h"


IMPLEMENT_CONOBJECT(ArrayObject);

ConsoleDocClass( ArrayObject,
   "@brief Data structure for storing indexed sequences of key/value pairs.\n\n"

   "This is a powerful array class providing PHP style arrays in TorqueScript.\n\n"

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
   "<li>counting: count the number of instaces of a particular value or key in "
   "the array, as well as the total number of elements</li>\n"
   "<li>advanced features: array append, array crop and array duplicate</li>\n"
   "</ul>\n\n"

   "Array element keys and values can be strings or numbers\n\n"

   "@ingroup Scripting"
);


bool ArrayObject::smDecreasing = false;
bool ArrayObject::smCaseSensitive = false;
const char* ArrayObject::smCompareFunction;


S32 QSORT_CALLBACK ArrayObject::_valueCompare( const void* a, const void* b )
{
   ArrayObject::Element *ea = (ArrayObject::Element *) (a);
   ArrayObject::Element *eb = (ArrayObject::Element *) (b);
   S32 result = smCaseSensitive ? dStrnatcmp(ea->value, eb->value) : dStrnatcasecmp(ea->value, eb->value);
   return ( smDecreasing ? -result : result );
}

S32 QSORT_CALLBACK ArrayObject::_valueNumCompare( const void* a, const void* b )
{
   ArrayObject::Element *ea = (ArrayObject::Element *) (a);
   ArrayObject::Element *eb = (ArrayObject::Element *) (b);
   F32 aCol = dAtof(ea->value);
   F32 bCol = dAtof(eb->value);
   F32 result = aCol - bCol;
   S32 res = result < 0 ? -1 : (result > 0 ? 1 : 0);
   return ( smDecreasing ? res : -res );
}

S32 QSORT_CALLBACK ArrayObject::_keyCompare( const void* a, const void* b )
{
   ArrayObject::Element *ea = (ArrayObject::Element *) (a);
   ArrayObject::Element *eb = (ArrayObject::Element *) (b);
   S32 result = smCaseSensitive ? dStrnatcmp(ea->key, eb->key) : dStrnatcasecmp(ea->key, eb->key);
   return ( smDecreasing ? -result : result );
}

S32 QSORT_CALLBACK ArrayObject::_keyNumCompare( const void* a, const void* b )
{
   ArrayObject::Element *ea = (ArrayObject::Element *) (a);
   ArrayObject::Element *eb = (ArrayObject::Element *) (b);
   const char* aCol = ea->key;
   const char* bCol = eb->key;
   F32 result = dAtof(aCol) - dAtof(bCol);
   S32 res = result < 0 ? -1 : (result > 0 ? 1 : 0);
   return ( smDecreasing ? res : -res );
}

S32 QSORT_CALLBACK ArrayObject::_keyFunctionCompare( const void* a, const void* b )
{
   ArrayObject::Element* ea = ( ArrayObject::Element* )( a );
   ArrayObject::Element* eb = ( ArrayObject::Element* )( b );
   
   const char* argv[ 3 ];
   argv[ 0 ] = smCompareFunction;
   argv[ 1 ] = ea->key;
   argv[ 2 ] = eb->key;
   
   S32 result = dAtoi( Con::execute( 3, argv ) );
   S32 res = result < 0 ? -1 : ( result > 0 ? 1 : 0 );
   return ( smDecreasing ? res : -res );
}

S32 QSORT_CALLBACK ArrayObject::_valueFunctionCompare( const void* a, const void* b )
{
   ArrayObject::Element* ea = ( ArrayObject::Element* )( a );
   ArrayObject::Element* eb = ( ArrayObject::Element* )( b );
   
   const char* argv[ 3 ];
   argv[ 0 ] = smCompareFunction;
   argv[ 1 ] = ea->value;
   argv[ 2 ] = eb->value;
   
   S32 result = dAtoi( Con::execute( 3, argv ) );
   S32 res = result < 0 ? -1 : ( result > 0 ? 1 : 0 );
   return ( smDecreasing ? res : -res );
}


//-----------------------------------------------------------------------------

ArrayObject::ArrayObject()
   : mCurrentIndex( NULL ),
     mCaseSensitive( false )
{
}

//-----------------------------------------------------------------------------

void ArrayObject::initPersistFields()
{
   addField( "caseSensitive",    TypeBool,   Offset( mCaseSensitive, ArrayObject ), 
      "Makes the keys and values case-sensitive.\n"
      "By default, comparison of key and value strings will be case-insensitive." );

   addProtectedField( "key", TypeCaseString, NULL, &_addKeyFromField, &emptyStringProtectedGetFn, 
      "Helper field which allows you to add new key['keyname'] = value pairs." );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool ArrayObject::_addKeyFromField( void *object, const char *index, const char *data )
{
   static_cast<ArrayObject*>( object )->push_back( index, data );
   return false;
}

//-----------------------------------------------------------------------------

S32 ArrayObject::getIndexFromValue( const String &value ) const
{
   S32 foundIndex = -1;
   for ( S32 i = mCurrentIndex; i < mArray.size(); i++ )
   {
      if ( isEqual( mArray[i].value, value ) )
      {
         foundIndex = i;
         break;
      }
   }

   if( foundIndex < 0 )
   {
      for ( S32 i = 0; i < mCurrentIndex; i++ )
      {
         if ( isEqual( mArray[i].value, value ) )
         {
            foundIndex = i;
            break;
         }
      }
   }

   return foundIndex;
}

//-----------------------------------------------------------------------------

S32 ArrayObject::getIndexFromKey( const String &key ) const
{
   S32 foundIndex = -1;
   for ( S32 i = mCurrentIndex; i < mArray.size(); i++ )
   {
      if ( isEqual( mArray[i].key, key ) )
      {
         foundIndex = i;
         break;
      }
   }

   if( foundIndex < 0 )
   {
      for ( S32 i = 0; i < mCurrentIndex; i++ )
      {
         if ( isEqual( mArray[i].key, key ) )
         {
            foundIndex = i;
            break;
         }
      }
   }

   return foundIndex;
}

//-----------------------------------------------------------------------------

S32 ArrayObject::getIndexFromKeyValue( const String &key, const String &value ) const
{
   S32 foundIndex = -1;
   for ( S32 i = mCurrentIndex; i < mArray.size(); i++ )
   {
      if ( isEqual( mArray[i].key, key ) && isEqual( mArray[i].value, value ) )
      {
         foundIndex = i;
         break;
      }
   }

   if ( foundIndex < 0 )
   {
      for ( S32 i = 0; i < mCurrentIndex; i++ )
      {
         if ( isEqual( mArray[i].key, key ) && isEqual( mArray[i].value, value ) )
         {
            foundIndex = i;
            break;
         }
      }
   }

   return foundIndex;
}

//-----------------------------------------------------------------------------

const String& ArrayObject::getKeyFromIndex( S32 index ) const
{
   if ( index >= mArray.size() || index < 0 )
      return String::EmptyString;

   return mArray[index].key;
}

//-----------------------------------------------------------------------------

const String& ArrayObject::getValueFromIndex( S32 index ) const
{
   if( index >= mArray.size() || index < 0 )
      return String::EmptyString;

   return mArray[index].value;
}

//-----------------------------------------------------------------------------

S32 ArrayObject::countValue( const String &value ) const
{
   S32 count = 0;
   for ( S32 i = 0; i < mArray.size(); i++ )
   {
      if ( isEqual( mArray[i].value, value ) )
         count++;
   }

   return count;
}

//-----------------------------------------------------------------------------

S32 ArrayObject::countKey( const String &key) const 
{
   S32 count = 0;
   for ( S32 i = 0; i < mArray.size(); i++ )
   {
      if ( isEqual( mArray[i].key, key ) )
         count++;
   }

   return count;
}

//-----------------------------------------------------------------------------

void ArrayObject::push_back( const String &key, const String &value )
{
   mArray.push_back( Element( key, value ) );
}

//-----------------------------------------------------------------------------

void ArrayObject::push_front( const String &key, const String &value )
{
   mArray.push_front( Element( key, value ) );
}

//-----------------------------------------------------------------------------

void ArrayObject::insert( const String &key, const String &value, S32 index )
{
   index = mClamp( index, 0, mArray.size() );
   mArray.insert( index, Element( key, value ) );
}

//-----------------------------------------------------------------------------

void ArrayObject::pop_back()
{
   if(mArray.size() <= 0)
      return;

   mArray.pop_back();

   if( mCurrentIndex >= mArray.size() )
      mCurrentIndex = mArray.size() - 1;
}

//-----------------------------------------------------------------------------

void ArrayObject::pop_front()
{
   if( mArray.size() <= 0 )
      return;

   mArray.pop_front();
   
   if( mCurrentIndex >= mArray.size() )
      mCurrentIndex = mArray.size() - 1;
}

//-----------------------------------------------------------------------------

void ArrayObject::erase( S32 index )
{
   if(index < 0 || index >= mArray.size())
      return;

   mArray.erase( index );
}

//-----------------------------------------------------------------------------

void ArrayObject::empty()
{
   mArray.clear();
   mCurrentIndex = 0;
}

//-----------------------------------------------------------------------------

void ArrayObject::moveIndex(S32 prev, S32 index)
{
   if(index >= mArray.size())
      push_back(mArray[prev].key, mArray[prev].value);
   else
      mArray[index] = mArray[prev];
   mArray[prev].value = String::EmptyString;
   mArray[prev].key = String::EmptyString;
}

//-----------------------------------------------------------------------------

void ArrayObject::uniqueValue()
{
   for(S32 i=0; i<mArray.size(); i++)
   {
      for(S32 j=i+1; j<mArray.size(); j++)
      {
         if ( isEqual( mArray[i].value, mArray[j].value ) )
         {
            erase(j);
            j--;
         }
      }
   }
}

//-----------------------------------------------------------------------------

void ArrayObject::uniqueKey()
{
   for(S32 i=0; i<mArray.size(); i++)
   {
      for(S32 j=i+1; j<mArray.size(); j++)
      {
         if( isEqual( mArray[i].key, mArray[j].key ) )
         {
            erase(j);
            j--;
         }
      }
   }
}

//-----------------------------------------------------------------------------

void ArrayObject::duplicate(ArrayObject* obj)
{
   empty();
   for(S32 i=0; i<obj->count(); i++)
   {
      const String& tempval = obj->getValueFromIndex(i);
      const String& tempkey = obj->getKeyFromIndex(i);
      push_back(tempkey, tempval);
   }
   mCurrentIndex = obj->getCurrent();
}

//-----------------------------------------------------------------------------

void ArrayObject::crop( ArrayObject *obj )
{
   for( S32 i = 0; i < obj->count(); i++ )
   {
      const String &tempkey = obj->getKeyFromIndex( i );
      for( S32 j = 0; j < mArray.size(); j++ )
      {
         if( isEqual( mArray[j].key, tempkey ) )
         {
            mArray.erase( j );
            j--;
         }
      }
   }
}

//-----------------------------------------------------------------------------

void ArrayObject::append(ArrayObject* obj)
{
   for(S32 i=0; i<obj->count(); i++)
   {
      const String& tempval = obj->getValueFromIndex(i);
      const String& tempkey = obj->getKeyFromIndex(i);
      push_back(tempkey, tempval);
   }
}

//-----------------------------------------------------------------------------

void ArrayObject::setKey( const String &key, S32 index )
{
   if ( index >= mArray.size() )
      return;

   mArray[index].key = key;
}

//-----------------------------------------------------------------------------

void ArrayObject::setValue( const String &value, S32 index )
{
   if ( index >= mArray.size() )
      return;
   
   mArray[index].value = value;
}

//-----------------------------------------------------------------------------

void ArrayObject::sort( bool valsort, bool asc, bool numeric )
{
   if ( mArray.size() <= 1 )
      return;

   smDecreasing = asc ? false : true;
   smCaseSensitive = isCaseSensitive();

   if ( numeric )
   {
      if ( valsort )
         dQsort( (void *)&(mArray[0]), mArray.size(), sizeof(Element), _valueNumCompare) ;
      else
         dQsort( (void *)&(mArray[0]), mArray.size(), sizeof(Element), _keyNumCompare );
   }
   else
   {
      if( valsort )
         dQsort( (void *)&(mArray[0]), mArray.size(), sizeof(Element), _valueCompare );
      else
         dQsort( (void *)&(mArray[0]), mArray.size(), sizeof(Element), _keyCompare );
   }
}

//-----------------------------------------------------------------------------

void ArrayObject::sort( bool valsort, bool asc, const char* callbackFunctionName )
{
   if( mArray.size() <= 1 )
      return;

   smDecreasing = asc ? false : true;
   smCompareFunction = callbackFunctionName;

   if( valsort )
      dQsort( ( void* ) &( mArray[ 0 ] ), mArray.size(), sizeof( Element ), _valueFunctionCompare ) ;
   else
      dQsort( ( void* ) &( mArray[ 0 ] ), mArray.size(), sizeof( Element ), _keyFunctionCompare );

   smCompareFunction = NULL;
}

//-----------------------------------------------------------------------------

S32 ArrayObject::moveFirst()
{
   mCurrentIndex = 0;
   return mCurrentIndex;
}

//-----------------------------------------------------------------------------

S32 ArrayObject::moveLast()
{
   if ( mArray.empty() )
      mCurrentIndex = 0;
   else
      mCurrentIndex = mArray.size() - 1;
   return mCurrentIndex;
}

//-----------------------------------------------------------------------------

S32 ArrayObject::moveNext()
{
   if ( mCurrentIndex >= mArray.size() - 1 )
      return -1;
   
   mCurrentIndex++;
   
   return mCurrentIndex;
}

//-----------------------------------------------------------------------------

S32 ArrayObject::movePrev()
{
   if ( mCurrentIndex <= 0 )
      return -1;

   mCurrentIndex--;
   
   return mCurrentIndex;
}

//-----------------------------------------------------------------------------

void ArrayObject::setCurrent( S32 idx )
{
   if ( idx < 0 || idx >= mArray.size() )
   {
      Con::errorf( "ArrayObject::setCurrent( %d ) is out of the array bounds!", idx );
      return;
   }

   mCurrentIndex = idx;
}

//-----------------------------------------------------------------------------

void ArrayObject::echo()
{
   Con::printf( "ArrayObject Listing:" );
   Con::printf( "Index   Key       Value" );
   for ( U32 i = 0; i < mArray.size(); i++ )
   {
      const String& key = mArray[i].key;
      const String& val = mArray[i].value;
      Con::printf( "%d      [%s]    =>    %s", i, key.c_str(), val.c_str() );
   }
}

//=============================================================================
//    Console Methods.
//=============================================================================

DefineEngineMethod( ArrayObject, getIndexFromValue, S32, ( const char* value ),,
   "Search the array from the current position for the element "
   "@param value Array value to search for\n"
   "@return Index of the first element found, or -1 if none\n" )
{
   return object->getIndexFromValue( value );
}

DefineEngineMethod( ArrayObject, getIndexFromKey, S32, ( const char* key ),,
   "Search the array from the current position for the key "
   "@param value Array key to search for\n"
   "@return Index of the first element found, or -1 if none\n" )
{
   return object->getIndexFromKey( key );
}

DefineEngineMethod( ArrayObject, getValue, const char*, ( S32 index ),,
   "Get the value of the array element at the submitted index.\n"
   "@param index 0-based index of the array element to get\n"
   "@return The value of the array element at the specified index, "
   "or \"\" if the index is out of range\n" )
{
   return object->getValueFromIndex( index ).c_str();
}

DefineEngineMethod( ArrayObject, getKey, const char*, ( S32 index ),,
   "Get the key of the array element at the submitted index.\n"
   "@param index 0-based index of the array element to get\n"
   "@return The key associated with the array element at the "
   "specified index, or \"\" if the index is out of range\n" )
{
   return object->getKeyFromIndex( index ).c_str();
}

DefineEngineMethod( ArrayObject, setKey, void, ( const char* key, S32 index ),,
   "Set the key at the given index.\n"
   "@param key New key value\n"
   "@param index 0-based index of the array element to update\n" )
{
   object->setKey( key, index );
}

DefineEngineMethod( ArrayObject, setValue, void, ( const char* value, S32 index ),,
   "Set the value at the given index.\n"
   "@param value New array element value\n"
   "@param index 0-based index of the array element to update\n" )
{
   object->setValue( value, index );
}

DefineEngineMethod( ArrayObject, count, S32, (),,
   "Get the number of elements in the array." )
{
   return (S32)object->count();
}

DefineEngineMethod( ArrayObject, countValue, S32, ( const char* value ),,
   "Get the number of times a particular value is found in the array.\n"
   "@param value Array element value to count\n" )
{
   return (S32)object->countValue( value );
}

DefineEngineMethod( ArrayObject, countKey, S32, ( const char* key ),,
   "Get the number of times a particular key is found in the array.\n"
   "@param key Key value to count\n" )
{
   return (S32)object->countKey( key );
}

DefineEngineMethod( ArrayObject, add, void, ( const char* key, const char* value ), ( "" ),
   "Adds a new element to the end of an array (same as push_back()).\n"
   "@param key Key for the new element\n"
   "@param value Value for the new element\n" )
{
   object->push_back( key, value );
}

DefineEngineMethod( ArrayObject, push_back, void, ( const char* key, const char* value ), ( "" ),
   "Adds a new element to the end of an array.\n"
   "@param key Key for the new element\n"
   "@param value Value for the new element\n" )
{
   object->push_back( key, value );
}

DefineEngineMethod( ArrayObject, push_front, void, ( const char* key, const char* value ), ( "" ),
   "Adds a new element to the front of an array" )
{
   object->push_front( key, value );
}

DefineEngineMethod( ArrayObject, insert, void, ( const char* key, const char* value, S32 index ),,
   "Adds a new element to a specified position in the array.\n"
   "- @a index = 0 will insert an element at the start of the array (same as push_front())\n"
   "- @a index = %array.count() will insert an element at the end of the array (same as push_back())\n\n"
   "@param key Key for the new element\n"
   "@param value Value for the new element\n"
   "@param index 0-based index at which to insert the new element" )
{
   object->insert( key, value, index );
}

DefineEngineMethod( ArrayObject, pop_back, void, (),,
   "Removes the last element from the array" )
{
   object->pop_back();
}

DefineEngineMethod( ArrayObject, pop_front, void, (),,
   "Removes the first element from the array" )
{
   object->pop_front();
}

DefineEngineMethod( ArrayObject, erase, void, ( S32 index ),,
   "Removes an element at a specific position from the array.\n"
   "@param index 0-based index of the element to remove\n" )
{
   object->erase( index );
}

DefineEngineMethod( ArrayObject, empty, void, (),,
   "Emptys all elements from an array" )
{
   object->empty();
}

DefineEngineMethod( ArrayObject, uniqueValue, void, (),,
   "Removes any elements that have duplicated values (leaving the first instance)" )
{
   object->uniqueValue();
}

DefineEngineMethod( ArrayObject, uniqueKey, void, (),,
   "Removes any elements that have duplicated keys (leaving the first instance)" )
{
   object->uniqueKey();
}

DefineEngineMethod( ArrayObject, duplicate, bool, ( ArrayObject* target ),,
   "Alters array into an exact duplicate of the target array.\n"
   "@param target ArrayObject to duplicate\n" )
{
   if ( target )
   {
      object->duplicate( target );
      return true;
   }

   return false;
}

DefineEngineMethod( ArrayObject, crop, bool, ( ArrayObject* target ),,
   "Removes elements with matching keys from array.\n"
   "@param target ArrayObject containing keys to remove from this array\n" )
{
   if ( target )
   {
      object->crop( target );
      return true;
   }

   return false;
}

DefineEngineMethod( ArrayObject, append, bool, ( ArrayObject* target ),,
   "Appends the target array to the array object.\n"
   "@param target ArrayObject to append to the end of this array\n" )
{
   if ( target )
   {
      object->append( target );
      return true;
   }

   return false;
}

DefineEngineMethod( ArrayObject, sort, void, ( bool ascending ), ( false ),
   "Alpha sorts the array by value\n\n"
   "@param ascending [optional] True for ascending sort, false for descending sort\n" )
{
   object->sort( true, ascending, false );
}

DefineEngineMethod( ArrayObject, sorta, void, (),,
   "Alpha sorts the array by value in ascending order" )
{
   object->sort( true, true, false );
}

DefineEngineMethod( ArrayObject, sortd, void, (),,
   "Alpha sorts the array by value in descending order" )
{
   object->sort( true, false, false );
}

DefineEngineMethod( ArrayObject, sortk, void, ( bool ascending ), ( false ),
   "Alpha sorts the array by key\n\n"
   "@param ascending [optional] True for ascending sort, false for descending sort\n" )
{
   object->sort( false, ascending, false );
}

DefineEngineMethod( ArrayObject, sortka, void, (),,
   "Alpha sorts the array by key in ascending order" )
{
   object->sort( false, true, false );
}

DefineEngineMethod( ArrayObject, sortkd, void, (),,
   "Alpha sorts the array by key in descending order" )
{
   object->sort( false, false, false );
}

DefineEngineMethod( ArrayObject, sortn, void, ( bool ascending ), ( false ),
   "Numerically sorts the array by value\n\n"
   "@param ascending [optional] True for ascending sort, false for descending sort\n" )
{
   object->sort( true, ascending, true );
}

DefineEngineMethod( ArrayObject, sortna, void, (),,
   "Numerically sorts the array by value in ascending order" ) 
{
   object->sort( true, true, true );
}

DefineEngineMethod( ArrayObject, sortnd, void, (),,
   "Numerically sorts the array by value in descending order" )
{
   object->sort( true, false, true );
}

DefineEngineMethod( ArrayObject, sortnk, void, ( bool ascending ), ( false ),
   "Numerically sorts the array by key\n\n"
   "@param ascending [optional] True for ascending sort, false for descending sort\n" )
{
   object->sort( false, ascending, true );
}

DefineEngineMethod( ArrayObject, sortnka, void, (),,
   "Numerical sorts the array by key in ascending order" )
{
   object->sort( false, true, true );
}

DefineEngineMethod( ArrayObject, sortnkd, void, (),,
   "Numerical sorts the array by key in descending order" )
{
   object->sort( false, false, true );
}

DefineEngineMethod( ArrayObject, sortf, void,  ( const char* functionName ),,
   "Sorts the array by value in ascending order using the given callback function.\n"
   "@param functionName Name of a function that takes two arguments A and B and returns -1 if A is less, 1 if B is less, and 0 if both are equal.\n\n"
   "@tsexample\n"
   "function mySortCallback(%a, %b)\n"
   "{\n"
   "   return strcmp( %a.name, %b.name );\n"
   "}\n\n"
   "%array.sortf( \"mySortCallback\" );\n"
   "@endtsexample\n" )
{
   object->sort( true, true, functionName );
}

DefineEngineMethod( ArrayObject, sortfk, void,  ( const char* functionName ),,
   "Sorts the array by key in ascending order using the given callback function.\n"
   "@param functionName Name of a function that takes two arguments A and B and returns -1 if A is less, 1 if B is less, and 0 if both are equal."
   "@see sortf\n" )
{
   object->sort( false, true, functionName );
}

DefineEngineMethod( ArrayObject, sortfd, void, ( const char* functionName ),,
   "Sorts the array by value in descending order using the given callback function.\n"
   "@param functionName Name of a function that takes two arguments A and B and returns -1 if A is less, 1 if B is less, and 0 if both are equal."
   "@see sortf\n" )
{
   object->sort( true, false, functionName );
}

DefineEngineMethod( ArrayObject, sortfkd, void, ( const char* functionName ),,
   "Sorts the array by key in descending order using the given callback function.\n"
   "@param functionName Name of a function that takes two arguments A and B and returns -1 if A is less, 1 if B is less, and 0 if both are equal."
   "@see sortf\n" )
{
   object->sort( false, false, functionName );
}

DefineEngineMethod( ArrayObject, moveFirst, S32, (),,
   "Moves array pointer to start of array\n\n"
   "@return Returns the new array pointer" )
{
   return object->moveFirst();
}

DefineEngineMethod( ArrayObject, moveLast, S32, (),,
   "Moves array pointer to end of array\n\n"
   "@return Returns the new array pointer" )
{
   return object->moveLast();
}

DefineEngineMethod( ArrayObject, moveNext, S32, (),,
   "Moves array pointer to next position\n\n"
   "@return Returns the new array pointer, or -1 if already at the end" )
{
   return object->moveNext();
}

DefineEngineMethod( ArrayObject, movePrev, S32, (),,
   "Moves array pointer to prev position\n\n"
   "@return Returns the new array pointer, or -1 if already at the start" )
{
   return object->movePrev();
}

DefineEngineMethod( ArrayObject, getCurrent, S32, (),,
   "Gets the current pointer index" )
{
   return object->getCurrent();
}

DefineEngineMethod( ArrayObject, setCurrent, void, ( S32 index ),,
   "Sets the current pointer index.\n"
   "@param index New 0-based pointer index\n" )
{
   object->setCurrent( index );
}

DefineEngineMethod( ArrayObject, echo, void, (),,
   "Echos the array contents to the console" )
{
   object->echo();
}
