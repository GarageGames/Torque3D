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

#include "unit/test.h"
#include "core/util/str.h"
#include "core/util/tVector.h"
#include "core/strings/stringFunctions.h"
#include "core/strings/unicode.h"


#ifndef TORQUE_SHIPPING

using namespace UnitTesting;

#define TEST( x ) test( ( x ), "FAIL: " #x )
#define XTEST( t, x ) t->test( ( x ), "FAIL: " #x )

CreateUnitTest( TestString, "Util/String" )
{
   struct StrTest
   {
      const UTF8* mData;
      const UTF16* mUTF16;
      U32 mLength;

      StrTest() : mData( 0 ), mUTF16( 0 ) {}
      StrTest( const char* str )
         : mData( str ), mLength( str ? dStrlen( str ) : 0 ), mUTF16( NULL )
      {
         if( str )
            mUTF16 = convertUTF8toUTF16( mData );
      }
      ~StrTest()
      {
         if( mUTF16 )
            delete [] mUTF16;
      }
   };

   Vector< StrTest* > mStrings;

   template< class T >
   void runTestOnStrings()
   {
      for( U32 i = 0; i < mStrings.size(); ++ i )
         T::run( this, *mStrings[ i ] );
   }
   template< class T >
   void runPairwiseTestOnStrings()
   {
      for( U32 i = 0; i < mStrings.size(); ++ i )
         for( U32 j = 0; j < mStrings.size(); ++ j )
            T::run( this, *mStrings[ i ], *mStrings[ j ] );
   }

   struct Test1
   {
      static void run( TestString* test, StrTest& data )
      {
         String str( data.mData );
         String str16( data.mUTF16 );

         XTEST( test, str.length() == data.mLength );
         XTEST( test, str.size() == data.mLength + 1 );
         XTEST( test, str.isEmpty() || str.length() > 0 );
         XTEST( test, str.length() == str16.length() );
         XTEST( test, str.size() == str16.size() );

         XTEST( test, dMemcmp( str.utf16(), str16.utf16(), str.length() * sizeof( UTF16 ) ) == 0 );
         XTEST( test, !data.mData || dMemcmp( str.utf16(), data.mUTF16, str.length() * sizeof( UTF16 ) ) == 0 );
         XTEST( test, !data.mData || dMemcmp( str16.utf8(), data.mData, str.length() ) == 0 );

         XTEST( test, !data.mData || dStrcmp( str.utf8(), data.mData ) == 0 );
         XTEST( test, !data.mData || dStrcmp( str.utf16(), data.mUTF16 ) == 0 );
      }
   };

   struct Test2
   {
      static void run( TestString* test, StrTest& data )
      {
         String str( data.mData );

         XTEST( test, str == str );
         XTEST( test, !( str != str ) );
         XTEST( test, !( str < str ) );
         XTEST( test, !( str > str ) );
         XTEST( test, str.equal( str ) );
         XTEST( test, str.equal( str, String::NoCase ) );
      }
   };

   struct Test3
   {
      static void run( TestString* test, StrTest& d1, StrTest& d2 )
      {
         if( &d1 != &d2 )
            XTEST( test, String( d1.mData ) != String( d2.mData )
                         || ( String( d1.mData ).isEmpty() && String( d2.mData ).isEmpty() ) );
         else
            XTEST( test, String( d1.mData ) == String( d2.mData ) );
      }
   };

   void testEmpty()
   {
      TEST( String().length() == 0 );
      TEST( String( "" ).length() == 0 );
      TEST( String().size() == 1 );
      TEST( String( "" ).size() == 1 );
      TEST( String().isEmpty() );
      TEST( String( "" ).isEmpty() );
   }

   void testTrim()
   {
      TEST( String( " Foobar Barfoo  \n\t " ).trim() == String( "Foobar Barfoo" ) );
      TEST( String( "Foobar" ).trim() == String( "Foobar" ) );
      TEST( String( "    " ).trim().isEmpty() );
   }

   void testCompare()
   {
      String str( "Foobar" );

      TEST( str.compare( "Foo", 3 ) == 0 );
      TEST( str.compare( "bar", 3, String::NoCase | String::Right ) == 0 );
      TEST( str.compare( "foo", 3, String::NoCase ) == 0 );
      TEST( str.compare( "BAR", 3, String::NoCase | String::Right ) == 0 );
      TEST( str.compare( "Foobar" ) == 0 );
      TEST( str.compare( "Foo" ) != 0 );
      TEST( str.compare( "foobar", 0, String::NoCase ) == 0 );
      TEST( str.compare( "FOOBAR", 0, String::NoCase ) == 0 );
      TEST( str.compare( "Foobar", 0, String::Right ) == 0 );
      TEST( str.compare( "foobar", 0, String::NoCase | String::Right ) == 0 );
   }

   void testOrder()
   {
      Vector< String > strs;

      strs.push_back( "a" );
      strs.push_back( "a0" );
      strs.push_back( "a1" );
      strs.push_back( "a1a" );
      strs.push_back( "a1b" );
      strs.push_back( "a2" );
      strs.push_back( "a10" );
      strs.push_back( "a20" );

      for( U32 i = 0; i < strs.size(); ++ i )
      {
         for( U32 j = 0; j < i; ++ j )
         {
            TEST( strs[ j ] < strs[ i ] );
            TEST( strs[ i ] > strs[ j ] );

            TEST( !( strs[ j ] > strs[ i ] ) );
            TEST( !( strs[ i ] < strs[ i ] ) );

            TEST( strs[ j ] <= strs[ i ] );
            TEST( strs[ i ] >= strs[ j ] );
         }

         TEST( !( strs[ i ] < strs[ i ] ) );
         TEST( !( strs[ i ] > strs[ i ] ) );
         TEST( strs[ i ] <= strs[ i ] );
         TEST( strs[ i ] >= strs[ i ] );

         for( U32 j = i + 1; j < strs.size(); ++ j )
         {
            TEST( strs[ j ] > strs[ i ] );
            TEST( strs[ i ] < strs[ j ] );

            TEST( !( strs[ j ] < strs[ i ] ) );
            TEST( !( strs[ i ] > strs[ j ] ) );

            TEST( strs[ j ] >= strs[ i ] );
            TEST( strs[ i ] <= strs[ j ] );
         }
      }
   }

   void testFind()
   {
      //TODO
   }

   void testInsert()
   {
      // String.insert( Pos, Char )
      TEST( String( "aa" ).insert( 1, 'c' ) == String( "aca" ) );
      
      // String.insert( Pos, String )
      TEST( String( "aa" ).insert( 1, "cc" ) == String( "acca" ) );
      TEST( String( "aa" ).insert( 1, String( "cc" ) ) == String( "acca" ) );
      
      // String.insert( Pos, String, Len )
      TEST( String( "aa" ).insert( 1, "ccdddd", 2 ) == String( "acca" ) );
   }

   void testErase()
   {
      TEST( String( "abba" ).erase( 1, 2 ) == String( "aa" ) );
      TEST( String( "abba" ).erase( 0, 4 ).isEmpty() );
   }

   void testReplace()
   {
      // String.replace( Pos, Len, String )
      TEST( String( "abba" ).replace( 1, 2, "ccc" ) == String( "accca" ) );
      TEST( String( "abba" ).replace( 1, 2, String( "ccc" ) ) == String( "accca" ) );
      TEST( String( "abba" ).replace( 0, 4, "" ).isEmpty() );
      TEST( String( "abba" ).replace( 2, 2, "c" ) == String( "abc" ) );

      // String.replace( Char, Char )
      TEST( String().replace( 'a', 'b' ).isEmpty() );
      TEST( String( "ababc" ).replace( 'a', 'b' ) == String( "bbbbc" ) );
      TEST( String( "ababc" ).replace( 'd', 'e' ) == String( "ababc" ) );

      // String.replace( String, String )
      TEST( String().replace( "foo", "bar" ).isEmpty() );
      TEST( String( "foobarfoo" ).replace( "foo", "bar" ) == String( "barbarbar" ) );
      TEST( String( "foobar" ).replace( "xx", "yy" ) == String( "foobar" ) );
      TEST( String( "foofoofoo" ).replace( "foo", "" ).isEmpty() );
   }

   void testSubstr()
   {
      TEST( String( "foobar" ).substr( 0, 3 ) == String( "foo" ) );
      TEST( String( "foobar" ).substr( 3 ) == String( "bar" ) );
      TEST( String( "foobar" ).substr( 2, 2 ) == String( "ob" ) );
      TEST( String( "foobar" ).substr( 2, 0 ).isEmpty() );
      TEST( String( "foobar" ).substr( 0, 6 ) == String( "foobar" ) );
   }

   void testToString()
   {
      TEST( String::ToString( U32( 1 ) ) == String( "1" ) );
      TEST( String::ToString( S32( -1 ) ) == String( "-1" ) );
      TEST( String::ToString( F32( 1.01 ) ) == String( "1.01" ) );
      TEST( String::ToString( "%s%i", "foo", 1 ) == String( "foo1" ) );
   }

   void testCaseConversion()
   {
      TEST( String::ToLower( "foobar123." ) == String( "foobar123." ) );
      TEST( String::ToLower( "FOOBAR123." ) == String( "foobar123." ) );
      TEST( String::ToUpper( "barfoo123." ) == String( "BARFOO123." ) );
      TEST( String::ToUpper( "BARFOO123." ) == String( "BARFOO123." ) );
   }

   void testConcat()
   {
      TEST( String( "foo" ) + String( "bar" ) == String( "foobar" ) );
      TEST( String() + String( "bar" ) == String( "bar" ) );
      TEST( String( "foo" ) + String() == String( "foo" ) );
      TEST( String() + String() == String() );
      TEST( String( "fo" ) + 'o' == String( "foo" ) );
      TEST( 'f' + String( "oo" ) == String( "foo" ) );
      TEST( String( "foo" ) + "bar" == String( "foobar" ) );
      TEST( "foo" + String( "bar" ) == String( "foobar" ) );
   }

   void testHash()
   {
      TEST( String( "foo" ).getHashCaseSensitive() == String( "foo" ).getHashCaseSensitive() );
      TEST( String( "foo" ).getHashCaseSensitive() != String( "bar" ).getHashCaseSensitive() );
      TEST( String( "foo" ).getHashCaseInsensitive() == String( "FOO" ).getHashCaseInsensitive() );
   }
   
   void testIntern()
   {
      TEST( String( "foo" ).intern().isSame( String( "foo" ).intern() ) );
      TEST( !String( "foo" ).intern().isSame( String( "bar" ).intern() ) );
      TEST( !String( "foo" ).intern().isSame( String( "Foo" ).intern() ) );
      TEST( String( "foo" ).intern() == String( "foo" ).intern() );
      TEST( String( "foo" ).intern() != String( "bar" ).intern() );
      TEST( String( "foo" ).intern().isInterned() );
   }

   void run()
   {
      mStrings.push_back( new StrTest( NULL ) );
      mStrings.push_back( new StrTest( "" ) );
      mStrings.push_back( new StrTest( "Torque" ) );
      mStrings.push_back( new StrTest( "TGEA" ) );
      mStrings.push_back( new StrTest( "GarageGames" ) );
      mStrings.push_back( new StrTest( "TGB" ) );
      mStrings.push_back( new StrTest( "games" ) );
      mStrings.push_back( new StrTest( "engine" ) );
      mStrings.push_back( new StrTest( "rocks" ) );
      mStrings.push_back( new StrTest( "technology" ) );
      mStrings.push_back( new StrTest( "Torque 3D" ) );
      mStrings.push_back( new StrTest( "Torque 2D" ) );

      runTestOnStrings< Test1 >();
      runTestOnStrings< Test2 >();

      runPairwiseTestOnStrings< Test3 >();

      testEmpty();
      testTrim();
      testCompare();
      testOrder();
      testFind();
      testInsert();
      testReplace();
      testErase();
      testSubstr();
      testToString();
      testCaseConversion();
      testConcat();
      testHash();
      testIntern();

      for( U32 i = 0; i < mStrings.size(); ++ i )
         delete mStrings[ i ];
      mStrings.clear();
   }
};

CreateUnitTest( TestStringBuilder, "Util/StringBuilder" )
{
   void run()
   {
      StringBuilder str;

      str.append( 'f' );
      str.append( "oo" );
      str.append( String( "ba" ) );
      str.append( "rfajskfdj", 1 );
      str.format( "%s", "barfoo" );

      TEST( str.end() == String( "foobarbarfoo" ) );
   }
};

#endif // !TORQUE_SHIPPING
