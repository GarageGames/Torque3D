//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

#ifdef TORQUE_TESTS_ENABLED
#include "testing/unitTesting.h"
#include "core/util/str.h"
#include "core/util/tVector.h"
#include "core/strings/stringFunctions.h"
#include "core/strings/unicode.h"

/// This is called Str, not String, because googletest doesn't let you use both
/// TEST(x) and TEST_FIX(x). So this fixture is called Str, to match the StrTest
/// struct, and the remaining fixture-les tests are named String.
FIXTURE(Str)
{
protected:
   struct StrTest
   {
      const UTF8* mData;
      const UTF16* mUTF16;
      U32 mLength;

      StrTest() : mData( 0 ), mUTF16( 0 ), mLength( 0 ) {}
      StrTest( const char* str )
         : mData( str ), mUTF16( NULL ), mLength( str ? dStrlen( str ) : 0 )
      {
         if( str )
            mUTF16 = createUTF16string( mData );
      }
      ~StrTest()
      {
         if( mUTF16 )
            delete [] mUTF16;
      }
   };

   Vector< StrTest* > mStrings;

   virtual void SetUp()
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
   }

   virtual void TearDown()
   {
      for( U32 i = 0; i < mStrings.size(); ++ i )
         delete mStrings[ i ];
      mStrings.clear();
   }
};

#define EACH_STRING(i) \
   for( U32 i = 0; i < mStrings.size(); ++ i )
#define EACH_PAIR(i, j) \
   for( U32 i = 0; i < mStrings.size(); ++ i ) \
      for( U32 j = 0; j < mStrings.size(); ++ j )

TEST_FIX(Str, Test1)
{
   EACH_STRING(i)
   {
      StrTest& data = *mStrings[i];
      String str( data.mData );
      String str16( data.mUTF16 );

      EXPECT_TRUE( str.length() == data.mLength );
      EXPECT_TRUE( str.size() == data.mLength + 1 );
      EXPECT_TRUE( str.isEmpty() || str.length() > 0 );
      EXPECT_TRUE( str.length() == str16.length() );
      EXPECT_TRUE( str.size() == str16.size() );

      EXPECT_TRUE( dMemcmp( str.utf16(), str16.utf16(), str.length() * sizeof( UTF16 ) ) == 0 );
      EXPECT_TRUE( !data.mData || dMemcmp( str.utf16(), data.mUTF16, str.length() * sizeof( UTF16 ) ) == 0 );
      EXPECT_TRUE( !data.mData || dMemcmp( str16.utf8(), data.mData, str.length() ) == 0 );

      EXPECT_TRUE( !data.mData || dStrcmp( str.utf8(), data.mData ) == 0 );
      EXPECT_TRUE( !data.mData || dStrcmp( str.utf16(), data.mUTF16 ) == 0 );
   }
}

TEST_FIX(Str, Test2)
{
   EACH_STRING(i)
   {
      StrTest& data = *mStrings[i];
      String str( data.mData );

      EXPECT_TRUE( str == str );
      EXPECT_FALSE( str != str );
      EXPECT_FALSE( str < str );
      EXPECT_FALSE( str > str );
      EXPECT_TRUE( str.equal( str ) );
      EXPECT_TRUE( str.equal( str, String::NoCase ) );
   }
}

TEST_FIX(Str, Test3)
{
   EACH_PAIR(i, j)
   {
      StrTest& d1 = *mStrings[i];
      StrTest& d2 = *mStrings[j];

      if( &d1 != &d2 )
         EXPECT_TRUE( String( d1.mData ) != String( d2.mData )
                        || ( String( d1.mData ).isEmpty() && String( d2.mData ).isEmpty() ) );
      else
         EXPECT_TRUE( String( d1.mData ) == String( d2.mData ) );
   }
}

TEST(String, Empty)
{
   EXPECT_TRUE( String().length() == 0 );
   EXPECT_TRUE( String( "" ).length() == 0 );
   EXPECT_TRUE( String().size() == 1 );
   EXPECT_TRUE( String( "" ).size() == 1 );
   EXPECT_TRUE( String().isEmpty() );
   EXPECT_TRUE( String( "" ).isEmpty() );
}

TEST(String, Trim)
{
   EXPECT_TRUE( String( " Foobar Barfoo  \n\t " ).trim() == String( "Foobar Barfoo" ) );
   EXPECT_TRUE( String( "Foobar" ).trim() == String( "Foobar" ) );
   EXPECT_TRUE( String( "    " ).trim().isEmpty() );
}

TEST(String, Compare)
{
   String str( "Foobar" );

   EXPECT_TRUE( str.compare( "Foo", 3 ) == 0 );
   EXPECT_TRUE( str.compare( "bar", 3, String::NoCase | String::Right ) == 0 );
   EXPECT_TRUE( str.compare( "foo", 3, String::NoCase ) == 0 );
   EXPECT_TRUE( str.compare( "BAR", 3, String::NoCase | String::Right ) == 0 );
   EXPECT_TRUE( str.compare( "Foobar" ) == 0 );
   EXPECT_TRUE( str.compare( "Foo" ) != 0 );
   EXPECT_TRUE( str.compare( "foobar", 0, String::NoCase ) == 0 );
   EXPECT_TRUE( str.compare( "FOOBAR", 0, String::NoCase ) == 0 );
   EXPECT_TRUE( str.compare( "Foobar", 0, String::Right ) == 0 );
   EXPECT_TRUE( str.compare( "foobar", 0, String::NoCase | String::Right ) == 0 );
}

TEST(String, Order)
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
         EXPECT_TRUE( strs[ j ] < strs[ i ] );
         EXPECT_TRUE( strs[ i ] > strs[ j ] );

         EXPECT_TRUE( !( strs[ j ] > strs[ i ] ) );
         EXPECT_TRUE( !( strs[ i ] < strs[ i ] ) );

         EXPECT_TRUE( strs[ j ] <= strs[ i ] );
         EXPECT_TRUE( strs[ i ] >= strs[ j ] );
      }

      EXPECT_TRUE( !( strs[ i ] < strs[ i ] ) );
      EXPECT_TRUE( !( strs[ i ] > strs[ i ] ) );
      EXPECT_TRUE( strs[ i ] <= strs[ i ] );
      EXPECT_TRUE( strs[ i ] >= strs[ i ] );

      for( U32 j = i + 1; j < strs.size(); ++ j )
      {
         EXPECT_TRUE( strs[ j ] > strs[ i ] );
         EXPECT_TRUE( strs[ i ] < strs[ j ] );

         EXPECT_TRUE( !( strs[ j ] < strs[ i ] ) );
         EXPECT_TRUE( !( strs[ i ] > strs[ j ] ) );

         EXPECT_TRUE( strs[ j ] >= strs[ i ] );
         EXPECT_TRUE( strs[ i ] <= strs[ j ] );
      }
   }
}

/// TODO
TEST(String, Find)
{
}

TEST(String, Insert)
{
   // String.insert( Pos, Char )
   EXPECT_TRUE( String( "aa" ).insert( 1, 'c' ) == String( "aca" ) );
      
   // String.insert( Pos, String )
   EXPECT_TRUE( String( "aa" ).insert( 1, "cc" ) == String( "acca" ) );
   EXPECT_TRUE( String( "aa" ).insert( 1, String( "cc" ) ) == String( "acca" ) );
      
   // String.insert( Pos, String, Len )
   EXPECT_TRUE( String( "aa" ).insert( 1, "ccdddd", 2 ) == String( "acca" ) );
}

TEST(String, Erase)
{
   EXPECT_TRUE( String( "abba" ).erase( 1, 2 ) == String( "aa" ) );
   EXPECT_TRUE( String( "abba" ).erase( 0, 4 ).isEmpty() );
}

TEST(String, Replace)
{
   // String.replace( Pos, Len, String )
   EXPECT_TRUE( String( "abba" ).replace( 1, 2, "ccc" ) == String( "accca" ) );
   EXPECT_TRUE( String( "abba" ).replace( 1, 2, String( "ccc" ) ) == String( "accca" ) );
   EXPECT_TRUE( String( "abba" ).replace( 0, 4, "" ).isEmpty() );
   EXPECT_TRUE( String( "abba" ).replace( 2, 2, "c" ) == String( "abc" ) );

   // String.replace( Char, Char )
   EXPECT_TRUE( String().replace( 'a', 'b' ).isEmpty() );
   EXPECT_TRUE( String( "ababc" ).replace( 'a', 'b' ) == String( "bbbbc" ) );
   EXPECT_TRUE( String( "ababc" ).replace( 'd', 'e' ) == String( "ababc" ) );

   // String.replace( String, String )
   EXPECT_TRUE( String().replace( "foo", "bar" ).isEmpty() );
   EXPECT_TRUE( String( "foobarfoo" ).replace( "foo", "bar" ) == String( "barbarbar" ) );
   EXPECT_TRUE( String( "foobar" ).replace( "xx", "yy" ) == String( "foobar" ) );
   EXPECT_TRUE( String( "foofoofoo" ).replace( "foo", "" ).isEmpty() );
}

TEST(String, SubStr)
{
   EXPECT_TRUE( String( "foobar" ).substr( 0, 3 ) == String( "foo" ) );
   EXPECT_TRUE( String( "foobar" ).substr( 3 ) == String( "bar" ) );
   EXPECT_TRUE( String( "foobar" ).substr( 2, 2 ) == String( "ob" ) );
   EXPECT_TRUE( String( "foobar" ).substr( 2, 0 ).isEmpty() );
   EXPECT_TRUE( String( "foobar" ).substr( 0, 6 ) == String( "foobar" ) );
}

TEST(String, ToString)
{
   EXPECT_TRUE( String::ToString( U32( 1 ) ) == String( "1" ) );
   EXPECT_TRUE( String::ToString( S32( -1 ) ) == String( "-1" ) );
   EXPECT_TRUE( String::ToString( F32( 1.01 ) ) == String( "1.01" ) );
   EXPECT_TRUE( String::ToString( "%s%i", "foo", 1 ) == String( "foo1" ) );
}

TEST(String, CaseConversion)
{
   EXPECT_TRUE( String::ToLower( "foobar123." ) == String( "foobar123." ) );
   EXPECT_TRUE( String::ToLower( "FOOBAR123." ) == String( "foobar123." ) );
   EXPECT_TRUE( String::ToUpper( "barfoo123." ) == String( "BARFOO123." ) );
   EXPECT_TRUE( String::ToUpper( "BARFOO123." ) == String( "BARFOO123." ) );
}

TEST(String, Concat)
{
   EXPECT_TRUE( String( "foo" ) + String( "bar" ) == String( "foobar" ) );
   EXPECT_TRUE( String() + String( "bar" ) == String( "bar" ) );
   EXPECT_TRUE( String( "foo" ) + String() == String( "foo" ) );
   EXPECT_TRUE( String() + String() == String() );
   EXPECT_TRUE( String( "fo" ) + 'o' == String( "foo" ) );
   EXPECT_TRUE( 'f' + String( "oo" ) == String( "foo" ) );
   EXPECT_TRUE( String( "foo" ) + "bar" == String( "foobar" ) );
   EXPECT_TRUE( "foo" + String( "bar" ) == String( "foobar" ) );
}

TEST(String, Hash)
{
   EXPECT_TRUE( String( "foo" ).getHashCaseSensitive() == String( "foo" ).getHashCaseSensitive() );
   EXPECT_TRUE( String( "foo" ).getHashCaseSensitive() != String( "bar" ).getHashCaseSensitive() );
   EXPECT_TRUE( String( "foo" ).getHashCaseInsensitive() == String( "FOO" ).getHashCaseInsensitive() );
}
   
TEST(String, Intern)
{
   EXPECT_TRUE( String( "foo" ).intern().isSame( String( "foo" ).intern() ) );
   EXPECT_TRUE( !String( "foo" ).intern().isSame( String( "bar" ).intern() ) );
   EXPECT_TRUE( !String( "foo" ).intern().isSame( String( "Foo" ).intern() ) );
   EXPECT_TRUE( String( "foo" ).intern() == String( "foo" ).intern() );
   EXPECT_TRUE( String( "foo" ).intern() != String( "bar" ).intern() );
   EXPECT_TRUE( String( "foo" ).intern().isInterned() );
}

TEST(StringBuilder, StringBuilder)
{
   StringBuilder str;

   str.append( 'f' );
   str.append( "oo" );
   str.append( String( "ba" ) );
   str.append( "rfajskfdj", 1 );
   str.format( "%s", "barfoo" );

   EXPECT_TRUE( str.end() == String( "foobarbarfoo" ) );
}

#endif
