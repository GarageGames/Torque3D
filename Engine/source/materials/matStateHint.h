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

#ifndef _MATSTATEHINT_H_
#define _MATSTATEHINT_H_

#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif

class ProcessedMaterial;


/// A simple object for generating and comparing string based
/// hints used for sorting and identifying materials uniquely
/// by its shaders and states.
class MatStateHint
{
public:

   /// Constructor.
   MatStateHint() {}

   /// Constructor for building special hints.
   MatStateHint( const String &state ) 
	   : mState( state.intern() )
   {
   }

   /// Initialize the state hint from a ProcessMaterial.  This
   /// assumes that the ProcessedMaterial has properly initialized
   /// its passes to describe the material uniquely.
   void init( const ProcessedMaterial *mat );

   /// Clears the hint.
   void clear() { mState.clear(); }

   /// Returns a 32bit hash key used for sorting by material state.
   operator U32() const { return mState.getHashCaseSensitive(); }

   /// Fast comparision of state for equality.
   bool operator ==( const MatStateHint& hint ) const { return mState == hint.mState; }

   /// Fast comparision of state for inequality.
   bool operator !=( const MatStateHint& hint ) const { return mState != hint.mState; }

   /// A default state hint.
   static const MatStateHint Default;

protected:

   /// An interned string of the combined material shader and state info
   /// for evert pass of the processed material.
   String mState;
   
};

#endif // _MATSTATEHINT_H_
