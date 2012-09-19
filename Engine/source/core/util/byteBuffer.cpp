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

#include "core/util/byteBuffer.h"


namespace Torque
{

class PrivateBBData
{
public:
   PrivateBBData()
      :  refCount( 1 ),
         dataSize( 0 ),
         data( NULL )
   {
   }

   U32   refCount;   ///< Reference count
   U32   dataSize;   ///< Length of buffer
   U8    *data;      ///< Our data buffer
};

//--------------------------------------

ByteBuffer::ByteBuffer()
{
   _data = new PrivateBBData;
   _data->dataSize = 0;
   _data->data = NULL;
}

ByteBuffer::ByteBuffer(U8 *dataPtr, U32 bufferSize)
{
   _data = new PrivateBBData;
   _data->dataSize = bufferSize;
   _data->data = new U8[bufferSize];

   dMemcpy( _data->data, dataPtr, bufferSize );
}

ByteBuffer::ByteBuffer(U32 bufferSize)
{
   _data = new PrivateBBData;
   _data->dataSize = bufferSize;
   _data->data = new U8[bufferSize];
}

ByteBuffer::ByteBuffer(const ByteBuffer &theBuffer)
{
   _data = theBuffer._data;
   _data->refCount++;
}

ByteBuffer  &ByteBuffer::operator=(const ByteBuffer &theBuffer)
{
   _data = theBuffer._data;
   _data->refCount++;

   return *this;
}

ByteBuffer::~ByteBuffer()
{
   if (!--_data->refCount)
   {
      delete [] _data->data;
      delete _data;

      _data = NULL;
   }
}

void ByteBuffer::setBuffer(U8 *dataPtr, U32 bufferSize, bool copyData)
{
   U8 *newData = dataPtr;

   if ( copyData )
   {
      newData = new U8[bufferSize];

      dMemcpy( newData, dataPtr, bufferSize );
   }

   delete [] _data->data;

   _data->data = newData;
   _data->dataSize = bufferSize;
}

void ByteBuffer::resize(U32 newBufferSize)
{
   U8    *newData = new U8[newBufferSize];

   U32   copyLen = getMin( newBufferSize, _data->dataSize );
   
   dMemcpy( newData, _data->data, copyLen );

   delete [] _data->data;

   _data->data = newData;
   _data->dataSize = newBufferSize;
}

void ByteBuffer::appendBuffer(const U8 *dataBuffer, U32 bufferSize)
{
   U32 start = _data->dataSize;
   resize(start + bufferSize);
   dMemcpy(_data->data + start, dataBuffer, bufferSize);
}

U32 ByteBuffer::getBufferSize() const
{
   return _data->dataSize;
}

U8 *ByteBuffer::getBuffer()
{
   return _data->data;
}

const U8 *ByteBuffer::getBuffer() const
{
   return _data->data;
}

ByteBuffer  ByteBuffer::getCopy() const
{
   return ByteBuffer( _data->data, _data->dataSize );
}

void ByteBuffer::clear()
{
   dMemset(_data->data, 0, _data->dataSize);
}


}
