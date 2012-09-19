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
#include "platform/platformFont.h"


const char *getCharSetName(const U32 charSet)
{
   switch(charSet)
   {
   case TGE_ANSI_CHARSET:        return "ansi";
   case TGE_SYMBOL_CHARSET:      return "symbol";
   case TGE_SHIFTJIS_CHARSET:    return "shiftjis";
   case TGE_HANGEUL_CHARSET:     return "hangeul";
   case TGE_HANGUL_CHARSET:      return "hangul";
   case TGE_GB2312_CHARSET:      return "gb2312";
   case TGE_CHINESEBIG5_CHARSET: return "chinesebig5";
   case TGE_OEM_CHARSET:         return "oem";
   case TGE_JOHAB_CHARSET:       return "johab";
   case TGE_HEBREW_CHARSET:      return "hebrew";
   case TGE_ARABIC_CHARSET:      return "arabic";
   case TGE_GREEK_CHARSET:       return "greek";
   case TGE_TURKISH_CHARSET:     return "turkish";
   case TGE_VIETNAMESE_CHARSET:  return "vietnamese";
   case TGE_THAI_CHARSET:        return "thai";
   case TGE_EASTEUROPE_CHARSET:  return "easteurope";
   case TGE_RUSSIAN_CHARSET:     return "russian";
   case TGE_MAC_CHARSET:         return "mac";
   case TGE_BALTIC_CHARSET:      return "baltic";
   }

   AssertISV(false, "getCharSetName - unknown charset! Update table in platformString.cc!");
   return "";
}