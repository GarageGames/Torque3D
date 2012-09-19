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

#ifndef _GFX_PC_D3D9DEVICE_H_
#define _GFX_PC_D3D9DEVICE_H_

#include "gfx/D3D9/gfxD3D9Device.h"

class PlatformWindow;
class VideoFrameGrabberD3D9;


class GFXPCD3D9Device : public GFXD3D9Device
{
   typedef GFXD3D9Device Parent;

public:
   // Set to true to force nvperfhud device creation
   static bool mEnableNVPerfHUD;

   GFXPCD3D9Device( LPDIRECT3D9 d3d, U32 index )
      : GFXD3D9Device( d3d, index ),
        mVideoFrameGrabber( NULL ) {};
   ~GFXPCD3D9Device();

   static GFXDevice *createInstance( U32 adapterIndex );

   virtual GFXFormat selectSupportedFormat(GFXTextureProfile *profile,
	   const Vector<GFXFormat> &formats, bool texture, bool mustblend, bool mustfilter);
   
   static void enumerateAdapters( Vector<GFXAdapter*> &adapterList );

   virtual void enumerateVideoModes();

   virtual GFXWindowTarget *allocWindowTarget(PlatformWindow *window);
   virtual GFXTextureTarget *allocRenderToTextureTarget();
   virtual bool beginSceneInternal();

   virtual void init( const GFXVideoMode &mode, PlatformWindow *window = NULL );

   virtual void enterDebugEvent(ColorI color, const char *name);
   virtual void leaveDebugEvent();
   virtual void setDebugMarker(ColorI color, const char *name);

   virtual void setMatrix( GFXMatrixType mtype, const MatrixF &mat );

   virtual void initStates();
   virtual void reset( D3DPRESENT_PARAMETERS &d3dpp );
   virtual D3DPRESENT_PARAMETERS setupPresentParams( const GFXVideoMode &mode, const HWND &hwnd ) const;
protected:

   VideoFrameGrabberD3D9* mVideoFrameGrabber;

   static GFXAdapter::CreateDeviceInstanceDelegate mCreateDeviceInstance;
   static void createDirect3D9(LPDIRECT3D9 &d3d9, LPDIRECT3D9EX &d3d9ex);
   HRESULT createDevice(U32 adapter, D3DDEVTYPE deviceType, HWND hFocusWindow, DWORD behaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters);

   virtual void _setTextureStageState( U32 stage, U32 state, U32 value );      
   void _validateMultisampleParams(D3DFORMAT format, D3DMULTISAMPLE_TYPE & aatype, DWORD & aalevel) const;
};

#endif