//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
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

#ifndef _TAML_CHILDREN_H_
#define _TAML_CHILDREN_H_

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif

//-----------------------------------------------------------------------------

class SimObject;

//-----------------------------------------------------------------------------

class TamlChildren
{
public:
    /// Called when Taml attempts to compile a list of children.
    virtual U32 getTamlChildCount( void ) const = 0;

    /// Called when Taml attempts to compile a list of children.
    virtual SimObject* getTamlChild( const U32 childIndex ) const = 0;

    /// Called when Taml attempts to populate an objects children during a read.
    virtual void addTamlChild( SimObject* pSimObject ) = 0;
};

#endif // _TAML_CHILDREN_H_