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
#ifndef _MATRIXSET_H_
#define _MATRIXSET_H_

#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif
#ifndef _UTIL_DELEGATE_H_
#include "core/util/delegate.h"
#endif
#ifndef _MATRIXSETDELEGATES_H_
#include "math/util/matrixSetDelegateMethods.h"
#endif


dALIGN_BEGIN

class MatrixSet
{
   typedef Delegate<const MatrixF &()> MatrixEvalDelegate;
   enum _Transforms
   {
      ObjectToWorld = 0,   // World
      WorldToCamera,       // View
      CameraToScreen,      // Projection
      ObjectToScreen,      // World * View * Proj
      ObjectToCamera,      // World * View
      WorldToObject,       // World^-1
      CameraToWorld,       // View^-1
      CameraToObject,      // (World * View)^-1
      WorldToScreen,       // View * Projection
      SceneView,           // The View matrix for the SceneState
      SceneProjection,     // The Projection matrix for the SceneState
      NumTransforms,
   };

   MatrixF mTransform[NumTransforms];
   MatrixEvalDelegate mEvalDelegate[NumTransforms];

   const MatrixF *mViewSource;
   const MatrixF *mProjectionSource;

   MATRIX_SET_GET_VALUE(ObjectToWorld);
   MATRIX_SET_GET_VALUE(WorldToCamera);
   MATRIX_SET_GET_VALUE(CameraToScreen);
   MATRIX_SET_GET_VALUE(ObjectToCamera);
   MATRIX_SET_GET_VALUE(WorldToObject);
   MATRIX_SET_GET_VALUE(CameraToWorld);
   MATRIX_SET_GET_VALUE(ObjectToScreen);
   MATRIX_SET_GET_VALUE(CameraToObject);
   MATRIX_SET_GET_VALUE(WorldToScreen);
   MATRIX_SET_GET_VALUE(SceneView);
   MATRIX_SET_GET_VALUE(SceneProjection);

   MATRIX_SET_IS_INVERSE_OF(WorldToObject, ObjectToWorld);
   MATRIX_SET_IS_INVERSE_OF(CameraToWorld, WorldToCamera);

   MATRIX_SET_MULT_ASSIGN(WorldToCamera, ObjectToWorld, ObjectToCamera);
   MATRIX_SET_IS_INVERSE_OF(CameraToObject, ObjectToCamera);

   MATRIX_SET_MULT_ASSIGN(CameraToScreen, WorldToCamera, WorldToScreen);

   MATRIX_SET_MULT_ASSIGN(WorldToScreen, ObjectToWorld, ObjectToScreen);

public:
   MatrixSet();

   // Direct accessors
   inline const MatrixF &getObjectToWorld() const { return mTransform[ObjectToWorld]; }
   inline const MatrixF &getWorldToCamera() const { return mTransform[WorldToCamera]; }
   inline const MatrixF &getCameraToScreen() const { return mTransform[CameraToScreen]; }

   // Delegate driven, lazy-evaluation accessors
   inline const MatrixF &getWorldToScreen() const { return mEvalDelegate[WorldToScreen](); }
   inline const MatrixF &getWorldViewProjection() const { return mEvalDelegate[ObjectToScreen](); }
   inline const MatrixF &getWorldToObject() const { return mEvalDelegate[WorldToObject](); }   
   inline const MatrixF &getCameraToWorld() const { return mEvalDelegate[CameraToWorld](); }
   inline const MatrixF &getObjectToCamera() const { return mEvalDelegate[ObjectToCamera](); }
   inline const MatrixF &getCameraToObject() const { return mEvalDelegate[CameraToObject](); }

   // Assignment for the world/view/projection matrices
   inline void setWorld(const MatrixF &world)
   {
      mTransform[ObjectToWorld] = world;
      mEvalDelegate[WorldToObject].bind(this, &MatrixSet::MATRIX_SET_IS_INVERSE_OF_FN(WorldToObject, ObjectToWorld));
      mEvalDelegate[ObjectToScreen].bind(this, &MatrixSet::MATRIX_SET_MULT_ASSIGN_FN(WorldToScreen, ObjectToWorld, ObjectToScreen));
      mEvalDelegate[ObjectToCamera].bind(this, &MatrixSet::MATRIX_SET_MULT_ASSIGN_FN(WorldToCamera, ObjectToWorld, ObjectToCamera));
      mEvalDelegate[CameraToObject].bind(this, &MatrixSet::MATRIX_SET_IS_INVERSE_OF_FN(CameraToObject, ObjectToCamera));
   }

   inline void setView(const MatrixF &view)
   {
      if(&view == mViewSource)
         return;

      mViewSource = &view;
      mTransform[WorldToCamera] = view;
      mEvalDelegate[CameraToWorld].bind(this, &MatrixSet::MATRIX_SET_IS_INVERSE_OF_FN(CameraToWorld, WorldToCamera)); 
      mEvalDelegate[ObjectToScreen].bind(this, &MatrixSet::MATRIX_SET_MULT_ASSIGN_FN(WorldToScreen, ObjectToWorld, ObjectToScreen));
      mEvalDelegate[WorldToScreen].bind(this, &MatrixSet::MATRIX_SET_MULT_ASSIGN_FN(CameraToScreen, WorldToCamera, WorldToScreen));
      mEvalDelegate[ObjectToCamera].bind(this, &MatrixSet::MATRIX_SET_MULT_ASSIGN_FN(WorldToCamera, ObjectToWorld, ObjectToCamera));
      mEvalDelegate[CameraToObject].bind(this, &MatrixSet::MATRIX_SET_IS_INVERSE_OF_FN(CameraToObject, ObjectToCamera));
   }

   inline void setProjection(const MatrixF &projection)
   {
      if(&projection == mProjectionSource)
         return;

      mProjectionSource = &projection;
      mTransform[CameraToScreen] = projection;
      mEvalDelegate[ObjectToScreen].bind(this, &MatrixSet::MATRIX_SET_MULT_ASSIGN_FN(WorldToScreen, ObjectToWorld, ObjectToScreen));
      mEvalDelegate[WorldToScreen].bind(this, &MatrixSet::MATRIX_SET_MULT_ASSIGN_FN(CameraToScreen, WorldToCamera, WorldToScreen));
   }

   void setSceneView(const MatrixF &view)
   {
      mViewSource = NULL;
      setView(view);
      mViewSource = &mTransform[WorldToCamera];
      mTransform[SceneView] = view;
   }

   void setSceneProjection(const MatrixF &projection)
   {
      mProjectionSource = NULL;
      setProjection(projection);
      mProjectionSource = &mTransform[CameraToScreen];
      mTransform[SceneProjection] = projection;
   }

   void restoreSceneViewProjection()
   {
      mViewSource = NULL;
      mProjectionSource = NULL;
      setView(mTransform[SceneView]);
      setProjection(mTransform[SceneProjection]);
      mViewSource = &mTransform[WorldToCamera];
      mProjectionSource = &mTransform[CameraToScreen];
   }
}

dALIGN_END;

#endif // _MATRIXSET_H_
