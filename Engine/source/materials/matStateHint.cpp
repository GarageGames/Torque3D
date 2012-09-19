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
#include "materials/matStateHint.h"

#include "materials/processedMaterial.h"


const MatStateHint MatStateHint::Default( "Default" );

void MatStateHint::init( const ProcessedMaterial *mat )
{
   PROFILE_SCOPE( MatStateHint_init );

   mState.clear();
   
   // Write the material identifier so that we batch by material
   // specific data like diffuse color and parallax scale.
   //
   // NOTE: This doesn't actually cause more draw calls, but it 
   // can cause some extra unnessasary material setup when materials
   // are different by their properties are the same.
   //
   const Material *material = mat->getMaterial();
   mState += String::ToString( "Material: '%s', %d\n", material->getName(), material->getId() );
   
   // Go thru each pass and write its state into
   // the string in the most compact but uniquely
   // identifiable way.
   U32 passes = mat->getNumPasses();
   for ( U32 i=0; i < passes; i++ )
      mState += mat->getPass( i )->describeSelf();

   // Finally intern the state string for
   // fast pointer comparisions.
   mState = mState.intern();
}




