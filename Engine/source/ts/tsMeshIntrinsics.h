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

#ifndef _TSMESHINTRINSICS_H_
#define _TSMESHINTRINSICS_H_

/// This is the batch-by-transform skin loop
///
/// @param mat       Bone transform
/// @param count     Number of input elements in the batch
/// @param batch     Pointer to the first element in an aligned array of input elements
/// @param outPtr    Pointer to index 0 of a TSMesh aligned vertex buffer
/// @param outStride Size, in bytes, of one entry in the vertex buffer
extern void (*m_matF_x_BatchedVertWeightList)
                                   (const MatrixF &mat, 
                                    const dsize_t count,
                                    const TSSkinMesh::BatchData::BatchedVertWeight * __restrict batch,
                                    U8 * const __restrict outPtr,
                                    const dsize_t outStride);

/// Set the vertex position and normal to (0, 0, 0)
///
/// @param count     Number of elements
/// @param outPtr    Pointer to a TSMesh aligned vertex buffer
/// @param outStride Size, in bytes, of one entry in the vertex buffer
extern void (*zero_vert_normal_bulk)
                          (const dsize_t count, 
                           U8 * __restrict const outPtr, 
                           const dsize_t outStride);

#endif

