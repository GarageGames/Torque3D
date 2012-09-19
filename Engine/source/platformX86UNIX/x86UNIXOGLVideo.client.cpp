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

#include "console/console.h"
#include "platform/event.h"
#include "platform/gameInterface.h"

#include "platformX86UNIX/platformX86UNIX.h"
#include "platformX86UNIX/platformGL.h"
#include "platformX86UNIX/x86UNIXOGLVideo.h"
#include "platformX86UNIX/x86UNIXState.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#include <SDL/SDL_version.h>

//------------------------------------------------------------------------------
bool InitOpenGL()
{
   DisplayDevice::init();

   // Get the video settings from the prefs:
   const char* resString = Con::getVariable( "$pref::Video::resolution" );
   char* tempBuf = new char[dStrlen( resString ) + 1];
   dStrcpy( tempBuf, resString );
   char* temp = dStrtok( tempBuf, " x\0" );
   U32 width = ( temp ? dAtoi( temp ) : 800 );
   temp = dStrtok( NULL, " x\0" );
   U32 height = ( temp ? dAtoi( temp ) : 600 );
   temp = dStrtok( NULL, "\0" );
   U32 bpp = ( temp ? dAtoi( temp ) : 16 );
   delete [] tempBuf;

   bool fullScreen = Con::getBoolVariable( "$pref::Video::fullScreen" );

   // the only supported video device in unix is OpenGL
   if ( !Video::setDevice( "OpenGL", width, height, bpp, fullScreen ) )
   {
      Con::errorf("Unable to create default OpenGL mode: %d %d %d %d",
         width, height, bpp, fullScreen);

      // if we can't create the default, attempt to create a "safe" window
      if ( !Video::setDevice( "OpenGL", 640, 480, 16, true ) )
      {
         DisplayErrorAlert("Could not find a compatible OpenGL display " \
            "resolution.  Please check your driver configuration.");
         return false;
      }
   }

   return true;
}

//------------------------------------------------------------------------------
bool OpenGLDevice::smCanSwitchBitDepth = false;

//------------------------------------------------------------------------------
OpenGLDevice::OpenGLDevice()
{
   initDevice();
}

//------------------------------------------------------------------------------
OpenGLDevice::~OpenGLDevice()
{
}

//------------------------------------------------------------------------------
void OpenGLDevice::addResolution(S32 width, S32 height, bool check)
{
   Point2I desktopSize = x86UNIXState->getDesktopSize();
   U32 desktopBpp = x86UNIXState->getDesktopBpp();

   // don't allow any resolution under this size
   if (width < 640 || height < 480)
      return;

   if (check)
   {
      // don't allow resolutions that exceed the current desktop size
      if (width > desktopSize.x || height > desktopSize.y)
         return;
   }

   if (smCanSwitchBitDepth)
   {
      // add both 16 and 32 bit resolutions
      mResolutionList.push_back(Resolution(width, height, 16));
      mResolutionList.push_back(Resolution(width, height, 32));
   }
   else
   {
      // add just the desktop resolution
      mResolutionList.push_back(Resolution(width, height, desktopBpp));
   }
}

//------------------------------------------------------------------------------
void OpenGLDevice::initDevice()
{
   mDeviceName = "OpenGL";
   mFullScreenOnly = false;
}

//------------------------------------------------------------------------------
void OpenGLDevice::loadResolutions()
{
   mResolutionList.clear();

   // X cannot switch bit depths on the fly.  In case this feature is
   // implemented someday, calling this function will let you take
   // advantage of it
   if (Con::getBoolVariable("$pref::Unix::CanSwitchBitDepth"))
      smCanSwitchBitDepth = true;

   // add some default resolutions
   addResolution(640, 480);
   addResolution(800, 600);
   addResolution(1024, 768);
   addResolution(1152, 864);
   addResolution(1280, 1024);
   addResolution(1600, 1200);

   // specifying full screen should give us the resolutions that the
   // X server allows
   SDL_Rect** modes = SDL_ListModes(NULL, SDL_FULLSCREEN);
   if (modes &&
      (modes != (SDL_Rect **)-1))
   {
      for (int i = 0; modes[i] != NULL; ++i)
      {
         // do we already have this mode?
         bool found = false;
         for (Vector<Resolution>::iterator iter = mResolutionList.begin();
              iter != mResolutionList.end();
              ++iter)
         {
            if (iter->w == modes[i]->w && iter->h == modes[i]->h)
            {
               found = true;
               break;
            }
         }
         if (!found)
            // don't check these resolutions because they should be OK
            // (and checking might drop resolutions that are higher than the
            // current desktop bpp)
            addResolution(modes[i]->w, modes[i]->h, false);
      }
   }
}

//------------------------------------------------------------------------------
bool OpenGLDevice::activate( U32 width, U32 height, U32 bpp, bool fullScreen )
{
   if (!setScreenMode(width, height, bpp, fullScreen))
   {
      Con::printf("Unable to set screen mode.");
      return false;
   }

   // Output some driver info to the console
   const char* vendorString   = (const char*) glGetString( GL_VENDOR );
   const char* rendererString = (const char*) glGetString( GL_RENDERER );
   const char* versionString  = (const char*) glGetString( GL_VERSION );
   Con::printf( "OpenGL driver information:" );
   if ( vendorString )
      Con::printf( "  Vendor: %s", vendorString );
   if ( rendererString )
      Con::printf( "  Renderer: %s", rendererString );
   if ( versionString )
      Con::printf( "  Version: %s", versionString );

   GL_EXT_Init();

   Con::setVariable( "$pref::Video::displayDevice", mDeviceName );

   // Do this here because we now know about the extensions:
   if ( gGLState.suppSwapInterval )
      setVerticalSync(
         !Con::getBoolVariable( "$pref::Video::disableVerticalSync" ) );
   Con::setBoolVariable("$pref::OpenGL::allowTexGen", true);

   return true;
}


//------------------------------------------------------------------------------
void OpenGLDevice::shutdown()
{
   // Shutdown is deferred to Platform::shutdown()
}

//------------------------------------------------------------------------------
static void PrintGLAttributes()
{
   int doubleBuf;
   int bufferSize, depthSize, stencilSize;
   int red, green, blue, alpha;
   int aRed, aGreen, aBlue, aAlpha;

   SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doubleBuf);
   SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE, &bufferSize);
   SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depthSize);
   SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &stencilSize);
   SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &red);
   SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &green);
   SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &blue);
   SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &alpha);
   SDL_GL_GetAttribute(SDL_GL_ACCUM_RED_SIZE, &aRed);
   SDL_GL_GetAttribute(SDL_GL_ACCUM_GREEN_SIZE, &aGreen);
   SDL_GL_GetAttribute(SDL_GL_ACCUM_BLUE_SIZE, &aBlue);
   SDL_GL_GetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, &aAlpha);

   Con::printf("OpenGL Attributes:");
   Con::printf("  DoubleBuffer: %d", doubleBuf);
   Con::printf("  BufferSize: %d, DepthSize: %d, StencilSize: %d",
      bufferSize, depthSize, stencilSize);
   Con::printf("  Red: %d, Green: %d, Blue: %d, Alpha: %d",
      red, green, blue, alpha);
   Con::printf("  Accum Red: %d, Green: %d, Blue: %d, Alpha: %d",
      aRed, aGreen, aBlue, aAlpha);
}

//------------------------------------------------------------------------------
bool OpenGLDevice::setScreenMode( U32 width, U32 height, U32 bpp,
   bool fullScreen, bool forceIt, bool repaint )
{
   // load resolutions, this is done lazily so that we can check the setting
   // of smCanSwitchBitDepth, which may be overridden by console
   if (mResolutionList.size()==0)
      loadResolutions();

   if (mResolutionList.size()==0)
   {
      Con::printf("No resolutions available!");
      return false;
   }

   if (bpp == 0)
   {
      // bpp comes in as "0" when it is set to "Default"
      bpp = x86UNIXState->getDesktopBpp();
   }

   if (height == 0 || width == 0)
   {
      // paranoia check.  set it to the default to prevent crashing
      width = 800;
      height = 600;
   }

   U32 desktopDepth = x86UNIXState->getDesktopBpp();
   // if we can't switch bit depths and the requested bpp is not equal to
   // the desktop bpp, set bpp to the desktop bpp
   if (!smCanSwitchBitDepth &&
      bpp != desktopDepth)
   {
      bpp = desktopDepth;
   }

   bool IsInList = false;

   Resolution NewResolution( width, height, bpp );

   // See if the desired resolution is in the list
   if ( mResolutionList.size() )
   {
      for ( int i = 0; i < mResolutionList.size(); i++ )
      {
         if ( width == mResolutionList[i].w
              && height == mResolutionList[i].h
              && bpp == mResolutionList[i].bpp )
         {
            IsInList = true;
            break;
         }
      }

      if ( !IsInList )
      {
         Con::printf( "Selected resolution not available: %d %d %d",
            width, height, bpp);
         return false;
      }
   }
   else
   {
      AssertFatal( false, "No resolution list found!!" );
   }

   // Here if we found a matching resolution in the list

   bool needResurrect = false;
   if (x86UNIXState->windowCreated())
   {
      Con::printf( "Killing the texture manager..." );
      Game->textureKill();
      needResurrect = true;
   }

   // Set the desired GL Attributes
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
// JMQ: NVIDIA 2802+ doesn't like this setting for stencil size
//   SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
   SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
   SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
   SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
   SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
   SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
//    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
//    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
//    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
//    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);

   U32 flags = SDL_OPENGL;
   if (fullScreen)
      flags |= SDL_FULLSCREEN;

   Con::printf( "Setting screen mode to %dx%dx%d (%s)...", width, height,
      bpp, ( fullScreen ? "fs" : "w" ) );

   // set the new video mode
   if (SDL_SetVideoMode(width, height, bpp, flags) == NULL)
   {
      Con::printf("Unable to set SDL Video Mode: %s", SDL_GetError());
      return false;
   }

   PrintGLAttributes();

   // clear screen here to prevent buffer garbage from being displayed when
   // video mode is switched
   glClearColor(0.0, 0.0, 0.0, 0.0);
   glClear(GL_COLOR_BUFFER_BIT);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   if ( needResurrect )
   {
      // Reload the textures:
      Con::printf( "Resurrecting the texture manager..." );
      Game->textureResurrect();
   }

   if ( gGLState.suppSwapInterval )
      setVerticalSync( !Con::getBoolVariable( "$pref::Video::disableVerticalSync" ) );

   // reset the window in platform state
   SDL_SysWMinfo sysinfo;
   SDL_VERSION(&sysinfo.version);
   if (SDL_GetWMInfo(&sysinfo) == 0)
   {
      Con::printf("Unable to set SDL Video Mode: %s", SDL_GetError());
      return false;
   }
   x86UNIXState->setWindow(sysinfo.info.x11.window);

   // set various other parameters
   x86UNIXState->setWindowCreated(true);
   smCurrentRes = NewResolution;
   Platform::setWindowSize ( width, height );
   smIsFullScreen = fullScreen;
   Con::setBoolVariable( "$pref::Video::fullScreen", smIsFullScreen );
   char tempBuf[15];
   dSprintf( tempBuf, sizeof( tempBuf ), "%d %d %d",
      smCurrentRes.w, smCurrentRes.h, smCurrentRes.bpp );
   Con::setVariable( "$pref::Video::resolution", tempBuf );

   // post a TORQUE_SETVIDEOMODE user event
   SDL_Event event;
   event.type = SDL_USEREVENT;
   event.user.code = TORQUE_SETVIDEOMODE;
   event.user.data1 = NULL;
   event.user.data2 = NULL;
   SDL_PushEvent(&event);

   // reset the caption
   SDL_WM_SetCaption(x86UNIXState->getWindowName(), NULL);

   // repaint
   if ( repaint )
      Con::evaluate( "resetCanvas();" );

   return true;
}

//------------------------------------------------------------------------------
void OpenGLDevice::swapBuffers()
{
   SDL_GL_SwapBuffers();
}

//------------------------------------------------------------------------------
const char* OpenGLDevice::getDriverInfo()
{
   const char* vendorString   = (const char*) glGetString( GL_VENDOR );
   const char* rendererString = (const char*) glGetString( GL_RENDERER );
   const char* versionString  = (const char*) glGetString( GL_VERSION );
   const char* extensionsString = (const char*) glGetString( GL_EXTENSIONS );

   U32 bufferLen = ( vendorString ? dStrlen( vendorString ) : 0 )
                 + ( rendererString ? dStrlen( rendererString ) : 0 )
                 + ( versionString  ? dStrlen( versionString ) : 0 )
                 + ( extensionsString ? dStrlen( extensionsString ) : 0 )
                 + 4;

   char* returnString = Con::getReturnBuffer( bufferLen );
   dSprintf( returnString, bufferLen, "%s\t%s\t%s\t%s",
      ( vendorString ? vendorString : "" ),
      ( rendererString ? rendererString : "" ),
      ( versionString ? versionString : "" ),
      ( extensionsString ? extensionsString : "" ) );

   return( returnString );
}

//------------------------------------------------------------------------------
bool OpenGLDevice::getGammaCorrection(F32 &g)
{
   U16 redtable[256];
   U16 greentable[256];
   U16 bluetable[256];

   if (SDL_GetGammaRamp(redtable, greentable, bluetable) == -1)
   {
      Con::warnf("getGammaCorrection error: %s", SDL_GetError());
      return false;
   }

   F32 csum = 0.0;
   U32 ccount = 0;

   for (U16 i = 0; i < 256; ++i)
   {
      if (i != 0 && redtable[i] != 0 && redtable[i] != 65535)
      {
         F64 b = (F64) i/256.0;
         F64 a = (F64) redtable[i]/65535.0;
         F32 c = (F32) (mLog(a)/mLog(b));

         csum += c;
         ++ccount;
      }
   }
   g = csum/ccount;

   return true;
}

//------------------------------------------------------------------------------
bool OpenGLDevice::setGammaCorrection(F32 g)
{
   U16 redtable[256];
   U16 greentable[256];
   U16 bluetable[256];

   for (U16 i = 0; i < 256; ++i)
      redtable[i] = static_cast<U16>(mPow((F32) i/256.0f, g) * 65535.0f);
   dMemcpy(greentable,redtable,256*sizeof(U16));
   dMemcpy(bluetable,redtable,256*sizeof(U16));

   S32 ok = SDL_SetGammaRamp(redtable, greentable, bluetable);
   if (ok == -1)
      Con::warnf("Error setting gamma correction: %s", SDL_GetError());

   return ok != -1;
}

//------------------------------------------------------------------------------
bool OpenGLDevice::setVerticalSync( bool on )
{
   Con::printf("WARNING: OpenGLDevice::setVerticalSync is unimplemented %s %d\n", __FILE__, __LINE__);
   return false;
#if 0
   if ( !gGLState.suppSwapInterval )
      return( false );

   return( qwglSwapIntervalEXT( on ? 1 : 0 ) );
#endif
}

//------------------------------------------------------------------------------
DisplayDevice* OpenGLDevice::create()
{
   return new OpenGLDevice();
}
