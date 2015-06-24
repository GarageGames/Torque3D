#ifndef PLATFORM_GL_H
#define PLATFORM_GL_H

class PlatformWindow;

namespace PlatformGL
{
   void init();

   void* CreateContextGL( PlatformWindow *window );

   void MakeCurrentGL( PlatformWindow *window, void *glContext );

   void setVSync(const int i);
}

#endif //PLATFORM_GL_H
