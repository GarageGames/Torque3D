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

#ifndef _PARTICLE_H_
#define _PARTICLE_H_

#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif

#define MaxParticleSize 50.0

struct Particle;

//*****************************************************************************
// Particle Data
//*****************************************************************************
class ParticleData : public SimDataBlock
{
   typedef SimDataBlock Parent;

  public:
   enum PDConst
   {
      PDC_NUM_KEYS = 4,
   };

   F32   dragCoefficient;
   F32   windCoefficient;
   F32   gravityCoefficient;

   F32   inheritedVelFactor;
   F32   constantAcceleration;

   S32   lifetimeMS;
   S32   lifetimeVarianceMS;

   F32   spinSpeed;        // degrees per second
   F32   spinRandomMin;
   F32   spinRandomMax;

   bool  useInvAlpha;

   bool  animateTexture;
   U32   numFrames;
   U32   framesPerSec;

   ColorF colors[ PDC_NUM_KEYS ];
   F32    sizes[ PDC_NUM_KEYS ];
   F32    times[ PDC_NUM_KEYS ];

   Point2F*          animTexUVs;
   Point2F           texCoords[4];   // default: {{0.0,0.0}, {0.0,1.0}, {1.0,1.0}, {1.0,0.0}} 
   Point2I           animTexTiling;
   StringTableEntry  animTexFramesString;
   Vector<U8>        animTexFrames;
   StringTableEntry  textureName;
   GFXTexHandle      textureHandle;

   static bool protectedSetSizes( void *object, const char *index, const char *data );
   static bool protectedSetTimes( void *object, const char *index, const char *data );

  public:
   ParticleData();
   ~ParticleData();

   // move this procedure to Particle
   void initializeParticle(Particle*, const Point3F&);

   void packData(BitStream* stream);
   void unpackData(BitStream* stream);
   bool onAdd();
   bool preload(bool server, String &errorStr);
   DECLARE_CONOBJECT(ParticleData);
   static void  initPersistFields();

   bool reload(char errorBuffer[256]);
};

//*****************************************************************************
// Particle
// 
// This structure should be as small as possible.
//*****************************************************************************
struct Particle
{
   Point3F  pos;     // current instantaneous position
   Point3F  vel;     //   "         "         velocity
   Point3F  acc;     // Constant acceleration
   Point3F  orientDir;  // direction particle should go if using oriented particles

   U32           totalLifetime;   // Total ms that this instance should be "live"
   ParticleData* dataBlock;       // datablock that contains global parameters for
                                  //  this instance
   U32       currentAge;


   // are these necessary to store here? - they are interpolated in real time
   ColorF           color;
   F32              size;

   F32              spinSpeed;
   Particle *       next;
};


#endif // _PARTICLE_H_