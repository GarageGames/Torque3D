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
#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"
#include "console/ast.h"

#ifndef _CONSOLFUNCTIONS_H_
#include "console/consoleFunctions.h"
#endif

#include "core/strings/findMatch.h"
#include "core/strings/stringUnit.h"
#include "core/strings/unicode.h"
#include "core/stream/fileStream.h"
#include "console/compiler.h"
#include "platform/platformInput.h"
#include "core/util/journal/journal.h"
#include "gfx/gfxEnums.h"
#include "core/util/uuid.h"
#include "core/color.h"
#include "math/mPoint3.h"
#include "math/mathTypes.h"

// This is a temporary hack to get tools using the library to
// link in this module which contains no other references.
bool LinkConsoleFunctions = false;

// Buffer for expanding script filenames.
static char scriptFilenameBuffer[1024];

bool isInt(const char* str)
{
   int len = dStrlen(str);
   if(len <= 0)
      return false;

   // Ignore whitespace
   int start = 0;
   for(int i = start; i < len; i++)
      if(str[i] != ' ')
      {
         start = i;
         break;
      }

      for(int i = start; i < len; i++)
         switch(str[i])
      {
         case '+': case '-':
            if(i != 0)
               return false;
            break;
         case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '0': 
            break;
         case ' ': // ignore whitespace
            for(int j = i+1; j < len; j++)
               if(str[j] != ' ')
                  return false;
            return true;
            break;
         default:
            return false;
      }
      return true;
}

bool isFloat(const char* str, bool sciOk = false)
{
   int len = dStrlen(str);
   if(len <= 0)
      return false;

   // Ingore whitespace
   int start = 0;
   for(int i = start; i < len; i++)
      if(str[i] != ' ')
      {
         start = i;
         break;
      }

      bool seenDot = false;
      int eLoc = -1;
      for(int i = 0; i < len; i++)
         switch(str[i])
      {
         case '+': case '-':
            if(sciOk)
            {
               //Haven't found e or scientific notation symbol
               if(eLoc == -1)
               {
                  //only allowed in beginning
                  if(i != 0)
                     return false;
               }
               else
               {
                  //if not right after the e
                  if(i != (eLoc + 1))
                     return false;
               }
            }
            else
            {
               //only allowed in beginning
               if(i != 0)
                  return false;
            }
            break;
         case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '0': 
            break;
         case 'e': case 'E':
            if(!sciOk)
               return false;
            else
            {
               //already saw it so can't have 2
               if(eLoc != -1)
                  return false;

               eLoc = i;
            }
            break;
         case '.':
            if(seenDot | (sciOk && eLoc != -1))
               return false;
            seenDot = true;
            break;
         case ' ': // ignore whitespace
            for(int j = i+1; j < len; j++)
               if(str[j] != ' ')
                  return false;
            return true;
            break;
         default:
            return false;
      }
      return true;
}

bool isValidIP(const char* ip)
{
   unsigned b1, b2, b3, b4;
   unsigned char c;
   int rc = dSscanf(ip, "%3u.%3u.%3u.%3u%c", &b1, &b2, &b3, &b4, &c);
   if (rc != 4 && rc != 5) return false;
   if ((b1 | b2 | b3 | b4) > 255) return false;
   if (dStrspn(ip, "0123456789.") < dStrlen(ip)) return false;
   return true;
}

bool isValidPort(U16 port)
{
   return (port >= 0 && port <=65535);
}

//=============================================================================
//    String Functions.
//=============================================================================
// MARK: ---- String Functions ----

//-----------------------------------------------------------------------------

DefineConsoleFunction( strasc, int, ( const char* chr ),,
   "Return the integer character code value corresponding to the first character in the given string.\n"
   "@param chr a (one-character) string.\n"
   "@return the UTF32 code value for the first character in the given string.\n"
   "@ingroup Strings" )
{
   return oneUTF8toUTF32( chr );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strformat, const char*, ( const char* format, const char* value ),,
   "Format the given value as a string using printf-style formatting.\n"
   "@param format A printf-style format string.\n"
   "@param value The value argument matching the given format string.\n\n"
   "@tsexample\n"
   "// Convert the given integer value to a string in a hex notation.\n"
   "%hex = strformat( \"%x\", %value );\n"
   "@endtsexample\n"
   "@ingroup Strings\n"
   "@see http://en.wikipedia.org/wiki/Printf" )
{
   static const U32 bufSize = 64;
   char* pBuffer = Con::getReturnBuffer(bufSize);
   const char *pch = format;

   pBuffer[0] = '\0';
   while (*pch != '\0' && *pch !='%')
      pch++;
   while (*pch != '\0' && !dIsalpha(*pch))
      pch++;
   if (*pch == '\0')
   {
      Con::errorf("strFormat: Invalid format string!\n");
      return pBuffer;
   }
   
   switch(*pch)
   {
      case 'c':
      case 'C':
      case 'd':
      case 'i':
      case 'o':
      case 'u':
      case 'x':
      case 'X':
         dSprintf( pBuffer, bufSize, format, dAtoi( value ) );
         break;

      case 'e':
      case 'E':
      case 'f':
      case 'g':
      case 'G':
         dSprintf( pBuffer, bufSize, format, dAtof( value ) );
         break;

      default:
         Con::errorf("strFormat: Invalid format string!\n");
         break;
   }

   return pBuffer;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strcmp, S32, ( const char* str1, const char* str2 ),,
   "Compares two strings using case-<b>sensitive</b> comparison.\n"
   "@param str1 The first string.\n"
   "@param str2 The second string.\n"
   "@return 0 if both strings are equal, a value <0 if the first character different in str1 has a smaller character code "
      "value than the character at the same position in str2, and a value >1 otherwise.\n\n"
   "@tsexample\n"
   "if( strcmp( %var, \"foobar\" ) == 0 )\n"
   "   echo( \"%var is equal to 'foobar'\" );\n"
   "@endtsexample\n"
   "@see stricmp\n"
   "@see strnatcmp\n"
   "@ingroup Strings" )
{
   return dStrcmp( str1, str2 );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( stricmp, S32, ( const char* str1, const char* str2 ),,
   "Compares two strings using case-<b>insensitive</b> comparison.\n"
   "@param str1 The first string.\n"
   "@param str2 The second string.\n"
   "@return 0 if both strings are equal, a value <0 if the first character different in str1 has a smaller character code "
      "value than the character at the same position in str2, and a value >0 otherwise.\n\n"
   "@tsexample\n"
   "if( stricmp( \"FOObar\", \"foobar\" ) == 0 )\n"
   "   echo( \"this is always true\" );\n"
   "@endtsexample\n"
   "@see strcmp\n"
   "@see strinatcmp\n"
   "@ingroup Strings" )
{
   return dStricmp( str1, str2 );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strnatcmp, S32, ( const char* str1, const char* str2 ),,
   "Compares two strings using \"natural order\" case-<b>sensitive</b> comparison.\n"
   "Natural order means that rather than solely comparing single character code values, strings are ordered in a "
   "natural way.  For example, the string \"hello10\" is considered greater than the string \"hello2\" even though "
   "the first numeric character in \"hello10\" actually has a smaller character value than the corresponding character "
   "in \"hello2\".  However, since 10 is greater than 2, strnatcmp will put \"hello10\" after \"hello2\".\n"
   "@param str1 The first string.\n"
   "@param str2 The second string.\n\n"
   "@return 0 if the strings are equal, a value >0 if @a str1 comes after @a str2 in a natural order, and a value "
      "<0 if @a str1 comes before @a str2 in a natural order.\n\n"
   "@tsexample\n"
   "// Bubble sort 10 elements of %array using natural order\n"
   "do\n"
   "{\n"
   "   %swapped = false;\n"
   "   for( %i = 0; %i < 10 - 1; %i ++ )\n"
   "      if( strnatcmp( %array[ %i ], %array[ %i + 1 ] ) > 0 )\n"
   "      {\n"
   "         %temp = %array[ %i ];\n"
   "         %array[ %i ] = %array[ %i + 1 ];\n"
   "         %array[ %i + 1 ] = %temp;\n"
   "         %swapped = true;\n"
   "      }\n"
   "}\n"
   "while( %swapped );\n"
   "@endtsexample\n"
   "@see strcmp\n"
   "@see strinatcmp\n"
   "@ingroup Strings" )
{
   return dStrnatcmp( str1, str2 );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strinatcmp, S32, ( const char* str1, const char* str2 ),,
   "Compares two strings using \"natural order\" case-<b>insensitive</b> comparison.\n"
   "Natural order means that rather than solely comparing single character code values, strings are ordered in a "
   "natural way.  For example, the string \"hello10\" is considered greater than the string \"hello2\" even though "
   "the first numeric character in \"hello10\" actually has a smaller character value than the corresponding character "
   "in \"hello2\".  However, since 10 is greater than 2, strnatcmp will put \"hello10\" after \"hello2\".\n"
   "@param str1 The first string.\n"
   "@param str2 The second string.\n"
   "@return 0 if the strings are equal, a value >0 if @a str1 comes after @a str2 in a natural order, and a value "
      "<0 if @a str1 comes before @a str2 in a natural order.\n\n"
   "@tsexample\n\n"
   "// Bubble sort 10 elements of %array using natural order\n"
   "do\n"
   "{\n"
   "   %swapped = false;\n"
   "   for( %i = 0; %i < 10 - 1; %i ++ )\n"
   "      if( strnatcmp( %array[ %i ], %array[ %i + 1 ] ) > 0 )\n"
   "      {\n"
   "         %temp = %array[ %i ];\n"
   "         %array[ %i ] = %array[ %i + 1 ];\n"
   "         %array[ %i + 1 ] = %temp;\n"
   "         %swapped = true;\n"
   "      }\n"
   "}\n"
   "while( %swapped );\n"
   "@endtsexample\n"
   "@see stricmp\n"
   "@see strnatcmp\n"
   "@ingroup Strings" )
{
   return dStrnatcasecmp( str1, str2 );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strlen, S32, ( const char* str ),,
   "Get the length of the given string in bytes.\n"
   "@note This does <b>not</b> return a true character count for strings with multi-byte characters!\n"
   "@param str A string.\n"
   "@return The length of the given string in bytes.\n"
   "@ingroup Strings" )
{
   return dStrlen( str );
}

//-----------------------------------------------------------------------------
DefineConsoleFunction( strlenskip, S32, ( const char* str, const char* first, const char* last ),,
   "Calculate the length of a string in characters, skipping everything between and including first and last.\n"
   "@param str A string.\n"
   "@param first First character to look for to skip block of text.\n"
   "@param last Second character to look for to skip block of text.\n"
   "@return The length of the given string skipping blocks of text between characters.\n"
   "@ingroup Strings" )
{
   const UTF8* pos = str;
   U32 size = 0;
   U32 length = dStrlen(str);
   bool count = true;

   //loop through each character counting each character, skipping tags (anything with < followed by >)
   for(U32 i = 0; i < length; i++, pos++)
   {
      if(count)
      {
         if(*pos == first[0])
            count = false;
         else
            size++;
      }
      else
      {
         if(*pos == last[0])
            count = true;
      }
   }

   return S32(size);
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strstr, S32, ( const char* string, const char* substring ),,
   "Find the start of @a substring in the given @a string searching from left to right.\n"
   "@param string The string to search.\n"
   "@param substring The string to search for.\n"
   "@return The index into @a string at which the first occurrence of @a substring was found or -1 if @a substring could not be found.\n\n"
   "@tsexample\n"
   "strstr( \"abcd\", \"c\" ) // Returns 2.\n"
   "@endtsexample\n"
   "@ingroup Strings" )
{
   const char* retpos = dStrstr( string, substring );
   if( !retpos )
      return -1;
      
   return retpos - string;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strpos, S32, ( const char* haystack, const char* needle, S32 offset ), ( 0 ),
   "Find the start of @a needle in @a haystack searching from left to right beginning at the given offset.\n"
   "@param haystack The string to search.\n"
   "@param needle The string to search for.\n"
   "@return The index at which the first occurrence of @a needle was found in @a haystack or -1 if no match was found.\n\n"
   "@tsexample\n"
   "strpos( \"b ab\", \"b\", 1 ) // Returns 3.\n"
   "@endtsexample\n"
   "@ingroup Strings" )
{
   S32 start = offset;
   U32 sublen = dStrlen( needle );
   U32 strlen = dStrlen( haystack );
   if(start < 0)
      return -1;
   if(sublen + start > strlen)
      return -1;
   for(; start + sublen <= strlen; start++)
      if(!dStrncmp(haystack + start, needle, sublen))
         return start;
   return -1;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strposr, S32, ( const char* haystack, const char* needle, S32 offset ), ( 0 ),
   "Find the start of @a needle in @a haystack searching from right to left beginning at the given offset.\n"
   "@param haystack The string to search.\n"
   "@param needle The string to search for.\n"
   "@return The index at which the first occurrence of @a needle was found in @a heystack or -1 if no match was found.\n\n"
   "@tsexample\n"
   "strposr( \"b ab\", \"b\", 1 ) // Returns 2.\n"
   "@endtsexample\n"
   "@ingroup Strings" )
{
   U32 sublen = dStrlen( needle );
   U32 strlen = dStrlen( haystack );
   S32 start = strlen - offset;
   	
   if(start < 0 || start > strlen)
      return -1;
   
   if (start + sublen > strlen)
	  start = strlen - sublen;
   for(; start >= 0; start--)
      if(!dStrncmp(haystack + start, needle, sublen))
         return start;
   return -1;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( ltrim, const char*, ( const char* str ),,
   "Remove leading whitespace from the string.\n"
   "@param str A string.\n"
   "@return A string that is the same as @a str but with any leading (i.e. leftmost) whitespace removed.\n\n"
   "@tsexample\n"
   "ltrim( \"   string  \" ); // Returns \"string  \".\n"
   "@endtsexample\n"
   "@see rtrim\n"
   "@see trim\n"
   "@ingroup Strings" )
{
   const char *ret = str;
   while(*ret == ' ' || *ret == '\n' || *ret == '\t')
      ret++;
   return ret;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( rtrim, const char*, ( const char* str ),,
   "Remove trailing whitespace from the string.\n"
   "@param str A string.\n"
   "@return A string that is the same as @a str but with any trailing (i.e. rightmost) whitespace removed.\n\n"
   "@tsexample\n"
   "rtrim( \"   string  \" ); // Returns \"   string\".\n"
   "@endtsexample\n"
   "@see ltrim\n"
   "@see trim\n"
   "@ingroup Strings" )
{
   S32 firstWhitespace = 0;
   S32 pos = 0;
   while(str[pos])
   {
      if(str[pos] != ' ' && str[pos] != '\n' && str[pos] != '\t')
         firstWhitespace = pos + 1;
      pos++;
   }
   char *ret = Con::getReturnBuffer(firstWhitespace + 1);
   dStrncpy(ret, str, firstWhitespace);
   ret[firstWhitespace] = 0;
   return ret;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( trim, const char*, ( const char* str ),,
   "Remove leading and trailing whitespace from the string.\n"
   "@param str A string.\n"
   "@return A string that is the same as @a str but with any leading (i.e. leftmost) and trailing (i.e. rightmost) whitespace removed.\n\n"
   "@tsexample\n"
   "trim( \"   string  \" ); // Returns \"string\".\n"
   "@endtsexample\n"
   "@ingroup Strings" )
{
   const char *ptr = str;
   while(*ptr == ' ' || *ptr == '\n' || *ptr == '\t')
      ptr++;
   S32 firstWhitespace = 0;
   S32 pos = 0;
   while(ptr[pos])
   {
      if(ptr[pos] != ' ' && ptr[pos] != '\n' && ptr[pos] != '\t')
         firstWhitespace = pos + 1;
      pos++;
   }
   char *ret = Con::getReturnBuffer(firstWhitespace + 1);
   dStrncpy(ret, ptr, firstWhitespace);
   ret[firstWhitespace] = 0;
   return ret;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( stripChars, const char*, ( const char* str, const char* chars ),,
   "Remove all occurrences of characters contained in @a chars from @a str.\n"
   "@param str The string to filter characters out from.\n"
   "@param chars A string of characters to filter out from @a str.\n"
   "@return A version of @a str with all occurrences of characters contained in @a chars filtered out.\n\n"
   "@tsexample\n"
   "stripChars( \"teststring\", \"se\" ); // Returns \"tttring\"."
   "@endtsexample\n"
   "@ingroup Strings" )
{
   char* ret = Con::getReturnBuffer( dStrlen( str ) + 1 );
   dStrcpy( ret, str );
   U32 pos = dStrcspn( ret, chars );
   while ( pos < dStrlen( ret ) )
   {
      dStrcpy( ret + pos, ret + pos + 1 );
      pos = dStrcspn( ret, chars );
   }
   return( ret );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strlwr, const char*, ( const char* str ),,
   "Return an all lower-case version of the given string.\n"
   "@param str A string.\n"
   "@return A version of @a str with all characters converted to lower-case.\n\n"
   "@tsexample\n"
   "strlwr( \"TesT1\" ) // Returns \"test1\"\n"
   "@endtsexample\n"
   "@see strupr\n"
   "@ingroup Strings" )
{
   char *ret = Con::getReturnBuffer(dStrlen(str) + 1);
   dStrcpy(ret, str);
   return dStrlwr(ret);
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strupr, const char*, ( const char* str ),,
   "Return an all upper-case version of the given string.\n"
   "@param str A string.\n"
   "@return A version of @a str with all characters converted to upper-case.\n\n"
   "@tsexample\n"
   "strupr( \"TesT1\" ) // Returns \"TEST1\"\n"
   "@endtsexample\n"
   "@see strlwr\n"
   "@ingroup Strings" )
{
   char *ret = Con::getReturnBuffer(dStrlen(str) + 1);
   dStrcpy(ret, str);
   return dStrupr(ret);
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strchr, const char*, ( const char* str, const char* chr ),,
   "Find the first occurrence of the given character in @a str.\n"
   "@param str The string to search.\n"
   "@param chr The character to search for.  Only the first character from the string is taken.\n"
   "@return The remainder of the input string starting with the given character or the empty string if the character could not be found.\n\n"
   "@see strrchr\n"
   "@ingroup Strings" )
{
   const char *ret = dStrchr( str, chr[ 0 ] );
   return ret ? ret : "";
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strrchr, const char*, ( const char* str, const char* chr ),,
   "Find the last occurrence of the given character in @a str."
   "@param str The string to search.\n"
   "@param chr The character to search for.  Only the first character from the string is taken.\n"
   "@return The remainder of the input string starting with the given character or the empty string if the character could not be found.\n\n"
   "@see strchr\n"
   "@ingroup Strings" )
{
   const char *ret = dStrrchr( str, chr[ 0 ] );
   return ret ? ret : "";
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strreplace, const char*, ( const char* source, const char* from, const char* to ),,
   "Replace all occurrences of @a from in @a source with @a to.\n"
   "@param source The string in which to replace the occurrences of @a from.\n"
   "@param from The string to replace in @a source.\n"
   "@param to The string with which to replace occurrences of @from.\n"
   "@return A string with all occurrences of @a from in @a source replaced by @a to.\n\n"
   "@tsexample\n"
   "strreplace( \"aabbccbb\", \"bb\", \"ee\" ) // Returns \"aaeeccee\".\n"
   "@endtsexample\n"
   "@ingroup Strings" )
{
   S32 fromLen = dStrlen( from );
   if(!fromLen)
      return source;

   S32 toLen = dStrlen( to );
   S32 count = 0;
   const char *scan = source;
   while(scan)
   {
      scan = dStrstr(scan, from);
      if(scan)
      {
         scan += fromLen;
         count++;
      }
   }
   char *ret = Con::getReturnBuffer(dStrlen(source) + 1 + (toLen - fromLen) * count);
   U32 scanp = 0;
   U32 dstp = 0;
   for(;;)
   {
      const char *scan = dStrstr(source + scanp, from);
      if(!scan)
      {
         dStrcpy(ret + dstp, source + scanp);
         return ret;
      }
      U32 len = scan - (source + scanp);
      dStrncpy(ret + dstp, source + scanp, len);
      dstp += len;
      dStrcpy(ret + dstp, to);
      dstp += toLen;
      scanp += len + fromLen;
   }
   return ret;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strrepeat, const char*, ( const char* str, S32 numTimes, const char* delimiter ), ( "" ),
   "Return a string that repeats @a str @a numTimes number of times delimiting each occurrence with @a delimiter.\n"
   "@param str The string to repeat multiple times.\n"
   "@param numTimes The number of times to repeat @a str in the result string.\n"
   "@param delimiter The string to put between each repetition of @a str.\n"
   "@return A string containing @a str repeated @a numTimes times.\n\n"
   "@tsexample\n"
   "strrepeat( \"a\", 5, \"b\" ) // Returns \"ababababa\".\n"
   "@endtsexample\n"
   "@ingroup Strings" )
{
   StringBuilder result;
   bool isFirst = false;
   for( U32 i = 0; i < numTimes; ++ i )
   {
      if( !isFirst )
         result.append( delimiter );
         
      result.append( str );
      isFirst = false;
   }
   
   return Con::getReturnBuffer( result );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getSubStr, const char*, ( const char* str, S32 start, S32 numChars ), ( -1 ),
   "@brief Return a substring of @a str starting at @a start and continuing either through to the end of @a str "
   "(if @a numChars is -1) or for @a numChars characters (except if this would exceed the actual source "
   "string length).\n"
   "@param str The string from which to extract a substring.\n"
   "@param start The offset at which to start copying out characters.\n"
   "@param numChars Optional argument to specify the number of characters to copy.  If this is -1, all characters up the end "
      "of the input string are copied.\n"
   "@return A string that contains the given portion of the input string.\n\n"
   "@tsexample\n"
   "getSubStr( \"foobar\", 1, 2 ) // Returns \"oo\".\n"
   "@endtsexample\n\n"
   "@ingroup Strings" )
{
   S32 baseLen = dStrlen( str );

   if( numChars == -1 )
      numChars = baseLen - start;
      
   if (start < 0 || numChars < 0) {
      Con::errorf(ConsoleLogEntry::Script, "getSubStr(...): error, starting position and desired length must be >= 0: (%d, %d)", start, numChars);

      return "";
   }

   if (baseLen < start)
      return "";

   U32 actualLen = numChars;
   if (start + numChars > baseLen)
      actualLen = baseLen - start;

   char *ret = Con::getReturnBuffer(actualLen + 1);
   dStrncpy(ret, str + start, actualLen);
   ret[actualLen] = '\0';

   return ret;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strIsMatchExpr, bool, ( const char* pattern, const char* str, bool caseSensitive ), ( false ),
   "Match a pattern against a string.\n"
   "@param pattern The wildcard pattern to match against.  The pattern can include characters, '*' to match "
      "any number of characters and '?' to match a single character.\n"
   "@param str The string which should be matched against @a pattern.\n"
   "@param caseSensitive If true, characters in the pattern are matched in case-sensitive fashion against "
      "this string.  If false, differences in casing are ignored.\n"
   "@return True if @a str matches the given @a pattern.\n\n"
   "@tsexample\n"
   "strIsMatchExpr( \"f?o*R\", \"foobar\" ) // Returns true.\n"
   "@endtsexample\n"
   "@see strIsMatchMultipleExpr\n"
   "@ingroup Strings" )
{
   return FindMatch::isMatch( pattern, str, caseSensitive );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( strIsMatchMultipleExpr, bool, ( const char* patterns, const char* str, bool caseSensitive ), ( false ),
   "Match a multiple patterns against a single string.\n"
   "@param patterns A tab-separated list of patterns.  Each pattern can include charaters, '*' to match "
      "any number of characters and '?' to match a single character.  Each of the patterns is tried in turn.\n"
   "@param str The string which should be matched against @a patterns.\n"
   "@param caseSensitive If true, characters in the pattern are matched in case-sensitive fashion against "
      "this string.  If false, differences in casing are ignored.\n"
   "@return True if @a str matches any of the given @a patterns.\n\n"
   "@tsexample\n"
   "strIsMatchMultipleExpr( \"*.cs *.gui *.mis\", \"mymission.mis\" ) // Returns true.\n"
   "@endtsexample\n"
   "@see strIsMatchExpr\n"
   "@ingroup Strings" )
{
   return FindMatch::isMatchMultipleExprs( patterns, str, caseSensitive );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getTrailingNumber, S32, ( const char* str ),,
   "Get the numeric suffix of the given input string.\n"
   "@param str The string from which to read out the numeric suffix.\n"
   "@return The numeric value of the number suffix of @a str or -1 if @a str has no such suffix.\n\n"
   "@tsexample\n"
   "getTrailingNumber( \"test123\" ) // Returns '123'.\n"
   "@endtsexample\n\n"
   "@see stripTrailingNumber\n"
   "@ingroup Strings" )
{
   S32 suffix = -1;
   String outStr( String::GetTrailingNumber( str, suffix ) );
   return suffix;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( stripTrailingNumber, String, ( const char* str ),,
   "Strip a numeric suffix from the given string.\n"
   "@param str The string from which to strip its numeric suffix.\n"
   "@return The string @a str without its number suffix or the original string @a str if it has no such suffix.\n\n"
   "@tsexample\n"
   "stripTrailingNumber( \"test123\" ) // Returns \"test\".\n"
   "@endtsexample\n\n"
   "@see getTrailingNumber\n"
   "@ingroup Strings" )
{
   S32 suffix;
   return String::GetTrailingNumber( str, suffix );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getFirstNumber, String, ( const char* str ),,
   "Get the first occuring number from @a str.\n"
   "@param str The string from which to read out the first number.\n"
   "@return String representation of the number or "" if no number.\n\n")
{
   U32 start;
   U32 end;
   return String::GetFirstNumber(str, start, end);
}

//----------------------------------------------------------------

DefineConsoleFunction( isspace, bool, ( const char* str, S32 index ),,
   "Test whether the character at the given position is a whitespace character.\n"
   "Characters such as tab, space, or newline are considered whitespace.\n"
   "@param str The string to test.\n"
   "@param index The index of a character in @a str.\n"
   "@return True if the character at the given index in @a str is a whitespace character; false otherwise.\n\n"
   "@see isalnum\n"
   "@ingroup Strings" )
{
   if( index >= 0 && index < dStrlen( str ) )
      return dIsspace( str[ index ] );
   else
      return false;
}

//----------------------------------------------------------------

DefineConsoleFunction( isalnum, bool, ( const char* str, S32 index ),,
   "Test whether the character at the given position is an alpha-numeric character.\n"
   "Alpha-numeric characters are characters that are either alphabetic (a-z, A-Z) or numbers (0-9).\n"
   "@param str The string to test.\n"
   "@param index The index of a character in @a str.\n"
   "@return True if the character at the given index in @a str is an alpha-numeric character; false otherwise.\n\n"
   "@see isspace\n"
   "@ingroup Strings" )
{
   if( index >= 0 && index < dStrlen( str ) )
      return dIsalnum( str[ index ] );
   else
      return false;
}

//----------------------------------------------------------------

DefineConsoleFunction( startsWith, bool, ( const char* str, const char* prefix, bool caseSensitive ), ( false ),
   "Test whether the given string begins with the given prefix.\n"
   "@param str The string to test.\n"
   "@param prefix The potential prefix of @a str.\n"
   "@param caseSensitive If true, the comparison will be case-sensitive; if false, differences in casing will "
      "not be taken into account.\n"
   "@return True if the first characters in @a str match the complete contents of @a prefix; false otherwise.\n\n"
   "@tsexample\n"
   "startsWith( \"TEST123\", \"test\" ) // Returns true.\n"
   "@endtsexample\n"
   "@see endsWith\n"
   "@ingroup Strings" )
{
   // if the target string is empty, return true (all strings start with the empty string)
   S32 srcLen = dStrlen( str );
   S32 targetLen = dStrlen( prefix );
   if( targetLen == 0 )
      return true;
   // else if the src string is empty, return false (empty src does not start with non-empty target)
   else if( srcLen == 0 )
      return false;

   if( caseSensitive )
      return ( dStrncmp( str, prefix, targetLen ) == 0 );

   // both src and target are non empty, create temp buffers for lowercase operation
   char* srcBuf = new char[ srcLen + 1 ];
   char* targetBuf = new char[ targetLen + 1 ];

   // copy src and target into buffers
   dStrcpy( srcBuf, str );
   dStrcpy( targetBuf, prefix );

   // reassign src/target pointers to lowercase versions
   str = dStrlwr( srcBuf );
   prefix = dStrlwr( targetBuf );

   // do the comparison
   bool startsWith = dStrncmp( str, prefix, targetLen ) == 0;

   // delete temp buffers
   delete [] srcBuf;
   delete [] targetBuf;

   return startsWith;
}

//----------------------------------------------------------------

DefineConsoleFunction( endsWith, bool, ( const char* str, const char* suffix, bool caseSensitive ), ( false ),
   "@brief Test whether the given string ends with the given suffix.\n\n"
   "@param str The string to test.\n"
   "@param suffix The potential suffix of @a str.\n"
   "@param caseSensitive If true, the comparison will be case-sensitive; if false, differences in casing will "
      "not be taken into account.\n"
   "@return True if the last characters in @a str match the complete contents of @a suffix; false otherwise.\n\n"
   "@tsexample\n"
   "startsWith( \"TEST123\", \"123\" ) // Returns true.\n"
   "@endtsexample\n\n"
   "@see startsWith\n"
   "@ingroup Strings" )
{
   // if the target string is empty, return true (all strings end with the empty string)
   S32 srcLen = dStrlen( str );
   S32 targetLen = dStrlen( suffix );
   if (targetLen == 0)
      return true;
   // else if the src string is empty, return false (empty src does not end with non-empty target)
   else if (srcLen == 0)
      return false;
   else if( targetLen > srcLen )
      return false;
      
   if( caseSensitive )
      return ( dStrcmp( &str[ srcLen - targetLen ], suffix ) == 0 );

   // both src and target are non empty, create temp buffers for lowercase operation
   char* srcBuf = new char[ srcLen + 1 ];
   char* targetBuf = new char[ targetLen + 1 ];

   // copy src and target into buffers
   dStrcpy( srcBuf, str );
   dStrcpy( targetBuf, suffix );

   // reassign src/target pointers to lowercase versions
   str = dStrlwr( srcBuf );
   suffix = dStrlwr( targetBuf );

   // set the src pointer to the appropriate place to check the end of the string
   str += srcLen - targetLen;

   // do the comparison
   bool endsWith = dStrcmp( str, suffix ) == 0;

   // delete temp buffers
   delete [] srcBuf;
   delete [] targetBuf;

   return endsWith;
}

//----------------------------------------------------------------

DefineConsoleFunction( strchrpos, S32, ( const char* str, const char* chr, S32 start ), ( 0 ),
   "Find the first occurrence of the given character in the given string.\n"
   "@param str The string to search.\n"
   "@param chr The character to look for.  Only the first character of this string will be searched for.\n"
   "@param start The index into @a str at which to start searching for the given character.\n"
   "@return The index of the first occurrence of @a chr in @a str or -1 if @a str does not contain the given character.\n\n"
   "@tsexample\n"
   "strchrpos( \"test\", \"s\" ) // Returns 2.\n"
   "@endtsexample\n"
   "@ingroup Strings" )
{
   if( start != 0 && start >= dStrlen( str ) )
      return -1;
   
   const char* ret = dStrchr( &str[ start ], chr[ 0 ] );
   return ret ? ret - str : -1;
}

//----------------------------------------------------------------

DefineConsoleFunction( strrchrpos, S32, ( const char* str, const char* chr, S32 start ), ( 0 ),
   "Find the last occurrence of the given character in the given string.\n"
   "@param str The string to search.\n"
   "@param chr The character to look for.  Only the first character of this string will be searched for.\n"
   "@param start The index into @a str at which to start searching for the given character.\n"
   "@return The index of the last occurrence of @a chr in @a str or -1 if @a str does not contain the given character.\n\n"
   "@tsexample\n"
   "strrchrpos( \"test\", \"t\" ) // Returns 3.\n"
   "@endtsexample\n"
   "@ingroup Strings" )
{
   if( start != 0 && start >= dStrlen( str ) )
      return -1;

   const char* ret = dStrrchr( str, chr[ 0 ] );
   if( !ret )
      return -1;
      
   S32 index = ret - str;
   if( index < start )
      return -1;
      
   return index;
}

//----------------------------------------------------------------

DefineConsoleFunction(ColorFloatToInt, ColorI, (ColorF color), ,
	"Convert from a float color to an integer color (0.0 - 1.0 to 0 to 255).\n"
	"@param color Float color value to be converted in the form \"R G B A\", where R is red, G is green, B is blue, and A is alpha.\n"
	"@return Converted color value (0 - 255)\n\n"
	"@tsexample\n"
	"ColorFloatToInt( \"0 0 1 0.5\" ) // Returns \"0 0 255 128\".\n"
	"@endtsexample\n"
	"@ingroup Strings")
{
	return (ColorI)color;
}

DefineConsoleFunction(ColorIntToFloat, ColorF, (ColorI color), ,
   "Convert from a integer color to an float color (0 to 255 to 0.0 - 1.0).\n"
   "@param color Integer color value to be converted in the form \"R G B A\", where R is red, G is green, B is blue, and A is alpha.\n"
   "@return Converted color value (0.0 - 1.0)\n\n"
   "@tsexample\n"
   "ColorIntToFloat( \"0 0 255 128\" ) // Returns \"0 0 1 0.5\".\n"
   "@endtsexample\n"
   "@ingroup Strings")
{
   return (ColorF)color;
}

DefineConsoleFunction(ColorRGBToHEX, const char*, (ColorI color), ,
   "Convert from a integer RGB (red, green, blue) color to hex color value (0 to 255 to 00 - FF).\n"
   "@param color Integer color value to be converted in the form \"R G B A\", where R is red, G is green, B is blue, and A is alpha. It excepts an alpha, but keep in mind this will not be converted.\n"
   "@return Hex color value (#000000 - #FFFFFF), alpha isn't handled/converted so it is only the RGB value\n\n"
   "@tsexample\n"
   "ColorRBGToHEX( \"0 0 255 128\" ) // Returns \"#0000FF\".\n"
   "@endtsexample\n"
   "@ingroup Strings")
{
   return Con::getReturnBuffer(color.getHex());
}

DefineConsoleFunction(ColorRGBToHSB, const char*, (ColorI color), ,
   "Convert from a integer RGB (red, green, blue) color to HSB (hue, saturation, brightness). HSB is also know as HSL or HSV as well, with the last letter standing for lightness or value.\n"
   "@param color Integer color value to be converted in the form \"R G B A\", where R is red, G is green, B is blue, and A is alpha. It excepts an alpha, but keep in mind this will not be converted.\n"
   "@return HSB color value, alpha isn't handled/converted so it is only the RGB value\n\n"
   "@tsexample\n"
   "ColorRBGToHSB( \"0 0 255 128\" ) // Returns \"240 100 100\".\n"
   "@endtsexample\n"
   "@ingroup Strings")
{
   ColorI::Hsb hsb(color.getHSB());
   String s(String::ToString(hsb.hue) + " " + String::ToString(hsb.sat) + " " + String::ToString(hsb.brightness));
   return Con::getReturnBuffer(s);
}

DefineConsoleFunction(ColorHEXToRGB, ColorI, (const char* hex), ,
   "Convert from a hex color value to an integer RGB (red, green, blue) color (00 - FF to 0 to 255).\n"
   "@param hex Hex color value (#000000 - #FFFFFF) to be converted to an RGB (red, green, blue) value.\n"
   "@return Integer color value to be converted in the form \"R G B A\", where R is red, G is green, B is blue, and A is alpha. Alpha isn't handled/converted so only pay attention to the RGB value\n\n"
   "@tsexample\n"
   "ColorHEXToRGB( \"#0000FF\" ) // Returns \"0 0 255 0\".\n"
   "@endtsexample\n"
   "@ingroup Strings")
{
   S32 rgb = dAtoui(hex, 16);

   ColorI color;
   color.set(rgb & 0x000000FF, (rgb & 0x0000FF00) >> 8, (rgb & 0x00FF0000) >> 16);
   return color;
}

DefineConsoleFunction(ColorHSBToRGB, ColorI, (Point3I hsb), ,
   "Convert from a HSB (hue, saturation, brightness) to an integer RGB (red, green, blue) color. HSB is also know as HSL or HSV as well, with the last letter standing for lightness or value.\n"
   "@param hsb HSB (hue, saturation, brightness) value to be converted.\n"
   "@return Integer color value to be converted in the form \"R G B A\", where R is red, G is green, B is blue, and A is alpha. Alpha isn't handled/converted so only pay attention to the RGB value\n\n"
   "@tsexample\n"
   "ColorHSBToRGB( \"240 100 100\" ) // Returns \"0 0 255 0\".\n"
   "@endtsexample\n"
   "@ingroup Strings")
{
   ColorI color;
   color.set(ColorI::Hsb(hsb.x, hsb.y, hsb.z));
   return color;
}

//----------------------------------------------------------------

DefineConsoleFunction( strToggleCaseToWords, const char*, ( const char* str ),,
   "Parse a Toggle Case word into separate words.\n"
   "@param str The string to parse.\n"
   "@return new string space separated.\n\n"
   "@tsexample\n"
   "strToggleCaseToWords( \"HelloWorld\" ) // Returns \"Hello World\".\n"
   "@endtsexample\n"
   "@ingroup Strings" )
{
   String newStr;
   for(S32 i = 0; str[i]; i++)
   {
      //If capitol add a space
      if(i != 0 && str[i] >= 65 && str[i] <= 90)
         newStr += " "; 

      newStr += str[i]; 
   }

   return Con::getReturnBuffer(newStr);
}

//----------------------------------------------------------------

// Warning: isInt and isFloat are very 'strict' and might need to be adjusted to allow other values. //seanmc
DefineConsoleFunction( isInt, bool, ( const char* str),,
   "Returns true if the string is an integer.\n"
   "@param str The string to test.\n"
   "@return true if @a str is an integer and false if not\n\n"
   "@tsexample\n"
   "isInt( \"13\" ) // Returns true.\n"
   "@endtsexample\n"
   "@ingroup Strings" )
{
   return isInt(str);
}

//----------------------------------------------------------------

DefineConsoleFunction( isFloat, bool, ( const char* str, bool sciOk), (false),
   "Returns true if the string is a float.\n"
   "@param str The string to test.\n"
   "@param sciOk Test for correct scientific notation and accept it (ex. 1.2e+14)"
   "@return true if @a str is a float and false if not\n\n"
   "@tsexample\n"
   "isFloat( \"13.5\" ) // Returns true.\n"
   "@endtsexample\n"
   "@ingroup Strings" )
{
   return isFloat(str, sciOk);
}

//----------------------------------------------------------------

DefineConsoleFunction( isValidPort, bool, ( const char* str),,
   "Returns true if the string is a valid port number.\n"
   "@param str The string to test.\n"
   "@return true if @a str is a port and false if not\n\n"
   "@tsexample\n"
   "isValidPort( \"8080\" ) // Returns true.\n"
   "@endtsexample\n"
   "@ingroup Strings" )
{
   if(isInt(str))
   {
      U16 port = dAtous(str);
      return isValidPort(port);
   }
   else
      return false;
}

//----------------------------------------------------------------

DefineConsoleFunction( isValidIP, bool, ( const char* str),,
   "Returns true if the string is a valid ip address, excepts localhost.\n"
   "@param str The string to test.\n"
   "@return true if @a str is a valid ip address and false if not\n\n"
   "@tsexample\n"
   "isValidIP( \"localhost\" ) // Returns true.\n"
   "@endtsexample\n"
   "@ingroup Strings" )
{
   if(dStrcmp(str, "localhost") == 0)
   {
      return true;
   }
   else
      return isValidIP(str);
}

//----------------------------------------------------------------

// Torque won't normally add another string if it already exists with another casing,
// so this forces the addition. It should be called once near the start, such as in main.cs.
ConsoleFunction(addCaseSensitiveStrings,void,2,0,"[string1, string2, ...]"
                "Adds case sensitive strings to the StringTable.")
{
	for(int i = 1; i < argc; i++)
		StringTable->insert(argv[i], true);
}

//=============================================================================
//    Field Manipulators.
//=============================================================================
// MARK: ---- Field Manipulators ----

//-----------------------------------------------------------------------------

DefineConsoleFunction( getWord, const char*, ( const char* text, S32 index ),,
   "Extract the word at the given @a index in the whitespace-separated list in @a text.\n"
   "Words in @a text must be separated by newlines, spaces, and/or tabs.\n"
   "@param text A whitespace-separated list of words.\n"
   "@param index The zero-based index of the word to extract.\n"
   "@return The word at the given index or \"\" if the index is out of range.\n\n"
   "@tsexample\n"
      "getWord( \"a b c\", 1 ) // Returns \"b\"\n"
   "@endtsexample\n\n"
   "@see getWords\n"
   "@see getWordCount\n"
   "@see getToken\n"
   "@see getField\n"
   "@see getRecord\n"
   "@ingroup FieldManip" )
{
   return Con::getReturnBuffer( StringUnit::getUnit( text, index, " \t\n") );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getWords, const char*, ( const char* text, S32 startIndex, S32 endIndex ), ( -1 ),
   "Extract a range of words from the given @a startIndex onwards thru @a endIndex.\n"
   "Words in @a text must be separated by newlines, spaces, and/or tabs.\n"
   "@param text A whitespace-separated list of words.\n"
   "@param startIndex The zero-based index of the first word to extract from @a text.\n"
   "@param endIndex The zero-based index of the last word to extract from @a text.  If this is -1, all words beginning "
      "with @a startIndex are extracted from @a text.\n"
   "@return A string containing the specified range of words from @a text or \"\" if @a startIndex "
      "is out of range or greater than @a endIndex.\n\n"
   "@tsexample\n"
      "getWords( \"a b c d\", 1, 2, ) // Returns \"b c\"\n"
   "@endtsexample\n\n"
   "@see getWord\n"
   "@see getWordCount\n"
   "@see getTokens\n"
   "@see getFields\n"
   "@see getRecords\n"
   "@ingroup FieldManip" )
{
   if( endIndex < 0 )
      endIndex = 1000000;

   return Con::getReturnBuffer( StringUnit::getUnits( text, startIndex, endIndex, " \t\n" ) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( setWord, const char*, ( const char* text, S32 index, const char* replacement ),,
   "Replace the word in @a text at the given @a index with @a replacement.\n"
   "Words in @a text must be separated by newlines, spaces, and/or tabs.\n"
   "@param text A whitespace-separated list of words.\n"
   "@param index The zero-based index of the word to replace.\n"
   "@param replacement The string with which to replace the word.\n"
   "@return A new string with the word at the given @a index replaced by @a replacement or the original "
      "string if @a index is out of range.\n\n"
   "@tsexample\n"
      "setWord( \"a b c d\", 2, \"f\" ) // Returns \"a b f d\"\n"
   "@endtsexample\n\n"
   "@see getWord\n"
   "@see setToken\n"
   "@see setField\n"
   "@see setRecord\n"
   "@ingroup FieldManip" )
{
   return Con::getReturnBuffer( StringUnit::setUnit( text, index, replacement, " \t\n") );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( removeWord, const char*, ( const char* text, S32 index ),,
   "Remove the word in @a text at the given @a index.\n"
   "Words in @a text must be separated by newlines, spaces, and/or tabs.\n"
   "@param text A whitespace-separated list of words.\n"
   "@param index The zero-based index of the word in @a text.\n"
   "@return A new string with the word at the given index removed or the original string if @a index is "
      "out of range.\n\n"
   "@tsexample\n"
      "removeWord( \"a b c d\", 2 ) // Returns \"a b d\"\n"
   "@endtsexample\n\n"
   "@see removeToken\n"
   "@see removeField\n"
   "@see removeRecord\n"
   "@ingroup FieldManip" )
{
   return Con::getReturnBuffer( StringUnit::removeUnit( text, index, " \t\n" ) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getWordCount, S32, ( const char* text ),,
   "Return the number of whitespace-separated words in @a text.\n"
   "Words in @a text must be separated by newlines, spaces, and/or tabs.\n"
   "@param text A whitespace-separated list of words.\n"
   "@return The number of whitespace-separated words in @a text.\n\n"
   "@tsexample\n"
      "getWordCount( \"a b c d e\" ) // Returns 5\n"
   "@endtsexample\n\n"
   "@see getTokenCount\n"
   "@see getFieldCount\n"
   "@see getRecordCount\n"
   "@ingroup FieldManip" )
{
   return StringUnit::getUnitCount( text, " \t\n" );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( monthNumToStr, String, ( S32 num, bool abbreviate ), (false),
   "@brief returns month as a word given a number or \"\" if number is bad"
   "@return month as a word given a number or \"\" if number is bad"
   "@ingroup FileSystem")
{
   switch(num)
   {
      case 1: return abbreviate ? "Jan" : "January"; break;
      case 2: return abbreviate ? "Feb" : "February"; break;
      case 3: return abbreviate ? "Mar" : "March"; break;
      case 4: return abbreviate ? "Apr" : "April"; break;
      case 5: return "May"; break;
      case 6: return abbreviate ? "Jun" : "June"; break;
      case 7: return abbreviate ? "Jul" : "July"; break;
      case 8: return abbreviate ? "Aug" : "August"; break;
      case 9: return abbreviate ? "Sep" : "September"; break;
      case 10: return abbreviate ? "Oct" : "October"; break;
      case 11: return abbreviate ? "Nov" : "November"; break;
      case 12: return abbreviate ? "Dec" : "December"; break;
      default: return "";
   }
}

DefineEngineFunction( weekdayNumToStr, String, ( S32 num, bool abbreviate ), (false),
   "@brief returns weekday as a word given a number or \"\" if number is bad"
   "@return weekday as a word given a number or \"\" if number is bad"
   "@ingroup FileSystem")
{
   switch(num)
   {
      case 0: return abbreviate ? "Sun" : "Sunday"; break;
      case 1: return abbreviate ? "Mon" : "Monday"; break;
      case 2: return abbreviate ? "Tue" : "Tuesday"; break;
      case 3: return abbreviate ? "Wed" : "Wednesday"; break;
      case 4: return abbreviate ? "Thu" : "Thursday"; break;
      case 5: return abbreviate ? "Fri" : "Friday"; break;
      case 6: return abbreviate ? "Sat" : "Saturday"; break;
      default: return "";
   }
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getField, const char*, ( const char* text, S32 index ),,
   "Extract the field at the given @a index in the newline and/or tab separated list in @a text.\n"
   "Fields in @a text must be separated by newlines and/or tabs.\n"
   "@param text A list of fields separated by newlines and/or tabs.\n"
   "@param index The zero-based index of the field to extract.\n"
   "@return The field at the given index or \"\" if the index is out of range.\n\n"
   "@tsexample\n"
      "getField( \"a b\" TAB \"c d\" TAB \"e f\", 1 ) // Returns \"c d\"\n"
   "@endtsexample\n\n"
   "@see getFields\n"
   "@see getFieldCount\n"
   "@see getWord\n"
   "@see getRecord\n"
   "@ingroup FieldManip" )
{
   return Con::getReturnBuffer( StringUnit::getUnit( text, index, "\t\n" ) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getFields, const char*, ( const char* text, S32 startIndex, S32 endIndex ), ( -1 ),
   "Extract a range of fields from the given @a startIndex onwards thru @a endIndex.\n"
   "Fields in @a text must be separated by newlines and/or tabs.\n"
   "@param text A list of fields separated by newlines and/or tabs.\n"
   "@param startIndex The zero-based index of the first field to extract from @a text.\n"
   "@param endIndex The zero-based index of the last field to extract from @a text.  If this is -1, all fields beginning "
      "with @a startIndex are extracted from @a text.\n"
   "@return A string containing the specified range of fields from @a text or \"\" if @a startIndex "
      "is out of range or greater than @a endIndex.\n\n"
   "@tsexample\n"
      "getFields( \"a b\" TAB \"c d\" TAB \"e f\", 1 ) // Returns \"c d\" TAB \"e f\"\n"
   "@endtsexample\n\n"
   "@see getField\n"
   "@see getFieldCount\n"
   "@see getWords\n"
   "@see getRecords\n"
   "@ingroup FieldManip" )
{
   if( endIndex < 0 )
      endIndex = 1000000;

   return Con::getReturnBuffer( StringUnit::getUnits( text, startIndex, endIndex, "\t\n" ) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( setField, const char*, ( const char* text, S32 index, const char* replacement ),,
   "Replace the field in @a text at the given @a index with @a replacement.\n"
   "Fields in @a text must be separated by newlines and/or tabs.\n"
   "@param text A list of fields separated by newlines and/or tabs.\n"
   "@param index The zero-based index of the field to replace.\n"
   "@param replacement The string with which to replace the field.\n"
   "@return A new string with the field at the given @a index replaced by @a replacement or the original "
      "string if @a index is out of range.\n\n"
   "@tsexample\n"
      "setField( \"a b\" TAB \"c d\" TAB \"e f\", 1, \"g h\" ) // Returns \"a b\" TAB \"g h\" TAB \"e f\"\n"
   "@endtsexample\n\n"
   "@see getField\n"
   "@see setWord\n"
   "@see setRecord\n"
   "@ingroup FieldManip" )
{
   return Con::getReturnBuffer( StringUnit::setUnit( text, index, replacement, "\t\n" ) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( removeField, const char*, ( const char* text, S32 index ),,
   "Remove the field in @a text at the given @a index.\n"
   "Fields in @a text must be separated by newlines and/or tabs.\n"
   "@param text A list of fields separated by newlines and/or tabs.\n"
   "@param index The zero-based index of the field in @a text.\n"
   "@return A new string with the field at the given index removed or the original string if @a index is "
      "out of range.\n\n"
   "@tsexample\n"
      "removeField( \"a b\" TAB \"c d\" TAB \"e f\", 1 ) // Returns \"a b\" TAB \"e f\"\n"
   "@endtsexample\n\n"
   "@see removeWord\n"
   "@see removeRecord\n"
   "@ingroup FieldManip" )
{
   return Con::getReturnBuffer( StringUnit::removeUnit( text, index, "\t\n" ) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getFieldCount, S32, ( const char* text ),,
   "Return the number of newline and/or tab separated fields in @a text.\n"
   "@param text A list of fields separated by newlines and/or tabs.\n"
   "@return The number of newline and/or tab sepearated elements in @a text.\n\n"
   "@tsexample\n"
      "getFieldCount( \"a b\" TAB \"c d\" TAB \"e f\" ) // Returns 3\n"
   "@endtsexample\n\n"
   "@see getWordCount\n"
   "@see getRecordCount\n"
   "@ingroup FieldManip" )
{
   return StringUnit::getUnitCount( text, "\t\n" );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getRecord, const char*, ( const char* text, S32 index ),,
   "Extract the record at the given @a index in the newline-separated list in @a text.\n"
   "Records in @a text must be separated by newlines.\n"
   "@param text A list of records separated by newlines.\n"
   "@param index The zero-based index of the record to extract.\n"
   "@return The record at the given index or \"\" if @a index is out of range.\n\n"
   "@tsexample\n"
      "getRecord( \"a b\" NL \"c d\" NL \"e f\", 1 ) // Returns \"c d\"\n"
   "@endtsexample\n\n"
   "@see getRecords\n"
   "@see getRecordCount\n"
   "@see getWord\n"
   "@see getField\n"
   "@ingroup FieldManip" )
{
   return Con::getReturnBuffer( StringUnit::getUnit( text, index, "\n" ) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getRecords, const char*, ( const char* text, S32 startIndex, S32 endIndex ), ( -1 ),
   "Extract a range of records from the given @a startIndex onwards thru @a endIndex.\n"
   "Records in @a text must be separated by newlines.\n"
   "@param text A list of records separated by newlines.\n"
   "@param startIndex The zero-based index of the first record to extract from @a text.\n"
   "@param endIndex The zero-based index of the last record to extract from @a text.  If this is -1, all records beginning "
      "with @a startIndex are extracted from @a text.\n"
   "@return A string containing the specified range of records from @a text or \"\" if @a startIndex "
      "is out of range or greater than @a endIndex.\n\n"
   "@tsexample\n"
      "getRecords( \"a b\" NL \"c d\" NL \"e f\", 1 ) // Returns \"c d\" NL \"e f\"\n"
   "@endtsexample\n\n"
   "@see getRecord\n"
   "@see getRecordCount\n"
   "@see getWords\n"
   "@see getFields\n"
   "@ingroup FieldManip" )
{
   if( endIndex < 0 )
      endIndex = 1000000;

   return Con::getReturnBuffer( StringUnit::getUnits( text, startIndex, endIndex, "\n" ) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( setRecord, const char*, ( const char* text, S32 index, const char* replacement ),,
   "Replace the record in @a text at the given @a index with @a replacement.\n"
   "Records in @a text must be separated by newlines.\n"
   "@param text A list of records separated by newlines.\n"
   "@param index The zero-based index of the record to replace.\n"
   "@param replacement The string with which to replace the record.\n"
   "@return A new string with the record at the given @a index replaced by @a replacement or the original "
      "string if @a index is out of range.\n\n"
   "@tsexample\n"
      "setRecord( \"a b\" NL \"c d\" NL \"e f\", 1, \"g h\" ) // Returns \"a b\" NL \"g h\" NL \"e f\"\n"
   "@endtsexample\n\n"
   "@see getRecord\n"
   "@see setWord\n"
   "@see setField\n"
   "@ingroup FieldManip" )
{
   return Con::getReturnBuffer( StringUnit::setUnit( text, index, replacement, "\n" ) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( removeRecord, const char*, ( const char* text, S32 index ),,
   "Remove the record in @a text at the given @a index.\n"
   "Records in @a text must be separated by newlines.\n"
   "@param text A list of records separated by newlines.\n"
   "@param index The zero-based index of the record in @a text.\n"
   "@return A new string with the record at the given @a index removed or the original string if @a index is "
      "out of range.\n\n"
   "@tsexample\n"
      "removeRecord( \"a b\" NL \"c d\" NL \"e f\", 1 ) // Returns \"a b\" NL \"e f\"\n"
   "@endtsexample\n\n"
   "@see removeWord\n"
   "@see removeField\n"
   "@ingroup FieldManip" )
{
   return Con::getReturnBuffer( StringUnit::removeUnit( text, index, "\n" ) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getRecordCount, S32, ( const char* text ),,
   "Return the number of newline-separated records in @a text.\n"
   "@param text A list of records separated by newlines.\n"
   "@return The number of newline-sepearated elements in @a text.\n\n"
   "@tsexample\n"
      "getRecordCount( \"a b\" NL \"c d\" NL \"e f\" ) // Returns 3\n"
   "@endtsexample\n\n"
   "@see getWordCount\n"
   "@see getFieldCount\n"
   "@ingroup FieldManip" )
{
   return StringUnit::getUnitCount( text, "\n" );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( firstWord, const char*, ( const char* text ),,
   "Return the first word in @a text.\n"
   "@param text A list of words separated by newlines, spaces, and/or tabs.\n"
   "@return The word at index 0 in @a text or \"\" if @a text is empty.\n\n"
   "@note This is equal to \n"
   "@tsexample_nopar\n"
      "getWord( text, 0 )\n"
   "@endtsexample\n\n"
   "@see getWord\n"
   "@ingroup FieldManip" )
{
   return Con::getReturnBuffer( StringUnit::getUnit( text, 0, " \t\n" ) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( restWords, const char*, ( const char* text ),,
   "Return all but the first word in @a text.\n"
   "@param text A list of words separated by newlines, spaces, and/or tabs.\n"
   "@return @a text with the first word removed.\n\n"
   "@note This is equal to \n"
   "@tsexample_nopar\n"
      "getWords( text, 1 )\n"
   "@endtsexample\n\n"
   "@see getWords\n"
   "@ingroup FieldManip" )
{
   const char* ptr = text;
   while( *ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' )
      ptr ++;
      
   // Skip separator.
   if( *ptr )
      ptr ++;
      
   return Con::getReturnBuffer( ptr );
}

//-----------------------------------------------------------------------------

static bool isInSet(char c, const char *set)
{
   if (set)
      while (*set)
         if (c == *set++)
            return true;

   return false;
}

DefineConsoleFunction( nextToken, const char*, ( const char* str1, const char* token, const char* delim), , "( string str, string token, string delimiters ) "
   "Tokenize a string using a set of delimiting characters.\n"
   "This function first skips all leading charaters in @a str that are contained in @a delimiters. "
   "From that position, it then scans for the next character in @a str that is contained in @a delimiters and stores all characters "
   "from the starting position up to the first delimiter in a variable in the current scope called @a token.  Finally, it "
   "skips all characters in @a delimiters after the token and then returns the remaining string contents in @a str.\n\n"
   "To scan out all tokens in a string, call this function repeatedly by passing the result it returns each time as the new @a str "
   "until the function returns \"\".\n\n"
   "@param str A string.\n"
   "@param token The name of the variable in which to store the current token.  This variable is set in the "
      "scope in which nextToken is called.\n"
   "@param delimiters A string of characters.  Each character is considered a delimiter.\n"
   "@return The remainder of @a str after the token has been parsed out or \"\" if no more tokens were found in @a str.\n\n"
   "@tsexample\n"
      "// Prints:\n"
      "// a\n"
      "// b\n"
      "// c\n"
      "%str = \"a   b c\";\n"
      "while ( %str !$= \"\" )\n"
      "{\n"
      "   // First time, stores \"a\" in the variable %token and sets %str to \"b c\".\n"
      "   %str = nextToken( %str, \"token\", \" \" );\n"
      "   echo( %token );\n"
      "}\n"
   "@endtsexample\n\n"
   "@ingroup Strings" )
{
	char buffer[4096];
   dStrncpy(buffer, str1, 4096);
   char *str = buffer;

   if( str[0] )
   {
      // skip over any characters that are a member of delim
      // no need for special '\0' check since it can never be in delim
      while (isInSet(*str, delim))
         str++;

      // skip over any characters that are NOT a member of delim
      const char *tmp = str;

      while (*str && !isInSet(*str, delim))
         str++;

      // terminate the token
      if (*str)
         *str++ = 0;

      // set local variable if inside a function
      if (gEvalState.getStackDepth() > 0 && 
         gEvalState.getCurrentFrame().scopeName)
         Con::setLocalVariable(token,tmp);
      else
         Con::setVariable(token,tmp);

      // advance str past the 'delim space'
      while (isInSet(*str, delim))
         str++;
   }

   U32 returnLen = dStrlen(str)+1;
   char *ret = Con::getReturnBuffer(returnLen);
   dStrncpy(ret, str, returnLen);
   return ret;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getToken, const char*, ( const char* text, const char* delimiters, S32 index ),,
   "Extract the substring at the given @a index in the @a delimiters separated list in @a text.\n"
   "@param text A @a delimiters list of substrings.\n"
   "@param delimiters Character or characters that separate the list of substrings in @a text.\n"
   "@param index The zero-based index of the substring to extract.\n"
   "@return The substring at the given index or \"\" if the index is out of range.\n\n"
   "@tsexample\n"
      "getToken( \"a b c d\", \" \", 2 ) // Returns \"c\"\n"
   "@endtsexample\n\n"
   "@see getTokens\n"
   "@see getTokenCount\n"
   "@see getWord\n"
   "@see getField\n"
   "@see getRecord\n"
   "@ingroup FieldManip" )
{
   return Con::getReturnBuffer( StringUnit::getUnit(text, index, delimiters));
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getTokens, const char*, ( const char* text, const char* delimiters, S32 startIndex, S32 endIndex ), ( -1 ),
   "Extract a range of substrings separated by @a delimiters at the given @a startIndex onwards thru @a endIndex.\n"
   "@param text A @a delimiters list of substrings.\n"
   "@param delimiters Character or characters that separate the list of substrings in @a text.\n"
   "@param startIndex The zero-based index of the first substring to extract from @a text.\n"
   "@param endIndex The zero-based index of the last substring to extract from @a text.  If this is -1, all words beginning "
      "with @a startIndex are extracted from @a text.\n"
   "@return A string containing the specified range of substrings from @a text or \"\" if @a startIndex "
      "is out of range or greater than @a endIndex.\n\n"
   "@tsexample\n"
      "getTokens( \"a b c d\", \" \", 1, 2, ) // Returns \"b c\"\n"
   "@endtsexample\n\n"
   "@see getToken\n"
   "@see getTokenCount\n"
   "@see getWords\n"
   "@see getFields\n"
   "@see getRecords\n"
   "@ingroup FieldManip" )
{
   if( endIndex < 0 )
      endIndex = 1000000;

   return Con::getReturnBuffer( StringUnit::getUnits( text, startIndex, endIndex, delimiters ) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( setToken, const char*, ( const char* text, const char* delimiters, S32 index, const char* replacement ),,
   "Replace the substring in @a text separated by @a delimiters at the given @a index with @a replacement.\n"
   "@param text A @a delimiters list of substrings.\n"
   "@param delimiters Character or characters that separate the list of substrings in @a text.\n"
   "@param index The zero-based index of the substring to replace.\n"
   "@param replacement The string with which to replace the substring.\n"
   "@return A new string with the substring at the given @a index replaced by @a replacement or the original "
      "string if @a index is out of range.\n\n"
   "@tsexample\n"
      "setToken( \"a b c d\", \" \", 2, \"f\" ) // Returns \"a b f d\"\n"
   "@endtsexample\n\n"
   "@see getToken\n"
   "@see setWord\n"
   "@see setField\n"
   "@see setRecord\n"
   "@ingroup FieldManip" )
{
   return Con::getReturnBuffer( StringUnit::setUnit( text, index, replacement, delimiters) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( removeToken, const char*, ( const char* text, const char* delimiters, S32 index ),,
   "Remove the substring in @a text separated by @a delimiters at the given @a index.\n"
   "@param text A @a delimiters list of substrings.\n"
   "@param delimiters Character or characters that separate the list of substrings in @a text.\n"
   "@param index The zero-based index of the word in @a text.\n"
   "@return A new string with the substring at the given index removed or the original string if @a index is "
      "out of range.\n\n"
   "@tsexample\n"
      "removeToken( \"a b c d\", \" \", 2 ) // Returns \"a b d\"\n"
   "@endtsexample\n\n"
   "@see removeWord\n"
   "@see removeField\n"
   "@see removeRecord\n"
   "@ingroup FieldManip" )
{
   return Con::getReturnBuffer( StringUnit::removeUnit( text, index, delimiters ) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getTokenCount, S32, ( const char* text, const char* delimiters),,
   "Return the number of @a delimiters substrings in @a text.\n"
   "@param text A @a delimiters list of substrings.\n"
   "@param delimiters Character or characters that separate the list of substrings in @a text.\n"
   "@return The number of @a delimiters substrings in @a text.\n\n"
   "@tsexample\n"
      "getTokenCount( \"a b c d e\", \" \" ) // Returns 5\n"
   "@endtsexample\n\n"
   "@see getWordCount\n"
   "@see getFieldCount\n"
   "@see getRecordCount\n"
   "@ingroup FieldManip" )
{
   return StringUnit::getUnitCount( text, delimiters );
}

//=============================================================================
//    Tagged Strings.
//=============================================================================
// MARK: ---- Tagged Strings ----

//-----------------------------------------------------------------------------

DefineEngineFunction( detag, const char*, ( const char* str ),,
   "@brief Returns the string from a tag string.\n\n"

   "Should only be used within the context of a function that receives a tagged "
   "string, and is not meant to be used outside of this context.  Use getTaggedString() "
   "to convert a tagged string ID back into a regular string at any time.\n\n"

   "@tsexample\n"
      "// From scripts/client/message.cs\n"
      "function clientCmdChatMessage(%sender, %voice, %pitch, %msgString, %a1, %a2, %a3, %a4, %a5, %a6, %a7, %a8, %a9, %a10)\n"
      "{\n"
      "   onChatMessage(detag(%msgString), %voice, %pitch);\n"
      "}\n"
	"@endtsexample\n\n"

   "@see \\ref syntaxDataTypes under Tagged %Strings\n"
   "@see getTag()\n"
   "@see getTaggedString()\n"

   "@ingroup Networking")
{
   if( str[ 0 ] == StringTagPrefixByte )
   {
      const char* word = dStrchr( str, ' ' );
      if( word == NULL )
         return "";
         
      char* ret = Con::getReturnBuffer( dStrlen( word + 1 ) + 1 );
      dStrcpy( ret, word + 1 );
      return ret;
   }
   else
      return str;
}

DefineConsoleFunction( getTag, const char*, ( const char* textTagString ), , "( string textTagString ) "
   "@brief Extracts the tag from a tagged string\n\n"

   "Should only be used within the context of a function that receives a tagged "
   "string, and is not meant to be used outside of this context.\n\n"

   "@param textTagString The tagged string to extract.\n"

   "@returns The tag ID of the string.\n"

   "@see \\ref syntaxDataTypes under Tagged %Strings\n"
   "@see detag()\n"
   "@ingroup Networking")
{
   if(textTagString[0] == StringTagPrefixByte)
   {
      const char * space = dStrchr(textTagString, ' ');

      U64 len;
      if(space)
         len = space - textTagString;
      else
         len = dStrlen(textTagString) + 1;

      char * ret = Con::getReturnBuffer(len);
      dStrncpy(ret, textTagString + 1, len - 1);
      ret[len - 1] = 0;

      return(ret);
   }
   else
      return(textTagString);
}


//=============================================================================
//    Output.
//=============================================================================
// MARK: ---- Output ----

//-----------------------------------------------------------------------------

ConsoleFunction( echo, void, 2, 0, "( string message... ) "
   "@brief Logs a message to the console.\n\n"
   "Concatenates all given arguments to a single string and prints the string to the console. "
   "A newline is added automatically after the text.\n\n"
   "@param message Any number of string arguments.\n\n"
   "@ingroup Logging" )
{
   U32 len = 0;
   S32 i;
   for(i = 1; i < argc; i++)
      len += dStrlen(argv[i]);

   char *ret = Con::getReturnBuffer(len + 1);
   ret[0] = 0;
   for(i = 1; i < argc; i++)
      dStrcat(ret, argv[i]);

   Con::printf("%s", ret);
   ret[0] = 0;
}

//-----------------------------------------------------------------------------

ConsoleFunction( warn, void, 2, 0, "( string message... ) "
   "@brief Logs a warning message to the console.\n\n"
   "Concatenates all given arguments to a single string and prints the string to the console as a warning "
   "message (in the in-game console, these will show up using a turquoise font by default). "
   "A newline is added automatically after the text.\n\n"
   "@param message Any number of string arguments.\n\n"
   "@ingroup Logging" )
{
   U32 len = 0;
   S32 i;
   for(i = 1; i < argc; i++)
      len += dStrlen(argv[i]);

   char *ret = Con::getReturnBuffer(len + 1);
   ret[0] = 0;
   for(i = 1; i < argc; i++)
      dStrcat(ret, argv[i]);

   Con::warnf(ConsoleLogEntry::General, "%s", ret);
   ret[0] = 0;
}

//-----------------------------------------------------------------------------

ConsoleFunction( error, void, 2, 0, "( string message... ) "
   "@brief Logs an error message to the console.\n\n"
   "Concatenates all given arguments to a single string and prints the string to the console as an error "
   "message (in the in-game console, these will show up using a red font by default). "
   "A newline is added automatically after the text.\n\n"
   "@param message Any number of string arguments.\n\n"
   "@ingroup Logging" )
{
   U32 len = 0;
   S32 i;
   for(i = 1; i < argc; i++)
      len += dStrlen(argv[i]);

   char *ret = Con::getReturnBuffer(len + 1);
   ret[0] = 0;
   for(i = 1; i < argc; i++)
      dStrcat(ret, argv[i]);

   Con::errorf(ConsoleLogEntry::General, "%s", ret);
   ret[0] = 0;
}

//-----------------------------------------------------------------------------

DefineEngineFunction( debugv, void, ( const char* variableName ),,
   "@brief Logs the value of the given variable to the console.\n\n"
   "Prints a string of the form \"<variableName> = <variable value>\" to the console.\n\n"
   "@param variableName Name of the local or global variable to print.\n\n"
   "@tsexample\n"
      "%var = 1;\n"
      "debugv( \"%var\" ); // Prints \"%var = 1\"\n"
   "@endtsexample\n\n"
   "@ingroup Debugging" )
{
   if( variableName[ 0 ] == '%' )
      Con::errorf( "%s = %s", variableName, Con::getLocalVariable( variableName ) );
   else
      Con::errorf( "%s = %s", variableName, Con::getVariable( variableName ) );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( expandEscape, const char*, ( const char* text ),,
   "@brief Replace all characters in @a text that need to be escaped for the string to be a valid string literal with their "
   "respective escape sequences.\n\n"
   "All characters in @a text that cannot appear in a string literal will be replaced by an escape sequence (\\\\n, \\\\t, etc).\n\n"
   "The primary use of this function is for converting strings suitable for being passed as string literals "
   "to the TorqueScript compiler.\n\n"
   "@param text A string\n"
   "@return A duplicate of the text parameter with all unescaped characters that cannot appear in string literals replaced by their respective "
   "escape sequences.\n\n"
   "@tsxample\n"
   "expandEscape( \"str\" NL \"ing\" ) // Returns \"str\\ning\".\n"
   "@endtsxample\n\n"
   "@see collapseEscape\n"
   "@ingroup Strings")
{
   char* ret = Con::getReturnBuffer(dStrlen( text ) * 2 + 1 );  // worst case situation
   expandEscape( ret, text );
   return ret;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( collapseEscape, const char*, ( const char* text ),,
   "Replace all escape sequences in @a text with their respective character codes.\n\n"
   "This function replaces all escape sequences (\\\\n, \\\\t, etc) in the given string "
   "with the respective characters they represent.\n\n"
   "The primary use of this function is for converting strings from their literal form into "
   "their compiled/translated form, as is normally done by the TorqueScript compiler.\n\n"
   "@param text A string.\n"
   "@return A duplicate of @a text with all escape sequences replaced by their respective character codes.\n\n"
   "@tsexample\n"
      "// Print:\n"
      "//\n"
      "//    str\n"
      "//    ing\n"
      "//\n"
      "// to the console.  Note how the backslash in the string must be escaped here\n"
      "// in order to prevent the TorqueScript compiler from collapsing the escape\n"
      "// sequence in the resulting string.\n"
      "echo( collapseEscape( \"str\\ning\" ) );\n"
   "@endtsexample\n\n"
   "@see expandEscape\n\n"
   "@ingroup Strings" )
{
   char* ret = Con::getReturnBuffer( text );
   collapseEscape( ret );
   return ret;
}

//-----------------------------------------------------------------------------

DefineEngineFunction( setLogMode, void, ( S32 mode ),,
	"@brief Determines how log files are written.\n\n"
	"Sets the operational mode of the console logging system.\n\n"
   "@param mode Parameter specifying the logging mode.  This can be:\n"
      "- 1: Open and close the console log file for each seperate string of output.  This will ensure that all "
         "parts get written out to disk and that no parts remain in intermediate buffers even if the process crashes.\n"
      "- 2: Keep the log file open and write to it continuously.  This will make the system operate faster but "
         "if the process crashes, parts of the output may not have been written to disk yet and will be missing from "
         "the log.\n\n"
         
      "Additionally, when changing the log mode and thus opening a new log file, either of the two mode values may be "
      "combined by binary OR with 0x4 to cause the logging system to flush all console log messages that had already been "
      "issued to the console system into the newly created log file.\n\n"

	"@note Xbox 360 does not support logging to a file. Use Platform::OutputDebugStr in C++ instead."
	"@ingroup Logging" )
{
   Con::setLogMode( mode );
}

//=============================================================================
//    Misc.
//=============================================================================
// MARK: ---- Misc ----

//-----------------------------------------------------------------------------

DefineConsoleFunction( quit, void, ( ),,
   "Shut down the engine and exit its process.\n"
   "This function cleanly uninitializes the engine and then exits back to the system with a process "
   "exit status indicating a clean exit.\n\n"
   "@see quitWithErrorMessage\n\n"
   "@ingroup Platform" )
{
   Platform::postQuitMessage(0);
}

//-----------------------------------------------------------------------------


DefineConsoleFunction( realQuit, void, (), , "")
{
   Platform::postQuitMessage(0);
}


//-----------------------------------------------------------------------------

DefineConsoleFunction( quitWithErrorMessage, void, ( const char* message, S32 status ), (0),
   "Display an error message box showing the given @a message and then shut down the engine and exit its process.\n"
   "This function cleanly uninitialized the engine and then exits back to the system with a process "
   "exit status indicating an error.\n\n"
   "@param message The message to log to the console and show in an error message box.\n"
   "@param status  The status code to return to the OS.\n\n"
   "@see quit\n\n"
   "@ingroup Platform" )
{
   Con::errorf( message );
   Platform::AlertOK( "Error", message );
   
   // [rene 03/30/10] This was previously using forceShutdown which is a bad thing
   //    as the script code should not be allowed to pretty much hard-crash the engine
   //    and prevent proper shutdown.  Changed this to use postQuitMessage.
   
   Platform::postQuitMessage( status );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( quitWithStatus, void, ( S32 status ), (0),
   "Shut down the engine and exit its process.\n"
   "This function cleanly uninitializes the engine and then exits back to the system with a given "
   "return status code.\n\n"
   "@param status The status code to return to the OS.\n\n"
   "@see quitWithErrorMessage\n\n"
   "@ingroup Platform" )
{
   Platform::postQuitMessage(status);
}

//-----------------------------------------------------------------------------

DefineEngineFunction( gotoWebPage, void, ( const char* address ),,
   "Open the given URL or file in the user's web browser.\n\n"
   "@param address The address to open.  If this is not prefixed by a protocol specifier (\"...://\"), then "
      "the function checks whether the address refers to a file or directory and if so, prepends \"file://\" "
      "to @a adress; if the file check fails, \"http://\" is prepended to @a address.\n\n"
   "@tsexample\n"
      "gotoWebPage( \"http://www.garagegames.com\" );\n"
   "@endtsexample\n\n"
   "@ingroup Platform" )
{
   // If there's a protocol prefix in the address, just invoke
   // the browser on the given address.
   
   char* protocolSep = dStrstr( address,"://");
   if( protocolSep != NULL )
   {
      Platform::openWebBrowser( address );
      return;
   }

   // If we don't see a protocol seperator, then we know that some bullethead
   // sent us a bad url. We'll first check to see if a file inside the sandbox
   // with that name exists, then we'll just glom "http://" onto the front of 
   // the bogus url, and hope for the best.
   
   String addr;
   if( Platform::isFile( address ) || Platform::isDirectory( address ) )
   {
#ifdef TORQUE2D_TOOLS_FIXME
      addr = String::ToString( "file://%s", address );
#else
      addr = String::ToString( "file://%s/%s", Platform::getCurrentDirectory(), address );
#endif
   }
   else
      addr = String::ToString( "http://%s", address );
   
   Platform::openWebBrowser( addr );
   return;
}

//-----------------------------------------------------------------------------

DefineEngineFunction( displaySplashWindow, bool, (const char* path), (""),
   "Display a startup splash window suitable for showing while the engine still starts up.\n\n"
   "@note This is currently only implemented on Windows.\n\n"
   "@param path	relative path to splash screen image to display.\n"
   "@return True if the splash window could be successfully initialized.\n\n"
   "@ingroup Platform" )
{
   if (path == "")
   {
      path = Con::getVariable("$Core::splashWindowImage");
   }

   return Platform::displaySplashWindow(path);
}

DefineEngineFunction( closeSplashWindow, void, (),,
   "Close our startup splash window.\n\n"
   "@note This is currently only implemented on Windows.\n\n"
   "@ingroup Platform" )
{
   Platform::closeSplashWindow();
}
//-----------------------------------------------------------------------------

DefineEngineFunction( getWebDeployment, bool, (),,
   "Test whether Torque is running in web-deployment mode.\n"
   "In this mode, Torque will usually run within a browser and certain restrictions apply (e.g. Torque will not "
   "be able to enter fullscreen exclusive mode).\n"
   "@return True if Torque is running in web-deployment mode.\n"
   "@ingroup Platform" )
{
   return Platform::getWebDeployment();
}

//-----------------------------------------------------------------------------

DefineEngineFunction( countBits, S32, ( S32 v ),,
   "Count the number of bits that are set in the given 32 bit integer.\n"
   "@param v An integer value.\n\n"
   "@return The number of bits that are set in @a v.\n\n"
   "@ingroup Utilities" )
{
   S32 c = 0;

   // from 
   // http://graphics.stanford.edu/~seander/bithacks.html

   // for at most 32-bit values in v:
   c =  ((v & 0xfff) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1f;
   c += (((v & 0xfff000) >> 12) * 0x1001001001001ULL & 0x84210842108421ULL) % 
      0x1f;
   c += ((v >> 24) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1f;

#ifndef TORQUE_SHIPPING
   // since the above isn't very obvious, for debugging compute the count in a more 
   // traditional way and assert if it is different
   {
      S32 c2 = 0;
      S32 v2 = v;
      for (c2 = 0; v2; v2 >>= 1)
      {
         c2 += v2 & 1;
      }
      if (c2 != c)
         Con::errorf("countBits: Uh oh bit count mismatch");
      AssertFatal(c2 == c, "countBits: uh oh, bit count mismatch");
   }
#endif

   return c;
}

//-----------------------------------------------------------------------------

DefineEngineFunction( generateUUID, Torque::UUID, (),,
   "Generate a new universally unique identifier (UUID).\n\n"
   "@return A newly generated UUID.\n\n"
   "@ingroup Utilities" )
{
   Torque::UUID uuid;
   
   uuid.generate();
   return uuid;
}

//=============================================================================
//    Meta Scripting.
//=============================================================================
// MARK: ---- Meta Scripting ----

//-----------------------------------------------------------------------------

ConsoleFunction( call, const char *, 2, 0, "( string functionName, string args... ) "
   "Apply the given arguments to the specified global function and return the result of the call.\n\n"
   "@param functionName The name of the function to call.  This function must be in the global namespace, i.e. "
      "you cannot call a function in a namespace through #call.  Use eval() for that.\n"
   "@return The result of the function call.\n\n"
   "@tsexample\n"
      "function myFunction( %arg )\n"
      "{\n"
      "  return ( %arg SPC \"World!\" );\n"
      "}\n"
      "\n"
      "echo( call( \"myFunction\", \"Hello\" ) ); // Prints \"Hello World!\" to the console.\n"
   "@endtsexample\n\n"
   "@ingroup Scripting" )
{
   return Con::execute( argc - 1, argv + 1 );
}

//-----------------------------------------------------------------------------

static U32 execDepth = 0;
static U32 journalDepth = 1;

DefineConsoleFunction( getDSOPath, const char*, ( const char* scriptFileName ),,
   "Get the absolute path to the file in which the compiled code for the given script file will be stored.\n"
   "@param scriptFileName %Path to the .cs script file.\n"
   "@return The absolute path to the .dso file for the given script file.\n\n"
   "@note The compiler will store newly compiled DSOs in the prefs path but pre-existing DSOs will be loaded "
      "from the current paths.\n\n"
   "@see compile\n"
   "@see getPrefsPath\n"
   "@ingroup Scripting" )
{
   Con::expandScriptFilename( scriptFilenameBuffer, sizeof(scriptFilenameBuffer), scriptFileName );
   
   const char* filename = Con::getDSOPath(scriptFilenameBuffer);
   if(filename == NULL || *filename == 0)
      return "";

   return filename;
}

//-----------------------------------------------------------------------------

DefineEngineFunction( compile, bool, ( const char* fileName, bool overrideNoDSO ), ( false ),
   "Compile a file to bytecode.\n\n"
   "This function will read the TorqueScript code in the specified file, compile it to internal bytecode, and, "
   "if DSO generation is enabled or @a overrideNoDDSO is true, will store the compiled code in a .dso file "
   "in the current DSO path mirrorring the path of @a fileName.\n\n"
   "@param fileName Path to the file to compile to bytecode.\n"
   "@param overrideNoDSO If true, force generation of DSOs even if the engine is compiled to not "
      "generate write compiled code to DSO files.\n\n"
   "@return True if the file was successfully compiled, false if not.\n\n"
   "@note The definitions contained in the given file will not be made available and no code will actually "
      "be executed.  Use exec() for that.\n\n"
   "@see getDSOPath\n"
   "@see exec\n"
   "@ingroup Scripting" )
{
   Con::expandScriptFilename( scriptFilenameBuffer, sizeof( scriptFilenameBuffer ), fileName );

   // Figure out where to put DSOs
   StringTableEntry dsoPath = Con::getDSOPath(scriptFilenameBuffer);
   if(dsoPath && *dsoPath == 0)
      return false;

   // If the script file extention is '.ed.cs' then compile it to a different compiled extention
   bool isEditorScript = false;
   const char *ext = dStrrchr( scriptFilenameBuffer, '.' );
   if( ext && ( dStricmp( ext, ".cs" ) == 0 ) )
   {
      const char* ext2 = ext - 3;
      if( dStricmp( ext2, ".ed.cs" ) == 0 )
         isEditorScript = true;
   }
   else if( ext && ( dStricmp( ext, ".gui" ) == 0 ) )
   {
      const char* ext2 = ext - 3;
      if( dStricmp( ext2, ".ed.gui" ) == 0 )
         isEditorScript = true;
   }

   const char *filenameOnly = dStrrchr(scriptFilenameBuffer, '/');
   if(filenameOnly)
      ++filenameOnly;
   else
      filenameOnly = scriptFilenameBuffer;
 
   char nameBuffer[512];

   if( isEditorScript )
      dStrcpyl(nameBuffer, sizeof(nameBuffer), dsoPath, "/", filenameOnly, ".edso", NULL);
   else
      dStrcpyl(nameBuffer, sizeof(nameBuffer), dsoPath, "/", filenameOnly, ".dso", NULL);
   
   void *data = NULL;
   U32 dataSize = 0;
   Torque::FS::ReadFile(scriptFilenameBuffer, data, dataSize, true);
   if(data == NULL)
   {
      Con::errorf(ConsoleLogEntry::Script, "compile: invalid script file %s.", scriptFilenameBuffer);
      return false;
   }

   const char *script = static_cast<const char *>(data);

#ifdef TORQUE_DEBUG
   Con::printf("Compiling %s...", scriptFilenameBuffer);
#endif 

   CodeBlock *code = new CodeBlock();
   code->compile(nameBuffer, scriptFilenameBuffer, script, overrideNoDSO);
   delete code;
   delete[] script;

   return true;
}

//-----------------------------------------------------------------------------

DefineEngineFunction( exec, bool, ( const char* fileName, bool noCalls, bool journalScript ), ( false, false ),
   "Execute the given script file.\n"
   "@param fileName Path to the file to execute\n"
   "@param noCalls Deprecated\n"
   "@param journalScript Deprecated\n"
   "@return True if the script was successfully executed, false if not.\n\n"
   "@tsexample\n"
      "// Execute the init.cs script file found in the same directory as the current script file.\n"
      "exec( \"./init.cs\" );\n"
   "@endtsexample\n\n"
   "@see compile\n"
   "@see eval\n"
   "@ingroup Scripting" )
{
   return Con::executeFile(fileName, noCalls, journalScript);
}

DefineConsoleFunction( eval, const char*, ( const char* consoleString ), , "eval(consoleString)" )
{
   return Con::evaluate(consoleString, false, NULL);
}

DefineConsoleFunction( getVariable, const char*, ( const char* varName ), , "(string varName)\n" 
   "@brief Returns the value of the named variable or an empty string if not found.\n\n"
   "@varName Name of the variable to search for\n"
   "@return Value contained by varName, \"\" if the variable does not exist\n"
   "@ingroup Scripting")
{
   return Con::getVariable(varName);
}

DefineConsoleFunction( setVariable, void, ( const char* varName, const char* value ), , "(string varName, string value)\n" 
   "@brief Sets the value of the named variable.\n\n"
   "@param varName Name of the variable to locate\n"
   "@param value New value of the variable\n"
   "@return True if variable was successfully found and set\n"
   "@ingroup Scripting")
{
   return Con::setVariable(varName, value);
}

DefineConsoleFunction( isFunction, bool, ( const char* funcName ), , "(string funcName)" 
	"@brief Determines if a function exists or not\n\n"
	"@param funcName String containing name of the function\n"
	"@return True if the function exists, false if not\n"
	"@ingroup Scripting")
{
   return Con::isFunction(funcName);
}

DefineConsoleFunction( getFunctionPackage, const char*, ( const char* funcName ), , "(string funcName)" 
	"@brief Provides the name of the package the function belongs to\n\n"
	"@param funcName String containing name of the function\n"
	"@return The name of the function's package\n"
	"@ingroup Packages")
{
   Namespace::Entry* nse = Namespace::global()->lookup( StringTable->insert( funcName ) );
   if( !nse )
      return "";

   return nse->mPackage;
}

DefineConsoleFunction( isMethod, bool, ( const char* nameSpace, const char* method ), , "(string namespace, string method)" 
	"@brief Determines if a class/namespace method exists\n\n"
	"@param namespace Class or namespace, such as Player\n"
	"@param method Name of the function to search for\n"
	"@return True if the method exists, false if not\n"
	"@ingroup Scripting\n")
{
   Namespace* ns = Namespace::find( StringTable->insert( nameSpace ) );
   Namespace::Entry* nse = ns->lookup( StringTable->insert( method ) );
   if( !nse )
      return false;

   return true;
}

DefineConsoleFunction( getMethodPackage, const char*, ( const char* nameSpace, const char* method ), , "(string namespace, string method)" 
	"@brief Provides the name of the package the method belongs to\n\n"
	"@param namespace Class or namespace, such as Player\n"
	"@param method Name of the funciton to search for\n"
	"@return The name of the method's package\n"
	"@ingroup Packages")
{
   Namespace* ns = Namespace::find( StringTable->insert( nameSpace ) );
   if( !ns )
      return "";

   Namespace::Entry* nse = ns->lookup( StringTable->insert( method ) );
   if( !nse )
      return "";

   return nse->mPackage;
}

DefineConsoleFunction( isDefined, bool, ( const char* varName, const char* varValue ), ("") , "(string varName)" 
	"@brief Determines if a variable exists and contains a value\n"
	"@param varName Name of the variable to search for\n"
	"@return True if the variable was defined in script, false if not\n"
   "@tsexample\n"
      "isDefined( \"$myVar\" );\n"
   "@endtsexample\n\n"
	"@ingroup Scripting")
{
   if(String::isEmpty(varName))
   {
      Con::errorf("isDefined() - did you forget to put quotes around the variable name?");
      return false;
   }

   StringTableEntry name = StringTable->insert(varName);

   // Deal with <var>.<value>
   if (dStrchr(name, '.'))
   {
      static char scratchBuffer[4096];

      S32 len = dStrlen(name);
      AssertFatal(len < sizeof(scratchBuffer)-1, "isDefined() - name too long");
      dMemcpy(scratchBuffer, name, len+1);

      char * token = dStrtok(scratchBuffer, ".");

      if (!token || token[0] == '\0')
         return false;

      StringTableEntry objName = StringTable->insert(token);

      // Attempt to find the object
      SimObject * obj = Sim::findObject(objName);

      // If we didn't find the object then we can safely
      // assume that the field variable doesn't exist
      if (!obj)
         return false;

      // Get the name of the field
      token = dStrtok(0, ".\0");
      if (!token)
         return false;

      while (token != NULL)
      {
         StringTableEntry valName = StringTable->insert(token);

         // Store these so we can restore them after we search for the variable
         bool saveModStatic = obj->canModStaticFields();
         bool saveModDyn = obj->canModDynamicFields();

         // Set this so that we can search both static and dynamic fields
         obj->setModStaticFields(true);
         obj->setModDynamicFields(true);

         const char* value = obj->getDataField(valName, 0);

         // Restore our mod flags to be safe
         obj->setModStaticFields(saveModStatic);
         obj->setModDynamicFields(saveModDyn);

         if (!value)
         {
            obj->setDataField(valName, 0, varValue);

            return false;
         }
         else
         {
            // See if we are field on a field
            token = dStrtok(0, ".\0");
            if (token)
            {
               // The previous field must be an object
               obj = Sim::findObject(value);
               if (!obj)
                  return false;
            }
            else
            {
               if (dStrlen(value) > 0)
                  return true;
               else if (!String::isEmpty(varValue))
               { 
                  obj->setDataField(valName, 0, varValue); 
               }
            }
         }
      }
   }
   else if (name[0] == '%')
   {
      // Look up a local variable
      if( gEvalState.getStackDepth() > 0 )
      {
         Dictionary::Entry* ent = gEvalState.getCurrentFrame().lookup(name);

         if (ent)
            return true;
         else if (!String::isEmpty(varValue))
         {
            gEvalState.getCurrentFrame().setVariable(name, varValue);
         }
      }
      else
         Con::errorf("%s() - no local variable frame.", __FUNCTION__);
   }
   else if (name[0] == '$')
   {
      // Look up a global value
      Dictionary::Entry* ent = gEvalState.globalVars.lookup(name);

      if (ent)
         return true;
      else if (!String::isEmpty(varValue))
      {
         gEvalState.globalVars.setVariable(name, varValue);
      }
   }
   else
   {
      // Is it an object?
      if (dStrcmp(varName, "0") && dStrcmp(varName, "") && (Sim::findObject(varName) != NULL))
         return true;
      else if (!String::isEmpty(varValue))
      {
         Con::errorf("%s() - can't assign a value to a variable of the form \"%s\"", __FUNCTION__, varValue);
      }
   }

   return false;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( isCurrentScriptToolScript, bool, (), , "()" 
   "Returns true if the calling script is a tools script.\n"
   "@hide")
{
   return Con::isCurrentScriptToolScript();
}

DefineConsoleFunction( getModNameFromPath, const char *, ( const char* path ), , "(string path)" 
				"@brief Attempts to extract a mod directory from path. Returns empty string on failure.\n\n"
				"@param File path of mod folder\n"
				"@note This is no longer relevant in Torque 3D (which does not use mod folders), should be deprecated\n"
				"@internal")
{
   StringTableEntry modPath = Con::getModNameFromPath(path);
   return modPath ? modPath : "";
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( pushInstantGroup, void, ( String group ),("") , "([group])" 
				"@brief Pushes the current $instantGroup on a stack "
				"and sets it to the given value (or clears it).\n\n"
				"@note Currently only used for editors\n"
				"@ingroup Editors\n"
				"@internal")
{
   if( group.size() > 0 )
      Con::pushInstantGroup( group );
   else
      Con::pushInstantGroup();
}

DefineConsoleFunction( popInstantGroup, void, (), , "()" 
				"@brief Pop and restore the last setting of $instantGroup off the stack.\n\n"
				"@note Currently only used for editors\n\n"
				"@ingroup Editors\n"
				"@internal")
{
   Con::popInstantGroup();
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( getPrefsPath, const char *, ( const char* relativeFileName ), (""), "([relativeFileName])" 
				"@note Appears to be useless in Torque 3D, should be deprecated\n"
				"@internal")
{
   const char *filename = Platform::getPrefsPath(relativeFileName);
   if(filename == NULL || *filename == 0)
      return "";
     
   return filename;
}

//-----------------------------------------------------------------------------

ConsoleFunction( execPrefs, bool, 2, 4, "( string relativeFileName, bool noCalls=false, bool journalScript=false )"
				"@brief Manually execute a special script file that contains game or editor preferences\n\n"
				"@param relativeFileName Name and path to file from project folder\n"
				"@param noCalls Deprecated\n"
				"@param journalScript Deprecated\n"
				"@return True if script was successfully executed\n"
				"@note Appears to be useless in Torque 3D, should be deprecated\n"
				"@ingroup Scripting")
{
   const char *filename = Platform::getPrefsPath(argv[1]);
   if(filename == NULL || *filename == 0)
      return false;

   // Scripts do this a lot, so we may as well help them out
   if(! Platform::isFile(filename) && ! Torque::FS::IsFile(filename))
      return true;

   argv[0] = "exec";
   argv[1] = filename;
   return dAtob(Con::execute(argc, argv));
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( export, void, ( const char* pattern, const char* filename, bool append ), ( "", false ),
   "Write out the definitions of all global variables matching the given name @a pattern.\n"
   "If @a fileName is not \"\", the variable definitions are written to the specified file.  Otherwise the "
   "definitions will be printed to the console.\n\n"
   "The output are valid TorqueScript statements that can be executed to restore the global variable "
   "values.\n\n"
   "@param pattern A global variable name pattern.  Must begin with '$'.\n"
   "@param filename %Path of the file to which to write the definitions or \"\" to write the definitions "
      "to the console.\n"
   "@param append If true and @a fileName is not \"\", then the definitions are appended to the specified file. "
      "Otherwise existing contents of the file (if any) will be overwritten.\n\n"
   "@tsexample\n"
      "// Write out all preference variables to a prefs.cs file.\n"
      "export( \"$prefs::*\", \"prefs.cs\" );\n"
   "@endtsexample\n\n"
   "@ingroup Scripting" )
{
   if( filename && filename[ 0 ] )
   {
#ifndef TORQUE2D_TOOLS_FIXME
      if(Con::expandScriptFilename(scriptFilenameBuffer, sizeof(scriptFilenameBuffer), filename))
         filename = scriptFilenameBuffer;
#else
      filename = Platform::getPrefsPath( filename );
      if(filename == NULL || *filename == 0)
         return;
#endif
   }
   else
      filename = NULL;

   gEvalState.globalVars.exportVariables( pattern, filename, append );
}

//-----------------------------------------------------------------------------

DefineEngineFunction( deleteVariables, void, ( const char* pattern ),,
   "Undefine all global variables matching the given name @a pattern.\n"
   "@param pattern A global variable name pattern.  Must begin with '$'.\n"
   "@tsexample\n"
      "// Define a global variable in the \"My\" namespace.\n"
      "$My::Variable = \"value\";\n\n"
      "// Undefine all variable in the \"My\" namespace.\n"
      "deleteVariables( \"$My::*\" );\n"
   "@endtsexample\n\n"
   "@see strIsMatchExpr\n"
   "@ingroup Scripting" )
{
   gEvalState.globalVars.deleteVariables( pattern );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( trace, void, ( bool enable ), ( true ),
   "Enable or disable tracing in the script code VM.\n\n"
   "When enabled, the script code runtime will trace the invocation and returns "
   "from all functions that are called and log them to the console. This is helpful in "
   "observing the flow of the script program.\n\n"
   "@param enable New setting for script trace execution, on by default.\n"
   "@ingroup Debugging" )
{
   gEvalState.traceOn = enable;
   Con::printf( "Console trace %s", gEvalState.traceOn ? "enabled." : "disabled." );
}

//-----------------------------------------------------------------------------

#if defined(TORQUE_DEBUG) || !defined(TORQUE_SHIPPING)

DefineConsoleFunction( debug, void, (),,
   "Drops the engine into the native C++ debugger.\n\n"
   "This function triggers a debug break and drops the process into the IDE's debugger.  If the process is not "
   "running with a debugger attached it will generate a runtime error on most platforms.\n\n"
   "@note This function is not available in shipping builds."
   "@ingroup Debugging" )
{
   Platform::debugBreak();
}

#endif

//-----------------------------------------------------------------------------

DefineEngineFunction( isShippingBuild, bool, (),,
   "Test whether the engine has been compiled with TORQUE_SHIPPING, i.e. in a form meant for final release.\n\n"
   "@return True if this is a shipping build; false otherwise.\n\n"
   "@ingroup Platform" )
{
#ifdef TORQUE_SHIPPING
   return true;
#else
   return false;
#endif
}

//-----------------------------------------------------------------------------

DefineEngineFunction( isDebugBuild, bool, (),,
   "Test whether the engine has been compiled with TORQUE_DEBUG, i.e. if it includes debugging functionality.\n\n"
   "@return True if this is a debug build; false otherwise.\n\n"
   "@ingroup Platform" )
{
#ifdef TORQUE_DEBUG
   return true;
#else
   return false;
#endif
}

//-----------------------------------------------------------------------------

DefineEngineFunction( isToolBuild, bool, (),,
   "Test whether the engine has been compiled with TORQUE_TOOLS, i.e. if it includes tool-related functionality.\n\n"
   "@return True if this is a tool build; false otherwise.\n\n"
   "@ingroup Platform" )
{
#ifdef TORQUE_TOOLS
   return true;
#else
   return false;
#endif
}

DefineEngineFunction( getMaxDynamicVerts, S32, (),,
	"Get max number of allowable dynamic vertices in a single vertex buffer.\n\n"
	"@return the max number of allowable dynamic vertices in a single vertex buffer" )
{
   return MAX_DYNAMIC_VERTS / 2;
}
