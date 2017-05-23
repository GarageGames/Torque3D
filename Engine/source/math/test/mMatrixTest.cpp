//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

#ifdef TORQUE_TESTS_ENABLED
#include "testing/unitTesting.h"
#include "platform/platform.h"
#include "math/mMatrix.h"
#include "math/mRandom.h"

extern void default_matF_x_matF_C(const F32 *a, const F32 *b, F32 *mresult);
extern void mInstallLibrary_ASM();

// If we're x86 and not Mac, then include these. There's probably a better way to do this.
#if defined(WIN32) && defined(TORQUE_CPU_X86)
void Athlon_MatrixF_x_MatrixF(const F32 *matA, const F32 *matB, F32 *result);
void SSE_MatrixF_x_MatrixF(const F32 *matA, const F32 *matB, F32 *result);
#endif

#if defined( __VEC__ )
extern void vec_MatrixF_x_MatrixF(const F32 *matA, const F32 *matB, F32 *result);
#endif

TEST(MatrixF, MultiplyImplmentations)
{
   F32 m1[16], m2[16], mrC[16];

   // I am not positive that the best way to do this is to use random numbers
   // but I think that using some kind of standard matrix may not always catch
   // all problems.
   for (S32 i = 0; i < 16; i++)
   {
      m1[i] = gRandGen.randF();
      m2[i] = gRandGen.randF();
   }

   // C will be the baseline
   default_matF_x_matF_C(m1, m2, mrC);

#if defined(WIN32) && defined(TORQUE_CPU_X86)
   // Check the CPU info
   U32 cpuProperties = Platform::SystemInfo.processor.properties;
   bool same;

   // Test 3D NOW! if it is available
   F32 mrAMD[16];
   if (cpuProperties & CPU_PROP_3DNOW)
   {
      Athlon_MatrixF_x_MatrixF(m1, m2, mrAMD);

      same = true;
      for (S32 i = 0; i < 16; i++)
         same &= mIsEqual(mrC[i], mrAMD[i]);

      EXPECT_TRUE(same) << "Matrix multiplication verification failed. (C vs. 3D NOW!)";
   }

   // Test SSE if it is available
   F32 mrSSE[16];
   if (cpuProperties & CPU_PROP_SSE)
   {
      SSE_MatrixF_x_MatrixF(m1, m2, mrSSE);

      same = true;
      for (S32 i = 0; i < 16; i++)
         same &= mIsEqual(mrC[i], mrSSE[i]);

      EXPECT_TRUE(same) << "Matrix multiplication verification failed. (C vs. SSE)";
   }

   same = true;
#endif

   // If Altivec exists, test it!
#if defined(__VEC__)
   bool same = false;
   F32 mrVEC[16];

   vec_MatrixF_x_MatrixF(m1, m2, mrVEC);

   for (S32 i = 0; i < 16; i++)
      same &= isEqual(mrC[i], mrVEC[i]);

   EXPECT_TRUE(same) << "Matrix multiplication verification failed. (C vs. Altivec)";
#endif
}

#endif