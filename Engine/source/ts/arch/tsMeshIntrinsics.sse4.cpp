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
#include "ts/tsMesh.h"

#if defined(TORQUE_CPU_X86) && (_MSC_VER >= 1500)
#include "ts/tsMeshIntrinsics.h"
#include <smmintrin.h>

void m_matF_x_BatchedVertWeightList_SSE4(const MatrixF &mat, 
                                    const dsize_t count,
                                    const TSSkinMesh::BatchData::BatchedVertWeight * __restrict batch,
                                    U8 * const __restrict outPtr,
                                    const dsize_t outStride)
{
   const char * __restrict iPtr = reinterpret_cast<const char *>(batch);
   const dsize_t inStride = sizeof(TSSkinMesh::BatchData::BatchedVertWeight);

   __m128 sseMat[3];
   sseMat[0] = _mm_loadu_ps(&mat[0]);
   sseMat[1] = _mm_loadu_ps(&mat[4]);
   sseMat[2] = _mm_loadu_ps(&mat[8]);

   // temp registers
   __m128 inPos, tempPos;
   __m128 inNrm, tempNrm;
   __m128 temp0, temp1, temp2, temp3;

   // pre-populate cache
   const TSSkinMesh::BatchData::BatchedVertWeight &firstElem = batch[0];
   for(int i = 0; i < 8; i++)
   {
      _mm_prefetch(reinterpret_cast<const char *>(iPtr +  inStride * i), _MM_HINT_T0);
      _mm_prefetch(reinterpret_cast<const char *>(outPtr +  outStride * (i + firstElem.vidx)), _MM_HINT_T0);
   }

   for(int i = 0; i < count; i++)
   {
      const TSSkinMesh::BatchData::BatchedVertWeight &inElem = batch[i];
      TSMesh::__TSMeshVertexBase *outElem = reinterpret_cast<TSMesh::__TSMeshVertexBase *>(outPtr + inElem.vidx * outStride);

      // process x (hiding the prefetches in the delays)
      inPos = _mm_load_ps(inElem.vert);
      inNrm = _mm_load_ps(inElem.normal);

      // prefetch input 
#define INPUT_PREFETCH_LOOKAHEAD 64
      const char *prefetchInput = reinterpret_cast<const char *>(batch) + inStride * (i + INPUT_PREFETCH_LOOKAHEAD);
      _mm_prefetch(prefetchInput, _MM_HINT_T0);

      // prefetch ouput with half the lookahead distance of the input
#define OUTPUT_PREFETCH_LOOKAHEAD (INPUT_PREFETCH_LOOKAHEAD >> 1)
      const char *outPrefetch = reinterpret_cast<const char*>(outPtr) + outStride * (inElem.vidx + OUTPUT_PREFETCH_LOOKAHEAD);
      _mm_prefetch(outPrefetch, _MM_HINT_T0);

      // Multiply position
      tempPos = _mm_dp_ps(inPos, sseMat[0], 0xF1);
      temp0 = _mm_dp_ps(inPos, sseMat[1], 0xF2);
      temp1 = _mm_dp_ps(inPos, sseMat[2], 0xF4);
      
      temp0 = _mm_or_ps(temp0, temp1);
      tempPos = _mm_or_ps(tempPos, temp0);

      // Multiply normal
      tempNrm = _mm_dp_ps(inNrm, sseMat[0], 0x71);
      temp2 = _mm_dp_ps(inNrm, sseMat[1], 0x72);
      temp3 = _mm_dp_ps(inNrm, sseMat[2], 0x74);

      temp2 = _mm_or_ps(temp2, temp3);
      tempNrm = _mm_or_ps(tempNrm, temp2);

      // Load bone weight and multiply
      temp3 = _mm_shuffle_ps(inPos, inPos, _MM_SHUFFLE(3, 3, 3, 3));

      tempPos = _mm_mul_ps(tempPos, temp3);
      tempNrm = _mm_mul_ps(tempNrm, temp3);

      inPos = _mm_load_ps(outElem->_vert);   //< load position for accumulation
      inNrm = _mm_load_ps(outElem->_normal); //< load normal for accumulation

      // accumulate with previous values
      tempNrm = _mm_add_ps(tempNrm, inNrm);
      tempPos = _mm_add_ps(tempPos, inPos);

      _mm_store_ps(outElem->_vert, tempPos);   //< output position
      _mm_store_ps(outElem->_normal, tempNrm); //< output normal
   }
}

#endif // TORQUE_CPU_X86