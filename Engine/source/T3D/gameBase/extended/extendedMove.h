#ifndef _EXTENDEDMOVE_H_
#define _EXTENDEDMOVE_H_

#include "T3D/gameBase/moveManager.h"
#include "math/mQuat.h"

struct ExtendedMove : public Move
{
   typedef Move Parent;

   enum Constants {
      MaxPositionsRotations = 2,

      MaxPositionBits = 13,
      MaxRotationBits = 11,
   };

   // Position is in millimeters
   S32 posX[MaxPositionsRotations], posY[MaxPositionsRotations], posZ[MaxPositionsRotations];

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
   static S32 mPosX[ExtendedMove::MaxPositionsRotations];
   static S32 mPosY[ExtendedMove::MaxPositionsRotations];
   static S32 mPosZ[ExtendedMove::MaxPositionsRotations];
   static F32 mRotAX[ExtendedMove::MaxPositionsRotations];
   static F32 mRotAY[ExtendedMove::MaxPositionsRotations];
   static F32 mRotAZ[ExtendedMove::MaxPositionsRotations];
   static F32 mRotAA[ExtendedMove::MaxPositionsRotations];

   static void init();
};

#endif   // _EXTENDEDMOVE_H_
