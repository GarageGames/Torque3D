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

#ifndef _MACCARBVOLUME_H_
#define _MACCARBVOLUME_H_

#import "platformPOSIX/posixVolume.h"
#import "core/util/tVector.h"

class MacFileSystem;


/// File system change notifications on Mac.
class MacFileSystemChangeNotifier : public Torque::FS::FileSystemChangeNotifier
{
   public:
   
      typedef Torque::FS::FileSystemChangeNotifier Parent;
      
      struct Event;

   protected:   
   
      /// Array of FSEventStream events set up.  Uses pointers to dynamically
      /// allocated memory rather than allocating inline with the array to be
      /// able to pass around pointers without colliding with array reallocation.
      Vector< Event* > mEvents;
   
      // FileSystemChangeNotifier.
      virtual void internalProcessOnce();
      virtual bool internalAddNotification( const Torque::Path& dir );
      virtual bool internalRemoveNotification( const Torque::Path& dir );
         
   public:
   
      MacFileSystemChangeNotifier( MacFileSystem* fs );
      
      virtual ~MacFileSystemChangeNotifier();
};

/// Mac filesystem.
class MacFileSystem : public Torque::Posix::PosixFileSystem
{
   public:
   
      typedef Torque::Posix::PosixFileSystem Parent;
      
      MacFileSystem( String volume )
         : Parent( volume )
      {
         mChangeNotifier = new MacFileSystemChangeNotifier( this );
      }
};

#endif // !_MACCARBVOLUME_H_
