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

#ifndef _INSTANCINGMATHOOK_H_
#define _INSTANCINGMATHOOK_H_

#ifndef _MATINSTANCEHOOK_H_
#include "materials/matInstanceHook.h"
#endif

class BaseMatInstance;


class InstancingMaterialHook : public MatInstanceHook
{
public:

   /// The material hook type.
   static const MatInstanceHookType Type;

   InstancingMaterialHook();
   virtual ~InstancingMaterialHook();

   // MatInstanceHook
   virtual const MatInstanceHookType& getType() const { return Type; }

   /// Returns the instancing material instance or the input material 
   /// instance if one could not be created.
   static BaseMatInstance* getInstancingMat( BaseMatInstance *matInst );

protected:

   /// The instancing material.
   BaseMatInstance *mMatInst;

};

#endif // _INSTANCINGMATHOOK_H_
