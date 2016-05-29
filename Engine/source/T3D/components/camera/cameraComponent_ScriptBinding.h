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
#include "T3D/components/camera/cameraComponent.h"

//Basically, this only exists for backwards compatibility for parts of the editors
ConsoleMethod(CameraComponent, getMode, const char*, 2, 2, "() - We get the first behavior of the requested type on our owner object.\n"
   "@return (string name) The type of the behavior we're requesting")
{
   return "fly";
}

DefineConsoleMethod(CameraComponent, getForwardVector, VectorF, (), ,
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   F32 pos = 0;
   MatrixF cameraMat;
   object->getCameraTransform(&pos, &cameraMat);

   VectorF returnVec = cameraMat.getForwardVector();
   returnVec = VectorF(mRadToDeg(returnVec.x), mRadToDeg(returnVec.y), mRadToDeg(returnVec.z));
   returnVec.normalize();
   return returnVec;
}

DefineConsoleMethod(CameraComponent, getRightVector, VectorF, (), ,
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   F32 pos = 0;
   MatrixF cameraMat;
   object->getCameraTransform(&pos, &cameraMat);

   VectorF returnVec = cameraMat.getRightVector();
   returnVec = VectorF(mRadToDeg(returnVec.x), mRadToDeg(returnVec.y), mRadToDeg(returnVec.z));
   returnVec.normalize();
   return returnVec;
}

DefineConsoleMethod(CameraComponent, getUpVector, VectorF, (), ,
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   F32 pos = 0;
   MatrixF cameraMat;
   object->getCameraTransform(&pos, &cameraMat);

   VectorF returnVec = cameraMat.getUpVector();
   returnVec = VectorF(mRadToDeg(returnVec.x), mRadToDeg(returnVec.y), mRadToDeg(returnVec.z));
   returnVec.normalize();
   return returnVec;
}

DefineConsoleMethod(CameraComponent, setForwardVector, void, (VectorF newForward), (VectorF(0, 0, 0)),
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   object->setForwardVector(newForward);
}

DefineConsoleMethod(CameraComponent, getWorldPosition, Point3F, (), ,
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   F32 pos = 0;
   MatrixF mat;
   object->getCameraTransform(&pos, &mat);

   return mat.getPosition();
}