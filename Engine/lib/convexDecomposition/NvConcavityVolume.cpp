/*

NvConcavityVolume.cpp : This is a code snippet that computes the volume of concavity of a traingle mesh.

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

#define SHOW_DEBUG 0
#if SHOW_DEBUG
#include "RenderDebug.h"
#endif
#include "NvConcavityVolume.h"
#include "NvFloatMath.h"
#include "NvRayCast.h"
#include <stdio.h>

#pragma warning(disable:4100 4189 4505 4127 4101)

namespace CONVEX_DECOMPOSITION
{

bool raycast(const NxF32 *p1,const NxF32 *normal,NxF32 *dest,iRayCast *cast_hull,iRayCast *cast_mesh)
{
	bool ret = true;

	NxF32 hit_hull[3];
	NxF32 hit_hullNormal[3];

	NxF32 hit_mesh[3];
	NxF32 hit_meshNormal[3];

	bool hitHull = cast_hull->castRay(p1,normal,hit_hull,hit_hullNormal);
	bool hitMesh = cast_mesh->castRay(p1,normal,hit_mesh,hit_meshNormal);

	if ( hitMesh )
	{
		float dot = fm_dot(normal,hit_meshNormal);
		if ( dot < 0 ) // skip if we hit an internal face of the mesh when projection out towards the convex hull.
		{
			ret = false;
		}
		else
		{
			NxF32 d1 = fm_distanceSquared(p1,hit_mesh);
			NxF32 d2 = fm_distanceSquared(p1,hit_hull);
			if ( d1 < d2 )
			{
				dest[0] = hit_mesh[0];
				dest[1] = hit_mesh[1];
				dest[2] = hit_mesh[2];
			}
			else
			{
				dest[0] = hit_hull[0];
				dest[1] = hit_hull[1];
				dest[2] = hit_hull[2];
			}
		}
	}
	else if ( hitHull )
	{
		dest[0] = hit_hull[0];
		dest[1] = hit_hull[1];
		dest[2] = hit_hull[2];
	}
	else
	{
		ret = false;
	}


	return ret;
}

void addTri(NxU32 *indices,NxU32 i1,NxU32 i2,NxU32 i3,NxU32 &tcount)
{
	indices[tcount*3+0] = i1;
	indices[tcount*3+1] = i2;
	indices[tcount*3+2] = i3;
	tcount++;
}

NxF32 computeConcavityVolume(NxU32 vcount_hull,
						     const NxF32 *vertices_hull,
						     NxU32 tcount_hull,
						     const NxU32 *indices_hull,
						     NxU32 vcount_mesh,
						     const NxF32 *vertices_mesh,
						     NxU32 tcount_mesh,
						     const NxU32 *indices_mesh)
{
	NxF32 total_volume = 0;

#if SHOW_DEBUG
	NVSHARE::gRenderDebug->pushRenderState();
	NVSHARE::gRenderDebug->setCurrentDisplayTime(150.0f);
#endif

	iRayCast *cast_hull = createRayCast(vertices_hull,tcount_hull,indices_hull);
	iRayCast *cast_mesh = createRayCast(vertices_mesh,tcount_mesh,indices_mesh);


	const NxU32 *indices = indices_mesh;
#if 0
	static NxU32 index = 0;
	NxU32 i = index++;
	indices = &indices[i*3];
#else
	for (NxU32 i=0; i<tcount_mesh; i++)
#endif
	{
		NxU32 i1 = indices[0];
		NxU32 i2 = indices[1];
		NxU32 i3 = indices[2];

		const NxF32 *p1 = &vertices_mesh[i1*3];
		const NxF32 *p2 = &vertices_mesh[i2*3];
		const NxF32 *p3 = &vertices_mesh[i3*3];

		NxF32 normal[3];
		NxF32 d = fm_computePlane(p3,p2,p1,normal);

		NxF32  vertices[6*3];

		vertices[0] = p1[0];
		vertices[1] = p1[1];
		vertices[2] = p1[2];

		vertices[3] = p2[0];
		vertices[4] = p2[1];
		vertices[5] = p2[2];

		vertices[6] = p3[0];
		vertices[7] = p3[1];
		vertices[8] = p3[2];

		NxF32 midPoint[3];
		midPoint[0] = (p1[0]+p2[0]+p3[0])/3;
		midPoint[1] = (p1[1]+p2[1]+p3[1])/3;
		midPoint[2] = (p1[2]+p2[2]+p3[2])/3;

		fm_lerp(midPoint,p1,&vertices[0],0.9999f);
		fm_lerp(midPoint,p2,&vertices[3],0.9999f);
		fm_lerp(midPoint,p3,&vertices[6],0.9999f);

		NxF32 *_p1 = &vertices[3*3];
		NxF32 *_p2 = &vertices[4*3];
		NxF32 *_p3 = &vertices[5*3];

		NxU32 hitCount = 0;

		if ( raycast(&vertices[0],normal, _p1,cast_hull,cast_mesh) ) hitCount++;
		if ( raycast(&vertices[3],normal, _p2,cast_hull,cast_mesh) ) hitCount++;
		if ( raycast(&vertices[6],normal, _p3,cast_hull,cast_mesh) ) hitCount++;

		// form triangle mesh!
		if ( hitCount == 3 )
		{
			NxU32 tcount = 0;
			NxU32 tindices[8*3];

			addTri(tindices,2,1,0,tcount);
			addTri(tindices,3,4,5,tcount);

			addTri(tindices,0,3,2,tcount);
			addTri(tindices,2,3,5,tcount);

			addTri(tindices,1,3,0,tcount);
			addTri(tindices,4,3,1,tcount);

			addTri(tindices,5,4,1,tcount);
			addTri(tindices,2,5,1,tcount);

			NxF32 volume = fm_computeMeshVolume(vertices,tcount,tindices);
			total_volume+=volume;
#if SHOW_DEBUG
			NVSHARE::gRenderDebug->setCurrentColor(0x0000FF,0xFFFFFF);
			NVSHARE::gRenderDebug->addToCurrentState(NVSHARE::DebugRenderState::SolidWireShaded);

			for (NxU32 i=0; i<tcount; i++)
			{
				NxU32 i1 = tindices[i*3+0];
				NxU32 i2 = tindices[i*3+1];
				NxU32 i3 = tindices[i*3+2];

				const NxF32 *p1 = &vertices[i1*3];
				const NxF32 *p2 = &vertices[i2*3];
				const NxF32 *p3 = &vertices[i3*3];

				NVSHARE::gRenderDebug->DebugTri(p1,p2,p3);
			}
#endif
		}
		indices+=3;
	}
#if SHOW_DEBUG
	NVSHARE::gRenderDebug->popRenderState();
#endif

	releaseRayCast(cast_hull);
	releaseRayCast(cast_mesh);

	return total_volume;
}

}; // end of namespace
