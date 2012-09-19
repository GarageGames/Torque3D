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
#ifndef _SHADEROP_H_
#define _SHADEROP_H_

#ifndef _LANG_ELEMENT_H_
#include "shaderGen/langElement.h"
#endif

//**************************************************************************
/*!
   This file contains "shader operation" classes.  Originally they were
   to represent basic language operations like adding, assignment, etc.
   That proved to be far too verbose when implementing shader features,
   so they became more generalized helper classes.  Along with LangElement
   classes, they are the building blocks for the procedurally generated
   shaders.
   
   Each shader is a linked list of LangElements.  The list is generated
   when the features of the shader are processed.  When all the features
   are processed, then ShaderGen prints them out by traversing the linked
   list of LangElement and calling their print() function.

   The ShaderOp classes are just extensions of LangElement.

*/
//**************************************************************************



///**************************************************************************
/// Shader operation base class
///**************************************************************************
class ShaderOp : public LangElement
{
protected:
   LangElement * mInput[2];

public:
   ShaderOp( LangElement *in1, LangElement *in2 );
};

//----------------------------------------------------------------------------
/*!
   DecOp - Declaration Operation - Used when declaring a variable in a shader
   feature.  It will automatically print the type of the variable and then
   the variable name.  If a shader feature set up code like:

   @code

   Var *foo = new Var;
   foo->setType( "float" );
   foo->setName( "foo" );
   LangElement *fooDecl = new DecOp( foo );

   LangElement *statement = new GenOp( "   @ = 8.0 * 5.0;", fooDecl );

   @endcode

   The output in the shader file would be:
   
   @code
   float foo = 8.0 * 5.0;
   @endcode
*/
//----------------------------------------------------------------------------
class DecOp : public ShaderOp
{
   typedef ShaderOp Parent;

public:
   DecOp( Var *in1 );
   virtual void print( Stream &stream );
};


//----------------------------------------------------------------------------
/*!
   Like the name suggests, EchoOp merely echoes back whatever string it is
   assigned.
*/
//----------------------------------------------------------------------------
class EchoOp : public ShaderOp
{
   typedef ShaderOp Parent;
   const char * mStatement;

public:
   EchoOp( const char * statement );
   ~EchoOp();
   virtual void print( Stream &stream );
};


//----------------------------------------------------------------------------
/*!
   GenOp - General Operation - Very useful for combining several variables
   into one LangElement statement.  It uses an elipses as a parameter, so it can
   take as many variables as you can throw at it.  It takes a string and parses
   it for the '@' symbol which it replaces with passed in parameters.  Similar
   to the C statement printf().  Here's an example:
   
   @code
   ( assuming three variables var1, var2, var3 exist and their assigned names
     are var1Name, var2Name, and var3Name )
   
   LangElement *statement = new GenOp( "  @ = @ * @.x + @.y;", var1, var1, var2, var3 );
   
   @endcode

   The output in the shader file would be:
   
   @code
   
   var1Name = var1Name * var2Name.x + var3Name.y;
   
   @endcode
*/
//----------------------------------------------------------------------------
class GenOp : public ShaderOp
{
   typedef ShaderOp Parent;

   Vector<LangElement *> mElemList;

public:
   GenOp( const char * statement, ... );
   virtual void print( Stream &stream );

};

#endif // _SHADEROP_H_
