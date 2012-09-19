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

#include "gfx/gfxDevice.h"


GFXVideoMode::GFXVideoMode()
{
   bitDepth = 32;
   fullScreen = false;
   refreshRate = 60;
   wideScreen = false;
   resolution.set(800,600);
   antialiasLevel = 0;
}

void GFXVideoMode::parseFromString( const char *str )
{
   if(!str)
      return;

   // Copy the string, as dStrtok is destructive
   char *tempBuf = new char[dStrlen( str ) + 1];
   dStrcpy( tempBuf, str );

#define PARSE_ELEM(type, var, func, tokParam, sep) \
   if(const char *ptr = dStrtok( tokParam, sep)) \
   { type tmp = func(ptr); if(tmp > 0) var = tmp; }

   PARSE_ELEM(S32, resolution.x, dAtoi, tempBuf, " x\0")
   PARSE_ELEM(S32, resolution.y, dAtoi, NULL,    " x\0")
   PARSE_ELEM(S32, fullScreen,   dAtob, NULL,    " \0")
   PARSE_ELEM(S32, bitDepth,     dAtoi, NULL,    " \0")
   PARSE_ELEM(S32, refreshRate,  dAtoi, NULL,    " \0")
   PARSE_ELEM(S32, antialiasLevel, dAtoi, NULL,    " \0")

#undef PARSE_ELEM

   delete [] tempBuf;
}

const String GFXVideoMode::toString() const
{
   return String::ToString("%d %d %s %d %d %d", resolution.x, resolution.y, (fullScreen ? "true" : "false"), bitDepth,  refreshRate, antialiasLevel);
}

void GFXShaderMacro::stringize( const Vector<GFXShaderMacro> &macros, String *outString )
{
   Vector<GFXShaderMacro>::const_iterator itr = macros.begin();
   for ( ; itr != macros.end(); itr++ )
   {
      (*outString) += itr->name;
      if ( itr->value.isNotEmpty() )
      {
         (*outString) += "=";
         (*outString) += itr->value;
      }
      (*outString) += ";";
   }
}