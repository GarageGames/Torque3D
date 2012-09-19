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

#include "platform/platform.h"
#include "math/mMath.h"
#include "math/util/frustum.h"
#include <math.h>    // Caution!!! Possible platform specific include


//------------------------------------------------------------------------------
// C version of Math Library

// asm externals
extern "C" {

   S32 m_mulDivS32_ASM( S32 a, S32 b, S32 c );
   U32 m_mulDivU32_ASM( U32 a, U32 b, U32 c );
   F32 m_fmod_ASM(F32 val, F32 modulo);
   F32 m_fmodD_ASM(F64 val, F64 modulo);
   void m_sincos_ASM( F32 angle, F32 *s, F32 *c );
   void m_sincosD_ASM( F64 angle, F64 *s, F64 *c );

}
//--------------------------------------

static S32 m_mulDivS32_C(S32 a, S32 b, S32 c)
{
   // S64/U64 support in most 32-bit compilers generate
   // horrible code, the C version are here just for porting
   // assembly implementation is recommended
   return (S32) ((S64)a*(S64)b) / (S64)c;
}

static U32 m_mulDivU32_C(S32 a, S32 b, U32 c)
{
   return (U32) ((S64)a*(S64)b) / (U64)c;
}


//--------------------------------------
static F32 m_catmullrom_C(F32 t, F32 p0, F32 p1, F32 p2, F32 p3)
{
   return 0.5f * ((3.0f*p1 - 3.0f*p2 + p3 - p0)*t*t*t
                  +  (2.0f*p0 - 5.0f*p1 + 4.0f*p2 - p3)*t*t
                  +  (p2-p0)*t
                  +  2.0f*p1);
}

//--------------------------------------
static void m_sincos_C( F32 angle, F32 *s, F32 *c )
{
   *s = mSin( angle );
   *c = mCos( angle );
}

static void m_sincosD_C( F64 angle, F64 *s, F64 *c )
{
   *s = mSin( angle );
   *c = mCos( angle );
}

//--------------------------------------
static void m_point2F_normalize_C(F32 *p)
{
   F32 factor = 1.0f / mSqrt(p[0]*p[0] + p[1]*p[1] );
   p[0] *= factor;
   p[1] *= factor;
}

//--------------------------------------
static void m_point2F_normalize_f_C(F32 *p, F32 val)
{
   F32 factor = val / mSqrt(p[0]*p[0] + p[1]*p[1] );
   p[0] *= factor;
   p[1] *= factor;
}

//--------------------------------------
static void m_point2D_normalize_C(F64 *p)
{
   F64 factor = 1.0f / mSqrtD(p[0]*p[0] + p[1]*p[1] );
   p[0] *= factor;
   p[1] *= factor;
}

//--------------------------------------
static void m_point2D_normalize_f_C(F64 *p, F64 val)
{
   F64 factor = val / mSqrtD(p[0]*p[0] + p[1]*p[1] );
   p[0] *= factor;
   p[1] *= factor;
}

//--------------------------------------
static void m_point3D_normalize_f_C(F64 *p, F64 val)
{
   F64 factor = val / mSqrtD(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
   p[0] *= factor;
   p[1] *= factor;
   p[2] *= factor;
}

//--------------------------------------
static void m_point3F_normalize_C(F32 *p)
{
   F32 squared = p[0]*p[0] + p[1]*p[1] + p[2]*p[2];
   // This can happen in Container::castRay -> ForceFieldBare::castRay
   //AssertFatal(squared != 0.0, "Error, zero length vector normalized!");
   if (squared != 0.0f) {
      F32 factor = 1.0f / mSqrt(squared);
      p[0] *= factor;
      p[1] *= factor;
      p[2] *= factor;
   } else {
      p[0] = 0.0f;
      p[1] = 0.0f;
      p[2] = 1.0f;
   }
}

//--------------------------------------
static void m_point3F_normalize_f_C(F32 *p, F32 val)
{
   F32 factor = val / mSqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2] );
   p[0] *= factor;
   p[1] *= factor;
   p[2] *= factor;
}

//--------------------------------------
static void m_point3F_interpolate_C(const F32 *from, const F32 *to, F32 factor, F32 *result )
{
#ifdef TORQUE_COMPILER_GCC
// remove possibility of aliases
   const F32 inverse = 1.0f - factor;
   const F32	from0 = from[0], from1 = from[1], from2 = from[2];
   const F32	to0 = to[0], to1 = to[1], to2 = to[2];
   
   result[0] = from0 * inverse + to0 * factor;
   result[1] = from1 * inverse + to1 * factor;
   result[2] = from2 * inverse + to2 * factor;
#else
   F32 inverse = 1.0f - factor;
   result[0] = from[0] * inverse + to[0] * factor;
   result[1] = from[1] * inverse + to[1] * factor;
   result[2] = from[2] * inverse + to[2] * factor;
#endif
}

//--------------------------------------
static void m_point3D_normalize_C(F64 *p)
{
   F64 factor = 1.0f / mSqrtD(p[0]*p[0] + p[1]*p[1] + p[2]*p[2] );
   p[0] *= factor;
   p[1] *= factor;
   p[2] *= factor;
}


//--------------------------------------
static void m_point3D_interpolate_C(const F64 *from, const F64 *to, F64 factor, F64 *result )
{
#ifdef TORQUE_COMPILER_GCC
// remove possibility of aliases
   const F64 inverse = 1.0f - factor;
   const F64	from0 = from[0], from1 = from[1], from2 = from[2];
   const F64	to0 = to[0], to1 = to[1], to2 = to[2];
   
   result[0] = from0 * inverse + to0 * factor;
   result[1] = from1 * inverse + to1 * factor;
   result[2] = from2 * inverse + to2 * factor;
#else
   F64 inverse = 1.0f - factor;
   result[0] = from[0] * inverse + to[0] * factor;
   result[1] = from[1] * inverse + to[1] * factor;
   result[2] = from[2] * inverse + to[2] * factor;
#endif
}


static void m_quatF_set_matF_C( F32 x, F32 y, F32 z, F32 w, F32* m )
{
#define qidx(r,c) (r*4 + c)
      F32 xs = x * 2.0f;
      F32 ys = y * 2.0f;
      F32 zs = z * 2.0f;
      F32 wx = w * xs;
      F32 wy = w * ys;
      F32 wz = w * zs;
      F32 xx = x * xs;
      F32 xy = x * ys;
      F32 xz = x * zs;
      F32 yy = y * ys;
      F32 yz = y * zs;
      F32 zz = z * zs;
      m[qidx(0,0)] = 1.0f - (yy + zz);
      m[qidx(1,0)] = xy - wz;
      m[qidx(2,0)] = xz + wy;
      m[qidx(3,0)] = 0.0f;
      m[qidx(0,1)] = xy + wz;
      m[qidx(1,1)] = 1.0f - (xx + zz);
      m[qidx(2,1)] = yz - wx;
      m[qidx(3,1)] = 0.0f;
      m[qidx(0,2)] = xz - wy;
      m[qidx(1,2)] = yz + wx;
      m[qidx(2,2)] = 1.0f - (xx + yy);
      m[qidx(3,2)] = 0.0f;

      m[qidx(0,3)] = 0.0f;
      m[qidx(1,3)] = 0.0f;
      m[qidx(2,3)] = 0.0f;
      m[qidx(3,3)] = 1.0f;
#undef qidx
}


//--------------------------------------
static void m_matF_set_euler_C(const F32 *e, F32 *result)
{
   enum {
      AXIS_X   = (1<<0),
      AXIS_Y   = (1<<1),
      AXIS_Z   = (1<<2)
   };

   U32 axis = 0;
   if (e[0] != 0.0f) axis |= AXIS_X;
   if (e[1] != 0.0f) axis |= AXIS_Y;
   if (e[2] != 0.0f) axis |= AXIS_Z;

   switch (axis)
   {
      case 0:
         m_matF_identity(result);
         break;

      case AXIS_X:
      {
         F32 cx,sx;
         mSinCos( e[0], sx, cx );

         result[0] = 1.0f;
         result[1] = 0.0f;
         result[2] = 0.0f;
         result[3] = 0.0f;

         result[4] = 0.0f;
         result[5] = cx;
         result[6] = sx;
         result[7] = 0.0f;

         result[8] = 0.0f;
         result[9] = -sx;
         result[10]= cx;
         result[11]= 0.0f;

         result[12]= 0.0f;
         result[13]= 0.0f;
         result[14]= 0.0f;
         result[15]= 1.0f;
         break;
      }

      case AXIS_Y:
      {
         F32 cy,sy;
         mSinCos( e[1], sy, cy );

         result[0] = cy;
         result[1] = 0.0f;
         result[2] = -sy;
         result[3] = 0.0f;

         result[4] = 0.0f;
         result[5] = 1.0f;
         result[6] = 0.0f;
         result[7] = 0.0f;

         result[8] = sy;
         result[9] = 0.0f;
         result[10]= cy;
         result[11]= 0.0f;

         result[12]= 0.0f;
         result[13]= 0.0f;
         result[14]= 0.0f;
         result[15]= 1.0f;
         break;
      }

      case AXIS_Z:
      {
         // the matrix looks like this:
         //  r1 - (r4 * sin(x))     r2 + (r3 * sin(x))   -cos(x) * sin(y)
         //  -cos(x) * sin(z)       cos(x) * cos(z)      sin(x)
         //  r3 + (r2 * sin(x))     r4 - (r1 * sin(x))   cos(x) * cos(y)
         //
         // where:
         //  r1 = cos(y) * cos(z)
         //  r2 = cos(y) * sin(z)
         //  r3 = sin(y) * cos(z)
         //  r4 = sin(y) * sin(z)
         F32 cz,sz;
         mSinCos( e[2], sz, cz );

         result[0] = cz;
         result[1] = sz;
         result[2] = 0.0f;
         result[3] = 0.0f;

         result[4] = -sz;
         result[5] = cz;
         result[6] = 0.0f;
         result[7] = 0.0f;

         result[8] = 0.0f;
         result[9] = 0.0f;
         result[10]= 1.0f;
         result[11]= 0.0f;

         result[12]= 0.0f;
         result[13]= 0.0f;
         result[14]= 0.0f;
         result[15]= 1.0f;
         break;
      }

      default:
         // the matrix looks like this:
         //  r1 - (r4 * sin(x))     r2 + (r3 * sin(x))   -cos(x) * sin(y)
         //  -cos(x) * sin(z)       cos(x) * cos(z)      sin(x)
         //  r3 + (r2 * sin(x))     r4 - (r1 * sin(x))   cos(x) * cos(y)
         //
         // where:
         //  r1 = cos(y) * cos(z)
         //  r2 = cos(y) * sin(z)
         //  r3 = sin(y) * cos(z)
         //  r4 = sin(y) * sin(z)
         F32 cx,sx;
         mSinCos( e[0], sx, cx );
         F32 cy,sy;
         mSinCos( e[1], sy, cy );
         F32 cz,sz;
         mSinCos( e[2], sz, cz );
         F32 r1 = cy * cz;
         F32 r2 = cy * sz;
         F32 r3 = sy * cz;
         F32 r4 = sy * sz;

         result[0] = r1 - (r4 * sx);
         result[1] = r2 + (r3 * sx);
         result[2] = -cx * sy;
         result[3] = 0.0f;

         result[4] = -cx * sz;
         result[5] = cx * cz;
         result[6] = sx;
         result[7] = 0.0f;

         result[8] = r3 + (r2 * sx);
         result[9] = r4 - (r1 * sx);
         result[10]= cx * cy;
         result[11]= 0.0f;

         result[12]= 0.0f;
         result[13]= 0.0f;
         result[14]= 0.0f;
         result[15]= 1.0f;
         break;
   }
}


//--------------------------------------
static void m_matF_set_euler_point_C(const F32 *e, const F32 *p, F32 *result)
{
   m_matF_set_euler(e, result);
   result[3] = p[0];
   result[7] = p[1];
   result[11]= p[2];
}

//--------------------------------------
static void m_matF_identity_C(F32 *m)
{
   *m++ = 1.0f;
   *m++ = 0.0f;
   *m++ = 0.0f;
   *m++ = 0.0f;

   *m++ = 0.0f;
   *m++ = 1.0f;
   *m++ = 0.0f;
   *m++ = 0.0f;

   *m++ = 0.0f;
   *m++ = 0.0f;
   *m++ = 1.0f;
   *m++ = 0.0f;

   *m++ = 0.0f;
   *m++ = 0.0f;
   *m++ = 0.0f;
   *m   = 1.0f;
}

#if 0
// Compile this out till we hook it up.  It's a more efficient matrix
// inverse than what we have (it uses intermediate results of determinant
// to same about 1/4 of the operations.
static void affineInvertTo(const F32 * m, F32 * out)
{
#define idx(r,c) (r*4 + c)
   F32 d1 = m[idx(2,2)] * m[idx(1,1)] - m[idx(2,1)] * m[idx(1,2)];
   F32 d2 = m[idx(2,0)] * m[idx(1,2)] - m[idx(2,2)] * m[idx(1,0)];
   F32 d3 = m[idx(2,1)] * m[idx(1,0)] - m[idx(2,0)] * m[idx(1,1)];

   F32 invDet = 1.0f / (m[idx(0,0)] * d1 + m[idx(0,1)] * d2 + m[idx(0,2)] * d3);

   F32 m00 = m[idx(0,0)] * invDet; 
   F32 m01 = m[idx(0,1)] * invDet; 
   F32 m02 = m[idx(0,2)] * invDet;

   F32 * result = out;
   *out++ = d1 * invDet;
   *out++ = m02 * m[idx(2,1)] - m01 * m[idx(2,2)];
   *out++ = m01 * m[idx(1,2)] - m02 * m[idx(1,1)];
   *out++ = 0.0f;

   *out++ = d2 * invDet;
   *out++ = m00 * m[idx(2,2)] - m02 * m[idx(2,0)];
   *out++ = m02 * m[idx(1,0)] - m00 * m[idx(1,2)];
   *out++ = 0.0f;

   *out++ = d3 * invDet;
   *out++ = m01 * m[idx(2,0)] - m00 * m[idx(2,1)];
   *out++ = m00 * m[idx(1,1)] - m01 * m[idx(1,0)];
   *out++ = 0.0f;

   *out++ = -result[idx(0,0)] * m[idx(0,3)] - result[idx(0,1)] * m[idx(1,3)] - result[idx(0,2)] * m[idx(2,3)];
   *out++ = -result[idx(1,0)] * m[idx(0,3)] - result[idx(1,1)] * m[idx(1,3)] - result[idx(1,2)] * m[idx(2,3)];
   *out++ = -result[idx(2,0)] * m[idx(0,3)] - result[idx(2,1)] * m[idx(1,3)] - result[idx(2,2)] * m[idx(2,3)];
   *out++ = 1.0f;
#undef idx
}
#endif

//--------------------------------------
static void m_matF_inverse_C(F32 *m)
{
   // using Cramers Rule find the Inverse
   // Minv = (1/det(M)) * adjoint(M)
   F32 det = m_matF_determinant( m );
   AssertFatal( det != 0.0f, "MatrixF::inverse: non-singular matrix, no inverse.");

   F32 invDet = 1.0f/det;
   F32 temp[16];

   temp[0] = (m[5] * m[10]- m[6] * m[9]) * invDet;
   temp[1] = (m[9] * m[2] - m[10]* m[1]) * invDet;
   temp[2] = (m[1] * m[6] - m[2] * m[5]) * invDet;

   temp[4] = (m[6] * m[8] - m[4] * m[10])* invDet;
   temp[5] = (m[10]* m[0] - m[8] * m[2]) * invDet;
   temp[6] = (m[2] * m[4] - m[0] * m[6]) * invDet;

   temp[8] = (m[4] * m[9] - m[5] * m[8]) * invDet;
   temp[9] = (m[8] * m[1] - m[9] * m[0]) * invDet;
   temp[10]= (m[0] * m[5] - m[1] * m[4]) * invDet;

   m[0] = temp[0];
   m[1] = temp[1];
   m[2] = temp[2];

   m[4] = temp[4];
   m[5] = temp[5];
   m[6] = temp[6];

   m[8] = temp[8];
   m[9] = temp[9];
   m[10] = temp[10];

   // invert the translation
   temp[0] = -m[3];
   temp[1] = -m[7];
   temp[2] = -m[11];
   m_matF_x_vectorF(m, temp, &temp[4]);
   m[3] = temp[4];
   m[7] = temp[5];
   m[11]= temp[6];
}

static void m_matF_invert_to_C(const F32 *m, F32 *d)
{
   // using Cramers Rule find the Inverse
   // Minv = (1/det(M)) * adjoint(M)
   F32 det = m_matF_determinant( m );
   AssertFatal( det != 0.0f, "MatrixF::inverse: non-singular matrix, no inverse.");

   F32 invDet = 1.0f/det;

   d[0] = (m[5] * m[10]- m[6] * m[9]) * invDet;
   d[1] = (m[9] * m[2] - m[10]* m[1]) * invDet;
   d[2] = (m[1] * m[6] - m[2] * m[5]) * invDet;

   d[4] = (m[6] * m[8] - m[4] * m[10])* invDet;
   d[5] = (m[10]* m[0] - m[8] * m[2]) * invDet;
   d[6] = (m[2] * m[4] - m[0] * m[6]) * invDet;

   d[8] = (m[4] * m[9] - m[5] * m[8]) * invDet;
   d[9] = (m[8] * m[1] - m[9] * m[0]) * invDet;
   d[10]= (m[0] * m[5] - m[1] * m[4]) * invDet;

   // invert the translation
   F32 temp[6];
   temp[0] = -m[3];
   temp[1] = -m[7];
   temp[2] = -m[11];
   m_matF_x_vectorF(d, temp, &temp[3]);
   d[3] = temp[3];
   d[7] = temp[4];
   d[11]= temp[5];
   d[ 12 ] = m[ 12 ];
   d[ 13 ] = m[ 13 ];
   d[ 14 ] = m[ 14 ];
   d[ 15 ] = m[ 15 ];
}

//--------------------------------------
static void m_matF_affineInverse_C(F32 *m)
{
   // Matrix class checks to make sure this is an affine transform before calling
   //  this function, so we can proceed assuming it is...
   F32 temp[16];
   dMemcpy(temp, m, 16 * sizeof(F32));

   // Transpose rotation
   m[1] = temp[4];
   m[4] = temp[1];
   m[2] = temp[8];
   m[8] = temp[2];
   m[6] = temp[9];
   m[9] = temp[6];

   m[3]  = -(temp[0]*temp[3] + temp[4]*temp[7] + temp[8]*temp[11]);
   m[7]  = -(temp[1]*temp[3] + temp[5]*temp[7] + temp[9]*temp[11]);
   m[11] = -(temp[2]*temp[3] + temp[6]*temp[7] + temp[10]*temp[11]);
}

inline void swap(F32 &a, F32 &b)
{
   F32 temp = a;
   a = b;
   b = temp;
}

//--------------------------------------
static void m_matF_transpose_C(F32 *m)
{
   swap(m[1], m[4]);
   swap(m[2], m[8]);
   swap(m[3], m[12]);
   swap(m[6], m[9]);
   swap(m[7], m[13]);
   swap(m[11],m[14]);
}

//--------------------------------------
static void m_matF_scale_C(F32 *m,const F32 *p)
{
   // Note, doesn't allow scaling w...

   m[0]  *= p[0];  m[1]  *= p[1];  m[2]  *= p[2];
   m[4]  *= p[0];  m[5]  *= p[1];  m[6]  *= p[2];
   m[8]  *= p[0];  m[9]  *= p[1];  m[10] *= p[2];
   m[12] *= p[0];  m[13] *= p[1];  m[14] *= p[2];
}

//--------------------------------------
static void m_matF_normalize_C(F32 *m)
{
   F32 col0[3], col1[3], col2[3];
   // extract columns 0 and 1
   col0[0] = m[0];
   col0[1] = m[4];
   col0[2] = m[8];

   col1[0] = m[1];
   col1[1] = m[5];
   col1[2] = m[9];

   // assure their relationships to one another
   mCross(*(Point3F*)col0, *(Point3F*)col1, (Point3F*)col2);
   mCross(*(Point3F*)col2, *(Point3F*)col0, (Point3F*)col1);

   // assure their length is 1.0f
   m_point3F_normalize( col0 );
   m_point3F_normalize( col1 );
   m_point3F_normalize( col2 );

   // store the normalized columns
   m[0] = col0[0];
   m[4] = col0[1];
   m[8] = col0[2];

   m[1] = col1[0];
   m[5] = col1[1];
   m[9] = col1[2];

   m[2] = col2[0];
   m[6] = col2[1];
   m[10]= col2[2];
}


//--------------------------------------
static F32 m_matF_determinant_C(const F32 *m)
{
   return m[0] * (m[5] * m[10] - m[6] * m[9])  +
      m[4] * (m[2] * m[9]  - m[1] * m[10]) +
      m[8] * (m[1] * m[6]  - m[2] * m[5])  ;
}


//--------------------------------------
// Removed static in order to write benchmarking code (that compares against
// specialized SSE/AMD versions) elsewhere.
void default_matF_x_matF_C(const F32 *a, const F32 *b, F32 *mresult)
{
   mresult[0] = a[0]*b[0] + a[1]*b[4] + a[2]*b[8]  + a[3]*b[12];
   mresult[1] = a[0]*b[1] + a[1]*b[5] + a[2]*b[9]  + a[3]*b[13];
   mresult[2] = a[0]*b[2] + a[1]*b[6] + a[2]*b[10] + a[3]*b[14];
   mresult[3] = a[0]*b[3] + a[1]*b[7] + a[2]*b[11] + a[3]*b[15];

   mresult[4] = a[4]*b[0] + a[5]*b[4] + a[6]*b[8]  + a[7]*b[12];
   mresult[5] = a[4]*b[1] + a[5]*b[5] + a[6]*b[9]  + a[7]*b[13];
   mresult[6] = a[4]*b[2] + a[5]*b[6] + a[6]*b[10] + a[7]*b[14];
   mresult[7] = a[4]*b[3] + a[5]*b[7] + a[6]*b[11] + a[7]*b[15];

   mresult[8] = a[8]*b[0] + a[9]*b[4] + a[10]*b[8] + a[11]*b[12];
   mresult[9] = a[8]*b[1] + a[9]*b[5] + a[10]*b[9] + a[11]*b[13];
   mresult[10]= a[8]*b[2] + a[9]*b[6] + a[10]*b[10]+ a[11]*b[14];
   mresult[11]= a[8]*b[3] + a[9]*b[7] + a[10]*b[11]+ a[11]*b[15];

   mresult[12]= a[12]*b[0]+ a[13]*b[4]+ a[14]*b[8] + a[15]*b[12];
   mresult[13]= a[12]*b[1]+ a[13]*b[5]+ a[14]*b[9] + a[15]*b[13];
   mresult[14]= a[12]*b[2]+ a[13]*b[6]+ a[14]*b[10]+ a[15]*b[14];
   mresult[15]= a[12]*b[3]+ a[13]*b[7]+ a[14]*b[11]+ a[15]*b[15];
}


// //--------------------------------------
// static void m_matF_x_point3F_C(const F32 *m, const F32 *p, F32 *presult)
// {
//    AssertFatal(p != presult, "Error, aliasing matrix mul pointers not allowed here!");
//    presult[0] = m[0]*p[0] + m[1]*p[1] + m[2]*p[2]  + m[3];
//    presult[1] = m[4]*p[0] + m[5]*p[1] + m[6]*p[2]  + m[7];
//    presult[2] = m[8]*p[0] + m[9]*p[1] + m[10]*p[2] + m[11];
// }


// //--------------------------------------
// static void m_matF_x_vectorF_C(const F32 *m, const F32 *v, F32 *vresult)
// {
//    AssertFatal(v != vresult, "Error, aliasing matrix mul pointers not allowed here!");
//    vresult[0] = m[0]*v[0] + m[1]*v[1] + m[2]*v[2];
//    vresult[1] = m[4]*v[0] + m[5]*v[1] + m[6]*v[2];
//    vresult[2] = m[8]*v[0] + m[9]*v[1] + m[10]*v[2];
// }


//--------------------------------------
static void m_matF_x_point4F_C(const F32 *m, const F32 *p, F32 *presult)
{
   AssertFatal(p != presult, "Error, aliasing matrix mul pointers not allowed here!");
   presult[0] = m[0]*p[0] + m[1]*p[1] + m[2]*p[2]  + m[3]*p[3];
   presult[1] = m[4]*p[0] + m[5]*p[1] + m[6]*p[2]  + m[7]*p[3];
   presult[2] = m[8]*p[0] + m[9]*p[1] + m[10]*p[2] + m[11]*p[3];
   presult[3] = m[12]*p[0]+ m[13]*p[1]+ m[14]*p[2] + m[15]*p[3];
}

//--------------------------------------
static void m_matF_x_scale_x_planeF_C(const F32* m, const F32* s, const F32* p, F32* presult)
{
   // We take in a matrix, a scale factor, and a plane equation.  We want to output
   //  the resultant normal
   // We have T = m*s
   // To multiply the normal, we want Inv(Tr(m*s))
   //  Inv(Tr(ms)) = Inv(Tr(s) * Tr(m))
   //              = Inv(Tr(m)) * Inv(Tr(s))
   //
   //  Inv(Tr(s)) = Inv(s) = [ 1/x   0   0  0]
   //                        [   0 1/y   0  0]
   //                        [   0   0 1/z  0]
   //                        [   0   0   0  1]
   //
   // Since m is an affine matrix,
   //  Tr(m) = [ [       ] 0 ]
   //          [ [   R   ] 0 ]
   //          [ [       ] 0 ]
   //          [ [ x y z ] 1 ]
   //
   // Inv(Tr(m)) = [ [    -1 ] 0 ]
   //              [ [   R   ] 0 ]
   //              [ [       ] 0 ]
   //              [ [ A B C ] 1 ]
   // Where:
   //
   //  P = (x, y, z)
   //  A = -(Row(0, r) * P);
   //  B = -(Row(1, r) * P);
   //  C = -(Row(2, r) * P);

   MatrixF invScale(true);
   F32* pScaleElems = invScale;
   pScaleElems[MatrixF::idx(0, 0)] = 1.0f / s[0];
   pScaleElems[MatrixF::idx(1, 1)] = 1.0f / s[1];
   pScaleElems[MatrixF::idx(2, 2)] = 1.0f / s[2];

   const Point3F shear( m[MatrixF::idx(3, 0)], m[MatrixF::idx(3, 1)], m[MatrixF::idx(3, 2)] );

   const Point3F row0(m[MatrixF::idx(0, 0)], m[MatrixF::idx(0, 1)], m[MatrixF::idx(0, 2)]);
   const Point3F row1(m[MatrixF::idx(1, 0)], m[MatrixF::idx(1, 1)], m[MatrixF::idx(1, 2)]);
   const Point3F row2(m[MatrixF::idx(2, 0)], m[MatrixF::idx(2, 1)], m[MatrixF::idx(2, 2)]);

   const F32 A = -mDot(row0, shear);
   const F32 B = -mDot(row1, shear);
   const F32 C = -mDot(row2, shear);

   MatrixF invTrMatrix(true);
   F32* destMat = invTrMatrix;
   destMat[MatrixF::idx(0, 0)] = m[MatrixF::idx(0, 0)];
   destMat[MatrixF::idx(1, 0)] = m[MatrixF::idx(1, 0)];
   destMat[MatrixF::idx(2, 0)] = m[MatrixF::idx(2, 0)];
   destMat[MatrixF::idx(0, 1)] = m[MatrixF::idx(0, 1)];
   destMat[MatrixF::idx(1, 1)] = m[MatrixF::idx(1, 1)];
   destMat[MatrixF::idx(2, 1)] = m[MatrixF::idx(2, 1)];
   destMat[MatrixF::idx(0, 2)] = m[MatrixF::idx(0, 2)];
   destMat[MatrixF::idx(1, 2)] = m[MatrixF::idx(1, 2)];
   destMat[MatrixF::idx(2, 2)] = m[MatrixF::idx(2, 2)];
   destMat[MatrixF::idx(0, 3)] = A;
   destMat[MatrixF::idx(1, 3)] = B;
   destMat[MatrixF::idx(2, 3)] = C;
   invTrMatrix.mul(invScale);

   Point3F norm(p[0], p[1], p[2]);
   Point3F point = norm * -p[3];
   invTrMatrix.mulP(norm);
   norm.normalize();

   MatrixF temp;
   dMemcpy(temp, m, sizeof(F32) * 16);
   point.x *= s[0];
   point.y *= s[1];
   point.z *= s[2];
   temp.mulP(point);

   PlaneF resultPlane(point, norm);
   presult[0] = resultPlane.x;
   presult[1] = resultPlane.y;
   presult[2] = resultPlane.z;
   presult[3] = resultPlane.d;
}

static void m_matF_x_box3F_C(const F32 *m, F32* min, F32* max)
{
   // Algorithm for axis aligned bounding box adapted from
   //  Graphic Gems I, pp 548-550
   //
   F32 originalMin[3];
   F32 originalMax[3];
   originalMin[0] = min[0];
   originalMin[1] = min[1];
   originalMin[2] = min[2];
   originalMax[0] = max[0];
   originalMax[1] = max[1];
   originalMax[2] = max[2];

   min[0] = max[0] = m[3];
   min[1] = max[1] = m[7];
   min[2] = max[2] = m[11];

   const F32 * row = &m[0];
   for (U32 i = 0; i < 3; i++)
   {
      #define  Do_One_Row(j)   {                         \
         F32    a = (row[j] * originalMin[j]);           \
         F32    b = (row[j] * originalMax[j]);           \
         if (a < b) { *min += a;  *max += b; }           \
         else       { *min += b;  *max += a; }     }

      // Simpler addressing (avoiding things like [ecx+edi*4]) might be worthwhile (LH):
      Do_One_Row(0);
      Do_One_Row(1);
      Do_One_Row(2);
      row += 4;
      min++;
      max++;
   }
}


void m_point3F_bulk_dot_C(const F32* refVector,
                          const F32* dotPoints,
                          const U32  numPoints,
                          const U32  pointStride,
                          F32*       output)
{
   for (U32 i = 0; i < numPoints; i++)
   {
      F32* pPoint = (F32*)(((U8*)dotPoints) + (pointStride * i));
      output[i] = ((refVector[0] * pPoint[0]) +
                   (refVector[1] * pPoint[1]) +
                   (refVector[2] * pPoint[2]));
   }
}

void m_point3F_bulk_dot_indexed_C(const F32* refVector,
                                  const F32* dotPoints,
                                  const U32  numPoints,
                                  const U32  pointStride,
                                  const U32* pointIndices,
                                  F32*       output)
{
   for (U32 i = 0; i < numPoints; i++)
   {
      F32* pPoint = (F32*)(((U8*)dotPoints) + (pointStride * pointIndices[i]));
      output[i] = ((refVector[0] * pPoint[0]) +
                   (refVector[1] * pPoint[1]) +
                   (refVector[2] * pPoint[2]));
   }
}

//------------------------------------------------------------------------------
// Math function pointer declarations

S32  (*m_mulDivS32)(S32 a, S32 b, S32 c) = m_mulDivS32_C;
U32  (*m_mulDivU32)(S32 a, S32 b, U32 c) = m_mulDivU32_C;

F32  (*m_catmullrom)(F32 t, F32 p0, F32 p1, F32 p2, F32 p3) = m_catmullrom_C;

void (*m_sincos)( F32 angle, F32 *s, F32 *c ) = m_sincos_C;
void (*m_sincosD)( F64 angle, F64 *s, F64 *c ) = m_sincosD_C;

void (*m_point2F_normalize)(F32 *p) = m_point2F_normalize_C;
void (*m_point2F_normalize_f)(F32 *p, F32 val) = m_point2F_normalize_f_C;
void (*m_point2D_normalize)(F64 *p) = m_point2D_normalize_C;
void (*m_point2D_normalize_f)(F64 *p, F64 val) = m_point2D_normalize_f_C;
void (*m_point3D_normalize_f)(F64 *p, F64 val) = m_point3D_normalize_f_C;
void (*m_point3F_normalize)(F32 *p) = m_point3F_normalize_C;
void (*m_point3F_normalize_f)(F32 *p, F32 len) = m_point3F_normalize_f_C;
void (*m_point3F_interpolate)(const F32 *from, const F32 *to, F32 factor, F32 *result) = m_point3F_interpolate_C;

void (*m_point3D_normalize)(F64 *p) = m_point3D_normalize_C;
void (*m_point3D_interpolate)(const F64 *from, const F64 *to, F64 factor, F64 *result) = m_point3D_interpolate_C;

void (*m_point3F_bulk_dot)(const F32* refVector,
                           const F32* dotPoints,
                           const U32  numPoints,
                           const U32  pointStride,
                           F32*       output) = m_point3F_bulk_dot_C;
void (*m_point3F_bulk_dot_indexed)(const F32* refVector,
                                   const F32* dotPoints,
                                   const U32  numPoints,
                                   const U32  pointStride,
                                   const U32* pointIndices,
                                   F32*       output) = m_point3F_bulk_dot_indexed_C;

void (*m_quatF_set_matF)( F32 x, F32 y, F32 z, F32 w, F32* m ) = m_quatF_set_matF_C;

void (*m_matF_set_euler)(const F32 *e, F32 *result) = m_matF_set_euler_C;
void (*m_matF_set_euler_point)(const F32 *e, const F32 *p, F32 *result) = m_matF_set_euler_point_C;
void (*m_matF_identity)(F32 *m)  = m_matF_identity_C;
void (*m_matF_inverse)(F32 *m)   = m_matF_inverse_C;
void (*m_matF_affineInverse)(F32 *m)   = m_matF_affineInverse_C;
void (*m_matF_invert_to)(const F32 *m, F32 *d) = m_matF_invert_to_C;
void (*m_matF_transpose)(F32 *m) = m_matF_transpose_C;
void (*m_matF_scale)(F32 *m,const F32* p) = m_matF_scale_C;
void (*m_matF_normalize)(F32 *m) = m_matF_normalize_C;
F32  (*m_matF_determinant)(const F32 *m) = m_matF_determinant_C;
void (*m_matF_x_matF)(const F32 *a, const F32 *b, F32 *mresult)    = default_matF_x_matF_C;
void (*m_matF_x_matF_aligned)(const F32 *a, const F32 *b, F32 *mresult)    = default_matF_x_matF_C;
// void (*m_matF_x_point3F)(const F32 *m, const F32 *p, F32 *presult) = m_matF_x_point3F_C;
// void (*m_matF_x_vectorF)(const F32 *m, const F32 *v, F32 *vresult) = m_matF_x_vectorF_C;
void (*m_matF_x_point4F)(const F32 *m, const F32 *p, F32 *presult) = m_matF_x_point4F_C;
void (*m_matF_x_scale_x_planeF)(const F32 *m, const F32* s, const F32 *p, F32 *presult) = m_matF_x_scale_x_planeF_C;
void (*m_matF_x_box3F)(const F32 *m, F32 *min, F32 *max)    = m_matF_x_box3F_C;

//------------------------------------------------------------------------------
void mInstallLibrary_C()
{
   m_mulDivS32             = m_mulDivS32_C;
   m_mulDivU32             = m_mulDivU32_C;

   m_catmullrom            = m_catmullrom_C;

   m_sincos = m_sincos_C;
   m_sincosD = m_sincosD_C;

   m_point2F_normalize     = m_point2F_normalize_C;
   m_point2F_normalize_f   = m_point2F_normalize_f_C;
   m_point2D_normalize     = m_point2D_normalize_C;
   m_point2D_normalize_f   = m_point2D_normalize_f_C;
   m_point3F_normalize     = m_point3F_normalize_C;
   m_point3F_normalize_f   = m_point3F_normalize_f_C;
   m_point3F_interpolate   = m_point3F_interpolate_C;

   m_point3D_normalize     = m_point3D_normalize_C;
   m_point3D_interpolate   = m_point3D_interpolate_C;

   m_point3F_bulk_dot      = m_point3F_bulk_dot_C;
   m_point3F_bulk_dot_indexed = m_point3F_bulk_dot_indexed_C;

   m_quatF_set_matF        = m_quatF_set_matF_C;

   m_matF_set_euler        = m_matF_set_euler_C;
   m_matF_set_euler_point  = m_matF_set_euler_point_C;
   m_matF_identity         = m_matF_identity_C;
   m_matF_inverse          = m_matF_inverse_C;
   m_matF_affineInverse    = m_matF_affineInverse_C;
   m_matF_invert_to        = m_matF_invert_to_C;
   m_matF_transpose        = m_matF_transpose_C;
   m_matF_scale            = m_matF_scale_C;
   m_matF_normalize        = m_matF_normalize_C;
   m_matF_determinant      = m_matF_determinant_C;
   m_matF_x_matF           = default_matF_x_matF_C;
   m_matF_x_matF_aligned   = default_matF_x_matF_C;
//    m_matF_x_point3F        = m_matF_x_point3F_C;
//    m_matF_x_vectorF        = m_matF_x_vectorF_C;
   m_matF_x_point4F        = m_matF_x_point4F_C;
   m_matF_x_scale_x_planeF = m_matF_x_scale_x_planeF_C;
   m_matF_x_box3F          = m_matF_x_box3F_C;
}

