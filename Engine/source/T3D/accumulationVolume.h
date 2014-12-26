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

#ifndef _ACCUMULATIONVOLUME_H_
#define _ACCUMULATIONVOLUME_H_

#ifndef _SCENEPOLYHEDRALSPACE_H_
#include "scene/scenePolyhedralSpace.h"
#endif

#ifndef _MSILHOUETTEEXTRACTOR_H_
#include "math/mSilhouetteExtractor.h"
#endif

#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif

/// A volume in space that blocks visibility.
class AccumulationVolume : public ScenePolyhedralSpace
{
   public:

      typedef ScenePolyhedralSpace Parent;

   protected:

      typedef SilhouetteExtractorPerspective< PolyhedronType > SilhouetteExtractorType;

      /// Whether the volume's transform has changed and we need to recompute
      /// transform-based data.
      bool mTransformDirty;

      /// World-space points of the volume's polyhedron.
      Vector< Point3F > mWSPoints;

      /// Silhouette extractor when using perspective projections.
      SilhouetteExtractorType mSilhouetteExtractor;
      
      mutable Vector< SceneObject* > mVolumeQueryList;

      // Name (path) of the accumulation texture.
      String mTextureName;
      
      // SceneSpace.
      virtual void _renderObject( ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* overrideMat );

   public:

      GFXTexHandle mAccuTexture;

      AccumulationVolume();
      ~AccumulationVolume();

      // SimObject.
      DECLARE_CONOBJECT( AccumulationVolume );
      DECLARE_DESCRIPTION( "Allows objects in an area to have accumulation effect applied." );
      DECLARE_CATEGORY( "3D Scene" );

      virtual bool onAdd();
      virtual void onRemove();
      void inspectPostApply();
      void setTexture( const String& name );

      // Static Functions.
      static void consoleInit();
      static void initPersistFields();
      static Vector< SimObjectPtr<SceneObject> > smAccuObjects;
      static Vector< SimObjectPtr<AccumulationVolume> > smAccuVolumes;
      static void addObject(SimObjectPtr<SceneObject> object);
      static void removeObject(SimObjectPtr<SceneObject> object);
      static void refreshVolumes();
      static bool _setTexture( void *object, const char *index, const char *data );
      static void updateObject(SceneObject* object);

      // Network
      U32 packUpdate( NetConnection *, U32 mask, BitStream *stream );
      void unpackUpdate( NetConnection *, BitStream *stream );

      // SceneObject.
      virtual void buildSilhouette( const SceneCameraState& cameraState, Vector< Point3F >& outPoints );
      virtual void setTransform( const MatrixF& mat );
};

#endif // !_AccumulationVolume_H_
