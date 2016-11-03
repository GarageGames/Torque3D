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

#ifndef _REFLECTOR_H_
#define _REFLECTOR_H_

#ifndef _GFXCUBEMAP_H_
#include "gfx/gfxCubemap.h"
#endif
#ifndef _GFXTARGET_H_
#include "gfx/gfxTarget.h"
#endif
#ifndef _SIMDATABLOCK_H_
#include "console/simDatablock.h"
#endif
#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _MATHUTIL_FRUSTUM_H_
#include "math/util/frustum.h"
#endif

struct CameraQuery;
class Point2I;
class Frustum;
class SceneManager;
class SceneObject;
class GFXOcclusionQuery;


struct ReflectParams
{
   const CameraQuery *query;
   Point2I viewportExtent;
   Frustum culler;
   U32 startOfUpdateMs;
   S8 eyeId;
};


class ReflectorDesc : public SimDataBlock
{
   typedef SimDataBlock Parent;

public:

   ReflectorDesc();
   virtual ~ReflectorDesc();

   DECLARE_CONOBJECT( ReflectorDesc );

   static void initPersistFields();

   virtual void packData( BitStream *stream );
   virtual void unpackData( BitStream* stream );   
   virtual bool preload( bool server, String &errorStr );

   U32 texSize;   
   F32 nearDist;
   F32 farDist;
   U32 objectTypeMask;
   F32 detailAdjust;
   F32 priority;
   U32 maxRateMs;
   bool useOcclusionQuery;
   //U32 lastLodSize;
};


class ReflectorBase
{
public:

   ReflectorBase();
   virtual ~ReflectorBase();

   bool isEnabled() const { return mEnabled; }

   virtual void unregisterReflector();
   virtual F32 calcScore( const ReflectParams &params );
   virtual void updateReflection( const ReflectParams &params ) {}

   GFXOcclusionQuery* getOcclusionQuery() const { return mOcclusionQuery; }

   bool isOccluded() const { return mOccluded; }

   /// Returns true if this reflector is in the process of rendering.
   bool isRendering() const { return mIsRendering; }   

   /// Signifies that the query has not finished yet and a new query
   /// does not need to be submitted.
   bool mQueryPending;

protected:

   bool mEnabled;

   bool mIsRendering;

   GFXOcclusionQuery *mOcclusionQuery;

   bool mOccluded;   

   SceneObject *mObject;

   ReflectorDesc *mDesc;

public:

   // These are public because some of them
   // are exposed as fields.

   F32 score;
   U32 lastUpdateMs;
   

};

typedef Vector<ReflectorBase*> ReflectorList;


class CubeReflector : public ReflectorBase
{
   typedef ReflectorBase Parent;

public:

   CubeReflector();
   virtual ~CubeReflector() {}

   void registerReflector( SceneObject *inObject,
                           ReflectorDesc *inDesc );

   virtual void unregisterReflector();
   virtual void updateReflection( const ReflectParams &params );   

   GFXCubemap* getCubemap() const { return cubemap; }

   void updateFace( const ReflectParams &params, U32 faceidx );
   F32 calcFaceScore( const ReflectParams &params, U32 faceidx );

protected:

   GFXTexHandle depthBuff;
   GFXTextureTargetRef renderTarget;   
   GFXCubemapHandle  cubemap;
   U32 mLastTexSize;

   class CubeFaceReflector : public ReflectorBase
   {
      typedef ReflectorBase Parent;
      friend class CubeReflector;

   public:
      U32 faceIdx;
      CubeReflector *cube;

      virtual void updateReflection( const ReflectParams &params ) { cube->updateFace( params, faceIdx ); } 
      virtual F32 calcScore( const ReflectParams &params );
   };

   CubeFaceReflector mFaces[6];
};


class PlaneReflector : public ReflectorBase
{
   typedef ReflectorBase Parent;

public:

   PlaneReflector() 
   {
      refplane.set( Point3F(0,0,0), Point3F(0,0,1) );
      objectSpace = false;
      mLastTexSize = Point2I(0,0);
   }

   virtual ~PlaneReflector() {}

   void registerReflector( SceneObject *inObject,
                           ReflectorDesc *inDesc );

   virtual F32 calcScore( const ReflectParams &params );
   virtual void updateReflection( const ReflectParams &params ); 

   /// Set up the GFX matrices
   void setGFXMatrices( const MatrixF &camTrans );

   /// Set up camera matrix for a reflection on the plane
   MatrixF getCameraReflection( const MatrixF &camTrans );

   /// Oblique frustum clipping - use near plane of zbuffer as a clip plane
   MatrixF getFrustumClipProj( MatrixF &modelview );

protected:

   Point2I mLastTexSize;

   // The camera position at the last update.
   Point3F mLastPos;

   // The camera direction at the last update.
   VectorF mLastDir;

public:

   GFXTextureTargetRef reflectTarget;

   GFXTexHandle innerReflectTex[2]; /// < Textures we actually render to
   GFXTexHandle reflectTex; ///< Last texture we rendered to
   GFXTexHandle depthBuff;
   PlaneF refplane;
   bool objectSpace;
};

#endif // _REFLECTOR_H_