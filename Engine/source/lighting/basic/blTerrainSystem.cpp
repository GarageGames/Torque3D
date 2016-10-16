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
#include "lighting/basic/blTerrainSystem.h"

#include "core/bitVector.h"
#include "lighting/common/shadowVolumeBSP.h"
#include "lighting/lightingInterfaces.h"
#include "terrain/terrData.h"
#include "lighting/basic/basicLightManager.h"
#include "lighting/common/sceneLighting.h"
#include "gfx/bitmap/gBitmap.h"
#include "collision/collision.h"

extern SceneLighting* gLighting;


struct blTerrainChunk : public PersistInfo::PersistChunk
{
   typedef PersistInfo::PersistChunk Parent;

   blTerrainChunk();
   ~blTerrainChunk();

   GBitmap *mLightmap;

   bool read(Stream &);
   bool write(Stream &);
};

//------------------------------------------------------------------------------
// Class SceneLighting::TerrainChunk
//------------------------------------------------------------------------------
blTerrainChunk::blTerrainChunk()
{
   mChunkType = PersistChunk::TerrainChunkType;
   mLightmap = NULL;
}

blTerrainChunk::~blTerrainChunk()
{
   if(mLightmap)
      delete mLightmap;
}

//------------------------------------------------------------------------------

bool blTerrainChunk::read(Stream & stream)
{
   if(!Parent::read(stream))
      return(false);

   mLightmap = new GBitmap();
   return mLightmap->readBitmap("png",stream);
}

bool blTerrainChunk::write(Stream & stream)
{
   if(!Parent::write(stream))
      return(false);

   if(!mLightmap)
      return(false);

   if(!mLightmap->writeBitmap("png",stream))
      return(false);

   return(true);
}

class blTerrainProxy : public SceneLighting::ObjectProxy
{
protected:

   typedef ObjectProxy Parent;

   BitVector               mShadowMask;
   ShadowVolumeBSP *       mShadowVolume;
   ColorF *                mLightmap;

   /// The dimension of the lightmap in pixels.
   const U32 mLightMapSize;

   /// The dimension of the terrain height map sample array.
   const U32 mTerrainBlockSize;

   ColorF *sgBakedLightmap;
   Vector<LightInfo *> sgLights;
   bool sgMarkStaticShadow(void *terrainproxy, SceneObject *sceneobject, LightInfo *light);
   //void postLight(bool lastLight);

   void lightVector(LightInfo *);

   struct SquareStackNode
   {
      U8          mLevel;
      U16         mClipFlags;
      Point2I     mPos;
   };

   S32 testSquare(const Point3F &, const Point3F &, S32, F32, const Vector<PlaneF> &);
   bool markObjectShadow(ObjectProxy *);
   bool sgIsCorrectStaticObjectType(SceneObject *obj);

   inline ColorF _getValue( S32 row, S32 column );

public:

   blTerrainProxy(SceneObject * obj);
   ~blTerrainProxy();
   TerrainBlock * operator->() {return(static_cast<TerrainBlock*>(static_cast<SceneObject*>(mObj)));}
   TerrainBlock * getObject() {return(static_cast<TerrainBlock*>(static_cast<SceneObject*>(mObj)));}

   bool getShadowedSquares(const Vector<PlaneF> &, Vector<U16> &);

   // lighting
   void init();
   bool preLight(LightInfo *);
   void light(LightInfo *);

   // persist
   U32 getResourceCRC();
   bool setPersistInfo(PersistInfo::PersistChunk *);
   bool getPersistInfo(PersistInfo::PersistChunk *);

   virtual bool supportsShadowVolume();
   virtual void getClipPlanes(Vector<PlaneF>& planes);
   virtual void addToShadowVolume(ShadowVolumeBSP * shadowVolume, LightInfo * light, S32 level);

   // events
   //virtual void processTGELightProcessEvent(U32 curr, U32 max, LightInfo* currlight); 
   //virtual void processSGObjectProcessEvent(LightInfo* currLight);
};

//-------------------------------------------------------------------------------
// Class SceneLighting::TerrainProxy:
//-------------------------------------------------------------------------------
blTerrainProxy::blTerrainProxy( SceneObject *obj ) :
   Parent( obj ),
   mLightMapSize( getObject()->getLightMapSize() ),
   mTerrainBlockSize( getObject()->getBlockSize() ),
   mShadowVolume( NULL ),
   mLightmap( NULL ),
   sgBakedLightmap( NULL )
{
}

blTerrainProxy::~blTerrainProxy()
{
   delete [] mLightmap;
}

//-------------------------------------------------------------------------------
void blTerrainProxy::init()
{
   mLightmap = new ColorF[ mLightMapSize * mLightMapSize ];
   dMemset(mLightmap, 0, mLightMapSize * mLightMapSize * sizeof(ColorF));
   mShadowMask.setSize( mTerrainBlockSize * mTerrainBlockSize );
}

bool blTerrainProxy::preLight(LightInfo * light)
{
   if(!bool(mObj))
      return(false);

   if(light->getType() != LightInfo::Vector)
      return(false);

   mShadowMask.clear();
   return(true);
}

inline ColorF blTerrainProxy::_getValue( S32 row, S32 column )
{
   while( row < 0 )
      row += mLightMapSize;
   row = row % mLightMapSize;

   while( column < 0 )
      column += mLightMapSize;
   column = column % mLightMapSize;

   U32 offset = row * mLightMapSize + column;

   return mLightmap[offset];
}

bool blTerrainProxy::markObjectShadow(ObjectProxy * proxy)
{
   if (!proxy->supportsShadowVolume())
      return false;

   // setup the clip planes
   Vector<PlaneF> clipPlanes;
   proxy->getClipPlanes(clipPlanes);

   Vector<U16> shadowList;
   if(!getShadowedSquares(clipPlanes, shadowList))
      return(false);

   // set the correct bit
   for(U32 i = 0; i < shadowList.size(); i++)
      mShadowMask.set(shadowList[i]);

   return(true);
}

void blTerrainProxy::light(LightInfo * light)
{
   // If we don't have terrain or its not a directional
   // light then skip processing.
   TerrainBlock * terrain = getObject();
   if ( !terrain || light->getType() != LightInfo::Vector )
      return;

   S32 time = Platform::getRealMilliseconds();

   // reset
   mShadowVolume = new ShadowVolumeBSP;

   // build interior shadow volume
   for(ObjectProxy ** itr = gLighting->mLitObjects.begin(); itr != gLighting->mLitObjects.end(); itr++)
   {
      ObjectProxy* objproxy = *itr;
      if (markObjectShadow(objproxy))
         objproxy->addToShadowVolume(mShadowVolume, light, SceneLighting::SHADOW_DETAIL);
   }

   lightVector(light);

   // set the lightmap...
   terrain->clearLightMap();

   // Blur...
   F32 kernel[3][3] = { {1, 2, 1},
                        {2, 3, 2},
                        {1, 2, 1} };

   F32 modifier = 1;
   F32 divisor = 0;


   for( U32 i=0; i<3; i++ )
   {
      for( U32 j=0; j<3; j++ )
      {
         if( i==1 && j==1 )
         {
            kernel[i][j] = 1 + kernel[i][j] * modifier;
         }
         else
         {
            kernel[i][j] = kernel[i][j] * modifier;
         }

         divisor += kernel[i][j];
      }
   }

   for( U32 i=0; i < mLightMapSize; i++ )
   {
      for( U32 j=0; j < mLightMapSize; j++ )
      {

         ColorF val;
         val  = _getValue( i-1, j-1  ) * kernel[0][0];
         val += _getValue( i-1, j    ) * kernel[0][1];
         val += _getValue( i-1, j+1  ) * kernel[0][2];
         val += _getValue(   i, j-1  ) * kernel[1][0];
         val += _getValue(   i, j    ) * kernel[1][1];
         val += _getValue(   i, j+1  ) * kernel[1][2];
         val += _getValue( i+1, j-1  ) * kernel[2][0];
         val += _getValue( i+1, j    ) * kernel[2][1];
         val += _getValue( i+1, j+1  ) * kernel[2][2];

         U32 edge = 0;

         if( j == 0 || j == mLightMapSize - 1 )
            edge++;

         if( i == 0 || i == mLightMapSize - 1 )
            edge++;

         if( !edge )
            val = val / divisor;
         else
            val = mLightmap[ i * mLightMapSize + j ];

         // clamp values
         mLightmap[ i * mLightMapSize + j ]= val;
      }
   }

   // And stuff it into the texture...
   GBitmap *terrLightMap = terrain->getLightMap();
   for(U32 y = 0; y < mLightMapSize; y++)
   {
      for(U32 x = 0; x < mLightMapSize; x++)
      {
         ColorI color(255, 255, 255, 255);
         
         color.red   = mLightmap[x + y * mLightMapSize].red   * 255;
         color.green = mLightmap[x + y * mLightMapSize].green * 255;
         color.blue  = mLightmap[x + y * mLightMapSize].blue  * 255;

         terrLightMap->setColor(x, y, color);
      }
   }

   /*
   // This handles matching up the outer edges of the terrain
   // lightmap when it has neighbors
   if (!terrain->isTiling())
   {
      for (S32 y = 0; y < terrLightMap->getHeight(); y++)
      {
         ColorI c;
         if (terrain->getFile()->mEdgeTerrainFiles[0])
         {
            terrLightMap->getColor(terrLightMap->getWidth()-1,y,c);
            terrLightMap->setColor(0,y,c);
            terrLightMap->setColor(1,y,c);
         }
         else
         {
            terrLightMap->getColor(0,y,c);
            terrLightMap->setColor(terrLightMap->getWidth()-1,y,c);
            terrLightMap->setColor(terrLightMap->getWidth()-2,y,c);
         }
      }

      for (S32 x = 0; x < terrLightMap->getHeight(); x++)
      {
         ColorI c;
         if (terrain->getFile()->mEdgeTerrainFiles[1])
         {
            terrLightMap->getColor(x,terrLightMap->getHeight()-1,c);
            terrLightMap->setColor(x,0,c);
            terrLightMap->setColor(x,1,c);
         }
         else
         {
            terrLightMap->getColor(x,0,c);
            terrLightMap->setColor(x,terrLightMap->getHeight()-1,c);
            terrLightMap->setColor(x,terrLightMap->getHeight()-2,c);
         }
      }
   }
   */

   delete mShadowVolume;

   Con::printf("    = terrain lit in %3.3f seconds", (Platform::getRealMilliseconds()-time)/1000.f);
}

//------------------------------------------------------------------------------
S32 blTerrainProxy::testSquare(const Point3F & min, const Point3F & max, S32 mask, F32 expand, const Vector<PlaneF> & clipPlanes)
{
   expand = 0;
   S32 retMask = 0;
   Point3F minPoint, maxPoint;
   for(S32 i = 0; i < clipPlanes.size(); i++)
   {
      if(mask & (1 << i))
      {
         if(clipPlanes[i].x > 0)
         {
            maxPoint.x = max.x;
            minPoint.x = min.x;
         }
         else
         {
            maxPoint.x = min.x;
            minPoint.x = max.x;
         }
         if(clipPlanes[i].y > 0)
         {
            maxPoint.y = max.y;
            minPoint.y = min.y;
         }
         else
         {
            maxPoint.y = min.y;
            minPoint.y = max.y;
         }
         if(clipPlanes[i].z > 0)
         {
            maxPoint.z = max.z;
            minPoint.z = min.z;
         }
         else
         {
            maxPoint.z = min.z;
            minPoint.z = max.z;
         }
         F32 maxDot = mDot(maxPoint, clipPlanes[i]);
         F32 minDot = mDot(minPoint, clipPlanes[i]);
         F32 planeD = clipPlanes[i].d;
         if(maxDot <= -(planeD + expand))
            return(U16(-1));
         if(minDot <= -planeD)
            retMask |= (1 << i);
      }
   }
   return(retMask);
}

bool blTerrainProxy::getShadowedSquares(const Vector<PlaneF> & clipPlanes, Vector<U16> & shadowList)
{
   TerrainBlock *terrain = getObject();
   if ( !terrain )
      return false;

   // TODO: Fix me for variable terrain sizes!
   return true;

   /*
   SquareStackNode stack[TerrainBlock::BlockShift * 4];

   stack[0].mLevel = TerrainBlock::BlockShift;
   stack[0].mClipFlags = 0xff;
   stack[0].mPos.set(0,0);

   U32 stackSize = 1;

   Point3F blockPos;
   terrain->getTransform().getColumn(3, &blockPos);
   S32 squareSize = terrain->getSquareSize();
   F32 floatSquareSize = (F32)squareSize;

   bool marked = false;

   // push through all the levels of the quadtree
   while(stackSize)
   {
      SquareStackNode * node = &stack[stackSize - 1];

      S32 clipFlags = node->mClipFlags;
      Point2I pos = node->mPos;
      GridSquare * sq = terrain->findSquare(node->mLevel, pos);

      Point3F minPoint, maxPoint;
      minPoint.set(squareSize * pos.x + blockPos.x,
         squareSize * pos.y + blockPos.y,
         fixedToFloat(sq->minHeight));
      maxPoint.set(minPoint.x + (squareSize << node->mLevel),
         minPoint.y + (squareSize << node->mLevel),
         fixedToFloat(sq->maxHeight));

      // test the square against the current level
      if(clipFlags)
      {
         clipFlags = testSquare(minPoint, maxPoint, clipFlags, floatSquareSize, clipPlanes);
         if(clipFlags == U16(-1))
         {
            stackSize--;
            continue;
         }
      }

      // shadowed?
      if(node->mLevel == 0)
      {
         marked = true;
         shadowList.push_back(pos.x + (pos.y << TerrainBlock::BlockShift));
         stackSize--;
         continue;
      }

      // setup the next level of squares
      U8 nextLevel = node->mLevel - 1;
      S32 squareHalfSize = 1 << nextLevel;

      for(U32 i = 0; i < 4; i++)
      {
         node[i].mLevel = nextLevel;
         node[i].mClipFlags = clipFlags;
      }

      node[3].mPos = pos;
      node[2].mPos.set(pos.x + squareHalfSize, pos.y);
      node[1].mPos.set(pos.x, pos.y + squareHalfSize);
      node[0].mPos.set(pos.x + squareHalfSize, pos.y + squareHalfSize);

      stackSize += 3;
   }

   return marked;
   */
}

void blTerrainProxy::lightVector(LightInfo * light)
{
   // Grab our terrain object
   TerrainBlock* terrain = getObject();
   if (!terrain)
      return;

   // Get the direction to the light (the inverse of the direction
   // the light is pointing)
   Point3F lightDir = -light->getDirection();
   lightDir.normalize();

   // Get the ratio between the light map pixel and world space (used below)   
   F32 lmTerrRatio = (F32)mTerrainBlockSize / (F32) mLightMapSize;
   lmTerrRatio *= terrain->getSquareSize();

   U32 i = 0;
   for (U32 y = 0; y < mLightMapSize; y++)
   {
      for (U32 x = 0; x < mLightMapSize; x++)
      {
         // Get the relative pixel position and scale it
         // by the ratio between lightmap and world space
         Point2F pixelPos(x, y);
         pixelPos *= lmTerrRatio;         
         
         // Start with a default normal of straight up
         Point3F normal(0.0f, 0.0f, 1.0f);
         
         // Try to get the actual normal from the terrain.
         // Note: this won't change the default normal if
         // it can't find a normal.
         terrain->getNormal(pixelPos, &normal);

         // The terrain lightmap only contains shadows.
         F32 shadowed = 0.0f;

         // Get the height at the lightmap pixel's position
         F32 height = 0.0f;
         terrain->getHeight(pixelPos, &height);

         // Calculate the 3D position of the pixel
         Point3F pixelPos3F(pixelPos.x, pixelPos.y, height);

         // Translate that position by the terrain's transform
         terrain->getTransform().mulP(pixelPos3F);

         // Offset slighting along the normal so that we don't
         // raycast into ourself
         pixelPos3F += (normal * 0.1f);

         // Calculate the light's position.
         // If it is a vector light like the sun (no position
         // just direction) then translate along that direction
         // a reasonable distance to get a point sufficiently
         // far away
         Point3F lightPos = light->getPosition();
         if(light->getType() == LightInfo::Vector)
         {
            lightPos = 1000.f * lightDir;            
            lightPos = pixelPos3F + lightPos;
         }

         // Cast a ray from the world space position of the lightmap pixel to the light source.
         // If we hit something then we are in shadow. This allows us to be shadowed by anything
         // that supports a castRay operation.
         RayInfo info;
         if(terrain->getContainer()->castRay(pixelPos3F, lightPos, STATIC_COLLISION_TYPEMASK, &info))
         {
            // Shadow the pixel.
            shadowed = 1.0f;
         }

         // Set the final lightmap color.
         mLightmap[i++] += ColorF::WHITE * mClampF( 1.0f - shadowed, 0.0f, 1.0f );
      }
   }
}

//--------------------------------------------------------------------------
U32 blTerrainProxy::getResourceCRC()
{
   TerrainBlock * terrain = getObject();
   if(!terrain)
      return(0);
   return(terrain->getCRC());
}

//--------------------------------------------------------------------------
bool blTerrainProxy::setPersistInfo(PersistInfo::PersistChunk * info)
{
   if(!Parent::setPersistInfo(info))
      return(false);

   blTerrainChunk * chunk = dynamic_cast<blTerrainChunk*>(info);
   AssertFatal(chunk, "blTerrainProxy::setPersistInfo: invalid info chunk!");

   TerrainBlock * terrain = getObject();
   if(!terrain || !terrain->getLightMap())
      return(false);

   terrain->setLightMap( new GBitmap( *chunk->mLightmap) );

   return(true);
}

bool blTerrainProxy::getPersistInfo(PersistInfo::PersistChunk * info)
{
   if(!Parent::getPersistInfo(info))
      return(false);

   blTerrainChunk * chunk = dynamic_cast<blTerrainChunk*>(info);
   AssertFatal(chunk, "blTerrainProxy::getPersistInfo: invalid info chunk!");

   TerrainBlock * terrain = getObject();
   if(!terrain || !terrain->getLightMap())
      return(false);

   if(chunk->mLightmap) delete chunk->mLightmap;

   chunk->mLightmap = new GBitmap(*terrain->getLightMap());

   return(true);
}

bool blTerrainProxy::supportsShadowVolume()
{
   return false;
}

void blTerrainProxy::getClipPlanes(Vector<PlaneF>& planes)
{

}

void blTerrainProxy::addToShadowVolume(ShadowVolumeBSP * shadowVolume, LightInfo * light, S32 level)
{

}



void blTerrainSystem::init()
{
}

U32 blTerrainSystem::addObjectType()
{
   return TerrainObjectType;
}

SceneLighting::ObjectProxy* blTerrainSystem::createObjectProxy(SceneObject* obj, SceneLighting::ObjectProxyList* sceneObjects)
{
   if ((obj->getTypeMask() & TerrainObjectType) != 0)
      return new blTerrainProxy(obj);
   else
      return NULL;
}

PersistInfo::PersistChunk* blTerrainSystem::createPersistChunk(const U32 chunkType)
{
   if (chunkType == PersistInfo::PersistChunk::TerrainChunkType)
      return new blTerrainChunk();
   else
      return NULL;
}

bool blTerrainSystem::createPersistChunkFromProxy(SceneLighting::ObjectProxy* objproxy, PersistInfo::PersistChunk **ret)
{
   if (dynamic_cast<blTerrainProxy*>(objproxy) != NULL)
   {
      *ret = new blTerrainChunk();
      return true;
   } else {
      return NULL;
   }
}

// Given a ray, this will return the color from the lightmap of this object, return true if handled
bool blTerrainSystem::getColorFromRayInfo(const RayInfo & collision, ColorF& result) const
{
   TerrainBlock *terrain = dynamic_cast<TerrainBlock *>(collision.object);
   if (!terrain)
      return false;

   Point2F uv;
   F32 terrainlength = (F32)terrain->getBlockSize();
   Point3F pos = terrain->getPosition();
   uv.x = (collision.point.x - pos.x) / terrainlength;
   uv.y = (collision.point.y - pos.y) / terrainlength;

   // similar to x = x & width...
   uv.x = uv.x - F32(U32(uv.x));
   uv.y = uv.y - F32(U32(uv.y));
   const GBitmap* lightmap = terrain->getLightMap();
   if (!lightmap)
      return false;

   result = lightmap->sampleTexel(uv.x, uv.y);
   // terrain lighting is dim - look into this (same thing done in shaders)...
   result *= 2.0f;
   return true;
}
