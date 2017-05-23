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
#include "ts/tsRenderState.h"


TSRenderState::TSRenderState()
   :  mState( NULL ),
      mCubemap( NULL ),
      mFadeOverride( 1.0f ),
      mNoRenderTranslucent( false ),
      mNoRenderNonTranslucent( false ),
      mMaterialHint( NULL ),
      mCuller( NULL ),
      mUseOriginSort( false ),
      mLightQuery( NULL ),
      mAccuTex( NULL ),
      mNodeTransforms( NULL ),
      mNodeTransformCount( 0 )
{
}

TSRenderState::TSRenderState( const TSRenderState &state )
   :  mState( state.mState ),
      mCubemap( state.mCubemap ),
      mFadeOverride( state.mFadeOverride ),
      mNoRenderTranslucent( state.mNoRenderTranslucent ),
      mNoRenderNonTranslucent( state.mNoRenderNonTranslucent ),
      mMaterialHint( state.mMaterialHint ),
      mCuller( state.mCuller ),
      mUseOriginSort( state.mUseOriginSort ),
      mLightQuery( state.mLightQuery ),
      mAccuTex( state.mAccuTex ),
      mNodeTransforms( state.mNodeTransforms ),
      mNodeTransformCount( state.mNodeTransformCount )
{
}
