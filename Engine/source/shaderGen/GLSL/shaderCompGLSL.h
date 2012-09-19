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

#ifndef _SHADERCOMP_GLSL_H_
#define _SHADERCOMP_GLSL_H_

#ifndef _SHADERCOMP_H_
#include "shaderGen/shaderComp.h"
#endif


class VertPixelConnectorGLSL : public ShaderConnector
{
public:

   // ShaderConnector
   virtual Var* getElement(   RegisterType type, 
                              U32 numElements = 1, 
                              U32 numRegisters = -1 );
   virtual void setName( char *newName );
   virtual void reset();
   virtual void sortVars();

   virtual void print( Stream &stream );
};

class AppVertConnectorGLSL : public ShaderConnector
{
public:
   virtual Var* getElement(   RegisterType type, 
                              U32 numElements = 1, 
                              U32 numRegisters = -1 );
   virtual void setName( char *newName );
   virtual void reset();
   virtual void sortVars();
   
   virtual void print( Stream &stream );
};


class VertexParamsDefGLSL : public ParamsDef
{
public:
   virtual void print( Stream &stream );
};


class PixelParamsDefGLSL : public ParamsDef
{
public:
   virtual void print( Stream &stream );
};

#endif // _SHADERCOMP_GLSL_H_