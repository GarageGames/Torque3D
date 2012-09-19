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

#ifndef _WORLDEDITORSELECTION_H_
#define _WORLDEDITORSELECTION_H_

#ifndef _SIMPERSISTSET_H_
   #include "console/simPersistSet.h"
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


class SceneObject;


/// A selection set in the World Editor.
///
/// Selections are by default transient objects, but can be made persistent and
/// saved to disk.
///
class WorldEditorSelection : public SimPersistSet
{
   public:
   
      typedef SimPersistSet Parent;

   private:
      
      /// The averaged positions of all objects in the selection.
      Point3F mCentroid;
      
      /// The center point of the bounding box around the selection.
      Point3F mBoxCentroid;
      
      /// The bounding box around the selection.
      Box3F mBoxBounds;
      
      ///
      MatrixF mTransform;
      
      /// If false, the selection has been modified and bounding box values
      /// and center points need to be recomputed.
      bool mCentroidValid;
      
      /// If true, the selection contains one or more objects that have
      /// global bounds enabled.
      bool mContainsGlobalBounds;
      
      bool mAutoSelect;
      Point3F mPrevCentroid;

      void updateCentroid();

   public:

      WorldEditorSelection();
      ~WorldEditorSelection();

      /// Return true if "object" is contained in the selection.
      bool objInSet( SimObject* object );

      void storeCurrentCentroid() { mPrevCentroid = getCentroid(); }
      bool hasCentroidChanged() { return (mPrevCentroid != getCentroid()); }

      bool containsGlobalBounds();
      
      /// @name Transforms
      ///
      /// Note that these methods do not update transforms of client objects.
      /// Use WorldEditor::updateClientTransforms to do that.
      ///
      /// @{

      /// 
      const Point3F& getCentroid();
      const Point3F& getBoxCentroid();
      const Box3F& getBoxBounds();
      Point3F getBoxBottomCenter();
      const MatrixF& getTransform();
      
      //
      void offset(const Point3F& delta, F32 gridSnap = 0.f );
      void setPosition(const Point3F & pos);
      void setCentroidPosition(bool useBoxCenter, const Point3F & pos);

      void orient(const MatrixF &, const Point3F &);
      void rotate(const EulerF &);
      void rotate(const EulerF &, const Point3F &);
      void setRotate(const EulerF &);

      void scale(const VectorF &);
      void scale(const VectorF &, const Point3F &);
      void setScale(const VectorF &);
      void setScale(const VectorF &, const Point3F &);

      void addSize(const VectorF &);
      void setSize(const VectorF &);

      /// @}

      /// Enable collision for all objects in the selection.
      void enableCollision();
      
      /// Disable collision for all objects in the selection.
      void disableCollision();
      
      //
      void setAutoSelect(bool b) { mAutoSelect = b; }
      void invalidateCentroid() { mCentroidValid = false; }
      
      // SimSet.
      virtual void addObject( SimObject* );
      virtual void removeObject( SimObject* );
      virtual void setCanSave( bool value );
      
      static void initPersistFields();
                  
      DECLARE_CONOBJECT( WorldEditorSelection );
      DECLARE_CATEGORY( "Editor World" );
      DECLARE_DESCRIPTION( "A selection set for the World Editor." );
};

#endif // !_WORLDEDITORSELECTION_H_
