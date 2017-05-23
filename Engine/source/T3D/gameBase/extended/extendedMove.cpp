#include "T3D/gameBase/extended/extendedMove.h"
#include "core/stream/bitStream.h"
#include "math/mathIO.h"
#include "math/mAngAxis.h"
#include "core/module.h"
#include "console/consoleTypes.h"
#include "core/strings/stringFunctions.h"

MODULE_BEGIN( ExtendedMoveManager )

   MODULE_INIT_AFTER( MoveManager )
   MODULE_INIT
   {
      ExtendedMoveManager::init();
   }

MODULE_END;

F32 ExtendedMoveManager::mPosX[ExtendedMove::MaxPositionsRotations] = { 0, };
F32 ExtendedMoveManager::mPosY[ExtendedMove::MaxPositionsRotations] = { 0, };
F32 ExtendedMoveManager::mPosZ[ExtendedMove::MaxPositionsRotations] = { 0, };
bool ExtendedMoveManager::mRotIsEuler[ExtendedMove::MaxPositionsRotations] = { 0, };
F32 ExtendedMoveManager::mRotAX[ExtendedMove::MaxPositionsRotations] = { 0, };
F32 ExtendedMoveManager::mRotAY[ExtendedMove::MaxPositionsRotations] = { 0, };
F32 ExtendedMoveManager::mRotAZ[ExtendedMove::MaxPositionsRotations] = { 0, };
F32 ExtendedMoveManager::mRotAA[ExtendedMove::MaxPositionsRotations] = { 1, };

F32 ExtendedMoveManager::mPosScale = 2.0f;

void ExtendedMoveManager::init()
{
   for(U32 i = 0; i < ExtendedMove::MaxPositionsRotations; ++i)
   {
      char varName[256];

      dSprintf(varName, sizeof(varName), "mvPosX%d", i);
      Con::addVariable(varName, TypeF32, &mPosX[i], 
         "X position of controller in millimeters.  Only 13 bits are networked.\n"
         "@ingroup Game");

      dSprintf(varName, sizeof(varName), "mvPosY%d", i);
      Con::addVariable(varName, TypeF32, &mPosY[i],
         "Y position of controller in millimeters.  Only 13 bits are networked.\n"
         "@ingroup Game");

      dSprintf(varName, sizeof(varName), "mvPosZ%d", i);
      Con::addVariable(varName, TypeF32, &mPosZ[i],
         "Z position of controller in millimeters.  Only 13 bits are networked.\n"
         "@ingroup Game");

      dSprintf(varName, sizeof(varName), "mvRotIsEuler%d", i);
      Con::addVariable(varName, TypeBool, &mRotIsEuler[i], 
         "@brief Indicates that the given rotation is Euler angles.\n\n"
         "When false (the default) the given rotation is a four component angled axis "
         "(a vector and angle).  When true, the given rotation is a three component "
         "Euler angle.  When using Euler angles, the $mvRotA component of the ExtendedMove "
         "is ignored for this set of rotations.\n"
         "@ingroup Game");

      dSprintf(varName, sizeof(varName), "mvRotX%d", i);
      Con::addVariable(varName, TypeF32, &mRotAX[i], 
         "X rotation vector component of controller.\n"
         "@ingroup Game");

      dSprintf(varName, sizeof(varName), "mvRotY%d", i);
      Con::addVariable(varName, TypeF32, &mRotAY[i], 
         "Y rotation vector component of controller.\n"
         "@ingroup Game");

      dSprintf(varName, sizeof(varName), "mvRotZ%d", i);
      Con::addVariable(varName, TypeF32, &mRotAZ[i], 
         "Z rotation vector component of controller.\n"
         "@ingroup Game");

      dSprintf(varName, sizeof(varName), "mvRotA%d", i);
      Con::addVariable(varName, TypeF32, &mRotAA[i], 
         "Angle rotation (in degrees) component of controller.\n"
         "@ingroup Game");
   }

   Con::addVariable("mvPosScale", TypeF32, &mPosScale,
      "@brief Indicates the scale to be given to mvPos values.\n\n"
      ""
      "@ingroup Game");
}

const ExtendedMove NullExtendedMove;

#define CLAMPPOS(x) ((S32)(((x + 1) * .5) * ((1 << MaxPositionBits) - 1)) & ((1<<MaxPositionBits)-1))
#define UNCLAMPPOS(x) ((F32)(x * 2 / F32((1 << MaxPositionBits) - 1) - 1.0f))
#define CLAMPROT(f) ((S32)(((f + 1) * .5) * ((1 << MaxRotationBits) - 1)) & ((1<<MaxRotationBits)-1))
#define UNCLAMPROT(x) ((F32)(x * 2 / F32((1 << MaxRotationBits) - 1) - 1.0f))

ExtendedMove::ExtendedMove() : Move()
{
   for(U32 i=0; i<MaxPositionsRotations; ++i)
   {
      posX[i] = 0;
      posY[i] = 0;
      posZ[i] = 0;
      rotX[i] = 0;
      rotY[i] = 0;
      rotZ[i] = 0;
      rotW[i] = 1;

      cposX[i] = 0;
      cposY[i] = 0;
      cposZ[i] = 0;

      EulerBasedRotation[i] = false;
   }
}

void ExtendedMove::pack(BitStream *stream, const Move * basemove)
{
   bool alwaysWriteAll = basemove!=NULL;
   if (!basemove)
      basemove = &NullExtendedMove;

   // Write the standard Move stuff
   packMove(stream, basemove, alwaysWriteAll);

   // Extended Move
   const ExtendedMove* extBaseMove = static_cast<const ExtendedMove*>(basemove);

   bool extendedDifferent = false;
   for(U32 i=0; i<MaxPositionsRotations; ++i)
   {
      bool check =   (posX[i] != extBaseMove->posX[i])   ||
                     (posY[i] != extBaseMove->posY[i])   ||
                     (posZ[i] != extBaseMove->posZ[i])   ||
                     (rotX[i] != extBaseMove->rotX[i])   ||
                     (rotY[i] != extBaseMove->rotY[i])   ||
                     (rotZ[i] != extBaseMove->rotZ[i]);
      if(!EulerBasedRotation[i])
      {
         check = check || (rotW[i] != extBaseMove->rotW[i]);
      }

      extendedDifferent = extendedDifferent || check;
   }

   if (alwaysWriteAll || stream->writeFlag(extendedDifferent))
   {
      for(U32 i=0; i<MaxPositionsRotations; ++i)
      {
         // Position
         if(stream->writeFlag(posX[i] != extBaseMove->posX[i]))
            stream->writeInt(cposX[i], MaxPositionBits);
         if(stream->writeFlag(posY[i] != extBaseMove->posY[i]))
            stream->writeInt(cposY[i], MaxPositionBits);
         if(stream->writeFlag(posZ[i] != extBaseMove->posZ[i]))
            stream->writeInt(cposZ[i], MaxPositionBits);

         // Rotation
         stream->writeFlag(EulerBasedRotation[i]);
         if(stream->writeFlag(rotX[i] != extBaseMove->rotX[i]))
            stream->writeInt(crotX[i], EulerBasedRotation[i] ? MaxRotationBits : MaxPositionBits);
         if(stream->writeFlag(rotY[i] != extBaseMove->rotY[i]))
            stream->writeInt(crotY[i], EulerBasedRotation[i] ? MaxRotationBits : MaxPositionBits);
         if(stream->writeFlag(rotZ[i] != extBaseMove->rotZ[i]))
            stream->writeInt(crotZ[i], EulerBasedRotation[i] ? MaxRotationBits : MaxPositionBits);
         if(!EulerBasedRotation[i])
         {
            if(stream->writeFlag(rotW[i] != extBaseMove->rotW[i]))
               stream->writeInt(crotW[i], MaxRotationBits);
         }
      }
   }
}

void ExtendedMove::unpack(BitStream *stream, const Move * basemove)
{
   bool alwaysReadAll = basemove!=NULL;
   if (!basemove)
      basemove=&NullExtendedMove;

   // Standard Move stuff
   bool isBaseMove = !unpackMove(stream, basemove, alwaysReadAll);

   // ExtendedMove
   const ExtendedMove* extBaseMove = static_cast<const ExtendedMove*>(basemove);

   if (alwaysReadAll || stream->readFlag())
   {
      isBaseMove = false;

      for(U32 i=0; i<MaxPositionsRotations; ++i)
      {
         // Position
         if (stream->readFlag())
         {
            cposX[i] = stream->readInt(MaxPositionBits);
            posX[i] = UNCLAMPPOS(cposX[i]) * ExtendedMoveManager::mPosScale;
         }
         else
            posX[i] = extBaseMove->posX[i];

         if (stream->readFlag())
         {
            cposY[i] = stream->readInt(MaxPositionBits);
            posY[i] = UNCLAMPPOS(cposY[i]) * ExtendedMoveManager::mPosScale;
         }
         else
            posY[i] = extBaseMove->posY[i];

         if (stream->readFlag())
         {
            cposZ[i] = stream->readInt(MaxPositionBits);
            posZ[i] = UNCLAMPPOS(cposZ[i]) * ExtendedMoveManager::mPosScale;
         }
         else
            posZ[i] = extBaseMove->posZ[i];

         // Rotation
         EulerBasedRotation[i] = stream->readFlag();
         F32 scale = 1.0f;
         if(EulerBasedRotation[i])
            scale = M_2PI_F;
         if(stream->readFlag())
         {
            crotX[i] = stream->readInt(EulerBasedRotation[i] ? MaxRotationBits : MaxPositionBits);
            rotX[i] = EulerBasedRotation[i] ? (UNCLAMPROT(crotX[i]) * scale) : UNCLAMPPOS(crotX[i]);
         }
         else
         {
            rotX[i] = extBaseMove->rotX[i];
         }

         if(stream->readFlag())
         {
            crotY[i] = stream->readInt(EulerBasedRotation[i] ? MaxRotationBits : MaxPositionBits);
            rotY[i] = EulerBasedRotation[i] ? (UNCLAMPROT(crotY[i]) * scale) : UNCLAMPPOS(crotY[i]);
         }
         else
         {
            rotY[i] = extBaseMove->rotY[i];
         }

         if(stream->readFlag())
         {
            crotZ[i] = stream->readInt(EulerBasedRotation[i] ? MaxRotationBits : MaxPositionBits);
            rotZ[i] = EulerBasedRotation[i] ? (UNCLAMPROT(crotZ[i]) * scale) : UNCLAMPPOS(crotZ[i]);
         }
         else
         {
            rotZ[i] = extBaseMove->rotZ[i];
         }

         if(!EulerBasedRotation[i])
         {
            if(stream->readFlag())
            {
               crotW[i] = stream->readInt(MaxRotationBits);
               rotW[i] = UNCLAMPROT(crotW[i]) * M_2PI_F;
            }
            else
            {
               rotW[i] = extBaseMove->rotW[i];
            }
         }
      }
   }

   if(isBaseMove)
   {
      *this = *extBaseMove;
   }
}

void ExtendedMove::clamp()
{
   // Clamp the values the same as for net traffic so the client matches the server
   for(U32 i=0; i<MaxPositionsRotations; ++i)
   {
      // Positions
      cposX[i] = CLAMPPOS(posX[i] / ExtendedMoveManager::mPosScale);
      cposY[i] = CLAMPPOS(posY[i] / ExtendedMoveManager::mPosScale);
      cposZ[i] = CLAMPPOS(posZ[i] / ExtendedMoveManager::mPosScale);

      // Rotations
      if(EulerBasedRotation[i])
      {
         crotX[i] = CLAMPROT(rotX[i] / M_2PI_F);
         crotY[i] = CLAMPROT(rotY[i] / M_2PI_F);
         crotZ[i] = CLAMPROT(rotZ[i] / M_2PI_F);
      }
      else
      {
         crotX[i] = CLAMPPOS(rotX[i]);
         crotY[i] = CLAMPPOS(rotY[i]);
         crotZ[i] = CLAMPPOS(rotZ[i]);
         crotW[i] = CLAMPROT(rotW[i] / M_2PI_F);
      }

      #ifdef DEBUG_CONTROLLER_MOVE
      if (i == 1)
      {
          F32 x, y, z, a;
          x = UNCLAMPPOS(crotX[i]);
          y = UNCLAMPPOS(crotY[i]);
          z = UNCLAMPPOS(crotZ[i]);
          a = UNCLAMPROT(crotW[i]) * M_2PI_F;

          Con::printf("INPUT POS == %f,%f,%f", ExtendedMoveManager::mPosX[i], ExtendedMoveManager::mPosY[i], ExtendedMoveManager::mPosZ[i]);
          Con::printf("rot %f,%f,%f,%f clamped to %f,%f,%f,%f", rotX[i], rotY[i], rotZ[i], rotW[i], x,y,z,a);
          x = UNCLAMPPOS(cposX[i]) * ExtendedMoveManager::mPosScale;
          y = UNCLAMPPOS(cposX[i]) * ExtendedMoveManager::mPosScale;
          z = UNCLAMPPOS(cposX[i]) * ExtendedMoveManager::mPosScale;
          Con::printf("pos %f,%f,%f clamped to %f,%f,%f", posX[i], posY[i], posZ[i], x, y, z);
      }
      #endif
   }

   // Perform the standard Move clamp
   Parent::clamp();
}

void ExtendedMove::unclamp()
{
   // Unclamp the values the same as for net traffic so the client matches the server
   for(U32 i=0; i<MaxPositionsRotations; ++i)
   {
      posX[i] = UNCLAMPPOS(cposX[i]) * ExtendedMoveManager::mPosScale;
      posY[i] = UNCLAMPPOS(cposY[i]) * ExtendedMoveManager::mPosScale;
      posZ[i] = UNCLAMPPOS(cposZ[i]) * ExtendedMoveManager::mPosScale;

      // Rotations
      if(EulerBasedRotation[i])
      {
         rotX[i] = UNCLAMPROT(crotX[i]) * M_2PI_F;
         rotY[i] = UNCLAMPROT(crotY[i]) * M_2PI_F;
         rotZ[i] = UNCLAMPROT(crotZ[i]) * M_2PI_F;
      }
      else
      {
         rotX[i] = UNCLAMPPOS(crotX[i]);
         rotY[i] = UNCLAMPPOS(crotY[i]);
         rotZ[i] = UNCLAMPPOS(crotZ[i]);
         rotW[i] = UNCLAMPROT(crotW[i]) * M_2PI_F;
      }
   }

   // Perform the standard Move unclamp
   Parent::unclamp();
}
