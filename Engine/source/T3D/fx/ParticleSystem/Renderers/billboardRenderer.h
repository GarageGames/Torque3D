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

#ifndef _SPHERE_EMITTER_H
#define _SPHERE_EMITTER_H

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#include <T3D/fx/ParticleSystem/particle.h>
#include <T3D/fx/ParticleSystem/particleSystem.h>
#include <T3D/fx/ParticleSystem/particleEmitter.h>
#include <T3D/fx/ParticleSystem/particleRenderer.h>
#include "renderInstance/renderPassManager.h"
#include <T3D/fx/ParticleSystem/particleSystemInterfaces.h>

#define MaxParticleSize 50.0f

//-----------------------------------------------
//! A datablock for @ref BillboardRenderers.
//! @ingroup particlerenderers
//-----------------------------------------------
class BillboardRendererData : public ParticleRendererData
{
   typedef GameBaseData Parent;

public:
   BillboardRendererData();
   ~BillboardRendererData();

   virtual ParticleRenderer* CreateRenderer(ParticleSystem *system);

   // Script interface
   static void  initPersistFields();

   // Networking
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
   bool preload(bool server, String& errorStr);
   // Geometry
   void allocPrimBuffer(const ParticlePool *partPool);

   // Getters and setters
   bool getSortParticles() const { return mSortParticles; };
   bool getOrientParticles() const { return mOrientParticles; };
   bool getOrientOnVelocity() const { return mOrientOnVelocity; };
   bool getReverseOrder() const { return mReverseOrder; };
   bool getAlignParticles() const { return mAlignParticles; };
   Point3F getAlignDirection() const { return mAlignDirection; };

   bool getRenderReflection() const { return mRenderReflection; };
   F32 getAmbientFactor() const { return mAmbientFactor; };
   bool getHighResOnly() const { return mHighResOnly; };
   bool getGlow() const { return mGlow; };
   Point2F getTexCoords(U8 i) const { return mTexCoords[i]; };

   F32 getSoftnessDistance() const { return mSoftnessDistance; };
   GFXTexHandle getTextureHandle() const { return mTextureHandle; };

   GFXPrimitiveBufferHandle* getPrimBuff() { return &mPrimBuff; };

   ParticleRenderInst::BlendStyle getBlendStyle() const { return mBlendStyle; };

   bool getAnimateTexture() { return mAnimateTexture; };
   U32 getNumFrames() { return mNumFrames; };
   U32 getFramesPerSec() { return mFramesPerSec; };
   Point2F* getAnimTexUVs() { return mAnimTexUVs; };
   Point2I getAnimTexTiling() { return mAnimTexTiling; };
   StringTableEntry getAnimTexFramesString() { return mAnimTexFramesString; };
   Vector<U8> getAnimTexFrames() { return mAnimTexFrames; };

   F32 getTime(U8 i) const { return mTimes[i]; };
   F32 getSize(U8 i) const { return mSizes[i]; };
   void setSize(U8 i, F32 size) { mSizes[i] = size; };
   ColorF getColor(U8 i) { return mColors[i]; };
   void setColor(U8 i, ColorF color) { mColors[i] = color; };

   static bool protectedSetSizes(void *object, const char *index, const char *data);
   static bool protectedSetTimes(void *object, const char *index, const char *data);
   static bool _setAlignDirection(void *object, const char *index, const char *data);

private:
   // Variables

   // Sorting
   bool mSortParticles; //!< If true, particles are sorted furthest to nearest, otherwise newest to oldest.
   bool mOrientParticles; //!< If true, particles will always face the camera.
   bool mReverseOrder; //!< If true, particle sorting is reversed.
   bool mAlignParticles; //!< If true, particles will be aligned along the mAlignDirection.
   Point3F mAlignDirection; //!< The direction which particles are aligned to.
   bool mOrientOnVelocity; //!< If true, particles will face the direction they are moving.

   // Rendering
   bool mRenderReflection; //!< If true, particles will be rendered onto reflective surfaces.
   bool mGlow; //!< If true, particles will glow.

   /// Used to generate the final particle color by controlling interpolation
   ///  between the particle color and the particle color multiplied by the
   ///  ambient light color.
   F32 mAmbientFactor;
   bool mHighResOnly; //!< If false, particles will utilize mixed-resolution rendering.

   /// @brief 4 element array defining the UV coords into textureName to use for this particle.
   /// Coords should be set for the first tile only when using animTexTiling; 
   /// coordinates for other tiles will be calculated automatically. \"0 0\" is 
   /// top left and \"1 1\" is bottom right.
   Point2F mTexCoords[4];

   /// For soft particles, the distance (in meters) where particles will be
   /// faded based on the difference in depth between the particle and the
   /// scene geometry.
   F32 mSoftnessDistance;

   StringTableEntry  mTextureName; //!< Optional texture to override ParticleData::textureName.
   GFXTexHandle mTextureHandle;

   GFXPrimitiveBufferHandle mPrimBuff;

   ParticleRenderInst::BlendStyle mBlendStyle; //!< String value that controls how emitted particles blend with the scene.

   // Animation
   bool  mAnimateTexture; //!< If true, allow the particle texture to be an animated sprite.
   U32   mNumFrames; //!< The number of frames in the texture, auto-calculated.
   U32   mFramesPerSec; //!< If animateTexture is true, this defines the frames per second of the sprite animation.

   Point2F*          mAnimTexUVs; //!< The UV's calculated from mAnimTexTiling.

   /// @brief The number of frames, in rows and columns stored in textureName (when animateTexture is true).
   /// A maximum of 256 frames can be stored in a single texture when using
   /// animTexTiling. Value should be "NumColumns NumRows", for example "4 4".
   Point2I           mAnimTexTiling;

   /// @brief A list of frames and / or frame ranges to use for particle
   /// animation if animateTexture is true.
   /// Each frame token must be separated by whitespace. A frame token must be
   /// a positive integer frame number or a range of frame numbers separated
   /// with a '-'. The range separator, '-', cannot have any whitspace around
   /// it.
   ///
   /// Ranges can be specified to move through the frames in reverse as well
   /// as forward (eg. 19-14). Frame numbers exceeding the number of tiles will
   /// wrap.
   /// @tsexample
   /// animTexFrames = \"0-16 20 19 18 17 31-21\";
   /// @endtsexample
   StringTableEntry  mAnimTexFramesString;
   Vector<U8>        mAnimTexFrames;

   // Key values
   // TODO: Subtract this into a Frame class.
   
   /// @brief Particle size keyframe values.
   /// The particle size will linearly interpolate between the size/time keys
   /// over the lifetime of the particle.
   F32       mSizes[ParticleSystem::PDC_NUM_KEYS];

   /// @brief Time keys used with the colors and sizes keyframes.
   /// Values are from 0.0 (particle creation) to 1.0 (end of lifespace).
   F32       mTimes[ParticleSystem::PDC_NUM_KEYS];

   /// @brief Particle RGBA color keyframe values.
   /// The particle color will linearly interpolate between the color/time keys
   /// over the lifetime of the particle.
   ColorF    mColors[ParticleSystem::PDC_NUM_KEYS];

   DECLARE_CONOBJECT(BillboardRendererData);
};

//-----------------------------------------------
//! A datablock for @ref SphereEmitter's.
//! @ingroup particlerenderers
//-----------------------------------------------
class BillboardRenderer : public ParticleRenderer, public IColoredParticleRenderer
{
#if defined(TORQUE_OS_XENON)
   typedef GFXVertexPCTT ParticleVertexType;
   GFX360MemVertexBufferHandle<ParticleVertexType> mVertBuff;
#else
   typedef GFXVertexPCT ParticleVertexType;
   GFXVertexBufferHandle<ParticleVertexType> mVertBuff;
#endif

public:
   BillboardRenderer(ParticleSystem* parentSystem) : ParticleRenderer(parentSystem) {}

   virtual bool renderPool(ParticlePool *pool, SceneRenderState* state);
   void copyToVB(Point3F const& camPos, ColorF const& ambientColor);
   void setupBillboard(Particle* part, Point3F* basePts, MatrixF const& camView, ColorF const& ambientColor, ParticleVertexType* lVerts);
   void setupOriented(Particle* part, Point3F const& camPos, ColorF const& ambientColor, ParticleVertexType* lVerts);
   void setupAligned(Particle const* part, ColorF const& ambientColor, ParticleVertexType* lVerts);

   F32 getParticleSize(Particle const* part);
   virtual void setSizes(F32* sizeList);
   ColorF getParticleColor(Particle const* part);
   virtual void setColors(ColorF* colorList);

   virtual BillboardRendererData* getDataBlock() { return dynamic_cast<BillboardRendererData*>(mDataBlock); };

protected:
   // Variables

   S32 mCurBuffSize;
   MatrixF mBBObjToWorld;
};

#endif //_SPHERE_EMITTER_H