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

#ifndef _SCENECAMERASTATE_H_
#define _SCENECAMERASTATE_H_

#ifndef _MATHUTIL_FRUSTUM_H_
#include "math/util/frustum.h"
#endif

#ifndef _MRECT_H_
#include "math/mRect.h"
#endif

#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif


/// An object that combines all the state that is relevant to looking into the
/// scene from a particular point of view.
class SceneCameraState
{
   protected:

      /// The screen-space viewport rectangle.
      RectI mViewport;

      /// The viewing frustum.
      Frustum mFrustum;

      /// The inverse of the frustum's transform stored here for caching.
      MatrixF mWorldViewMatrix;

      /// Actual head position (will be - eye pos)
      MatrixF mHeadWorldViewMatrix;

      /// The projection matrix.
      MatrixF mProjectionMatrix;

      /// World-space vector representing the view direction.
      Point3F mViewDirection;

      /// Internal constructor.
      SceneCameraState() {}

   public:

      /// Freeze the given viewing state.
      ///
      /// @param viewport Screen-space viewport rectangle.
      /// @param frustum Camera frustum.
      /// @param worldView World->view matrix.
      /// @param projection Projection matrix.
      SceneCameraState( const RectI& viewport, const Frustum& frustum, const MatrixF& worldView, const MatrixF& projection );

      /// Capture the view state from the current GFX state.
      static SceneCameraState fromGFX();

      ///
      static SceneCameraState fromGFXWithViewport( const RectI& viewport );

      /// Return the screen-space viewport rectangle.
      const RectI& getViewport() const { return mViewport; }

      /// Return the camera frustum.
      const Frustum& getFrustum() const { return mFrustum; }

      /// Return the view position.  This is a shortcut for getFrustum().getPosition().
      const Point3F& getViewPosition() const { return mFrustum.getPosition(); }

      /// Return the world-space view vector.
      const Point3F& getViewDirection() const { return mViewDirection; }

      /// Returns the world->view transform for the head (used to calculate various display metrics)
      const MatrixF& getHeadWorldViewMatrix() const { return mHeadWorldViewMatrix; }

      /// Return the view->world transform.  This is a shortcut for getFrustum().getTransform().
      const MatrixF& getViewWorldMatrix() const { return mFrustum.getTransform(); }

      /// Return the world->view transform.
      const MatrixF& getWorldViewMatrix() const { return mWorldViewMatrix; }

      /// Return the projection transform.
      const MatrixF& getProjectionMatrix() const { return mProjectionMatrix; }
};

#endif // !_SCENECAMERASTATE_H_
