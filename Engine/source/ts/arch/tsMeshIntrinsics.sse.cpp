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

#if (defined( TORQUE_CPU_X86 ) || defined( TORQUE_CPU_X64 ))
#include "ts/tsMeshIntrinsics.h"
#include <xmmintrin.h>

void zero_vert_normal_bulk_SSE(const dsize_t count, U8 * __restrict const outPtr, const dsize_t outStride)
{
   // A U8 * version of the in/out pointer
   char *outData = reinterpret_cast<char *>(outPtr);
   
   __m128 vPos;
   __m128 vNrm;
   __m128 vMask;

   const __m128 _point3f_zero_mask = { 0.0f, 0.0f, 0.0f, 1.0f };
   vMask = _mm_load_ps((const F32*)&_point3f_zero_mask);

   // pre-populate cache
   for(S32 i = 0; i < 8; i++)
      _mm_prefetch(reinterpret_cast<const char *>(outData +  outStride * i), _MM_HINT_T0);

   for(S32 i = 0; i < count; i++)
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

#endif // TORQUE_CPU_X86