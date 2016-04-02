//-----------------------------------------------------------------------------
// Copyright (c) 2016 GarageGames, LLC
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
#include "gfx/D3D11/screenshotD3D11.h"
#include "gfx/D3D11/gfxD3D11Device.h"

//Note if MSAA is ever enabled this will need fixing
GBitmap* ScreenShotD3D11::_captureBackBuffer()
{
   ID3D11Texture2D* backBuf = D3D11->getBackBufferTexture();
   D3D11_TEXTURE2D_DESC desc;
   backBuf->GetDesc(&desc);
   desc.BindFlags = 0;
   desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
   desc.Usage = D3D11_USAGE_STAGING;

   //create temp texure
   ID3D11Texture2D* pNewTexture = NULL;
   HRESULT hr = D3D11DEVICE->CreateTexture2D(&desc, NULL, &pNewTexture);
   if (FAILED(hr))
      return NULL;

   U32 width = desc.Width;
   U32 height = desc.Height;
   // pixel data
   U8 *pData = new U8[width * height * 4];

   D3D11DEVICECONTEXT->CopyResource(pNewTexture, backBuf);
   D3D11_MAPPED_SUBRESOURCE Resource;

   hr = D3D11DEVICECONTEXT->Map(pNewTexture, 0, D3D11_MAP_READ, 0, &Resource);
   if (FAILED(hr))
   {
      //cleanup
      SAFE_DELETE_ARRAY(pData);
      SAFE_RELEASE(pNewTexture);
      return NULL;
   }

   const U32 pitch = width << 2;
   const U8* pSource = (U8*)Resource.pData;
   U32 totalPitch = 0;
   for (U32 i = 0; i < height; ++i)
   {
      dMemcpy(pData, pSource, width * 4);
      pSource += Resource.RowPitch;
      pData += pitch;
      totalPitch += pitch;
   }

   D3D11DEVICECONTEXT->Unmap(pNewTexture, 0);  
   pData -= totalPitch;
   GBitmap *gb = new GBitmap(width, height);

   //Set GBitmap data and convert from bgr to rgb
   ColorI c;
   for (S32 i = 0; i<height; i++)
   {
      const U8 *a = pData + i * width * 4;
      for (S32 j = 0; j<width; j++)
      {
         c.blue = *(a++);
         c.green = *(a++);
         c.red = *(a++);
         a++; // Ignore alpha.
         gb->setColor(j, i, c);
      }
   }

   //cleanup
   SAFE_DELETE_ARRAY(pData);
   SAFE_RELEASE(pNewTexture);
   

   return gb;

}

