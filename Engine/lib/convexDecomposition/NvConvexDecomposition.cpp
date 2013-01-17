
/*

NvConvexDecomposition.cpp : The main interface to the convex decomposition library.

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
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "NvConvexDecomposition.h"
#include "NvHashMap.h"
#include "NvFloatMath.h"
#include "NvRemoveTjunctions.h"
#include "NvMeshIslandGeneration.h"
#include "NvStanHull.h"
#include "NvConcavityVolume.h"
#include "NvSplitMesh.h"
#include "NvThreadConfig.h"


#pragma warning(disable:4996 4100 4189)

namespace CONVEX_DECOMPOSITION
{


#define GRANULARITY 0.0000000001f

typedef CONVEX_DECOMPOSITION::Array< NxU32 > NxU32Array;

class ConvexHull : public Memalloc
{
public:
	ConvexHull(NxU32 vcount,const NxF32 *vertices,NxU32 tcount,const NxU32 *indices)
	{
		mTested = false;
		mVcount = vcount;
		mTcount = tcount;
		mVertices = 0;
		mIndices = 0;
		mHullVolume = 0;
		if ( vcount )
		{
			mVertices = (NxF32 *)MEMALLOC_MALLOC(sizeof(NxF32)*3*vcount);
			memcpy(mVertices,vertices,sizeof(NxF32)*3*vcount);
		}
		if ( tcount )
		{
			mIndices = (NxU32 *)MEMALLOC_MALLOC(sizeof(NxU32)*3*tcount);
			memcpy(mIndices,indices,sizeof(NxU32)*3*tcount);
		}
		if ( mVcount && mTcount )
		{
			mHullVolume = fm_computeMeshVolume( mVertices, mTcount, mIndices);
		}
	}

	~ConvexHull(void)
	{
		reset();
	}

    void reset(void)
    {
		MEMALLOC_FREE(mVertices);
		MEMALLOC_FREE(mIndices);
		mVertices = 0;
		mIndices = 0;
		mVcount = 0;
		mTcount = 0;
		mHullVolume = 0;
	}

	// return true if merging this hull with the 'mergeHull' produces a new convex hull which is no greater in volume than the
	// mergeThresholdPercentage
	bool canMerge(ConvexHull *mergeHull,NxF32 mergeThresholdPercent,NxU32 maxVertices,NxF32 skinWidth,NxF32 &percent)
	{
		bool ret = false;

		if ( mHullVolume > 0 && mergeHull->mHullVolume > 0 )
		{
			NxU32 combineVcount = mVcount + mergeHull->mVcount;
			NxF32 *vertices = (NxF32 *)MEMALLOC_MALLOC(sizeof(NxF32)*combineVcount*3);
			NxF32 *dest = vertices;
			const NxF32 *source = mVertices;

			for (NxU32 i=0; i<mVcount; i++)
			{
				dest[0] = source[0];
				dest[1] = source[1];
				dest[2] = source[2];
				dest+=3;
				source+=3;
			}
			source = mergeHull->mVertices;
			for (NxU32 i=0; i<mergeHull->mVcount; i++)
			{
				dest[0] = source[0];
				dest[1] = source[1];
				dest[2] = source[2];
				dest+=3;
				source+=3;
			}

			// create the combined convex hull.
    		HullDesc hd;
    		hd.mVcount 			= combineVcount;
    		hd.mVertices 		= vertices;
    		hd.mVertexStride 	= sizeof(NxF32)*3;
    		hd.mMaxVertices 	= maxVertices;
    		hd.mSkinWidth		= skinWidth;
    		HullLibrary hl;
    		HullResult result;
    		hl.CreateConvexHull(hd,result);

			NxF32 combinedVolume = fm_computeMeshVolume(result.mOutputVertices, result.mNumFaces, result.mIndices );
			NxF32 seperateVolume = mHullVolume+mergeHull->mHullVolume;

			NxF32 percentMerge = 100 - (seperateVolume*100 / combinedVolume );

			if ( percentMerge <= mergeThresholdPercent )
			{
				percent = percentMerge;
				ret = true;
			}
			MEMALLOC_FREE(vertices);
			hl.ReleaseResult(result);
		}
		return ret;
	}

	void merge(ConvexHull *mergeHull,NxU32 maxVertices,NxF32 skinWidth)
	{
		NxU32 combineVcount = mVcount + mergeHull->mVcount;
		NxF32 *vertices = (NxF32 *)MEMALLOC_MALLOC(sizeof(NxF32)*combineVcount*3);
		NxF32 *dest = vertices;
		const NxF32 *source = mVertices;

		for (NxU32 i=0; i<mVcount; i++)
		{
			dest[0] = source[0];
			dest[1] = source[1];
			dest[2] = source[2];
			dest+=3;
			source+=3;
		}
		source = mergeHull->mVertices;
		for (NxU32 i=0; i<mergeHull->mVcount; i++)
		{
			dest[0] = source[0];
			dest[1] = source[1];
			dest[2] = source[2];
			dest+=3;
			source+=3;
		}

		// create the combined convex hull.
   		HullDesc hd;
   		hd.mVcount 			= combineVcount;
   		hd.mVertices 		= vertices;
   		hd.mVertexStride 	= sizeof(NxF32)*3;
   		hd.mMaxVertices 	= maxVertices;
   		hd.mSkinWidth		= skinWidth;
   		HullLibrary hl;
   		HullResult result;
   		hl.CreateConvexHull(hd,result);

		reset();
		mergeHull->reset();
		mergeHull->mTested = true; // it's been tested.
		mVcount = result.mNumOutputVertices;
		mVertices = (NxF32 *)MEMALLOC_MALLOC(sizeof(NxF32)*3*mVcount);
		memcpy(mVertices,result.mOutputVertices,sizeof(NxF32)*3*mVcount);
		mTcount = result.mNumFaces;
		mIndices = (NxU32 *)MEMALLOC_MALLOC(sizeof(NxU32)*mTcount*3);
		memcpy(mIndices, result.mIndices, sizeof(NxU32)*mTcount*3);

		MEMALLOC_FREE(vertices);
		hl.ReleaseResult(result);
	}

	void setTested(bool state)
	{
		mTested = state;
	}

    bool beenTested(void) const { return mTested; };

	bool    mTested;
	NxF32	mHullVolume;
	NxU32	mVcount;
	NxF32	*mVertices;
	NxU32	mTcount;
	NxU32	*mIndices;
};

typedef Array< ConvexHull *> ConvexHullVector;

class ConvexDecomposition : public iConvexDecomposition, public CONVEX_DECOMPOSITION::Memalloc, public ThreadInterface
{
public:
	ConvexDecomposition(void)
	{
		mVertexIndex = 0;
		mComplete = false;
		mCancel = false;
		mThread = 0;
	}

	~ConvexDecomposition(void)
	{
		wait();
		reset();
		if ( mThread )
		{
			tc_releaseThread(mThread);
		}
	}

	void wait(void) const
	{
		while ( mThread && !mComplete );
	}

	virtual void reset(void)  // reset the input mesh data.
	{
		wait();
		if ( mVertexIndex )
		{
			fm_releaseVertexIndex(mVertexIndex);
			mVertexIndex = 0;
		}
		mIndices.clear();
		ConvexHullVector::Iterator i;
		for (i=mHulls.begin(); i!=mHulls.end(); ++i)
		{
			ConvexHull *ch = (*i);
			delete ch;
		}
		mHulls.clear();
	}

	virtual bool addTriangle(const NxF32 *p1,const NxF32 *p2,const NxF32 *p3)
	{
		bool ret = true;
		wait();
		if ( mVertexIndex == 0 )
		{
			mVertexIndex = fm_createVertexIndex(GRANULARITY,false);
		}

		bool newPos;
		NxU32 i1 = mVertexIndex->getIndex(p1,newPos);
		NxU32 i2 = mVertexIndex->getIndex(p2,newPos);
		NxU32 i3 = mVertexIndex->getIndex(p3,newPos);

		if ( i1 == i2 || i1 == i3 || i2 == i3 )
		{
			ret = false; // triangle is degenerate
		}
		else
		{
			mIndices.pushBack(i1);
			mIndices.pushBack(i2);
			mIndices.pushBack(i3);
		}
		return ret;
	}

	ConvexHull * getNonTested(void) const
	{
		ConvexHull *ret = 0;
		for (NxU32 i=0; i<mHulls.size(); i++)
		{
			ConvexHull *ch = mHulls[i];
			if ( !ch->beenTested() )
			{
				ret = ch;
				break;
			}
		}
		return ret;
	}

	virtual NxU32 computeConvexDecomposition(NxF32 skinWidth,
											 NxU32 decompositionDepth,
											 NxU32 maxHullVertices,
											 NxF32 concavityThresholdPercent,
											 NxF32 mergeThresholdPercent,
											 NxF32 volumeSplitThresholdPercent,
											 bool  useInitialIslandGeneration,
											 bool  useIslandGeneration,
											 bool  useThreads)
	{
		NxU32 ret = 0;

		if ( mThread )
			return 0;

		if ( mVertexIndex )
		{

			mSkinWidth = skinWidth;
			mDecompositionDepth = decompositionDepth;
			mMaxHullVertices = maxHullVertices;
			mConcavityThresholdPercent = concavityThresholdPercent;
			mMergeThresholdPercent = mergeThresholdPercent;
			mVolumeSplitThresholdPercent = volumeSplitThresholdPercent;
			mUseInitialIslandGeneration = useInitialIslandGeneration;
			mUseIslandGeneration = false; // Not currently supported. useIslandGeneration;
			mComplete = false;
			mCancel   = false;

			if ( useThreads )
			{
				mThread = tc_createThread(this);
			}
			else
			{
				threadMain();
				ret = getHullCount();
       		}
    	}
		return ret;
	}

	void performConvexDecomposition(NxU32 vcount,
									 const NxF32 *vertices,
									 NxU32 tcount,
									 const NxU32 *indices,
									 NxF32 skinWidth,
									 NxU32 decompositionDepth,
									 NxU32 maxHullVertices,
									 NxF32 concavityThresholdPercent,
									 NxF32 mergeThresholdPercent,
									 NxF32 volumeSplitThresholdPercent,
									 bool  useInitialIslandGeneration,
									 bool  useIslandGeneration,
									 NxU32 depth)
	{
		if ( mCancel ) return;
		if ( depth >= decompositionDepth ) return;

		RemoveTjunctionsDesc desc;
		desc.mVcount 	= vcount;
		desc.mVertices 	= vertices;
		desc.mTcount	= tcount;
		desc.mIndices	= indices;

#if 0
		RemoveTjunctions *rt = createRemoveTjunctions();
		rt->removeTjunctions(desc);
#else

		desc.mTcountOut = desc.mTcount;
		desc.mIndicesOut = desc.mIndices;

#endif
   	    // ok..we now have a clean mesh without any tjunctions.
		bool island = (depth == 0 ) ? useInitialIslandGeneration : useIslandGeneration;
   	    if ( island )
   	    {
   	    	MeshIslandGeneration *mi = createMeshIslandGeneration();
   	    	NxU32 icount = mi->islandGenerate(desc.mTcountOut,desc.mIndicesOut,desc.mVertices);
   	    	for (NxU32 i=0; i<icount && !mCancel; i++)
   	    	{
				NxU32 tcount;
   	    		NxU32 *indices = mi->getIsland(i,tcount);

   	    		baseConvexDecomposition(desc.mVcount,desc.mVertices,
											tcount,indices,
   	    									skinWidth,
   	    									decompositionDepth,
   	    									maxHullVertices,
   											concavityThresholdPercent,
   											mergeThresholdPercent,
   											volumeSplitThresholdPercent,
											useInitialIslandGeneration,
   											useIslandGeneration,depth);
   			}
   			releaseMeshIslandGeneration(mi);
   	    }
   	    else
   	    {
       		baseConvexDecomposition(desc.mVcount,desc.mVertices,desc.mTcountOut,
									desc.mIndicesOut,
   									skinWidth,
   									decompositionDepth,
   									maxHullVertices,
									concavityThresholdPercent,
									mergeThresholdPercent,
									volumeSplitThresholdPercent,
									useInitialIslandGeneration,
   									useIslandGeneration,depth);
   	    }
#if 0
   	    releaseRemoveTjunctions(rt);
#endif
	}

	virtual void baseConvexDecomposition(NxU32 vcount,
										 const NxF32 *vertices,
										 NxU32 tcount,
										 const NxU32 *indices,
										 NxF32 skinWidth,
										 NxU32 decompositionDepth,
										 NxU32 maxHullVertices,
										 NxF32 concavityThresholdPercent,
										 NxF32 mergeThresholdPercent,
										 NxF32 volumeSplitThresholdPercent,
										 bool  useInitialIslandGeneration,
										 bool  useIslandGeneration,
										 NxU32 depth)
	{

		if ( mCancel ) return;

		bool split = false; // by default we do not split


		NxU32 *out_indices 	= (NxU32 *)MEMALLOC_MALLOC( sizeof(NxU32)*tcount*3 );
		NxF32 *out_vertices = (NxF32 *)MEMALLOC_MALLOC( sizeof(NxF32)*3*vcount );

		NxU32 out_vcount = fm_copyUniqueVertices( vcount, vertices, out_vertices, tcount, indices, out_indices );
		// get a copy of only the unique vertices which are actually being used.

		HullDesc hd;
		hd.mVcount 			= out_vcount;
		hd.mVertices 		= out_vertices;
		hd.mVertexStride 	= sizeof(NxF32)*3;
		hd.mMaxVertices 	= maxHullVertices;
		hd.mSkinWidth		= skinWidth;
		HullLibrary hl;
		HullResult result;
		hl.CreateConvexHull(hd,result);

		NxF32 meshVolume = fm_computeMeshVolume(result.mOutputVertices, result.mNumFaces, result.mIndices );

		if ( (depth+1) < decompositionDepth )
		{
			// compute the volume of this mesh...
			NxF32 percentVolume = (meshVolume*100)/mOverallMeshVolume; // what percentage of the overall mesh volume are we?
			if ( percentVolume > volumeSplitThresholdPercent ) // this piece must be greater thant he volume split threshold percent
			{
				// ok..now we will compute the concavity...
				NxF32 concave_volume = computeConcavityVolume(result.mNumOutputVertices, result.mOutputVertices, result.mNumFaces, result.mIndices, out_vcount, out_vertices,	tcount, out_indices );
				NxF32 concave_percent = (concave_volume*100) / meshVolume;
				if ( concave_percent >=	concavityThresholdPercent )
				{
					// ready to do split here..
					split = true;
				}
			}
		}

		if ( !split )
		{
			saveConvexHull(result.mNumOutputVertices,result.mOutputVertices,result.mNumFaces,result.mIndices);
		}

		// Compute the best fit plane relative to the computed convex hull.
		NxF32 plane[4];
		bool ok = fm_computeSplitPlane(result.mNumOutputVertices,result.mOutputVertices,result.mNumFaces,result.mIndices,plane);
		assert(ok);

		hl.ReleaseResult(result);
		MEMALLOC_FREE(out_indices);
		MEMALLOC_FREE(out_vertices);

		if ( split )
		{
			iSplitMesh *sm = createSplitMesh();

			NvSplitMesh n;
			n.mVcount 	=	vcount;
			n.mVertices =  vertices;
			n.mTcount	= tcount;
			n.mIndices	= indices;
			if ( ok )
			{
				NvSplitMesh leftMesh;
				NvSplitMesh rightMesh;

				sm->splitMesh(n,leftMesh,rightMesh,plane,GRANULARITY);

				if ( leftMesh.mTcount )
				{
					performConvexDecomposition(leftMesh.mVcount,
											   leftMesh.mVertices,
											   leftMesh.mTcount,
											   leftMesh.mIndices,
											   skinWidth,
											   decompositionDepth,
											   maxHullVertices,
											   concavityThresholdPercent,
											   mergeThresholdPercent,
											   volumeSplitThresholdPercent,
											   useInitialIslandGeneration,
											   useIslandGeneration,
											   depth+1);

				}
				if ( rightMesh.mTcount )
				{
					performConvexDecomposition(rightMesh.mVcount,
											   rightMesh.mVertices,
											   rightMesh.mTcount,
											   rightMesh.mIndices,
											   skinWidth,
											   decompositionDepth,
											   maxHullVertices,
											   concavityThresholdPercent,
											   mergeThresholdPercent,
											   volumeSplitThresholdPercent,
											   useInitialIslandGeneration,
											   useIslandGeneration,
											   depth+1);
				}
			}
			releaseSplitMesh(sm);
		}
	}

	// Copies only the vertices which are actually used.
	// Then computes the convex hull around these used vertices.
	// Next computes the volume of this convex hull.
	// Frees up scratch memory and returns the volume of the convex hull around the source triangle mesh.
	NxF32 computeHullMeshVolume(NxU32 vcount,const NxF32 *vertices,NxU32 tcount,const NxU32 *indices,NxU32 maxVertices,NxF32 skinWidth)
	{
		if ( mCancel ) return 0;
		// first thing we should do is compute the overall mesh volume.
		NxU32 *out_indices 	= (NxU32 *)MEMALLOC_MALLOC( sizeof(NxU32)*tcount*3 );
		NxF32 *out_vertices = (NxF32 *)MEMALLOC_MALLOC( sizeof(NxF32)*3*vcount );

		NxU32 out_vcount = fm_copyUniqueVertices( vcount, vertices, out_vertices, tcount, indices, out_indices );
		// get a copy of only the unique vertices which are actually being used.

		HullDesc hd;
		hd.mVcount 			= out_vcount;
		hd.mVertices 		= out_vertices;
		hd.mVertexStride 	= sizeof(NxF32)*3;
		hd.mMaxVertices 	= maxVertices;
		hd.mSkinWidth		= skinWidth;
		HullLibrary hl;
		HullResult result;
		hl.CreateConvexHull(hd,result);

		NxF32 volume = fm_computeMeshVolume(result.mOutputVertices, result.mNumFaces, result.mIndices );

		hl.ReleaseResult(result);
		MEMALLOC_FREE(out_indices);
		MEMALLOC_FREE(out_vertices);

		return volume;
	}


	virtual bool isComputeComplete(void)  // if building the convex hulls in a background thread, this returns true if it is complete.
	{
		bool ret = true;

		if ( mThread )
		{
			ret = mComplete;
			if ( ret )
			{
				tc_releaseThread(mThread);
				mThread = 0;
			}
		}

		return ret;
	}


	virtual NxU32 getHullCount(void) 
	{
		NxU32 hullCount = 0;
		wait();
		if ( mCancel )
		{
			reset();
		}
		for (NxU32 i=0; i<mHulls.size(); i++)
		{
			ConvexHull *ch = mHulls[i];
			if ( ch->mTcount )
			{
				hullCount++;
			}
		}
		return hullCount;
	}

	virtual bool  getConvexHullResult(NxU32 hullIndex,ConvexHullResult &result)
	{
		bool ret = false;

		wait();
		NxU32 index = 0;
		for (NxU32 i=0; i<mHulls.size(); i++)
		{
			ConvexHull *ch = mHulls[i];
			if ( ch->mTcount )
			{
				if ( hullIndex == index )
				{
					ret = true;
					result.mVcount = ch->mVcount;
					result.mTcount = ch->mTcount;
					result.mVertices = ch->mVertices;
					result.mIndices = ch->mIndices;
					break;
				}
				index++;
			}
		}

		return ret;
	}

	void saveConvexHull(NxU32 vcount,const NxF32 *vertices,NxU32 tcount,const NxU32 *indices)
	{
		ConvexHull *ch = MEMALLOC_NEW(ConvexHull)(vcount,vertices,tcount,indices);
		mHulls.pushBack(ch);
	}

  	virtual void threadMain(void)
  	{
    	mOverallMeshVolume = computeHullMeshVolume( mVertexIndex->getVcount(),
    												mVertexIndex->getVerticesFloat(),
    												mIndices.size()/3,
    												&mIndices[0],
    												mMaxHullVertices, mSkinWidth );

   		performConvexDecomposition(mVertexIndex->getVcount(),mVertexIndex->getVerticesFloat(),
         							       	mIndices.size()/3,&mIndices[0],
         									mSkinWidth,
         									mDecompositionDepth,
         									mMaxHullVertices,
     										mConcavityThresholdPercent,
     										mMergeThresholdPercent,
     										mVolumeSplitThresholdPercent,
    										mUseInitialIslandGeneration,
     										mUseIslandGeneration,0);

		if ( mHulls.size() && !mCancel )
		{
			// While convex hulls can be merged...
			ConvexHull *ch = getNonTested();
			while ( ch && !mCancel )
			{
				// Sort all convex hulls by volume, largest to smallest.
				NxU32 hullCount = mHulls.size();
				ConvexHull *bestHull = 0;
				NxF32 bestPercent = 100;

				for (NxU32 i=0; i<hullCount; i++)
				{
					ConvexHull *mergeHull = mHulls[i];
					if ( !mergeHull->beenTested() && mergeHull != ch )
					{
						NxF32 percent;
            			if ( ch->canMerge(mergeHull,mMergeThresholdPercent,mMaxHullVertices,mSkinWidth,percent) )
            			{
            				if ( percent < bestPercent )
            				{
            					bestHull = mergeHull;
            					bestPercent = percent;
            				}
            			}
            		}
				}

				if ( bestHull )
				{
            		ch->merge(bestHull,mMaxHullVertices,mSkinWidth);
				}
				else
				{
					ch->setTested(true);
				}

				ch = getNonTested();
			}
		}
    	mComplete = true;
  	}

	virtual bool cancelCompute(void)  // cause background thread computation to abort early.  Will return no results. Use 'isComputeComplete' to confirm the thread is done.
	{
		bool ret = false;

		if ( mThread && !mComplete )
		{
			mCancel = true;
			ret = true;
		}

		return ret;
	}

private:
	bool				mComplete;
	bool				mCancel;
	fm_VertexIndex 		*mVertexIndex;
	NxU32Array			mIndices;
	NxF32				mOverallMeshVolume;
	ConvexHullVector	mHulls;
	Thread				*mThread;

	NxF32 				mSkinWidth;
	NxU32 				mDecompositionDepth;
	NxU32 				mMaxHullVertices;
	NxF32 				mConcavityThresholdPercent;
	NxF32 				mMergeThresholdPercent;
	NxF32 				mVolumeSplitThresholdPercent;
	bool  				mUseInitialIslandGeneration;
	bool  				mUseIslandGeneration;

};


iConvexDecomposition * createConvexDecomposition(void)
{
	ConvexDecomposition *cd = MEMALLOC_NEW(ConvexDecomposition);
	return static_cast< iConvexDecomposition *>(cd);

}

void				   releaseConvexDecomposition(iConvexDecomposition *ic)
{
	ConvexDecomposition *cd = static_cast< ConvexDecomposition *>(ic);
	delete cd;
}

}; // end of namespace
