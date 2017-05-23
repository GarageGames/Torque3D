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

#include "platformX86UNIX/platformX86UNIX.h"
#include "core/strings/stringFunctions.h"
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>


const char *stristr(const char *szStringToBeSearched, const char *szSubstringToSearchFor)
{
   const char *pPos = NULL;
   char *szCopy1 = NULL;
   char *szCopy2 = NULL;

   // verify parameters
   if ( szStringToBeSearched == NULL ||
        szSubstringToSearchFor == NULL )
   {
      return szStringToBeSearched;
   }

   // empty substring - return input (consistent with strstr)
   if (strlen(szSubstringToSearchFor) == 0 ) {
      return szStringToBeSearched;
   }

   szCopy1 = dStrlwr(strdup(szStringToBeSearched));
   szCopy2 = dStrlwr(strdup(szSubstringToSearchFor));

   if ( szCopy1 == NULL || szCopy2 == NULL  ) {
      // another option is to raise an exception here
      free((void*)szCopy1);
      free((void*)szCopy2);
      return NULL;
   }

   pPos = strstr((const char*)szCopy1, (const char*)szCopy2);

   if ( pPos != NULL ) {
      // map to the original string
      pPos = szStringToBeSearched + (pPos - szCopy1);
   }

   free((void*)szCopy1);
   free((void*)szCopy2);

   return pPos;
} // stristr(...)

