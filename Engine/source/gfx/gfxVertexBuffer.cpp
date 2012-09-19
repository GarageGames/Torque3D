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
#include "gfx/gfxVertexBuffer.h"

#include "core/strings/stringFunctions.h"
#include "gfx/gfxDevice.h"


void GFXVertexBufferHandleBase::set(   GFXDevice *theDevice,
                                       U32 numVerts, 
                                       const GFXVertexFormat *vertexFormat,
                                       U32 vertexSize, 
                                       GFXBufferType type )
{
   StrongRefPtr<GFXVertexBuffer>::operator=( theDevice->allocVertexBuffer( numVerts, vertexFormat, vertexSize, type ) );
}

const String GFXVertexBuffer::describeSelf() const
{
   const char *bufType;

   switch(mBufferType)
   {
   case GFXBufferTypeStatic:
      bufType = "Static";
      break;
   case GFXBufferTypeDynamic:
      bufType = "Dynamic";
      break;
   case GFXBufferTypeVolatile:
      bufType = "Volatile";
      break;
   default:
      bufType = "Unknown";
      break;
   }

   return String::ToString("numVerts: %i vertSize: %i bufferType: %s", mNumVerts, mVertexSize, bufType);   
}