// a set of routines that let you do common 3d math
// operations without any vector, matrix, or quaternion
// classes or templates.
//
// a vector (or point) is a 'NxF32 *' to 3 floating point numbers.
// a matrix is a 'NxF32 *' to an array of 16 floating point numbers representing a 4x4 transformation matrix compatible with D3D or OGL
// a quaternion is a 'NxF32 *' to 4 floats representing a quaternion x,y,z,w
//
//
/*!
**
** Copyright (c) 2009 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
** Portions of this source has been released with the PhysXViewer application, as well as
** Rocket, CreateDynamics, ODF, and as a number of sample code snippets.
**
** If you find this code useful or you are feeling particularily generous I would
** ask that you please go to http://www.amillionpixels.us and make a donation
** to Troy DeMolay.
**
** DeMolay is a youth group for young men between the ages of 12 and 21.
** It teaches strong moral principles, as well as leadership skills and
** public speaking.  The donations page uses the 'pay for pixels' paradigm
** where, in this case, a pixel is only a single penny.  Donations can be
** made for as small as $4 or as high as a $100 block.  Each person who donates
** will get a link to their own site as well as acknowledgement on the
** donations blog located here http://www.amillionpixels.blogspot.com/
**
** If you wish to contact me you can use the following methods:
**
** Skype ID: jratcliff63367
** Yahoo: jratcliff63367
** AOL: jratcliff1961
** email: jratcliffscarab@gmail.com
**
**
** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#pragma warning(disable:4996)

#include "NvUserMemAlloc.h"
#include "NvHashMap.h"

namespace CONVEX_DECOMPOSITION
{

void fm_inverseRT(const REAL matrix[16],const REAL pos[3],REAL t[3]) // inverse rotate translate the point.
{

	REAL _x = pos[0] - matrix[3*4+0];
	REAL _y = pos[1] - matrix[3*4+1];
	REAL _z = pos[2] - matrix[3*4+2];

	// Multiply inverse-translated source vector by inverted rotation transform

	t[0] = (matrix[0*4+0] * _x) + (matrix[0*4+1] * _y) + (matrix[0*4+2] * _z);
	t[1] = (matrix[1*4+0] * _x) + (matrix[1*4+1] * _y) + (matrix[1*4+2] * _z);
	t[2] = (matrix[2*4+0] * _x) + (matrix[2*4+1] * _y) + (matrix[2*4+2] * _z);

}

REAL fm_getDeterminant(const REAL matrix[16])
{
  REAL tempv[3];
  REAL p0[3];
  REAL p1[3];
  REAL p2[3];


	p0[0] = matrix[0*4+0];
	p0[1] = matrix[0*4+1];
	p0[2] = matrix[0*4+2];

	p1[0] = matrix[1*4+0];
	p1[1] = matrix[1*4+1];
	p1[2] = matrix[1*4+2];

	p2[0] = matrix[2*4+0];
	p2[1] = matrix[2*4+1];
	p2[2] = matrix[2*4+2];

  fm_cross(tempv,p1,p2);

  return fm_dot(p0,tempv);

}

REAL fm_squared(REAL x) { return x*x; };

void fm_decomposeTransform(const REAL local_transform[16],REAL trans[3],REAL rot[4],REAL scale[3])
{

  trans[0] = local_transform[12];
  trans[1] = local_transform[13];
  trans[2] = local_transform[14];

  scale[0] = sqrt(fm_squared(local_transform[0*4+0]) + fm_squared(local_transform[0*4+1]) + fm_squared(local_transform[0*4+2]));
  scale[1] = sqrt(fm_squared(local_transform[1*4+0]) + fm_squared(local_transform[1*4+1]) + fm_squared(local_transform[1*4+2]));
  scale[2] = sqrt(fm_squared(local_transform[2*4+0]) + fm_squared(local_transform[2*4+1]) + fm_squared(local_transform[2*4+2]));

  REAL m[16];
  memcpy(m,local_transform,sizeof(REAL)*16);

  REAL sx = 1.0f / scale[0];
  REAL sy = 1.0f / scale[1];
  REAL sz = 1.0f / scale[2];

  m[0*4+0]*=sx;
  m[0*4+1]*=sx;
  m[0*4+2]*=sx;

  m[1*4+0]*=sy;
  m[1*4+1]*=sy;
  m[1*4+2]*=sy;

  m[2*4+0]*=sz;
  m[2*4+1]*=sz;
  m[2*4+2]*=sz;

  fm_matrixToQuat(m,rot);

}

void fm_getSubMatrix(NxI32 ki,NxI32 kj,REAL pDst[16],const REAL matrix[16])
{
	NxI32 row, col;
	NxI32 dstCol = 0, dstRow = 0;

	for ( col = 0; col < 4; col++ )
	{
		if ( col == kj )
		{
			continue;
		}
		for ( dstRow = 0, row = 0; row < 4; row++ )
		{
			if ( row == ki )
			{
				continue;
			}
			pDst[dstCol*4+dstRow] = matrix[col*4+row];
			dstRow++;
		}
		dstCol++;
	}
}

void  fm_inverseTransform(const REAL matrix[16],REAL inverse_matrix[16])
{
	REAL determinant = fm_getDeterminant(matrix);
	determinant = 1.0f / determinant;
	for (NxI32 i = 0; i < 4; i++ )
	{
		for (NxI32 j = 0; j < 4; j++ )
		{
			NxI32 sign = 1 - ( ( i + j ) % 2 ) * 2;
			REAL subMat[16];
      fm_identity(subMat);
			fm_getSubMatrix( i, j, subMat, matrix );
			REAL subDeterminant = fm_getDeterminant(subMat);
			inverse_matrix[i*4+j] = ( subDeterminant * sign ) * determinant;
		}
	}
}

void fm_identity(REAL matrix[16]) // set 4x4 matrix to identity.
{
	matrix[0*4+0] = 1;
	matrix[1*4+1] = 1;
	matrix[2*4+2] = 1;
	matrix[3*4+3] = 1;

	matrix[1*4+0] = 0;
	matrix[2*4+0] = 0;
	matrix[3*4+0] = 0;

	matrix[0*4+1] = 0;
	matrix[2*4+1] = 0;
	matrix[3*4+1] = 0;

	matrix[0*4+2] = 0;
	matrix[1*4+2] = 0;
	matrix[3*4+2] = 0;

	matrix[0*4+3] = 0;
	matrix[1*4+3] = 0;
	matrix[2*4+3] = 0;

}

void  fm_quatToEuler(const REAL quat[4],REAL &ax,REAL &ay,REAL &az)
{
  REAL x = quat[0];
  REAL y = quat[1];
  REAL z = quat[2];
  REAL w = quat[3];

	REAL sint	     = (2.0f * w * y) - (2.0f * x * z);
	REAL cost_temp = 1.0f - (sint * sint);
	REAL cost	   	 = 0;

	if ( (REAL)fabs(cost_temp) > 0.001f )
	{
		cost = sqrt( cost_temp );
	}

	REAL sinv, cosv, sinf, cosf;
	if ( (REAL)fabs(cost) > 0.001f )
	{
    cost = 1.0f / cost;
		sinv = ((2.0f * y * z) + (2.0f * w * x)) * cost;
		cosv = (1.0f - (2.0f * x * x) - (2.0f * y * y)) * cost;
		sinf = ((2.0f * x * y) + (2.0f * w * z)) * cost;
		cosf = (1.0f - (2.0f * y * y) - (2.0f * z * z)) * cost;
	}
	else
	{
		sinv = (2.0f * w * x) - (2.0f * y * z);
		cosv = 1.0f - (2.0f * x * x) - (2.0f * z * z);
		sinf = 0;
		cosf = 1.0f;
	}

	// compute output rotations
	ax	= atan2( sinv, cosv );
	ay	= atan2( sint, cost );
  az	= atan2( sinf, cosf );

}

void fm_eulerToMatrix(REAL ax,REAL ay,REAL az,REAL *matrix) // convert euler (in radians) to a dest 4x4 matrix (translation set to zero)
{
  REAL quat[4];
  fm_eulerToQuat(ax,ay,az,quat);
  fm_quatToMatrix(quat,matrix);
}

void fm_getAABB(NxU32 vcount,const REAL *points,NxU32 pstride,REAL *bmin,REAL *bmax)
{

  const NxU8 *source = (const NxU8 *) points;

	bmin[0] = points[0];
	bmin[1] = points[1];
	bmin[2] = points[2];

	bmax[0] = points[0];
	bmax[1] = points[1];
	bmax[2] = points[2];


  for (NxU32 i=1; i<vcount; i++)
  {
  	source+=pstride;
  	const REAL *p = (const REAL *) source;

  	if ( p[0] < bmin[0] ) bmin[0] = p[0];
  	if ( p[1] < bmin[1] ) bmin[1] = p[1];
  	if ( p[2] < bmin[2] ) bmin[2] = p[2];

		if ( p[0] > bmax[0] ) bmax[0] = p[0];
		if ( p[1] > bmax[1] ) bmax[1] = p[1];
		if ( p[2] > bmax[2] ) bmax[2] = p[2];

  }
}

void  fm_eulerToQuat(const REAL *euler,REAL *quat) // convert euler angles to quaternion.
{
  fm_eulerToQuat(euler[0],euler[1],euler[2],quat);
}

void fm_eulerToQuat(REAL roll,REAL pitch,REAL yaw,REAL *quat) // convert euler angles to quaternion.
{
	roll  *= 0.5f;
	pitch *= 0.5f;
	yaw   *= 0.5f;

	REAL cr = cos(roll);
	REAL cp = cos(pitch);
	REAL cy = cos(yaw);

	REAL sr = sin(roll);
	REAL sp = sin(pitch);
	REAL sy = sin(yaw);

	REAL cpcy = cp * cy;
	REAL spsy = sp * sy;
	REAL spcy = sp * cy;
	REAL cpsy = cp * sy;

	quat[0]   = ( sr * cpcy - cr * spsy);
	quat[1]   = ( cr * spcy + sr * cpsy);
	quat[2]   = ( cr * cpsy - sr * spcy);
	quat[3]   = cr * cpcy + sr * spsy;
}

void fm_quatToMatrix(const REAL *quat,REAL *matrix) // convert quaterinion rotation to matrix, zeros out the translation component.
{

	REAL xx = quat[0]*quat[0];
	REAL yy = quat[1]*quat[1];
	REAL zz = quat[2]*quat[2];
	REAL xy = quat[0]*quat[1];
	REAL xz = quat[0]*quat[2];
	REAL yz = quat[1]*quat[2];
	REAL wx = quat[3]*quat[0];
	REAL wy = quat[3]*quat[1];
	REAL wz = quat[3]*quat[2];

	matrix[0*4+0] = 1 - 2 * ( yy + zz );
	matrix[1*4+0] =     2 * ( xy - wz );
	matrix[2*4+0] =     2 * ( xz + wy );

	matrix[0*4+1] =     2 * ( xy + wz );
	matrix[1*4+1] = 1 - 2 * ( xx + zz );
	matrix[2*4+1] =     2 * ( yz - wx );

	matrix[0*4+2] =     2 * ( xz - wy );
	matrix[1*4+2] =     2 * ( yz + wx );
	matrix[2*4+2] = 1 - 2 * ( xx + yy );

	matrix[3*4+0] = matrix[3*4+1] = matrix[3*4+2] = (REAL) 0.0f;
	matrix[0*4+3] = matrix[1*4+3] = matrix[2*4+3] = (REAL) 0.0f;
	matrix[3*4+3] =(REAL) 1.0f;

}


void fm_quatRotate(const REAL *quat,const REAL *v,REAL *r) // rotate a vector directly by a quaternion.
{
  REAL left[4];

	left[0] =   quat[3]*v[0] + quat[1]*v[2] - v[1]*quat[2];
	left[1] =   quat[3]*v[1] + quat[2]*v[0] - v[2]*quat[0];
	left[2] =   quat[3]*v[2] + quat[0]*v[1] - v[0]*quat[1];
	left[3] = - quat[0]*v[0] - quat[1]*v[1] - quat[2]*v[2];

	r[0] = (left[3]*-quat[0]) + (quat[3]*left[0]) + (left[1]*-quat[2]) - (-quat[1]*left[2]);
	r[1] = (left[3]*-quat[1]) + (quat[3]*left[1]) + (left[2]*-quat[0]) - (-quat[2]*left[0]);
	r[2] = (left[3]*-quat[2]) + (quat[3]*left[2]) + (left[0]*-quat[1]) - (-quat[0]*left[1]);

}


void fm_getTranslation(const REAL *matrix,REAL *t)
{
	t[0] = matrix[3*4+0];
	t[1] = matrix[3*4+1];
	t[2] = matrix[3*4+2];
}

void fm_matrixToQuat(const REAL *matrix,REAL *quat) // convert the 3x3 portion of a 4x4 matrix into a quaterion as x,y,z,w
{

	REAL tr = matrix[0*4+0] + matrix[1*4+1] + matrix[2*4+2];

	// check the diagonal

	if (tr > 0.0f )
	{
		REAL s = (REAL) sqrt ( (NxF64) (tr + 1.0f) );
		quat[3] = s * 0.5f;
		s = 0.5f / s;
		quat[0] = (matrix[1*4+2] - matrix[2*4+1]) * s;
		quat[1] = (matrix[2*4+0] - matrix[0*4+2]) * s;
		quat[2] = (matrix[0*4+1] - matrix[1*4+0]) * s;

	}
	else
	{
		// diagonal is negative
		NxI32 nxt[3] = {1, 2, 0};
		REAL  qa[4];

		NxI32 i = 0;

		if (matrix[1*4+1] > matrix[0*4+0]) i = 1;
		if (matrix[2*4+2] > matrix[i*4+i]) i = 2;

		NxI32 j = nxt[i];
		NxI32 k = nxt[j];

		REAL s = sqrt ( ((matrix[i*4+i] - (matrix[j*4+j] + matrix[k*4+k])) + 1.0f) );

		qa[i] = s * 0.5f;

		if (s != 0.0f ) s = 0.5f / s;

		qa[3] = (matrix[j*4+k] - matrix[k*4+j]) * s;
		qa[j] = (matrix[i*4+j] + matrix[j*4+i]) * s;
		qa[k] = (matrix[i*4+k] + matrix[k*4+i]) * s;

		quat[0] = qa[0];
		quat[1] = qa[1];
		quat[2] = qa[2];
		quat[3] = qa[3];
	}


}


REAL fm_sphereVolume(REAL radius) // return's the volume of a sphere of this radius (4/3 PI * R cubed )
{
	return (4.0f / 3.0f ) * FM_PI * radius * radius * radius;
}


REAL fm_cylinderVolume(REAL radius,REAL h)
{
	return FM_PI * radius * radius *h;
}

REAL fm_capsuleVolume(REAL radius,REAL h)
{
	REAL volume = fm_sphereVolume(radius); // volume of the sphere portion.
	REAL ch = h-radius*2; // this is the cylinder length
	if ( ch > 0 )
	{
		volume+=fm_cylinderVolume(radius,ch);
	}
	return volume;
}

void  fm_transform(const REAL matrix[16],const REAL v[3],REAL t[3]) // rotate and translate this point
{
  if ( matrix )
  {
    REAL tx = (matrix[0*4+0] * v[0]) +  (matrix[1*4+0] * v[1]) + (matrix[2*4+0] * v[2]) + matrix[3*4+0];
    REAL ty = (matrix[0*4+1] * v[0]) +  (matrix[1*4+1] * v[1]) + (matrix[2*4+1] * v[2]) + matrix[3*4+1];
    REAL tz = (matrix[0*4+2] * v[0]) +  (matrix[1*4+2] * v[1]) + (matrix[2*4+2] * v[2]) + matrix[3*4+2];
    t[0] = tx;
    t[1] = ty;
    t[2] = tz;
  }
  else
  {
    t[0] = v[0];
    t[1] = v[1];
    t[2] = v[2];
  }
}

void  fm_rotate(const REAL matrix[16],const REAL v[3],REAL t[3]) // rotate and translate this point
{
  if ( matrix )
  {
    REAL tx = (matrix[0*4+0] * v[0]) +  (matrix[1*4+0] * v[1]) + (matrix[2*4+0] * v[2]);
    REAL ty = (matrix[0*4+1] * v[0]) +  (matrix[1*4+1] * v[1]) + (matrix[2*4+1] * v[2]);
    REAL tz = (matrix[0*4+2] * v[0]) +  (matrix[1*4+2] * v[1]) + (matrix[2*4+2] * v[2]);
    t[0] = tx;
    t[1] = ty;
    t[2] = tz;
  }
  else
  {
    t[0] = v[0];
    t[1] = v[1];
    t[2] = v[2];
  }
}


REAL fm_distance(const REAL *p1,const REAL *p2)
{
	REAL dx = p1[0] - p2[0];
	REAL dy = p1[1] - p2[1];
	REAL dz = p1[2] - p2[2];

	return sqrt( dx*dx + dy*dy + dz *dz );
}

REAL fm_distanceSquared(const REAL *p1,const REAL *p2)
{
	REAL dx = p1[0] - p2[0];
	REAL dy = p1[1] - p2[1];
	REAL dz = p1[2] - p2[2];

	return dx*dx + dy*dy + dz *dz;
}


REAL fm_distanceSquaredXZ(const REAL *p1,const REAL *p2)
{
	REAL dx = p1[0] - p2[0];
	REAL dz = p1[2] - p2[2];

	return dx*dx +  dz *dz;
}


REAL fm_computePlane(const REAL *A,const REAL *B,const REAL *C,REAL *n) // returns D
{
	REAL vx = (B[0] - C[0]);
	REAL vy = (B[1] - C[1]);
	REAL vz = (B[2] - C[2]);

	REAL wx = (A[0] - B[0]);
	REAL wy = (A[1] - B[1]);
	REAL wz = (A[2] - B[2]);

	REAL vw_x = vy * wz - vz * wy;
	REAL vw_y = vz * wx - vx * wz;
	REAL vw_z = vx * wy - vy * wx;

	REAL mag = sqrt((vw_x * vw_x) + (vw_y * vw_y) + (vw_z * vw_z));

	if ( mag < 0.000001f )
	{
		mag = 0;
	}
	else
	{
		mag = 1.0f/mag;
	}

	REAL x = vw_x * mag;
	REAL y = vw_y * mag;
	REAL z = vw_z * mag;


	REAL D = 0.0f - ((x*A[0])+(y*A[1])+(z*A[2]));

  n[0] = x;
  n[1] = y;
  n[2] = z;

	return D;
}

REAL fm_distToPlane(const REAL *plane,const REAL *p) // computes the distance of this point from the plane.
{
  return p[0]*plane[0]+p[1]*plane[1]+p[2]*plane[2]+plane[3];
}

REAL fm_dot(const REAL *p1,const REAL *p2)
{
  return p1[0]*p2[0]+p1[1]*p2[1]+p1[2]*p2[2];
}

void fm_cross(REAL *cross,const REAL *a,const REAL *b)
{
	cross[0] = a[1]*b[2] - a[2]*b[1];
	cross[1] = a[2]*b[0] - a[0]*b[2];
	cross[2] = a[0]*b[1] - a[1]*b[0];
}

void fm_computeNormalVector(REAL *n,const REAL *p1,const REAL *p2)
{
  n[0] = p2[0] - p1[0];
  n[1] = p2[1] - p1[1];
  n[2] = p2[2] - p1[2];
  fm_normalize(n);
}

bool  fm_computeWindingOrder(const REAL *p1,const REAL *p2,const REAL *p3) // returns true if the triangle is clockwise.
{
  bool ret = false;

  REAL v1[3];
  REAL v2[3];

  fm_computeNormalVector(v1,p1,p2); // p2-p1 (as vector) and then normalized
  fm_computeNormalVector(v2,p1,p3); // p3-p1 (as vector) and then normalized

  REAL cross[3];

  fm_cross(cross, v1, v2 );
  REAL ref[3] = { 1, 0, 0 };

  REAL d = fm_dot( cross, ref );


  if ( d <= 0 )
    ret = false;
  else
    ret = true;

  return ret;
}

REAL fm_normalize(REAL *n) // normalize this vector
{
  REAL dist = (REAL)sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
  if ( dist > 0.0000001f )
  {
    REAL mag = 1.0f / dist;
    n[0]*=mag;
    n[1]*=mag;
    n[2]*=mag;
  }
  else
  {
    n[0] = 1;
    n[1] = 0;
    n[2] = 0;
  }

  return dist;
}


void  fm_matrixMultiply(const REAL *pA,const REAL *pB,REAL *pM)
{
#if 1

  REAL a = pA[0*4+0] * pB[0*4+0] + pA[0*4+1] * pB[1*4+0] + pA[0*4+2] * pB[2*4+0] + pA[0*4+3] * pB[3*4+0];
  REAL b = pA[0*4+0] * pB[0*4+1] + pA[0*4+1] * pB[1*4+1] + pA[0*4+2] * pB[2*4+1] + pA[0*4+3] * pB[3*4+1];
  REAL c = pA[0*4+0] * pB[0*4+2] + pA[0*4+1] * pB[1*4+2] + pA[0*4+2] * pB[2*4+2] + pA[0*4+3] * pB[3*4+2];
  REAL d = pA[0*4+0] * pB[0*4+3] + pA[0*4+1] * pB[1*4+3] + pA[0*4+2] * pB[2*4+3] + pA[0*4+3] * pB[3*4+3];

  REAL e = pA[1*4+0] * pB[0*4+0] + pA[1*4+1] * pB[1*4+0] + pA[1*4+2] * pB[2*4+0] + pA[1*4+3] * pB[3*4+0];
  REAL f = pA[1*4+0] * pB[0*4+1] + pA[1*4+1] * pB[1*4+1] + pA[1*4+2] * pB[2*4+1] + pA[1*4+3] * pB[3*4+1];
  REAL g = pA[1*4+0] * pB[0*4+2] + pA[1*4+1] * pB[1*4+2] + pA[1*4+2] * pB[2*4+2] + pA[1*4+3] * pB[3*4+2];
  REAL h = pA[1*4+0] * pB[0*4+3] + pA[1*4+1] * pB[1*4+3] + pA[1*4+2] * pB[2*4+3] + pA[1*4+3] * pB[3*4+3];

  REAL i = pA[2*4+0] * pB[0*4+0] + pA[2*4+1] * pB[1*4+0] + pA[2*4+2] * pB[2*4+0] + pA[2*4+3] * pB[3*4+0];
  REAL j = pA[2*4+0] * pB[0*4+1] + pA[2*4+1] * pB[1*4+1] + pA[2*4+2] * pB[2*4+1] + pA[2*4+3] * pB[3*4+1];
  REAL k = pA[2*4+0] * pB[0*4+2] + pA[2*4+1] * pB[1*4+2] + pA[2*4+2] * pB[2*4+2] + pA[2*4+3] * pB[3*4+2];
  REAL l = pA[2*4+0] * pB[0*4+3] + pA[2*4+1] * pB[1*4+3] + pA[2*4+2] * pB[2*4+3] + pA[2*4+3] * pB[3*4+3];

  REAL m = pA[3*4+0] * pB[0*4+0] + pA[3*4+1] * pB[1*4+0] + pA[3*4+2] * pB[2*4+0] + pA[3*4+3] * pB[3*4+0];
  REAL n = pA[3*4+0] * pB[0*4+1] + pA[3*4+1] * pB[1*4+1] + pA[3*4+2] * pB[2*4+1] + pA[3*4+3] * pB[3*4+1];
  REAL o = pA[3*4+0] * pB[0*4+2] + pA[3*4+1] * pB[1*4+2] + pA[3*4+2] * pB[2*4+2] + pA[3*4+3] * pB[3*4+2];
  REAL p = pA[3*4+0] * pB[0*4+3] + pA[3*4+1] * pB[1*4+3] + pA[3*4+2] * pB[2*4+3] + pA[3*4+3] * pB[3*4+3];

  pM[0] = a;
  pM[1] = b;
  pM[2] = c;
  pM[3] = d;

  pM[4] = e;
  pM[5] = f;
  pM[6] = g;
  pM[7] = h;

  pM[8] = i;
  pM[9] = j;
  pM[10] = k;
  pM[11] = l;

  pM[12] = m;
  pM[13] = n;
  pM[14] = o;
  pM[15] = p;


#else
	memset(pM, 0, sizeof(REAL)*16);
	for(NxI32 i=0; i<4; i++ )
		for(NxI32 j=0; j<4; j++ )
			for(NxI32 k=0; k<4; k++ )
				pM[4*i+j] +=  pA[4*i+k] * pB[4*k+j];
#endif
}


void  fm_eulerToQuatDX(REAL x,REAL y,REAL z,REAL *quat) // convert euler angles to quaternion using the fucked up DirectX method
{
  REAL matrix[16];
  fm_eulerToMatrix(x,y,z,matrix);
  fm_matrixToQuat(matrix,quat);
}

// implementation copied from: http://blogs.msdn.com/mikepelton/archive/2004/10/29/249501.aspx
void  fm_eulerToMatrixDX(REAL x,REAL y,REAL z,REAL *matrix) // convert euler angles to quaternion using the fucked up DirectX method.
{
  fm_identity(matrix);
  matrix[0*4+0] = cos(z)*cos(y) + sin(z)*sin(x)*sin(y);
  matrix[0*4+1] = sin(z)*cos(x);
  matrix[0*4+2] = cos(z)*-sin(y) + sin(z)*sin(x)*cos(y);

  matrix[1*4+0] = -sin(z)*cos(y)+cos(z)*sin(x)*sin(y);
  matrix[1*4+1] = cos(z)*cos(x);
  matrix[1*4+2] = sin(z)*sin(y) +cos(z)*sin(x)*cos(y);

  matrix[2*4+0] = cos(x)*sin(y);
  matrix[2*4+1] = -sin(x);
  matrix[2*4+2] = cos(x)*cos(y);
}


void  fm_scale(REAL x,REAL y,REAL z,REAL *fscale) // apply scale to the matrix.
{
  fscale[0*4+0] = x;
  fscale[1*4+1] = y;
  fscale[2*4+2] = z;
}


void  fm_composeTransform(const REAL *position,const REAL *quat,const REAL *scale,REAL *matrix)
{
  fm_identity(matrix);
  fm_quatToMatrix(quat,matrix);

  if ( scale && ( scale[0] != 1 || scale[1] != 1 || scale[2] != 1 ) )
  {
    REAL work[16];
    memcpy(work,matrix,sizeof(REAL)*16);
    REAL mscale[16];
    fm_identity(mscale);
    fm_scale(scale[0],scale[1],scale[2],mscale);
    fm_matrixMultiply(work,mscale,matrix);
  }

  matrix[12] = position[0];
  matrix[13] = position[1];
  matrix[14] = position[2];
}


void  fm_setTranslation(const REAL *translation,REAL *matrix)
{
  matrix[12] = translation[0];
  matrix[13] = translation[1];
  matrix[14] = translation[2];
}

static REAL enorm0_3d ( REAL x0, REAL y0, REAL z0, REAL x1, REAL y1, REAL z1 )

/**********************************************************************/

/*
Purpose:

ENORM0_3D computes the Euclidean norm of (P1-P0) in 3D.

Modified:

18 April 1999

Author:

John Burkardt

Parameters:

Input, REAL X0, Y0, Z0, X1, Y1, Z1, the coordinates of the points 
P0 and P1.

Output, REAL ENORM0_3D, the Euclidean norm of (P1-P0).
*/
{
  REAL value;

  value = sqrt (
    ( x1 - x0 ) * ( x1 - x0 ) + 
    ( y1 - y0 ) * ( y1 - y0 ) + 
    ( z1 - z0 ) * ( z1 - z0 ) );

  return value;
}


static REAL triangle_area_3d ( REAL x1, REAL y1, REAL z1, REAL x2,REAL y2, REAL z2, REAL x3, REAL y3, REAL z3 )

                        /**********************************************************************/

                        /*
                        Purpose:

                        TRIANGLE_AREA_3D computes the area of a triangle in 3D.

                        Modified:

                        22 April 1999

                        Author:

                        John Burkardt

                        Parameters:

                        Input, REAL X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, the (X,Y,Z)
                        coordinates of the corners of the triangle.

                        Output, REAL TRIANGLE_AREA_3D, the area of the triangle.
                        */
{
  REAL a;
  REAL alpha;
  REAL area;
  REAL b;
  REAL base;
  REAL c;
  REAL dot;
  REAL height;
  /*
  Find the projection of (P3-P1) onto (P2-P1).
  */
  dot = 
    ( x2 - x1 ) * ( x3 - x1 ) +
    ( y2 - y1 ) * ( y3 - y1 ) +
    ( z2 - z1 ) * ( z3 - z1 );

  base = enorm0_3d ( x1, y1, z1, x2, y2, z2 );
  /*
  The height of the triangle is the length of (P3-P1) after its
  projection onto (P2-P1) has been subtracted.
  */
  if ( base == 0.0 ) {

    height = 0.0;

  }
  else {

    alpha = dot / ( base * base );

    a = x3 - x1 - alpha * ( x2 - x1 );
    b = y3 - y1 - alpha * ( y2 - y1 );
    c = z3 - z1 - alpha * ( z2 - z1 );

    height = sqrt ( a * a + b * b + c * c );

  }

  area = 0.5f * base * height;

  return area;
}


REAL fm_computeArea(const REAL *p1,const REAL *p2,const REAL *p3)
{
  REAL ret = 0;

  ret = triangle_area_3d(p1[0],p1[1],p1[2],p2[0],p2[1],p2[2],p3[0],p3[1],p3[2]);

  return ret;
}


void  fm_lerp(const REAL *p1,const REAL *p2,REAL *dest,REAL lerpValue)
{
  dest[0] = ((p2[0] - p1[0])*lerpValue) + p1[0];
  dest[1] = ((p2[1] - p1[1])*lerpValue) + p1[1];
  dest[2] = ((p2[2] - p1[2])*lerpValue) + p1[2];
}

bool fm_pointTestXZ(const REAL *p,const REAL *i,const REAL *j)
{
  bool ret = false;

  if (((( i[2] <= p[2] ) && ( p[2]  < j[2] )) || (( j[2] <= p[2] ) && ( p[2]  < i[2] ))) && ( p[0] < (j[0] - i[0]) * (p[2] - i[2]) / (j[2] - i[2]) + i[0]))
    ret = true;

  return ret;
};


bool  fm_insideTriangleXZ(const REAL *p,const REAL *p1,const REAL *p2,const REAL *p3)
{
  bool ret = false;

  NxI32 c = 0;
  if ( fm_pointTestXZ(p,p1,p2) ) c = !c;
  if ( fm_pointTestXZ(p,p2,p3) ) c = !c;
  if ( fm_pointTestXZ(p,p3,p1) ) c = !c;
  if ( c ) ret = true;

  return ret;
}

bool  fm_insideAABB(const REAL *pos,const REAL *bmin,const REAL *bmax)
{
  bool ret = false;

  if ( pos[0] >= bmin[0] && pos[0] <= bmax[0] &&
       pos[1] >= bmin[1] && pos[1] <= bmax[1] &&
       pos[2] >= bmin[2] && pos[2] <= bmax[2] )
    ret = true;

  return ret;
}


NxU32 fm_clipTestPoint(const REAL *bmin,const REAL *bmax,const REAL *pos)
{
  NxU32 ret = 0;

  if ( pos[0] < bmin[0] )
    ret|=FMCS_XMIN;
  else if ( pos[0] > bmax[0] )
    ret|=FMCS_XMAX;

  if ( pos[1] < bmin[1] )
    ret|=FMCS_YMIN;
  else if ( pos[1] > bmax[1] )
    ret|=FMCS_YMAX;

  if ( pos[2] < bmin[2] )
    ret|=FMCS_ZMIN;
  else if ( pos[2] > bmax[2] )
    ret|=FMCS_ZMAX;

  return ret;
}

NxU32 fm_clipTestPointXZ(const REAL *bmin,const REAL *bmax,const REAL *pos) // only tests X and Z, not Y
{
  NxU32 ret = 0;

  if ( pos[0] < bmin[0] )
    ret|=FMCS_XMIN;
  else if ( pos[0] > bmax[0] )
    ret|=FMCS_XMAX;

  if ( pos[2] < bmin[2] )
    ret|=FMCS_ZMIN;
  else if ( pos[2] > bmax[2] )
    ret|=FMCS_ZMAX;

  return ret;
}

NxU32 fm_clipTestAABB(const REAL *bmin,const REAL *bmax,const REAL *p1,const REAL *p2,const REAL *p3,NxU32 &andCode)
{
  NxU32 orCode = 0;

  andCode = FMCS_XMIN | FMCS_XMAX | FMCS_YMIN | FMCS_YMAX | FMCS_ZMIN | FMCS_ZMAX;

  NxU32 c = fm_clipTestPoint(bmin,bmax,p1);
  orCode|=c;
  andCode&=c;

  c = fm_clipTestPoint(bmin,bmax,p2);
  orCode|=c;
  andCode&=c;

  c = fm_clipTestPoint(bmin,bmax,p3);
  orCode|=c;
  andCode&=c;

  return orCode;
}

bool intersect(const REAL *si,const REAL *ei,const REAL *bmin,const REAL *bmax,REAL *time)
{
  REAL st,et,fst = 0,fet = 1;

  for (NxI32 i = 0; i < 3; i++)
  {
    if (*si < *ei)
    {
      if (*si > *bmax || *ei < *bmin)
        return false;
      REAL di = *ei - *si;
      st = (*si < *bmin)? (*bmin - *si) / di: 0;
      et = (*ei > *bmax)? (*bmax - *si) / di: 1;
    }
    else
    {
      if (*ei > *bmax || *si < *bmin)
        return false;
      REAL di = *ei - *si;
      st = (*si > *bmax)? (*bmax - *si) / di: 0;
      et = (*ei < *bmin)? (*bmin - *si) / di: 1;
    }

    if (st > fst) fst = st;
    if (et < fet) fet = et;
    if (fet < fst)
      return false;
    bmin++; bmax++;
    si++; ei++;
  }

  *time = fst;
  return true;
}



bool fm_lineTestAABB(const REAL *p1,const REAL *p2,const REAL *bmin,const REAL *bmax,REAL &time)
{
  bool sect = intersect(p1,p2,bmin,bmax,&time);
  return sect;
}


bool fm_lineTestAABBXZ(const REAL *p1,const REAL *p2,const REAL *bmin,const REAL *bmax,REAL &time)
{
  REAL _bmin[3];
  REAL _bmax[3];

  _bmin[0] = bmin[0];
  _bmin[1] = -1e9;
  _bmin[2] = bmin[2];

  _bmax[0] = bmax[0];
  _bmax[1] = 1e9;
  _bmax[2] = bmax[2];

  bool sect = intersect(p1,p2,_bmin,_bmax,&time);

  return sect;
}

void  fm_minmax(const REAL *p,REAL *bmin,REAL *bmax) // accmulate to a min-max value
{

  if ( p[0] < bmin[0] ) bmin[0] = p[0];
  if ( p[1] < bmin[1] ) bmin[1] = p[1];
  if ( p[2] < bmin[2] ) bmin[2] = p[2];

  if ( p[0] > bmax[0] ) bmax[0] = p[0];
  if ( p[1] > bmax[1] ) bmax[1] = p[1];
  if ( p[2] > bmax[2] ) bmax[2] = p[2];

}

REAL fm_solveX(const REAL *plane,REAL y,REAL z) // solve for X given this plane equation and the other two components.
{
  REAL x = (y*plane[1]+z*plane[2]+plane[3]) / -plane[0];
  return x;
}

REAL fm_solveY(const REAL *plane,REAL x,REAL z) // solve for Y given this plane equation and the other two components.
{
  REAL y = (x*plane[0]+z*plane[2]+plane[3]) / -plane[1];
  return y;
}


REAL fm_solveZ(const REAL *plane,REAL x,REAL y) // solve for Y given this plane equation and the other two components.
{
  REAL z = (x*plane[0]+y*plane[1]+plane[3]) / -plane[2];
  return z;
}


void  fm_getAABBCenter(const REAL *bmin,const REAL *bmax,REAL *center)
{
  center[0] = (bmax[0]-bmin[0])*0.5f+bmin[0];
  center[1] = (bmax[1]-bmin[1])*0.5f+bmin[1];
  center[2] = (bmax[2]-bmin[2])*0.5f+bmin[2];
}

FM_Axis fm_getDominantAxis(const REAL normal[3])
{
  FM_Axis ret = FM_XAXIS;

  REAL x = fabs(normal[0]);
  REAL y = fabs(normal[1]);
  REAL z = fabs(normal[2]);

  if ( y > x && y > z )
    ret = FM_YAXIS;
  else if ( z > x && z > y )
    ret = FM_ZAXIS;

  return ret;
}


bool fm_lineSphereIntersect(const REAL *center,REAL radius,const REAL *p1,const REAL *p2,REAL *intersect)
{
  bool ret = false;

  REAL dir[3];

  dir[0] = p2[0]-p1[0];
  dir[1] = p2[1]-p1[1];
  dir[2] = p2[2]-p1[2];

  REAL distance = sqrt( dir[0]*dir[0]+dir[1]*dir[1]+dir[2]*dir[2]);

  if ( distance > 0 )
  {
    REAL recip = 1.0f / distance;
    dir[0]*=recip;
    dir[1]*=recip;
    dir[2]*=recip;
    ret = fm_raySphereIntersect(center,radius,p1,dir,distance,intersect);
  }
  else
  {
    dir[0] = center[0]-p1[0];
    dir[1] = center[1]-p1[1];
    dir[2] = center[2]-p1[2];
    REAL d2 = dir[0]*dir[0]+dir[1]*dir[1]+dir[2]*dir[2];
    REAL r2 = radius*radius;
    if ( d2 < r2 )
    {
      ret = true;
      if ( intersect )
      {
        intersect[0] = p1[0];
        intersect[1] = p1[1];
        intersect[2] = p1[2];
      }
    }
  }
  return ret;
}

#define DOT(p1,p2) (p1[0]*p2[0]+p1[1]*p2[1]+p1[2]*p2[2])

bool fm_raySphereIntersect(const REAL *center,REAL radius,const REAL *pos,const REAL *dir,REAL distance,REAL *intersect)
{
  bool ret = false;

  REAL E0[3];

  E0[0] = center[0] - pos[0];
  E0[1] = center[1] - pos[1];
  E0[2] = center[2] - pos[2];

  REAL V[3];

  V[0]  = dir[0];
  V[1]  = dir[1];
  V[2]  = dir[2];


  REAL dist2   = E0[0]*E0[0] + E0[1]*E0[1] + E0[2] * E0[2];
  REAL radius2 = radius*radius; // radius squared..

  // Bug Fix For Gem, if origin is *inside* the sphere, invert the
  // direction vector so that we get a valid intersection location.
  if ( dist2 < radius2 )
  {
    V[0]*=-1;
    V[1]*=-1;
    V[2]*=-1;
  }


	REAL v = DOT(E0,V);

	REAL disc = radius2 - (dist2 - v*v);

	if (disc > 0.0f)
	{
		if ( intersect )
		{
		  REAL d = sqrt(disc);
      REAL diff = v-d;
      if ( diff < distance )
      {
        intersect[0] = pos[0]+V[0]*diff;
        intersect[1] = pos[1]+V[1]*diff;
        intersect[2] = pos[2]+V[2]*diff;
        ret = true;
      }
    }
	}

	return ret;
}


void fm_catmullRom(REAL *out_vector,const REAL *p1,const REAL *p2,const REAL *p3,const REAL *p4, const REAL s)
{
  REAL s_squared = s * s;
  REAL s_cubed = s_squared * s;

  REAL coefficient_p1 = -s_cubed + 2*s_squared - s;
  REAL coefficient_p2 = 3 * s_cubed - 5 * s_squared + 2;
  REAL coefficient_p3 = -3 * s_cubed +4 * s_squared + s;
  REAL coefficient_p4 = s_cubed - s_squared;

  out_vector[0] = (coefficient_p1 * p1[0] + coefficient_p2 * p2[0] + coefficient_p3 * p3[0] + coefficient_p4 * p4[0])*0.5f;
  out_vector[1] = (coefficient_p1 * p1[1] + coefficient_p2 * p2[1] + coefficient_p3 * p3[1] + coefficient_p4 * p4[1])*0.5f;
  out_vector[2] = (coefficient_p1 * p1[2] + coefficient_p2 * p2[2] + coefficient_p3 * p3[2] + coefficient_p4 * p4[2])*0.5f;
}

bool fm_intersectAABB(const REAL *bmin1,const REAL *bmax1,const REAL *bmin2,const REAL *bmax2)
{
  if ((bmin1[0] > bmax2[0]) || (bmin2[0] > bmax1[0])) return false;
  if ((bmin1[1] > bmax2[1]) || (bmin2[1] > bmax1[1])) return false;
  if ((bmin1[2] > bmax2[2]) || (bmin2[2] > bmax1[2])) return false;
  return true;

}

bool  fm_insideAABB(const REAL *obmin,const REAL *obmax,const REAL *tbmin,const REAL *tbmax) // test if bounding box tbmin/tmbax is fully inside obmin/obmax
{
  bool ret = false;

  if ( tbmax[0] <= obmax[0] &&
       tbmax[1] <= obmax[1] &&
       tbmax[2] <= obmax[2] &&
       tbmin[0] >= obmin[0] &&
       tbmin[1] >= obmin[1] &&
       tbmin[2] >= obmin[2] ) ret = true;

  return ret;
}


// Reference, from Stan Melax in Game Gems I
//  Quaternion q;
//  vector3 c = CrossProduct(v0,v1);
//  REAL   d = DotProduct(v0,v1);
//  REAL   s = (REAL)sqrt((1+d)*2);
//  q.x = c.x / s;
//  q.y = c.y / s;
//  q.z = c.z / s;
//  q.w = s /2.0f;
//  return q;
void fm_rotationArc(const REAL *v0,const REAL *v1,REAL *quat)
{
  REAL cross[3];

  fm_cross(cross,v0,v1);
  REAL d = fm_dot(v0,v1);
  REAL s = sqrt((1+d)*2);
  REAL recip = 1.0f / s;

  quat[0] = cross[0] * recip;
  quat[1] = cross[1] * recip;
  quat[2] = cross[2] * recip;
  quat[3] = s * 0.5f;

}


REAL fm_distancePointLineSegment(const REAL *Point,const REAL *LineStart,const REAL *LineEnd,REAL *intersection,LineSegmentType &type,REAL epsilon)
{
  REAL ret;

  REAL LineMag = fm_distance( LineEnd, LineStart );

  if ( LineMag > 0 )
  {
    REAL U = ( ( ( Point[0] - LineStart[0] ) * ( LineEnd[0] - LineStart[0] ) ) + ( ( Point[1] - LineStart[1] ) * ( LineEnd[1] - LineStart[1] ) ) + ( ( Point[2] - LineStart[2] ) * ( LineEnd[2] - LineStart[2] ) ) ) / ( LineMag * LineMag );
    if( U < 0.0f || U > 1.0f )
    {
      REAL d1 = fm_distanceSquared(Point,LineStart);
      REAL d2 = fm_distanceSquared(Point,LineEnd);
      if ( d1 <= d2 )
      {
        ret = sqrt(d1);
        intersection[0] = LineStart[0];
        intersection[1] = LineStart[1];
        intersection[2] = LineStart[2];
        type = LS_START;
      }
      else
      {
        ret = sqrt(d2);
        intersection[0] = LineEnd[0];
        intersection[1] = LineEnd[1];
        intersection[2] = LineEnd[2];
        type = LS_END;
      }
    }
    else
    {
      intersection[0] = LineStart[0] + U * ( LineEnd[0] - LineStart[0] );
      intersection[1] = LineStart[1] + U * ( LineEnd[1] - LineStart[1] );
      intersection[2] = LineStart[2] + U * ( LineEnd[2] - LineStart[2] );

      ret = fm_distance(Point,intersection);

      REAL d1 = fm_distanceSquared(intersection,LineStart);
      REAL d2 = fm_distanceSquared(intersection,LineEnd);
	  REAL mag = (epsilon*2)*(epsilon*2);

      if ( d1 < mag ) // if less than 1/100th the total distance, treat is as the 'start'
      {
        type = LS_START;
      }
      else if ( d2 < mag )
      {
        type = LS_END;
      }
      else
      {
        type = LS_MIDDLE;
      }

    }
  }
  else
  {
    ret = LineMag;
    intersection[0] = LineEnd[0];
    intersection[1] = LineEnd[1];
    intersection[2] = LineEnd[2];
    type = LS_END;
  }

  return ret;
}


#ifndef BEST_FIT_PLANE_H

#define BEST_FIT_PLANE_H

template <class Type> class Eigen
{
public:


  void DecrSortEigenStuff(void)
  {
    Tridiagonal(); //diagonalize the matrix.
    QLAlgorithm(); //
    DecreasingSort();
    GuaranteeRotation();
  }

  void Tridiagonal(void)
  {
    Type fM00 = mElement[0][0];
    Type fM01 = mElement[0][1];
    Type fM02 = mElement[0][2];
    Type fM11 = mElement[1][1];
    Type fM12 = mElement[1][2];
    Type fM22 = mElement[2][2];

    m_afDiag[0] = fM00;
    m_afSubd[2] = 0;
    if (fM02 != (Type)0.0)
    {
      Type fLength = sqrt(fM01*fM01+fM02*fM02);
      Type fInvLength = ((Type)1.0)/fLength;
      fM01 *= fInvLength;
      fM02 *= fInvLength;
      Type fQ = ((Type)2.0)*fM01*fM12+fM02*(fM22-fM11);
      m_afDiag[1] = fM11+fM02*fQ;
      m_afDiag[2] = fM22-fM02*fQ;
      m_afSubd[0] = fLength;
      m_afSubd[1] = fM12-fM01*fQ;
      mElement[0][0] = (Type)1.0;
      mElement[0][1] = (Type)0.0;
      mElement[0][2] = (Type)0.0;
      mElement[1][0] = (Type)0.0;
      mElement[1][1] = fM01;
      mElement[1][2] = fM02;
      mElement[2][0] = (Type)0.0;
      mElement[2][1] = fM02;
      mElement[2][2] = -fM01;
      m_bIsRotation = false;
    }
    else
    {
      m_afDiag[1] = fM11;
      m_afDiag[2] = fM22;
      m_afSubd[0] = fM01;
      m_afSubd[1] = fM12;
      mElement[0][0] = (Type)1.0;
      mElement[0][1] = (Type)0.0;
      mElement[0][2] = (Type)0.0;
      mElement[1][0] = (Type)0.0;
      mElement[1][1] = (Type)1.0;
      mElement[1][2] = (Type)0.0;
      mElement[2][0] = (Type)0.0;
      mElement[2][1] = (Type)0.0;
      mElement[2][2] = (Type)1.0;
      m_bIsRotation = true;
    }
  }

  bool QLAlgorithm(void)
  {
    const NxI32 iMaxIter = 32;

    for (NxI32 i0 = 0; i0 <3; i0++)
    {
      NxI32 i1;
      for (i1 = 0; i1 < iMaxIter; i1++)
      {
        NxI32 i2;
        for (i2 = i0; i2 <= (3-2); i2++)
        {
          Type fTmp = fabs(m_afDiag[i2]) + fabs(m_afDiag[i2+1]);
          if ( fabs(m_afSubd[i2]) + fTmp == fTmp )
            break;
        }
        if (i2 == i0)
        {
          break;
        }

        Type fG = (m_afDiag[i0+1] - m_afDiag[i0])/(((Type)2.0) * m_afSubd[i0]);
        Type fR = sqrt(fG*fG+(Type)1.0);
        if (fG < (Type)0.0)
        {
          fG = m_afDiag[i2]-m_afDiag[i0]+m_afSubd[i0]/(fG-fR);
        }
        else
        {
          fG = m_afDiag[i2]-m_afDiag[i0]+m_afSubd[i0]/(fG+fR);
        }
        Type fSin = (Type)1.0, fCos = (Type)1.0, fP = (Type)0.0;
        for (NxI32 i3 = i2-1; i3 >= i0; i3--)
        {
          Type fF = fSin*m_afSubd[i3];
          Type fB = fCos*m_afSubd[i3];
          if (fabs(fF) >= fabs(fG))
          {
            fCos = fG/fF;
            fR = sqrt(fCos*fCos+(Type)1.0);
            m_afSubd[i3+1] = fF*fR;
            fSin = ((Type)1.0)/fR;
            fCos *= fSin;
          }
          else
          {
            fSin = fF/fG;
            fR = sqrt(fSin*fSin+(Type)1.0);
            m_afSubd[i3+1] = fG*fR;
            fCos = ((Type)1.0)/fR;
            fSin *= fCos;
          }
          fG = m_afDiag[i3+1]-fP;
          fR = (m_afDiag[i3]-fG)*fSin+((Type)2.0)*fB*fCos;
          fP = fSin*fR;
          m_afDiag[i3+1] = fG+fP;
          fG = fCos*fR-fB;
          for (NxI32 i4 = 0; i4 < 3; i4++)
          {
            fF = mElement[i4][i3+1];
            mElement[i4][i3+1] = fSin*mElement[i4][i3]+fCos*fF;
            mElement[i4][i3] = fCos*mElement[i4][i3]-fSin*fF;
          }
        }
        m_afDiag[i0] -= fP;
        m_afSubd[i0] = fG;
        m_afSubd[i2] = (Type)0.0;
      }
      if (i1 == iMaxIter)
      {
        return false;
      }
    }
    return true;
  }

  void DecreasingSort(void)
  {
    //sort eigenvalues in decreasing order, e[0] >= ... >= e[iSize-1]
    for (NxI32 i0 = 0, i1; i0 <= 3-2; i0++)
    {
      // locate maximum eigenvalue
      i1 = i0;
      Type fMax = m_afDiag[i1];
      NxI32 i2;
      for (i2 = i0+1; i2 < 3; i2++)
      {
        if (m_afDiag[i2] > fMax)
        {
          i1 = i2;
          fMax = m_afDiag[i1];
        }
      }

      if (i1 != i0)
      {
        // swap eigenvalues
        m_afDiag[i1] = m_afDiag[i0];
        m_afDiag[i0] = fMax;
        // swap eigenvectors
        for (i2 = 0; i2 < 3; i2++)
        {
          Type fTmp = mElement[i2][i0];
          mElement[i2][i0] = mElement[i2][i1];
          mElement[i2][i1] = fTmp;
          m_bIsRotation = !m_bIsRotation;
        }
      }
    }
  }


  void GuaranteeRotation(void)
  {
    if (!m_bIsRotation)
    {
      // change sign on the first column
      for (NxI32 iRow = 0; iRow <3; iRow++)
      {
        mElement[iRow][0] = -mElement[iRow][0];
      }
    }
  }

  Type mElement[3][3];
  Type m_afDiag[3];
  Type m_afSubd[3];
  bool m_bIsRotation;
};

#endif

bool fm_computeBestFitPlane(NxU32 vcount,
                     const REAL *points,
                     NxU32 vstride,
                     const REAL *weights,
                     NxU32 wstride,
                     REAL *plane)
{
  bool ret = false;

  REAL kOrigin[3] = { 0, 0, 0 };

  REAL wtotal = 0;

  {
    const char *source  = (const char *) points;
    const char *wsource = (const char *) weights;

    for (NxU32 i=0; i<vcount; i++)
    {

      const REAL *p = (const REAL *) source;

      REAL w = 1;

      if ( wsource )
      {
        const REAL *ws = (const REAL *) wsource;
        w = *ws; //
        wsource+=wstride;
      }

      kOrigin[0]+=p[0]*w;
      kOrigin[1]+=p[1]*w;
      kOrigin[2]+=p[2]*w;

      wtotal+=w;

      source+=vstride;
    }
  }

  REAL recip = 1.0f / wtotal; // reciprocol of total weighting

  kOrigin[0]*=recip;
  kOrigin[1]*=recip;
  kOrigin[2]*=recip;


  REAL fSumXX=0;
  REAL fSumXY=0;
  REAL fSumXZ=0;

  REAL fSumYY=0;
  REAL fSumYZ=0;
  REAL fSumZZ=0;


  {
    const char *source  = (const char *) points;
    const char *wsource = (const char *) weights;

    for (NxU32 i=0; i<vcount; i++)
    {

      const REAL *p = (const REAL *) source;

      REAL w = 1;

      if ( wsource )
      {
        const REAL *ws = (const REAL *) wsource;
        w = *ws; //
        wsource+=wstride;
      }

      REAL kDiff[3];

      kDiff[0] = w*(p[0] - kOrigin[0]); // apply vertex weighting!
      kDiff[1] = w*(p[1] - kOrigin[1]);
      kDiff[2] = w*(p[2] - kOrigin[2]);

      fSumXX+= kDiff[0] * kDiff[0]; // sume of the squares of the differences.
      fSumXY+= kDiff[0] * kDiff[1]; // sume of the squares of the differences.
      fSumXZ+= kDiff[0] * kDiff[2]; // sume of the squares of the differences.

      fSumYY+= kDiff[1] * kDiff[1];
      fSumYZ+= kDiff[1] * kDiff[2];
      fSumZZ+= kDiff[2] * kDiff[2];


      source+=vstride;
    }
  }

  fSumXX *= recip;
  fSumXY *= recip;
  fSumXZ *= recip;
  fSumYY *= recip;
  fSumYZ *= recip;
  fSumZZ *= recip;

  // setup the eigensolver
  Eigen<REAL> kES;

  kES.mElement[0][0] = fSumXX;
  kES.mElement[0][1] = fSumXY;
  kES.mElement[0][2] = fSumXZ;

  kES.mElement[1][0] = fSumXY;
  kES.mElement[1][1] = fSumYY;
  kES.mElement[1][2] = fSumYZ;

  kES.mElement[2][0] = fSumXZ;
  kES.mElement[2][1] = fSumYZ;
  kES.mElement[2][2] = fSumZZ;

  // compute eigenstuff, smallest eigenvalue is in last position
  kES.DecrSortEigenStuff();

  REAL kNormal[3];

  kNormal[0] = kES.mElement[0][2];
  kNormal[1] = kES.mElement[1][2];
  kNormal[2] = kES.mElement[2][2];

  // the minimum energy
  plane[0] = kNormal[0];
  plane[1] = kNormal[1];
  plane[2] = kNormal[2];

  plane[3] = 0 - fm_dot(kNormal,kOrigin);

  ret = true;

  return ret;
}


bool fm_colinear(const REAL a1[3],const REAL a2[3],const REAL b1[3],const REAL b2[3],REAL epsilon)  // true if these two line segments are co-linear.
{
  bool ret = false;

  REAL dir1[3];
  REAL dir2[3];

  dir1[0] = (a2[0] - a1[0]);
  dir1[1] = (a2[1] - a1[1]);
  dir1[2] = (a2[2] - a1[2]);

  dir2[0] = (b2[0]-a1[0]) - (b1[0]-a1[0]);
  dir2[1] = (b2[1]-a1[1]) - (b1[1]-a1[1]);
  dir2[2] = (b2[2]-a2[2]) - (b1[2]-a2[2]);

  fm_normalize(dir1);
  fm_normalize(dir2);

  REAL dot = fm_dot(dir1,dir2);

  if ( dot >= epsilon )
  {
    ret = true;
  }


  return ret;
}

bool fm_colinear(const REAL *p1,const REAL *p2,const REAL *p3,REAL epsilon)
{
  bool ret = false;

  REAL dir1[3];
  REAL dir2[3];

  dir1[0] = p2[0] - p1[0];
  dir1[1] = p2[1] - p1[1];
  dir1[2] = p2[2] - p1[2];

  dir2[0] = p3[0] - p2[0];
  dir2[1] = p3[1] - p2[1];
  dir2[2] = p3[2] - p2[2];

  fm_normalize(dir1);
  fm_normalize(dir2);

  REAL dot = fm_dot(dir1,dir2);

  if ( dot >= epsilon )
  {
    ret = true;
  }


  return ret;
}

void  fm_initMinMax(const REAL *p,REAL *bmin,REAL *bmax)
{
  bmax[0] = bmin[0] = p[0];
  bmax[1] = bmin[1] = p[1];
  bmax[2] = bmin[2] = p[2];
}

IntersectResult fm_intersectLineSegments2d(const REAL *a1,const REAL *a2,const REAL *b1,const REAL *b2,REAL *intersection)
{
  IntersectResult ret;

  REAL denom  = ((b2[1] - b1[1])*(a2[0] - a1[0])) - ((b2[0] - b1[0])*(a2[1] - a1[1]));
  REAL nume_a = ((b2[0] - b1[0])*(a1[1] - b1[1])) - ((b2[1] - b1[1])*(a1[0] - b1[0]));
  REAL nume_b = ((a2[0] - a1[0])*(a1[1] - b1[1])) - ((a2[1] - a1[1])*(a1[0] - b1[0]));
  if (denom == 0 )
  {
    if(nume_a == 0 && nume_b == 0)
    {
      ret = IR_COINCIDENT;
    }
    else
    {
      ret = IR_PARALLEL;
    }
  }
  else
  {

    REAL recip = 1 / denom;
    REAL ua = nume_a * recip;
    REAL ub = nume_b * recip;

    if(ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1 )
    {
      // Get the intersection point.
      intersection[0] = a1[0] + ua*(a2[0] - a1[0]);
      intersection[1] = a1[1] + ua*(a2[1] - a1[1]);
      ret = IR_DO_INTERSECT;
    }
    else
    {
      ret = IR_DONT_INTERSECT;
    }
  }
  return ret;
}

IntersectResult fm_intersectLineSegments2dTime(const REAL *a1,const REAL *a2,const REAL *b1,const REAL *b2,REAL &t1,REAL &t2)
{
  IntersectResult ret;

  REAL denom  = ((b2[1] - b1[1])*(a2[0] - a1[0])) - ((b2[0] - b1[0])*(a2[1] - a1[1]));
  REAL nume_a = ((b2[0] - b1[0])*(a1[1] - b1[1])) - ((b2[1] - b1[1])*(a1[0] - b1[0]));
  REAL nume_b = ((a2[0] - a1[0])*(a1[1] - b1[1])) - ((a2[1] - a1[1])*(a1[0] - b1[0]));
  if (denom == 0 )
  {
    if(nume_a == 0 && nume_b == 0)
    {
      ret = IR_COINCIDENT;
    }
    else
    {
      ret = IR_PARALLEL;
    }
  }
  else
  {

    REAL recip = 1 / denom;
    REAL ua = nume_a * recip;
    REAL ub = nume_b * recip;

    if(ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1 )
    {
      t1 = ua;
      t2 = ub;
      ret = IR_DO_INTERSECT;
    }
    else
    {
      ret = IR_DONT_INTERSECT;
    }
  }
  return ret;
}

//**** Plane Triangle Intersection





// assumes that the points are on opposite sides of the plane!
void fm_intersectPointPlane(const REAL *p1,const REAL *p2,REAL *split,const REAL *plane)
{

  REAL dp1 = fm_distToPlane(plane,p1);

  REAL dir[3];

  dir[0] = p2[0] - p1[0];
  dir[1] = p2[1] - p1[1];
  dir[2] = p2[2] - p1[2];

  REAL dot1 = dir[0]*plane[0] + dir[1]*plane[1] + dir[2]*plane[2];
  REAL dot2 = dp1 - plane[3];

  REAL    t = -(plane[3] + dot2 ) / dot1;

  split[0] = (dir[0]*t)+p1[0];
  split[1] = (dir[1]*t)+p1[1];
  split[2] = (dir[2]*t)+p1[2];

}

PlaneTriResult fm_getSidePlane(const REAL *p,const REAL *plane,REAL epsilon)
{
  PlaneTriResult ret = PTR_ON_PLANE;

  REAL d = fm_distToPlane(plane,p);

  if ( d < -epsilon || d > epsilon )
  {
    if ( d > 0 )
  		ret =  PTR_FRONT; // it is 'in front' within the provided epsilon value.
    else
      ret = PTR_BACK;
  }

  return ret;
}



#ifndef PLANE_TRIANGLE_INTERSECTION_H

#define PLANE_TRIANGLE_INTERSECTION_H

#define MAXPTS 256

template <class Type> class point
{
public:

  void set(const Type *p)
  {
    x = p[0];
    y = p[1];
    z = p[2];
  }

  Type x;
  Type y;
  Type z;
};

template <class Type> class plane
{
public:
  plane(const Type *p)
  {
    normal.x = p[0];
    normal.y = p[1];
    normal.z = p[2];
    D        = p[3];
  }

  Type Classify_Point(const point<Type> &p)
  {
    return p.x*normal.x + p.y*normal.y + p.z*normal.z + D;
  }

  point<Type> normal;
  Type  D;
};

template <class Type> class polygon
{
public:
  polygon(void)
  {
    mVcount = 0;
  }

  polygon(const Type *p1,const Type *p2,const Type *p3)
  {
    mVcount = 3;
    mVertices[0].set(p1);
    mVertices[1].set(p2);
    mVertices[2].set(p3);
  }


  NxI32 NumVertices(void) const { return mVcount; };

  const point<Type>& Vertex(NxI32 index)
  {
    if ( index < 0 ) index+=mVcount;
    return mVertices[index];
  };


  void set(const point<Type> *pts,NxI32 count)
  {
    for (NxI32 i=0; i<count; i++)
    {
      mVertices[i] = pts[i];
    }
    mVcount = count;
  }


  void Split_Polygon(polygon<Type> *poly,plane<Type> *part, polygon<Type> &front, polygon<Type> &back)
  {
    NxI32   count = poly->NumVertices ();
    NxI32   out_c = 0, in_c = 0;
    point<Type> ptA, ptB,outpts[MAXPTS],inpts[MAXPTS];
    Type sideA, sideB;
    ptA = poly->Vertex (count - 1);
    sideA = part->Classify_Point (ptA);
    for (NxI32 i = -1; ++i < count;)
    {
      ptB = poly->Vertex(i);
      sideB = part->Classify_Point(ptB);
      if (sideB > 0)
      {
        if (sideA < 0)
        {
  			  point<Type> v;
          fm_intersectPointPlane(&ptB.x, &ptA.x, &v.x, &part->normal.x );
          outpts[out_c++] = inpts[in_c++] = v;
        }
        outpts[out_c++] = ptB;
      }
      else if (sideB < 0)
      {
        if (sideA > 0)
        {
          point<Type> v;
          fm_intersectPointPlane(&ptB.x, &ptA.x, &v.x, &part->normal.x );
          outpts[out_c++] = inpts[in_c++] = v;
        }
        inpts[in_c++] = ptB;
      }
      else
         outpts[out_c++] = inpts[in_c++] = ptB;
      ptA = ptB;
      sideA = sideB;
    }

    front.set(&outpts[0], out_c);
    back.set(&inpts[0], in_c);
  }

  NxI32           mVcount;
  point<Type>   mVertices[MAXPTS];
};



#endif

static inline void add(const REAL *p,REAL *dest,NxU32 tstride,NxU32 &pcount)
{
  char *d = (char *) dest;
  d = d + pcount*tstride;
  dest = (REAL *) d;
  dest[0] = p[0];
  dest[1] = p[1];
  dest[2] = p[2];
  pcount++;
	assert( pcount <= 4 );
}


PlaneTriResult fm_planeTriIntersection(const REAL *_plane,    // the plane equation in Ax+By+Cz+D format
                                    const REAL *triangle, // the source triangle.
                                    NxU32 tstride,  // stride in bytes of the input and output *vertices*
                                    REAL        epsilon,  // the co-planar epsilon value.
                                    REAL       *front,    // the triangle in front of the
                                    NxU32 &fcount,  // number of vertices in the 'front' triangle
                                    REAL       *back,     // the triangle in back of the plane
                                    NxU32 &bcount) // the number of vertices in the 'back' triangle.
{

  fcount = 0;
  bcount = 0;

  const char *tsource = (const char *) triangle;

  // get the three vertices of the triangle.
  const REAL *p1     = (const REAL *) (tsource);
  const REAL *p2     = (const REAL *) (tsource+tstride);
  const REAL *p3     = (const REAL *) (tsource+tstride*2);


  PlaneTriResult r1   = fm_getSidePlane(p1,_plane,epsilon); // compute the side of the plane each vertex is on
  PlaneTriResult r2   = fm_getSidePlane(p2,_plane,epsilon);
  PlaneTriResult r3   = fm_getSidePlane(p3,_plane,epsilon);

  // If any of the points lay right *on* the plane....
  if ( r1 == PTR_ON_PLANE || r2 == PTR_ON_PLANE || r3 == PTR_ON_PLANE )
  {
    // If the triangle is completely co-planar, then just treat it as 'front' and return!
    if ( r1 == PTR_ON_PLANE && r2 == PTR_ON_PLANE && r3 == PTR_ON_PLANE )
    {
      add(p1,front,tstride,fcount);
      add(p2,front,tstride,fcount);
      add(p3,front,tstride,fcount);
      return PTR_FRONT;
    }
    // Decide to place the co-planar points on the same side as the co-planar point.
    PlaneTriResult r= PTR_ON_PLANE;
    if ( r1 != PTR_ON_PLANE )
      r = r1;
    else if ( r2 != PTR_ON_PLANE )
      r = r2;
    else if ( r3 != PTR_ON_PLANE )
      r = r3;

    if ( r1 == PTR_ON_PLANE ) r1 = r;
    if ( r2 == PTR_ON_PLANE ) r2 = r;
    if ( r3 == PTR_ON_PLANE ) r3 = r;

  }

  if ( r1 == r2 && r1 == r3 ) // if all three vertices are on the same side of the plane.
  {
    if ( r1 == PTR_FRONT ) // if all three are in front of the plane, then copy to the 'front' output triangle.
    {
      add(p1,front,tstride,fcount);
      add(p2,front,tstride,fcount);
      add(p3,front,tstride,fcount);
    }
    else
    {
      add(p1,back,tstride,bcount); // if all three are in 'back' then copy to the 'back' output triangle.
      add(p2,back,tstride,bcount);
      add(p3,back,tstride,bcount);
    }
    return r1; // if all three points are on the same side of the plane return result
  }


  polygon<REAL> pi(p1,p2,p3);
  polygon<REAL>  pfront,pback;

  plane<REAL>    part(_plane);

  pi.Split_Polygon(&pi,&part,pfront,pback);

  for (NxI32 i=0; i<pfront.mVcount; i++)
  {
    add( &pfront.mVertices[i].x, front, tstride, fcount );
  }

  for (NxI32 i=0; i<pback.mVcount; i++)
  {
    add( &pback.mVertices[i].x, back, tstride, bcount );
  }

  PlaneTriResult ret = PTR_SPLIT;

  if ( fcount < 3 ) fcount = 0;
  if ( bcount < 3 ) bcount = 0;

  if ( fcount == 0 && bcount )
    ret = PTR_BACK;

  if ( bcount == 0 && fcount )
    ret = PTR_FRONT;


  return ret;
}

// computes the OBB for this set of points relative to this transform matrix.
void computeOBB(NxU32 vcount,const REAL *points,NxU32 pstride,REAL *sides,REAL *matrix)
{
  const char *src = (const char *) points;

  REAL bmin[3] = { 1e9, 1e9, 1e9 };
  REAL bmax[3] = { -1e9, -1e9, -1e9 };

  for (NxU32 i=0; i<vcount; i++)
  {
    const REAL *p = (const REAL *) src;
    REAL t[3];

    fm_inverseRT(matrix, p, t ); // inverse rotate translate

    if ( t[0] < bmin[0] ) bmin[0] = t[0];
    if ( t[1] < bmin[1] ) bmin[1] = t[1];
    if ( t[2] < bmin[2] ) bmin[2] = t[2];

    if ( t[0] > bmax[0] ) bmax[0] = t[0];
    if ( t[1] > bmax[1] ) bmax[1] = t[1];
    if ( t[2] > bmax[2] ) bmax[2] = t[2];

    src+=pstride;
  }

  REAL center[3];

  sides[0] = bmax[0]-bmin[0];
  sides[1] = bmax[1]-bmin[1];
  sides[2] = bmax[2]-bmin[2];

  center[0] = sides[0]*0.5f+bmin[0];
  center[1] = sides[1]*0.5f+bmin[1];
  center[2] = sides[2]*0.5f+bmin[2];

  REAL ocenter[3];

  fm_rotate(matrix,center,ocenter);

  matrix[12]+=ocenter[0];
  matrix[13]+=ocenter[1];
  matrix[14]+=ocenter[2];

}

void fm_computeBestFitOBB(NxU32 vcount,const REAL *points,NxU32 pstride,REAL *sides,REAL *matrix,bool bruteForce)
{
  REAL plane[4];
  fm_computeBestFitPlane(vcount,points,pstride,0,0,plane);
  fm_planeToMatrix(plane,matrix);
  computeOBB( vcount, points, pstride, sides, matrix );

  REAL refmatrix[16];
  memcpy(refmatrix,matrix,16*sizeof(REAL));

  REAL volume = sides[0]*sides[1]*sides[2];
  if ( bruteForce )
  {
    for (REAL a=10; a<180; a+=10)
    {
      REAL quat[4];
      fm_eulerToQuat(0,a*FM_DEG_TO_RAD,0,quat);
      REAL temp[16];
      REAL pmatrix[16];
      fm_quatToMatrix(quat,temp);
      fm_matrixMultiply(temp,refmatrix,pmatrix);
      REAL psides[3];
      computeOBB( vcount, points, pstride, psides, pmatrix );
      REAL v = psides[0]*psides[1]*psides[2];
      if ( v < volume )
      {
        volume = v;
        memcpy(matrix,pmatrix,sizeof(REAL)*16);
        sides[0] = psides[0];
        sides[1] = psides[1];
        sides[2] = psides[2];
      }
    }
  }
}

void fm_computeBestFitOBB(NxU32 vcount,const REAL *points,NxU32 pstride,REAL *sides,REAL *pos,REAL *quat,bool bruteForce)
{
  REAL matrix[16];
  fm_computeBestFitOBB(vcount,points,pstride,sides,matrix,bruteForce);
  fm_getTranslation(matrix,pos);
  fm_matrixToQuat(matrix,quat);
}

void fm_computeBestFitABB(NxU32 vcount,const REAL *points,NxU32 pstride,REAL *sides,REAL *pos)
{
	REAL bmin[3];
	REAL bmax[3];

  bmin[0] = points[0];
  bmin[1] = points[1];
  bmin[2] = points[2];

  bmax[0] = points[0];
  bmax[1] = points[1];
  bmax[2] = points[2];

	const char *cp = (const char *) points;
	for (NxU32 i=0; i<vcount; i++)
	{
		const REAL *p = (const REAL *) cp;

		if ( p[0] < bmin[0] ) bmin[0] = p[0];
		if ( p[1] < bmin[1] ) bmin[1] = p[1];
		if ( p[2] < bmin[2] ) bmin[2] = p[2];

    if ( p[0] > bmax[0] ) bmax[0] = p[0];
    if ( p[1] > bmax[1] ) bmax[1] = p[1];
    if ( p[2] > bmax[2] ) bmax[2] = p[2];

    cp+=pstride;
	}


	sides[0] = bmax[0] - bmin[0];
	sides[1] = bmax[1] - bmin[1];
	sides[2] = bmax[2] - bmin[2];

	pos[0] = bmin[0]+sides[0]*0.5f;
	pos[1] = bmin[1]+sides[1]*0.5f;
	pos[2] = bmin[2]+sides[2]*0.5f;

}


void fm_planeToMatrix(const REAL *plane,REAL *matrix) // convert a plane equation to a 4x4 rotation matrix
{
  REAL ref[3] = { 0, 1, 0 };
  REAL quat[4];
  fm_rotationArc(ref,plane,quat);
  fm_quatToMatrix(quat,matrix);
  REAL origin[3] = { 0, -plane[3], 0 };
  REAL center[3];
  fm_transform(matrix,origin,center);
  fm_setTranslation(center,matrix);
}

void fm_planeToQuat(const REAL *plane,REAL *quat,REAL *pos) // convert a plane equation to a quaternion and translation
{
  REAL ref[3] = { 0, 1, 0 };
  REAL matrix[16];
  fm_rotationArc(ref,plane,quat);
  fm_quatToMatrix(quat,matrix);
  REAL origin[3] = { 0, plane[3], 0 };
  fm_transform(matrix,origin,pos);
}

void fm_eulerMatrix(REAL ax,REAL ay,REAL az,REAL *matrix) // convert euler (in radians) to a dest 4x4 matrix (translation set to zero)
{
  REAL quat[4];
  fm_eulerToQuat(ax,ay,az,quat);
  fm_quatToMatrix(quat,matrix);
}


//**********************************************************
//**********************************************************
//**** Vertex Welding
//**********************************************************
//**********************************************************

#ifndef VERTEX_INDEX_H

#define VERTEX_INDEX_H

namespace VERTEX_INDEX
{

class KdTreeNode;

typedef CONVEX_DECOMPOSITION::Array< KdTreeNode * > KdTreeNodeVector;

enum Axes
{
  X_AXIS = 0,
  Y_AXIS = 1,
  Z_AXIS = 2
};

class KdTreeFindNode
{
public:
  KdTreeFindNode(void)
  {
    mNode = 0;
    mDistance = 0;
  }
  KdTreeNode  *mNode;
  NxF64        mDistance;
};

class KdTreeInterface
{
public:
  virtual const NxF64 * getPositionDouble(NxU32 index) const = 0;
  virtual const NxF32  * getPositionFloat(NxU32 index) const = 0;
};

class KdTreeNode
{
public:
  KdTreeNode(void)
  {
    mIndex = 0;
    mLeft = 0;
    mRight = 0;
  }

  KdTreeNode(NxU32 index)
  {
    mIndex = index;
    mLeft = 0;
    mRight = 0;
  };

	~KdTreeNode(void)
  {
  }


  void addDouble(KdTreeNode *node,Axes dim,const KdTreeInterface *iface)
  {
    const NxF64 *nodePosition = iface->getPositionDouble( node->mIndex );
    const NxF64 *position     = iface->getPositionDouble( mIndex );
    switch ( dim )
    {
      case X_AXIS:
        if ( nodePosition[0] <= position[0] )
        {
          if ( mLeft )
            mLeft->addDouble(node,Y_AXIS,iface);
          else
            mLeft = node;
        }
        else
        {
          if ( mRight )
            mRight->addDouble(node,Y_AXIS,iface);
          else
            mRight = node;
        }
        break;
      case Y_AXIS:
        if ( nodePosition[1] <= position[1] )
        {
          if ( mLeft )
            mLeft->addDouble(node,Z_AXIS,iface);
          else
            mLeft = node;
        }
        else
        {
          if ( mRight )
            mRight->addDouble(node,Z_AXIS,iface);
          else
            mRight = node;
        }
        break;
      case Z_AXIS:
        if ( nodePosition[2] <= position[2] )
        {
          if ( mLeft )
            mLeft->addDouble(node,X_AXIS,iface);
          else
            mLeft = node;
        }
        else
        {
          if ( mRight )
            mRight->addDouble(node,X_AXIS,iface);
          else
            mRight = node;
        }
        break;
    }

  }


  void addFloat(KdTreeNode *node,Axes dim,const KdTreeInterface *iface)
  {
    const NxF32 *nodePosition = iface->getPositionFloat( node->mIndex );
    const NxF32 *position     = iface->getPositionFloat( mIndex );
    switch ( dim )
    {
      case X_AXIS:
        if ( nodePosition[0] <= position[0] )
        {
          if ( mLeft )
            mLeft->addFloat(node,Y_AXIS,iface);
          else
            mLeft = node;
        }
        else
        {
          if ( mRight )
            mRight->addFloat(node,Y_AXIS,iface);
          else
            mRight = node;
        }
        break;
      case Y_AXIS:
        if ( nodePosition[1] <= position[1] )
        {
          if ( mLeft )
            mLeft->addFloat(node,Z_AXIS,iface);
          else
            mLeft = node;
        }
        else
        {
          if ( mRight )
            mRight->addFloat(node,Z_AXIS,iface);
          else
            mRight = node;
        }
        break;
      case Z_AXIS:
        if ( nodePosition[2] <= position[2] )
        {
          if ( mLeft )
            mLeft->addFloat(node,X_AXIS,iface);
          else
            mLeft = node;
        }
        else
        {
          if ( mRight )
            mRight->addFloat(node,X_AXIS,iface);
          else
            mRight = node;
        }
        break;
    }

  }


  NxU32 getIndex(void) const { return mIndex; };

  void search(Axes axis,const NxF64 *pos,NxF64 radius,NxU32 &count,NxU32 maxObjects,KdTreeFindNode *found,const KdTreeInterface *iface)
  {

    const NxF64 *position = iface->getPositionDouble(mIndex);

    NxF64 dx = pos[0] - position[0];
    NxF64 dy = pos[1] - position[1];
    NxF64 dz = pos[2] - position[2];

    KdTreeNode *search1 = 0;
    KdTreeNode *search2 = 0;

    switch ( axis )
    {
      case X_AXIS:
       if ( dx <= 0 )     // JWR  if we are to the left
       {
        search1 = mLeft; // JWR  then search to the left
        if ( -dx < radius )  // JWR  if distance to the right is less than our search radius, continue on the right as well.
          search2 = mRight;
       }
       else
       {
         search1 = mRight; // JWR  ok, we go down the left tree
         if ( dx < radius ) // JWR  if the distance from the right is less than our search radius
	  			search2 = mLeft;
        }
        axis = Y_AXIS;
        break;
      case Y_AXIS:
        if ( dy <= 0 )
        {
          search1 = mLeft;
          if ( -dy < radius )
    				search2 = mRight;
        }
        else
        {
          search1 = mRight;
          if ( dy < radius )
    				search2 = mLeft;
        }
        axis = Z_AXIS;
        break;
      case Z_AXIS:
        if ( dz <= 0 )
        {
          search1 = mLeft;
          if ( -dz < radius )
    				search2 = mRight;
        }
        else
        {
          search1 = mRight;
          if ( dz < radius )
    				search2 = mLeft;
        }
        axis = X_AXIS;
        break;
    }

    NxF64 r2 = radius*radius;
    NxF64 m  = dx*dx+dy*dy+dz*dz;

    if ( m < r2 )
    {
      switch ( count )
      {
        case 0:
          found[count].mNode = this;
          found[count].mDistance = m;
          break;
        case 1:
          if ( m < found[0].mDistance )
          {
            if ( maxObjects == 1 )
            {
              found[0].mNode = this;
              found[0].mDistance = m;
            }
            else
            {
              found[1] = found[0];
              found[0].mNode = this;
              found[0].mDistance = m;
            }
          }
          else if ( maxObjects > 1)
          {
            found[1].mNode = this;
            found[1].mDistance = m;
          }
          break;
        default:
          {
            bool inserted = false;

            for (NxU32 i=0; i<count; i++)
            {
              if ( m < found[i].mDistance ) // if this one is closer than a pre-existing one...
              {
                // insertion sort...
                NxU32 scan = count;
                if ( scan >= maxObjects ) scan=maxObjects-1;
                for (NxU32 j=scan; j>i; j--)
                {
                  found[j] = found[j-1];
                }
                found[i].mNode = this;
                found[i].mDistance = m;
                inserted = true;
                break;
              }
            }

            if ( !inserted && count < maxObjects )
            {
              found[count].mNode = this;
              found[count].mDistance = m;
            }
          }
          break;
      }
      count++;
      if ( count > maxObjects )
      {
        count = maxObjects;
      }
    }


    if ( search1 )
  		search1->search( axis, pos,radius, count, maxObjects, found, iface);

    if ( search2 )
	  	search2->search( axis, pos,radius, count, maxObjects, found, iface);

  }

  void search(Axes axis,const NxF32 *pos,NxF32 radius,NxU32 &count,NxU32 maxObjects,KdTreeFindNode *found,const KdTreeInterface *iface)
  {

    const NxF32 *position = iface->getPositionFloat(mIndex);

    NxF32 dx = pos[0] - position[0];
    NxF32 dy = pos[1] - position[1];
    NxF32 dz = pos[2] - position[2];

    KdTreeNode *search1 = 0;
    KdTreeNode *search2 = 0;

    switch ( axis )
    {
      case X_AXIS:
       if ( dx <= 0 )     // JWR  if we are to the left
       {
        search1 = mLeft; // JWR  then search to the left
        if ( -dx < radius )  // JWR  if distance to the right is less than our search radius, continue on the right as well.
          search2 = mRight;
       }
       else
       {
         search1 = mRight; // JWR  ok, we go down the left tree
         if ( dx < radius ) // JWR  if the distance from the right is less than our search radius
	  			search2 = mLeft;
        }
        axis = Y_AXIS;
        break;
      case Y_AXIS:
        if ( dy <= 0 )
        {
          search1 = mLeft;
          if ( -dy < radius )
    				search2 = mRight;
        }
        else
        {
          search1 = mRight;
          if ( dy < radius )
    				search2 = mLeft;
        }
        axis = Z_AXIS;
        break;
      case Z_AXIS:
        if ( dz <= 0 )
        {
          search1 = mLeft;
          if ( -dz < radius )
    				search2 = mRight;
        }
        else
        {
          search1 = mRight;
          if ( dz < radius )
    				search2 = mLeft;
        }
        axis = X_AXIS;
        break;
    }

    NxF32 r2 = radius*radius;
    NxF32 m  = dx*dx+dy*dy+dz*dz;

    if ( m < r2 )
    {
      switch ( count )
      {
        case 0:
          found[count].mNode = this;
          found[count].mDistance = m;
          break;
        case 1:
          if ( m < found[0].mDistance )
          {
            if ( maxObjects == 1 )
            {
              found[0].mNode = this;
              found[0].mDistance = m;
            }
            else
            {
              found[1] = found[0];
              found[0].mNode = this;
              found[0].mDistance = m;
            }
          }
          else if ( maxObjects > 1)
          {
            found[1].mNode = this;
            found[1].mDistance = m;
          }
          break;
        default:
          {
            bool inserted = false;

            for (NxU32 i=0; i<count; i++)
            {
              if ( m < found[i].mDistance ) // if this one is closer than a pre-existing one...
              {
                // insertion sort...
                NxU32 scan = count;
                if ( scan >= maxObjects ) scan=maxObjects-1;
                for (NxU32 j=scan; j>i; j--)
                {
                  found[j] = found[j-1];
                }
                found[i].mNode = this;
                found[i].mDistance = m;
                inserted = true;
                break;
              }
            }

            if ( !inserted && count < maxObjects )
            {
              found[count].mNode = this;
              found[count].mDistance = m;
            }
          }
          break;
      }
      count++;
      if ( count > maxObjects )
      {
        count = maxObjects;
      }
    }


    if ( search1 )
  		search1->search( axis, pos,radius, count, maxObjects, found, iface);

    if ( search2 )
	  	search2->search( axis, pos,radius, count, maxObjects, found, iface);

  }

private:

  void setLeft(KdTreeNode *left) { mLeft = left; };
  void setRight(KdTreeNode *right) { mRight = right; };

	KdTreeNode *getLeft(void)         { return mLeft; }
	KdTreeNode *getRight(void)        { return mRight; }

  NxU32          mIndex;
  KdTreeNode     *mLeft;
  KdTreeNode     *mRight;
};


#define MAX_BUNDLE_SIZE 1024  // 1024 nodes at a time, to minimize memory allocation and guarentee that pointers are persistent.

class KdTreeNodeBundle : public Memalloc
{
public:

  KdTreeNodeBundle(void)
  {
    mNext = 0;
    mIndex = 0;
  }

  bool isFull(void) const
  {
    return (bool)( mIndex == MAX_BUNDLE_SIZE );
  }

  KdTreeNode * getNextNode(void)
  {
    assert(mIndex<MAX_BUNDLE_SIZE);
    KdTreeNode *ret = &mNodes[mIndex];
    mIndex++;
    return ret;
  }

  KdTreeNodeBundle  *mNext;
  NxU32             mIndex;
  KdTreeNode         mNodes[MAX_BUNDLE_SIZE];
};


typedef CONVEX_DECOMPOSITION::Array< NxF64 > DoubleVector;
typedef CONVEX_DECOMPOSITION::Array< NxF32 >  FloatVector;

class KdTree : public KdTreeInterface, public Memalloc
{
public:
  KdTree(void)
  {
    mRoot = 0;
    mBundle = 0;
    mVcount = 0;
    mUseDouble = false;
  }

  virtual ~KdTree(void)
  {
    reset();
  }

  const NxF64 * getPositionDouble(NxU32 index) const
  {
    assert( mUseDouble );
    assert ( index < mVcount );
    return  &mVerticesDouble[index*3];
  }

  const NxF32 * getPositionFloat(NxU32 index) const
  {
    assert( !mUseDouble );
    assert ( index < mVcount );
    return  &mVerticesFloat[index*3];
  }

  NxU32 search(const NxF64 *pos,NxF64 radius,NxU32 maxObjects,KdTreeFindNode *found) const
  {
    assert( mUseDouble );
    if ( !mRoot )	return 0;
    NxU32 count = 0;
    mRoot->search(X_AXIS,pos,radius,count,maxObjects,found,this);
    return count;
  }

  NxU32 search(const NxF32 *pos,NxF32 radius,NxU32 maxObjects,KdTreeFindNode *found) const
  {
    assert( !mUseDouble );
    if ( !mRoot )	return 0;
    NxU32 count = 0;
    mRoot->search(X_AXIS,pos,radius,count,maxObjects,found,this);
    return count;
  }

  void reset(void)
  {
    mRoot = 0;
    mVerticesDouble.clear();
    mVerticesFloat.clear();
    KdTreeNodeBundle *bundle = mBundle;
    while ( bundle )
    {
      KdTreeNodeBundle *next = bundle->mNext;
      delete bundle;
      bundle = next;
    }
    mBundle = 0;
    mVcount = 0;
  }

  NxU32 add(NxF64 x,NxF64 y,NxF64 z)
  {
    assert(mUseDouble);
    NxU32 ret = mVcount;
    mVerticesDouble.pushBack(x);
    mVerticesDouble.pushBack(y);
    mVerticesDouble.pushBack(z);
    mVcount++;
    KdTreeNode *node = getNewNode(ret);
    if ( mRoot )
    {
      mRoot->addDouble(node,X_AXIS,this);
    }
    else
    {
      mRoot = node;
    }
    return ret;
  }

  NxU32 add(NxF32 x,NxF32 y,NxF32 z)
  {
    assert(!mUseDouble);
    NxU32 ret = mVcount;
    mVerticesFloat.pushBack(x);
    mVerticesFloat.pushBack(y);
    mVerticesFloat.pushBack(z);
    mVcount++;
    KdTreeNode *node = getNewNode(ret);
    if ( mRoot )
    {
      mRoot->addFloat(node,X_AXIS,this);
    }
    else
    {
      mRoot = node;
    }
    return ret;
  }

  KdTreeNode * getNewNode(NxU32 index)
  {
    if ( mBundle == 0 )
    {
      mBundle = MEMALLOC_NEW(KdTreeNodeBundle);
    }
    if ( mBundle->isFull() )
    {
      KdTreeNodeBundle *bundle = MEMALLOC_NEW(KdTreeNodeBundle);
      mBundle->mNext = bundle;
      mBundle = bundle;
    }
    KdTreeNode *node = mBundle->getNextNode();
    new ( node ) KdTreeNode(index);
    return node;
  }

  NxU32 getNearest(const NxF64 *pos,NxF64 radius,bool &_found) const // returns the nearest possible neighbor's index.
  {
    assert( mUseDouble );
    NxU32 ret = 0;

    _found = false;
    KdTreeFindNode found[1];
    NxU32 count = search(pos,radius,1,found);
    if ( count )
    {
      KdTreeNode *node = found[0].mNode;
      ret = node->getIndex();
      _found = true;
    }
    return ret;
  }

  NxU32 getNearest(const NxF32 *pos,NxF32 radius,bool &_found) const // returns the nearest possible neighbor's index.
  {
    assert( !mUseDouble );
    NxU32 ret = 0;

    _found = false;
    KdTreeFindNode found[1];
    NxU32 count = search(pos,radius,1,found);
    if ( count )
    {
      KdTreeNode *node = found[0].mNode;
      ret = node->getIndex();
      _found = true;
    }
    return ret;
  }

  const NxF64 * getVerticesDouble(void) const
  {
    assert( mUseDouble );
    const NxF64 *ret = 0;
    if ( !mVerticesDouble.empty() )
    {
      ret = &mVerticesDouble[0];
    }
    return ret;
  }

  const NxF32 * getVerticesFloat(void) const
  {
    assert( !mUseDouble );
    const NxF32 * ret = 0;
    if ( !mVerticesFloat.empty() )
    {
      ret = &mVerticesFloat[0];
    }
    return ret;
  }

  NxU32 getVcount(void) const { return mVcount; };

  void setUseDouble(bool useDouble)
  {
    mUseDouble = useDouble;
  }

private:
  bool                    mUseDouble;
  KdTreeNode             *mRoot;
  KdTreeNodeBundle       *mBundle;
  NxU32                  mVcount;
  DoubleVector            mVerticesDouble;
  FloatVector             mVerticesFloat;
};

}; // end of namespace VERTEX_INDEX

class MyVertexIndex : public fm_VertexIndex, public Memalloc
{
public:
  MyVertexIndex(NxF64 granularity,bool snapToGrid)
  {
    mDoubleGranularity = granularity;
    mFloatGranularity  = (NxF32)granularity;
    mSnapToGrid        = snapToGrid;
    mUseDouble         = true;
    mKdTree.setUseDouble(true);
  }

  MyVertexIndex(NxF32 granularity,bool snapToGrid)
  {
    mDoubleGranularity = granularity;
    mFloatGranularity  = (NxF32)granularity;
    mSnapToGrid        = snapToGrid;
    mUseDouble         = false;
    mKdTree.setUseDouble(false);
  }

  virtual ~MyVertexIndex(void)
  {

  }


  NxF64 snapToGrid(NxF64 p)
  {
    NxF64 m = fmod(p,mDoubleGranularity);
    p-=m;
    return p;
  }

  NxF32 snapToGrid(NxF32 p)
  {
    NxF32 m = fmodf(p,mFloatGranularity);
    p-=m;
    return p;
  }

  NxU32    getIndex(const NxF32 *_p,bool &newPos)  // get index for a vector NxF32
  {
    NxU32 ret;

    if ( mUseDouble )
    {
      NxF64 p[3];
      p[0] = _p[0];
      p[1] = _p[1];
      p[2] = _p[2];
      return getIndex(p,newPos);
    }

    newPos = false;

    NxF32 p[3];

    if ( mSnapToGrid )
    {
      p[0] = snapToGrid(_p[0]);
      p[1] = snapToGrid(_p[1]);
      p[2] = snapToGrid(_p[2]);
    }
    else
    {
      p[0] = _p[0];
      p[1] = _p[1];
      p[2] = _p[2];
    }

    bool found;
    ret = mKdTree.getNearest(p,mFloatGranularity,found);
    if ( !found )
    {
      newPos = true;
      ret = mKdTree.add(p[0],p[1],p[2]);
    }


    return ret;
  }

  NxU32    getIndex(const NxF64 *_p,bool &newPos)  // get index for a vector NxF64
  {
    NxU32 ret;

    if ( !mUseDouble )
    {
      NxF32 p[3];
      p[0] = (NxF32)_p[0];
      p[1] = (NxF32)_p[1];
      p[2] = (NxF32)_p[2];
      return getIndex(p,newPos);
    }

    newPos = false;

    NxF64 p[3];

    if ( mSnapToGrid )
    {
      p[0] = snapToGrid(_p[0]);
      p[1] = snapToGrid(_p[1]);
      p[2] = snapToGrid(_p[2]);
    }
    else
    {
      p[0] = _p[0];
      p[1] = _p[1];
      p[2] = _p[2];
    }

    bool found;
    ret = mKdTree.getNearest(p,mDoubleGranularity,found);
    if ( !found )
    {
      newPos = true;
      ret = mKdTree.add(p[0],p[1],p[2]);
    }


    return ret;
  }

  const NxF32 *   getVerticesFloat(void) const
  {
    const NxF32 * ret = 0;

    assert( !mUseDouble );

    ret = mKdTree.getVerticesFloat();

    return ret;
  }

  const NxF64 *  getVerticesDouble(void) const
  {
    const NxF64 * ret = 0;

    assert( mUseDouble );

    ret = mKdTree.getVerticesDouble();

    return ret;
  }

  const NxF32 *   getVertexFloat(NxU32 index) const
  {
    const NxF32 * ret  = 0;
    assert( !mUseDouble );
#ifdef _DEBUG
    NxU32 vcount = mKdTree.getVcount();
    assert( index < vcount );
#endif
    ret =  mKdTree.getVerticesFloat();
    ret = &ret[index*3];
    return ret;
  }

  const NxF64 *   getVertexDouble(NxU32 index) const
  {
    const NxF64 * ret = 0;
    assert( mUseDouble );
#ifdef _DEBUG
    NxU32 vcount = mKdTree.getVcount();
    assert( index < vcount );
#endif
    ret =  mKdTree.getVerticesDouble();
    ret = &ret[index*3];

    return ret;
  }

  NxU32    getVcount(void) const
  {
    return mKdTree.getVcount();
  }

  bool isDouble(void) const
  {
    return mUseDouble;
  }


  bool            saveAsObj(const char *fname,NxU32 tcount,NxU32 *indices)
  {
    bool ret = false;


    FILE *fph = fopen(fname,"wb");
    if ( fph )
    {
      ret = true;

      NxU32 vcount    = getVcount();
      if ( mUseDouble )
      {
        const NxF64 *v  = getVerticesDouble();
        for (NxU32 i=0; i<vcount; i++)
        {
          fprintf(fph,"v %0.9f %0.9f %0.9f\r\n", (NxF32)v[0], (NxF32)v[1], (NxF32)v[2] );
          v+=3;
        }
      }
      else
      {
        const NxF32 *v  = getVerticesFloat();
        for (NxU32 i=0; i<vcount; i++)
        {
          fprintf(fph,"v %0.9f %0.9f %0.9f\r\n", v[0], v[1], v[2] );
          v+=3;
        }
      }

      for (NxU32 i=0; i<tcount; i++)
      {
        NxU32 i1 = *indices++;
        NxU32 i2 = *indices++;
        NxU32 i3 = *indices++;
        fprintf(fph,"f %d %d %d\r\n", i1+1, i2+1, i3+1 );
      }
      fclose(fph);
    }

    return ret;
  }

private:
  bool    mUseDouble:1;
  bool    mSnapToGrid:1;
  NxF64  mDoubleGranularity;
  NxF32   mFloatGranularity;
  VERTEX_INDEX::KdTree  mKdTree;
};

fm_VertexIndex * fm_createVertexIndex(NxF64 granularity,bool snapToGrid) // create an indexed vertex system for doubles
{
  MyVertexIndex *ret = MEMALLOC_NEW(MyVertexIndex)(granularity,snapToGrid);
  return static_cast< fm_VertexIndex *>(ret);
}

fm_VertexIndex * fm_createVertexIndex(NxF32 granularity,bool snapToGrid)  // create an indexed vertext system for floats
{
  MyVertexIndex *ret = MEMALLOC_NEW(MyVertexIndex)(granularity,snapToGrid);
  return static_cast< fm_VertexIndex *>(ret);
}

void          fm_releaseVertexIndex(fm_VertexIndex *vindex)
{
  MyVertexIndex *m = static_cast< MyVertexIndex *>(vindex);
  delete m;
}

#endif   // END OF VERTEX WELDING CODE


//**********************************************************
//**********************************************************
//**** LineSweep Line-Segment Intersection Code
//**********************************************************
//**********************************************************

//#ifndef LINE_SWEEP_H
#if 0

#define LINE_SWEEP_H

class fm_quickSort
{
public:
	void qsort(void **base,NxI32 num); // perform the qsort.
protected:
  // -1 less, 0 equal, +1 greater.
	virtual NxI32 compare(void **p1,void **p2) = 0;
private:
	void inline swap(char **a,char **b);
};


void fm_quickSort::swap(char **a,char **b)
{
	char *tmp;

	if ( a != b )
	{
		tmp = *a;
		*a++ = *b;
		*b++ = tmp;
	}
}


void fm_quickSort::qsort(void **b,NxI32 num)
{
	char *lo,*hi;
	char *mid;
	char *bottom, *top;
	NxI32 size;
	char *lostk[30], *histk[30];
	NxI32 stkptr;
	char **base = (char **)b;

	if (num < 2 ) return;

	stkptr = 0;

	lo = (char *)base;
	hi = (char *)base + sizeof(char **) * (num-1);

nextone:

	size = (NxI32)(hi - lo) / sizeof(char**) + 1;

	mid = lo + (size / 2) * sizeof(char **);
	swap((char **)mid,(char **)lo);
	bottom = lo;
	top = hi + sizeof(char **);

	for (;;)
	{
		do
		{
			bottom += sizeof(char **);
		} while (bottom <= hi && compare((void **)bottom,(void **)lo) <= 0);

		do
		{
			top -= sizeof(char **);
		} while (top > lo && compare((void **)top,(void **)lo) >= 0);

		if (top < bottom) break;

		swap((char **)bottom,(char **)top);

	}

	swap((char **)lo,(char **)top);

	if ( top - 1 - lo >= hi - bottom )
	{
		if (lo + sizeof(char **) < top)
		{
			lostk[stkptr] = lo;
			histk[stkptr] = top - sizeof(char **);
			stkptr++;
		}
		if (bottom < hi)
		{
			lo = bottom;
			goto nextone;
		}
	}
	else
	{
		if ( bottom < hi )
		{
			lostk[stkptr] = bottom;
			histk[stkptr] = hi;
			stkptr++;
		}
		if (lo + sizeof(char **) < top)
		{
			hi = top - sizeof(char **);
			goto nextone; 					/* do small recursion */
		}
	}

	stkptr--;

	if (stkptr >= 0)
	{
		lo = lostk[stkptr];
		hi = histk[stkptr];
		goto nextone;
	}
	return;
}


typedef CONVEX_DECOMPOSITION::Array< fm_LineSegment > LineSegmentVector;

static inline void setMinMax(NxF64 &vmin,NxF64 &vmax,NxF64 v1,NxF64 v2)
{
  if ( v1 <= v2 )
  {
    vmin = v1;
    vmax = v2;
  }
  else
  {
    vmin = v2;
    vmax = v1;
  }
}


class Intersection
{
public:
  Intersection(void)
  {
    mIndex = 0;
    mTime = 0;
  }
  Intersection(NxF64 time,const NxF64 *from,const NxF64 *to,fm_VertexIndex *vpool)
  {
    mTime = time;
    NxF64 pos[3];
    pos[0] = (to[0]-from[0])*time+from[0];
    pos[1] = (to[1]-from[1])*time+from[1];
    pos[2] = (to[2]-from[2])*time+from[2];
    bool newPos;
    mIndex = vpool->getIndex(pos,newPos);
  }

  NxU32    mIndex;
  NxF64    mTime;
};


typedef CONVEX_DECOMPOSITION::Array< Intersection > IntersectionList;

class MyLineSegment : public fm_LineSegment, public Memalloc
{
public:

  void init(const fm_LineSegment &s,fm_VertexIndex *vpool,NxU32 x)
  {
    fm_LineSegment *dest = static_cast< fm_LineSegment *>(this);
    *dest = s;

    mFlipped = false;

    const NxF64 *p1 = vpool->getVertexDouble(mE1);
    const NxF64 *p2 = vpool->getVertexDouble(mE2);

    setMinMax(mMin[0],mMax[0],p1[0],p2[0]);
    setMinMax(mMin[1],mMax[1],p1[1],p2[1]);
    setMinMax(mMin[2],mMax[2],p1[2],p2[2]);

    if ( p1[x] <= p2[x] )
    {
      mFrom[0] = p1[0];
      mFrom[1] = p1[1];
      mFrom[2] = p1[2];

      mTo[0]   = p2[0];
      mTo[1]   = p2[1];
      mTo[2]   = p2[2];
    }
    else
    {
      mFrom[0] = p2[0];
      mFrom[1] = p2[1];
      mFrom[2] = p2[2];

      mTo[0]   = p1[0];
      mTo[1]   = p1[1];
      mTo[2]   = p1[2];

      mFlipped = true;

      swap(mE1,mE2);
    }

  }

  // we already know that the x-extent overlaps or we wouldn't be in this routine..
  void intersect(MyLineSegment *segment,NxU32 x,NxU32 y,NxU32 /* z */,fm_VertexIndex *vpool)
  {
    NxU32 count = 0;

    // if the two segments share any start/end points then they cannot intersect at all!

    if ( segment->mE1 == mE1 || segment->mE1 == mE2 ) count++;
    if ( segment->mE2 == mE1 || segment->mE2 == mE2 ) count++;

    if ( count == 0 )
    {
      if ( mMax[y] < segment->mMin[y] ) // no intersection...
      {

      }
      else if ( mMin[y] > segment->mMax[y] ) // no intersection
      {

      }
      else
      {

        NxF64 a1[2];
        NxF64 a2[2];
        NxF64 b1[2];
        NxF64 b2[2];

        a1[0] = mFrom[x];
        a1[1] = mFrom[y];

        a2[0] = mTo[x];
        a2[1] = mTo[y];

        b1[0] = segment->mFrom[x];
        b1[1] = segment->mFrom[y];

        b2[0] = segment->mTo[x];
        b2[1] = segment->mTo[y];


        NxF64 t1,t2;
        IntersectResult result = fm_intersectLineSegments2dTime(a1,a2,b1,b2,t1,t2);

        if ( result == IR_DO_INTERSECT )
        {
          addIntersect(t1,vpool);
          segment->addIntersect(t2,vpool);
        }


      }
    }
  }

  void addIntersect(NxF64 time,fm_VertexIndex *vpool)
  {
    Intersection intersect(time,mFrom,mTo,vpool);

    if ( mE1 == intersect.mIndex || mE2 == intersect.mIndex )
    {
      //printf("Split too close to the beginning or the end of the line segment.\r\n");
    }
    else
    {
      if ( mIntersections.empty() )
      {
        mIntersections.pushBack(intersect);
      }
      else
      {
        IntersectionList::Iterator i;
        for (i=mIntersections.begin(); i!=mIntersections.end(); ++i)
        {
          Intersection &it = (*i);
          if ( it.mIndex == intersect.mIndex )
          {
            //printf("Duplicate Intersection, throwing it away.\r\n");
            break;
          }
          else
          {
            if ( it.mTime > time )
            {
//*** TODO TODO TODO              mIntersections.insert(i,intersect);
              break;
            }
          }
        }
        if ( i==mIntersections.end() )
        {
          mIntersections.pushBack(intersect);
        }
      }
    }
  }

  void getResults(LineSegmentVector &results)
  {
    if ( mIntersections.empty() )
    {
      fm_LineSegment seg(mE1,mE2);
      if ( mFlipped )
      {
        swap(seg.mE1,seg.mE2);
      }
      results.pushBack(seg);
    }
    else
    {
      NxU32 prev = mE1;
      IntersectionList::Iterator i;
      for (i=mIntersections.begin(); i!=mIntersections.end(); ++i)
      {
        Intersection &it = (*i);
        fm_LineSegment seg(prev,it.mIndex);
        if ( mFlipped )
        {
          swap(seg.mE1,seg.mE2);
        }
        results.pushBack(seg);
        prev = it.mIndex;
      }
      fm_LineSegment seg(prev,mE2);
      if ( mFlipped )
      {
        swap(seg.mE1,seg.mE2);
      }
      results.pushBack(seg);
    }
  }

  void swap(NxU32 &a,NxU32 &b)
  {
    NxU32 temp = a;
    a = b;
    b = temp;
  }

  bool             mFlipped;
  NxF64            mFrom[3];
  NxF64            mTo[3];
  NxF64            mMin[3];
  NxF64            mMax[3];
  IntersectionList mIntersections;
};

typedef CONVEX_DECOMPOSITION::Array< MyLineSegment > MyLineSegmentVector;

class MyLineSweep : public fm_LineSweep, public fm_quickSort, public Memalloc
{
public:
  virtual ~MyLineSweep(void)
  {

  }
  fm_LineSegment * performLineSweep(const fm_LineSegment *segments,NxU32 icount,const NxF64 *planeEquation,fm_VertexIndex *pool,NxU32 &scount)
  {
    fm_LineSegment *ret = 0;

    FM_Axis axis = fm_getDominantAxis(planeEquation);
    switch ( axis )
    {
      case FM_XAXIS:
        mX = 1;
        mY = 2;
        mZ = 0;
        break;
      case FM_YAXIS:
        mX = 0;
        mY = 2;
        mZ = 1;
        break;
      case FM_ZAXIS:
        mX = 0;
        mY = 1;
        mZ = 2;
        break;
    }


    mResults.clear();
    scount = 0;

    MyLineSegment *mls   = MEMALLOC_NEW(MyLineSegment)[icount];
    MyLineSegment **mptr = (MyLineSegment **)MEMALLOC_MALLOC(sizeof(MyLineSegment *)*icount);

    for (NxU32 i=0; i<icount; i++)
    {
      mls[i].init(segments[i],pool,mX);
      mptr[i] = &mls[i];
    }

    qsort((void **)mptr,(NxI32)icount);

    for (NxU32 i=0; i<icount; i++)
    {
      MyLineSegment *segment = mptr[i];
      NxF64 esegment = segment->mTo[mX];
      for (NxU32 j=i+1; j<icount; j++)
      {
        MyLineSegment *test = mptr[j];
        if ( test->mFrom[mX] >= esegment )
        {
          break;
        }
        else
        {
          test->intersect(segment,mX,mY,mZ,pool);
        }
      }
    }

    for (NxU32 i=0; i<icount; i++)
    {
      MyLineSegment *segment = mptr[i];
      segment->getResults(mResults);
    }


    delete []mls;
    MEMALLOC_FREE(mptr);

    if ( !mResults.empty() )
    {
      scount = (NxU32)mResults.size();
      ret = &mResults[0];
    }

    return ret;
  }

	NxI32 compare(void **p1,void **p2)
  {
    NxI32 ret = 0;

    MyLineSegment **m1 = (MyLineSegment **) p1;
    MyLineSegment **m2 = (MyLineSegment **) p2;

    MyLineSegment *s1 = *m1;
    MyLineSegment *s2 = *m2;

    if ( s1->mFrom[mX] < s2->mFrom[mX] )
      ret = -1;
    else if ( s1->mFrom[mX] > s2->mFrom[mX] )
      ret = 1;
    else if ( s1->mFrom[mY] < s2->mFrom[mY] )
      ret = -1;
    else if ( s1->mFrom[mY] > s2->mFrom[mY] )
      ret = 1;

    return ret;
  }

  NxU32              mX;  // index for the x-axis
  NxU32              mY;  // index for the y-axis
  NxU32              mZ;
  fm_VertexIndex        *mfm_VertexIndex;
  LineSegmentVector  mResults;
};


fm_LineSweep * fm_createLineSweep(void)
{
  MyLineSweep *mls = MEMALLOC_NEW(MyLineSweep);
  return static_cast< fm_LineSweep *>(mls);
}

void        fm_releaseLineSweep(fm_LineSweep *sweep)
{
  MyLineSweep *mls = static_cast< MyLineSweep *>(sweep);
  delete mls;
}



#endif  // End of LineSweep code




REAL fm_computeBestFitAABB(NxU32 vcount,const REAL *points,NxU32 pstride,REAL *bmin,REAL *bmax) // returns the diagonal distance
{

  const NxU8 *source = (const NxU8 *) points;

	bmin[0] = points[0];
	bmin[1] = points[1];
	bmin[2] = points[2];

	bmax[0] = points[0];
	bmax[1] = points[1];
	bmax[2] = points[2];


  for (NxU32 i=1; i<vcount; i++)
  {
  	source+=pstride;
  	const REAL *p = (const REAL *) source;

  	if ( p[0] < bmin[0] ) bmin[0] = p[0];
  	if ( p[1] < bmin[1] ) bmin[1] = p[1];
  	if ( p[2] < bmin[2] ) bmin[2] = p[2];

		if ( p[0] > bmax[0] ) bmax[0] = p[0];
		if ( p[1] > bmax[1] ) bmax[1] = p[1];
		if ( p[2] > bmax[2] ) bmax[2] = p[2];

  }

  REAL dx = bmax[0] - bmin[0];
  REAL dy = bmax[1] - bmin[1];
  REAL dz = bmax[2] - bmin[2];

	return (REAL) sqrt( dx*dx + dy*dy + dz*dz );

}



/* a = b - c */
#define vector(a,b,c) \
	(a)[0] = (b)[0] - (c)[0];	\
	(a)[1] = (b)[1] - (c)[1];	\
	(a)[2] = (b)[2] - (c)[2];



#define innerProduct(v,q) \
		((v)[0] * (q)[0] + \
		(v)[1] * (q)[1] + \
		(v)[2] * (q)[2])

#define crossProduct(a,b,c) \
	(a)[0] = (b)[1] * (c)[2] - (c)[1] * (b)[2]; \
	(a)[1] = (b)[2] * (c)[0] - (c)[2] * (b)[0]; \
	(a)[2] = (b)[0] * (c)[1] - (c)[0] * (b)[1];


bool fm_lineIntersectsTriangle(const REAL *rayStart,const REAL *rayEnd,const REAL *p1,const REAL *p2,const REAL *p3,REAL *sect)
{
	REAL dir[3];

  dir[0] = rayEnd[0] - rayStart[0];
  dir[1] = rayEnd[1] - rayStart[1];
  dir[2] = rayEnd[2] - rayStart[2];

  REAL d = (REAL)sqrt(dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2]);
  REAL r = 1.0f / d;

  dir[0]*=r;
  dir[1]*=r;
  dir[2]*=r;


  REAL t;

	bool ret = fm_rayIntersectsTriangle(rayStart, dir, p1, p2, p3, t );

	if ( ret )
	{
		if ( t > d )
		{
			sect[0] = rayStart[0] + dir[0]*t;
			sect[1] = rayStart[1] + dir[1]*t;
			sect[2] = rayStart[2] + dir[2]*t;
		}
		else
		{
			ret = false;
		}
	}

  return ret;
}



bool fm_rayIntersectsTriangle(const REAL *p,const REAL *d,const REAL *v0,const REAL *v1,const REAL *v2,REAL &t)
{
	REAL e1[3],e2[3],h[3],s[3],q[3];
	REAL a,f,u,v;

	vector(e1,v1,v0);
	vector(e2,v2,v0);
	crossProduct(h,d,e2);
	a = innerProduct(e1,h);

	if (a > -0.00001 && a < 0.00001)
		return(false);

	f = 1/a;
	vector(s,p,v0);
	u = f * (innerProduct(s,h));

	if (u < 0.0 || u > 1.0)
		return(false);

	crossProduct(q,s,e1);
	v = f * innerProduct(d,q);
	if (v < 0.0 || u + v > 1.0)
		return(false);
	// at this stage we can compute t to find out where
	// the intersection point is on the line
	t = f * innerProduct(e2,q);
	if (t > 0) // ray intersection
		return(true);
	else // this means that there is a line intersection
		 // but not a ray intersection
		 return (false);
}


inline REAL det(const REAL *p1,const REAL *p2,const REAL *p3)
{
  return  p1[0]*p2[1]*p3[2] + p2[0]*p3[1]*p1[2] + p3[0]*p1[1]*p2[2] -p1[0]*p3[1]*p2[2] - p2[0]*p1[1]*p3[2] - p3[0]*p2[1]*p1[2];
}


REAL  fm_computeMeshVolume(const REAL *vertices,NxU32 tcount,const NxU32 *indices)
{
	REAL volume = 0;

	for (NxU32 i=0; i<tcount; i++,indices+=3)
	{
  	const REAL *p1 = &vertices[ indices[0]*3 ];
		const REAL *p2 = &vertices[ indices[1]*3 ];
		const REAL *p3 = &vertices[ indices[2]*3 ];
		volume+=det(p1,p2,p3); // compute the volume of the tetrahedran relative to the origin.
	}

	volume*=(1.0f/6.0f);
	if ( volume < 0 )
		volume*=-1;
	return volume;
}


const REAL * fm_getPoint(const REAL *points,NxU32 pstride,NxU32 index)
{
  const NxU8 *scan = (const NxU8 *)points;
  scan+=(index*pstride);
  return (REAL *)scan;
}


bool fm_insideTriangle(REAL Ax, REAL Ay,
                      REAL Bx, REAL By,
                      REAL Cx, REAL Cy,
                      REAL Px, REAL Py)

{
  REAL ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
  REAL cCROSSap, bCROSScp, aCROSSbp;

  ax = Cx - Bx;  ay = Cy - By;
  bx = Ax - Cx;  by = Ay - Cy;
  cx = Bx - Ax;  cy = By - Ay;
  apx= Px - Ax;  apy= Py - Ay;
  bpx= Px - Bx;  bpy= Py - By;
  cpx= Px - Cx;  cpy= Py - Cy;

  aCROSSbp = ax*bpy - ay*bpx;
  cCROSSap = cx*apy - cy*apx;
  bCROSScp = bx*cpy - by*cpx;

  return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
}


REAL fm_areaPolygon2d(NxU32 pcount,const REAL *points,NxU32 pstride)
{
  NxI32 n = (NxI32)pcount;

  REAL A=0.0f;
  for(NxI32 p=n-1,q=0; q<n; p=q++)
  {
    const REAL *p1 = fm_getPoint(points,pstride,p);
    const REAL *p2 = fm_getPoint(points,pstride,q);
    A+= p1[0]*p2[1] - p2[0]*p1[1];
  }
  return A*0.5f;
}


bool  fm_pointInsidePolygon2d(NxU32 pcount,const REAL *points,NxU32 pstride,const REAL *point,NxU32 xindex,NxU32 yindex)
{
  NxU32 j = pcount-1;
  NxI32 oddNodes = 0;

  REAL x = point[xindex];
  REAL y = point[yindex];

  for (NxU32 i=0; i<pcount; i++)
  {
    const REAL *p1 = fm_getPoint(points,pstride,i);
    const REAL *p2 = fm_getPoint(points,pstride,j);

    REAL x1 = p1[xindex];
    REAL y1 = p1[yindex];

    REAL x2 = p2[xindex];
    REAL y2 = p2[yindex];

    if ( y1 < y && y2 >= y ||  y2 < y && y1 >= y )
    {
      if (x1+(y-y1)/(y2-y1)*(x2-x1)<x)
      {
        oddNodes = 1-oddNodes;
      }
    }
    j = i;
  }

  return oddNodes ? true : false;
}


NxU32 fm_consolidatePolygon(NxU32 pcount,const REAL *points,NxU32 pstride,REAL *_dest,REAL epsilon) // collapses co-linear edges.
{
  NxU32 ret = 0;


  if ( pcount >= 3 )
  {
    const REAL *prev = fm_getPoint(points,pstride,pcount-1);
    const REAL *current = points;
    const REAL *next    = fm_getPoint(points,pstride,1);
    REAL *dest = _dest;

    for (NxU32 i=0; i<pcount; i++)
    {

      next = (i+1)==pcount ? points : next;

      if ( !fm_colinear(prev,current,next,epsilon) )
      {
        dest[0] = current[0];
        dest[1] = current[1];
        dest[2] = current[2];

        dest+=3;
        ret++;
      }

      prev = current;
      current+=3;
      next+=3;

    }
  }

  return ret;
}


#ifndef RECT3D_TEMPLATE

#define RECT3D_TEMPLATE

template <class T> class Rect3d
{
public:
  Rect3d(void) { };

  Rect3d(const T *bmin,const T *bmax)
  {

    mMin[0] = bmin[0];
    mMin[1] = bmin[1];
    mMin[2] = bmin[2];

    mMax[0] = bmax[0];
    mMax[1] = bmax[1];
    mMax[2] = bmax[2];

  }

  void SetMin(const T *bmin)
  {
    mMin[0] = bmin[0];
    mMin[1] = bmin[1];
    mMin[2] = bmin[2];
  }

  void SetMax(const T *bmax)
  {
    mMax[0] = bmax[0];
    mMax[1] = bmax[1];
    mMax[2] = bmax[2];
  }

	void SetMin(T x,T y,T z)
	{
		mMin[0] = x;
		mMin[1] = y;
		mMin[2] = z;
	}

	void SetMax(T x,T y,T z)
	{
		mMax[0] = x;
		mMax[1] = y;
		mMax[2] = z;
	}

  T mMin[3];
  T mMax[3];
};

#endif

void splitRect(NxU32 axis,
						   const Rect3d<REAL> &source,
							 Rect3d<REAL> &b1,
							 Rect3d<REAL> &b2,
							 const REAL *midpoint)
{
	switch ( axis )
	{
		case 0:
			b1.SetMin(source.mMin);
			b1.SetMax( midpoint[0], source.mMax[1], source.mMax[2] );

			b2.SetMin( midpoint[0], source.mMin[1], source.mMin[2] );
			b2.SetMax(source.mMax);

			break;
		case 1:
			b1.SetMin(source.mMin);
			b1.SetMax( source.mMax[0], midpoint[1], source.mMax[2] );

			b2.SetMin( source.mMin[0], midpoint[1], source.mMin[2] );
			b2.SetMax(source.mMax);

			break;
		case 2:
			b1.SetMin(source.mMin);
			b1.SetMax( source.mMax[0], source.mMax[1], midpoint[2] );

			b2.SetMin( source.mMin[0], source.mMin[1], midpoint[2] );
			b2.SetMax(source.mMax);

			break;
	}
}

bool fm_computeSplitPlane(NxU32 vcount,
                          const REAL *vertices,
                          NxU32 /* tcount */,
                          const NxU32 * /* indices */,
                          REAL *plane)
{

  REAL sides[3];
  REAL matrix[16];

  fm_computeBestFitOBB( vcount, vertices, sizeof(REAL)*3, sides, matrix );

  REAL bmax[3];
  REAL bmin[3];

  bmax[0] = sides[0]*0.5f;
  bmax[1] = sides[1]*0.5f;
  bmax[2] = sides[2]*0.5f;

  bmin[0] = -bmax[0];
  bmin[1] = -bmax[1];
  bmin[2] = -bmax[2];


  REAL dx = sides[0];
  REAL dy = sides[1];
  REAL dz = sides[2];


	REAL laxis = dx;

	NxU32 axis = 0;

	if ( dy > dx )
	{
		axis = 1;
		laxis = dy;
	}

	if ( dz > dx && dz > dy )
	{
		axis = 2;
		laxis = dz;
	}

  REAL p1[3];
  REAL p2[3];
  REAL p3[3];

  p3[0] = p2[0] = p1[0] = bmin[0] + dx*0.5f;
  p3[1] = p2[1] = p1[1] = bmin[1] + dy*0.5f;
  p3[2] = p2[2] = p1[2] = bmin[2] + dz*0.5f;

  Rect3d<REAL> b(bmin,bmax);

  Rect3d<REAL> b1,b2;

  splitRect(axis,b,b1,b2,p1);


  switch ( axis )
  {
    case 0:
      p2[1] = bmin[1];
      p2[2] = bmin[2];

      if ( dz > dy )
      {
        p3[1] = bmax[1];
        p3[2] = bmin[2];
      }
      else
      {
        p3[1] = bmin[1];
        p3[2] = bmax[2];
      }

      break;
    case 1:
      p2[0] = bmin[0];
      p2[2] = bmin[2];

      if ( dx > dz )
      {
        p3[0] = bmax[0];
        p3[2] = bmin[2];
      }
      else
      {
        p3[0] = bmin[0];
        p3[2] = bmax[2];
      }

      break;
    case 2:
      p2[0] = bmin[0];
      p2[1] = bmin[1];

      if ( dx > dy )
      {
        p3[0] = bmax[0];
        p3[1] = bmin[1];
      }
      else
      {
        p3[0] = bmin[0];
        p3[1] = bmax[1];
      }

      break;
  }

  REAL tp1[3];
  REAL tp2[3];
  REAL tp3[3];

  fm_transform(matrix,p1,tp1);
  fm_transform(matrix,p2,tp2);
  fm_transform(matrix,p3,tp3);

	plane[3] = fm_computePlane(tp1,tp2,tp3,plane);

  return true;

}

#pragma warning(disable:4100)

void fm_nearestPointInTriangle(const REAL *nearestPoint,const REAL *p1,const REAL *p2,const REAL *p3,REAL *nearest)
{

}

static REAL Partial(const REAL *a,const REAL *p) 
{
	return (a[0]*p[1]) - (p[0]*a[1]);
}

REAL  fm_areaTriangle(const REAL *p0,const REAL *p1,const REAL *p2)
{
  REAL A = Partial(p0,p1);
	A+= Partial(p1,p2);
	A+= Partial(p2,p0);
	return A*0.5f;
}

void fm_subtract(const REAL *A,const REAL *B,REAL *diff) // compute A-B and store the result in 'diff'
{
  diff[0] = A[0]-B[0];
  diff[1] = A[1]-B[1];
  diff[2] = A[2]-B[2];
}


void  fm_multiplyTransform(const REAL *pA,const REAL *pB,REAL *pM)
{

  REAL a = pA[0*4+0] * pB[0*4+0] + pA[0*4+1] * pB[1*4+0] + pA[0*4+2] * pB[2*4+0] + pA[0*4+3] * pB[3*4+0];
  REAL b = pA[0*4+0] * pB[0*4+1] + pA[0*4+1] * pB[1*4+1] + pA[0*4+2] * pB[2*4+1] + pA[0*4+3] * pB[3*4+1];
  REAL c = pA[0*4+0] * pB[0*4+2] + pA[0*4+1] * pB[1*4+2] + pA[0*4+2] * pB[2*4+2] + pA[0*4+3] * pB[3*4+2];
  REAL d = pA[0*4+0] * pB[0*4+3] + pA[0*4+1] * pB[1*4+3] + pA[0*4+2] * pB[2*4+3] + pA[0*4+3] * pB[3*4+3];

  REAL e = pA[1*4+0] * pB[0*4+0] + pA[1*4+1] * pB[1*4+0] + pA[1*4+2] * pB[2*4+0] + pA[1*4+3] * pB[3*4+0];
  REAL f = pA[1*4+0] * pB[0*4+1] + pA[1*4+1] * pB[1*4+1] + pA[1*4+2] * pB[2*4+1] + pA[1*4+3] * pB[3*4+1];
  REAL g = pA[1*4+0] * pB[0*4+2] + pA[1*4+1] * pB[1*4+2] + pA[1*4+2] * pB[2*4+2] + pA[1*4+3] * pB[3*4+2];
  REAL h = pA[1*4+0] * pB[0*4+3] + pA[1*4+1] * pB[1*4+3] + pA[1*4+2] * pB[2*4+3] + pA[1*4+3] * pB[3*4+3];

  REAL i = pA[2*4+0] * pB[0*4+0] + pA[2*4+1] * pB[1*4+0] + pA[2*4+2] * pB[2*4+0] + pA[2*4+3] * pB[3*4+0];
  REAL j = pA[2*4+0] * pB[0*4+1] + pA[2*4+1] * pB[1*4+1] + pA[2*4+2] * pB[2*4+1] + pA[2*4+3] * pB[3*4+1];
  REAL k = pA[2*4+0] * pB[0*4+2] + pA[2*4+1] * pB[1*4+2] + pA[2*4+2] * pB[2*4+2] + pA[2*4+3] * pB[3*4+2];
  REAL l = pA[2*4+0] * pB[0*4+3] + pA[2*4+1] * pB[1*4+3] + pA[2*4+2] * pB[2*4+3] + pA[2*4+3] * pB[3*4+3];

  REAL m = pA[3*4+0] * pB[0*4+0] + pA[3*4+1] * pB[1*4+0] + pA[3*4+2] * pB[2*4+0] + pA[3*4+3] * pB[3*4+0];
  REAL n = pA[3*4+0] * pB[0*4+1] + pA[3*4+1] * pB[1*4+1] + pA[3*4+2] * pB[2*4+1] + pA[3*4+3] * pB[3*4+1];
  REAL o = pA[3*4+0] * pB[0*4+2] + pA[3*4+1] * pB[1*4+2] + pA[3*4+2] * pB[2*4+2] + pA[3*4+3] * pB[3*4+2];
  REAL p = pA[3*4+0] * pB[0*4+3] + pA[3*4+1] * pB[1*4+3] + pA[3*4+2] * pB[2*4+3] + pA[3*4+3] * pB[3*4+3];

  pM[0] = a;  pM[1] = b;  pM[2] = c;  pM[3] = d;

  pM[4] = e;  pM[5] = f;  pM[6] = g;  pM[7] = h;

  pM[8] = i;  pM[9] = j;  pM[10] = k;  pM[11] = l;

  pM[12] = m;  pM[13] = n;  pM[14] = o;  pM[15] = p;
}

void fm_multiply(REAL *A,REAL scaler)
{
  A[0]*=scaler;
  A[1]*=scaler;
  A[2]*=scaler;
}

void fm_add(const REAL *A,const REAL *B,REAL *sum)
{
  sum[0] = A[0]+B[0];
  sum[1] = A[1]+B[1];
  sum[2] = A[2]+B[2];
}

void fm_copy3(const REAL *source,REAL *dest)
{
  dest[0] = source[0];
  dest[1] = source[1];
  dest[2] = source[2];
}


NxU32  fm_copyUniqueVertices(NxU32 vcount,const REAL *input_vertices,REAL *output_vertices,NxU32 tcount,const NxU32 *input_indices,NxU32 *output_indices)
{
  NxU32 ret = 0;

  REAL *vertices = (REAL *)MEMALLOC_MALLOC(sizeof(REAL)*vcount*3);
  memcpy(vertices,input_vertices,sizeof(REAL)*vcount*3);
  REAL *dest = output_vertices;

  NxU32 *reindex = (NxU32 *)MEMALLOC_MALLOC(sizeof(NxU32)*vcount);
  memset(reindex,0xFF,sizeof(NxU32)*vcount);

  NxU32 icount = tcount*3;

  for (NxU32 i=0; i<icount; i++)
  {
    NxU32 index = *input_indices++;

    assert( index < vcount );

    if ( reindex[index] == 0xFFFFFFFF )
    {
      *output_indices++ = ret;
      reindex[index] = ret;
      const REAL *pos = &vertices[index*3];
      dest[0] = pos[0];
      dest[1] = pos[1];
      dest[2] = pos[2];
      dest+=3;
      ret++;
    }
    else
    {
      *output_indices++ = reindex[index];
    }
  }
  MEMALLOC_FREE(vertices);
  MEMALLOC_FREE(reindex);
  return ret;
}

bool    fm_isMeshCoplanar(NxU32 tcount,const NxU32 *indices,const REAL *vertices,bool doubleSided) // returns true if this collection of indexed triangles are co-planar!
{
  bool ret = true;

  if ( tcount > 0 )
  {
    NxU32 i1 = indices[0];
    NxU32 i2 = indices[1];
    NxU32 i3 = indices[2];
    const REAL *p1 = &vertices[i1*3];
    const REAL *p2 = &vertices[i2*3];
    const REAL *p3 = &vertices[i3*3];
    REAL plane[4];
    plane[3] = fm_computePlane(p1,p2,p3,plane);
    const NxU32 *scan = &indices[3];
    for (NxU32 i=1; i<tcount; i++)
    {
      i1 = *scan++;
      i2 = *scan++;
      i3 = *scan++;
      p1 = &vertices[i1*3];
      p2 = &vertices[i2*3];
      p3 = &vertices[i3*3];
      REAL _plane[4];
      _plane[3] = fm_computePlane(p1,p2,p3,_plane);
      if ( !fm_samePlane(plane,_plane,0.01f,0.001f,doubleSided) )
      {
        ret = false;
        break;
      }
    }
  }
  return ret;
}


bool fm_samePlane(const REAL p1[4],const REAL p2[4],REAL normalEpsilon,REAL dEpsilon,bool doubleSided)
{
  bool ret = false;

  REAL diff = (REAL) fabs(p1[3]-p2[3]);
  if ( diff < dEpsilon ) // if the plane -d  co-efficient is within our epsilon
  {
    REAL dot = fm_dot(p1,p2); // compute the dot-product of the vector normals.
    if ( doubleSided ) dot = (REAL)fabs(dot);
    REAL dmin = 1 - normalEpsilon;
    REAL dmax = 1 + normalEpsilon;
    if ( dot >= dmin && dot <= dmax )
    {
      ret = true; // then the plane equation is for practical purposes identical.
    }
  }

  return ret;
}


void  fm_initMinMax(REAL bmin[3],REAL bmax[3])
{
  bmin[0] = FLT_MAX;
  bmin[1] = FLT_MAX;
  bmin[2] = FLT_MAX;
  bmax[0] = FLT_MIN;
  bmax[1] = FLT_MIN;
  bmax[2] = FLT_MIN;
}


#ifndef TESSELATE_H

#define TESSELATE_H

typedef CONVEX_DECOMPOSITION::Array< NxU32 > UintVector;

class Myfm_Tesselate : public fm_Tesselate, public Memalloc
{
public:
  virtual ~Myfm_Tesselate(void)
  {

  }

  const NxU32 * tesselate(fm_VertexIndex *vindex,NxU32 tcount,const NxU32 *indices,NxF32 longEdge,NxU32 maxDepth,NxU32 &outcount)
  {
    const NxU32 *ret = 0;

    mMaxDepth = maxDepth;
    mLongEdge  = longEdge*longEdge;
    mLongEdgeD = mLongEdge;
    mVertices = vindex;

    if ( mVertices->isDouble() )
    {
      NxU32 vcount = mVertices->getVcount();
      NxF64 *vertices = (NxF64 *)MEMALLOC_MALLOC(sizeof(NxF64)*vcount*3);
      memcpy(vertices,mVertices->getVerticesDouble(),sizeof(NxF64)*vcount*3);

      for (NxU32 i=0; i<tcount; i++)
      {
        NxU32 i1 = *indices++;
        NxU32 i2 = *indices++;
        NxU32 i3 = *indices++;

        const NxF64 *p1 = &vertices[i1*3];
        const NxF64 *p2 = &vertices[i2*3];
        const NxF64 *p3 = &vertices[i3*3];

        tesselate(p1,p2,p3,0);

      }
      MEMALLOC_FREE(vertices);
    }
    else
    {
      NxU32 vcount = mVertices->getVcount();
      NxF32 *vertices = (NxF32 *)MEMALLOC_MALLOC(sizeof(NxF32)*vcount*3);
      memcpy(vertices,mVertices->getVerticesFloat(),sizeof(NxF32)*vcount*3);


      for (NxU32 i=0; i<tcount; i++)
      {
        NxU32 i1 = *indices++;
        NxU32 i2 = *indices++;
        NxU32 i3 = *indices++;

        const NxF32 *p1 = &vertices[i1*3];
        const NxF32 *p2 = &vertices[i2*3];
        const NxF32 *p3 = &vertices[i3*3];

        tesselate(p1,p2,p3,0);

      }
      MEMALLOC_FREE(vertices);
    }

    outcount = (NxU32)(mIndices.size()/3);
    ret = &mIndices[0];


    return ret;
  }

  void tesselate(const NxF32 *p1,const NxF32 *p2,const NxF32 *p3,NxU32 recurse)
  {
  	bool split = false;
  	NxF32 l1,l2,l3;

    l1 = l2 = l3 = 0;

  	if ( recurse < mMaxDepth )
  	{
  	  l1 = fm_distanceSquared(p1,p2);
    	l2 = fm_distanceSquared(p2,p3);
    	l3 = fm_distanceSquared(p3,p1);

  	  if (  l1 > mLongEdge || l2 > mLongEdge || l3 > mLongEdge )
  	  	split = true;

    }

    if ( split )
  	{
  		NxU32 edge;

  		if ( l1 >= l2 && l1 >= l3 )
  			edge = 0;
  		else if ( l2 >= l1 && l2 >= l3 )
  			edge = 1;
  		else
  			edge = 2;

			NxF32 split[3];

  		switch ( edge )
  		{
  			case 0:
  				{
            fm_lerp(p1,p2,split,0.5f);
            tesselate(p1,split,p3, recurse+1 );
            tesselate(split,p2,p3, recurse+1 );
  				}
  				break;
  			case 1:
  				{
            fm_lerp(p2,p3,split,0.5f);
            tesselate(p1,p2,split, recurse+1 );
            tesselate(p1,split,p3, recurse+1 );
  				}
  				break;
  			case 2:
  				{
  					fm_lerp(p3,p1,split,0.5f);
            tesselate(p1,p2,split, recurse+1 );
            tesselate(split,p2,p3, recurse+1 );
  				}
  				break;
  		}
  	}
  	else
  	{
      bool newp;

      NxU32 i1 = mVertices->getIndex(p1,newp);
      NxU32 i2 = mVertices->getIndex(p2,newp);
      NxU32 i3 = mVertices->getIndex(p3,newp);

      mIndices.pushBack(i1);
      mIndices.pushBack(i2);
      mIndices.pushBack(i3);
    }

  }

  void tesselate(const NxF64 *p1,const NxF64 *p2,const NxF64 *p3,NxU32 recurse)
  {
  	bool split = false;
  	NxF64 l1,l2,l3;

    l1 = l2 = l3 = 0;

  	if ( recurse < mMaxDepth )
  	{
  	  l1 = fm_distanceSquared(p1,p2);
    	l2 = fm_distanceSquared(p2,p3);
    	l3 = fm_distanceSquared(p3,p1);

  	  if (  l1 > mLongEdgeD || l2 > mLongEdgeD || l3 > mLongEdgeD )
  	  	split = true;

    }

    if ( split )
  	{
  		NxU32 edge;

  		if ( l1 >= l2 && l1 >= l3 )
  			edge = 0;
  		else if ( l2 >= l1 && l2 >= l3 )
  			edge = 1;
  		else
  			edge = 2;

			NxF64 split[3];

  		switch ( edge )
  		{
  			case 0:
  				{
            fm_lerp(p1,p2,split,0.5);
            tesselate(p1,split,p3, recurse+1 );
            tesselate(split,p2,p3, recurse+1 );
  				}
  				break;
  			case 1:
  				{
            fm_lerp(p2,p3,split,0.5);
            tesselate(p1,p2,split, recurse+1 );
            tesselate(p1,split,p3, recurse+1 );
  				}
  				break;
  			case 2:
  				{
  					fm_lerp(p3,p1,split,0.5);
            tesselate(p1,p2,split, recurse+1 );
            tesselate(split,p2,p3, recurse+1 );
  				}
  				break;
  		}
  	}
  	else
  	{
      bool newp;

      NxU32 i1 = mVertices->getIndex(p1,newp);
      NxU32 i2 = mVertices->getIndex(p2,newp);
      NxU32 i3 = mVertices->getIndex(p3,newp);

      mIndices.pushBack(i1);
      mIndices.pushBack(i2);
      mIndices.pushBack(i3);
    }

  }

private:
  NxF32           mLongEdge;
  NxF64          mLongEdgeD;
  fm_VertexIndex *mVertices;
  UintVector    mIndices;
  NxU32          mMaxDepth;
};

fm_Tesselate * fm_createTesselate(void)
{
  Myfm_Tesselate *m = MEMALLOC_NEW(Myfm_Tesselate);
  return static_cast< fm_Tesselate * >(m);
}

void           fm_releaseTesselate(fm_Tesselate *t)
{
  Myfm_Tesselate *m = static_cast< Myfm_Tesselate *>(t);
  delete m;
}

#endif


#ifndef RAY_ABB_INTERSECT

#define RAY_ABB_INTERSECT

//! Integer representation of a floating-point value.
#define IR(x)	((NxU32&)x)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
*	A method to compute a ray-AABB intersection.
*	Original code by Andrew Woo, from "Graphics Gems", Academic Press, 1990
*	Optimized code by Pierre Terdiman, 2000 (~20-30% faster on my Celeron 500)
*	Epsilon value added by Klaus Hartmann. (discarding it saves a few cycles only)
*
*	Hence this version is faster as well as more robust than the original one.
*
*	Should work provided:
*	1) the integer representation of 0.0f is 0x00000000
*	2) the sign bit of the NxF32 is the most significant one
*
*	Report bugs: p.terdiman@codercorner.com
*
*	\param		aabb		[in] the axis-aligned bounding box
*	\param		origin		[in] ray origin
*	\param		dir			[in] ray direction
*	\param		coord		[out] impact coordinates
*	\return		true if ray intersects AABB
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define RAYAABB_EPSILON 0.00001f
bool fm_intersectRayAABB(const NxF32 MinB[3],const NxF32 MaxB[3],const NxF32 origin[3],const NxF32 dir[3],NxF32 coord[3])
{
  bool Inside = true;
  NxF32 MaxT[3];
  MaxT[0]=MaxT[1]=MaxT[2]=-1.0f;

  // Find candidate planes.
  for(NxU32 i=0;i<3;i++)
  {
    if(origin[i] < MinB[i])
    {
      coord[i]	= MinB[i];
      Inside		= false;

      // Calculate T distances to candidate planes
      if(IR(dir[i]))	MaxT[i] = (MinB[i] - origin[i]) / dir[i];
    }
    else if(origin[i] > MaxB[i])
    {
      coord[i]	= MaxB[i];
      Inside		= false;

      // Calculate T distances to candidate planes
      if(IR(dir[i]))	MaxT[i] = (MaxB[i] - origin[i]) / dir[i];
    }
  }

  // Ray origin inside bounding box
  if(Inside)
  {
    coord[0] = origin[0];
    coord[1] = origin[1];
    coord[2] = origin[2];
    return true;
  }

  // Get largest of the maxT's for final choice of intersection
  NxU32 WhichPlane = 0;
  if(MaxT[1] > MaxT[WhichPlane])	WhichPlane = 1;
  if(MaxT[2] > MaxT[WhichPlane])	WhichPlane = 2;

  // Check final candidate actually inside box
  if(IR(MaxT[WhichPlane])&0x80000000) return false;

  for(NxU32 i=0;i<3;i++)
  {
    if(i!=WhichPlane)
    {
      coord[i] = origin[i] + MaxT[WhichPlane] * dir[i];
#ifdef RAYAABB_EPSILON
      if(coord[i] < MinB[i] - RAYAABB_EPSILON || coord[i] > MaxB[i] + RAYAABB_EPSILON)	return false;
#else
      if(coord[i] < MinB[i] || coord[i] > MaxB[i])	return false;
#endif
    }
  }
  return true;	// ray hits box
}

bool fm_intersectLineSegmentAABB(const NxF32 bmin[3],const NxF32 bmax[3],const NxF32 p1[3],const NxF32 p2[3],NxF32 intersect[3])
{
  bool ret = false;

  NxF32 dir[3];
  dir[0] = p2[0] - p1[0];
  dir[1] = p2[1] - p1[1];
  dir[2] = p2[2] - p1[2];
  NxF32 dist = fm_normalize(dir);
  if ( dist > RAYAABB_EPSILON )
  {
    ret = fm_intersectRayAABB(bmin,bmax,p1,dir,intersect);
    if ( ret )
    {
      NxF32 d = fm_distanceSquared(p1,intersect);
      if ( d  > (dist*dist) )
      {
        ret = false;
      }
    }
  }
  return ret;
}

#endif

#ifndef OBB_TO_AABB

#define OBB_TO_AABB

#pragma warning(disable:4100)
void    fm_OBBtoAABB(const NxF32 obmin[3],const NxF32 obmax[3],const NxF32 matrix[16],NxF32 abmin[3],NxF32 abmax[3])
{
  assert(0); // not yet implemented.
}


const REAL * computePos(NxU32 index,const REAL *vertices,NxU32 vstride)
{
  const char *tmp = (const char *)vertices;
  tmp+=(index*vstride);
  return (const REAL*)tmp;
}

void computeNormal(NxU32 index,REAL *normals,NxU32 nstride,const REAL *normal)
{
  char *tmp = (char *)normals;
  tmp+=(index*nstride);
  REAL *dest = (REAL *)tmp;
  dest[0]+=normal[0];
  dest[1]+=normal[1];
  dest[2]+=normal[2];
}

void fm_computeMeanNormals(NxU32 vcount,       // the number of vertices
                           const REAL *vertices,     // the base address of the vertex position data.
                           NxU32 vstride,      // the stride between position data.
                           REAL *normals,            // the base address  of the destination for mean vector normals
                           NxU32 nstride,      // the stride between normals
                           NxU32 tcount,       // the number of triangles
                           const NxU32 *indices)     // the triangle indices
{

  // Step #1 : Zero out the vertex normals
  char *dest = (char *)normals;
  for (NxU32 i=0; i<vcount; i++)
  {
    REAL *n = (REAL *)dest;
    n[0] = 0;
    n[1] = 0;
    n[2] = 0;
    dest+=nstride;
  }

  // Step #2 : Compute the face normals and accumulate them
  const NxU32 *scan = indices;
  for (NxU32 i=0; i<tcount; i++)
  {

    NxU32 i1 = *scan++;
    NxU32 i2 = *scan++;
    NxU32 i3 = *scan++;

    const REAL *p1 = computePos(i1,vertices,vstride);
    const REAL *p2 = computePos(i2,vertices,vstride);
    const REAL *p3 = computePos(i3,vertices,vstride);

    REAL normal[3];
    fm_computePlane(p3,p2,p1,normal);

    computeNormal(i1,normals,nstride,normal);
    computeNormal(i2,normals,nstride,normal);
    computeNormal(i3,normals,nstride,normal);
  }


  // Normalize the accumulated normals
  dest = (char *)normals;
  for (NxU32 i=0; i<vcount; i++)
  {
    REAL *n = (REAL *)dest;
    fm_normalize(n);
    dest+=nstride;
  }

}

#endif


#define BIGNUMBER 100000000.0  		/* hundred million */

static inline void Set(REAL *n,REAL x,REAL y,REAL z)
{
	n[0] = x;
	n[1] = y;
	n[2] = z;
};

static inline void Copy(REAL *dest,const REAL *source)
{
	dest[0] = source[0];
	dest[1] = source[1];
	dest[2] = source[2];
}


REAL  fm_computeBestFitSphere(NxU32 vcount,const REAL *points,NxU32 pstride,REAL *center)
{
  REAL radius;
  REAL radius2;

	REAL xmin[3];
	REAL xmax[3];
	REAL ymin[3];
	REAL ymax[3];
	REAL zmin[3];
	REAL zmax[3];
	REAL dia1[3];
	REAL dia2[3];

  /* FIRST PASS: find 6 minima/maxima points */
  Set(xmin,BIGNUMBER,BIGNUMBER,BIGNUMBER);
  Set(xmax,-BIGNUMBER,-BIGNUMBER,-BIGNUMBER);
  Set(ymin,BIGNUMBER,BIGNUMBER,BIGNUMBER);
  Set(ymax,-BIGNUMBER,-BIGNUMBER,-BIGNUMBER);
  Set(zmin,BIGNUMBER,BIGNUMBER,BIGNUMBER);
  Set(zmax,-BIGNUMBER,-BIGNUMBER,-BIGNUMBER);

  const char *scan = (const char *)points;


  for (NxU32 i=0; i<vcount; i++)
	{
		const REAL *caller_p = (const REAL *)scan;

   	if (caller_p[0]<xmin[0])
  	  Copy(xmin,caller_p); /* New xminimum point */
  	if (caller_p[0]>xmax[0])
  	  Copy(xmax,caller_p);
  	if (caller_p[1]<ymin[1])
  	  Copy(ymin,caller_p);
  	if (caller_p[1]>ymax[1])
  	  Copy(ymax,caller_p);
  	if (caller_p[2]<zmin[2])
  	  Copy(zmin,caller_p);
  	if (caller_p[2]>zmax[2])
  	  Copy(zmax,caller_p);

    scan+=pstride;
	}

  /* Set xspan = distance between the 2 points xmin & xmax (squared) */
  REAL dx = xmax[0] - xmin[0];
  REAL dy = xmax[1] - xmin[1];
  REAL dz = xmax[2] - xmin[2];
  REAL xspan = dx*dx + dy*dy + dz*dz;

  /* Same for y & z spans */
  dx = ymax[0] - ymin[0];
  dy = ymax[1] - ymin[1];
  dz = ymax[2] - ymin[2];
  REAL yspan = dx*dx + dy*dy + dz*dz;

  dx = zmax[0] - zmin[0];
  dy = zmax[1] - zmin[1];
  dz = zmax[2] - zmin[2];
  REAL zspan = dx*dx + dy*dy + dz*dz;

  /* Set points dia1 & dia2 to the maximally separated pair */
  Copy(dia1,xmin);
  Copy(dia2,xmax); /* assume xspan biggest */
  REAL maxspan = xspan;

  if (yspan>maxspan)
	{
	  maxspan = yspan;
  	Copy(dia1,ymin);
  	Copy(dia2,ymax);
	}

  if (zspan>maxspan)
	{
	  Copy(dia1,zmin);
	  Copy(dia2,zmax);
	}


  /* dia1,dia2 is a diameter of initial sphere */
  /* calc initial center */
  center[0] = (dia1[0]+dia2[0])*0.5f;
  center[1] = (dia1[1]+dia2[1])*0.5f;
  center[2] = (dia1[2]+dia2[2])*0.5f;

  /* calculate initial radius**2 and radius */

  dx = dia2[0]-center[0]; /* x component of radius vector */
  dy = dia2[1]-center[1]; /* y component of radius vector */
  dz = dia2[2]-center[2]; /* z component of radius vector */

  radius2 = dx*dx + dy*dy + dz*dz;
  radius = REAL(sqrt(radius2));

  /* SECOND PASS: increment current sphere */
  {
    const char *scan = (const char *)points;
	  for (NxU32 i=0; i<vcount; i++)
		{
			const REAL *caller_p = (const REAL *)scan;

  		dx = caller_p[0]-center[0];
		  dy = caller_p[1]-center[1];
  		dz = caller_p[2]-center[2];

		  REAL old_to_p_sq = dx*dx + dy*dy + dz*dz;

  		if (old_to_p_sq > radius2) 	/* do r**2 test first */
			{ 	/* this point is outside of current sphere */
	  		REAL old_to_p = REAL(sqrt(old_to_p_sq));
			  /* calc radius of new sphere */
  			radius = (radius + old_to_p) * 0.5f;
	  		radius2 = radius*radius; 	/* for next r**2 compare */
  			REAL old_to_new = old_to_p - radius;

	  		/* calc center of new sphere */

  		  REAL recip = 1.0f /old_to_p;

  			REAL cx = (radius*center[0] + old_to_new*caller_p[0]) * recip;
	  		REAL cy = (radius*center[1] + old_to_new*caller_p[1]) * recip;
			  REAL cz = (radius*center[2] + old_to_new*caller_p[2]) * recip;

		    Set(center,cx,cy,cz);

        scan+=pstride;
			}
		}
  }

  return radius;
}


void fm_computeBestFitCapsule(NxU32 vcount,const REAL *points,NxU32 pstride,REAL &radius,REAL &height,REAL matrix[16],bool bruteForce)
{
  REAL sides[3];
  REAL omatrix[16];
  fm_computeBestFitOBB(vcount,points,pstride,sides,omatrix,bruteForce);

  NxI32 axis = 0;
  if ( sides[0] > sides[1] && sides[0] > sides[2] )
    axis = 0;
  else if ( sides[1] > sides[0] && sides[1] > sides[2] )
    axis = 1;
  else 
    axis = 2;

  REAL localTransform[16];

  REAL maxDist = 0;
  REAL maxLen = 0;

  switch ( axis )
  {
    case 0:
      {
        fm_eulerMatrix(0,0,FM_PI/2,localTransform);
        fm_matrixMultiply(localTransform,omatrix,matrix);

        const NxU8 *scan = (const NxU8 *)points;
        for (NxU32 i=0; i<vcount; i++)
        {
          const REAL *p = (const REAL *)scan;
          REAL t[3];
          fm_inverseRT(omatrix,p,t);
          REAL dist = t[1]*t[1]+t[2]*t[2];
          if ( dist > maxDist )
          {
            maxDist = dist;
          }
          REAL l = (REAL) fabs(t[0]);
          if ( l > maxLen )
          {
            maxLen = l;
          }
          scan+=pstride;
        }
      }
      height = sides[0];
      break;
    case 1:
      {
        fm_eulerMatrix(0,FM_PI/2,0,localTransform);
        fm_matrixMultiply(localTransform,omatrix,matrix);

        const NxU8 *scan = (const NxU8 *)points;
        for (NxU32 i=0; i<vcount; i++)
        {
          const REAL *p = (const REAL *)scan;
          REAL t[3];
          fm_inverseRT(omatrix,p,t);
          REAL dist = t[0]*t[0]+t[2]*t[2];
          if ( dist > maxDist )
          {
            maxDist = dist;
          }
          REAL l = (REAL) fabs(t[1]);
          if ( l > maxLen )
          {
            maxLen = l;
          }
          scan+=pstride;
        }
      }
      height = sides[1];
      break;
    case 2:
      {
        fm_eulerMatrix(FM_PI/2,0,0,localTransform);
        fm_matrixMultiply(localTransform,omatrix,matrix);

        const NxU8 *scan = (const NxU8 *)points;
        for (NxU32 i=0; i<vcount; i++)
        {
          const REAL *p = (const REAL *)scan;
          REAL t[3];
          fm_inverseRT(omatrix,p,t);
          REAL dist = t[0]*t[0]+t[1]*t[1];
          if ( dist > maxDist )
          {
            maxDist = dist;
          }
          REAL l = (REAL) fabs(t[2]);
          if ( l > maxLen )
          {
            maxLen = l;
          }
          scan+=pstride;
        }
      }
      height = sides[2];
      break;
  }
  radius = (REAL)sqrt(maxDist);
  height = (maxLen*2)-(radius*2);
}


//************* Triangulation

#ifndef TRIANGULATE_H

#define TRIANGULATE_H

typedef NxU32 TU32;

class TVec
{
public:
	TVec(NxF64 _x,NxF64 _y,NxF64 _z) { x = _x; y = _y; z = _z; };
	TVec(void) { };

  NxF64 x;
  NxF64 y;
  NxF64 z;
};

typedef CONVEX_DECOMPOSITION::Array< TVec >  TVecVector;
typedef CONVEX_DECOMPOSITION::Array< TU32 >  TU32Vector;

class CTriangulator
{
public:
    ///     Default constructor
    CTriangulator();

    ///     Default destructor
    virtual ~CTriangulator();

    ///     Triangulates the contour
    void triangulate(TU32Vector &indices);

    ///     Returns the given point in the triangulator array
    inline TVec get(const TU32 id) { return mPoints[id]; }

    virtual void reset(void)
    {
        mInputPoints.clear();
        mPoints.clear();
        mIndices.clear();
    }

    virtual void addPoint(NxF64 x,NxF64 y,NxF64 z)
    {
        TVec v(x,y,z);
        // update bounding box...
        if ( mInputPoints.empty() )
        {
            mMin = v;
            mMax = v;
        }
        else
        {
            if ( x < mMin.x ) mMin.x = x;
            if ( y < mMin.y ) mMin.y = y;
            if ( z < mMin.z ) mMin.z = z;

            if ( x > mMax.x ) mMax.x = x;
            if ( y > mMax.y ) mMax.y = y;
            if ( z > mMax.z ) mMax.z = z;
        }
        mInputPoints.pushBack(v);
    }

    // Triangulation happens in 2d.  We could inverse transform the polygon around the normal direction, or we just use the two most signficant axes
    // Here we find the two longest axes and use them to triangulate.  Inverse transforming them would introduce more doubleing point error and isn't worth it.
    virtual NxU32 * triangulate(NxU32 &tcount,NxF64 epsilon)
    {
        NxU32 *ret = 0;
        tcount = 0;
        mEpsilon = epsilon;

        if ( !mInputPoints.empty() )
        {
            mPoints.clear();

          NxF64 dx = mMax.x - mMin.x; // locate the first, second and third longest edges and store them in i1, i2, i3
          NxF64 dy = mMax.y - mMin.y;
          NxF64 dz = mMax.z - mMin.z;

          NxU32 i1,i2,i3;

          if ( dx > dy && dx > dz )
          {
              i1 = 0;
              if ( dy > dz )
              {
                  i2 = 1;
                  i3 = 2;
              }
              else
              {
                  i2 = 2;
                  i3 = 1;
              }
          }
          else if ( dy > dx && dy > dz )
          {
              i1 = 1;
              if ( dx > dz )
              {
                  i2 = 0;
                  i3 = 2;
              }
              else
              {
                  i2 = 2;
                  i3 = 0;
              }
          }
          else
          {
              i1 = 2;
              if ( dx > dy )
              {
                  i2 = 0;
                  i3 = 1;
              }
              else
              {
                  i2 = 1;
                  i3 = 0;
              }
          }

          NxU32 pcount = (NxU32)mInputPoints.size();
          const NxF64 *points = &mInputPoints[0].x;
          for (NxU32 i=0; i<pcount; i++)
          {
            TVec v( points[i1], points[i2], points[i3] );
            mPoints.pushBack(v);
            points+=3;
          }

          mIndices.clear();
          triangulate(mIndices);
          tcount = (NxU32)mIndices.size()/3;
          if ( tcount )
          {
              ret = &mIndices[0];
          }
        }
        return ret;
    }

    virtual const NxF64 * getPoint(NxU32 index)
    {
        return &mInputPoints[index].x;
    }


private:
    NxF64                  mEpsilon;
    TVec                   mMin;
    TVec                   mMax;
    TVecVector             mInputPoints;
    TVecVector             mPoints;
    TU32Vector             mIndices;

    ///     Tests if a point is inside the given triangle
    bool _insideTriangle(const TVec& A, const TVec& B, const TVec& C,const TVec& P);

    ///     Returns the area of the contour
    NxF64 _area();

    bool _snip(NxI32 u, NxI32 v, NxI32 w, NxI32 n, NxI32 *V);

    ///     Processes the triangulation
    void _process(TU32Vector &indices);

};

///     Default constructor
CTriangulator::CTriangulator(void)
{
}

///     Default destructor
CTriangulator::~CTriangulator()
{
}

///     Triangulates the contour
void CTriangulator::triangulate(TU32Vector &indices)
{
    _process(indices);
}

///     Processes the triangulation
void CTriangulator::_process(TU32Vector &indices)
{
    const NxI32 n = (const NxI32)mPoints.size();
    if (n < 3)
        return;
    NxI32 *V = (NxI32 *)MEMALLOC_MALLOC(sizeof(NxI32)*n);

	bool flipped = false;

    if (0.0f < _area())
    {
        for (NxI32 v = 0; v < n; v++)
            V[v] = v;
    }
    else
    {
		flipped = true;
        for (NxI32 v = 0; v < n; v++)
            V[v] = (n - 1) - v;
    }

    NxI32 nv = n;
    NxI32 count = 2 * nv;
    for (NxI32 m = 0, v = nv - 1; nv > 2;)
    {
        if (0 >= (count--))
            return;

        NxI32 u = v;
        if (nv <= u)
            u = 0;
        v = u + 1;
        if (nv <= v)
            v = 0;
        NxI32 w = v + 1;
        if (nv <= w)
            w = 0;

        if (_snip(u, v, w, nv, V))
        {
            NxI32 a, b, c, s, t;
            a = V[u];
            b = V[v];
            c = V[w];
			if ( flipped )
			{
				indices.pushBack(a);
				indices.pushBack(b);
				indices.pushBack(c);
			}
			else
			{
				indices.pushBack(c);
				indices.pushBack(b);
				indices.pushBack(a);
			}
            m++;
            for (s = v, t = v + 1; t < nv; s++, t++)
                V[s] = V[t];
            nv--;
            count = 2 * nv;
        }
    }

    MEMALLOC_FREE(V);
}

///     Returns the area of the contour
NxF64 CTriangulator::_area()
{
    NxI32 n = (NxU32)mPoints.size();
    NxF64 A = 0.0f;
    for (NxI32 p = n - 1, q = 0; q < n; p = q++)
    {
        const TVec &pval = mPoints[p];
        const TVec &qval = mPoints[q];
        A += pval.x * qval.y - qval.x * pval.y;
    }
	A*=0.5f;
    return A;
}

bool CTriangulator::_snip(NxI32 u, NxI32 v, NxI32 w, NxI32 n, NxI32 *V)
{
    NxI32 p;

    const TVec &A = mPoints[ V[u] ];
    const TVec &B = mPoints[ V[v] ];
    const TVec &C = mPoints[ V[w] ];

    if (mEpsilon > (((B.x - A.x) * (C.y - A.y)) - ((B.y - A.y) * (C.x - A.x))) )
        return false;

    for (p = 0; p < n; p++)
    {
        if ((p == u) || (p == v) || (p == w))
            continue;
        const TVec &P = mPoints[ V[p] ];
        if (_insideTriangle(A, B, C, P))
            return false;
    }
    return true;
}

///     Tests if a point is inside the given triangle
bool CTriangulator::_insideTriangle(const TVec& A, const TVec& B, const TVec& C,const TVec& P)
{
    NxF64 ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
    NxF64 cCROSSap, bCROSScp, aCROSSbp;

    ax = C.x - B.x;  ay = C.y - B.y;
    bx = A.x - C.x;  by = A.y - C.y;
    cx = B.x - A.x;  cy = B.y - A.y;
    apx = P.x - A.x;  apy = P.y - A.y;
    bpx = P.x - B.x;  bpy = P.y - B.y;
    cpx = P.x - C.x;  cpy = P.y - C.y;

    aCROSSbp = ax * bpy - ay * bpx;
    cCROSSap = cx * apy - cy * apx;
    bCROSScp = bx * cpy - by * cpx;

    return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
}

class Triangulate : public fm_Triangulate, public Memalloc
{
public:
  Triangulate(void)
  {
    mPointsFloat = 0;
    mPointsDouble = 0;
  }

  virtual ~Triangulate(void)
  {
    reset();
  }
  void reset(void)
  {
    MEMALLOC_FREE(mPointsFloat);
    MEMALLOC_FREE(mPointsDouble);
    mPointsFloat = 0;
    mPointsDouble = 0;
  }

  virtual const NxF64 *       triangulate3d(NxU32 pcount,
                                             const NxF64 *_points,
                                             NxU32 vstride,
                                             NxU32 &tcount,
                                             bool consolidate,
                                             NxF64 epsilon)
  {
    reset();

    NxF64 *points = (NxF64 *)MEMALLOC_MALLOC(sizeof(NxF64)*pcount*3);
    if ( consolidate )
    {
      pcount = fm_consolidatePolygon(pcount,_points,vstride,points,1-epsilon);
    }
    else
    {
      NxF64 *dest = points;
      for (NxU32 i=0; i<pcount; i++)
      {
        const NxF64 *src = fm_getPoint(_points,vstride,i);
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
        dest+=3;
      }
      vstride = sizeof(NxF64)*3;
    }

    if ( pcount >= 3 )
    {
      CTriangulator ct;
      for (NxU32 i=0; i<pcount; i++)
      {
        const NxF64 *src = fm_getPoint(points,vstride,i);
        ct.addPoint( src[0], src[1], src[2] );
      }
      NxU32 _tcount;
      NxU32 *indices = ct.triangulate(_tcount,epsilon);
      if ( indices )
      {
        tcount = _tcount;
        mPointsDouble = (NxF64 *)MEMALLOC_MALLOC(sizeof(NxF64)*tcount*3*3);
        NxF64 *dest = mPointsDouble;
        for (NxU32 i=0; i<tcount; i++)
        {
          NxU32 i1 = indices[i*3+0];
          NxU32 i2 = indices[i*3+1];
          NxU32 i3 = indices[i*3+2];
          const NxF64 *p1 = ct.getPoint(i1);
          const NxF64 *p2 = ct.getPoint(i2);
          const NxF64 *p3 = ct.getPoint(i3);

          dest[0] = p1[0];
          dest[1] = p1[1];
          dest[2] = p1[2];

          dest[3] = p2[0];
          dest[4] = p2[1];
          dest[5] = p2[2];

          dest[6] = p3[0];
          dest[7] = p3[1];
          dest[8] = p3[2];
          dest+=9;
        }
      }
    }
    MEMALLOC_FREE(points);

    return mPointsDouble;
  }

  virtual const NxF32  *       triangulate3d(NxU32 pcount,
                                             const NxF32  *points,
                                             NxU32 vstride,
                                             NxU32 &tcount,
                                             bool consolidate,
                                             NxF32 epsilon)
  {
    reset();

    NxF64 *temp = (NxF64 *)MEMALLOC_MALLOC(sizeof(NxF64)*pcount*3);
    NxF64 *dest = temp;
    for (NxU32 i=0; i<pcount; i++)
    {
      const NxF32 *p = fm_getPoint(points,vstride,i);
      dest[0] = p[0];
      dest[1] = p[1];
      dest[2] = p[2];
      dest+=3;
    }
    const NxF64 *results = triangulate3d(pcount,temp,sizeof(NxF64)*3,tcount,consolidate,epsilon);
    if ( results )
    {
      NxU32 fcount = tcount*3*3;
      mPointsFloat = (NxF32 *)MEMALLOC_MALLOC(sizeof(NxF32)*tcount*3*3);
      NxF32 *dest = mPointsFloat;
      for (NxU32 i=0; i<fcount; i++)
      {
        dest[i] = (NxF32) results[i];
      }
      MEMALLOC_FREE(mPointsDouble);
      mPointsDouble = 0;
    }
    MEMALLOC_FREE(temp);

    return mPointsFloat;
  }

private:
  NxF32 *mPointsFloat;
  NxF64 *mPointsDouble;
};

fm_Triangulate * fm_createTriangulate(void)
{
  Triangulate *t = MEMALLOC_NEW(Triangulate);
  return static_cast< fm_Triangulate *>(t);
}

void             fm_releaseTriangulate(fm_Triangulate *t)
{
  Triangulate *tt = static_cast< Triangulate *>(t);
  delete tt;
}

#endif

bool validDistance(const REAL *p1,const REAL *p2,REAL epsilon)
{
	bool ret = true;

	REAL dx = p1[0] - p2[0];
	REAL dy = p1[1] - p2[1];
	REAL dz = p1[2] - p2[2];
	REAL dist = dx*dx+dy*dy+dz*dz;
	if ( dist < (epsilon*epsilon) )
	{
		ret = false;
	}
	return ret;
}

bool fm_isValidTriangle(const REAL *p1,const REAL *p2,const REAL *p3,REAL epsilon)
{
  bool ret = false;

  if ( validDistance(p1,p2,epsilon) &&
	   validDistance(p1,p3,epsilon) &&
	   validDistance(p2,p3,epsilon) )
  {

	  REAL area = fm_computeArea(p1,p2,p3);
	  if ( area > epsilon )
	  {
		REAL _vertices[3*3],vertices[64*3];

		_vertices[0] = p1[0];
		_vertices[1] = p1[1];
		_vertices[2] = p1[2];

		_vertices[3] = p2[0];
		_vertices[4] = p2[1];
		_vertices[5] = p2[2];

		_vertices[6] = p3[0];
		_vertices[7] = p3[1];
		_vertices[8] = p3[2];

		NxU32 pcount = fm_consolidatePolygon(3,_vertices,sizeof(REAL)*3,vertices,1-epsilon);
		if ( pcount == 3 )
		{
		  ret = true;
		}
	  }
  }
  return ret;
}


void  fm_multiplyQuat(const REAL *left,const REAL *right,REAL *quat)
{
	REAL a,b,c,d;

	a = left[3]*right[3] - left[0]*right[0] - left[1]*right[1] - left[2]*right[2];
	b = left[3]*right[0] + right[3]*left[0] + left[1]*right[2] - right[1]*left[2];
	c = left[3]*right[1] + right[3]*left[1] + left[2]*right[0] - right[2]*left[0];
	d = left[3]*right[2] + right[3]*left[2] + left[0]*right[1] - right[0]*left[1];

	quat[3] = a;
	quat[0] = b;
	quat[1] = c;
	quat[2] = d;
}

}; // end of namespace
