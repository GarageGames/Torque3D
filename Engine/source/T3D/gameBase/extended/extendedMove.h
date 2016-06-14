#ifndef _EXTENDEDMOVE_H_
#define _EXTENDEDMOVE_H_

#include "T3D/gameBase/moveManager.h"
#include "math/mQuat.h"

struct ExtendedMove : public Move
{
   typedef Move Parent;

   enum Constants {
      MaxPositionsRotations = 3,

      MaxPositionBits = 16,
      MaxRotationBits = 16,
   };

   // Position is in millimeters
   F32 posX[MaxPositionsRotations], posY[MaxPositionsRotations], posZ[MaxPositionsRotations];

   S32 cposX[MaxPositionsRotations], cposY[MaxPositionsRotations], cposZ[MaxPositionsRotations];

   bool EulerBasedRotation[MaxPositionsRotations];

   F32 rotX[MaxPositionsRotations], rotY[MaxPositionsRotations], rotZ[MaxPositionsRotations], rotW[MaxPositionsRotations];

   // Network clamped rotation
   S32 crotX[MaxPositionsRotations], crotY[MaxPositionsRotations], crotZ[MaxPositionsRotations], crotW[MaxPositionsRotations];

   ExtendedMove();

   virtual void pack(BitStream *stream, const Move * move = NULL);
   virtual void unpack(BitStream *stream, const Move * move = NULL);

   virtual void clamp();
   virtual void unclamp();
};

extern const ExtendedMove NullExtendedMove;

class ExtendedMoveManager
{
public:
   static F32 mPosX[ExtendedMove::MaxPositionsRotations];
   static F32 mPosY[ExtendedMove::MaxPositionsRotations];
   static F32 mPosZ[ExtendedMove::MaxPositionsRotations];
   static bool mRotIsEuler[ExtendedMove::MaxPositionsRotations];
   static F32 mRotAX[ExtendedMove::MaxPositionsRotations];
   static F32 mRotAY[ExtendedMove::MaxPositionsRotations];
   static F32 mRotAZ[ExtendedMove::MaxPositionsRotations];
   static F32 mRotAA[ExtendedMove::MaxPositionsRotations];

   static F32 mPosScale;

   static void init();
};

#endif   // _EXTENDEDMOVE_H_
