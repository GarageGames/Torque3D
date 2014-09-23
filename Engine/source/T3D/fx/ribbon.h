//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

//--------------------------------------------------------------------------
class RibbonData : public GameBaseData
{
   typedef GameBaseData Parent;

protected:
   bool onAdd();

public:

   enum Constants
   {
      NumFields = 4
   };

   F32 mSizes[NumFields];      ///< The radius for each keyframe.
   ColorF mColours[NumFields]; ///< The colour of the ribbon for each keyframe.
   F32 mTimes[NumFields];      ///< The relative time for each keyframe.

   U32 mRibbonLength;      ///< The amount of segments that will make up the ribbon.
   S32 segmentsPerUpdate;  ///< Amount of segments to add each update.
   S32 mSegmentSkipAmount; ///< The amount of segments to skip each time segments are added.

   bool mUseFadeOut;          ///< If true, the ribbon will fade away after deletion.
   F32 mFadeAwayStep;         ///< How quickly the ribbons is faded away after deletion.
   StringTableEntry mMatName; ///< The material for the ribbon.
   F32 mTileScale;            ///< A scalar to scale the texcoord.
   bool mFixedTexcoords;      ///< If true, texcoords will stay the same over the lifetime for each segment.
   bool mTexcoordsRelativeToDistance; ///< If true, texcoords will not be stretched if the distance between 2 segments are long.

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

   bool mDeleteOnEnd;   ///< If true, the ribbon should delete itself as soon as the last segment is deleted
   bool mUseFadeOut;    ///< If true, the ribbon will fade away upon deletion
   F32 mFadeAwayStep;   ///< How quickly the ribbons is faded away after deletion.
   F32 mFadeOut;
   F32 mTravelledDistance; ///< How far the ribbon has travelled in it's lifetime.

   Vector<Point3F> mSegmentPoints; ///< The points in space where the ribbon has spawned segments.
   U32 mSegmentOffset;
   U32 mSegmentIdx;

   bool mUpdateBuffers; ///< If true, the vertex buffers need to be updated.
   BaseMatInstance *mRibbonMat;
   MaterialParameterHandle* mRadiusSC;
   MaterialParameterHandle* mRibbonProjSC;
   GFXPrimitiveBufferHandle primBuffer;
   GFXVertexBufferHandle<GFXVertexPCNTT> verts;

protected:

   bool onAdd();
   void processTick(const Move*);
   void advanceTime(F32);
   void interpolateTick(F32 delta);

   // Rendering
   void prepRenderImage(SceneRenderState *state);
   void setShaderParams();

   ///Checks to see if ribbon is too long
   U32 checkRibbonDistance(S32 segments);

   /// Construct the vertex and primitive buffers
   void createBuffers(SceneRenderState *state, GFXVertexBufferHandle<GFXVertexPCNTT> &verts, GFXPrimitiveBufferHandle &pb, U32 segments);

public:
   Ribbon();
   ~Ribbon();

   DECLARE_CONOBJECT(Ribbon);
   static void initPersistFields();
   bool onNewDataBlock(GameBaseData*,bool);
   void onRemove();

   /// Used to add another segment to the ribbon.
   void addSegmentPoint(Point3F &point, MatrixF &mat);

   /// Delete all segments.
   void clearSegments() { mSegmentPoints.clear(); }

   /// Delete the ribbon when all segments have been deleted.
   void deleteOnEnd();
};

#endif // _H_RIBBON

