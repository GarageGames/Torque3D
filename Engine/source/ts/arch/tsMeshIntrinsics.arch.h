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

#ifndef _TSMESHINTRINSICS_ARCH_H_
#define _TSMESHINTRINSICS_ARCH_H_

#if (defined( TORQUE_CPU_X86 ) || defined( TORQUE_CPU_X64 )) 
# // x86 CPU family implementations
extern void zero_vert_normal_bulk_SSE(const dsize_t count, U8 * __restrict const outPtr, const dsize_t outStride);
#
#elif defined(TORQUE_CPU_PPC)
# // PPC CPU family implementations
#  if defined(TORQUE_OS_XENON)
extern void zero_vert_normal_bulk_X360(const dsize_t count, U8 * __restrict const outPtr, const dsize_t outStride);
#  else
extern void zero_vert_normal_bulk_gccvec(const dsize_t count, U8 * __restrict const outPtr, const dsize_t outStride);
#  endif
#
#else
# // Other CPU types go here...
#endif

#endif // _TSMESHINTRINSICS_ARCH_H_

