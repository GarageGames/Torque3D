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
#ifndef _LANG_ELEMENT_H_
#define _LANG_ELEMENT_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _STREAM_H_
#include "core/stream/stream.h"
#endif

#define WRITESTR( a ){ stream.write( dStrlen(a), a ); }


//**************************************************************************
/*!
   The LangElement class is the base building block for procedurally
   generated shader code.  LangElement and its subclasses are strung
   together using the static Vector 'elementList'.
   When a shader needs to be written to disk, the elementList is
   traversed and print() is called on each LangElement and the shader
   is output.  elementList is cleared after each shader is printed out.
*/
//**************************************************************************


//**************************************************************************
// Language element
//**************************************************************************
struct LangElement
{
   static Vector<LangElement*> elementList;
   static LangElement * find( const char *name );
   static void deleteElements();
      
   U8    name[32];
   
   LangElement();
   virtual ~LangElement() {};
   virtual void print( Stream &stream ){};
   void setName(const char *newName );

};

enum ConstantSortPosition
{
   /// Default / unset
   cspUninit = 0,      
   /// Updated before every draw primitive call.
   cspPrimitive,
   /// Potentially updated every draw primitive call, but not necessarily (lights for example)
   cspPotentialPrimitive,
   /// Updated one per pass
   cspPass,
   /// Count var, do not use
   csp_Count
};

//----------------------------------------------------------------------------
/*!
   Var - Variable - used to specify a variable to be used in a shader.
   Var stores information such  that when it is printed out, its context 
   can be identified and the proper information will automatically be printed.
   For instance, if a variable is created with 'uniform' set to true, when the 
   shader function definition is printed, it will automatically add that 
   variable to the incoming parameters of the shader.  There are several
   similar cases such as when a new variable is declared within a shader.
   
   example:
   
   @code   

   Var *modelview = new Var;
   modelview->setType( "float4x4" );
   modelview->setName( "modelview" );
   modelview->uniform = true;
   modelview->constSortPos = cspPass;

   @endcode
   
   it prints out in the shader declaration as:
   
   @code
      ConnectData main( VertData IN,
                        uniform float4x4 modelview : register(C0) )
   @endcode

*/
//----------------------------------------------------------------------------
struct Var : public LangElement
{
   U8    type[32];
   U8    structName[32];
   char  connectName[32];
   ConstantSortPosition constSortPos; // used to calculate constant number 
   U32   constNum;
   U32   texCoordNum;
   bool  uniform;       // argument passed in through constant registers
   bool  vertData;      // argument coming from vertex data
   bool  connector;     // variable that will be passed to pixel shader
   bool  sampler;       // texture
   bool  mapsToSampler; // for ps 1.x shaders - texcoords must be mapped to same sampler stage
   U32   arraySize;     // 1 = no array, > 1 array of "type"

   static U32  texUnitCount;
   static U32  getTexUnitNum(U32 numElements = 1);
   static void reset();

   // Default
   Var();   
   Var( const char *name, const char *type );   
   
   void setStructName(const char *newName );
   void setConnectName(const char *newName );
   void setType(const char *newType );
  
   virtual void print( Stream &stream );

   // Construct a uniform / shader const var
   void setUniform(const String& constType, const String& constName, ConstantSortPosition sortPos);
};

//----------------------------------------------------------------------------
/*!
   MultiLine - Multi Line Statement - This class simply ties multiple
   
   example:
   
   @code   

   MultiLine *meta = new MultiLine;
   meta->addStatement( new GenOp( "foo = true;\r\n" ) );
   meta->addStatement( new GenOp( "bar = false;\r\n ) );

   @endcode
   
   it prints out in the shader declaration as:
   
   @code
      foo = true;
      bar = false;
   @endcode

*/
//----------------------------------------------------------------------------
class MultiLine : public LangElement
{
   Vector <LangElement*> mStatementList;

public:
   MultiLine()
   {
      VECTOR_SET_ASSOCIATION( mStatementList );
   }

   void addStatement( LangElement *elem );
   virtual void print( Stream &stream );
};



#endif // _LANG_ELEMENT_H_
