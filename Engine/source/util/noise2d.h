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

#ifndef _NOISE2D_H_
#define _NOISE2D_H_


#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _MRANDOM_H_
#include "math/mRandom.h"
#endif

class Noise2D
{
private:
   enum Constants {
      SIZE      = 0x100,
      SIZE_MASK = 0x0ff
   };
   S32 mPermutation[SIZE + SIZE + 2];
   F32 mGradient[SIZE + SIZE + 2][2];

   U32 mSeed;

   MRandom mRandom;

   F32  lerp(F32 t, F32 a, F32 b);
   F32  curve(F32 t);
   void setup(F32 t, S32 &b0, S32 &b1, F32 &r0, F32 &r1);
   F32  dot(const F32 *q, F32 rx, F32 ry);
   void normalize(F32 v[2]);

public:
   Noise2D();
   ~Noise2D();

   void setSeed(U32 seed);
   U32  getSeed();

   F32  getValue(F32 u, F32 v, S32 interval);

   /// @name Noise
   /// These functions actually generate
   /// noise values into the passed in destination
   /// array.
   ///
   /// Note that the output values of these functions
   /// are from -1.0 to 1.0.
   /// 
   /// fBm - Fractal Brownian Motion - A simple noise generation
   /// algorithm, it tends to generate either flowing rounded
   /// hills or rounded mountainous shapes.
   /// @{
   void fBm( Vector<F32> *dst, U32 size, U32 interval, F32 h, F32 octave=5.0f);
   
   /// rigidMultiFractal
   /// Creates ridged mountains with a high frequency detail.
   void rigidMultiFractal( Vector<F32> *dst, Vector<F32> *signal, U32 size, U32 interval, F32 h, F32 octave=5.0f);
   /// @}
   
   bool erodeHydraulic(Vector<F32> *src, Vector<F32> *dst, U32 iterations, U32 size );
   bool erodeThermal(Vector<F32> *src, Vector<F32> *dst, F32 slope, F32 materialLoss, U32 iterations, U32 size, U32 squareSize, F32 maxHeight );

   F32  turbulence(F32 x, F32 y, F32 freq);

   void getMinMax( Vector<F32> *src, F32 *maxNoise, F32 *minNoise, U32 size );
};



#endif  // _NOISE2D_H_
