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

#include "sfx/sfxMemoryStream.h"
#include "platform/typetraits.h"
#include "console/console.h"


SFXMemoryStream::SFXMemoryStream( const SFXFormat& format,
                                  SourceStreamType* stream,
                                  U32 numSamples )
   : IInputStreamFilter< U8, SourceStreamType* >( stream ),
     mFormat( format ),
     mNumSamplesTotal( numSamples ),
     mNumSamplesLeft( numSamples ),
     mCurrentPacket( NULL ),
     mCurrentPacketOffset( 0 )
{
}

void SFXMemoryStream::reset()
{
   if( dynamic_cast< IResettable* >( getSourceStream() ) )
   {
      reinterpret_cast< IResettable* >( getSourceStream() )->reset();
      
      if( mCurrentPacket )
         destructSingle( mCurrentPacket );
         
      mCurrentPacket = NULL;
      mCurrentPacketOffset = 0;
      mNumSamplesLeft = mNumSamplesTotal;
   }
   else
      Con::errorf( "SFXMemoryStream - cannot reset source stream" );
}

U32 SFXMemoryStream::read( U8* buffer, U32 length )
{
   U32 bufferOffset = 0;
   
   // Determine how much we're supposed to read.
   
   U32 numBytesToCopy = length;
   if( mNumSamplesLeft != U32_MAX )
      numBytesToCopy = getMin( length, mNumSamplesLeft * mFormat.getBytesPerSample() );
   numBytesToCopy -= numBytesToCopy % mFormat.getBytesPerSample();
      
   // Copy the data.
   
   U32 numBytesLeftToCopy = numBytesToCopy;
   while( numBytesLeftToCopy )
   {
      // If we have a current packet, use its data.
      
      if( mCurrentPacket )
      {
         U32 numBytesLeftInCurrentPacket = mCurrentPacket->size - mCurrentPacketOffset;
         
         // Copy data.
         
         if( numBytesLeftInCurrentPacket )
         {
            const U32 numBytesToCopy = getMin( numBytesLeftInCurrentPacket, numBytesLeftToCopy );
            dMemcpy( &buffer[ bufferOffset ], &mCurrentPacket->data[ mCurrentPacketOffset ], numBytesToCopy );
            
            bufferOffset                  += numBytesToCopy;
            mCurrentPacketOffset          += numBytesToCopy;
            numBytesLeftInCurrentPacket   -= numBytesToCopy;
            numBytesLeftToCopy            -= numBytesToCopy;
         }
         
         // Discard the packet if there's no data left.
         
         if( !numBytesLeftInCurrentPacket )
         {
            destructSingle( mCurrentPacket );
            mCurrentPacket = NULL;
            mCurrentPacketOffset = 0;
         }
      }
      else
      {
         // Read a new packet.
         
         if( !getSourceStream()->read( &mCurrentPacket, 1 ) )
            break;
      }
   }
   
   // Update count of remaining samples.
   
   U32 numBytesCopied = numBytesToCopy - numBytesLeftToCopy;
   if( mNumSamplesLeft != U32_MAX )
      mNumSamplesLeft -= ( numBytesCopied / mFormat.getBytesPerSample() );
      
   return numBytesCopied;
}
