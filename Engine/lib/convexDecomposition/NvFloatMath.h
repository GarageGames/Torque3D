#ifndef NV_FLOAT_MATH_H

#define NV_FLOAT_MATH_H

#include "NvUserMemAlloc.h"

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

#include <float.h>

namespace CONVEX_DECOMPOSITION
{

enum FM_ClipState
{
  FMCS_XMIN       = (1<<0),
  FMCS_XMAX       = (1<<1),
  FMCS_YMIN       = (1<<2),
  FMCS_YMAX       = (1<<3),
  FMCS_ZMIN       = (1<<4),
  FMCS_ZMAX       = (1<<5),
};

enum FM_Axis
{
  FM_XAXIS   = (1<<0),
  FM_YAXIS   = (1<<1),
  FM_ZAXIS   = (1<<2)
};

enum LineSegmentType
{
  LS_START,
  LS_MIDDLE,
  LS_END
};


const NxF32 FM_PI = 3.1415926535897932384626433832795028841971693993751f;
const NxF32 FM_DEG_TO_RAD = ((2.0f * FM_PI) / 360.0f);
const NxF32 FM_RAD_TO_DEG = (360.0f / (2.0f * FM_PI));

//***************** Float versions
//***
//*** vectors are assumed to be 3 floats or 3 doubles representing X, Y, Z
//*** quaternions are assumed to be 4 floats or 4 doubles representing X,Y,Z,W
//*** matrices are assumed to be 16 floats or 16 doubles representing a standard D3D or OpenGL style 4x4 matrix
//*** bounding volumes are expressed as two sets of 3 floats/NxF64 representing bmin(x,y,z) and bmax(x,y,z)
//*** Plane equations are assumed to be 4 floats or 4 doubles representing Ax,By,Cz,D

FM_Axis fm_getDominantAxis(const NxF32 normal[3]);
FM_Axis fm_getDominantAxis(const NxF64 normal[3]);

void fm_decomposeTransform(const NxF32 local_transform[16],NxF32 trans[3],NxF32 rot[4],NxF32 scale[3]);
void fm_decomposeTransform(const NxF64 local_transform[16],NxF64 trans[3],NxF64 rot[4],NxF64 scale[3]);

void  fm_multiplyTransform(const NxF32 *pA,const NxF32 *pB,NxF32 *pM);
void  fm_multiplyTransform(const NxF64 *pA,const NxF64 *pB,NxF64 *pM);

void  fm_inverseTransform(const NxF32 matrix[16],NxF32 inverse_matrix[16]);
void  fm_inverseTransform(const NxF64 matrix[16],NxF64 inverse_matrix[16]);

void  fm_identity(NxF32 matrix[16]); // set 4x4 matrix to identity.
void  fm_identity(NxF64 matrix[16]); // set 4x4 matrix to identity.

void  fm_inverseRT(const NxF32 matrix[16], const NxF32 pos[3], NxF32 t[3]); // inverse rotate translate the point.
void  fm_inverseRT(const NxF64 matrix[16],const NxF64 pos[3],NxF64 t[3]); // inverse rotate translate the point.

void  fm_transform(const NxF32 matrix[16], const NxF32 pos[3], NxF32 t[3]); // rotate and translate this point.
void  fm_transform(const NxF64 matrix[16],const NxF64 pos[3],NxF64 t[3]); // rotate and translate this point.

NxF32  fm_getDeterminant(const NxF32 matrix[16]);
NxF64 fm_getDeterminant(const NxF64 matrix[16]);

void fm_getSubMatrix(NxI32 ki,NxI32 kj,NxF32 pDst[16],const NxF32 matrix[16]);
void fm_getSubMatrix(NxI32 ki,NxI32 kj,NxF64 pDst[16],const NxF32 matrix[16]);

void  fm_rotate(const NxF32 matrix[16],const NxF32 pos[3],NxF32 t[3]); // only rotate the point by a 4x4 matrix, don't translate.
void  fm_rotate(const NxF64 matri[16],const NxF64 pos[3],NxF64 t[3]); // only rotate the point by a 4x4 matrix, don't translate.

void  fm_eulerToMatrix(NxF32 ax,NxF32 ay,NxF32 az,NxF32 matrix[16]); // convert euler (in radians) to a dest 4x4 matrix (translation set to zero)
void  fm_eulerToMatrix(NxF64 ax,NxF64 ay,NxF64 az,NxF64 matrix[16]); // convert euler (in radians) to a dest 4x4 matrix (translation set to zero)

void  fm_getAABB(NxU32 vcount,const NxF32 *points,NxU32 pstride,NxF32 bmin[3],NxF32 bmax[3]);
void  fm_getAABB(NxU32 vcount,const NxF64 *points,NxU32 pstride,NxF64 bmin[3],NxF64 bmax[3]);

void  fm_getAABBCenter(const NxF32 bmin[3],const NxF32 bmax[3],NxF32 center[3]);
void  fm_getAABBCenter(const NxF64 bmin[3],const NxF64 bmax[3],NxF64 center[3]);

void  fm_eulerToQuat(NxF32 x,NxF32 y,NxF32 z,NxF32 quat[4]); // convert euler angles to quaternion.
void  fm_eulerToQuat(NxF64 x,NxF64 y,NxF64 z,NxF64 quat[4]); // convert euler angles to quaternion.

void  fm_quatToEuler(const NxF32 quat[4],NxF32 &ax,NxF32 &ay,NxF32 &az);
void  fm_quatToEuler(const NxF64 quat[4],NxF64 &ax,NxF64 &ay,NxF64 &az);

void  fm_eulerToQuat(const NxF32 euler[3],NxF32 quat[4]); // convert euler angles to quaternion. Angles must be radians not degrees!
void  fm_eulerToQuat(const NxF64 euler[3],NxF64 quat[4]); // convert euler angles to quaternion.

void  fm_scale(NxF32 x,NxF32 y,NxF32 z,NxF32 matrix[16]); // apply scale to the matrix.
void  fm_scale(NxF64 x,NxF64 y,NxF64 z,NxF64 matrix[16]); // apply scale to the matrix.

void  fm_eulerToQuatDX(NxF32 x,NxF32 y,NxF32 z,NxF32 quat[4]); // convert euler angles to quaternion using the fucked up DirectX method
void  fm_eulerToQuatDX(NxF64 x,NxF64 y,NxF64 z,NxF64 quat[4]); // convert euler angles to quaternion using the fucked up DirectX method

void  fm_eulerToMatrixDX(NxF32 x,NxF32 y,NxF32 z,NxF32 matrix[16]); // convert euler angles to quaternion using the fucked up DirectX method.
void  fm_eulerToMatrixDX(NxF64 x,NxF64 y,NxF64 z,NxF64 matrix[16]); // convert euler angles to quaternion using the fucked up DirectX method.

void  fm_quatToMatrix(const NxF32 quat[4],NxF32 matrix[16]); // convert quaterinion rotation to matrix, translation set to zero.
void  fm_quatToMatrix(const NxF64 quat[4],NxF64 matrix[16]); // convert quaterinion rotation to matrix, translation set to zero.

void  fm_quatRotate(const NxF32 quat[4],const NxF32 v[3],NxF32 r[3]); // rotate a vector directly by a quaternion.
void  fm_quatRotate(const NxF64 quat[4],const NxF64 v[3],NxF64 r[3]); // rotate a vector directly by a quaternion.

void  fm_getTranslation(const NxF32 matrix[16],NxF32 t[3]);
void  fm_getTranslation(const NxF64 matrix[16],NxF64 t[3]);

void  fm_setTranslation(const NxF32 *translation,NxF32 matrix[16]);
void  fm_setTranslation(const NxF64 *translation,NxF64 matrix[16]);

void  fm_multiplyQuat(const NxF32 *qa,const NxF32 *qb,NxF32 *quat);
void  fm_multiplyQuat(const NxF64 *qa,const NxF64 *qb,NxF64 *quat);

void  fm_matrixToQuat(const NxF32 matrix[16],NxF32 quat[4]); // convert the 3x3 portion of a 4x4 matrix into a quaterion as x,y,z,w
void  fm_matrixToQuat(const NxF64 matrix[16],NxF64 quat[4]); // convert the 3x3 portion of a 4x4 matrix into a quaterion as x,y,z,w

NxF32 fm_sphereVolume(NxF32 radius); // return's the volume of a sphere of this radius (4/3 PI * R cubed )
NxF64 fm_sphereVolume(NxF64 radius); // return's the volume of a sphere of this radius (4/3 PI * R cubed )

NxF32 fm_cylinderVolume(NxF32 radius,NxF32 h);
NxF64 fm_cylinderVolume(NxF64 radius,NxF64 h);

NxF32 fm_capsuleVolume(NxF32 radius,NxF32 h);
NxF64 fm_capsuleVolume(NxF64 radius,NxF64 h);

NxF32 fm_distance(const NxF32 p1[3],const NxF32 p2[3]);
NxF64 fm_distance(const NxF64 p1[3],const NxF64 p2[3]);

NxF32 fm_distanceSquared(const NxF32 p1[3],const NxF32 p2[3]);
NxF64 fm_distanceSquared(const NxF64 p1[3],const NxF64 p2[3]);

NxF32 fm_distanceSquaredXZ(const NxF32 p1[3],const NxF32 p2[3]);
NxF64 fm_distanceSquaredXZ(const NxF64 p1[3],const NxF64 p2[3]);

NxF32 fm_computePlane(const NxF32 p1[3],const NxF32 p2[3],const NxF32 p3[3],NxF32 *n); // return D
NxF64 fm_computePlane(const NxF64 p1[3],const NxF64 p2[3],const NxF64 p3[3],NxF64 *n); // return D

NxF32 fm_distToPlane(const NxF32 plane[4],const NxF32 pos[3]); // computes the distance of this point from the plane.
NxF64 fm_distToPlane(const NxF64 plane[4],const NxF64 pos[3]); // computes the distance of this point from the plane.

NxF32 fm_dot(const NxF32 p1[3],const NxF32 p2[3]);
NxF64 fm_dot(const NxF64 p1[3],const NxF64 p2[3]);

void  fm_cross(NxF32 cross[3],const NxF32 a[3],const NxF32 b[3]);
void  fm_cross(NxF64 cross[3],const NxF64 a[3],const NxF64 b[3]);

void  fm_computeNormalVector(NxF32 n[3],const NxF32 p1[3],const NxF32 p2[3]); // as P2-P1 normalized.
void  fm_computeNormalVector(NxF64 n[3],const NxF64 p1[3],const NxF64 p2[3]); // as P2-P1 normalized.

bool  fm_computeWindingOrder(const NxF32 p1[3],const NxF32 p2[3],const NxF32 p3[3]); // returns true if the triangle is clockwise.
bool  fm_computeWindingOrder(const NxF64 p1[3],const NxF64 p2[3],const NxF64 p3[3]); // returns true if the triangle is clockwise.

NxF32  fm_normalize(NxF32 n[3]); // normalize this vector and return the distance
NxF64  fm_normalize(NxF64 n[3]); // normalize this vector and return the distance

void  fm_matrixMultiply(const NxF32 A[16],const NxF32 B[16],NxF32 dest[16]);
void  fm_matrixMultiply(const NxF64 A[16],const NxF64 B[16],NxF64 dest[16]);

void  fm_composeTransform(const NxF32 position[3],const NxF32 quat[4],const NxF32 scale[3],NxF32 matrix[16]);
void  fm_composeTransform(const NxF64 position[3],const NxF64 quat[4],const NxF64 scale[3],NxF64 matrix[16]);

NxF32 fm_computeArea(const NxF32 p1[3],const NxF32 p2[3],const NxF32 p3[3]);
NxF64 fm_computeArea(const NxF64 p1[3],const NxF64 p2[3],const NxF64 p3[3]);

void  fm_lerp(const NxF32 p1[3],const NxF32 p2[3],NxF32 dest[3],NxF32 lerpValue);
void  fm_lerp(const NxF64 p1[3],const NxF64 p2[3],NxF64 dest[3],NxF64 lerpValue);

bool  fm_insideTriangleXZ(const NxF32 test[3],const NxF32 p1[3],const NxF32 p2[3],const NxF32 p3[3]);
bool  fm_insideTriangleXZ(const NxF64 test[3],const NxF64 p1[3],const NxF64 p2[3],const NxF64 p3[3]);

bool  fm_insideAABB(const NxF32 pos[3],const NxF32 bmin[3],const NxF32 bmax[3]);
bool  fm_insideAABB(const NxF64 pos[3],const NxF64 bmin[3],const NxF64 bmax[3]);

bool  fm_insideAABB(const NxF32 obmin[3],const NxF32 obmax[3],const NxF32 tbmin[3],const NxF32 tbmax[3]); // test if bounding box tbmin/tmbax is fully inside obmin/obmax
bool  fm_insideAABB(const NxF64 obmin[3],const NxF64 obmax[3],const NxF64 tbmin[3],const NxF64 tbmax[3]); // test if bounding box tbmin/tmbax is fully inside obmin/obmax

NxU32 fm_clipTestPoint(const NxF32 bmin[3],const NxF32 bmax[3],const NxF32 pos[3]);
NxU32 fm_clipTestPoint(const NxF64 bmin[3],const NxF64 bmax[3],const NxF64 pos[3]);

NxU32 fm_clipTestPointXZ(const NxF32 bmin[3],const NxF32 bmax[3],const NxF32 pos[3]); // only tests X and Z, not Y
NxU32 fm_clipTestPointXZ(const NxF64 bmin[3],const NxF64 bmax[3],const NxF64 pos[3]); // only tests X and Z, not Y


NxU32 fm_clipTestAABB(const NxF32 bmin[3],const NxF32 bmax[3],const NxF32 p1[3],const NxF32 p2[3],const NxF32 p3[3],NxU32 &andCode);
NxU32 fm_clipTestAABB(const NxF64 bmin[3],const NxF64 bmax[3],const NxF64 p1[3],const NxF64 p2[3],const NxF64 p3[3],NxU32 &andCode);


bool     fm_lineTestAABBXZ(const NxF32 p1[3],const NxF32 p2[3],const NxF32 bmin[3],const NxF32 bmax[3],NxF32 &time);
bool     fm_lineTestAABBXZ(const NxF64 p1[3],const NxF64 p2[3],const NxF64 bmin[3],const NxF64 bmax[3],NxF64 &time);

bool     fm_lineTestAABB(const NxF32 p1[3],const NxF32 p2[3],const NxF32 bmin[3],const NxF32 bmax[3],NxF32 &time);
bool     fm_lineTestAABB(const NxF64 p1[3],const NxF64 p2[3],const NxF64 bmin[3],const NxF64 bmax[3],NxF64 &time);


void  fm_initMinMax(const NxF32 p[3],NxF32 bmin[3],NxF32 bmax[3]);
void  fm_initMinMax(const NxF64 p[3],NxF64 bmin[3],NxF64 bmax[3]);

void  fm_initMinMax(NxF32 bmin[3],NxF32 bmax[3]);
void  fm_initMinMax(NxF64 bmin[3],NxF64 bmax[3]);

void  fm_minmax(const NxF32 p[3],NxF32 bmin[3],NxF32 bmax[3]); // accmulate to a min-max value
void  fm_minmax(const NxF64 p[3],NxF64 bmin[3],NxF64 bmax[3]); // accmulate to a min-max value


NxF32 fm_solveX(const NxF32 plane[4],NxF32 y,NxF32 z); // solve for X given this plane equation and the other two components.
NxF64 fm_solveX(const NxF64 plane[4],NxF64 y,NxF64 z); // solve for X given this plane equation and the other two components.

NxF32 fm_solveY(const NxF32 plane[4],NxF32 x,NxF32 z); // solve for Y given this plane equation and the other two components.
NxF64 fm_solveY(const NxF64 plane[4],NxF64 x,NxF64 z); // solve for Y given this plane equation and the other two components.

NxF32 fm_solveZ(const NxF32 plane[4],NxF32 x,NxF32 y); // solve for Z given this plane equation and the other two components.
NxF64 fm_solveZ(const NxF64 plane[4],NxF64 x,NxF64 y); // solve for Z given this plane equation and the other two components.

bool  fm_computeBestFitPlane(NxU32 vcount,     // number of input data points
                     const NxF32 *points,     // starting address of points array.
                     NxU32 vstride,    // stride between input points.
                     const NxF32 *weights,    // *optional point weighting values.
                     NxU32 wstride,    // weight stride for each vertex.
                     NxF32 plane[4]);

bool  fm_computeBestFitPlane(NxU32 vcount,     // number of input data points
                     const NxF64 *points,     // starting address of points array.
                     NxU32 vstride,    // stride between input points.
                     const NxF64 *weights,    // *optional point weighting values.
                     NxU32 wstride,    // weight stride for each vertex.
                     NxF64 plane[4]);


NxF32  fm_computeBestFitAABB(NxU32 vcount,const NxF32 *points,NxU32 pstride,NxF32 bmin[3],NxF32 bmax[3]); // returns the diagonal distance
NxF64 fm_computeBestFitAABB(NxU32 vcount,const NxF64 *points,NxU32 pstride,NxF64 bmin[3],NxF64 bmax[3]); // returns the diagonal distance

NxF32  fm_computeBestFitSphere(NxU32 vcount,const NxF32 *points,NxU32 pstride,NxF32 center[3]);
NxF64  fm_computeBestFitSphere(NxU32 vcount,const NxF64 *points,NxU32 pstride,NxF64 center[3]);

bool fm_lineSphereIntersect(const NxF32 center[3],NxF32 radius,const NxF32 p1[3],const NxF32 p2[3],NxF32 intersect[3]);
bool fm_lineSphereIntersect(const NxF64 center[3],NxF64 radius,const NxF64 p1[3],const NxF64 p2[3],NxF64 intersect[3]);

bool fm_intersectRayAABB(const NxF32 bmin[3],const NxF32 bmax[3],const NxF32 pos[3],const NxF32 dir[3],NxF32 intersect[3]);
bool fm_intersectLineSegmentAABB(const NxF32 bmin[3],const NxF32 bmax[3],const NxF32 p1[3],const NxF32 p2[3],NxF32 intersect[3]);

bool fm_lineIntersectsTriangle(const NxF32 rayStart[3],const NxF32 rayEnd[3],const NxF32 p1[3],const NxF32 p2[3],const NxF32 p3[3],NxF32 sect[3]);
bool fm_lineIntersectsTriangle(const NxF64 rayStart[3],const NxF64 rayEnd[3],const NxF64 p1[3],const NxF64 p2[3],const NxF64 p3[3],NxF64 sect[3]);

bool fm_rayIntersectsTriangle(const NxF32 origin[3],const NxF32 dir[3],const NxF32 v0[3],const NxF32 v1[3],const NxF32 v2[3],NxF32 &t);
bool fm_rayIntersectsTriangle(const NxF64 origin[3],const NxF64 dir[3],const NxF64 v0[3],const NxF64 v1[3],const NxF64 v2[3],NxF64 &t);

bool fm_raySphereIntersect(const NxF32 center[3],NxF32 radius,const NxF32 pos[3],const NxF32 dir[3],NxF32 distance,NxF32 intersect[3]);
bool fm_raySphereIntersect(const NxF64 center[3],NxF64 radius,const NxF64 pos[3],const NxF64 dir[3],NxF64 distance,NxF64 intersect[3]);

void fm_catmullRom(NxF32 out_vector[3],const NxF32 p1[3],const NxF32 p2[3],const NxF32 p3[3],const NxF32 *p4, const NxF32 s);
void fm_catmullRom(NxF64 out_vector[3],const NxF64 p1[3],const NxF64 p2[3],const NxF64 p3[3],const NxF64 *p4, const NxF64 s);

bool fm_intersectAABB(const NxF32 bmin1[3],const NxF32 bmax1[3],const NxF32 bmin2[3],const NxF32 bmax2[3]);
bool fm_intersectAABB(const NxF64 bmin1[3],const NxF64 bmax1[3],const NxF64 bmin2[3],const NxF64 bmax2[3]);


// computes the rotation quaternion to go from unit-vector v0 to unit-vector v1
void fm_rotationArc(const NxF32 v0[3],const NxF32 v1[3],NxF32 quat[4]);
void fm_rotationArc(const NxF64 v0[3],const NxF64 v1[3],NxF64 quat[4]);

NxF32  fm_distancePointLineSegment(const NxF32 Point[3],const NxF32 LineStart[3],const NxF32 LineEnd[3],NxF32 intersection[3],LineSegmentType &type,NxF32 epsilon);
NxF64 fm_distancePointLineSegment(const NxF64 Point[3],const NxF64 LineStart[3],const NxF64 LineEnd[3],NxF64 intersection[3],LineSegmentType &type,NxF64 epsilon);


bool fm_colinear(const NxF64 p1[3],const NxF64 p2[3],const NxF64 p3[3],NxF64 epsilon=0.999);               // true if these three points in a row are co-linear
bool fm_colinear(const NxF32  p1[3],const NxF32  p2[3],const NxF32 p3[3],NxF32 epsilon=0.999f);

bool fm_colinear(const NxF32 a1[3],const NxF32 a2[3],const NxF32 b1[3],const NxF32 b2[3],NxF32 epsilon=0.999f);  // true if these two line segments are co-linear.
bool fm_colinear(const NxF64 a1[3],const NxF64 a2[3],const NxF64 b1[3],const NxF64 b2[3],NxF64 epsilon=0.999);  // true if these two line segments are co-linear.

enum IntersectResult
{
  IR_DONT_INTERSECT,
  IR_DO_INTERSECT,
  IR_COINCIDENT,
  IR_PARALLEL,
};

IntersectResult fm_intersectLineSegments2d(const NxF32 a1[3], const NxF32 a2[3], const NxF32 b1[3], const NxF32 b2[3], NxF32 intersectionPoint[3]);
IntersectResult fm_intersectLineSegments2d(const NxF64 a1[3],const NxF64 a2[3],const NxF64 b1[3],const NxF64 b2[3],NxF64 intersectionPoint[3]);

IntersectResult fm_intersectLineSegments2dTime(const NxF32 a1[3], const NxF32 a2[3], const NxF32 b1[3], const NxF32 b2[3],NxF32 &t1,NxF32 &t2);
IntersectResult fm_intersectLineSegments2dTime(const NxF64 a1[3],const NxF64 a2[3],const NxF64 b1[3],const NxF64 b2[3],NxF64 &t1,NxF64 &t2);

// Plane-Triangle splitting

enum PlaneTriResult
{
  PTR_ON_PLANE,
  PTR_FRONT,
  PTR_BACK,
  PTR_SPLIT,
};

PlaneTriResult fm_planeTriIntersection(const NxF32 plane[4],    // the plane equation in Ax+By+Cz+D format
                                    const NxF32 *triangle, // the source triangle.
                                    NxU32 tstride,  // stride in bytes of the input and output *vertices*
                                    NxF32        epsilon,  // the co-planer epsilon value.
                                    NxF32       *front,    // the triangle in front of the
                                    NxU32 &fcount,  // number of vertices in the 'front' triangle
                                    NxF32       *back,     // the triangle in back of the plane
                                    NxU32 &bcount); // the number of vertices in the 'back' triangle.


PlaneTriResult fm_planeTriIntersection(const NxF64 plane[4],    // the plane equation in Ax+By+Cz+D format
                                    const NxF64 *triangle, // the source triangle.
                                    NxU32 tstride,  // stride in bytes of the input and output *vertices*
                                    NxF64        epsilon,  // the co-planer epsilon value.
                                    NxF64       *front,    // the triangle in front of the
                                    NxU32 &fcount,  // number of vertices in the 'front' triangle
                                    NxF64       *back,     // the triangle in back of the plane
                                    NxU32 &bcount); // the number of vertices in the 'back' triangle.


void fm_intersectPointPlane(const NxF32 p1[3],const NxF32 p2[3],NxF32 *split,const NxF32 plane[4]);
void fm_intersectPointPlane(const NxF64 p1[3],const NxF64 p2[3],NxF64 *split,const NxF64 plane[4]);

PlaneTriResult fm_getSidePlane(const NxF32 p[3],const NxF32 plane[4],NxF32 epsilon);
PlaneTriResult fm_getSidePlane(const NxF64 p[3],const NxF64 plane[4],NxF64 epsilon);


void fm_computeBestFitOBB(NxU32 vcount,const NxF32 *points,NxU32 pstride,NxF32 *sides,NxF32 matrix[16],bool bruteForce=true);
void fm_computeBestFitOBB(NxU32 vcount,const NxF64 *points,NxU32 pstride,NxF64 *sides,NxF64 matrix[16],bool bruteForce=true);

void fm_computeBestFitOBB(NxU32 vcount,const NxF32 *points,NxU32 pstride,NxF32 *sides,NxF32 pos[3],NxF32 quat[4],bool bruteForce=true);
void fm_computeBestFitOBB(NxU32 vcount,const NxF64 *points,NxU32 pstride,NxF64 *sides,NxF64 pos[3],NxF64 quat[4],bool bruteForce=true);

void fm_computeBestFitABB(NxU32 vcount,const NxF32 *points,NxU32 pstride,NxF32 *sides,NxF32 pos[3]);
void fm_computeBestFitABB(NxU32 vcount,const NxF64 *points,NxU32 pstride,NxF64 *sides,NxF64 pos[3]);


//** Note, if the returned capsule height is less than zero, then you must represent it is a sphere of size radius.
void fm_computeBestFitCapsule(NxU32 vcount,const NxF32 *points,NxU32 pstride,NxF32 &radius,NxF32 &height,NxF32 matrix[16],bool bruteForce=true);
void fm_computeBestFitCapsule(NxU32 vcount,const NxF64 *points,NxU32 pstride,NxF32 &radius,NxF32 &height,NxF64 matrix[16],bool bruteForce=true);


void fm_planeToMatrix(const NxF32 plane[4],NxF32 matrix[16]); // convert a plane equation to a 4x4 rotation matrix.  Reference vector is 0,1,0
void fm_planeToQuat(const NxF32 plane[4],NxF32 quat[4],NxF32 pos[3]); // convert a plane equation to a quaternion and translation

void fm_planeToMatrix(const NxF64 plane[4],NxF64 matrix[16]); // convert a plane equation to a 4x4 rotation matrix
void fm_planeToQuat(const NxF64 plane[4],NxF64 quat[4],NxF64 pos[3]); // convert a plane equation to a quaternion and translation

inline void fm_doubleToFloat3(const NxF64 p[3],NxF32 t[3]) { t[0] = (NxF32) p[0]; t[1] = (NxF32)p[1]; t[2] = (NxF32)p[2]; };
inline void fm_floatToDouble3(const NxF32 p[3],NxF64 t[3]) { t[0] = (NxF64)p[0]; t[1] = (NxF64)p[1]; t[2] = (NxF64)p[2]; };


void  fm_eulerMatrix(NxF32 ax,NxF32 ay,NxF32 az,NxF32 matrix[16]); // convert euler (in radians) to a dest 4x4 matrix (translation set to zero)
void  fm_eulerMatrix(NxF64 ax,NxF64 ay,NxF64 az,NxF64 matrix[16]); // convert euler (in radians) to a dest 4x4 matrix (translation set to zero)


NxF32  fm_computeMeshVolume(const NxF32 *vertices,NxU32 tcount,const NxU32 *indices);
NxF64 fm_computeMeshVolume(const NxF64 *vertices,NxU32 tcount,const NxU32 *indices);


#define FM_DEFAULT_GRANULARITY 0.001f  // 1 millimeter is the default granularity

class fm_VertexIndex
{
public:
  virtual NxU32          getIndex(const NxF32 pos[3],bool &newPos) = 0;  // get welded index for this NxF32 vector[3]
  virtual NxU32          getIndex(const NxF64 pos[3],bool &newPos) = 0;  // get welded index for this NxF64 vector[3]
  virtual const NxF32 *   getVerticesFloat(void) const = 0;
  virtual const NxF64 *  getVerticesDouble(void) const = 0;
  virtual const NxF32 *   getVertexFloat(NxU32 index) const = 0;
  virtual const NxF64 *  getVertexDouble(NxU32 index) const = 0;
  virtual NxU32          getVcount(void) const = 0;
  virtual bool            isDouble(void) const = 0;
  virtual bool            saveAsObj(const char *fname,NxU32 tcount,NxU32 *indices) = 0;
};

fm_VertexIndex * fm_createVertexIndex(NxF64 granularity,bool snapToGrid); // create an indexed vertex system for doubles
fm_VertexIndex * fm_createVertexIndex(NxF32 granularity,bool snapToGrid);  // create an indexed vertext system for floats
void             fm_releaseVertexIndex(fm_VertexIndex *vindex);



#if 0 // currently disabled

class fm_LineSegment
{
public:
  fm_LineSegment(void)
  {
    mE1 = mE2 = 0;
  }

  fm_LineSegment(NxU32 e1,NxU32 e2)
  {
    mE1 = e1;
    mE2 = e2;
  }

  NxU32 mE1;
  NxU32 mE2;
};


// LineSweep *only* supports doublees.  As a geometric operation it needs as much precision as possible.
class fm_LineSweep
{
public:

 virtual fm_LineSegment * performLineSweep(const fm_LineSegment *segments,
                                   NxU32 icount,
                                   const NxF64 *planeEquation,
                                   fm_VertexIndex *pool,
                                   NxU32 &scount) = 0;


};

fm_LineSweep * fm_createLineSweep(void);
void           fm_releaseLineSweep(fm_LineSweep *sweep);

#endif

class fm_Triangulate
{
public:
  virtual const NxF64 *       triangulate3d(NxU32 pcount,
                                             const NxF64 *points,
                                             NxU32 vstride,
                                             NxU32 &tcount,
                                             bool consolidate,
                                             NxF64 epsilon) = 0;

  virtual const NxF32  *       triangulate3d(NxU32 pcount,
                                             const NxF32  *points,
                                             NxU32 vstride,
                                             NxU32 &tcount,
                                             bool consolidate,
                                             NxF32 epsilon) = 0;
};

fm_Triangulate * fm_createTriangulate(void);
void             fm_releaseTriangulate(fm_Triangulate *t);


const NxF32 * fm_getPoint(const NxF32 *points,NxU32 pstride,NxU32 index);
const NxF64 * fm_getPoint(const NxF64 *points,NxU32 pstride,NxU32 index);

bool   fm_insideTriangle(NxF32 Ax, NxF32 Ay,NxF32 Bx, NxF32 By,NxF32 Cx, NxF32 Cy,NxF32 Px, NxF32 Py);
bool   fm_insideTriangle(NxF64 Ax, NxF64 Ay,NxF64 Bx, NxF64 By,NxF64 Cx, NxF64 Cy,NxF64 Px, NxF64 Py);
NxF32  fm_areaPolygon2d(NxU32 pcount,const NxF32 *points,NxU32 pstride);
NxF64 fm_areaPolygon2d(NxU32 pcount,const NxF64 *points,NxU32 pstride);

bool  fm_pointInsidePolygon2d(NxU32 pcount,const NxF32 *points,NxU32 pstride,const NxF32 *point,NxU32 xindex=0,NxU32 yindex=1);
bool  fm_pointInsidePolygon2d(NxU32 pcount,const NxF64 *points,NxU32 pstride,const NxF64 *point,NxU32 xindex=0,NxU32 yindex=1);

NxU32 fm_consolidatePolygon(NxU32 pcount,const NxF32 *points,NxU32 pstride,NxF32 *dest,NxF32 epsilon=0.999999f); // collapses co-linear edges.
NxU32 fm_consolidatePolygon(NxU32 pcount,const NxF64 *points,NxU32 pstride,NxF64 *dest,NxF64 epsilon=0.999999); // collapses co-linear edges.


bool fm_computeSplitPlane(NxU32 vcount,const NxF64 *vertices,NxU32 tcount,const NxU32 *indices,NxF64 *plane);
bool fm_computeSplitPlane(NxU32 vcount,const NxF32 *vertices,NxU32 tcount,const NxU32 *indices,NxF32 *plane);

void fm_nearestPointInTriangle(const NxF32 *pos,const NxF32 *p1,const NxF32 *p2,const NxF32 *p3,NxF32 *nearest);
void fm_nearestPointInTriangle(const NxF64 *pos,const NxF64 *p1,const NxF64 *p2,const NxF64 *p3,NxF64 *nearest);

NxF32  fm_areaTriangle(const NxF32 *p1,const NxF32 *p2,const NxF32 *p3);
NxF64 fm_areaTriangle(const NxF64 *p1,const NxF64 *p2,const NxF64 *p3);

void fm_subtract(const NxF32 *A,const NxF32 *B,NxF32 *diff); // compute A-B and store the result in 'diff'
void fm_subtract(const NxF64 *A,const NxF64 *B,NxF64 *diff); // compute A-B and store the result in 'diff'

void fm_multiply(NxF32 *A,NxF32 scaler);
void fm_multiply(NxF64 *A,NxF64 scaler);

void fm_add(const NxF32 *A,const NxF32 *B,NxF32 *sum);
void fm_add(const NxF64 *A,const NxF64 *B,NxF64 *sum);

void fm_copy3(const NxF32 *source,NxF32 *dest);
void fm_copy3(const NxF64 *source,NxF64 *dest);

// re-indexes an indexed triangle mesh but drops unused vertices.  The output_indices can be the same pointer as the input indices.
// the output_vertices can point to the input vertices if you desire.  The output_vertices buffer should be at least the same size
// is the input buffer.  The routine returns the new vertex count after re-indexing.
NxU32  fm_copyUniqueVertices(NxU32 vcount,const NxF32 *input_vertices,NxF32 *output_vertices,NxU32 tcount,const NxU32 *input_indices,NxU32 *output_indices);
NxU32  fm_copyUniqueVertices(NxU32 vcount,const NxF64 *input_vertices,NxF64 *output_vertices,NxU32 tcount,const NxU32 *input_indices,NxU32 *output_indices);

bool    fm_isMeshCoplanar(NxU32 tcount,const NxU32 *indices,const NxF32 *vertices,bool doubleSided); // returns true if this collection of indexed triangles are co-planar!
bool    fm_isMeshCoplanar(NxU32 tcount,const NxU32 *indices,const NxF64 *vertices,bool doubleSided); // returns true if this collection of indexed triangles are co-planar!

bool    fm_samePlane(const NxF32 p1[4],const NxF32 p2[4],NxF32 normalEpsilon=0.01f,NxF32 dEpsilon=0.001f,bool doubleSided=false); // returns true if these two plane equations are identical within an epsilon
bool    fm_samePlane(const NxF64 p1[4],const NxF64 p2[4],NxF64 normalEpsilon=0.01,NxF64 dEpsilon=0.001,bool doubleSided=false);

void    fm_OBBtoAABB(const NxF32 obmin[3],const NxF32 obmax[3],const NxF32 matrix[16],NxF32 abmin[3],NxF32 abmax[3]);

// a utility class that will tesseleate a mesh.
class fm_Tesselate
{
public:
  virtual const NxU32 * tesselate(fm_VertexIndex *vindex,NxU32 tcount,const NxU32 *indices,NxF32 longEdge,NxU32 maxDepth,NxU32 &outcount) = 0;
};

fm_Tesselate * fm_createTesselate(void);
void           fm_releaseTesselate(fm_Tesselate *t);

void fm_computeMeanNormals(NxU32 vcount,       // the number of vertices
                           const NxF32 *vertices,     // the base address of the vertex position data.
                           NxU32 vstride,      // the stride between position data.
                           NxF32 *normals,            // the base address  of the destination for mean vector normals
                           NxU32 nstride,      // the stride between normals
                           NxU32 tcount,       // the number of triangles
                           const NxU32 *indices);     // the triangle indices

void fm_computeMeanNormals(NxU32 vcount,       // the number of vertices
                           const NxF64 *vertices,     // the base address of the vertex position data.
                           NxU32 vstride,      // the stride between position data.
                           NxF64 *normals,            // the base address  of the destination for mean vector normals
                           NxU32 nstride,      // the stride between normals
                           NxU32 tcount,       // the number of triangles
                           const NxU32 *indices);     // the triangle indices


bool fm_isValidTriangle(const NxF32 *p1,const NxF32 *p2,const NxF32 *p3,NxF32 epsilon=0.00001f);
bool fm_isValidTriangle(const NxF64 *p1,const NxF64 *p2,const NxF64 *p3,NxF64 epsilon=0.00001f);

}; // end of namespace

#endif
