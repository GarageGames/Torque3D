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

#ifndef _OGGINPUTSTREAM_H_
#define _OGGINPUTSTREAM_H_

#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif
#ifndef _TYPETRAITS_H_
   #include "platform/typetraits.h"
#endif
#ifndef _PLATFORM_THREADS_MUTEX_H_
   #include "platform/threads/mutex.h"
#endif
#ifndef _THREADSAFEREFCOUNT_H_
   #include "platform/threads/threadSafeRefCount.h"
#endif
#include "ogg/ogg.h"


class Stream;
class OggInputStream;


/// Single substream in a multiplexed OGG stream.
class OggDecoder
{
   public:
   
      typedef void Parent;
      friend class OggInputStream;
           
   protected:
   
      /// The Ogg container stream.
      OggInputStream* mOggStream;
   
      /// The Ogg bitstream.
      ogg_stream_state mOggStreamState;
      
      /// Lock for synchronizing access to Ogg stream state.
      Mutex mMutex;
            
      /// Read the next packet in the stream.
      /// @return false if there is no next packet.
      bool _readNextPacket( ogg_packet* packet );
      
      ///
      bool _nextPacket();
      
      ///
      virtual bool _detect( ogg_page* startPage ) = 0;

      ///
      virtual bool _init() = 0;
      
      ///
      virtual bool _packetin( ogg_packet* packet ) = 0;
      
      ///
      void _setStartPage( ogg_page* startPage );
      
   public:
   
      ///
      OggDecoder( const ThreadSafeRef< OggInputStream >& stream );
      
      virtual ~OggDecoder();
      
      /// Return the serial number of the Ogg bitstream.
      U32 getStreamSerialNo() const { return mOggStreamState.serialno; }
      
      ///
      virtual const char* getName() const = 0;
};

/// A multiplexed OGG input stream feeding into stream decoders.
class OggInputStream : public ThreadSafeRefCount< OggInputStream >
{
   public:
   
      typedef void Parent;
      friend class OggDecoder; // _requestData
      
   protected:
   
      typedef OggDecoder* ( *Constructor )( const ThreadSafeRef< OggInputStream >& stream );
   
      template< typename T >
      struct _SpellItOutForGCC
      {
         static OggDecoder* _fn( const ThreadSafeRef< OggInputStream >& stream  )
         {
            return constructSingle< T* >( stream );
         }
      };
      
      ///
      bool mIsAtEnd;
      
      ///
      Stream* mStream;
      
      ///
      Vector< Constructor > mConstructors;
      
      ///
      Vector< OggDecoder* > mDecoders;

      ///
      ogg_sync_state mOggSyncState;
      
      ///
      Mutex mMutex;
      
      /// Pull the next page from the OGG stream.
      bool _pullNextPage( ogg_page* page );
      
      /// Push the given page to the attached decoder streams.
      void _pushNextPage( ogg_page* page );
      
      ///
      bool _requestData();
      
      ///
      void _freeDecoders();
            
   public:
   
      ///
      /// @note Ownership of "stream" is transferred to OggInputStream.
      OggInputStream( Stream* stream );
      
      ~OggInputStream();
   
      /// Register a decoder class with the stream.
      template< class T >
      void addDecoder()
      {
         mConstructors.push_back( &_SpellItOutForGCC< T >::_fn );
      }
      
      ///
      OggDecoder* getDecoder( const String& name ) const;
      
      ///
      bool init();
      
      ///
      bool isAtEnd();
};

#endif // !_OGGINPUTSTREAM_H_
