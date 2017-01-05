#ifndef GL_CIRCULAR_VOLATILE_BUFFER_H
#define GL_CIRCULAR_VOLATILE_BUFFER_H

#include "gfx/gl/gfxGLDevice.h"
#include "gfx/gl/gfxGLUtils.h"

class GLFenceRange 
{
public:
   GLFenceRange() : mStart(0), mEnd(0), mSync(0)
   {         
     
   }

   ~GLFenceRange()
   {
      //the order of creation/destruction of static variables is indetermined... depends on detail of the build
      //looks like for some reason on windows + sdl + opengl the order make invalid / wrong the process TODO: Refactor -LAR
      //AssertFatal( mSync == 0, "");
   }

   void init(U32 start, U32 end)
   {  
      PROFILE_SCOPE(GFXGLQueryFence_issue);
      mStart = start;
      mEnd = end;
      mSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
   }

   bool checkOverlap(U32 start, U32 end) 
   {         
      if( mStart < end && start < mEnd )
         return true;

      return false;
   }

   void wait()
   {   
      PROFILE_SCOPE(GFXGLQueryFence_block);
      GLbitfield waitFlags = 0;
      GLuint64 waitDuration = 0;
      while( 1 ) 
      {
         GLenum waitRet = glClientWaitSync( mSync, waitFlags, waitDuration );
         if( waitRet == GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED ) 
         {
            break;
         }

         if( waitRet == GL_WAIT_FAILED ) 
         {
            AssertFatal(0, "GLSync failed.");
            break;
         }
         
         waitFlags = GL_SYNC_FLUSH_COMMANDS_BIT;
         waitDuration = scOneSecondInNanoSeconds;
      }     

      glDeleteSync(mSync);
      mSync = 0;
   }

   void swap( GLFenceRange &r )
   {
      GLFenceRange temp;
      temp = *this;
      *this = r;
      r = temp;
   }

protected:
   U32 mStart, mEnd;
   GLsync mSync;
   static const GLuint64 scOneSecondInNanoSeconds = 1000000000;

   GLFenceRange( const GLFenceRange &);
   GLFenceRange& operator=(const GLFenceRange &r)
   {
      mStart = r.mStart;
      mEnd = r.mEnd;
      mSync = r.mSync;
      return *this;
   }
};

class GLOrderedFenceRangeManager
{
public:

   ~GLOrderedFenceRangeManager( )
   {
      //the order of creation/destruction of static variables is indetermined... depends on detail of the build
      //looks like for some reason on windows + sdl + opengl the order make invalid / wrong the process TODO: Refactor -LAR
      //waitAllRanges( );
   }

   void protectOrderedRange( U32 start, U32 end )
   {
      mFenceRanges.increment();
      GLFenceRange &range = mFenceRanges.last();
      range.init( start, end );
   }

   void waitFirstRange( U32 start, U32 end )
   {
      if( !mFenceRanges.size() || !mFenceRanges[0].checkOverlap( start, end ) )
         return;
         
      mFenceRanges[0].wait();
      mFenceRanges.pop_front();
   }

   void waitOverlapRanges( U32 start, U32 end )
   {
      for( U32 i = 0; i < mFenceRanges.size(); ++i )
      {
         if( !mFenceRanges[i].checkOverlap( start, end ) )
            continue;
         
         mFenceRanges[i].wait();
         mFenceRanges.erase(i);
      }
   }

   void waitAllRanges()
   {
      for( int i = 0; i < mFenceRanges.size(); ++i )            
         mFenceRanges[i].wait();      

      mFenceRanges.clear();
   }

protected:
   Vector<GLFenceRange> mFenceRanges;
};

class GLCircularVolatileBuffer
{
public:
   GLCircularVolatileBuffer(GLuint binding) 
      : mBinding(binding), mBufferName(0), mBufferPtr(NULL), mBufferSize(0), mBufferFreePos(0), mCurrectUsedRangeStart(0)
   { 
      init();
   }

   ~GLCircularVolatileBuffer()
   {
      glDeleteBuffers(1, &mBufferName);
   }

   void init()
   {
      glGenBuffers(1, &mBufferName);

      PRESERVE_BUFFER( mBinding );
      glBindBuffer(mBinding, mBufferName);
     
      const U32 cSizeInMB = 10;
      mBufferSize = (cSizeInMB << 20);

      if( GFXGL->mCapabilities.bufferStorage )
      {      
         const GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
         glBufferStorage(mBinding, mBufferSize, NULL, flags);
         mBufferPtr = glMapBufferRange(mBinding, 0, mBufferSize, flags);
      }
      else
      {
         glBufferData(mBinding, mBufferSize, NULL, GL_DYNAMIC_DRAW);
      }
   }

   struct 
   {
      U32 mOffset, mSize;
   }_getBufferData;

   void lock(const U32 size, U32 offsetAlign, U32 &outOffset, void* &outPtr)
   {
      if( !size )
      {
         AssertFatal(0, "");
         outOffset = 0;
         outPtr = NULL;
      }

      mLockManager.waitFirstRange( mBufferFreePos, (mBufferFreePos + size)-1 );

      if( mBufferFreePos + size > mBufferSize )
      {         
         mUsedRanges.push_back( UsedRange( mBufferFreePos, mBufferSize-1 ) );
         mBufferFreePos = 0;
      }

      // force offset buffer align
      if( offsetAlign )
         mBufferFreePos = ( (mBufferFreePos/offsetAlign) + 1 ) * offsetAlign;

      outOffset = mBufferFreePos;

      if( GFXGL->mCapabilities.bufferStorage )
      {         
         outPtr = (U8*)(mBufferPtr) + mBufferFreePos; 
      }
      else if( GFXGL->glUseMap() )
      {
         PRESERVE_BUFFER( mBinding );
         glBindBuffer(mBinding, mBufferName);

         const GLbitfield access = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
         outPtr = glMapBufferRange(mBinding, outOffset, size, access);
      }
      else
      {
         _getBufferData.mOffset = outOffset;
         _getBufferData.mSize = size;

         outPtr = mFrameAllocator.lock( size );
      }      

      //set new buffer pos
      mBufferFreePos = mBufferFreePos + size;

      //align 4bytes
      mBufferFreePos = ( (mBufferFreePos/4) + 1 ) * 4;
   }

   void unlock()
   {
      if( GFXGL->mCapabilities.bufferStorage )
      {
         return;
      }
      else if( GFXGL->glUseMap() )
      {
         PRESERVE_BUFFER( mBinding );
         glBindBuffer(mBinding, mBufferName);

         glUnmapBuffer(mBinding);
      }
      else
      {
         PRESERVE_BUFFER( mBinding );
         glBindBuffer(mBinding, mBufferName);

         glBufferSubData( mBinding, _getBufferData.mOffset, _getBufferData.mSize, mFrameAllocator.getlockedPtr() );

         _getBufferData.mOffset = 0;
         _getBufferData.mSize = 0;

         mFrameAllocator.unlock();
      }
      
   }

   U32 getHandle() const { return mBufferName; }

   void protectUsedRange()
   {
      for( int i = 0; i < mUsedRanges.size(); ++i )
      {
         mLockManager.protectOrderedRange( mUsedRanges[i].start, mUsedRanges[i].end );
      }
      mUsedRanges.clear();

      if( mCurrectUsedRangeStart < mBufferFreePos )
      {
         mLockManager.protectOrderedRange( mCurrectUsedRangeStart, mBufferFreePos-1 );      
         mCurrectUsedRangeStart = mBufferFreePos;
      }
   }

protected:   

   GLuint mBinding;
   GLuint mBufferName;
   void *mBufferPtr;
   U32 mBufferSize;
   U32 mBufferFreePos;
   U32 mCurrectUsedRangeStart;

   GLOrderedFenceRangeManager mLockManager;
   FrameAllocatorLockableHelper mFrameAllocator;

   struct UsedRange
   {
      UsedRange(U32 _start = 0, U32 _end = 0)
         : start(_start), end(_end)
      {

      }
      U32 start, end;
   };
   Vector<UsedRange> mUsedRanges;
};


#endif
