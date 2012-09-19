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

#include "T3DTransform.h"
#include "math/mPoint.h"
#include "math/mQuat.h"

//---------------------------------------------------------
// T3DTransform
//---------------------------------------------------------

void Transform3D::setParentTransform(Transform3D * parent)
{
   if (_parentTransform == parent)
      return;
   _flags |= Transform3D::ParentDirty;
   _parentTransform = parent;
   if (_dirtyListener != NULL)
      _dirtyListener->onTransformDirty();
}

bool Transform3D::hasObjectScale() const
{
   if (hasLocalScale())
      return true;

   // check all parent transforms except last for scale
   Transform3D * walk = _parentTransform;
   while (walk != NULL && walk->_parentTransform != NULL)
   {
      if (walk->hasLocalScale())
         return true;
      walk = walk->getParentTransform();
   }
   return false;
}

bool Transform3D::hasWorldScale() const
{
   if (hasLocalScale())
      return true;

   // check all parent transforms for scale
   Transform3D * walk = _parentTransform;
   while (walk != NULL)
   {
      if (walk->hasLocalScale())
         return true;
      walk = walk->getParentTransform();
   }
   return false;
}

void Transform3D::setWorldMatrix(const MatrixF & world)
{
   if (_parentTransform != NULL)
   {
      MatrixF parentMatrix;
      _parentTransform->getWorldMatrix(parentMatrix, true);
      MatrixF parentMatrixInv = parentMatrix;
      parentMatrixInv.inverse();
      MatrixF localMat;
      localMat = parentMatrixInv * world;
      setLocalMatrix(localMat);
   }
   else
      setLocalMatrix(world);
}

void Transform3D::setObjectMatrix(const MatrixF & objMatrix)
{
   if (_parentTransform != NULL)
   {
      MatrixF parentMatrix;
      _parentTransform->getObjectMatrix(parentMatrix, true);
      MatrixF parentMatrixInv = parentMatrix;
      parentMatrixInv.inverse();
      MatrixF localMat;
      localMat = parentMatrixInv * objMatrix;
      setLocalMatrix(localMat);
   }
   else
      setLocalMatrix(objMatrix);
}

bool Transform3D::isChildOf(Transform3D * parent, bool recursive) const
{
   if (_parentTransform != NULL)
   {
      if (_parentTransform == parent)
         return true;
      else if (recursive)
         return _parentTransform->isChildOf(parent, true);
      else
         return false;
   }
   else
   {
      return false;
   }
}

//---------------------------------------------------------
// Transform3DInPlace
//---------------------------------------------------------

Point3F Transform3DInPlace::getPosition() const
{
   return _position; 
}

void Transform3DInPlace::setPosition(const Point3F & position)
{
   _position = position;
   _flags |= Transform3D::LocalPositionDirty;
   if (_dirtyListener != NULL)
      _dirtyListener->onTransformDirty();
}

QuatF Transform3DInPlace::getRotation() const
{ 
   return _rotation; 
}

void Transform3DInPlace::setRotation(const QuatF & rotation)
{
   _rotation = rotation;
   _flags |= Transform3D::LocalRotationDirty;
   if (_dirtyListener != NULL)
      _dirtyListener->onTransformDirty();
}

Point3F Transform3DInPlace::getScale() const
{ 
   return _scale;
}

void Transform3DInPlace::setScale(const Point3F & scale)
{
   _scale = scale;
   _flags |= Transform3D::LocalScaleDirty;
   if (_dirtyListener != NULL)
      _dirtyListener->onTransformDirty();
}

void Transform3DInPlace::getWorldMatrix(MatrixF & worldMat, bool includeLocalScale) const
{
   if (_parentTransform == NULL)
      getLocalMatrix(worldMat, includeLocalScale);
   else
   {
      MatrixF localMat, parentMat;
      getLocalMatrix(localMat, includeLocalScale);
      _parentTransform->getWorldMatrix(parentMat, true);
      worldMat = parentMat * localMat;
   }
}

void Transform3DInPlace::getObjectMatrix(MatrixF & objectMat, bool includeLocalScale) const
{
   if (_parentTransform == NULL)
      objectMat = MatrixF::smIdentity;
   else if (_parentTransform->getParentTransform() == NULL)
      getLocalMatrix(objectMat, includeLocalScale);
   else
   {
      MatrixF localMat, parentMat;
      getLocalMatrix(localMat, includeLocalScale);
      _parentTransform->getObjectMatrix(parentMat, true);
      objectMat = parentMat * localMat;
   }
}

void Transform3DInPlace::getLocalMatrix(MatrixF & localMat, bool includeLocalScale) const
{
   _rotation.setMatrix(&localMat);
   localMat.setColumn(3,_position);
   if (includeLocalScale)
      localMat.scale(_scale);
}

void Transform3DInPlace::setLocalMatrix(const MatrixF & localMat)
{
   _rotation.set(localMat);
   _position = localMat.getPosition();
   _scale = localMat.getScale();
   _flags |= Transform3D::LocalScaleDirty | Transform3D::LocalRotationDirty | Transform3D::LocalPositionDirty;
   if (_dirtyListener != NULL)
      _dirtyListener->onTransformDirty();
}

//---------------------------------------------------------
// TSTransform3D
//---------------------------------------------------------

TSTransform3D::TSTransform3D(TSShapeInstance * si, S32 nodeIndex)
{
   _shapeInstance = si;
   _nodeIndex = nodeIndex;
   AssertFatal(_nodeIndex >= 0 && _nodeIndex < _shapeInstance->mNodeTransforms.size(), "TSTransform3D nodeIndex out of range");
}

Point3F TSTransform3D::getPosition() const
{
   if (!doHandleLocal())
   {
      MatrixF mat;
      return getTSLocal(mat).getPosition();
   }
   return _position;
}

void TSTransform3D::setPosition(const Point3F & position)
{
   setHandleLocal(true);
   _position = position;
   _flags |= Transform3D::LocalPositionDirty;
   if (_dirtyListener != NULL)
      _dirtyListener->onTransformDirty();
}

QuatF TSTransform3D::getRotation() const
{
   if (!doHandleLocal())
   {
      MatrixF mat;
      return QuatF(getTSLocal(mat));
   }
   return _rotation;
}
void TSTransform3D::setRotation(const QuatF & rotation)
{
   setHandleLocal(true);
   _rotation = rotation;
   _flags |= Transform3D::LocalRotationDirty;
   if (_dirtyListener != NULL)
      _dirtyListener->onTransformDirty();
}

Point3F TSTransform3D::getScale() const
{
   if (!doHandleLocal())
   {
      MatrixF mat;
      return getTSLocal(mat).getScale();
   }
   return _scale;
}
void TSTransform3D::setScale(const Point3F & scale)
{
   setHandleLocal(true);
   _scale = scale;
   _flags |= Transform3D::LocalScaleDirty;
   if (_dirtyListener != NULL)
      _dirtyListener->onTransformDirty();
}

void TSTransform3D::getWorldMatrix(MatrixF & worldMat, bool includeLocalScale) const
{
   _shapeInstance->animate();
   if (_parentTransform == NULL)
   {
      worldMat = _shapeInstance->mNodeTransforms[_nodeIndex];
   }
   else
   {
      MatrixF parentMat;
      _parentTransform->getWorldMatrix(parentMat, true);
      worldMat = parentMat * _shapeInstance->mNodeTransforms[_nodeIndex];
   }
}

void TSTransform3D::getObjectMatrix(MatrixF & objectMat, bool includeLocalScale) const
{
   if (_parentTransform == NULL)
      objectMat = MatrixF::smIdentity;
   else if (_parentTransform->getParentTransform() == NULL)
   {
      _shapeInstance->animate();
      objectMat = _shapeInstance->mNodeTransforms[_nodeIndex];
   }
   else
   {
      _shapeInstance->animate();

      MatrixF parentMat;
      _parentTransform->getObjectMatrix(parentMat, true);
      objectMat = parentMat * _shapeInstance->mNodeTransforms[_nodeIndex];
   }
}

void TSTransform3D::getLocalMatrix(MatrixF & localMat, bool includeLocalScale) const
{
   if (doHandleLocal())
   {
      _rotation.setMatrix(&localMat);
      localMat.setPosition(_position);
      if (includeLocalScale)
         localMat.scale(_scale);
   }
   else
   {
      _shapeInstance->animate();
      localMat = _shapeInstance->mNodeTransforms[_nodeIndex];
      if (!includeLocalScale && (_flags & Transform3D::LocalHasScale) != Transform3D::None)
      {
         // reverse any scale on matrix -- this is a inconvenient, but not a common case
         Point3F scale = localMat.getScale();
         scale.x = 1.0f / scale.x;
         scale.y = 1.0f / scale.y;
         scale.z = 1.0f / scale.z;
         localMat.scale(scale);
      }
   }
}

void TSTransform3D::setLocalMatrix(const MatrixF & localMatrix)
{
   setHandleLocal(true);
   _position = localMatrix.getPosition();
   _rotation.set(localMatrix);
   _scale = localMatrix.getScale();
   _flags |= Transform3D::LocalScaleDirty | Transform3D::LocalRotationDirty | Transform3D::LocalPositionDirty;
   if (_dirtyListener != NULL)
      _dirtyListener->onTransformDirty();
}

MatrixF & TSTransform3D::getTSLocal(MatrixF & mat) const
{
     _shapeInstance->animate();

      // if node has no parent, easy enough to just grab the matrix of the node
      int parentIdx = _shapeInstance->getShape()->nodes[_nodeIndex].parentIndex;
      if (parentIdx < 0)
      {
         return _shapeInstance->mNodeTransforms[_nodeIndex];
      }

      // has parent, local is transform from this node to parent so get local matrix the hard way
      MatrixF parentMat = _shapeInstance->mNodeTransforms[parentIdx];
      parentMat.inverse();
      mat = parentMat * _shapeInstance->mNodeTransforms[_nodeIndex];
      return mat;
}

void TSTransform3D::setHandleLocal(bool handleLocal)
{
   if (handleLocal == doHandleLocal())
      return;

   if (handleLocal)
   {
      _position = getPosition();
      _rotation = getRotation();
      _scale = getScale();
      _shapeInstance->setNodeAnimationState(_nodeIndex, 0, this);
   }
   else
      _shapeInstance->setNodeAnimationState(_nodeIndex, 0);
   _flags ^= TSTransform3D::HandleLocal;
}

void TSTransform3D::setNodeTransform(TSShapeInstance * si, S32 nodeIndex, MatrixF & localTransform)
{
   AssertFatal(si == _shapeInstance,"TSTransform3D hooked up to wrong shape.");
   getLocalMatrix(localTransform, true);
}
