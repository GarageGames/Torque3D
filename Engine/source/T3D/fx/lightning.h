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

#ifndef _LIGHTNING_H_
#define _LIGHTNING_H_

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#ifndef _TORQUE_LIST_
#include "core/util/tList.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif
#ifndef _RENDERPASSMANAGER_H_
#include "renderInstance/renderPassManager.h"
#endif
#ifndef _ENGINEAPI_H_
#include "console/engineAPI.h"
#endif

#include "gfx/gfxTextureHandle.h"



class ShapeBase;
class LightningStrikeEvent;
class SFXTrack;


// -------------------------------------------------------------------------
class LightningData : public GameBaseData
{
   typedef GameBaseData Parent;

  public:
   enum Constants {
      MaxThunders = 8,
      MaxTextures = 8
   };

   //-------------------------------------- Console set variables
  public:
   SFXTrack*          thunderSounds[MaxThunders];
   SFXTrack*         strikeSound;
   StringTableEntry  strikeTextureNames[MaxTextures];

   //-------------------------------------- load set variables
  public:

   GFXTexHandle  strikeTextures[MaxTextures];
   U32           numThunders;

  protected:
   bool onAdd();

  public:
   LightningData();
   ~LightningData();

   void packData(BitStream*);
   void unpackData(BitStream*);
   bool preload(bool server, String &errorStr);

   DECLARE_CONOBJECT(LightningData);
   static void initPersistFields();
};


// -------------------------------------------------------------------------
struct LightningBolt
{

   struct Node
   {
      Point3F        point;
      VectorF        dirToMainLine;
   };

   struct NodeManager
   {
      Node     nodeList[10];

      Point3F  startPoint;
      Point3F  endPoint;
      U32      numNodes;
      F32      maxAngle;

      void generateNodes();
   };

   NodeManager mMajorNodes;
   Vector< NodeManager > mMinorNodes;
   
   typedef Torque::List<LightningBolt> LightingBoltList;
   LightingBoltList splitList;

   F32      lifetime;
   F32      elapsedTime;
   F32      fadeTime;
   bool     isFading;
   F32      percentFade;
   bool     startRender;
   F32      renderTime;

   F32      width;
   F32      chanceOfSplit;
   Point3F  startPoint;
   Point3F  endPoint;

   U32      numMajorNodes;
   F32      maxMajorAngle;
   U32      numMinorNodes;
   F32      maxMinorAngle;

   LightningBolt();
   ~LightningBolt();

   void  createSplit( const Point3F &startPoint, const Point3F &endPoint, U32 depth, F32 width );
   F32   findHeight( Point3F &point, SceneManager* sceneManager );
   void  render( const Point3F &camPos );
   void  renderSegment( NodeManager &segment, const Point3F &camPos, bool renderLastPoint );
   void  generate();
   void  generateMinorNodes();
   void  startSplits();
   void  update( F32 dt );

};


// -------------------------------------------------------------------------
class Lightning : public GameBase
{
   typedef GameBase Parent;

  protected:
   bool onAdd();
   void onRemove();
   bool onNewDataBlock( GameBaseData *dptr, bool reload );

   DECLARE_CALLBACK( void, applyDamage, ( const Point3F& hitPosition, const Point3F& hitNormal, SceneObject* hitObject ));

   struct Strike {
      F32     xVal;             // Position in cloud layer of strike
      F32     yVal;             //  top

      bool    targetedStrike;   // Is this a targeted strike?
      U32     targetGID;

      F32     deathAge;         // Age at which this strike expires
      F32     currentAge;       // Current age of this strike (updated by advanceTime)

      LightningBolt bolt[3];

      Strike* next;
   };
   struct Thunder {
      F32      tRemaining;
      Thunder* next;
   };

  public:

   //-------------------------------------- Console set variables
  public:

   U32      strikesPerMinute;
   F32      strikeWidth;
   F32      chanceToHitTarget;
   F32      strikeRadius;
   F32      boltStartRadius;
   ColorF   color;
   ColorF   fadeColor;
   bool     useFog;

   GFXStateBlockRef  mLightningSB;

  protected:

   // Rendering
   void prepRenderImage(SceneRenderState *state);
   void renderObject(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* );

   // Time management
   void processTick(const Move *move);
   void interpolateTick(F32 delta);
   void advanceTime(F32 dt);

   // Strike management
   void scheduleThunder(Strike*);

   // Data members
  private:
   LightningData* mDataBlock;

  protected:
   U32     mLastThink;        // Valid only on server

   Strike*  mStrikeListHead;   // Valid on on the client
   Thunder* mThunderListHead;

   static const U32 csmTargetMask;

  public:
   Lightning();
   ~Lightning();

   void warningFlashes();
   void strikeRandomPoint();
   void strikeObject(ShapeBase*);
   void processEvent(LightningStrikeEvent*);

   DECLARE_CONOBJECT(Lightning);
   static void initPersistFields();

   U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
   void unpackUpdate(NetConnection *conn,           BitStream *stream);
};

#endif // _H_LIGHTNING

