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

// This file is Mac specific.
#if defined( __APPLE__ )

#include <vecLib/vecLib.h>
#include "math/mMathFn.h"
#include "console/console.h"
#include "platform/profiler.h"

#if defined( __VEC__ )

// tests show BLAS to be about 4x slower than aligned altivec, 3x slower than unaligned altivec code below.

/// Altivec 4x4 Matrix multiplication.
/// Most of our time is spent moving data in & out of the vector registers.
/// Alignment of the matrix data to 16-byte boundaries is very important,
/// because we get a much better speed gain if we can assume the data is aligned.
void vec_MatrixF_x_MatrixF(const F32 *matA, const F32 *matB, F32 *result)
{
   vector float A[4][1];
   vector float B[4][1];
   vector float C[4][1];

   /// If the incoming pointers are not 16-byte aligned, we have to load & store the slow way.
   if((int)matA & 0xF || (int)matB & 0xF || (int)result & 0xF)
   {
      F32 *loader;
      loader = (F32*) &A;
      loader[0] = matA[0];
      loader[1] = matA[1];
      loader[2] = matA[2];
      loader[3] = matA[3];
      loader[4] = matA[4];
      loader[5] = matA[5];
      loader[6] = matA[6];
      loader[7] = matA[7];
      loader[8] = matA[8];
      loader[9] = matA[9];
      loader[10] = matA[10];
      loader[11] = matA[11];
      loader[12] = matA[12];
      loader[13] = matA[13];
      loader[14] = matA[14];
      loader[15] = matA[15];
      loader = (F32*) &B;
      loader[0] = matB[0];
      loader[1] = matB[1];
      loader[2] = matB[2];
      loader[3] = matB[3];
      loader[4] = matB[4];
      loader[5] = matB[5];
      loader[6] = matB[6];
      loader[7] = matB[7];
      loader[8] = matB[8];
      loader[9] = matB[9];
      loader[10] = matB[10];
      loader[11] = matB[11];
      loader[12] = matB[12];
      loader[13] = matB[13];
      loader[14] = matB[14];
      loader[15] = matB[15];
      
      vMultMatMat_4x4( A, B, C);
      
      loader = (F32*) &C;
      
      result[0] = loader[0];
      result[1] = loader[1];
      result[2] = loader[2];
      result[3] = loader[3];
      result[4] = loader[4];
      result[5] = loader[5];
      result[6] = loader[6];
      result[7] = loader[7];
      result[8] = loader[8];
      result[9] = loader[9];
      result[10] = loader[10];
      result[11] = loader[11];
      result[12] = loader[12];
      result[13] = loader[13];
      result[14] = loader[14];
      result[15] = loader[15];
   }
   else
   {
      A[0][0] = vec_ld(0,  matA);
      A[1][0] = vec_ld(16, matA);
      A[2][0] = vec_ld(32, matA);
      A[3][0] = vec_ld(48, matA);
      B[0][0] = vec_ld(0,  matB);
      B[1][0] = vec_ld(16, matB);
      B[2][0] = vec_ld(32, matB);
      B[3][0] = vec_ld(48, matB);
      
      vMultMatMat_4x4( A, B, C);
      
      vec_st(C[0][0], 0,  result);
      vec_st(C[1][0], 16, result);
      vec_st(C[2][0], 32, result);
      vec_st(C[3][0], 48, result);
   }
}

void mInstallLibrary_Vec()
{
   m_matF_x_matF           = vec_MatrixF_x_MatrixF;
}
#else // defined(__VEC__)
void mInstallLibrary_Vec()
{
   Con::warnf("Cannot use altivec math, this build does not support altivec.");
}
#endif// defined(__VEC__)

#endif// defined(__APPLE__)