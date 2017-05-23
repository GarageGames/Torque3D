#ifndef SDL_INPUT_H
#define SDL_INPUT_H

#include "platform/types.h"

namespace KeyMapSDL
{
   U32 getTorqueScanCodeFromSDL(U32 sdl);

   U32 getSDLScanCodeFromTorque(U32 torque);
}

#endif
