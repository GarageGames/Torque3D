
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "afx/arcaneFX.h"
#include "afxEase.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

F32
afxEase::t(F32 t, F32 ein, F32 eout)
{
  if (t == 0.0)
    return 0.0;
  
  if (t == 1.0)
    return 1.0;
  
  F32 ee = eout - ein + 1.0;
  
  // ease in section
  if (t <= ein)
  {
    F32 tin = t/ein;
    return (mSin(M_PI_F*(tin - 1.0)) + M_PI_F*tin)*ein*(1.0/M_PI_F)/ee;
  }

  // middle linear section
  else if (t <= eout)
  {
    return (2.0*t - ein)/ee;
  }

  // ease out section
  else 
  {
    F32 iout = 1.0 - eout;
    F32 g	= (t - eout)*M_PI_F/iout;
    return ((mSin(g) + g)*(iout)/M_PI_F + 2.0*eout - ein)*1.0/ee + 0.0;
  }
}

F32
afxEase::eq(F32 t, F32 a, F32 b, F32 ein, F32 eout)
{
  if (t == 0.0)
    return a;
  
  if (t == 1.0)
    return b;
  
  F32 ab = b - a;
  F32 ee = eout - ein + 1.0;
  
  // ease in section
  if (t <= ein)
  {
    F32 tin = t/ein;
    return a + (mSin(M_PI_F*(tin - 1.0)) + M_PI_F*tin)*ab*ein*(1.0/M_PI_F)/ee;
  }

  // middle linear section
  else if (t <= eout)
  {
    return a + ab*(2.0*t - ein)/ee;
  }

  // ease out section
  else 
  {
    F32 iout = 1.0 - eout;
    F32 g	= (t - eout)*M_PI_F/iout;
    return ((mSin(g) + g)*(iout)/M_PI_F + 2.0*eout - ein)*ab/ee + a;
  }
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
