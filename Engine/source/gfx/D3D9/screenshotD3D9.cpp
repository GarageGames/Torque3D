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
#include "gfx/D3D9/screenshotD3D9.h"

#include "gfx/D3D9/gfxD3D9Device.h"

#include <d3d9.h>
#include <d3dx9core.h>
#include <d3dx9tex.h>


GBitmap* ScreenShotD3D9::_captureBackBuffer()
{
#ifdef TORQUE_OS_XENON
   return NULL;
#else
   LPDIRECT3DDEVICE9 D3DDevice = dynamic_cast<GFXD3D9Device *>(GFX)->getDevice();

   IDirect3DSurface9 * backBuffer;
   D3DDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer );

   // Figure the size we're snagging.
   D3DSURFACE_DESC desc;
   backBuffer->GetDesc(&desc);

   Point2I size;
   size.x = desc.Width;
   size.y = desc.Height;

   // set up the 2 copy surfaces
   GFXTexHandle tex[2];
   IDirect3DSurface9 *surface[2];

   tex[0].set( size.x, size.y, GFXFormatR8G8B8X8, &GFXDefaultRenderTargetProfile, avar("%s() - tex[0] (line %d)", __FUNCTION__, __LINE__) );
   tex[1].set( size.x, size.y, GFXFormatR8G8B8X8, &GFXSystemMemProfile, avar("%s() - tex[1] (line %d)", __FUNCTION__, __LINE__) );

   // grab the top level surface of tex 0
   GFXD3D9TextureObject *to = (GFXD3D9TextureObject *) &(*tex[0]);
   D3D9Assert( to->get2DTex()->GetSurfaceLevel( 0, &surface[0] ), NULL );

   // use StretchRect because it allows a copy from a multisample surface
   // to a normal rendertarget surface
   D3DDevice->StretchRect( backBuffer, NULL, surface[0], NULL, D3DTEXF_NONE );

   // grab the top level surface of tex 1
   to = (GFXD3D9TextureObject *) &(*tex[1]);
   D3D9Assert( to->get2DTex()->GetSurfaceLevel( 0, &surface[1] ), NULL );

   // copy the data from the render target to the system memory texture
   D3DDevice->GetRenderTargetData( surface[0], surface[1] );

   // Allocate a GBitmap and copy into it.
   GBitmap *gb = new GBitmap(size.x, size.y);

   D3DLOCKED_RECT r;
   D3DSURFACE_DESC d;
   surface[1]->GetDesc(&d);
   surface[1]->LockRect( &r, NULL, D3DLOCK_READONLY);

   // We've got the X8 in there so we have to manually copy stuff.
   ColorI c;
   for(S32 i=0; i<size.y; i++)
   {
      const U8 *a = ((U8*)r.pBits) + i * size.x * 4;
      for(S32 j=0; j<size.x; j++)
      {
         c.blue  = *(a++);
         c.green = *(a++);
         c.red   = *(a++);
         a++; // Ignore X.

         gb->setColor(j, i, c);
      }
   }

   surface[1]->UnlockRect();

   //  Also save it out with D3DX
   //D3DXSaveSurfaceToFile( dT( "testScreen.png" ), D3DXIFF_PNG, surface[1], NULL, NULL );

   // release the COM pointers
   surface[0]->Release();
   surface[1]->Release();
   backBuffer->Release();

   return gb;
#endif
}

