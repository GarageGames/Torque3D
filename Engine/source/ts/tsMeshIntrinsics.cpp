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
#include "ts/tsMeshIntrinsics.h"
#include "ts/arch/tsMeshIntrinsics.arch.h"
#include "core/module.h"


void (*zero_vert_normal_bulk)(const dsize_t count, U8 * __restrict const outPtr, const dsize_t outStride) = NULL;
void (*m_matF_x_BatchedVertWeightList)(const MatrixF &mat, const dsize_t count, const TSSkinMesh::BatchData::BatchedVertWeight * __restrict batch, U8 * const __restrict outPtr, const dsize_t outStride) = NULL;

//------------------------------------------------------------------------------
// Default C++ Implementations (pretty slow)
//------------------------------------------------------------------------------

void zero_vert_normal_bulk_C(const dsize_t count, U8 * __restrict const outPtr, const dsize_t outStride)
{
   register char *outData = reinterpret_cast<char *>(outPtr);

   // TODO: Try prefetch w/ ptr de-reference

   for(register int i = 0; i < count; i++)
   {
      TSMesh::__TSMeshVertexBase *outElem = reinterpret_cast<TSMesh::__TSMeshVertexBase *>(outData);
      outElem->_vert.zero();
      outElem->_normal.zero();
      outData += outStride;
   }
}

//------------------------------------------------------------------------------

void m_matF_x_BatchedVertWeightList_C(const MatrixF &mat, 
                                    const dsize_t count,
                                    const TSSkinMesh::BatchData::BatchedVertWeight * __restrict batch,
                                    U8 * const __restrict outPtr,
                                    const dsize_t outStride)
{
   const register MatrixF m = mat;

   register Point3F tempPt;
   register Point3F tempNrm;

   for(register int i = 0; i < count; i++)
   {
      const TSSkinMesh::BatchData::BatchedVertWeight &inElem = batch[i];

      TSMesh::__TSMeshVertexBase *outElem = reinterpret_cast<TSMesh::__TSMeshVertexBase *>(outPtr + inElem.vidx * outStride);

      m.mulP( inElem.vert, &tempPt );
      m.mulV( inElem.normal, &tempNrm );

      outElem->_vert += ( tempPt * inElem.weight );
      outElem->_normal += ( tempNrm * inElem.weight );
   }
}

//------------------------------------------------------------------------------
// Initializer.
//------------------------------------------------------------------------------

MODULE_BEGIN( TSMeshIntrinsics )

   MODULE_INIT_AFTER( 3D )
   
   MODULE_INIT
   {
      // Assign defaults (C++ versions)
      zero_vert_normal_bulk = zero_vert_normal_bulk_C;
      m_matF_x_BatchedVertWeightList = m_matF_x_BatchedVertWeightList_C;

   #if defined(TORQUE_OS_XENON)
      zero_vert_normal_bulk = zero_vert_normal_bulk_X360;
      m_matF_x_BatchedVertWeightList = m_matF_x_BatchedVertWeightList_X360;
   #else
      // Find the best implementation for the current CPU
      if(Platform::SystemInfo.processor.properties & CPU_PROP_SSE)
      {
   #if defined(TORQUE_CPU_X86)
         
         zero_vert_normal_bulk = zero_vert_normal_bulk_SSE;
         m_matF_x_BatchedVertWeightList = m_matF_x_BatchedVertWeightList_SSE;

         /* This code still has a bug left in it
   #if (_MSC_VER >= 1500)
         if(Platform::SystemInfo.processor.properties & CPU_PROP_SSE4_1)
            m_matF_x_BatchedVertWeightList = m_matF_x_BatchedVertWeightList_SSE4;
   #endif
            */
   #endif
      }
      else if(Platform::SystemInfo.processor.properties & CPU_PROP_ALTIVEC)
      {
   #if !defined(TORQUE_OS_XENON) && defined(TORQUE_CPU_PPC)
         zero_vert_normal_bulk = zero_vert_normal_bulk_gccvec;
         m_matF_x_BatchedVertWeightList = m_matF_x_BatchedVertWeightList_gccvec;
   #endif
      }
   #endif
   }

MODULE_END;
