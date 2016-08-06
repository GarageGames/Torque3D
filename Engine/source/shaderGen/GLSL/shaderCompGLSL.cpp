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
#include "gfx/gl/gfxGLVertexAttribLocation.h"


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
         newVar->setConnectName( "vPosition" );
         return newVar;
      }

      case RT_NORMAL:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "vNormal" );
         return newVar;
      }

      case RT_BINORMAL:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "vBinormal" );
         return newVar;
      }      

      case RT_COLOR:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "vColor" );
         return newVar;
      }

      case RT_TANGENT:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );         
         newVar->setConnectName( "vTangent" );
         return newVar;
      }

      case RT_TANGENTW:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );         
         newVar->setConnectName( "vTangentW" );
         return newVar;
      }

      case RT_TEXCOORD:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         
         char out[32];
         dSprintf( (char*)out, sizeof(out), "vTexCoord%d", mCurTexElem );
         newVar->setConnectName( out );
         newVar->constNum = mCurTexElem;
         newVar->arraySize = numElements;

         if ( numRegisters != -1 )
            mCurTexElem += numRegisters;
         else
            mCurTexElem += numElements;

         return newVar;
      }

      case RT_BLENDINDICES:
      {
         Var *newVar = new Var;
         newVar->constNum = mCurBlendIndicesElem;
         mElementList.push_back(newVar);
         char out[32];
         const U32 blendIndicesOffset = Torque::GL_VertexAttrib_BlendIndex0 - Torque::GL_VertexAttrib_TexCoord0;
         dSprintf((char*)out, sizeof(out), "vTexCoord%d", blendIndicesOffset + mCurBlendIndicesElem);
         mCurBlendIndicesElem += 1;
         newVar->setConnectName(out);
         return newVar;
      }

      case RT_BLENDWEIGHT:
      {
         Var *newVar = new Var;
         newVar->constNum = mCurBlendWeightsElem;
         mElementList.push_back(newVar);
         char out[32];
         const U32 blendWeightsOffset = Torque::GL_VertexAttrib_BlendWeight0 - Torque::GL_VertexAttrib_TexCoord0;
         dSprintf((char*)out, sizeof(out), "vTexCoord%d", blendWeightsOffset + mCurBlendWeightsElem);
         mCurBlendWeightsElem += 1;
         newVar->setConnectName(out);
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

void AppVertConnectorGLSL::print( Stream &stream, bool isVertexShader )
{
   if(!isVertexShader)
      return;

   U8 output[256];

   // print struct
   dSprintf( (char*)output, sizeof(output), "struct VertexData\r\n" );
   stream.write( dStrlen((char*)output), output );
   dSprintf( (char*)output, sizeof(output), "{\r\n" );
   stream.write( dStrlen((char*)output), output );

   for( U32 i=0; i<mElementList.size(); i++ )
   {
      Var *var = mElementList[i];
      
      if( var->arraySize == 1)
      {         
         dSprintf( (char*)output, sizeof(output), "   %s %s;\r\n", var->type, (char*)var->name );
         stream.write( dStrlen((char*)output), output );
      }
      else
      {
         dSprintf( (char*)output, sizeof(output), "   %s %s[%d];\r\n", var->type, (char*)var->name, var->arraySize );
         stream.write( dStrlen((char*)output), output );
      }
   }

   dSprintf( (char*)output, sizeof(output), "} IN;\r\n\r\n" );
   stream.write( dStrlen((char*)output), output );   

   // print in elements
   for( U32 i=0; i<mElementList.size(); i++ )
   {
      Var *var = mElementList[i];
      
      for(int j = 0; j < var->arraySize; ++j)
      {        
         const char *name = j == 0 ? var->connectName : avar("vTexCoord%d", var->constNum + j) ;
         dSprintf( (char*)output, sizeof(output), "in %s %s;\r\n", var->type, name );
         stream.write( dStrlen((char*)output), output );         
      }

      dSprintf( (char*)output, sizeof(output), "#define IN_%s IN.%s\r\n", var->name, var->name ); // TODO REMOVE
      stream.write( dStrlen((char*)output), output );
   }
   const char* newLine ="\r\n";
   stream.write( dStrlen((char*)newLine), newLine );
}

Var * VertPixelConnectorGLSL::getElement( RegisterType type, 
                                          U32 numElements, 
                                          U32 numRegisters )
{
   switch( type )
   {
   case RT_POSITION:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "POSITION" );
         return newVar;
      }

   case RT_NORMAL:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "NORMAL" );
         return newVar;
      }

   case RT_COLOR:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "COLOR" );
         return newVar;
      }

   /*case RT_BINORMAL:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "BINORMAL" );
         return newVar;
      }

   case RT_TANGENT:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "TANGENT" );
         return newVar;
      }   */

   case RT_TEXCOORD:
   case RT_BINORMAL:
   case RT_TANGENT:
      {
         Var *newVar = new Var;
         newVar->arraySize = numElements;

         char out[32];
         dSprintf( (char*)out, sizeof(out), "TEXCOORD%d", mCurTexElem );
         newVar->setConnectName( out );

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

void VertPixelConnectorGLSL::print( Stream &stream, bool isVerterShader )
{
   // print out elements
   for( U32 i=0; i<mElementList.size(); i++ )
   {
      U8 output[256];

      Var *var = mElementList[i];
      if(!dStrcmp((const char*)var->name, "gl_Position"))
         continue;

      if(var->arraySize <= 1)
         dSprintf((char*)output, sizeof(output), "%s %s _%s_;\r\n", (isVerterShader ? "out" : "in"), var->type, var->connectName);
      else
         dSprintf((char*)output, sizeof(output), "%s %s _%s_[%d];\r\n", (isVerterShader ? "out" : "in"),var->type, var->connectName, var->arraySize);      

      stream.write( dStrlen((char*)output), output );
   }

   printStructDefines(stream, !isVerterShader);
}

void VertPixelConnectorGLSL::printOnMain( Stream &stream, bool isVerterShader )
{
   if(isVerterShader)
      return;

   const char *newLine = "\r\n";
   const char *header = "   //-------------------------\r\n";
   stream.write( dStrlen((char*)newLine), newLine );
   stream.write( dStrlen((char*)header), header );

   // print out elements
   for( U32 i=0; i<mElementList.size(); i++ )
   {
      U8 output[256];

      Var *var = mElementList[i];
      if(!dStrcmp((const char*)var->name, "gl_Position"))
         continue;
  
      dSprintf((char*)output, sizeof(output), "   %s IN_%s = _%s_;\r\n", var->type, var->name, var->connectName);      

      stream.write( dStrlen((char*)output), output );
   }

   stream.write( dStrlen((char*)header), header );
   stream.write( dStrlen((char*)newLine), newLine );
}


void AppVertConnectorGLSL::printOnMain( Stream &stream, bool isVerterShader )
{
   if(!isVerterShader)
      return;   

   const char *newLine = "\r\n";
   const char *header = "   //-------------------------\r\n";
   stream.write( dStrlen((char*)newLine), newLine );
   stream.write( dStrlen((char*)header), header );

   // print out elements
   for( U32 i=0; i<mElementList.size(); i++ )
   {
      Var *var = mElementList[i];
      U8 output[256];  

      if(var->arraySize <= 1)
      {
         dSprintf((char*)output, sizeof(output), "   IN.%s = %s;\r\n", var->name, var->connectName);
         stream.write( dStrlen((char*)output), output );
      }
      else
      {
         for(int j = 0; j < var->arraySize; ++j)
         {
            const char *name = j == 0 ? var->connectName : avar("vTexCoord%d", var->constNum + j) ;
            dSprintf((char*)output, sizeof(output), "   IN.%s[%d] = %s;\r\n", var->name, j, name );
            stream.write( dStrlen((char*)output), output );
         }
      }
   }

   stream.write( dStrlen((char*)header), header );
   stream.write( dStrlen((char*)newLine), newLine );
}




Vector<String> initDeprecadedDefines()
{
   Vector<String> vec;
   vec.push_back( "isBack"); 
   return vec;
}

void VertPixelConnectorGLSL::printStructDefines( Stream &stream, bool in )
{
   const char* connectionDir = in ? "IN" : "OUT";

   static Vector<String> deprecatedDefines = initDeprecadedDefines();

   const char *newLine = "\r\n";
   const char *header = "// Struct defines\r\n";
   stream.write( dStrlen((char*)newLine), newLine );
   stream.write( dStrlen((char*)header), header );

   // print out elements
   for( U32 i=0; i<mElementList.size(); i++ )
   {
      U8 output[256];

      Var *var = mElementList[i];
      if(!dStrcmp((const char*)var->name, "gl_Position"))
         continue;      
  
      if(!in)
      {
         dSprintf((char*)output, sizeof(output), "#define %s_%s _%s_\r\n", connectionDir, var->name, var->connectName);
         stream.write( dStrlen((char*)output), output );
         continue;
      }
   }

   stream.write( dStrlen((char*)newLine), newLine );
}

void VertexParamsDefGLSL::print( Stream &stream, bool isVerterShader )
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

void PixelParamsDefGLSL::print( Stream &stream, bool isVerterShader )
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

   for( U32 i=0; i<LangElement::elementList.size(); i++)
   {
      Var *var = dynamic_cast<Var*>(LangElement::elementList[i]);
      if( var )
      {
         if( var->uniform && !var->sampler)
         {
            U8 output[256];
            if(var->arraySize <= 1)
               dSprintf((char*)output, sizeof(output), "   %s %s = %s;\r\n", var->type, var->name, var->name);
            else
               dSprintf((char*)output, sizeof(output), "   %s %s[%d] = %s;\r\n", var->type, var->name, var->arraySize, var->name);

            stream.write( dStrlen((char*)output), output );
         }
      }
   }
}
