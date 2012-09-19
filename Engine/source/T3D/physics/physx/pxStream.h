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

#ifndef _T3D_PHYSICS_PXSTREAM_H_
#define _T3D_PHYSICS_PXSTREAM_H_

#ifndef _PHYSX_H_
#include "T3D/physics/physX/px.h"
#endif
#ifndef _MEMSTREAM_H_
#include "core/stream/memStream.h"
#endif


class PxMemStream : public NxStream
{
public:
   
   PxMemStream();
	virtual ~PxMemStream();
			
   void resetPosition();

   // NxStream
	NxU8 readByte() const;
	NxU16	readWord() const;
	NxU32	readDword() const;
	float	readFloat()	const;
	double readDouble() const;
	void readBuffer( void *buffer, NxU32 size ) const;
	NxStream& storeByte( NxU8 b );
	NxStream& storeWord( NxU16 w );
	NxStream& storeDword( NxU32 d );
	NxStream& storeFloat( NxReal f );
	NxStream& storeDouble( NxF64 f );
	NxStream& storeBuffer( const void* buffer, NxU32 size );

protected:

   mutable MemStream mMemStream;
};


class PxConsoleStream : public NxUserOutputStream
{
protected:

   // NxUserOutputStream
   void reportError( NxErrorCode code, const char *message, const char* file, int line );
   NxAssertResponse reportAssertViolation( const char *message, const char *file, int line );
   void print( const char *message );

public:

   PxConsoleStream();
   ~PxConsoleStream();
};

#endif // _T3D_PHYSICS_PXSTREAM_H_