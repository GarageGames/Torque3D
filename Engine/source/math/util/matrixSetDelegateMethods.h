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
#ifndef _MATRIXSETDELEGATES_H_
#define _MATRIXSETDELEGATES_H_

   // Access to the direct value
#define MATRIX_SET_GET_VALUE_FN(xfm) _transform_##xfm
#define MATRIX_SET_GET_VALUE(xfm) inline const MatrixF &MATRIX_SET_GET_VALUE_FN(xfm)() { return mTransform[xfm]; }
#define MATRIX_SET_BIND_VALUE(xfm) mEvalDelegate[xfm].bind(this, &MatrixSet::MATRIX_SET_GET_VALUE_FN(xfm))

#define MATRIX_SET_IS_INVERSE_OF_FN(inv_xfm, src_xfm) _##inv_xfm##_is_inverse_of_##src_xfm
#define MATRIX_SET_IS_INVERSE_OF(inv_xfm, src_xfm) inline const MatrixF &MATRIX_SET_IS_INVERSE_OF_FN(inv_xfm, src_xfm)() \
   { \
      m_matF_invert_to(mEvalDelegate[src_xfm](), mTransform[inv_xfm]); \
      MATRIX_SET_BIND_VALUE(inv_xfm); \
      return mTransform[inv_xfm]; \
   }


#define MATRIX_SET_MULT_ASSIGN_FN(matA, matB, matC) _##matC##_is_##matA##_x_##matB
#define MATRIX_SET_MULT_ASSIGN(matA, matB, matC) inline const MatrixF &MATRIX_SET_MULT_ASSIGN_FN(matA, matB, matC)() \
   { \
      m_matF_x_matF_aligned(mEvalDelegate[matA](), mEvalDelegate[matB](), mTransform[matC]); \
      MATRIX_SET_BIND_VALUE(matC); \
      return mTransform[matC]; \
   }


#endif // _MATRIXSETDELEGATES_H_
