//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _TSSORTEDMESH_H_
#define _TSSORTEDMESH_H_

#ifndef _TSMESH_H_
#include "ts/tsMesh.h"
#endif

/// TSSortedMesh is for meshes that need sorting (obviously).  Such meshes
/// are usually partially or completely composed of translucent/parent polygons.
class TSSortedMesh : public TSMesh
{
public:
   typedef TSMesh Parent;

   /// This is a group of primitives that belong "together" in the rendering sequence.
   /// For example, if a player model had a helmet with a translucent visor, the visor
   /// would be a Cluster.
   struct Cluster
   {
      S32 startPrimitive;
      S32 endPrimitive;
      Point3F normal;
      F32 k;
      S32 frontCluster; ///< go to this cluster if in front of plane, if frontCluster<0, no cluster
      S32 backCluster;  ///< go to this cluster if in back of plane, if backCluster<0, no cluster
                        ///< if frontCluster==backCluster, no plane to test against...
   };

   Vector<Cluster> clusters; ///< All of the clusters of primitives to be drawn
   Vector<S32> startCluster; ///< indexed by frame number
   Vector<S32> firstVerts;   ///< indexed by frame number
   Vector<S32> numVerts;     ///< indexed by frame number
   Vector<S32> firstTVerts;  ///< indexed by frame number or matFrame number, depending on which one animates (never both)

   /// sometimes, we want to write the depth value to the frame buffer even when object is translucent
   bool alwaysWriteDepth;

   // render methods..
   void render(S32 frame, S32 matFrame, TSMaterialList *);

   bool buildPolyList( S32 frame, AbstractPolyList *polyList, U32 &surfaceKey, TSMaterialList *materials );
   bool castRay( S32 frame, const Point3F &start, const Point3F &end, RayInfo *rayInfo, TSMaterialList *materials );
   bool buildConvexHull(); ///< does nothing, skins don't use this
                           ///
                           ///  @returns false ALWAYS
   S32 getNumPolys();

   void assemble(bool skip);
   void disassemble();

   TSSortedMesh() {
      meshType = SortedMeshType;
   }
};

#endif // _TS_SORTED_MESH


