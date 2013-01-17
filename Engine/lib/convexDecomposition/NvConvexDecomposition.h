#ifndef CONVEX_DECOMPOSITION_H

#define CONVEX_DECOMPOSITION_H

/*

NvConvexDecomposition.h : The main interface to the convex decomposition library.

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

#include "NvSimpleTypes.h"

namespace CONVEX_DECOMPOSITION
{

struct ConvexHullResult
{
	NxU32	mVcount;				// number of vertices.
	NxF32	*mVertices;				// vertex positions.
	NxU32	mTcount;				// number of triangles.
	NxU32	*mIndices;				// indexed triangle list.
};

class iConvexDecomposition
{
public:
	virtual void reset(void) = 0; // reset the input mesh data.

	virtual bool addTriangle(const NxF32 *p1,const NxF32 *p2,const NxF32 *p3) = 0; // add the input mesh one triangle at a time.

	virtual NxU32 computeConvexDecomposition(NxF32 skinWidth=0,			// Skin width on the convex hulls generated
											 NxU32 decompositionDepth=8, // recursion depth for convex decomposition.
											 NxU32 maxHullVertices=64,	// maximum number of vertices in output convex hulls.
											 NxF32 concavityThresholdPercent=0.1f, // The percentage of concavity allowed without causing a split to occur.
											 NxF32 mergeThresholdPercent=30.0f,    // The percentage of volume difference allowed to merge two convex hulls.
											 NxF32 volumeSplitThresholdPercent=0.1f, // The percentage of the total volume of the object above which splits will still occur.
											 bool  useInitialIslandGeneration=true,	// whether or not to perform initial island generation on the input mesh.
											 bool  useIslandGeneration=false,		// Whether or not to perform island generation at each split.  Currently disabled due to bug in RemoveTjunctions
											 bool  useBackgroundThread=true) = 0;	// Whether or not to compute the convex decomposition in a background thread, the default is true.

	virtual bool isComputeComplete(void) = 0; // if building the convex hulls in a background thread, this returns true if it is complete.

	virtual bool cancelCompute(void) = 0; // cause background thread computation to abort early.  Will return no results. Use 'isComputeComplete' to confirm the thread is done.


	virtual NxU32 getHullCount(void)  = 0; // returns the number of convex hulls produced.
	virtual bool  getConvexHullResult(NxU32 hullIndex,ConvexHullResult &result) = 0; // returns each convex hull result.

protected:
   	virtual ~iConvexDecomposition(void)
   	{
	}

};


iConvexDecomposition * createConvexDecomposition(void);
void				   releaseConvexDecomposition(iConvexDecomposition *ic);

}; // end of namespace

#endif
