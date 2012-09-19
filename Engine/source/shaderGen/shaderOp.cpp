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

#include "core/strings/stringFunctions.h"
#include <stdarg.h>


#include "shaderOp.h"

//**************************************************************************
// Shader Operations
//**************************************************************************
ShaderOp::ShaderOp( LangElement *in1, LangElement *in2 )
{
   mInput[0] = in1;
   mInput[1] = in2;
}

//**************************************************************************
// Declaration Operation - for variables
//**************************************************************************
DecOp::DecOp( Var *in1 ) : Parent( in1, NULL )
{
   mInput[0] = in1;
}

//--------------------------------------------------------------------------
// Print
//--------------------------------------------------------------------------
void DecOp::print( Stream &stream )
{
   Var *var = dynamic_cast<Var*>( mInput[0] );

   WRITESTR( (char*)var->type );
   WRITESTR( " " );

   mInput[0]->print( stream );
}

//**************************************************************************
// Echo operation - deletes incoming statement!
//**************************************************************************
EchoOp::EchoOp( const char * statement ) : Parent( NULL, NULL )
{
   mStatement = statement;
}

//--------------------------------------------------------------------------
// Destructor
//--------------------------------------------------------------------------
EchoOp::~EchoOp()
{
   delete [] mStatement;
}

//--------------------------------------------------------------------------
// Print
//--------------------------------------------------------------------------
void EchoOp::print( Stream &stream )
{
   WRITESTR( mStatement );
}


//**************************************************************************
// General operation
//**************************************************************************
GenOp::GenOp( const char * statement, ... ) : Parent( NULL, NULL )
{
   VECTOR_SET_ASSOCIATION( mElemList );

   va_list args;
   va_start(args, statement);

   char* lastEntry = (char*)statement;

   while( 1 )
   {
      // search 'statement' for @ symbol
      char * str = dStrstr( lastEntry, (char *)"@" );

      if( !str )
      {
         // not found, handle end of line
         str = (char*)&statement[ dStrlen( (char*)statement ) ];

         U32 diff = str - lastEntry + 1;
         if( diff == 1 ) break;

         char * newStr = new char[diff];

         dMemcpy( (void*)newStr, lastEntry, diff );

         mElemList.push_back( new EchoOp( newStr ) );

         break;
      }

      // create and store statement fragment
      U32 diff = str - lastEntry + 1;

      if( diff == 1 )
      {
         // store langElement
         LangElement *elem = va_arg(args, LangElement* );
         AssertFatal( elem, "NULL arguement." );
         mElemList.push_back( elem );
         lastEntry++;
         continue;
      }

      char * newStr = new char[diff];

      dMemcpy( (void*)newStr, lastEntry, diff );
      newStr[diff-1] = '\0';

      lastEntry = str + 1;

      mElemList.push_back( new EchoOp( newStr ) );

      // store langElement
      LangElement *elem = va_arg(args, LangElement* );
      AssertFatal( elem, "NULL argument." );
      mElemList.push_back( elem );
   }

   va_end( args );
}

//--------------------------------------------------------------------------
// Print
//--------------------------------------------------------------------------
void GenOp::print( Stream &stream )
{
   for( U32 i=0; i<mElemList.size(); i++ )
   {
      mElemList[i]->print( stream );
   }
}
