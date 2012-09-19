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

#ifndef _SHADER_DEPENDENCY_H_
#define _SHADER_DEPENDENCY_H_

#ifndef _PATH_H_
#include "core/util/path.h"
#endif


class Stream;


/// The base class for shader dependencies
class ShaderDependency
{
public:
   virtual ~ShaderDependency() {}
   
   /// Compare this dependency to another one.
   virtual bool operator==( const ShaderDependency &cmpTo ) const 
   { 
      return this == &cmpTo; 
   }

   /// Print the dependency into the header of a shader.
   virtual void print( Stream &s ) const = 0;
};


/// A shader dependency for adding #include's into shadergen shaders.
class ShaderIncludeDependency : public ShaderDependency
{
protected:

   Torque::Path mIncludePath;

public:

   ShaderIncludeDependency( const Torque::Path &pathToInclude );

   virtual bool operator==( const ShaderDependency &cmpTo ) const;
   virtual void print( Stream &s ) const;
};

#endif // _SHADER_DEPENDENCY_H_