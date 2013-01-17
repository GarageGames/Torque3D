#ifndef REMOVE_TJUNCTIONS_H

#define REMOVE_TJUNCTIONS_H

/*

NvRemoveTjunctions.h : A code snippet to remove tjunctions from a triangle mesh.  This version is currently disabled as it appears to have a bug.

*/


#include "NvUserMemAlloc.h"

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

namespace CONVEX_DECOMPOSITION
{

class RemoveTjunctionsDesc
{
public:
  RemoveTjunctionsDesc(void)
  {
    mVcount = 0;
    mVertices = 0;
    mTcount = 0;
    mIndices = 0;
    mIds = 0;
    mTcountOut = 0;
    mIndicesOut = 0;
    mIdsOut = 0;
	mEpsilon = 0.00000001f;
  }

// input
  NxF32        mEpsilon;
  NxF32        mDistanceEpsilon;
  NxU32        mVcount;  		// input vertice count.
  const NxF32 *mVertices; 		// input vertices as NxF32s or...
  NxU32        mTcount;    		// number of input triangles.
  const NxU32 *mIndices;   		// triangle indices.
  const NxU32 *mIds;       		// optional triangle Id numbers.
// output..
  NxU32        mTcountOut;  // number of output triangles.
  const NxU32 *mIndicesOut; // output triangle indices
  const NxU32 *mIdsOut;     // output retained id numbers.
};

// Removes t-junctions from an input mesh.  Does not generate any new data points, but may possible produce additional triangles and new indices.
class RemoveTjunctions
{
public:

   virtual NxU32 removeTjunctions(RemoveTjunctionsDesc &desc) =0; // returns number of triangles output and the descriptor is filled with the appropriate results.


};

RemoveTjunctions * createRemoveTjunctions(void);
void               releaseRemoveTjunctions(RemoveTjunctions *tj);

}; // end of namespace

#endif
