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

#import "platform/platform.h"
#import "console/console.h"
#import "math/mMath.h"
#import "core/strings/stringFunctions.h"

extern void mInstallLibrary_C();
extern void mInstallLibrary_Vec();
extern void mInstall_Library_SSE();

static MRandomLCG sgPlatRandom;

U32 Platform::getMathControlState()
{
   U16 cw;
   asm("fstcw %0" : "=m" (cw) :);
   return cw;
}

void Platform::setMathControlState(U32 state)
{
   U16 cw = state;
   asm("fldcw %0" : : "m" (cw));
}

void Platform::setMathControlStateKnown()
{
   U16 cw = 0x27F;
   asm("fldcw %0" : : "m" (cw));
}

//--------------------------------------
ConsoleFunction( MathInit, void, 1, 10, "(DETECT|C|SSE)")
{
   U32 properties = CPU_PROP_C;  // C entensions are always used
   
   if (argc == 1)
   {
         Math::init(0);
         return;
   }
   for (argc--, argv++; argc; argc--, argv++)
   {
      if (dStricmp(*argv, "DETECT") == 0) { 
         Math::init(0);
         return;
      }
      if (dStricmp(*argv, "C") == 0) { 
         properties |= CPU_PROP_C; 
         continue; 
      }
      if( dStricmp( *argv, "SSE" ) == 0 )
      {
         properties |= CPU_PROP_SSE;
         continue;
      }
      //Con::printf("Error: MathInit(): ignoring unknown math extension '%s'", *argv);
   }
   Math::init(properties);
}



//------------------------------------------------------------------------------
void Math::init(U32 properties)
{
   if (!properties)
      // detect what's available
      properties = Platform::SystemInfo.processor.properties;  
   else
      // Make sure we're not asking for anything that's not supported
      properties &= Platform::SystemInfo.processor.properties;  

   Con::printf("Math Init:");
   Con::printf("   Installing Standard C extensions");
   mInstallLibrary_C();   

   #ifdef TORQUE_CPU_X86
   if( properties & CPU_PROP_SSE )
   {
      Con::printf( "   Installing SSE extensions" );
      mInstall_Library_SSE();
   }
   #endif
   
   Con::printf(" ");
}   

//------------------------------------------------------------------------------
F32 Platform::getRandom()
{
   return sgPlatRandom.randF();
}

