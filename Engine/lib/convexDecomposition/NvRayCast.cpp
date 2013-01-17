/*

NvRayCast.cpp : A code snippet to cast a ray against a triangle mesh. This implementation does not use any acceleration data structures.  That is a 'to do' item.

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
#include "NvRayCast.h"
#include "NvUserMemAlloc.h"
#include "NvFloatMath.h"

#pragma warning(disable:4100)

namespace CONVEX_DECOMPOSITION
{

class RayCast : public iRayCast, public Memalloc
{
public:
	RayCast(const NxF32 *vertices,NxU32 tcount,const NxU32 *indices)
	{
		mVertices = vertices;
		mTcount	  = tcount;
		mIndices  = indices;
	}

	~RayCast(void)
	{
	}

	virtual bool castRay(const NxF32 *orig,const NxF32 *dir,NxF32 *dest,NxF32 *hitNormal)
	{
		bool ret = false;

    	NxF32	p2[3];

    	const NxF32 RAY_DIST=50;

    	dest[0] = p2[0] = orig[0]+ dir[0]*RAY_DIST;
    	dest[1] = p2[1] = orig[1]+ dir[1]*RAY_DIST;
    	dest[2] = p2[2] = orig[2]+ dir[2]*RAY_DIST;

    	NxF32 nearest = 1e9;
    	NxU32 near_face=0;


    	for (NxU32 i=0; i<mTcount; i++)
    	{
    		NxU32 i1 = mIndices[i*3+0];
    		NxU32 i2 = mIndices[i*3+1];
    		NxU32 i3 = mIndices[i*3+2];

    		const NxF32 *t1 = &mVertices[i1*3];
    		const NxF32 *t2 = &mVertices[i2*3];
    		const NxF32 *t3 = &mVertices[i3*3];

    		NxF32 t;
    		if ( fm_rayIntersectsTriangle(orig,dir,t1,t2,t3,t) )
    		{
    			if ( t < nearest )
    			{
    				dest[0] = orig[0]+dir[0]*t;
    				dest[1] = orig[1]+dir[1]*t;
    				dest[2] = orig[2]+dir[2]*t;
    				ret = true;
    				near_face = i;
    				nearest = t;
    			}
    		}
    	}
    	if ( ret )
    	{
    		// If the nearest face we hit was back-facing, then reject this cast!
    		NxU32 i1 = mIndices[near_face*3+0];
    		NxU32 i2 = mIndices[near_face*3+1];
    		NxU32 i3 = mIndices[near_face*3+2];

    		const NxF32 *t1 = &mVertices[i1*3];
    		const NxF32 *t2 = &mVertices[i2*3];
    		const NxF32 *t3 = &mVertices[i3*3];

    		fm_computePlane(t3,t2,t1,hitNormal);
    	}

		return ret;
	}
private:
	const	NxF32	*mVertices;
	NxU32			 mTcount;
	const   NxU32	*mIndices;

};


iRayCast *createRayCast(const NxF32 *vertices,NxU32 tcount,const NxU32 *indices)
{
	RayCast *rc = MEMALLOC_NEW(RayCast)(vertices,tcount,indices);
	return static_cast< iRayCast *>(rc);
}

void	  releaseRayCast(iRayCast *rc)
{
	RayCast *r = static_cast< RayCast *>(rc);
	delete r;
}

};

