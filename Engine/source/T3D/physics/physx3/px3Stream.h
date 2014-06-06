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

#ifndef _PX3STREAM_H_
#define _PX3STREAM_H_

#ifndef _PHYSX3_H_
#include "T3D/physics/physx3/px3.h"
#endif
#ifndef _MEMSTREAM_H_
#include "core/stream/memStream.h"
#endif


class Px3MemOutStream : public physx::PxOutputStream
{
public:
   
   Px3MemOutStream();
	virtual ~Px3MemOutStream();
			
   void resetPosition();

	virtual physx::PxU32  write(const void *src, physx::PxU32 count);
	physx::PxU32 getSize() const {return mMemStream.getStreamSize();}
	physx::PxU8* getData() const {return (physx::PxU8*)mMemStream.getBuffer();}

protected:

   mutable MemStream mMemStream;
};

class Px3MemInStream: public physx::PxInputData
{
	public:
		Px3MemInStream(physx::PxU8* data, physx::PxU32 length);
		virtual physx::PxU32 read(void* dest, physx::PxU32 count);
		physx::PxU32 getLength() const;
		virtual void seek(physx::PxU32 pos);
		virtual physx::PxU32 tell() const;
protected:
   mutable MemStream mMemStream;

	};

class Px3ConsoleStream : public physx::PxDefaultErrorCallback
{
protected:

  virtual void reportError( physx::PxErrorCode code, const char *message, const char* file, int line );

public:

   Px3ConsoleStream();
   virtual ~Px3ConsoleStream();
};

#endif // _PX3STREAM_H_
