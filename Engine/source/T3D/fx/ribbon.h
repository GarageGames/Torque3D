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

#ifndef _RIBBON_H_
#define _RIBBON_H_

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif

#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif

#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif

#include "materials/materialParameters.h"
#include "math/util/matrixSet.h"

#define RIBBON_NUM_FIELDS 4

//--------------------------------------------------------------------------
class RibbonData : public GameBaseData
{
   typedef GameBaseData Parent;

protected:
   bool onAdd();

public:

   U32 mRibbonLength; ///< The amount of segments that will make up the ribbon.
   F32 mSizes[RIBBON_NUM_FIELDS]; ///< The radius for each keyframe.
   ColorF mColours[RIBBON_NUM_FIELDS]; ///< The colour of the ribbon for each keyframe.
   F32 mTimes[RIBBON_NUM_FIELDS]; ///< The relative time for each keyframe.
   StringTableEntry mMatName; ///< The material for the ribbon.
   bool mUseFadeOut; ///< If true, the ribbon will fade away after deletion.
   F32 mFadeAwayStep; ///< How quickly the ribbons is faded away after deletion.
   S32 segmentsPerUpdate; ///< Amount of segments to add each update.
   F32 mTileScale; ///< A scalar to scale the texcoord.
   bool mFixedTexcoords; ///< If true, texcoords will stay the same over the lifetime for each segment.
   bool mTexcoordsRelativeToDistance; ///< If true, texcoords will not be stretched if the distance between 2 segments are long.
   S32 mSegmentSkipAmount; ///< The amount of segments to skip each time segments are added.

   RibbonData();

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, String &errorBuffer);

   static void initPersistFields();
   DECLARE_CONOBJECT(RibbonData);
};

//--------------------------------------------------------------------------
class Ribbon : public GameBase
{
   typedef GameBase Parent;
   RibbonData* mDataBlock;
   Vector<Point3F> mSegmentPoints; ///< The points in space where the ribbon has spawned segments.
   BaseMatInstance *mRibbonMat;
   MaterialParameterHandle* mRadiusSC;
   MaterialParameterHandle* mRibbonProjSC;
   GFXPrimitiveBufferHandle primBuffer;
   GFXVertexBufferHandle<GFXVertexPCNTT> verts;
   bool mUpdateBuffers; ///< If true, the vertex buffers need to be updated.
   bool mDeleteOnEnd; ///< If true, the ribbon should delete itself as soon as the last segment is deleted
   bool mUseFadeOut; ///< If true, the ribbon will fade away upon deletion
   F32 mFadeAwayStep; ///< How quickly the ribbons is faded away after deletion.
   F32 mFadeOut;
   U32 mSegmentOffset;
   U32 mSegmentIdx;
   F32 mTravelledDistance; ///< How far the ribbon has travelled in it's lifetime.

protected:

   bool onAdd();
   void processTick(const Move*);
   void advanceTime(F32);
   void interpolateTick(F32 delta);

   // Rendering
   void prepRenderImage(SceneRenderState *state);

   ///Checks to see if ribbon is too long
   U32 checkRibbonDistance(S32 segments);
   void setShaderParams();
   /// Construct the vertex and primitive buffers
   void createBuffers(SceneRenderState *state, GFXVertexBufferHandle<GFXVertexPCNTT> &verts, GFXPrimitiveBufferHandle &pb, U32 segments);

public:
   Ribbon();
   ~Ribbon();

   DECLARE_CONOBJECT(Ribbon);
   static void initPersistFields();
   bool onNewDataBlock(GameBaseData*,bool);
   void addSegmentPoint(Point3F &point, MatrixF &mat);  ///< Used to add another segment to the ribbon.
   void clearSegments() { mSegmentPoints.clear(); } ///< Delete all segments.
   void deleteOnEnd(); ///< Delete the ribbon when all segments have been deleted.
   void onRemove();

};

#endif // _H_RIBBON

