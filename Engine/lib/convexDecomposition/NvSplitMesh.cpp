/*

NvSplitMesh.cpp : A code snippet to split a mesh into two seperate meshes based on a slicing plane.

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

#include "NvSplitMesh.h"
#include "NvFloatMath.h"
#include "NvHashMap.h"

#pragma warning(disable:4100)

namespace CONVEX_DECOMPOSITION
{


typedef Array< NxU32 > NxU32Array;

class SplitMesh : public iSplitMesh, public Memalloc
{
public:
	SplitMesh(void)
	{
		mLeftVertices = 0;
		mRightVertices = 0;
	}

	~SplitMesh(void)
	{
		reset();
	}

	void reset(void)
	{
		if ( mLeftVertices )
		{
			fm_releaseVertexIndex(mLeftVertices);
			mLeftVertices = 0;
		}
		if ( mRightVertices )
		{
			fm_releaseVertexIndex(mRightVertices);
			mRightVertices = 0;
		}
		mLeftIndices.clear();
		mRightIndices.clear();
	}


	virtual void splitMesh(const NvSplitMesh &source,NvSplitMesh &leftMesh,NvSplitMesh &rightMesh,const NxF32 *planeEquation,NxF32 precision)
	{
		reset();

		mLeftVertices 	= fm_createVertexIndex(precision,false);
		mRightVertices 	= fm_createVertexIndex(precision,false);

		for (NxU32 i=0; i<source.mTcount; i++)
		{
			NxU32 i1 = source.mIndices[i*3+0];
			NxU32 i2 = source.mIndices[i*3+1];
			NxU32 i3 = source.mIndices[i*3+2];

			const NxF32 *p1 = &source.mVertices[i1*3];
			const NxF32 *p2 = &source.mVertices[i2*3];
			const NxF32 *p3 = &source.mVertices[i3*3];

			NxF32 source_tri[3*3];

			source_tri[0] = p1[0];
			source_tri[1] = p1[1];
			source_tri[2] = p1[2];

			source_tri[3] = p2[0];
			source_tri[4] = p2[1];
			source_tri[5] = p2[2];

			source_tri[6] = p3[0];
			source_tri[7] = p3[1];
			source_tri[8] = p3[2];

			NxF32 	front_tri[3*5];
			NxF32 	back_tri[3*5];

			NxU32	fcount,bcount;

			fm_planeTriIntersection(planeEquation,source_tri,sizeof(NxF32)*3,0.000001f,front_tri,fcount,back_tri,bcount);
			bool newPos;

			if ( fcount )
			{
				NxU32 i1,i2,i3,i4;
				i1 = mLeftVertices->getIndex( &front_tri[0],newPos );
				i2 = mLeftVertices->getIndex( &front_tri[3],newPos );
				i3 = mLeftVertices->getIndex( &front_tri[6],newPos );
				mLeftIndices.pushBack(i1);
				mLeftIndices.pushBack(i2);
				mLeftIndices.pushBack(i3);
				#if SHOW_DEBUG
				NVSHARE::gRenderDebug->setCurrentColor(0xFFFFFF);
				NVSHARE::gRenderDebug->DebugTri(&front_tri[0],&front_tri[3],&front_tri[6]);
				#endif
				if ( fcount == 4 )
				{
					i4 = mLeftVertices->getIndex( &front_tri[9],newPos );
					mLeftIndices.pushBack(i1);
					mLeftIndices.pushBack(i3);
					mLeftIndices.pushBack(i4);
 							#if SHOW_DEBUG
					NVSHARE::gRenderDebug->setCurrentColor(0xFFFF00);
 							NVSHARE::gRenderDebug->DebugTri(&front_tri[0],&front_tri[6],&front_tri[9]);
 							#endif
				}
			}
			if ( bcount )
			{
				NxU32 i1,i2,i3,i4;
				i1 = mRightVertices->getIndex( &back_tri[0],newPos );
				i2 = mRightVertices->getIndex( &back_tri[3],newPos );
				i3 = mRightVertices->getIndex( &back_tri[6],newPos );
				mRightIndices.pushBack(i1);
				mRightIndices.pushBack(i2);
				mRightIndices.pushBack(i3);
				#if SHOW_DEBUG
				NVSHARE::gRenderDebug->setCurrentColor(0xFF8080);
				NVSHARE::gRenderDebug->DebugTri(&back_tri[0],&back_tri[3],&back_tri[6]);
				#endif
				if ( bcount == 4 )
				{
					i4 = mRightVertices->getIndex( &back_tri[9],newPos );
					mRightIndices.pushBack(i1);
					mRightIndices.pushBack(i3);
					mRightIndices.pushBack(i4);
 							#if SHOW_DEBUG
					NVSHARE::gRenderDebug->setCurrentColor(0x00FF00);
 							NVSHARE::gRenderDebug->DebugTri(&back_tri[0],&back_tri[6],&back_tri[9]);
 							#endif
				}
			}
		}

		leftMesh.mVcount 	= mLeftVertices->getVcount();
		leftMesh.mVertices 	= mLeftVertices->getVerticesFloat();
		leftMesh.mTcount	= mLeftIndices.size()/3;
		leftMesh.mIndices	= &mLeftIndices[0];

		rightMesh.mVcount	= mRightVertices->getVcount();
		rightMesh.mVertices	= mRightVertices->getVerticesFloat();
		rightMesh.mTcount	= mRightIndices.size()/3;
		rightMesh.mIndices	= &mRightIndices[0];

	}


	fm_VertexIndex	*mLeftVertices;
	fm_VertexIndex	*mRightVertices;
 	NxU32Array		 mLeftIndices;
 	NxU32Array		 mRightIndices;
};

iSplitMesh *createSplitMesh(void)
{
	SplitMesh *sm = MEMALLOC_NEW(SplitMesh);
	return static_cast< iSplitMesh *>(sm);
}

void        releaseSplitMesh(iSplitMesh *splitMesh)
{
	SplitMesh *sm = static_cast< SplitMesh *>(splitMesh);
	delete sm;
}

}; // end of namespace
