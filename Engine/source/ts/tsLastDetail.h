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

#ifndef _TSLASTDETAIL_H_
#define _TSLASTDETAIL_H_

#ifndef _MATHTYPES_H_
#include "math/mathTypes.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif
#ifndef _TSRENDERDATA_H_
#include "ts/tsRenderState.h"
#endif
#ifndef _GFXVERTEXFORMAT_H_
#include "gfx/gfxVertexFormat.h"
#endif
#ifndef _SIM_H_
#include "console/simObject.h"
#endif


class TSShape;
class TSRenderState;
class SceneRenderState;
class Material;
class BaseMatInstance;


/// The imposter state vertex format.
GFXDeclareVertexFormat( ImposterState )
{
   /// .xyz = imposter center
   /// .w = billboard corner... damn SM 2.0
   Point3F center;
   float corner;

   /// .x = scaled half size
   /// .y = alpha fade out
   float halfSize;
   float alpha;

   /// The rotation encoded as the up
   /// and right vectors... cross FTW.
   Point3F upVec;
   Point3F rightVec;
};


/// This neat little class renders the object to a texture so that when the object
/// is far away, it can be drawn as a billboard instead of a mesh.  This happens
/// when the model is first loaded as to keep the realtime render as fast as possible.
/// It also renders the model from a few different perspectives so that it would actually
/// pass as a model instead of a silly old billboard.  In other words, this is an imposter.
class TSLastDetail
{
protected:

   /// The shape which we're impostering.
   TSShape *mShape;

   /// This is the path of the object, which is
   /// where we'll be storing our cache for rendered imposters.
   String mCachePath;

   /// The shape detail level to capture into
   /// the imposters.
   S32 mDl;

   /// The bounding radius of the shape 
   /// used to size the billboard.
   F32 mRadius;

   /// The center offset for the bounding
   /// sphere used to render the imposter.
   Point3F mCenter;

   /// The square dimensions of each 
   /// captured imposter image.
   S32 mDim;

   /// The number steps around the equator of
   /// the globe at which we capture an imposter.
   U32 mNumEquatorSteps;

   /// The number of steps to go from equator to
   /// each polar region (0 means equator only) at
   /// which we capture an imposter.
   U32 mNumPolarSteps;   

   /// The angle in radians of sub-polar regions.
   F32 mPolarAngle;

   /// If true we captures polar images in the 
   /// imposter texture.
   bool mIncludePoles;

   /// The combined imposter state and corner data vertex
   /// format used for rendering with multiple streams. 
   GFXVertexFormat mImposterVertDecl;

   /// The material for this imposter.
   SimObjectPtr<Material> mMaterial;

   /// The material instance used to render this imposter.
   BaseMatInstance *mMatInstance;

   /// This is a global list of all the TSLastDetail
   /// objects in the system.
   static Vector<TSLastDetail*> smLastDetails;

   /// The maximum texture size for a billboard texture.
   static const U32 smMaxTexSize = 2048;

   /// This update actually regenerates the imposter images.
   void _update();

   ///
   void _validateDim();

   /// Helper which returns the imposter diffuse map path.
   String _getDiffuseMapPath() const { return mCachePath + ".imposter.dds"; }

   /// Helper which returns the imposter normal map path.
   String _getNormalMapPath() const { return mCachePath + ".imposter_normals.dds"; }

public:

   TSLastDetail(  TSShape *shape, 
                  const String &cachePath,
                  U32 numEquatorSteps, 
                  U32 numPolarSteps, 
                  F32 polarAngle, 
                  bool includePoles, 
                  S32 dl, 
                  S32 dim );

   ~TSLastDetail();

   /// Global preference for rendering imposters to shadows.
   static bool smCanShadow;

   /// Calls update on all TSLastDetail objects in the system.
   /// @see update()
   static void updateImposterImages( bool forceUpdate = false );

   /// Loads the imposter images by reading them from the disk
   /// or generating them if the TSShape is more recient than the
   /// cached imposter textures.
   ///
   /// This should not be called from within any rendering code.
   ///
   /// @param forceUpdate  If true the disk cache is invalidated and
   ///                     new imposter images are rendered.
   ///
   void update( bool forceUpdate = false );


   /// Internal function called from TSShapeInstance to 
   /// submit an imposter render instance.
   void render( const TSRenderState &rdata, F32 alpha );

   /// Returns the material instance used to render this imposter.
   BaseMatInstance* getMatInstance() const { return mMatInstance; }

   /// Helper function which deletes the cached imposter 
   /// texture files from disk.
   void deleteImposterCacheTextures();

   /// Returns the radius.
   /// @see mRadius
   F32 getRadius() const { return mRadius; }
};


#endif // _TSLASTDETAIL_H_


