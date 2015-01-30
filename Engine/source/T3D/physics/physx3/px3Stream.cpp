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
#include "T3D/physics/physx3/px3Stream.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "core/strings/stringFunctions.h"


Px3MemOutStream::Px3MemOutStream() : mMemStream(1024)
{
}

Px3MemOutStream::~Px3MemOutStream()
{
}

physx::PxU32 Px3MemOutStream::write(const void *src, physx::PxU32 count)
{
	physx::PxU32 out=0;
	if(!mMemStream.write(count,src))
		return out;

	out  = count;
	return out;
}

Px3MemInStream::Px3MemInStream(physx::PxU8* data, physx::PxU32 length):mMemStream(length,data)
{
}

physx::PxU32 Px3MemInStream::read(void* dest, physx::PxU32 count)
{
	physx::PxU32 read =0;
	if(!mMemStream.read(count,dest))
		return read;

	read = count;
	return read;
}

void Px3MemInStream::seek(physx::PxU32 pos)
{
	mMemStream.setPosition(pos);
}

physx::PxU32 Px3MemInStream::getLength() const
{
	return mMemStream.getStreamSize();
}

physx::PxU32 Px3MemInStream::tell() const
{
	return mMemStream.getPosition();
}

Px3ConsoleStream::Px3ConsoleStream()
{
}

Px3ConsoleStream::~Px3ConsoleStream()
{
}

void Px3ConsoleStream::reportError( physx::PxErrorCode code, const char *message, const char* file, int line )
{
	UTF8 info[1024];
	dSprintf( info, 1024, "File: %s\nLine: %d\n%s", file, line, message );
	Platform::AlertOK( "PhysX Error", info );
	// Con::printf( "PhysX Error:\n   %s(%d) : %s\n", file, line, message );
}