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

#ifndef _H_PATHEDINTERIOR
#define _H_PATHEDINTERIOR

#ifndef _INTERIOR_H_
#include "interior/interior.h"
#endif
#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#ifndef _INTERIORRES_H_
#include "interior/interiorRes.h"
#endif
#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif

class InteriorInstance;
class EditGeometry;
class EditInteriorResource;
class SFXProfile;
class SFXSource;

struct PathedInteriorData : public GameBaseData {
   typedef GameBaseData Parent;
public:
   enum Sounds {
      StartSound,
      SustainSound,
      StopSound,
      MaxSounds
   };
   SFXProfile *sound[MaxSounds];
   static void initPersistFields();
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
   bool preload(bool server, String &errorStr);
   PathedInteriorData();

   DECLARE_CONOBJECT(PathedInteriorData);
};

class PathedInterior : public GameBase
{
   typedef GameBase Parent;
   friend class InteriorInstance;
   friend class EditGeometry;
   friend class EditInteriorResource;

   PathedInteriorData *mDataBlock;

public:
   enum UpdateMasks {
      NewTargetMask = Parent::NextFreeMask,
      NewPositionMask = Parent::NextFreeMask << 1,
      NextFreeMask = Parent::NextFreeMask << 2,
   };
private:

   U32 getPathKey();                            // only used on the server

   // Persist fields
  protected:
   StringTableEntry         mName;
   S32                      mPathIndex;
   Vector<StringTableEntry> mTriggers;
   Point3F                  mOffset;
   Box3F mExtrudedBox;
   bool mStopped;

   // Loaded resources and fields
  protected:
   static PathedInterior      *mClientPathedInteriors;

   SFXSource* mSustainSound;

   StringTableEntry           mInteriorResName;
   S32                        mInteriorResIndex;
   Resource<InteriorResource> mInteriorRes;
   Interior*                  mInterior;
   Vector<ColorI>             mVertexColorsNormal;
   Vector<ColorI>             mVertexColorsAlarm;

   MatrixF                    mBaseTransform;
   Point3F                    mBaseScale;

   U32                        mPathKey;         // only used on the client
   F64                        mCurrentPosition;
   S32                        mTargetPosition;
   Point3F                    mCurrentVelocity;

   PathedInterior *mNextClientPI;

   // Rendering
  protected:
   void prepRenderImage( SceneRenderState *state );
   void renderObject( SceneRenderState *state );
   void renderShadowVolumes( SceneRenderState *state );

  protected:
   bool onAdd();
   void onRemove();

  public:
   PathedInterior();
   ~PathedInterior();

   PathedInterior *getNext() { return mNextClientPI; }

   static PathedInterior *getClientPathedInteriors() { return mClientPathedInteriors; }

   void processTick(const Move* move);
   void setStopped() { mStopped = true; }
   void resolvePathKey();
   
   bool onNewDataBlock( GameBaseData *dptr, bool reload );
   bool  buildPolyList(AbstractPolyList* polyList, const Box3F &box, const SphereF &sphere);
   bool            readPI(Stream&);
   bool            writePI(Stream&) const;
   PathedInterior* clone() const;

   DECLARE_CONOBJECT(PathedInterior);
   static void initPersistFields();
   void setPathPosition(S32 newPosition);
   void setTargetPosition(S32 targetPosition);
   void computeNextPathStep(U32 timeDelta);
   Box3F getExtrudedBox() { return mExtrudedBox; }
   Point3F getVelocity();
   void advance(F64 timeDelta);

   U32  packUpdate(NetConnection *conn, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection *conn, BitStream* stream);
};

#endif // _H_PATHEDINTERIOR

