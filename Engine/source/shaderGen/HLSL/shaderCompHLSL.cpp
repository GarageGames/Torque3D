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
#include "shaderGen/HLSL/shaderCompHLSL.h"

#include "shaderGen/shaderComp.h"
#include "shaderGen/langElement.h"
#include "gfx/gfxDevice.h"


Var * ShaderConnectorHLSL::getElement( RegisterType type, 
                                       U32 numElements, 
                                       U32 numRegisters )
{
   Var *ret = getIndexedElement( mCurTexElem, type, numElements, numRegisters );

   // Adjust texture offset if this is a texcoord type
   if( type == RT_TEXCOORD )
   {
      if ( numRegisters != -1 )
         mCurTexElem += numRegisters;
      else
         mCurTexElem += numElements;
   }

   return ret;
}

Var * ShaderConnectorHLSL::getIndexedElement( U32 index, RegisterType type, U32 numElements /*= 1*/, U32 numRegisters /*= -1 */ )
{
   switch( type )
   { 
   case RT_POSITION:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "POSITION" );
         newVar->rank = 0;
         return newVar;
      }

   case RT_VPOS:
      {
         Var *newVar = new Var;
         mElementList.push_back(newVar);
         newVar->setConnectName("VPOS");
         newVar->rank = 0;
         return newVar;
      }

   case RT_SVPOSITION:
      {
         Var *newVar = new Var;
         mElementList.push_back(newVar);
         newVar->setConnectName("SV_Position");
         newVar->rank = 0;
         return newVar;
      }

   case RT_NORMAL:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "NORMAL" );
         newVar->rank = 1;
         return newVar;
      }

   case RT_BINORMAL:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "BINORMAL" );
         newVar->rank = 2;
         return newVar;
      }

   case RT_TANGENT:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "TANGENT" );
         newVar->rank = 3;
         return newVar;
      }

   case RT_COLOR:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );
         newVar->setConnectName( "COLOR" );
         newVar->rank = 4;
         return newVar;
      }

   case RT_TEXCOORD:
      {
         Var *newVar = new Var;
         mElementList.push_back( newVar );

         // This was needed for hardware instancing, but
         // i don't really remember why right now.
         if ( index > mCurTexElem )
            mCurTexElem = index + 1;

         char out[32];
         dSprintf( (char*)out, sizeof(out), "TEXCOORD%d", index );
         newVar->setConnectName( out );
         newVar->constNum = index;
         newVar->arraySize = numElements;
         newVar->rank = 5 + index;

         return newVar;
      }

   default:
      break;
   }

   return NULL;
}



S32 QSORT_CALLBACK ShaderConnectorHLSL::_hlsl4VarSort(const void* e1, const void* e2)
{
   Var* a = *((Var **)e1);
   Var* b = *((Var **)e2);

   return a->rank - b->rank;
}

void ShaderConnectorHLSL::sortVars()
{

   // If shader model 4+ than we gotta sort the vars to make sure the order is consistent
   if (GFX->getPixelShaderVersion() >= 4.f)
   {
      dQsort((void *)&mElementList[0], mElementList.size(), sizeof(Var *), _hlsl4VarSort);
      return;
   }

   return;
}

void ShaderConnectorHLSL::setName( char *newName )
{
   dStrcpy( (char*)mName, newName );
}

void ShaderConnectorHLSL::reset()
{
   for( U32 i=0; i<mElementList.size(); i++ )
   {
      mElementList[i] = NULL;
   }

   mElementList.setSize( 0 );
   mCurTexElem = 0;
}

void ShaderConnectorHLSL::print( Stream &stream, bool isVertexShader )
{
   const char * header = "struct ";
   const char * header2 = "\r\n{\r\n";
   const char * footer = "};\r\n\r\n\r\n";

   stream.write( dStrlen(header), header );
   stream.write( dStrlen((char*)mName), mName );
   stream.write( dStrlen(header2), header2 );


   // print out elements
   for( U32 i=0; i<mElementList.size(); i++ )
   {
      U8 output[256];

      Var *var = mElementList[i];
      if (var->arraySize <= 1)
         dSprintf( (char*)output, sizeof(output), "   %s %-15s : %s;\r\n", var->type, var->name, var->connectName );
      else
         dSprintf( (char*)output, sizeof(output), "   %s %s[%d] : %s;\r\n", var->type, var->name, var->arraySize, var->connectName );

      stream.write( dStrlen((char*)output), output );
   }

   stream.write( dStrlen(footer), footer );
}

void ParamsDefHLSL::assignConstantNumbers()
{

   // Here we assign constant number to uniform vars, sorted 
   // by their update frequency.

   U32 mCurrConst = 0;
   for (U32 bin = cspUninit+1; bin < csp_Count; bin++)
   {   
      // Find all the uniform variables that are part of this group and assign constant numbers
      for( U32 i=0; i<LangElement::elementList.size(); i++)
      {
         Var *var = dynamic_cast<Var*>(LangElement::elementList[i]);
         if( var )
         {            
            bool shaderConst = var->uniform && !var->sampler && !var->texture;
            AssertFatal((!shaderConst) || var->constSortPos != cspUninit, "Const sort position has not been set, variable will not receive a constant number!!");
            if( shaderConst && var->constSortPos == bin)
            {
               var->constNum = mCurrConst;
               // Increment our constant number based on the variable type
               if (dStrcmp((const char*)var->type, "float4x4") == 0)
               {
                  mCurrConst += (4 * var->arraySize);
               } else {
                  if (dStrcmp((const char*)var->type, "float3x3") == 0)
                  {
                     mCurrConst += (3 * var->arraySize);
                  } else {
                     mCurrConst += var->arraySize;
                  }
               }
            }
         }
      }
   }
}

void VertexParamsDefHLSL::print( Stream &stream, bool isVerterShader )
{
   assignConstantNumbers();

   const char *opener = "ConnectData main( VertData IN";
   stream.write( dStrlen(opener), opener );

   // find all the uniform variables and print them out
   for( U32 i=0; i<LangElement::elementList.size(); i++)
   {
      Var *var = dynamic_cast<Var*>(LangElement::elementList[i]);
      if( var )
      {
         if( var->uniform )
         {
            const char* nextVar = ",\r\n                  ";
            stream.write( dStrlen(nextVar), nextVar );            

            U8 varNum[64];
            dSprintf( (char*)varNum, sizeof(varNum), "register(C%d)", var->constNum );

            U8 output[256];
            if (var->arraySize <= 1)
               dSprintf( (char*)output, sizeof(output), "uniform %-8s %-15s : %s", var->type, var->name, varNum );
            else
               dSprintf( (char*)output, sizeof(output), "uniform %-8s %s[%d] : %s", var->type, var->name, var->arraySize, varNum );

            stream.write( dStrlen((char*)output), output );
         }
      }
   }

   const char *closer = "\r\n)\r\n{\r\n   ConnectData OUT;\r\n\r\n";
   stream.write( dStrlen(closer), closer );
}

void PixelParamsDefHLSL::print( Stream &stream, bool isVerterShader )
{
   assignConstantNumbers();

   const char * opener = "Fragout main( ConnectData IN";
   stream.write( dStrlen(opener), opener );

   // find all the sampler & uniform variables and print them out
   for( U32 i=0; i<LangElement::elementList.size(); i++)
   {
      Var *var = dynamic_cast<Var*>(LangElement::elementList[i]);
      if( var )
      {
         if( var->uniform )
         {
            WRITESTR( ",\r\n              " );

            U8 varNum[32];

            if( var->sampler )
            {
               dSprintf( (char*)varNum, sizeof(varNum), ": register(S%d)", var->constNum );
            }
            else if (var->texture)
            {
               dSprintf((char*)varNum, sizeof(varNum), ": register(T%d)", var->constNum);
            }
            else
            {
               dSprintf( (char*)varNum, sizeof(varNum), ": register(C%d)", var->constNum );
            }

            U8 output[256];
            if (var->arraySize <= 1)
               dSprintf( (char*)output, sizeof(output), "uniform %-9s %-15s %s", var->type, var->name, varNum );
            else
               dSprintf( (char*)output, sizeof(output), "uniform %-9s %s[%d] %s", var->type, var->name, var->arraySize, varNum );

            WRITESTR( (char*) output );
         }
      }
   }

   const char *closer = "\r\n)\r\n{\r\n   Fragout OUT;\r\n\r\n";
   stream.write( dStrlen(closer), closer );
}
