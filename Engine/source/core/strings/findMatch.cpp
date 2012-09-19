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

#include "core/strings/findMatch.h"
#include "core/strings/stringFunctions.h"


//--------------------------------------------------------------------------------
// NAME
//   FindMatch::FindMatch( const char *_expression, S32 maxNumMatches )
//
// DESCRIPTION
//   Class to match regular expressions (file names)
//   only works with '*','?', and 'chars'
//
// ARGUMENTS
//   _expression  -  The regular expression you intend to match (*.??abc.bmp)
//   _maxMatches  -  The maximum number of strings you wish to match.
//
// RETURNS
//
// NOTES
//
//--------------------------------------------------------------------------------

FindMatch::FindMatch( U32 _maxMatches )
{
   VECTOR_SET_ASSOCIATION(matchList);

   expression = NULL;
   maxMatches = _maxMatches;
   matchList.reserve( maxMatches );
}

FindMatch::FindMatch( char *_expression, U32 _maxMatches )
{
   VECTOR_SET_ASSOCIATION(matchList);

   expression = NULL;
   setExpression( _expression );
   maxMatches = _maxMatches;
   matchList.reserve( maxMatches );
}

FindMatch::~FindMatch()
{
   delete [] expression;
   matchList.clear();
}

void FindMatch::setExpression( const char *_expression )
{
   delete [] expression;

   expression = new char[dStrlen(_expression) + 1];
   dStrcpy(expression, _expression);
   dStrupr(expression);
}

bool FindMatch::findMatch( const char *str, bool caseSensitive )
{
   if ( isFull() )
      return false;

   char nstr[512];
   dStrcpy( nstr,str );
   dStrupr(nstr);
   if ( isMatch( expression, nstr, caseSensitive ) )
   {
      matchList.push_back( (char*)str );
      return true;
   }
   return false;
}

inline bool IsCharMatch( char e, char s, bool caseSensitive )
{
   return ( ( e == '?' ) || ( caseSensitive && e == s ) || ( dToupper(e) == dToupper(s) ) );
}

bool FindMatch::isMatch( const char *exp, const char *str, bool caseSensitive )
{
   while ( *str && ( *exp != '*' ) )
   {
      if ( !IsCharMatch( *exp++, *str++, caseSensitive ) )
         return false;
   }

   const char* cp = NULL;
   const char* mp = NULL;

   while ( *str )
   {
      if ( *exp == '*' )
      {
         if ( !*++exp )
            return true;

         mp = exp;
         cp = str+1;
      }
      else if ( IsCharMatch( *exp, *str, caseSensitive ) )
      {
         exp++;
         str++;
      }
      else
      {
         exp = mp;
         str = cp++;
      }
   }

   while ( *exp == '*' )
      exp++;

   return !*exp;
}


bool FindMatch::isMatchMultipleExprs( const char *exps, const char *str, bool caseSensitive )
{
   char *tok = 0;
   int len = dStrlen(exps);

   char *e = new char[len+1];
   dStrcpy(e,exps);

   // [tom, 12/18/2006] This no longer supports space separated expressions as
   // they don't work when the paths have spaces in.

   // search for each expression. return true soon as we see one.
   for( tok = dStrtok(e,"\t"); tok != NULL; tok = dStrtok(NULL,"\t"))
   {
      if( isMatch( tok, str, caseSensitive) )
      {
         delete []e;
         return true;
      }
   }

   delete []e;
   return false;
}
