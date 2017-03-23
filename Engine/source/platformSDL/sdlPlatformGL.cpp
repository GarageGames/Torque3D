#include <SDL.h>
#include "windowManager/sdl/sdlWindow.h"
#include "console/console.h"

#ifdef TORQUE_OS_WIN
#include "gfx/gl/tGL/tWGL.h"
#endif

#include "gfx/gl/gfxGLUtils.h"

namespace PlatformGL
{

   void init()
   {
       const U32 majorOGL = 3;
       const U32 minorOGL = 2;
       U32 debugFlag = 0;
#ifdef TORQUE_DEBUG
       debugFlag |= SDL_GL_CONTEXT_DEBUG_FLAG;
#endif

       SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, majorOGL);
       SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minorOGL);
       SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
       SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, debugFlag);
#ifdef TORQUE_GL_SOFTWARE
       SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 0);
#endif

       SDL_ClearError();
   }

   void* CreateContextGL( PlatformWindow *window )
   {
       init();

       PlatformWindowSDL* windowSdl = dynamic_cast<PlatformWindowSDL*>(window);
       AssertFatal(windowSdl, "");

       if( !windowSdl )
           return NULL;

       SDL_ClearError();
       SDL_GLContext ctx = SDL_GL_CreateContext( windowSdl->getSDLWindow() );
       if( !ctx )
       {
           const char *err = SDL_GetError();
           Con::printf( err );
           AssertFatal(0, err );
       }

       return ctx;
   }

   void MakeCurrentGL( PlatformWindow *window, void *glContext )
   {
       PlatformWindowSDL* windowSdl = dynamic_cast<PlatformWindowSDL*>(window);
       AssertFatal( windowSdl && glContext, "" );

       SDL_ClearError();
       SDL_GL_MakeCurrent( windowSdl->getSDLWindow(), glContext );

       const char *err = SDL_GetError();
       if( err && err[0] )
       {
           Con::printf( err );
           AssertFatal(0, err );
       }
   }

   void setVSync(const int i)
   {
      PRESERVE_FRAMEBUFFER();
      // Nvidia needs to have the default framebuffer bound or the vsync calls fail
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
       if( i == 1 || i == -1 )
       {
           int ret = SDL_GL_SetSwapInterval(-1);

           if( ret == -1)
               SDL_GL_SetSwapInterval(1);
       }
       else
           SDL_GL_SetSwapInterval(0);

   }

}
