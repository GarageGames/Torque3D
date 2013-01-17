#ifndef NV_STAN_HULL_H

#define NV_STAN_HULL_H

/*

NvStanHull.h : A convex hull generator written by Stan Melax

*/


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

#include "NvUserMemAlloc.h"

namespace CONVEX_DECOMPOSITION
{

class HullResult
{
public:
	HullResult(void)
	{
		mPolygons = true;
		mNumOutputVertices = 0;
		mOutputVertices = 0;
		mNumFaces = 0;
		mNumIndices = 0;
		mIndices = 0;
	}
	bool             mPolygons;                  // true if indices represents polygons, false indices are triangles
	NxU32            mNumOutputVertices;         // number of vertices in the output hull
	NxF32           *mOutputVertices;            // array of vertices, 3 floats each x,y,z
	NxU32            mNumFaces;                  // the number of faces produced
	NxU32            mNumIndices;                // the total number of indices
	NxU32           *mIndices;                   // pointer to indices.

// If triangles, then indices are array indexes into the vertex list.
// If polygons, indices are in the form (number of points in face) (p1, p2, p3, ..) etc..
};

enum HullFlag
{
	QF_TRIANGLES         = (1<<0),             // report results as triangles, not polygons.
	QF_REVERSE_ORDER     = (1<<1),             // reverse order of the triangle indices.
	QF_SKIN_WIDTH        = (1<<2),             // extrude hull based on this skin width
	QF_DEFAULT           = (QF_TRIANGLES | QF_SKIN_WIDTH)
};


class HullDesc
{
public:
	HullDesc(void)
	{
		mFlags          = QF_DEFAULT;
		mVcount         = 0;
		mVertices       = 0;
		mVertexStride   = 0;
		mNormalEpsilon  = 0.001f;
		mMaxVertices = 4096; // maximum number of points to be considered for a convex hull.
		mSkinWidth = 0.01f; // default is one centimeter
	};

	HullDesc(HullFlag flag,
						 NxU32 vcount,
						 const NxF32 *vertices,
						 NxU32 stride)
	{
		mFlags          = flag;
		mVcount         = vcount;
		mVertices       = vertices;
		mVertexStride   = stride;
		mNormalEpsilon  = 0.001f;
		mMaxVertices    = 4096;
		mSkinWidth = 0.01f; // default is one centimeter
	}

	bool HasHullFlag(HullFlag flag) const
	{
		if ( mFlags & flag ) return true;
		return false;
	}

	void SetHullFlag(HullFlag flag)
	{
		mFlags|=flag;
	}

	void ClearHullFlag(HullFlag flag)
	{
		mFlags&=~flag;
	}

	NxU32      mFlags;           // flags to use when generating the convex hull.
	NxU32      mVcount;          // number of vertices in the input point cloud
	const NxF32      *mVertices;        // the array of vertices.
	NxU32      mVertexStride;    // the stride of each vertex, in bytes.
	NxF32             mNormalEpsilon;   // the epsilon for removing duplicates.  This is a normalized value, if normalized bit is on.
	NxF32             mSkinWidth;
	NxU32      mMaxVertices;               // maximum number of vertices to be considered for the hull!
};

enum HullError
{
	QE_OK,            // success!
	QE_FAIL,           // failed.
	QE_NOT_READY,
};

// This class is used when converting a convex hull into a triangle mesh.
class ConvexHullVertex
{
public:
	NxF32         mPos[3];
	NxF32         mNormal[3];
	NxF32         mTexel[2];
};

// A virtual interface to receive the triangles from the convex hull.
class ConvexHullTriangleInterface
{
public:
	virtual void ConvexHullTriangle(const ConvexHullVertex &v1,const ConvexHullVertex &v2,const ConvexHullVertex &v3) = 0;
};


class HullLibrary
{
public:

	HullError CreateConvexHull(const HullDesc       &desc,           // describes the input request
															HullResult           &result);        // contains the resulst

	HullError ReleaseResult(HullResult &result); // release memory allocated for this result, we are done with it.

	HullError CreateTriangleMesh(HullResult &answer,ConvexHullTriangleInterface *iface);
private:
	NxF32 ComputeNormal(NxF32 *n,const NxF32 *A,const NxF32 *B,const NxF32 *C);
	void AddConvexTriangle(ConvexHullTriangleInterface *callback,const NxF32 *p1,const NxF32 *p2,const NxF32 *p3);

	void BringOutYourDead(const NxF32 *verts,NxU32 vcount, NxF32 *overts,NxU32 &ocount,NxU32 *indices,NxU32 indexcount);

	bool    CleanupVertices(NxU32 svcount,
													const NxF32 *svertices,
													NxU32 stride,
													NxU32 &vcount,       // output number of vertices
													NxF32 *vertices,                 // location to store the results.
													NxF32  normalepsilon,
													NxF32 *scale);
};

}; // end of namespace

#endif
