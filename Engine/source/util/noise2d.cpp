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

#include "util/noise2d.h"
#include "core/util/tVector.h"

//--------------------------------------
Noise2D::Noise2D()
{
   mSeed   = 0;
}

Noise2D::~Noise2D()
{
}


//--------------------------------------
void Noise2D::normalize(F32 v[2])
{
	F32 s;

	s = mSqrt(v[0] * v[0] + v[1] * v[1]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
}


//--------------------------------------
void Noise2D::setSeed(U32 seed)
{
   if (mSeed == seed)
      return;
   mSeed = seed;
   mRandom.setSeed(mSeed);

	S32 i, j, k;

	for (i = 0 ; i < SIZE ; i++) {
		mPermutation[i] = i;

		for (j = 0 ; j < 2 ; j++)
			mGradient[i][j] = mRandom.randF( -1.0f, 1.0f );
		normalize(mGradient[i]);
	}

	while (--i) {
		k = mPermutation[i];
      j = mRandom.randI(0, SIZE-1);
		mPermutation[i] = mPermutation[j];
		mPermutation[j] = k;
	}

   // extend the size of the arrays x2 to get rid of a bunch of MODs
   // we'd have to do later in the code
	for (i = 0 ; i < SIZE + 2 ; i++) {
		mPermutation[SIZE + i] = mPermutation[i];
		for (j = 0 ; j < 2 ; j++)
			mGradient[SIZE + i][j] = mGradient[i][j];
	}
}


//--------------------------------------
U32 Noise2D::getSeed()
{
   return mSeed;
}


inline F32 Noise2D::lerp(F32 t, F32 a, F32 b)
{
   return a + t * (b - a);
}


inline F32 Noise2D::curve(F32 t)
{
   return t * t * (3.0f - 2.0f * t);
}


inline F32 clamp(F32 f, F32 m)
{
   while (f > m)
      f -= m;
   while (f < 0.0f)
      f += m;
   return f;
}


//--------------------------------------
void Noise2D::fBm( Vector<F32> *dst, U32 size, U32 interval, F32 h, F32 octaves )
{
   interval = getMin(U32(128), getMax(U32(1), interval));
   F32 H = getMin(1.0f, getMax(0.0f, h));
   octaves = getMin(5.0f, getMax(1.0f, octaves));
   F32 lacunarity = 2.0f;

   F32 exponent_array[32];

   U32 shift = getBinLog2( size );

   // precompute and store spectral weights
   // seize required memory for exponent_array
   F32 frequency = 1.0;
   for (U32 i=0; i<=octaves; i++)
   {
      // compute weight for each frequency
      exponent_array[i] = mPow( frequency, -H );
      frequency *= lacunarity;
   }

   // initialize dst
   for (S32 k=0; k < (size*size); k++)
      (*dst)[k] = 0.0f;

   F32 scale = 1.0f / (F32)size * interval;
   for (S32 o=0; o<octaves; o++)
   {
      F32 exp = exponent_array[o];
      for (S32 y=0; y<size; y++)
      {
         F32 fy = (F32)y * scale;
         for (S32 x=0; x<size; x++)
         {
            F32 fx = (F32)x * scale;
            F32 noise = getValue(fx, fy, interval);
            (*dst)[x + (y << shift)] += noise * exp;
         }
      }
      scale    *= lacunarity;
      interval  = (U32)(interval * lacunarity);
   }
}


//--------------------------------------
void Noise2D::rigidMultiFractal(Vector<F32> *dst, Vector<F32> *sig, U32 size, U32 interval, F32 h, F32 octaves)
{
   interval = getMin(U32(128), getMax(U32(1), interval));
   F32 H = getMin(1.0f, getMax(0.0f, h));
   octaves = getMin(5.0f, getMax(1.0f, octaves));
   F32 lacunarity = 2.0f;
   F32 offset     = 1.0f;
   F32 gain       = 2.0f;

   U32 shift = getBinLog2( size );

   F32 exponent_array[32];

   // precompute and store spectral weights
   // seize required memory for exponent_array
   F32 frequency = 1.0;
   for (U32 i=0; i<=octaves; i++)
   {
      // compute weight for each frequency
      exponent_array[i] = mPow( frequency, -H );
      frequency *= lacunarity;
   }

   F32 scale = 1.0f / (F32)size * interval;

   //--------------------------------------
   // compute first octave
   for (S32 y=0; y<size; y++)
   {
      F32 fy = (F32)y * scale;
      for (S32 x=0; x<size; x++)
      {
         F32 fx = (F32)x * scale;

         F32 signal = mFabs(getValue(fx,fy,interval));   // get absolute value of signal (this creates the ridges)
         //signal = mSqrt(signal);
         signal = offset - signal;  // invert and translate (note that "offset" should be ~= 1.0)
         signal *= signal + 0.1;          // square the signal, to increase "sharpness" of ridges

         // assign initial values
         (*dst)[x + (y << shift)] = signal;
         (*sig)[x + (y << shift)] = signal;
      }
   }

   //--------------------------------------
   // compute remaining octaves
   for (S32 o=1; o<octaves; o++)
   {
      // increase the frequency
      scale    *= lacunarity;
      interval  = (U32)(interval * lacunarity);
      F32 exp   = exponent_array[o];

      for (S32 y=0; y<size; y++)
      {
         F32 fy = (F32)y * scale;
         for (S32 x=0; x<size; x++)
         {
            F32 fx = (F32)x * scale;
            U32 index  = x + (y << shift);
            F32 result = (*dst)[index];
            F32 signal = (*sig)[index];

            // weight successive contributions by previous signal
            F32 weight = mClampF(signal * gain, 0.0f, 1.0f);

            signal = mFabs(getValue( fx, fy, interval ));

            signal = offset - signal;
            signal *= signal + 0.2;
            // weight the contribution
            signal *= weight;
            result += signal * exp;

            (*dst)[index] = result;
            (*sig)[index] = signal;
         }
      }
   }
   
   for (S32 k=0; k < (size*size); k++)
      (*dst)[k] = ((*dst)[k]-1.0f)/2.0f;
}

bool Noise2D::erodeHydraulic( Vector<F32> *src, Vector<F32> *dst, U32 iterations, U32 size )
{
   // early out if there is nothing to do
   if (iterations == 0 )
   {
      *dst = *src;
      return true;
   }

   F32 fmin, fmax;
   getMinMax( src, &fmin, &fmax, size);

   U32 shift = getBinLog2( size );
   U32 mask = size - 1;


// currently using SCRATCH_3 for debugging -- Rick
   Vector<F32> scratch = *src;
   U32 *o = (U32*)scratch.address();
   Vector<F32> a = *src;
   Vector<F32> b = *src;
   Vector<F32> c = *src;

   for (S32 k=0; k < (size*size); k++)
      c[k] = 0.0f;

   for (S32 i=0; i<iterations; i++)
   {
      b = a;

      for (S32 y=0; y<size; y++)
      {
         for (S32 x=0; x<size; x++)
         {
            U32 srcOffset = (x + (y << shift));
            F32 height    = a[srcOffset];
            o[srcOffset]  = srcOffset;
            for (S32 y1=y-1; y1 <= y+1; y1++)
            {
               F32 maxDelta = 0.0f;
               S32 ywrap = (y1 & mask);
               for (S32 x1=x-1; x1 <= x+1; x1++)
               {
                  if (x1 != x && y1 != y)
                  {
                     U32 adjOffset  = ((x1 & mask) + (ywrap << shift));
                     F32 &adjHeight = a[adjOffset];
                     F32 delta   = height - adjHeight;
                     if (x1 != x || y1 != y)
                        delta *= 1.414213562f;    // compensate for diagonals
                     if (delta > maxDelta)
                     {
                        maxDelta = delta;
                        o[srcOffset] = adjOffset;
                     }
                  }
               }
            }
         }
      }
      for (S32 j=0; j < (size*size); j++)
      {
         F32 &s = a[j];
         F32 &d = b[ o[j] ];
         F32 delta = s - d;
         if (delta > 0.0f)
         {
            F32 alt = (s-fmin) / (fmax-fmin);
            F32 amt = delta * (0.1f * (1.0f-alt));
            s -= amt;
            d += amt;
         }
      }
// debug only
      for (S32 k=0; k < (size*size); k++)
         c[k] += b[k] - a[k];

      Vector<F32> tmp = a;
      a = b;
      b = tmp;
   }
   *dst = b;
   //*dst = *c;

   return true;
}



bool Noise2D::erodeThermal(Vector<F32> *src, Vector<F32> *dst, F32 slope, F32 materialLoss, U32 iterations, U32 size, U32 squareSize, F32 maxHeight )
{
   // early out if there is nothing to do
   if (iterations == 0 )
   {
      *dst = *src;
      return true;
   }


   F32 fmin, fmax;
   getMinMax(src, &fmin, &fmax, size);

   Vector<F32> a = *src;
   // Heightfield *b = getScratch(1);
   Vector<F32> r;
   r.setSize( size * size );
   //dMemset( r.address(), 0, r.memSize() );

   F32 conservation = 1.0f - mClampF(materialLoss, 0.0f,  100.0f)/100.0f;
   slope            = mClampF(conservation, 0.0f, 89.0f);                  // clamp to 0-89 degrees

   F32 talusConst = mTan(mDegToRad(slope)) * squareSize; // in world units
   talusConst = talusConst * (fmax-fmin) / maxHeight;     // scale to current height units
   F32 p = 0.1f;

   U32 mask = size - 1;
   U32 shift = getBinLog2( size );

   for (U32 i=0; i<iterations; i++)
   {
      // clear out the rubble accumulation field
      dMemset( r.address(), 0, r.memSize() );

      for (S32 y=0; y<size; y++)
      {
         for (S32 x=0; x<size; x++)
         {
            F32 *height    = &a[ x + ( y << shift )];
            F32 *dstHeight = &r[ x + ( y << shift )];

            // for each height look at the immediate surrounding heights
            // if any are higher than talusConst erode on me
            for (S32 y1=y-1; y1 <= y+1; y1++)
            {
               S32 ywrap = (y1 & mask);
               for (S32 x1=x-1; x1 <= x+1; x1++)
               {
                  if (x1 != x && y1 != y)
                  {
                     S32 adjOffset = ((x1 & mask) + (ywrap << shift));
                     F32 adjHeight = a[adjOffset];
                     F32 delta     = adjHeight - *height;
                     if (delta > talusConst)
                     {
                        F32 rubble    = p * (delta - talusConst);
                        r[adjOffset] -= rubble;
                        *dstHeight   += rubble * conservation;
                     }
                  }
               }
            }
         }
      }
      for (S32 k=0; k < (size*size); k++)
         a[k] += r[k];
   }
   *dst = a;
   return true;
}

void Noise2D::getMinMax( Vector<F32> *src, F32 *fmin, F32 *fmax, U32 size )
{
   if (!src)
      return;

   F32 *p = (*src).address();
   *fmin = *p;
   *fmax = *p;
   for (S32 i=0; i < (size*size); i++, p++)
   {
      if (*fmin > *p) *fmin = *p;
      if (*fmax < *p) *fmax = *p;
   }
}

//--------------------------------------
F32 Noise2D::turbulence(F32 x, F32 y, F32 freq)
{
	F32 t, x2, y2;

	for ( t = 0.0f ; freq >= 3.0f ; freq /= 2.0f)
	{
		x2 = freq * x;
		y2 = freq * y;
		t += mFabs(getValue(x2, y2, (S32)freq)) / freq;
	}
	return t;
}


//--------------------------------------
inline void Noise2D::setup(F32 t, S32 &b0, S32 &b1, F32 &r0, F32 &r1)
{
   // find the bounding integers of u
	b0 = S32(t) & SIZE_MASK;
	b1 = (b0+1) & SIZE_MASK;

   // seperate the fractional components
	r0 = t - (S32)t;
	r1 = r0 - 1.0f;
}

inline F32 Noise2D::dot(const F32 *q, F32 rx, F32 ry)
{
   return (rx * q[0] + ry * q[1] );
}



//--------------------------------------
F32 Noise2D::getValue(F32 x, F32 y, S32 interval)
{
	S32 bx0, bx1, by0, by1;
	F32 rx0, rx1, ry0, ry1;

	// Imagine having a square of the type
	//  p0---p1    Where p0 = (bx0, by0)   +----> U
	//  |(u,v)|          p1 = (bx1, by0)   |
	//  |     |          p2 = (bx0, by1)   |    Coordinate System
	//  p2---p3          p3 = (bx1, by1)   V
	// The u, v point in 2D texture space is bounded by this rectangle.

	// Goal, determine the scalar at the points p0, p1, p2, p3.
	// Then the scalar of the point (u, v) will be found by linear interpolation.

	// First step:  Get the 2D coordinates of the points p0, p1, p2, p3.
	// We also need vectors pointing from each point in the square above and
	// ending at the (u,v) coordinate located inside the square.
	// The vector (rx0, ry0) goes from P0 to the (u,v) coordinate.
	// The vector (rx1, ry0) goes from P1 to the (u,v) coordinate.
	// The vector (rx0, ry1) goes from P2 to the (u,v) coordinate.
	// The vector (rx1, ry1) goes from P3 to the (u,v) coordinate.

   setup(x, bx0, bx1, rx0, rx1);
   setup(y, by0, by1, ry0, ry1);

   // Make sure the box corners fall within the interval
   // so that the final output will wrap on itself
   bx0 = bx0 % interval;
   bx1 = bx1 % interval;
   by0 = by0 % interval;
   by1 = by1 % interval;

	S32 i = mPermutation[ bx0 ];
	S32 j = mPermutation[ bx1 ];

	S32 b00 = mPermutation[ i + by0 ];
	S32 b10 = mPermutation[ j + by0 ];
	S32 b01 = mPermutation[ i + by1 ];
	S32 b11 = mPermutation[ j + by1 ];

	// Next, calculate the dropoff component about the point p0.
	F32 sx = curve(rx0);
	F32 sy = curve(ry0);

	// Now, for each point in the square shown above, calculate the dot
	// product of the gradiant vector and the vector going from each square
	// corner point to the (u,v) point inside the square.
   F32 u = dot(mGradient[ b00 ], rx0,ry0);
   F32 v = dot(mGradient[ b10 ], rx1,ry0);

	// Interpolation along the X axis.
	F32 a = lerp(sx, u, v);

   u = dot(mGradient[ b01 ], rx0,ry1);
   v = dot(mGradient[ b11 ], rx1,ry1);

	// Interpolation along the Y axis.
	F32 b = lerp(sx, u, v);

	// Final Interpolation
	return lerp(sy, a, b);
}


