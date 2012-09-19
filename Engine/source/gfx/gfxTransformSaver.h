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

#ifndef _GFX_GFXTRANSFORMSAVER_H_
#define _GFX_GFXTRANSFORMSAVER_H_

#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif


/// Helper class to store viewport and matrix stack state, and restore it
/// later.
///
/// When doing complex out-of-scene rendering, for instance, doing a 
/// render to texture operation that needs its own transform state, it
/// is very easy to nuke important rendering state, like the viewport
/// or the projection matrix stored in vertex shader constant zero.
///
/// This class simplifies save and cleanup of those properties. You can
/// either treat it as a stack helper, e.g.
///
/// @code
/// void myFunc()
/// {
///    GFXTransformSaver saver;
///
///    // Lots of nasty render state changes...
///
///    // Everything is magically cleaned up when saver is destructed!
/// }
/// @endcode
///
/// Or you can manually control when you do saves or restores:
///
/// @code
/// void myFunc()
/// {
///    GFXTransformSaver saver(false, false);
///
///    if(!somePrecondition)
///       return false;     // Note early out.
///
///    saver.save();
///
///    // Lots of nasty render state changes...
///
///    // If we had passed (false, true) to the constructor then it would
///    // clean up automagically for us; but we want to do it manually.
///    saver.restore();
/// }
/// @endcode
/// 
class GFXTransformSaver
{
protected:

   RectI   mSavedViewport;
   MatrixF mSavedProjectionMatrix, mSavedViewMatrix;
   bool    mHaveSavedData, mRestoreSavedDataOnDestruct;

public:

   /// Constructor - controls how data is saved.
   ///
   /// @param saveDataNow If true, indicates that saveData() should be called
   ///                    immediately. Otherwise, you can do it manually.
   ///
   /// @param restoreDataOnDestruct If true, indicates that restoreData() should
   ///                              be called on destruct. Otherwise, you'll
   ///                              have to do it manually.
   GFXTransformSaver(bool saveDataNow = true, bool restoreDataOnDestruct = true)
   {
      mHaveSavedData = false;

      if(saveDataNow)
         save();
 
      mRestoreSavedDataOnDestruct = restoreDataOnDestruct;
   }

   ~GFXTransformSaver()
   {
      if(mRestoreSavedDataOnDestruct)
         restore();
   }

   void save()
   {
      AssertFatal(mHaveSavedData==false, "GFXTransformSaver::saveData - can't save twice!");
      mSavedViewport         = GFX->getViewport();
      mSavedProjectionMatrix = GFX->getProjectionMatrix();
      mSavedViewMatrix       = GFX->getViewMatrix();
      GFX->pushWorldMatrix();

      // Note we have saved data!
      mHaveSavedData = true;
   }

   void restore()
   {
      AssertFatal(mHaveSavedData==true, "GFXTransformSaver::restoreData - no saved data to restore!");

      GFX->popWorldMatrix();
      GFX->setViewMatrix(mSavedViewMatrix);
      GFX->setProjectionMatrix(mSavedProjectionMatrix);
      GFX->setViewport(mSavedViewport);

      // Once we've restored we do not want to be able to restore again...
      mHaveSavedData = false;

      // And we don't want to restore on destruct!
      mRestoreSavedDataOnDestruct = false;
   }

   /// Returns the saved viewport.
   const RectI& getViewport() const { return mSavedViewport; }

   /// Returns the saved projection matrix.
   const MatrixF& getProjectionMatrix() const { return mSavedProjectionMatrix; }

   /// Returns the saved projection matrix.
   const MatrixF& getViewMatrix() const { return mSavedViewMatrix; }
};

#endif // _GFX_GFXTRANSFORMSAVER_H_