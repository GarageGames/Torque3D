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

#ifndef _ARRAYOBJECT_H_
#define _ARRAYOBJECT_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

// This class is based on original code by community
// member Daniel Neilsen:
//
// http://www.garagegames.com/community/resources/view/4711


/// A data structure holding indexed sequences of key/value pairs for script use.
class ArrayObject : public SimObject
{
   typedef SimObject Parent;

protected:

   struct Element
   {
      String key;
      String value;
      Element() { }
      Element( const String& _key, const String& _value ) : key(_key), value(_value) { }
   };

   bool mCaseSensitive;
   S32 mCurrentIndex;
   
   /// The array of key/value pairs.
   Vector< Element > mArray;
   
   /// @name Sorting
   /// @{

   static bool smDecreasing;
   static bool smCaseSensitive;
   static const char* smCompareFunction;

   static S32 QSORT_CALLBACK _valueCompare( const void *a, const void *b );
   static S32 QSORT_CALLBACK _valueNumCompare( const void *a, const void *b );
   static S32 QSORT_CALLBACK _keyCompare( const void *a, const void *b );
   static S32 QSORT_CALLBACK _keyNumCompare( const void *a, const void *b );
   static S32 QSORT_CALLBACK _keyFunctionCompare( const void* a, const void* b );
   static S32 QSORT_CALLBACK _valueFunctionCompare( const void* a, const void* b );
   
   /// @}

   static bool _addKeyFromField( void *object, const char *index, const char *data );

public:
  
   ArrayObject();

   /// @name Data Query 
   /// @{
   
   /// Returns true if string handling by the array is case-sensitive.
   bool isCaseSensitive() const { return mCaseSensitive; }

   bool isEqual( const String &valA, const String &valB ) const
   {
      return valA.equal( valB, isCaseSensitive() ? String::Case : String::NoCase );
   }

   /// Searches the array for the first matching value from the 
   /// current array position.  It will return -1 if no matching
   /// index is found.
   S32 getIndexFromValue( const String &value ) const;

   /// Searches the array for the first matching key from the current
   /// array position.  It will return -1 if no matching index found.
   S32 getIndexFromKey( const String &key ) const;
   
   /// Returns the key for a given index.
   /// Will return a null value for an invalid index
   const String& getKeyFromIndex( S32 index ) const;

   /// Returns the value for a given index.
   /// Will return a null value for an invalid index
   const String&	getValueFromIndex( S32 index ) const;
   
   ///
   S32 getIndexFromKeyValue( const String &key, const String &value ) const;

   /// Counts the number of elements in the array
   S32 count() const { return mArray.size(); }

   /// Counts the number of instances of a particular value in the array
   S32 countValue( const String &value ) const;

   /// Counts the number of instances of a particular key in the array
   S32 countKey( const String &key ) const;

   /// @}

   /// @name Data Alteration
   /// @{

   /// Adds a new array item to the end of the array
   void push_back( const String &key, const String &value );

   /// Adds a new array item to the front of the array
   void push_front( const String &key, const String &value );

   /// Adds a new array item to a particular index of the array
   void insert( const String &key, const String &value, S32 index );

   /// Removes an array item from the end of the array
   void pop_back();

   /// Removes an array item from the end of the array
   void pop_front();

   /// Removes an array item from a particular index of the array
   void erase( S32 index );

   /// Clears an array
   void empty();

   /// Moves a key and value from one index location to another.
   void moveIndex( S32 prev, S32 index );

   /// @}

   /// @name Complex Data Alteration
   /// @{

   /// Removes any duplicate values from the array
   /// (keeps the first instance only)
   void uniqueValue();

   /// Removes any duplicate keys from the array
   /// (keeps the first instance only)
   void uniqueKey();

   /// Makes this array an exact duplicate of another array
   void duplicate( ArrayObject *obj );

   /// Crops the keys that exists in the target array from our current array
   void crop( ArrayObject *obj );

   /// Appends the target array to our current array
   void append( ArrayObject *obj );

   /// Sets the key at the given index
   void setKey( const String &key, S32 index );

   /// Sets the key at the given index
   void setValue( const String &value, S32 index );

   /// This sorts the array.
   /// @param valtest  Determines whether sorting by value or key.
   /// @param asc      Determines if sorting ascending or descending.
   /// @param numeric  Determines if sorting alpha or numeric search.
   void sort( bool valtest, bool asc, bool numeric );
   
   /// This sorts the array using a script callback.
   /// @param valtest  Determines whether sorting by value or key.
   /// @param asc      Determines if sorting ascending or descending.
   /// @param callbackFunctionName Name of the script function.
   void sort( bool valtest, bool asc, const char* callbackFunctionName );

   /// @}

   /// @name Pointer Manipulation
   /// @{

   /// Moves pointer to arrays first position
   S32 moveFirst();

   /// Moves pointer to arrays last position
   S32 moveLast();

   /// Moves pointer to arrays next position
   /// If last position it returns -1 and no move occurs;
   S32 moveNext();

   /// Moves pointer to arrays prev position
   /// If first position it returns -1 and no move occurs;
   S32 movePrev();

   /// Returns current pointer index.
   S32 getCurrent() const { return mCurrentIndex; }

   ///
   void setCurrent( S32 idx );

   /// @}


   /// @name Data Listing
   /// @{

   /// Echos the content of the array to the console.
   void echo();

   /// @}

   // SimObject
   DECLARE_CONOBJECT( ArrayObject );
   DECLARE_CATEGORY( "Core" );
   DECLARE_DESCRIPTION( "An object storing an indexed sequence of key/value pairs." );
   
   static void initPersistFields();
};

#endif // _ARRAYOBJECT_H_