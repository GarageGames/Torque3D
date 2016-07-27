//-----------------------------------------------------------------------------
// Copyright (c) 2015 GarageGames, LLC
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

#ifndef _TORQUE_SHADERMODEL_AUTOGEN_
#define _TORQUE_SHADERMODEL_AUTOGEN_

#include "shadergen:/autogenConditioners.h"

// Portability helpers for autogenConditioners
#if (TORQUE_SM >= 10 && TORQUE_SM <=30)
   #define TORQUE_PREPASS_UNCONDITION(tex, coords) prepassUncondition(tex, coords)
#elif TORQUE_SM >= 40
   #define TORQUE_PREPASS_UNCONDITION(tex, coords) prepassUncondition(tex, texture_##tex, coords)
#endif

#endif //_TORQUE_SHADERMODEL_AUTOGEN_
