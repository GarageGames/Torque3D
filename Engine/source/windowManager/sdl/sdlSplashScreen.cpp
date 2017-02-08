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

#include "platform/platform.h"
#include "console/console.h"
#include "gfx/bitmap/gBitmap.h"
#include "SDL.h"
#include "windowManager/sdl/sdlWindow.h"

static SDL_Window* gSplashWindow = nullptr;
static SDL_Surface* gSplashImage = nullptr;
static SDL_Texture* gSplashTexture = nullptr;
static SDL_Renderer* gSplashRenderer = nullptr;

bool Platform::displaySplashWindow( String path )
{
   if(path.isEmpty())
      return false;

   Torque::Path iconPath = Torque::Path(path);

   if (iconPath.getExtension() == String("bmp"))
   {
      Con::errorf("Unable to use bmp format images for the splash screen. Please use a different format.");
      return false;
   }

   Resource<GBitmap> img = GBitmap::load(iconPath);
   if (img != NULL)
   {
      U32 pitch;
      U32 width = img->getWidth();
      bool hasAlpha = img->getHasTransparency();
      U32 depth;

      if (hasAlpha)
      {
         pitch = 4 * width;
         depth = 32;
      }
      else
      {
         pitch = 3 * width;
         depth = 24;
      }

      Uint32 rmask, gmask, bmask, amask;
      if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
      {
         S32 shift = hasAlpha ? 8 : 0;
         rmask = 0xff000000 >> shift;
         gmask = 0x00ff0000 >> shift;
         bmask = 0x0000ff00 >> shift;
         amask = 0x000000ff >> shift;
      }
      else
      {
         rmask = 0x000000ff;
         gmask = 0x0000ff00;
         bmask = 0x00ff0000;
         amask = hasAlpha ? 0xff000000 : 0;
      }

      gSplashImage = SDL_CreateRGBSurfaceFrom(img->getAddress(0, 0), img->getWidth(), img->getHeight(), depth, pitch, rmask, gmask, bmask, amask);
   }

   //now the pop-up window
   if (gSplashImage)
   {
      gSplashWindow = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
         gSplashImage->w, gSplashImage->h, SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN);

      gSplashRenderer = SDL_CreateRenderer(gSplashWindow, -1, SDL_RENDERER_ACCELERATED);

      gSplashTexture = SDL_CreateTextureFromSurface(gSplashRenderer, gSplashImage);

      SDL_RenderCopy(gSplashRenderer, gSplashTexture, NULL, NULL);

      SDL_RenderPresent(gSplashRenderer);
   }

	return true;
}

bool Platform::closeSplashWindow()
{
   SDL_DestroyTexture(gSplashTexture);
   SDL_FreeSurface(gSplashImage);
   SDL_DestroyRenderer(gSplashRenderer);
   SDL_DestroyWindow(gSplashWindow);

   return true;
}