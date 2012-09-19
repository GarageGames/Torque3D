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

#ifndef _T3DTRANSFORM_H_
#define _T3DTRANSFORM_H_

#include "ts/tsShapeInstance.h"
#include "math/mMatrix.h"

//---------------------------------------------------------
// T3DTransform
//---------------------------------------------------------

class Transform3D
{
public:

   class IDirtyListener
   {
   public:
      virtual void onTransformDirty() = 0;
   };

   // local/object/world matrix access

   void setWorldMatrix(const MatrixF & world);
   void setObjectMatrix(const MatrixF & objMatrix);
   virtual void setLocalMatrix(const MatrixF & localMatrix) = 0;

   virtual void getWorldMatrix(MatrixF & worldMatrix, bool includeLocalScale) const = 0;
   virtual void getObjectMatrix(MatrixF & objectMatrix, bool includeLocalScale) const = 0;
   virtual void getLocalMatrix(MatrixF & localMatrix, bool includeLocalScale) const = 0;

   MatrixF getWorldMatrix() const
   {
      MatrixF world;
      getWorldMatrix(world, true);
      return world;
   }

   MatrixF getObjectMatrix() const
   {
      MatrixF objMatrix;
      getObjectMatrix(objMatrix, true);
      return objMatrix;
   }

   MatrixF getLocalMatrix() const
   {
      MatrixF loc;
      getLocalMatrix(loc, true);
      return loc;
   }

   // local position/rotation/scale

   virtual Point3F getPosition() const = 0;
   virtual void setPosition(const Point3F & position) = 0;

   virtual QuatF getRotation() const = 0;
   virtual void setRotation(const QuatF & rotation) = 0;

   virtual Point3F getScale() const = 0;
   virtual void setScale(const Point3F & scale) = 0;

   // scale tests

   bool hasLocalScale() const
   {
      return (_flags & Transform3D::LocalHasScale) != Transform3D::None;
   }

   bool hasObjectScale() const;
   bool hasWorldScale() const;

   // parent/child methods

   Transform3D * getParentTransform() const
   { 
      return _parentTransform;
   }

   void setParentTransform(Transform3D * parent);

   IDirtyListener * getDirtyListener() const
   {
      return _dirtyListener;
   }

   void setDirtyListener(IDirtyListener * dirtyListener)
   {
      _dirtyListener = dirtyListener;
   }

   bool isChildOf(Transform3D * parent, bool recursive) const;

protected:

   enum TransformFlags
   {
      None = 0,
      LocalHasScale = 1 << 0,
      LocalPositionDirty = 1 << 1,
      LocalRotationDirty = 1 << 2,
      LocalScaleDirty = 1 << 3,
      LocalDirty = LocalPositionDirty | LocalRotationDirty | LocalScaleDirty,
      ParentDirty = 1 << 4,
      LastFlag = 1 << 4
   };

   Transform3D * _parentTransform;
   IDirtyListener * _dirtyListener;
   U32 _flags;
};

//---------------------------------------------------------
// Transform3DInPlace
//---------------------------------------------------------

class Transform3DInPlace : public Transform3D
{
public:

   Transform3DInPlace() : _position(0,0,0), _rotation(0,0,0,1), _scale(1,1,1)
   {
   }

   Point3F getPosition() const;
   void setPosition(const Point3F & position);

   QuatF getRotation() const;
   void setRotation(const QuatF & rotation);

   Point3F getScale() const;
   void setScale(const Point3F & scale);

   void getWorldMatrix(MatrixF & worldMat, bool includeLocalScale) const;
   void getObjectMatrix(MatrixF & objectMat, bool includeLocalScale) const;
   void getLocalMatrix(MatrixF & localMat, bool includeLocalScale) const;
   void setLocalMatrix(const MatrixF & localMat);

protected:

   Point3F _position;
   QuatF _rotation;
   Point3F _scale;
};

//---------------------------------------------------------
// TSTransform3D
//---------------------------------------------------------

class TSTransform3D : public Transform3D, public TSCallback
{
public:
   TSTransform3D(TSShapeInstance * si, S32 nodeIndex);

   Point3F getPosition() const;
   void setPosition(const Point3F & position);

   QuatF getRotation() const;
   void setRotation(const QuatF & rotation);

   Point3F getScale() const;
   void setScale(const Point3F & scale);

   void getWorldMatrix(MatrixF & worldMat, bool includeLocalScale) const;
   void getObjectMatrix(MatrixF & objectMat, bool includeLocalScale) const;
   void getLocalMatrix(MatrixF & localMat, bool includeLocalScale) const;
   void setLocalMatrix(const MatrixF & localMatrix);

   // Define TSCallback interface
   void setNodeTransform(TSShapeInstance * si, S32 nodeIndex, MatrixF & localTransform);

protected:

   enum TSTransformFlags
   {
      HandleLocal = Transform3D::LastFlag << 1,
      LastFlag = Transform3D::LastFlag << 1
   };

   bool doHandleLocal() const
   {
      return (_flags & (TransformFlags)TSTransform3D::HandleLocal) != Transform3D::None;
   }

   void setHandleLocal(bool handleLocal);
   MatrixF & getTSLocal(MatrixF & mat) const;

   TSShapeInstance * _shapeInstance;
   int _nodeIndex;

   Point3F _position;
   QuatF _rotation;
   Point3F _scale;
};

#endif // _T3DTRANSFORM_H_