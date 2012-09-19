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

#if defined(TORQUE_CPU_X86)
#include "ts/tsMeshIntrinsics.h"
#include <xmmintrin.h>

void zero_vert_normal_bulk_SSE(const dsize_t count, U8 * __restrict const outPtr, const dsize_t outStride)
{
   // A U8 * version of the in/out pointer
   register char *outData = reinterpret_cast<char *>(outPtr);
   
   register __m128 vPos;
   register __m128 vNrm;
   register __m128 vMask;

   const __m128 _point3f_zero_mask = { 0.0f, 0.0f, 0.0f, 1.0f };
   vMask = _mm_load_ps((const F32*)&_point3f_zero_mask);

   // pre-populate cache
   for(int i = 0; i < 8; i++)
      _mm_prefetch(reinterpret_cast<const char *>(outData +  outStride * i), _MM_HINT_T0);

   for(int i = 0; i < count; i++)
   {
      TSMesh::__TSMeshVertexBase *curElem = reinterpret_cast<TSMesh::__TSMeshVertexBase *>(outData);

      // prefetch 8 items ahead (should really detect cache size or something)
      _mm_prefetch(reinterpret_cast<const char *>(outData +  outStride * 8), _MM_HINT_T0);

      // load
      vPos = _mm_load_ps(curElem->_vert);
      vNrm = _mm_load_ps(curElem->_normal);

      // mask
      vPos = _mm_mul_ps(vPos, _point3f_zero_mask);
      vNrm = _mm_mul_ps(vNrm, _point3f_zero_mask);

      // store
      _mm_store_ps(curElem->_vert, vPos);
      _mm_store_ps(curElem->_normal, vNrm);

      // update output pointer
      outData += outStride;
   }
}

//------------------------------------------------------------------------------

void m_matF_x_BatchedVertWeightList_SSE(const MatrixF &mat, 
                                    const dsize_t count,
                                    const TSSkinMesh::BatchData::BatchedVertWeight * __restrict batch,
                                    U8 * const __restrict outPtr,
                                    const dsize_t outStride)
{
   const char * __restrict iPtr = reinterpret_cast<const char *>(batch);
   const dsize_t inStride = sizeof(TSSkinMesh::BatchData::BatchedVertWeight);

   // SSE intrinsic version
   // Based on: http://www.cortstratton.org/articles/HugiCode.html

   // Load matrix, transposed, into registers
   MatrixF transMat;
   mat.transposeTo(transMat);
   register __m128 sseMat[4];

   sseMat[0] = _mm_loadu_ps(&transMat[0]);
   sseMat[1] = _mm_loadu_ps(&transMat[4]);
   sseMat[2] = _mm_loadu_ps(&transMat[8]);
   sseMat[3] = _mm_loadu_ps(&transMat[12]);

   // mask
   const __m128 _w_mask = { 1.0f, 1.0f, 1.0f, 0.0f };

   // temp registers
   register __m128 tempPos;
   register __m128 tempNrm;
   register __m128 scratch0;
   register __m128 scratch1;
   register __m128 inPos;
   register __m128 inNrm;

   // pre-populate cache
   const TSSkinMesh::BatchData::BatchedVertWeight &firstElem = batch[0];
   for(int i = 0; i < 8; i++)
   {
      _mm_prefetch(reinterpret_cast<const char *>(iPtr +  inStride * i), _MM_HINT_T0);
      _mm_prefetch(reinterpret_cast<const char *>(outPtr +  outStride * (i + firstElem.vidx)), _MM_HINT_T0);
   }

   for(register int i = 0; i < count; i++)
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

      // propagate the .x elements across the vectors
      tempPos = _mm_shuffle_ps(inPos, inPos, _MM_SHUFFLE(0, 0, 0, 0));
      tempNrm = _mm_shuffle_ps(inNrm, inNrm, _MM_SHUFFLE(0, 0, 0, 0));

      // prefetch ouput with half the lookahead distance of the input
#define OUTPUT_PREFETCH_LOOKAHEAD (INPUT_PREFETCH_LOOKAHEAD >> 1)
      const char *outPrefetch = reinterpret_cast<const char*>(outPtr) + outStride * (inElem.vidx + OUTPUT_PREFETCH_LOOKAHEAD);
      _mm_prefetch(outPrefetch, _MM_HINT_T0);

      // mul by column 0
      tempPos = _mm_mul_ps(tempPos, sseMat[0]);
      tempNrm = _mm_mul_ps(tempNrm, sseMat[0]);

      // process y
      scratch0 = _mm_shuffle_ps(inPos, inPos, _MM_SHUFFLE(1, 1, 1, 1));
      scratch1 = _mm_shuffle_ps(inNrm, inNrm, _MM_SHUFFLE(1, 1, 1, 1));

      scratch0 = _mm_mul_ps(scratch0, sseMat[1]);
      scratch1 = _mm_mul_ps(scratch1, sseMat[1]);

      tempPos = _mm_add_ps(tempPos, scratch0);
      tempNrm = _mm_add_ps(tempNrm, scratch1);


      // process z
      scratch0 = _mm_shuffle_ps(inPos, inPos, _MM_SHUFFLE(2, 2, 2, 2));
      scratch1 = _mm_shuffle_ps(inNrm, inNrm, _MM_SHUFFLE(2, 2, 2, 2));

      scratch0 = _mm_mul_ps(scratch0, sseMat[2]);
      scratch1 = _mm_mul_ps(scratch1, sseMat[2]);

      tempPos = _mm_add_ps(tempPos, scratch0);

      inNrm = _mm_load_ps(outElem->_normal); //< load normal for accumulation
      scratch0 = _mm_shuffle_ps(inPos, inPos, _MM_SHUFFLE(3, 3, 3, 3));//< load bone weight across all elements of scratch0

      tempNrm = _mm_add_ps(tempNrm, scratch1);

      scratch0 = _mm_mul_ps(scratch0, _w_mask); //< mask off last

      // Translate the position by adding the 4th column of the matrix to it
      tempPos = _mm_add_ps(tempPos, sseMat[3]);

      // now multiply by the blend weight, and mask out the W component of both vectors
      tempPos = _mm_mul_ps(tempPos, scratch0);
      tempNrm = _mm_mul_ps(tempNrm, scratch0);

      inPos = _mm_load_ps(outElem->_vert);   //< load position for accumulation

      // accumulate with previous values
      tempNrm = _mm_add_ps(tempNrm, inNrm);
      tempPos = _mm_add_ps(tempPos, inPos);

      _mm_store_ps(outElem->_vert, tempPos);   //< output position
      _mm_store_ps(outElem->_normal, tempNrm); //< output normal
   }
}

#endif // TORQUE_CPU_X86