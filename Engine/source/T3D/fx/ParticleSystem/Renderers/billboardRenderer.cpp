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

#include "billboardRenderer.h"
#include "scene/sceneRenderState.h"
#include "platform/profiler.h"
#include "renderInstance/renderPassManager.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include <math/mathIO.h>

IMPLEMENT_CO_DATABLOCK_V1(BillboardRendererData);

BillboardRendererData::BillboardRendererData()
{
   // Sorting
   mSortParticles = false;
   mOrientParticles = false;
   mReverseOrder = false;
   mAlignParticles = false;
   mAlignDirection = Point3F(0.0f, 1.0f, 0.0f);
   mOrientOnVelocity = true;

   // Rendering
   mRenderReflection = true;
   mAmbientFactor = 0.0f;
   mHighResOnly = true;
   mGlow = false;

   mTexCoords[0].set(0.0, 0.0);   // texture coords at 4 corners
   mTexCoords[1].set(0.0, 1.0);   // of particle quad
   mTexCoords[2].set(1.0, 1.0);   // (defaults to entire particle)
   mTexCoords[3].set(1.0, 0.0);

   mSoftnessDistance = 1.0f;

   mTextureName = NULL;
   mTextureHandle = NULL;

   mBlendStyle = ParticleRenderInst::BlendUndefined;

   // Animation
   mAnimateTexture = false;
   mNumFrames = 1;
   mFramesPerSec = mNumFrames;

   mAnimTexTiling.set(0, 0);      // tiling dimensions 
   mAnimTexFramesString = NULL;  // string of animation frame indices
   mAnimTexUVs = NULL;           // array of tile vertex UVs

   // Key values
   S32 i;
   for (i = 0; i < ParticleSystem::PDC_NUM_KEYS; i++)
   {
      mTimes[i] = F32(i) / F32(ParticleSystem::PDC_NUM_KEYS - 1);
      mColors[i].set(1.0, 1.0, 1.0, 1.0);
      mSizes[i] = 1.0;
   }
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
BillboardRendererData::~BillboardRendererData()
{
   if (mAnimTexUVs)
   {
      delete[] mAnimTexUVs;
   }
}

ParticleRenderer* BillboardRendererData::CreateRenderer(ParticleSystem* system)
{
   BillboardRenderer *renderer = new BillboardRenderer(system);
   renderer->setDataBlock(this);
   return renderer;
}

bool BillboardRendererData::protectedSetSizes(void *object, const char *index, const char *data)
{
   BillboardRendererData *pData = static_cast<BillboardRendererData*>(object);
   F32 val = dAtof(data);
   U32 i;

   if (!index)
      i = 0;
   else
      i = dAtoui(index);

   pData->mSizes[i] = mClampF(val, 0.f, MaxParticleSize);

   return false;
}

bool BillboardRendererData::protectedSetTimes(void *object, const char *index, const char *data)
{
   BillboardRendererData *pData = static_cast<BillboardRendererData*>(object);
   F32 val = dAtof(data);
   U32 i;

   if (!index)
      i = 0;
   else
      i = dAtoui(index);

   pData->mTimes[i] = mClampF(val, 0.f, 1.f);

   return false;
}

bool BillboardRendererData::_setAlignDirection(void *object, const char *index, const char *data)
{
   BillboardRendererData *p = static_cast<BillboardRendererData*>(object);

   Con::setData(TypePoint3F, &p->mAlignDirection, 0, 1, &data);
   p->mAlignDirection.normalizeSafe();

   // we already set the field
   return false;
}

// Enum tables used for fields blendStyle, srcBlendFactor, dstBlendFactor.
// Note that the enums for srcBlendFactor and dstBlendFactor are consistent
// with the blending enums used in Torque Game Builder.

typedef ParticleRenderInst::BlendStyle bbr_ParticleBlendStyle;
DefineEnumType(bbr_ParticleBlendStyle);

ImplementEnumType(bbr_ParticleBlendStyle,
   "The type of visual blending style to apply to the particles.\n"
   "@ingroup FX\n\n")
{
   ParticleRenderInst::BlendNormal, "NORMAL", "No blending style.\n"
},
{ ParticleRenderInst::BlendAdditive, "ADDITIVE", "Adds the color of the pixel to the frame buffer with full alpha for each pixel.\n" },
{ ParticleRenderInst::BlendSubtractive, "SUBTRACTIVE", "Subtractive Blending. Reverses the color model, causing dark colors to have a stronger visual effect.\n" },
{ ParticleRenderInst::BlendPremultAlpha, "PREMULTALPHA", "Color blends with the colors of the imagemap rather than the alpha.\n" },
EndImplementEnumType;

   void BillboardRendererData::initPersistFields()
   {
      addField("sortParticles", TYPEID< bool >(), Offset(mSortParticles, BillboardRendererData),
         "If true, particles are sorted furthest to nearest.");

      addField("orientParticles", TYPEID< bool >(), Offset(mOrientParticles, BillboardRendererData),
         "If true, Particles will always face the camera.");

      addField("reverseOrder", TYPEID< bool >(), Offset(mReverseOrder, BillboardRendererData),
         "@brief If true, reverses the normal draw order of particles.\n\n"
         "Particles are normally drawn from newest to oldest, or in Z order "
         "(furthest first) if sortParticles is true. Setting this field to "
         "true will reverse that order: oldest first, or nearest first if "
         "sortParticles is true.");

      addField("alignParticles", TYPEID< bool >(), Offset(mAlignParticles, BillboardRendererData),
         "If true, particles always face along the axis defined by alignDirection.");

      addProtectedField("alignDirection", TYPEID< Point3F>(), Offset(mAlignDirection, BillboardRendererData), &BillboardRendererData::_setAlignDirection, &defaultProtectedGetFn,
         "The direction aligned particles should face, only valid if alignParticles is true.");

      addField("orientOnVelocity", TYPEID< bool >(), Offset(mOrientOnVelocity, BillboardRendererData),
         "If true, particles will be oriented to face in the direction they are moving.");

      addField("renderReflection", TYPEID< bool >(), Offset(mRenderReflection, BillboardRendererData),
         "Controls whether particles are rendered onto reflective surfaces like water.");

      addField("ambientFactor", TYPEID< F32 >(), Offset(mAmbientFactor, BillboardRendererData),
         "Used to generate the final particle color by controlling interpolation "
         "between the particle color and the particle color multiplied by the "
         "ambient light color.");

      addField("highResOnly", TYPEID< bool >(), Offset(mHighResOnly, BillboardRendererData),
         "This particle system should not use the mixed-resolution renderer. "
         "If your particle system has large amounts of overdraw, consider "
         "disabling this option.");

      addField("glow", TYPEID< bool >(), Offset(mGlow, BillboardRendererData),
         "If true, the particles are rendered to the glow buffer as well.");

      addField("animateTexture", TYPEID< bool >(), Offset(mAnimateTexture, BillboardRendererData),
         "If true, allow the particle texture to be an animated sprite.");
      addField("framesPerSec", TYPEID< S32 >(), Offset(mFramesPerSec, BillboardRendererData),
         "If animateTexture is true, this defines the frames per second of the "
         "sprite animation.");

      addField("animTexTiling", TYPEID< Point2I >(), Offset(mAnimTexTiling, BillboardRendererData),
         "@brief The number of frames, in rows and columns stored in textureName "
         "(when animateTexture is true).\n\n"
         "A maximum of 256 frames can be stored in a single texture when using "
         "animTexTiling. Value should be \"NumColumns NumRows\", for example \"4 4\".");
      addField("animTexFrames", TYPEID< StringTableEntry >(), Offset(mAnimTexFramesString, BillboardRendererData),
         "@brief A list of frames and/or frame ranges to use for particle "
         "animation if animateTexture is true.\n\n"
         "Each frame token must be separated by whitespace. A frame token must be "
         "a positive integer frame number or a range of frame numbers separated "
         "with a '-'. The range separator, '-', cannot have any whitspace around "
         "it.\n\n"
         "Ranges can be specified to move through the frames in reverse as well "
         "as forward (eg. 19-14). Frame numbers exceeding the number of tiles will "
         "wrap.\n"
         "@tsexample\n"
         "animTexFrames = \"0-16 20 19 18 17 31-21\";\n"
         "@endtsexample\n");



      addField("textureCoords", TYPEID< Point2F >(), Offset(mTexCoords, BillboardRendererData), 4,
         "@brief 4 element array defining the UV coords into textureName to use "
         "for this particle.\n\n"
         "Coords should be set for the first tile only when using animTexTiling; "
         "coordinates for other tiles will be calculated automatically. \"0 0\" is "
         "top left and \"1 1\" is bottom right.");

      addField("softnessDistance", TYPEID< F32 >(), Offset(mSoftnessDistance, BillboardRendererData),
         "For soft particles, the distance (in meters) where particles will be "
         "faded based on the difference in depth between the particle and the "
         "scene geometry.");

      addField("textureName", TYPEID< StringTableEntry >(), Offset(mTextureName, BillboardRendererData),
         "Optional texture to override ParticleData::textureName.");

      addField("blendStyle", TYPEID< ParticleRenderInst::BlendStyle >(), Offset(mBlendStyle, BillboardRendererData),
         "String value that controls how emitted particles blend with the scene.");

      addField("colors", TYPEID< ColorF >(), Offset(mColors, BillboardRendererData), ParticleSystem::PDC_NUM_KEYS,
         "@brief Particle RGBA color keyframe values.\n\n"
         "The particle color will linearly interpolate between the color/time keys "
         "over the lifetime of the particle.");
      addProtectedField("sizes", TYPEID< F32 >(), Offset(mSizes, BillboardRendererData), &protectedSetSizes,
         &defaultProtectedGetFn, ParticleSystem::PDC_NUM_KEYS,
         "@brief Particle size keyframe values.\n\n"
         "The particle size will linearly interpolate between the size/time keys "
         "over the lifetime of the particle.");
      addProtectedField("times", TYPEID< F32 >(), Offset(mTimes, BillboardRendererData), &protectedSetTimes,
         &defaultProtectedGetFn, ParticleSystem::PDC_NUM_KEYS,
         "@brief Time keys used with the colors and sizes keyframes.\n\n"
         "Values are from 0.0 (particle creation) to 1.0 (end of lifespace).");

      Parent::initPersistFields();
   }

   void BillboardRendererData::packData(BitStream* stream)
   {
      Parent::packData(stream);

      stream->writeFlag(mSortParticles);
      stream->writeFlag(mOrientParticles);
      stream->writeFlag(mReverseOrder);
      if (stream->writeFlag(mAlignParticles))
      {
         stream->write(mAlignDirection.x);
         stream->write(mAlignDirection.y);
         stream->write(mAlignDirection.z);
      }
      stream->writeFlag(mOrientOnVelocity);

      stream->writeFlag(mRenderReflection);
      stream->write(mAmbientFactor);
      stream->writeFlag(mHighResOnly);
      stream->writeFlag(mGlow);

      S32 i, count;

      for (i = 0; i < 4; i++)
         mathWrite(*stream, mTexCoords[i]);

      stream->write(mSoftnessDistance);

      if (stream->writeFlag(mTextureName && mTextureName[0]))
         stream->writeString(mTextureName);

      stream->writeInt(mBlendStyle, 4);

      if (stream->writeFlag(mAnimateTexture))
      {
         if (stream->writeFlag(mAnimTexFramesString && mAnimTexFramesString[0]))
         {
            stream->writeString(mAnimTexFramesString);
         }
         mathWrite(*stream, mAnimTexTiling);
         stream->writeInt(mFramesPerSec, 8);
      }

      // see how many frames there are:
      for (count = 1; count <= ParticleSystem::PDC_NUM_KEYS; count++)
         if (mTimes[count - 1] >= 1)
            break;

      stream->writeInt(count - 1, 4);

      for (i = 0; i < count; i++)
      {
         stream->writeFloat(mColors[i].red, 7);
         stream->writeFloat(mColors[i].green, 7);
         stream->writeFloat(mColors[i].blue, 7);
         stream->writeFloat(mColors[i].alpha, 7);
         stream->writeFloat(mSizes[i] / MaxParticleSize, 14);
         stream->writeFloat(mTimes[i], 8);
      }
   }

   void BillboardRendererData::unpackData(BitStream* stream)
   {
      Parent::unpackData(stream);

      mSortParticles = stream->readFlag();
      mOrientParticles = stream->readFlag();
      mReverseOrder = stream->readFlag();
      mAlignParticles = stream->readFlag();
      if (mAlignParticles)
      {
         stream->read(&mAlignDirection.x);
         stream->read(&mAlignDirection.y);
         stream->read(&mAlignDirection.z);
      }
      mOrientOnVelocity = stream->readFlag();

      mRenderReflection = stream->readFlag();
      stream->read(&mAmbientFactor);
      mHighResOnly = stream->readFlag();
      mGlow = stream->readFlag();

      S32 i, count;

      for (i = 0; i < 4; i++)
         mathRead(*stream, &mTexCoords[i]);

      stream->read(&mSoftnessDistance);

      mTextureName = (stream->readFlag()) ? stream->readSTString() : 0;

      mBlendStyle = (ParticleRenderInst::BlendStyle)stream->readInt(4);

      mAnimateTexture = stream->readFlag();
      if (mAnimateTexture)
      {
         mAnimTexFramesString = (stream->readFlag()) ? stream->readSTString() : 0;
         mathRead(*stream, &mAnimTexTiling);
         mFramesPerSec = stream->readInt(8);
      }

      count = stream->readInt(4);

      for (i = 0; i < count; i++)
      {
         mColors[i].red = stream->readFloat(7);
         mColors[i].green = stream->readFloat(7);
         mColors[i].blue = stream->readFloat(7);
         mColors[i].alpha = stream->readFloat(7);
         mSizes[i] = stream->readFloat(14) * MaxParticleSize;
         mTimes[i] = stream->readFloat(8);
      }
   }

   bool BillboardRendererData::preload(bool server, String &errorStr)
   {
      if (Parent::preload(server, errorStr) == false)
         return false;

      bool error = false;
      if (!server)
      {
         // Here we attempt to load the particle's texture if specified. An undefined
         // texture is *not* an error since the emitter may provide one.
         if (mTextureName && mTextureName[0])
         {
            mTextureHandle = GFXTexHandle(mTextureName, &GFXDefaultStaticDiffuseProfile, avar("%s() - textureHandle (line %d)", __FUNCTION__, __LINE__));
            if (!mTextureHandle)
            {
               errorStr = String::ToString("Missing particle texture: %s", mTextureName);
               error = true;
            }
         }
      }

      if (mAnimateTexture)
      {
         // Here we parse animTexFramesString into byte-size frame numbers in animTexFrames.
         // Each frame token must be separated by whitespace.
         // A frame token must be a positive integer frame number or a range of frame numbers
         // separated with a '-'. 
         // The range separator, '-', cannot have any whitspace around it.
         // Ranges can be specified to move through the frames in reverse as well as forward.
         // Frame numbers exceeding the number of tiles will wrap.
         //   example:
         //     "0-16 20 19 18 17 31-21"


         S32 n_tiles = mAnimTexTiling.x * mAnimTexTiling.y;
         AssertFatal(n_tiles > 0 && n_tiles <= 256, "Error, bad animTexTiling setting.");


         mAnimTexFrames.clear();


         char* tokCopy = new char[dStrlen(mAnimTexFramesString) + 1];
         dStrcpy(tokCopy, mAnimTexFramesString);


         char* currTok = dStrtok(tokCopy, " \t");
         while (currTok != NULL)
         {
            char* minus = dStrchr(currTok, '-');
            if (minus)
            {
               // add a range of frames
               *minus = '\0';
               S32 range_a = dAtoi(currTok);
               S32 range_b = dAtoi(minus + 1);
               if (range_b < range_a)
               {
                  // reverse frame range
                  for (S32 i = range_a; i >= range_b; i--)
                     mAnimTexFrames.push_back((U8)(i % n_tiles));
               }
               else
               {
                  // forward frame range
                  for (S32 i = range_a; i <= range_b; i++)
                     mAnimTexFrames.push_back((U8)(i % n_tiles));
               }
            }
            else
            {
               // add one frame
               mAnimTexFrames.push_back((U8)(dAtoi(currTok) % n_tiles));
            }
            currTok = dStrtok(NULL, " \t");
         }


         // Here we pre-calculate the UVs for each frame tile, which are
         // tiled inside the UV region specified by texCoords. Since the
         // UVs are calculated using bilinear interpolation, the texCoords
         // region does *not* have to be an axis-aligned rectangle.


         if (mAnimTexUVs)
            delete[] mAnimTexUVs;


         mAnimTexUVs = new Point2F[(mAnimTexTiling.x + 1)*(mAnimTexTiling.y + 1)];


         // interpolate points on the left and right edge of the uv quadrangle
         Point2F lf_pt = mTexCoords[0];
         Point2F rt_pt = mTexCoords[3];


         // per-row delta for left and right interpolated points
         Point2F lf_d = (mTexCoords[1] - mTexCoords[0]) / (F32)mAnimTexTiling.y;
         Point2F rt_d = (mTexCoords[2] - mTexCoords[3]) / (F32)mAnimTexTiling.y;


         S32 idx = 0;
         for (S32 yy = 0; yy <= mAnimTexTiling.y; yy++)
         {
            Point2F p = lf_pt;
            Point2F dp = (rt_pt - lf_pt) / (F32)mAnimTexTiling.x;
            for (S32 xx = 0; xx <= mAnimTexTiling.x; xx++)
            {
               mAnimTexUVs[idx++] = p;
               p += dp;
            }
            lf_pt += lf_d;
            rt_pt += rt_d;
         }


         // cleanup
         delete[] tokCopy;
         mNumFrames = mAnimTexFrames.size();
      }

      return !error;
   }

   //-----------------------------------------------------------------------------
   // alloc PrimitiveBuffer
   // The datablock allocates this static index buffer because it's the same
   // for all of the emitters - each particle quad uses the same index ordering
   //-----------------------------------------------------------------------------
   void BillboardRendererData::allocPrimBuffer(const ParticlePool *partPool)
   {
      // create index buffer based on that size
      U32 indexListSize = partPool->getCapacity() * 6; // 6 indices per particle
      U16 *indices = new U16[indexListSize];

      for (U32 i = 0; i < partPool->getCapacity(); i++)
      {
         // this index ordering should be optimal (hopefully) for the vertex cache
         U16 *idx = &indices[i * 6];
         volatile U32 offset = i * 4;  // set to volatile to fix VC6 Release mode compiler bug
         idx[0] = 0 + offset;
         idx[1] = 1 + offset;
         idx[2] = 3 + offset;
         idx[3] = 1 + offset;
         idx[4] = 3 + offset;
         idx[5] = 2 + offset;
      }


      U16 *ibIndices;
      GFXBufferType bufferType = GFXBufferTypeStatic;

#ifdef TORQUE_OS_XENON
      // Because of the way the volatile buffers work on Xenon this is the only
      // way to do this.
      bufferType = GFXBufferTypeVolatile;
#endif
      mPrimBuff.set(GFX, indexListSize, 0, bufferType);
      mPrimBuff.lock(&ibIndices);
      dMemcpy(ibIndices, indices, indexListSize * sizeof(U16));
      mPrimBuff.unlock();

      delete[] indices;
   }


   F32 BillboardRenderer::getParticleSize(Particle const* part)
   {
      AssertFatal(part->totalLifetime > 0, "Particle lifetime must be larger than 0.");
      F32 t = F32(part->currentAge) / F32(part->totalLifetime);
      AssertFatal(t <= 1.0f, "Out out bounds filter function for particle.");

      for (U32 i = 1; i < ParticleSystem::PDC_NUM_KEYS; i++)
      {
         if (getDataBlock()->getTime(i) >= t)
         {
            F32 firstPart = t - getDataBlock()->getTime(i - 1);
            F32 total = getDataBlock()->getTime(i) -
               getDataBlock()->getTime(i - 1);

            firstPart /= total;

            return (getDataBlock()->getSize(i - 1) * (1.0 - firstPart)) +
               (getDataBlock()->getSize(i)   * firstPart);
         }
      }
   }

   //-----------------------------------------------------------------------------
   // setSizes
   //-----------------------------------------------------------------------------
   void BillboardRenderer::setSizes(F32 *sizeList)
   {
      for (int i = 0; i < ParticleSystem::PDC_NUM_KEYS; i++)
      {
         getDataBlock()->setSize(i, sizeList[i]);
      }
   }

   //-----------------------------------------------------------------------------
   // setColors
   //-----------------------------------------------------------------------------
   void BillboardRenderer::setColors(ColorF *colorList)
   {
      for (int i = 0; i < ParticleSystem::PDC_NUM_KEYS; i++)
      {
         getDataBlock()->setColor(i, colorList[i]);
      }
   }

   //-----------------------------------------------------------------------------
   // getParticleColor
   // - Calculates the current color of a particle based on its age and the key
   //   values.
   //-----------------------------------------------------------------------------
   ColorF BillboardRenderer::getParticleColor(Particle const* part)
   {
      AssertFatal(part->totalLifetime > 0, "Particle lifetime must be larger than 0.");
      F32 t = F32(part->currentAge) / F32(part->totalLifetime);
      AssertFatal(t <= 1.0f, "Out out bounds filter function for particle.");

      for (U32 i = 1; i < ParticleSystem::PDC_NUM_KEYS; i++)
      {
         if (getDataBlock()->getTime(i) >= t)
         {
            F32 firstPart = t - getDataBlock()->getTime(i - 1);
            F32 total = getDataBlock()->getTime(i) -
               getDataBlock()->getTime(i - 1);

            firstPart /= total;
            ColorF outCol;
            outCol.interpolate(getDataBlock()->getColor(i - 1), getDataBlock()->getColor(i), firstPart);
            return outCol;
         }
      }
   }

   //-----------------------------------------------------------------------------
   // renderPool
   // - Renders a ParticlePool.
   //-----------------------------------------------------------------------------
   bool BillboardRenderer::renderPool(ParticlePool *pool, SceneRenderState* state)
   {
      ParticlePool *partPool = mParentSystem->getParticlePool();
      if (state->isReflectPass() && !getDataBlock()->getRenderReflection())
         return false;

      // Never render into shadows.
      if (state->isShadowPass())
         return false;

      PROFILE_SCOPE(ParticleEmitter_prepRenderImage);

      // Early out if there are no particles.
      if (partPool->getCount() == 0 ||
         partPool->GetParticleHead()->next == NULL)
         return false;

      RenderPassManager *renderManager = state->getRenderPass();
      const Point3F &camPos = state->getCameraPosition();
      copyToVB(camPos, state->getAmbientLightColor());

      if (!mVertBuff.isValid())
         return false;

      if (getDataBlock()->getPrimBuff()->isNull() || !getDataBlock()->getPrimBuff()->isValid())
         getDataBlock()->allocPrimBuffer(mParentSystem->getParticlePool());

      ParticleRenderInst *ri = renderManager->allocInst<ParticleRenderInst>();

      ri->vertBuff = &mVertBuff;
      ri->primBuff = getDataBlock()->getPrimBuff();
      ri->translucentSort = true;
      ri->type = RenderPassManager::RIT_Particle;
      ri->sortDistSq = mParentSystem->getRenderWorldBox().getSqDistanceToPoint(camPos);
      ri->glow = getDataBlock()->getGlow();

      // Draw the system offscreen unless the highResOnly flag is set on the datablock
      ri->systemState = (getDataBlock()->getHighResOnly() ? PSS_AwaitingHighResDraw : PSS_AwaitingOffscreenDraw);

      ri->modelViewProj = renderManager->allocUniqueXform(GFX->getProjectionMatrix() *
         GFX->getViewMatrix() *
         GFX->getWorldMatrix());

      // Update position on the matrix before multiplying it
      mBBObjToWorld.setPosition(mParentSystem->getLastPosition());

      ri->bbModelViewProj = renderManager->allocUniqueXform(*ri->modelViewProj * mBBObjToWorld);

      ri->count = partPool->getCount();

      ri->blendStyle = getDataBlock()->getBlendStyle();

      // use first particle's texture unless there is an emitter texture to override it
      if (getDataBlock()->getTextureHandle())
         ri->diffuseTex = &*(getDataBlock()->getTextureHandle());

      ri->softnessDistance = getDataBlock()->getSoftnessDistance();

      // Sort by texture too.
      ri->defaultKey = ri->diffuseTex ? (U32)ri->diffuseTex : (U32)ri->vertBuff;

      renderManager->addInst(ri);

      return true;
   }

   //-----------------------------------------------------------------------------
   // Copy particles to vertex buffer
   //-----------------------------------------------------------------------------

   // structure used for particle sorting.
   struct SortParticle
   {
      Particle* p;
      F32       k;
   };

   // qsort callback function for particle sorting
   S32 QSORT_CALLBACK behavior_cmpSortParticles(const void* p1, const void* p2)
   {
      const SortParticle* sp1 = (const SortParticle*)p1;
      const SortParticle* sp2 = (const SortParticle*)p2;

      if (sp2->k > sp1->k)
         return 1;
      else if (sp2->k == sp1->k)
         return 0;
      else
         return -1;
   }

   void BillboardRenderer::copyToVB(Point3F const& camPos, ColorF const& ambientColor)
   {
      static Vector<SortParticle> orderedVector(__FILE__, __LINE__);

      PROFILE_START(ParticleEmitter_copyToVB);

      PROFILE_START(ParticleEmitter_copyToVB_Sort);
      // build sorted list of particles (far to near)
      if (getDataBlock()->getSortParticles())
      {
         orderedVector.clear();

         MatrixF modelview = GFX->getWorldMatrix();
         Point3F viewvec; modelview.getRow(1, &viewvec);

         // add each particle and a distance based sort key to orderedVector
         for (Particle* pp = mParentSystem->getParticlePool()->GetParticleHead()->next; pp != NULL; pp = pp->next)
         {
            orderedVector.increment();
            orderedVector.last().p = pp;
            orderedVector.last().k = mDot(pp->pos, viewvec);
         }

         // qsort the list into far to near ordering
         dQsort(orderedVector.address(), orderedVector.size(), sizeof(SortParticle), behavior_cmpSortParticles);
      }
      PROFILE_END();

#if defined(TORQUE_OS_XENON)
      // Allocate writecombined since we don't read back from this buffer (yay!)
      if(mVertBuff.isNull())
         mVertBuff = new GFX360MemVertexBuffer(GFX, 1, getGFXVertexFormat<ParticleVertexType>(), sizeof(ParticleVertexType), GFXBufferTypeDynamic, PAGE_WRITECOMBINE);
      if( n_parts > mCurBuffSize )
      {
         mCurBuffSize = n_parts;
         mVertBuff.resize(n_parts * 4);
      }

      ParticleVertexType *buffPtr = mVertBuff.lock();
#else
      static Vector<ParticleVertexType> tempBuff(2048);
      tempBuff.reserve(mParentSystem->getParticlePool()->getCount() * 4 + 64); // make sure tempBuff is big enough
      ParticleVertexType *buffPtr = tempBuff.address(); // use direct pointer (faster)
#endif

      if (getDataBlock()->getOrientParticles())
      {
         PROFILE_START(ParticleEmitter_copyToVB_Orient);

         if (getDataBlock()->getReverseOrder())
         {
            buffPtr += 4 * (mParentSystem->getParticlePool()->getCount() - 1);
            // do sorted-oriented particles
            if (getDataBlock()->getSortParticles())
            {
               SortParticle* partPtr = orderedVector.address();
               for (U32 i = 0; i < mParentSystem->getParticlePool()->getCount(); i++, partPtr++, buffPtr -= 4)
                  setupOriented(partPtr->p, camPos, ambientColor, buffPtr);
            }
            // do unsorted-oriented particles
            else
            {
               for (Particle* partPtr = mParentSystem->getParticlePool()->GetParticleHead()->next; partPtr != NULL; partPtr = partPtr->next, buffPtr -= 4)
                  setupOriented(partPtr, camPos, ambientColor, buffPtr);
            }
         }
         else
         {
            // do sorted-oriented particles
            if (getDataBlock()->getSortParticles())
            {
               SortParticle* partPtr = orderedVector.address();
               for (U32 i = 0; i < mParentSystem->getParticlePool()->getCount(); i++, partPtr++, buffPtr += 4)
                  setupOriented(partPtr->p, camPos, ambientColor, buffPtr);
            }
            // do unsorted-oriented particles
            else
            {
               for (Particle* partPtr = mParentSystem->getParticlePool()->GetParticleHead()->next; partPtr != NULL; partPtr = partPtr->next, buffPtr += 4)
                  setupOriented(partPtr, camPos, ambientColor, buffPtr);
            }
         }
         PROFILE_END();
      }
      else if (getDataBlock()->getAlignParticles())
      {
         PROFILE_START(ParticleEmitter_copyToVB_Aligned);

         if (getDataBlock()->getReverseOrder())
         {
            buffPtr += 4 * (mParentSystem->getParticlePool()->getCount() - 1);

            // do sorted-oriented particles
            if (getDataBlock()->getSortParticles())
            {
               SortParticle* partPtr = orderedVector.address();
               for (U32 i = 0; i < mParentSystem->getParticlePool()->getCount(); i++, partPtr++, buffPtr -= 4)
                  setupAligned(partPtr->p, ambientColor, buffPtr);
            }
            // do unsorted-oriented particles
            else
            {
               Particle *partPtr = mParentSystem->getParticlePool()->GetParticleHead()->next;
               for (; partPtr != NULL; partPtr = partPtr->next, buffPtr -= 4)
                  setupAligned(partPtr, ambientColor, buffPtr);
            }
         }
         else
         {
            // do sorted-oriented particles
            if (getDataBlock()->getSortParticles())
            {
               SortParticle* partPtr = orderedVector.address();
               for (U32 i = 0; i < mParentSystem->getParticlePool()->getCount(); i++, partPtr++, buffPtr += 4)
                  setupAligned(partPtr->p, ambientColor, buffPtr);
            }
            // do unsorted-oriented particles
            else
            {
               Particle *partPtr = mParentSystem->getParticlePool()->GetParticleHead()->next;
               for (; partPtr != NULL; partPtr = partPtr->next, buffPtr += 4)
                  setupAligned(partPtr, ambientColor, buffPtr);
            }
         }
         PROFILE_END();
      }
      else
      {
         PROFILE_START(ParticleEmitter_copyToVB_NonOriented);
         // somewhat odd ordering so that texture coordinates match the oriented
         // particles
         Point3F basePoints[4];
         basePoints[0] = Point3F(-1.0, 0.0, 1.0);
         basePoints[1] = Point3F(-1.0, 0.0, -1.0);
         basePoints[2] = Point3F(1.0, 0.0, -1.0);
         basePoints[3] = Point3F(1.0, 0.0, 1.0);

         MatrixF camView = GFX->getWorldMatrix();
         camView.transpose();  // inverse - this gets the particles facing camera

         if (getDataBlock()->getReverseOrder())
         {
            buffPtr += 4 * (mParentSystem->getParticlePool()->getCount() - 1);
            // do sorted-billboard particles
            if (getDataBlock()->getSortParticles())
            {
               SortParticle *partPtr = orderedVector.address();
               for (U32 i = 0; i < mParentSystem->getParticlePool()->getCount(); i++, partPtr++, buffPtr -= 4)
                  setupBillboard(partPtr->p, basePoints, camView, ambientColor, buffPtr);
            }
            // do unsorted-billboard particles
            else
            {
               for (Particle* partPtr = mParentSystem->getParticlePool()->GetParticleHead()->next; partPtr != NULL; partPtr = partPtr->next, buffPtr -= 4)
                  setupBillboard(partPtr, basePoints, camView, ambientColor, buffPtr);
            }
         }
         else
         {
            // do sorted-billboard particles
            if (getDataBlock()->getSortParticles())
            {
               SortParticle *partPtr = orderedVector.address();
               for (U32 i = 0; i < mParentSystem->getParticlePool()->getCount(); i++, partPtr++, buffPtr += 4)
                  setupBillboard(partPtr->p, basePoints, camView, ambientColor, buffPtr);
            }
            // do unsorted-billboard particles
            else
            {
               for (Particle* partPtr = mParentSystem->getParticlePool()->GetParticleHead()->next; partPtr != NULL; partPtr = partPtr->next, buffPtr += 4)
                  setupBillboard(partPtr, basePoints, camView, ambientColor, buffPtr);
            }
         }

         PROFILE_END();
      }

#if defined(TORQUE_OS_XENON)
      mVertBuff.unlock();
#else
      PROFILE_START(ParticleEmitter_copyToVB_LockCopy);
      // create new VB if emitter size grows
      if (!mVertBuff || mParentSystem->getParticlePool()->getCount() > mCurBuffSize)
      {
         mCurBuffSize = mParentSystem->getParticlePool()->getCount();
         mVertBuff.set(GFX, mParentSystem->getParticlePool()->getCount() * 4, GFXBufferTypeDynamic);
      }
      // lock and copy tempBuff to video RAM
      ParticleVertexType *verts = mVertBuff.lock();
      dMemcpy(verts, tempBuff.address(), mParentSystem->getParticlePool()->getCount() * 4 * sizeof(ParticleVertexType));
      mVertBuff.unlock();
      PROFILE_END();
#endif

      PROFILE_END();
   }

   //-----------------------------------------------------------------------------
   // Set up particle for billboard style render
   //-----------------------------------------------------------------------------
   void BillboardRenderer::setupBillboard(Particle *part,
      Point3F *basePts,
      const MatrixF &camView,
      const ColorF &ambientColor,
      ParticleVertexType *lVerts)
   {
      F32 width = getParticleSize(part) * 0.5f;
      F32 spinAngle = part->spinSpeed * part->currentAge * ParticleSystem::AgedSpinToRadians;

      F32 sy, cy;
      mSinCos(spinAngle, sy, cy);

      const F32 ambientLerp = mClampF(getDataBlock()->getAmbientFactor(), 0.0f, 1.0f);
      ColorF partCol = mLerp(getParticleColor(part), (getParticleColor(part) * ambientColor), ambientLerp);

      // fill four verts, use macro and unroll loop
#define fillVert(){ \
      lVerts->point.x = cy * basePts->x - sy * basePts->z;  \
      lVerts->point.y = 0.0f;                                \
      lVerts->point.z = sy * basePts->x + cy * basePts->z;  \
      camView.mulV( lVerts->point );                        \
      lVerts->point *= width;                               \
      lVerts->point += part->pos;                           \
      lVerts->color = partCol; } \

      // Here we deal with UVs for animated particle (billboard)
      if (getDataBlock()->getAnimateTexture())
      {
         S32 fm = (S32)(part->currentAge*(1.0 / 1000.0)*getDataBlock()->getFramesPerSec());
         U8 fm_tile = getDataBlock()->getAnimTexFrames()[fm % getDataBlock()->getNumFrames()];
         S32 uv[4];
         uv[0] = fm_tile + fm_tile / getDataBlock()->getAnimTexTiling().x;
         uv[1] = uv[0] + (getDataBlock()->getAnimTexTiling().x + 1);
         uv[2] = uv[1] + 1;
         uv[3] = uv[0] + 1;

         fillVert();
         // Here and below, we copy UVs from particle datablock's current frame's UVs (billboard)
         lVerts->texCoord = getDataBlock()->getAnimTexUVs()[uv[0]];
         ++lVerts;
         ++basePts;

         fillVert();
         lVerts->texCoord = getDataBlock()->getAnimTexUVs()[uv[1]];
         ++lVerts;
         ++basePts;

         fillVert();
         lVerts->texCoord = getDataBlock()->getAnimTexUVs()[uv[2]];
         ++lVerts;
         ++basePts;

         fillVert();
         lVerts->texCoord = getDataBlock()->getAnimTexUVs()[uv[3]];
         ++lVerts;
         ++basePts;

         return;
      }

      fillVert();
      // Here and below, we copy UVs from particle datablock's texCoords (billboard)
      lVerts->texCoord = getDataBlock()->getTexCoords(0);
      ++lVerts;
      ++basePts;

      fillVert();
      lVerts->texCoord = getDataBlock()->getTexCoords(1);
      ++lVerts;
      ++basePts;

      fillVert();
      lVerts->texCoord = getDataBlock()->getTexCoords(2);
      ++lVerts;
      ++basePts;

      fillVert();
      lVerts->texCoord = getDataBlock()->getTexCoords(3);
      ++lVerts;
      ++basePts;
   }

   //-----------------------------------------------------------------------------
   // Set up oriented particle
   //-----------------------------------------------------------------------------
   void BillboardRenderer::setupOriented(Particle *part,
      const Point3F &camPos,
      const ColorF &ambientColor,
      ParticleVertexType *lVerts)
   {
      Point3F dir;

      if (getDataBlock()->getOrientOnVelocity())
      {
         // don't render oriented particle if it has no velocity
         if (part->vel.magnitudeSafe() == 0.0) return;
         dir = part->vel;
      }
      else
      {
         dir = part->orientDir;
      }

      Point3F dirFromCam = part->pos - camPos;
      Point3F crossDir;
      mCross(dirFromCam, dir, &crossDir);
      crossDir.normalize();
      dir.normalize();

      F32 width = getParticleSize(part) * 0.5f;
      dir *= width;
      crossDir *= width;
      Point3F start = part->pos - dir;
      Point3F end = part->pos + dir;

      const F32 ambientLerp = mClampF(getDataBlock()->getAmbientFactor(), 0.0f, 1.0f);
      ColorF partCol = mLerp(getParticleColor(part), (getParticleColor(part) * ambientColor), ambientLerp);

      // Here we deal with UVs for animated particle (oriented)
      if (getDataBlock()->getAnimateTexture())
      {
         // Let particle compute the UV indices for current frame
         S32 fm = (S32)(part->currentAge*(1.0f / 1000.0f)*getDataBlock()->getFramesPerSec());
         U8 fm_tile = getDataBlock()->getAnimTexFrames()[fm % getDataBlock()->getNumFrames()];
         S32 uv[4];
         uv[0] = fm_tile + fm_tile / getDataBlock()->getAnimTexTiling().x;
         uv[1] = uv[0] + (getDataBlock()->getAnimTexTiling().x + 1);
         uv[2] = uv[1] + 1;
         uv[3] = uv[0] + 1;

         lVerts->point = start + crossDir;
         lVerts->color = partCol;
         // Here and below, we copy UVs from particle datablock's current frame's UVs (oriented)
         lVerts->texCoord = getDataBlock()->getAnimTexUVs()[uv[0]];
         ++lVerts;

         lVerts->point = start - crossDir;
         lVerts->color = partCol;
         lVerts->texCoord = getDataBlock()->getAnimTexUVs()[uv[1]];
         ++lVerts;

         lVerts->point = end - crossDir;
         lVerts->color = partCol;
         lVerts->texCoord = getDataBlock()->getAnimTexUVs()[uv[2]];
         ++lVerts;

         lVerts->point = end + crossDir;
         lVerts->color = partCol;
         lVerts->texCoord = getDataBlock()->getAnimTexUVs()[uv[3]];
         ++lVerts;

         return;
      }

      lVerts->point = start + crossDir;
      lVerts->color = partCol;
      // Here and below, we copy UVs from particle datablock's texCoords (oriented)
      lVerts->texCoord = getDataBlock()->getTexCoords(0);
      ++lVerts;

      lVerts->point = start - crossDir;
      lVerts->color = partCol;
      lVerts->texCoord = getDataBlock()->getTexCoords(1);
      ++lVerts;

      lVerts->point = end - crossDir;
      lVerts->color = partCol;
      lVerts->texCoord = getDataBlock()->getTexCoords(2);
      ++lVerts;

      lVerts->point = end + crossDir;
      lVerts->color = partCol;
      lVerts->texCoord = getDataBlock()->getTexCoords(3);
      ++lVerts;
   }

   void BillboardRenderer::setupAligned(const Particle *part,
      const ColorF &ambientColor,
      ParticleVertexType *lVerts)
   {
      // The aligned direction will always be normalized.
      Point3F dir = getDataBlock()->getAlignDirection();

      // Find a right vector for this particle.
      Point3F right;
      if (mFabs(dir.y) > mFabs(dir.z))
         mCross(Point3F::UnitZ, dir, &right);
      else
         mCross(Point3F::UnitY, dir, &right);
      right.normalize();

      // If we have a spin velocity.
      if (!mIsZero(part->spinSpeed))
      {
         F32 spinAngle = part->spinSpeed * part->currentAge * ParticleSystem::AgedSpinToRadians;

         // This is an inline quaternion vector rotation which
         // is faster that QuatF.mulP(), but generates different
         // results and hence cannot replace it right now.

         F32 sin, qw;
         mSinCos(spinAngle * 0.5f, sin, qw);
         F32 qx = dir.x * sin;
         F32 qy = dir.y * sin;
         F32 qz = dir.z * sin;

         F32 vx = (right.x * qw) + (right.z * qy) - (right.y * qz);
         F32 vy = (right.y * qw) + (right.x * qz) - (right.z * qx);
         F32 vz = (right.z * qw) + (right.y * qx) - (right.x * qy);
         F32 vw = (right.x * qx) + (right.y * qy) + (right.z * qz);

         right.x = (qw * vx) + (qx * vw) + (qy * vz) - (qz * vy);
         right.y = (qw * vy) + (qy * vw) + (qz * vx) - (qx * vz);
         right.z = (qw * vz) + (qz * vw) + (qx * vy) - (qy * vx);
      }

      // Get the cross vector.
      Point3F cross;
      mCross(right, dir, &cross);

      F32 width = getParticleSize(part) * 0.5f;
      right *= width;
      cross *= width;
      Point3F start = part->pos - right;
      Point3F end = part->pos + right;

      const F32 ambientLerp = mClampF(getDataBlock()->getAmbientFactor(), 0.0f, 1.0f);
      ColorF partCol = mLerp(getParticleColor(part), (getParticleColor(part) * ambientColor), ambientLerp);

      // Here we deal with UVs for animated particle
      if (getDataBlock()->getAnimateTexture())
      {
         // Let particle compute the UV indices for current frame
         S32 fm = (S32)(part->currentAge*(1.0f / 1000.0f)*getDataBlock()->getFramesPerSec());
         U8 fm_tile = getDataBlock()->getAnimTexFrames()[fm % getDataBlock()->getNumFrames()];
         S32 uv[4];
         uv[0] = fm_tile + fm_tile / getDataBlock()->getAnimTexTiling().x;
         uv[1] = uv[0] + (getDataBlock()->getAnimTexTiling().x + 1);
         uv[2] = uv[1] + 1;
         uv[3] = uv[0] + 1;

         lVerts->point = start + cross;
         lVerts->color = partCol;
         lVerts->texCoord = getDataBlock()->getAnimTexUVs()[uv[0]];
         ++lVerts;

         lVerts->point = start - cross;
         lVerts->color = partCol;
         lVerts->texCoord = getDataBlock()->getAnimTexUVs()[uv[1]];
         ++lVerts;

         lVerts->point = end - cross;
         lVerts->color = partCol;
         lVerts->texCoord = getDataBlock()->getAnimTexUVs()[uv[2]];
         ++lVerts;

         lVerts->point = end + cross;
         lVerts->color = partCol;
         lVerts->texCoord = getDataBlock()->getAnimTexUVs()[uv[3]];
         ++lVerts;
      }

      // Here and below, we copy UVs from particle datablock's texCoords
      lVerts->point = start + cross;
      lVerts->color = partCol;
      lVerts->texCoord = getDataBlock()->getTexCoords(0);
      ++lVerts;

      lVerts->point = start - cross;
      lVerts->color = partCol;
      lVerts->texCoord = getDataBlock()->getTexCoords(1);
      ++lVerts;

      lVerts->point = end - cross;
      lVerts->color = partCol;
      lVerts->texCoord = getDataBlock()->getTexCoords(2);
      ++lVerts;

      lVerts->point = end + cross;
      lVerts->color = partCol;
      lVerts->texCoord = getDataBlock()->getTexCoords(3);
      ++lVerts;
   }