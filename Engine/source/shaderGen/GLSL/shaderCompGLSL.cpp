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
#include "shaderGen/GLSL/shaderCompGLSL.h"

#include "shaderGen/shaderComp.h"
#include "shaderGen/langElement.h"
#include "gfx/gfxDevice.h"


Var * AppVertConnectorGLSL::getElement(   RegisterType type, 
                                          U32 numElements, 
                                          U32 numRegisters )
{
   switch( type )
   { 
      case RT_POSITION:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "gl_Vertex" );
         return newVar;
      }

      case RT_NORMAL:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "gl_Normal" );
         return newVar;
      }
      

      case RT_COLOR:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "gl_Color" );
         return newVar;
      }

      case RT_TEXCOORD:
      case RT_BINORMAL:
      case RT_TANGENT:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         
         char out[32];
         dSprintf( (char*)out, sizeof(out), "gl_MultiTexCoord%d", mCurTexElem );
         newVar->setConnectName( out );
         newVar->constNum = mCurTexElem;
         newVar->arraySize = numElements;

         if ( numRegisters != -1 )
            mCurTexElem += numRegisters;
         else
            mCurTexElem += numElements;

         return newVar;
      }

      default:
         break;
   }
   
   return NULL;
}

void AppVertConnectorGLSL::sortVars()
{
   // Not required in GLSL
}

void AppVertConnectorGLSL::setName( char *newName )
{
   dStrcpy( (char*)mName, newName );
}

void AppVertConnectorGLSL::reset()
{
   for( U32 i=0; i<mElementList.size(); i++ )
   {
      mElementList[i] = NULL;
   }

   mElementList.setSize( 0 );
   mCurTexElem = 0;
}

void AppVertConnectorGLSL::print( Stream &stream )
{
   // print out elements
   for( U32 i=0; i<mElementList.size(); i++ )
   {
      Var *var = mElementList[i];
      U8 output[256];
      const char* swizzle;
      if(!dStrcmp((const char*)var->type, "float"))
         swizzle = "x";
      else if(!dStrcmp((const char*)var->type, "vec2"))
         swizzle = "xy";
      else if(!dStrcmp((const char*)var->type, "vec3"))
         swizzle = "xyz";
      else
         swizzle = "xyzw";

      // This is ugly.  We use #defines to match user defined names with 
      // built in vars.  There is no cleaner way to do this.
      dSprintf( (char*)output, sizeof(output), "#define %s %s.%s\r\n", var->name, var->connectName, swizzle );

      stream.write( dStrlen((char*)output), output );
   }
}

Var * VertPixelConnectorGLSL::getElement( RegisterType type, 
                                          U32 numElements, 
                                          U32 numRegisters )
{
   switch( type )
   {
   case RT_POSITION:
   case RT_NORMAL:
   case RT_COLOR:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         return newVar;
      }

   case RT_TEXCOORD:
   case RT_BINORMAL:
   case RT_TANGENT:
      {
         Var *newVar = new Var;
         newVar->arraySize = numElements;

         if ( numRegisters != -1 )
            mCurTexElem += numRegisters;
         else
            mCurTexElem += numElements;
         
         mElementList.push_back( newVar );
         return newVar;
      }

   default:
      break;
   }

   return NULL;
}

void VertPixelConnectorGLSL::sortVars()
{
   // Not needed in GLSL
}

void VertPixelConnectorGLSL::setName( char *newName )
{
   dStrcpy( (char*)mName, newName );
}

void VertPixelConnectorGLSL::reset()
{
   for( U32 i=0; i<mElementList.size(); i++ )
   {
      mElementList[i] = NULL;
   }

   mElementList.setSize( 0 );
   mCurTexElem = 0;
}

void VertPixelConnectorGLSL::print( Stream &stream )
{
   // print out elements
   for( U32 i=0; i<mElementList.size(); i++ )
   {
      U8 output[256];

      Var *var = mElementList[i];
      if(!dStrcmp((const char*)var->name, "gl_Position"))
         continue;

      if(var->arraySize <= 1)
         dSprintf((char*)output, sizeof(output), "varying %s %s;\r\n", var->type, var->name);
      else
         dSprintf((char*)output, sizeof(output), "varying %s %s[%d];\r\n", var->type, var->name, var->arraySize);

      stream.write( dStrlen((char*)output), output );
   }
}

void VertexParamsDefGLSL::print( Stream &stream )
{
   // find all the uniform variables and print them out
   for( U32 i=0; i<LangElement::elementList.size(); i++)
   {
      Var *var = dynamic_cast<Var*>(LangElement::elementList[i]);
      if( var )
      {
         if( var->uniform )
         {
            U8 output[256];
            if(var->arraySize <= 1)
               dSprintf((char*)output, sizeof(output), "uniform %-8s %-15s;\r\n", var->type, var->name);
            else
               dSprintf((char*)output, sizeof(output), "uniform %-8s %-15s[%d];\r\n", var->type, var->name, var->arraySize);

            stream.write( dStrlen((char*)output), output );
         }
      }
   }

   const char *closer = "\r\n\r\nvoid main()\r\n{\r\n";
   stream.write( dStrlen(closer), closer );
}

void PixelParamsDefGLSL::print( Stream &stream )
{
   // find all the uniform variables and print them out
   for( U32 i=0; i<LangElement::elementList.size(); i++)
   {
      Var *var = dynamic_cast<Var*>(LangElement::elementList[i]);
      if( var )
      {
         if( var->uniform )
         {
            U8 output[256];
            if(var->arraySize <= 1)
               dSprintf((char*)output, sizeof(output), "uniform %-8s %-15s;\r\n", var->type, var->name);
            else
               dSprintf((char*)output, sizeof(output), "uniform %-8s %-15s[%d];\r\n", var->type, var->name, var->arraySize);

            stream.write( dStrlen((char*)output), output );
         }
      }
   }

   const char *closer = "\r\nvoid main()\r\n{\r\n";
   stream.write( dStrlen(closer), closer );
}
