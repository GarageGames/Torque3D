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

//------------------------------------------------------------------------------
// Default C++ Implementations (pretty slow)
//------------------------------------------------------------------------------

void zero_vert_normal_bulk_C(const dsize_t count, U8 * __restrict const outPtr, const dsize_t outStride)
{
   char *outData = reinterpret_cast<char *>(outPtr);

   // TODO: Try prefetch w/ ptr de-reference

   for(S32 i = 0; i < count; i++)
   {
      TSMesh::__TSMeshVertexBase *outElem = reinterpret_cast<TSMesh::__TSMeshVertexBase *>(outData);
      outElem->_vert.zero();
      outElem->_normal.zero();
      outData += outStride;
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

   #if defined(TORQUE_OS_XENON)
      zero_vert_normal_bulk = zero_vert_normal_bulk_X360;
   #else
      // Find the best implementation for the current CPU
      if(Platform::SystemInfo.processor.properties & CPU_PROP_SSE)
      {
   #if (defined( TORQUE_CPU_X86 ) || defined( TORQUE_CPU_X64 )) 
         
         zero_vert_normal_bulk = zero_vert_normal_bulk_SSE;
   #endif
      }
      else if(Platform::SystemInfo.processor.properties & CPU_PROP_ALTIVEC)
      {
   #if !defined(TORQUE_OS_XENON) && defined(TORQUE_CPU_PPC)
         zero_vert_normal_bulk = zero_vert_normal_bulk_gccvec;
   #endif
      }
   #endif
   }

MODULE_END;
