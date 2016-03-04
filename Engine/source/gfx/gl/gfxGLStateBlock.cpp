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

#include "gfx/gl/gfxGLStateBlock.h"
#include "gfx/gl/gfxGLDevice.h"
#include "gfx/gl/gfxGLEnumTranslate.h"
#include "gfx/gl/gfxGLUtils.h"
#include "gfx/gl/gfxGLTextureObject.h"
#include "core/crc.h"

namespace DictHash
{
   inline U32 hash(const GFXSamplerStateDesc &data)
   {
      return CRC::calculateCRC(&data, sizeof(GFXSamplerStateDesc));;
   }
}

GFXGLStateBlock::GFXGLStateBlock(const GFXStateBlockDesc& desc) :
   mDesc(desc),
   mCachedHashValue(desc.getHashValue())
{
    if( !gglHasExtension(ARB_sampler_objects) )
	   return;

   static Map<GFXSamplerStateDesc, U32> mSamplersMap;

	for(int i = 0; i < TEXTURE_STAGE_COUNT; ++i)
	{
		GLuint &id = mSamplerObjects[i];
		GFXSamplerStateDesc &ssd = mDesc.samplers[i];
      Map<GFXSamplerStateDesc, U32>::Iterator itr =  mSamplersMap.find(ssd);
      if(itr == mSamplersMap.end())
      {
		   glGenSamplers(1, &id);

		   glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, 1) );
		   glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GFXGLTextureFilter[ssd.magFilter]);
		   glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GFXGLTextureAddress[ssd.addressModeU]);
		   glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GFXGLTextureAddress[ssd.addressModeV]);
		   glSamplerParameteri(id, GL_TEXTURE_WRAP_R, GFXGLTextureAddress[ssd.addressModeW]);
		   if(static_cast< GFXGLDevice* >( GFX )->supportsAnisotropic() )
			   glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, ssd.maxAnisotropy);

         mSamplersMap[ssd] = id;
      }
      else
         id = itr->value;
	}
}

GFXGLStateBlock::~GFXGLStateBlock()
{
}

/// Returns the hash value of the desc that created this block
U32 GFXGLStateBlock::getHashValue() const
{
   return mCachedHashValue;
}

/// Returns a GFXStateBlockDesc that this block represents
const GFXStateBlockDesc& GFXGLStateBlock::getDesc() const
{
   return mDesc;   
}

/// Called by OpenGL device to active this state block.
/// @param oldState  The current state, used to make sure we don't set redundant states on the device.  Pass NULL to reset all states.
void GFXGLStateBlock::activate(const GFXGLStateBlock* oldState)
{
   // Big scary warning copied from Apple docs 
   // http://developer.apple.com/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_performance/chapter_13_section_2.html#//apple_ref/doc/uid/TP40001987-CH213-SW12
   // Don't set a state that's already set. Once a feature is enabled, it does not need to be enabled again.
   // Calling an enable function more than once does nothing except waste time because OpenGL does not check 
   // the state of a feature when you call glEnable or glDisable. For instance, if you call glEnable(GL_LIGHTING) 
   // more than once, OpenGL does not check to see if the lighting state is already enabled. It simply updates 
   // the state value even if that value is identical to the current value.

#define STATE_CHANGE(state) (!oldState || oldState->mDesc.state != mDesc.state)
#define TOGGLE_STATE(state, enum) if(mDesc.state) glEnable(enum); else glDisable(enum)
#define CHECK_TOGGLE_STATE(state, enum) if(!oldState || oldState->mDesc.state != mDesc.state) if(mDesc.state) glEnable(enum); else glDisable(enum)

   // Blending
   CHECK_TOGGLE_STATE(blendEnable, GL_BLEND);
   if(STATE_CHANGE(blendSrc) || STATE_CHANGE(blendDest))
      glBlendFunc(GFXGLBlend[mDesc.blendSrc], GFXGLBlend[mDesc.blendDest]);
   if(STATE_CHANGE(blendOp))
      glBlendEquation(GFXGLBlendOp[mDesc.blendOp]);

   if (mDesc.separateAlphaBlendEnable == true)
   {
       if (STATE_CHANGE(separateAlphaBlendSrc) || STATE_CHANGE(separateAlphaBlendDest))
           glBlendFuncSeparate(GFXGLBlend[mDesc.blendSrc], GFXGLBlend[mDesc.blendDest], GFXGLBlend[mDesc.separateAlphaBlendSrc], GFXGLBlend[mDesc.separateAlphaBlendDest]);
       if (STATE_CHANGE(separateAlphaBlendOp))
           glBlendEquationSeparate(GFXGLBlendOp[mDesc.blendOp], GFXGLBlendOp[mDesc.separateAlphaBlendOp]);
   }

   // Color write masks
   if(STATE_CHANGE(colorWriteRed) || STATE_CHANGE(colorWriteBlue) || STATE_CHANGE(colorWriteGreen) || STATE_CHANGE(colorWriteAlpha))
      glColorMask(mDesc.colorWriteRed, mDesc.colorWriteBlue, mDesc.colorWriteGreen, mDesc.colorWriteAlpha);
   
   // Culling
   if(STATE_CHANGE(cullMode))
   {
      TOGGLE_STATE(cullMode, GL_CULL_FACE);
      glCullFace(GFXGLCullMode[mDesc.cullMode]);
   }

   // Depth
   CHECK_TOGGLE_STATE(zEnable, GL_DEPTH_TEST);
   
   if(STATE_CHANGE(zFunc))
      glDepthFunc(GFXGLCmpFunc[mDesc.zFunc]);
   
   if(STATE_CHANGE(zBias))
   {
      if (mDesc.zBias == 0)
      {
         glDisable(GL_POLYGON_OFFSET_FILL);
      } else {
         F32 bias = mDesc.zBias * 10000.0f;
         glEnable(GL_POLYGON_OFFSET_FILL);
         glPolygonOffset(bias, bias);
      } 
   }
   
   if(STATE_CHANGE(zWriteEnable))
      glDepthMask(mDesc.zWriteEnable);

   // Stencil
   CHECK_TOGGLE_STATE(stencilEnable, GL_STENCIL_TEST);
   if(STATE_CHANGE(stencilFunc) || STATE_CHANGE(stencilRef) || STATE_CHANGE(stencilMask))
      glStencilFunc(GFXGLCmpFunc[mDesc.stencilFunc], mDesc.stencilRef, mDesc.stencilMask);
   if(STATE_CHANGE(stencilFailOp) || STATE_CHANGE(stencilZFailOp) || STATE_CHANGE(stencilPassOp))
      glStencilOp(GFXGLStencilOp[mDesc.stencilFailOp], GFXGLStencilOp[mDesc.stencilZFailOp], GFXGLStencilOp[mDesc.stencilPassOp]);
   if(STATE_CHANGE(stencilWriteMask))
      glStencilMask(mDesc.stencilWriteMask);
   

   if(STATE_CHANGE(fillMode))
      glPolygonMode(GL_FRONT_AND_BACK, GFXGLFillMode[mDesc.fillMode]);

#undef CHECK_STATE
#undef TOGGLE_STATE
#undef CHECK_TOGGLE_STATE

   //sampler objects
   if( gglHasExtension(ARB_sampler_objects) )
   {
      for (U32 i = 0; i < getMin(getOwningDevice()->getNumSamplers(), (U32) TEXTURE_STAGE_COUNT); i++)
      {
         if(!oldState || oldState->mSamplerObjects[i] != mSamplerObjects[i])
		      glBindSampler(i, mSamplerObjects[i] );
      }
   }	  

   // TODO: states added for detail blend   
}
