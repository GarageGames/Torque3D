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

#ifndef _X86UNIXOGLVIDEO_H_
#define _X86UNIXOGLVIDEO_H_

#include "core/util/tVector.h"

struct Resolution {
  S32 w;
  S32 h;
  int bpp;
  Resolution(S32 width, S32 height, int b): w(width), h(height), bpp(b) {}
  Resolution(): w(0), h(0), bpp(-1) {}
};

class DisplayDevice {
public:
  virtual ~DisplayDevice() {}

  static void init() {}

protected:
  Vector<Resolution> mResolutionList;
  const char* mDeviceName;
  bool mFullScreenOnly;
};

struct GLState {
  int primMode; // unsure on this type.
  bool suppSwapInterval;
  bool suppPalettedTexture;
  bool suppLockedArrays;
  bool suppARBMultitexture;
  bool suppEXTblendcolor;
  bool suppEXTblendminmax;
  bool suppFogCoord;
  bool suppS3TC;
  bool suppTextureCompression;
  bool suppFXT1;
  bool suppPackedPixels;
  bool suppTextureEnvCombine;
  bool suppEdgeClamp;
  bool suppTexEnvAdd;
  bool suppTexAnisotropic;
  int maxTextureUnits; // unsure on this type
  float maxAnisotropy;
  bool suppVertexArrayRange;
};

namespace Video {
  bool setDevice(const char*, U32 width, U32 height, U16 bpp, bool fullscreen);
  bool isFullScreen();
  void toggleFullScreen();
  Resolution getDesktopResolution();
}

class OpenGLDevice : public DisplayDevice
{
      static bool smCanSwitchBitDepth;

      bool mRestoreGamma;
      U16  mOriginalRamp[256*3];

      void addResolution(S32 width, S32 height, bool check=true);

   public:
      OpenGLDevice();
      virtual ~OpenGLDevice();

      void initDevice();
      bool activate( U32 width, U32 height, U32 bpp, bool fullScreen );
      void shutdown();
      void destroy();
      bool setScreenMode( U32 width, U32 height, U32 bpp, bool fullScreen, bool forceIt = false, bool repaint = true );
      void swapBuffers();
      const char* getDriverInfo();
      bool getGammaCorrection(F32 &g);
      bool setGammaCorrection(F32 g);
      bool setVerticalSync( bool on );
      void loadResolutions();

      static DisplayDevice* create();
};

#endif // _H_X86UNIXOGLVIDEO
