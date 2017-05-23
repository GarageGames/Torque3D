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

#include "console/engineAPI.h"
#include "T3D/components/render/meshComponent.h"
#include "scene/sceneObject.h"
#include "math/mTransform.h"

DefineEngineMethod(MeshComponent, getShapeBounds, Box3F, (), ,
   "@brief Get the cobject we're in contact with.\n\n"

   "The controlling client is the one that will send moves to us to act on.\n"

   "@return the ID of the controlling GameConnection, or 0 if this object is not "
   "controlled by any client.\n"

   "@see GameConnection\n")
{
   return object->getShapeBounds();
}

DefineEngineMethod(MeshComponent, mountObject, bool,
   (SceneObject* objB, String node, TransformF txfm), (MatrixF::Identity),
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   if (objB)
   {
      //BUG: Unsure how it broke, but atm the default transform passed in here is rotated 180 degrees. This doesn't happen
      //for the SceneObject mountobject method. Hackish, but for now, just default to a clean MatrixF::Identity
      object->mountObjectToNode(objB, node, /*MatrixF::Identity*/txfm.getMatrix());
      return true;
   }
   return false;
}

DefineEngineMethod(MeshComponent, getNodeTransform, TransformF,
   (S32 node), (-1),
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   if (node != -1)
   {
      //BUG: Unsure how it broke, but atm the default transform passed in here is rotated 180 degrees. This doesn't happen
      //for the SceneObject mountobject method. Hackish, but for now, just default to a clean MatrixF::Identity
      //object->mountObjectToNode( objB, node, /*MatrixF::Identity*/txfm.getMatrix() );
      MatrixF mat = object->getNodeTransform(node);
      return mat;
   }

   return TransformF::Identity;
}

DefineEngineMethod(MeshComponent, getNodeEulerRot, EulerF,
   (S32 node, bool radToDeg), (-1, true),
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   if (node != -1)
   {
      //BUG: Unsure how it broke, but atm the default transform passed in here is rotated 180 degrees. This doesn't happen
      //for the SceneObject mountobject method. Hackish, but for now, just default to a clean MatrixF::Identity
      //object->mountObjectToNode( objB, node, /*MatrixF::Identity*/txfm.getMatrix() );
      MatrixF mat = object->getNodeTransform(node);

      EulerF eul = mat.toEuler();
      if (radToDeg)
         eul = EulerF(mRadToDeg(eul.x), mRadToDeg(eul.y), mRadToDeg(eul.z));

      return eul;
   }

   return EulerF(0, 0, 0);
}

DefineEngineMethod(MeshComponent, getNodePosition, Point3F,
   (S32 node), (-1),
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   if (node != -1)
   {
      //BUG: Unsure how it broke, but atm the default transform passed in here is rotated 180 degrees. This doesn't happen
      //for the SceneObject mountobject method. Hackish, but for now, just default to a clean MatrixF::Identity
      //object->mountObjectToNode( objB, node, /*MatrixF::Identity*/txfm.getMatrix() );
      MatrixF mat = object->getNodeTransform(node);

      return mat.getPosition();
   }

   return Point3F(0, 0, 0);
}

DefineEngineMethod(MeshComponent, getNodeByName, S32,
   (String nodeName), ,
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   if (!nodeName.isEmpty())
   {
      //BUG: Unsure how it broke, but atm the default transform passed in here is rotated 180 degrees. This doesn't happen
      //for the SceneObject mountobject method. Hackish, but for now, just default to a clean MatrixF::Identity
      //object->mountObjectToNode( objB, node, /*MatrixF::Identity*/txfm.getMatrix() );
      S32 node = object->getNodeByName(nodeName);

      return node;
   }

   return -1;
}

DefineEngineMethod(MeshComponent, changeMaterial, void, (U32 slot, const char* newMat), (0, ""),
   "@brief Change one of the materials on the shape.\n\n")
{
   object->changeMaterial(slot, newMat);
}