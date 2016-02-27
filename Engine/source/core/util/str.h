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

#ifndef _TORQUE_STRING_H_
#define _TORQUE_STRING_H_

#include <cstdarg>

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif


template< class T > class Vector;


typedef UTF8 StringChar;


/// The String class represents a 0-terminated array of characters.
class String
{
public:
   class StringData;

   /// Default mode is case sensitive starting from the left
   enum Mode
   {
      Case = 0,         ///< Case sensitive
      NoCase = 1,       ///< Case insensitive
      Left = 0,         ///< Start at left end of string
      Right = 2,        ///< Start at right end of string
   };

   typedef U32 SizeType;
   typedef StringChar ValueType;

   static const SizeType NPos;   ///< Indicates 'not found' when using find() functions

   /// A predefined empty string.
   static const String EmptyString;

   String();
   String(const String &str);
   String(const StringChar *str);
   String(const StringChar *str, SizeType size); ///< Copy from raw data
   String(const UTF16 *str);
   ~String();

   const UTF8  *c_str() const;   ///< Return the string as a native type
   const UTF16 *utf16() const;
   const UTF8* utf8() const { return c_str(); }

   SizeType length() const;   ///< Returns the length of the string in bytes.
   SizeType size() const;     ///< Returns the length of the string in bytes including the NULL terminator.
   SizeType numChars() const; ///< Returns the length of the string in characters.
   bool     isEmpty() const;  ///< Is this an empty string [""]?
   static bool isEmpty(const char*); // is the input empty?
   bool     isNotEmpty() const { return !isEmpty(); }  ///< Is this not an empty string [""]?

   /// Erases all characters in a string.
   void clear() { *this = EmptyString; }

   bool     isShared() const; ///< Is this string's reference count greater than 1?
   bool     isSame( const String& str ) const; ///< Return true if both strings refer to the same shared data.

   U32   getHashCaseSensitive() const;    ///< Get the case-sensitive hash of the string [only calculates the hash as necessary]
   U32   getHashCaseInsensitive() const;  ///< Get the case-insensitive hash of the string  [only calculates the hash as necessary]

   String& operator=(StringChar);
   String& operator+=(StringChar);
   String& operator=(const StringChar*);
   String& operator+=(const StringChar*);
   String& operator=(const String&);
   String& operator+=(const String&);
   
   /**
      Compare this string with another.
      @param str  The string to compare against.
      @param len  If len is non-zero, then at most len characters are compared.
      @param mode Comparison mode.
      @return Difference between the first two characters that don't match.
   */
   S32 compare(const StringChar *str, SizeType len = 0, U32 mode = Case|Left) const;
   S32 compare(const String &str, SizeType len = 0, U32 mode = Case|Left) const; ///< @see compare(const StringChar *, SizeType, U32) const

   /**
      Compare two strings for equality.
      It will use the string hashes to determine inequality.
      @param str  The string to compare against.
      @param mode Comparison mode - case sensitive or not.
   */
   bool equal(const String &str, U32 mode = Case) const;

   SizeType find(StringChar c, SizeType pos = 0, U32 mode = Case|Left) const;
   SizeType find(const StringChar *str, SizeType pos = 0, U32 mode = Case|Left) const;
   SizeType find(const String &str, SizeType pos = 0, U32 mode = Case|Left) const;
   
   String   &insert(SizeType pos, const StringChar c) { return insert(pos,&c,1); }
   String   &insert(SizeType pos, const StringChar *str);
   String   &insert(SizeType pos, const String &str);
   String   &insert(SizeType pos, const StringChar *str, SizeType len);

   String   &erase(SizeType pos, SizeType len);

   String   &replace(SizeType pos, SizeType len, const StringChar *str);
   String   &replace(SizeType pos, SizeType len, const String &str);
   
   /// Replace all occurrences of character 'c1' with 'c2'
   String &replace( StringChar c1, StringChar c2 );

   /// Replace all occurrences of StringData 's1' with StringData 's2'
   String &replace(const String &s1, const String &s2);

   String substr( SizeType pos, SizeType len = -1 ) const;
   
   /// Remove leading and trailing whitespace.
   String trim() const;
   
   /// Replace all characters that need to be escaped for the string to be a valid string literal with their
   /// respective escape sequences.
   String expandEscapes() const;
   
   /// Replace all escape sequences in with their respective character codes.
   String collapseEscapes() const;
   
   /// Split the string into its components separated by the given delimiter.
   void split( const char* delimiter, Vector< String >& outElements ) const;
   
   /// Return true if the string starts with the given text.
   bool startsWith( const char* text ) const;
   
   /// Return true if the string ends with the given text.
   bool endsWith( const char* text ) const;

   operator const StringChar*() const { return c_str(); }

   StringChar operator []( U32 i ) const { return c_str()[i]; }
   StringChar operator []( S32 i ) const { return c_str()[i]; }

   bool operator==(const String &str) const;
   bool operator!=(const String &str) const { return !(*this == str); }
   bool operator==( StringChar c ) const;
   bool operator!=( StringChar c ) const { return !(*this == c); }
   bool operator<(const String &str) const;
   bool operator>(const String &str) const;
   bool operator<=(const String &str) const;
   bool operator>=(const String &str) const;

   friend String operator+(const String &a, StringChar c);
   friend String operator+(StringChar c, const String &a);
   friend String operator+(const String &a, const StringChar *b);
   friend String operator+(const String &a, const String &b);
   friend String operator+(const StringChar *a, const String &b);

public:
   /// @name String Utility routines
   /// @{

   static String ToString(const char *format, ...);
   static String VToString(const char* format, va_list args);

   static String ToString( bool v );
   static inline String ToString( U32 v ) { return ToString( "%u", v ); }
   static inline String ToString( S32 v ) { return ToString( "%d", v ); }
   static inline String ToString( F32 v ) { return ToString( "%g", v ); }
   static inline String ToString( F64 v ) { return ToString( "%Lg", v ); }

   static String SpanToString(const char* start, const char* end);

   static String ToLower(const String &string);
   static String ToUpper(const String &string);

   static String GetTrailingNumber(const char* str, S32& number);
   static String GetFirstNumber(const char* str, U32& startPos, U32& endPos);

   /// @}

   /// @name Interning
   ///
   /// Interning maps identical strings to unique instances so that equality
   /// amounts to simple pointer comparisons.
   ///
   /// Note that using interned strings within global destructors is not safe
   /// as table destruction runs within this phase as well.  Uses o interned
   /// strings in global destructors is thus dependent on object file ordering.
   ///
   /// Also, interned strings are not reference-counted.  Once interned, a
   /// string will persist until shutdown.  This is to avoid costly concurrent
   /// reference counting that would otherwise be necessary.
   ///
   /// @{
   
   /// Return the interned version of the string.
   /// @note Interning is case-sensitive.
   String intern() const;
   
   /// Return true if this string is interned.
   bool isInterned() const;
      
   /// @}

   /** An internal support class for ToString().
      StrFormat manages the formatting of arbitrary length strings.
      The class starts with a default internal fixed size buffer and
      moves to dynamic allocation from the heap when that is exceeded.
      Constructing the class on the stack will result in its most
      efficient use. This class is meant to be used as a helper class,
      and not for the permanent storage of string data.
      @code
         char* indexString(U32 index)
         {
            StrFormat format("Index: %d",index);
            char* str = new char[format.size()];
            format.copy(str);
            return str;
         }
      @endcode
   */
   class StrFormat
   {
   public:
      StrFormat()
         :  _dynamicBuffer( NULL ),
            _dynamicSize( 0 ),
            _len( 0 )
      {
         _fixedBuffer[0] = '\0';
      }

      StrFormat(const char *formatStr, va_list args)
         :  _dynamicBuffer( NULL ),
            _dynamicSize( 0 ),
            _len( 0 )
      {
         format(formatStr, args);
      }

      ~StrFormat();

      S32 format( const char *format, va_list args );
      S32 formatAppend( const char *format, va_list args );
      S32 append(const char * str, S32 len);
      S32 append(const char * str);

      String getString() { return String(c_str(),_len); }

      const char * c_str() const { return _dynamicBuffer ? _dynamicBuffer : _fixedBuffer; }

      void reset()
      {
         _len = 0;
         _fixedBuffer[0] = '\0';
      }

      /// Copy the formatted string into the output buffer which must be at least size() characters.
      char  *copy(char* buffer) const;

      /// Return the length of the formated string (does not include the terminating 0)
      U32 length() const { return _len; };

   public:
      char  _fixedBuffer[2048];  //< Fixed size buffer
      char  *_dynamicBuffer;     //< Temporary format buffer
      U32   _dynamicSize;        //< Dynamic buffer size
      U32   _len;                //< Len of the formatted string
   };

private:
   String(StringData *str)
      : _string( str ) {}

   // Generate compile error if operator bool is used.  Without this we use
   // operator const char *, which is always true...including operator bool
   // causes an ambiguous cast compile error.  Making it private is simply
   // more insurance that it isn't used on different compilers.
   // NOTE: disable on GCC since it causes hyper casting to U32 on gcc.
#if !defined(TORQUE_COMPILER_GCC) && !defined(__clang__)
   operator const bool() const { return false; }
#endif

   static void copy(StringChar *dst, const StringChar *src, U32 size);

   StringData   *_string;
};

// Utility class for formatting strings.
class StringBuilder
{
   protected:

      ///
      String::StrFormat mFormat;

   public:

      StringBuilder() {}
      
      U32 length() const
      {
         return mFormat.length();
      }
      
      void copy( char* buffer ) const
      {
         mFormat.copy( buffer );
      }

      const char* data() const
      {
         return mFormat.c_str();
      }

      String end()
      {
         return mFormat.getString();
      }

      S32 append( char ch )
      {
         char str[2];
         str[0]=ch;
         str[1]='\0';
         return mFormat.append(str);
      }
      S32 append( const char* str )
      {
         return mFormat.append(str);
      }
      S32 append( const String& str )
      {
         return mFormat.append( str.c_str(), str.length() );
      }
      S32 append( const char* str, U32 length )
      {
         return mFormat.append(str,length);
      }
      S32 format( const char* fmt, ... )
      {
         va_list args;
         va_start(args, fmt);
         return mFormat.formatAppend(fmt, args);
      }
};

// For use in hash tables and the like for explicitly requesting case sensitive hashing.
// Meant to only appear in hash table definition (casting will take care of the rest).
class StringCase : public String
{
public:
   StringCase() : String() {}
   StringCase(const String & s) : String(s) {}
};

// For use in hash tables and the like for explicitly requesting case insensitive hashing.
// Meant to only appear in hash table definition (casting will take care of the rest).
class StringNoCase : public String
{
public:
   StringNoCase() : String() {}
   StringNoCase(const String & s) : String(s) {}
};

class FileName : public String
{
public:
   FileName() : String() {}
   FileName(const String & s) : String(s) {}
   FileName & operator=(const String & s) { String::operator=(s); return *this; }
};

//-----------------------------------------------------------------------------

extern String operator+(const String &a, StringChar c);
extern String operator+(StringChar c, const String &a);
extern String operator+(const String &a, const StringChar *b);
extern String operator+(const String &a, const String &b);
extern String operator+(const StringChar *a, const String &b);

#endif

