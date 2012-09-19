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

#include "core/stream/stream.h"
#include "interior/interiorInstance.h"

#include "interior/interiorSubObject.h"
#include "interior/mirrorSubObject.h"


InteriorSubObject::InteriorSubObject()
{
   mInteriorInstance = NULL;
}

InteriorSubObject::~InteriorSubObject()
{
   mInteriorInstance = NULL;
}

InteriorSubObject* InteriorSubObject::readISO(Stream& stream)
{
   U32 soKey;
   stream.read(&soKey);

   InteriorSubObject* pObject = NULL;
   switch (soKey) {
     case MirrorSubObjectKey:
      pObject = new MirrorSubObject;
      break;

     default:
      Con::errorf(ConsoleLogEntry::General, "Bad key in subObject stream!");
      return NULL;
   };

   if (pObject) {
      bool readSuccess = pObject->_readISO(stream);
      if (readSuccess == false) {
         delete pObject;
         pObject = NULL;
      }
   }

   return pObject;
}

bool InteriorSubObject::writeISO(Stream& stream) const
{
   stream.write(getSubObjectKey());
   return _writeISO(stream);
}

bool InteriorSubObject::_readISO(Stream& stream)
{
   return (stream.getStatus() == Stream::Ok);
}

bool InteriorSubObject::_writeISO(Stream& stream) const
{
   return (stream.getStatus() == Stream::Ok);
}

const MatrixF& InteriorSubObject::getSOTransform() const
{
   static const MatrixF csBadMatrix(true);

   if (mInteriorInstance != NULL) {
      return mInteriorInstance->getTransform();
   } else {
      AssertWarn(false, "Returning bad transform for subobject");
      return csBadMatrix;
   }
}

const Point3F& InteriorSubObject::getSOScale() const
{
   return mInteriorInstance->getScale();
}

InteriorInstance* InteriorSubObject::getInstance()
{
   return mInteriorInstance;
}

void InteriorSubObject::noteTransformChange()
{
   //
}
