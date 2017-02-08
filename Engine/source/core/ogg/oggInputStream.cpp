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

#include "core/ogg/oggInputStream.h"
#include "core/stream/stream.h"
#include "core/util/safeDelete.h"


//#define DEBUG_SPEW


//-----------------------------------------------------------------------------
//    OggDecoder implementation.
//-----------------------------------------------------------------------------

OggDecoder::OggDecoder( const ThreadSafeRef< OggInputStream >& stream )
   : mOggStream( stream )
{
}

OggDecoder::~OggDecoder()
{
   ogg_stream_clear( &mOggStreamState );
}

void OggDecoder::_setStartPage( ogg_page* startPage )
{
   ogg_stream_init( &mOggStreamState, ogg_page_serialno( startPage ) );
   ogg_stream_pagein( &mOggStreamState, startPage );   
}

bool OggDecoder::_readNextPacket( ogg_packet* packet )
{
   MutexHandle mutex;
   mutex.lock( &mMutex, true );
      
   while( 1 )
   {
      S32 result = ogg_stream_packetout( &mOggStreamState, packet );
      if( result == 0 )
      {
         if( !mOggStream->_requestData() )
            return false;
      }
      else if( result < 0 )
         return false;
      else
      {
         #ifdef DEBUG_SPEW
         Platform::outputDebugString( "[OggDecoder] read packet %i in %s (bytes: %i, bos: %s, eos: %s)",
            ( U32 ) packet->packetno,
            getName(),
            ( U32 ) packet->bytes,
            packet->b_o_s ? "1" : "0",
            packet->e_o_s ? "1" : "0" );
         #endif

         return true;
      }
   }
}

bool OggDecoder::_nextPacket()
{
   MutexHandle mutex;
   mutex.lock( &mMutex, true );
   
   ogg_packet packet;
   
   do
   {
      if( !_readNextPacket( &packet ) )
         return false;
   }
   while( !_packetin( &packet ) );
   
   return true;
}

//-----------------------------------------------------------------------------
//    OggInputStream implementation.
//-----------------------------------------------------------------------------

OggInputStream::OggInputStream( Stream* stream )
   : mIsAtEnd( false ),
     mStream( stream )
{
   ogg_sync_init( &mOggSyncState );
   
   VECTOR_SET_ASSOCIATION( mConstructors );
   VECTOR_SET_ASSOCIATION( mDecoders );
}

OggInputStream::~OggInputStream()
{
   _freeDecoders();
   ogg_sync_clear( &mOggSyncState );
   
   if( mStream )
      SAFE_DELETE( mStream );
}

OggDecoder* OggInputStream::getDecoder( const String& name ) const
{
   for( U32 i = 0; i < mDecoders.size(); ++ i )
      if( name.equal( mDecoders[ i ]->getName(), String::NoCase ) )
         return mDecoders[ i ];
         
   return NULL;
}

bool OggInputStream::isAtEnd()
{
   MutexHandle mutex;
   mutex.lock( &mMutex, true );
   
   return mIsAtEnd;
}

bool OggInputStream::init()
{
   if( !mStream->hasCapability( Stream::StreamPosition ) )
      return false;
      
   mStream->setPosition( 0 );
   
   // Read all beginning-of-stream pages and construct decoders
   // for all streams we recognize.
   
   while( 1 )
   {
      // Read next page.
      
      ogg_page startPage;
      _pullNextPage( &startPage );
         
      // If not a beginning-of-stream page, push it to the decoders
      // and stop reading headers.

      if( !ogg_page_bos( &startPage ) )
      {
         _pushNextPage( &startPage );
         break;
      }
         
      // Try the list of constructors for one that consumes
      // the page.
         
      for( U32 i = 0; i < mConstructors.size(); ++ i )
      {
         OggDecoder* decoder = mConstructors[ i ]( this );
         if( decoder->_detect( &startPage ) )
            mDecoders.push_back( decoder );
         else
            delete decoder;
      }
   }
   
   // Initialize decoders and let all them finish up header processing.
   
   for( U32 i = 0; i < mDecoders.size(); ++ i )
      if( !mDecoders[ i ]->_init() )
      {
         delete mDecoders[ i ];
         mDecoders.erase( i );
         -- i;
      }
   
   if( !mDecoders.size() )
      return false;
      
   return true;
}

void OggInputStream::_freeDecoders()
{
   for( U32 i = 0; i < mDecoders.size(); ++ i )
      delete mDecoders[ i ];
   mDecoders.clear();
}

bool OggInputStream::_pullNextPage( ogg_page* page)
{   
   // Read another page.
   
   while( ogg_sync_pageout( &mOggSyncState, page ) != 1 )
   {
      enum { BUFFER_SIZE = 4096 };
      
      // Read more data.
      
      char* buffer = ogg_sync_buffer( &mOggSyncState, BUFFER_SIZE );
      const U32 oldPos = mStream->getPosition();
      mStream->read( BUFFER_SIZE, buffer );
         
      const U32 numBytes = mStream->getPosition() - oldPos;
      if( numBytes )
         ogg_sync_wrote( &mOggSyncState, numBytes );
      else
         return false;
   }
   
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[OggInputStream] pulled next page (header: %i, body: %i)",
      page->header_len, page->body_len );
   #endif
   
   return true;
}

void OggInputStream::_pushNextPage( ogg_page* page )
{
   for( U32 i = 0; i < mDecoders.size(); ++ i )
   {
      MutexHandle mutex;
      mutex.lock( &mDecoders[ i ]->mMutex, true );
      
      ogg_stream_pagein( &mDecoders[ i ]->mOggStreamState, page );
   }
}

bool OggInputStream::_requestData()
{
   // Lock at this level to ensure correct ordering of page writes.
   // Technically, the proper place to lock would be _pullNextPage
   // but then it could happen that one thread pushes a page before
   // another thread gets to push a page that has been read earlier.
   
   MutexHandle mutex;
   mutex.lock( &mMutex, true );

   ogg_page nextPage;
   
   if( !_pullNextPage( &nextPage ) )
   {
      mIsAtEnd = true;
      return false;
   }
      
   _pushNextPage( &nextPage );
   return true;
}
