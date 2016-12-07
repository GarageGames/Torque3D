#if defined(TORQUE_OPENGL) && !defined(TORQUE_SDL)

#include "platform/platformGL.h"
#include "gfx/gl/tGL/tWGL.h"

void PlatformGL::setVSync(const int i)
{
   if (gglHasWExtension(EXT_swap_control))
   {
      if (gglHasWExtension(EXT_swap_control_tear))
      {
         if (i == 1 || i == -1)
         {
            int ret = wglSwapIntervalEXT(-1);

            if (ret == -1)
               wglSwapIntervalEXT(1);
         }
         else
         {
            wglSwapIntervalEXT(i);
         }
         return;
      }
      //fallback with no EXT_swap_control_tear
      wglSwapIntervalEXT(i);
   }
}

#endif
