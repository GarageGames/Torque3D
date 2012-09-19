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

#include "gfx/D3D9/gfxD3D9Device.h"
#include "console/console.h"
#include "gfx/D3D9/gfxD3D9EnumTranslate.h"

// Cut and paste from console.log into GFXD3D9Device::initStates()
void GFXD3D9Device::regenStates() 
{
   DWORD temp;
   Con::printf( "   //-------------------------------------" );
   Con::printf( "   // Auto-generated default states, see regenStates() for details" );
   Con::printf( "   //" );
   Con::printf( "" );
   Con::printf( "   // Render states" );

   for( U32 state = GFXRenderState_FIRST; state < GFXRenderState_COUNT; state++ ) 
   {
      if( GFXD3D9RenderState[state] == GFX_UNSUPPORTED_VAL )
         continue;

      temp = 0;
      mD3DDevice->GetRenderState( GFXD3D9RenderState[state], &temp );
      Con::printf( "   mD3DDevice->SetRenderState( GFXD3D9RenderState[%d], %d );", state, temp );
   }

#ifndef TORQUE_OS_XENON 
   Con::printf( "" );
   Con::printf( "   // Texture Stage states" );

   for( U32 stage = 0; stage < TEXTURE_STAGE_COUNT; stage++ ) 
   {
      if( stage >= GFX->getNumSamplers() )
      {
         Con::errorf( "Sampler %d out of range for this device, ignoring.", stage );
         break;
      }

      for( U32 state = GFXTSS_FIRST; state < GFXTSS_COUNT; state++ ) 
      {
         if( GFXD3D9TextureStageState[state] == GFX_UNSUPPORTED_VAL )
            continue;

         temp = 0;
         mD3DDevice->GetTextureStageState( stage, GFXD3D9TextureStageState[state], &temp );
         Con::printf( "   mD3DDevice->SetTextureStageState( %d, GFXD3D9TextureStageState[%d], %d );", stage, state, temp );
      }
   }
#endif

   Con::printf( "" );
   Con::printf( "   // Sampler states" );
   for( U32 stage = 0; stage < TEXTURE_STAGE_COUNT; stage++ ) 
   {
      if( stage >= GFX->getNumSamplers() )
      {
         Con::errorf( "Sampler %d out of range for this device, ignoring.", stage );
         break;
      }

      for( U32 state = GFXSAMP_FIRST; state < GFXSAMP_COUNT; state++ ) 
      {
         if( GFXD3D9SamplerState[state] == GFX_UNSUPPORTED_VAL )
            continue;

         temp = 0;

         mD3DDevice->GetSamplerState( stage, GFXD3D9SamplerState[state], &temp );
         Con::printf( "   mD3DDevice->SetSamplerState( %d, GFXD3D9SamplerState[%d], %d );", stage, state, temp );
      }
   }
}
