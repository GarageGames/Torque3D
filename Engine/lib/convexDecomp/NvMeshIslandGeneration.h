#ifndef MESH_ISLAND_GENERATION_H

#define MESH_ISLAND_GENERATION_H

/*

NvMeshIslandGeneration.h : This code snippet walks the toplogy of a triangle mesh and detects the set of unique connected 'mesh islands'

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

class MeshIslandGeneration
{
public:

  virtual NxU32 islandGenerate(NxU32 tcount,const NxU32 *indices,const NxF32 *vertices) = 0;
  virtual NxU32 islandGenerate(NxU32 tcount,const NxU32 *indices,const NxF64 *vertices) = 0;

  // sometimes island generation can produce co-planar islands.  Slivers if you will.  If you are passing these islands into a geometric system
  // that wants to turn them into physical objects, they may not be acceptable.  In this case it may be preferable to merge the co-planar islands with
  // other islands that it 'touches'.
  virtual NxU32 mergeCoplanarIslands(const NxF32 *vertices) = 0;
  virtual NxU32 mergeCoplanarIslands(const NxF64 *vertices) = 0;

  virtual NxU32 mergeTouchingIslands(const NxF32 *vertices) = 0;
  virtual NxU32 mergeTouchingIslands(const NxF64 *vertices) = 0;

  virtual NxU32 *   getIsland(NxU32 index,NxU32 &tcount) = 0;


};

MeshIslandGeneration * createMeshIslandGeneration(void);
void                   releaseMeshIslandGeneration(MeshIslandGeneration *cm);

}; // end of namespace

#endif
