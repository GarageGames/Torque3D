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

#include "console/console.h"
#include "platformX86UNIX/platformX86UNIX.h"
#include "platform/platformRedBook.h"
#include "core/strings/stringFunctions.h"

#if defined(__linux__)
#include <linux/cdrom.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#endif

#include <SDL.h>

class SDL_CD; // TODO SDL remove

class UnixRedBookDevice : public RedBookDevice
{
#if !defined(__FreeBSD__)
   private:
      S32 mDeviceId;
      SDL_CD *mCD;
      cdrom_volctrl mOriginalVolume;
      bool mVolumeInitialized;
#endif
      bool mPlaying;

      void openVolume();
      void closeVolume();
      void setLastError(const char *);

   public:
      UnixRedBookDevice();
      ~UnixRedBookDevice();

      bool open();
      bool close();
      bool play(U32);
      bool stop();
      bool getTrackCount(U32 *);
      bool getVolume(F32 *);
      bool setVolume(F32);

      bool isPlaying() { return mPlaying; }
      bool updateStatus();
      void setDeviceInfo(S32 deviceId, const char *deviceName);
};

//-------------------------------------------------------------------------------
// Class: UnixRedBookDevice
//-------------------------------------------------------------------------------
UnixRedBookDevice::UnixRedBookDevice()
{
#if !defined(__FreeBSD__)
   mVolumeInitialized = false;
   mDeviceId = -1;
   mDeviceName = NULL;
   mCD = NULL;
   mPlaying = false;
#endif	// !defined(__FreeBSD__)
}

//------------------------------------------------------------------------------
UnixRedBookDevice::~UnixRedBookDevice()
{
#if !defined(__FreeBSD__)
   close();
#endif	// !defined(__FreeBSD__)
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::updateStatus()
{
   return false; // TODO LINUX
}

//------------------------------------------------------------------------------
void UnixRedBookDevice::setDeviceInfo(S32 deviceId, const char *deviceName)
{
#if !defined(__FreeBSD__)
   mDeviceId = deviceId;
   mDeviceName = new char[dStrlen(deviceName) + 1];
   dStrcpy(mDeviceName, deviceName);
#endif	// !defined(__FreeBSD__)
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::open()
{
   return false; // TODO LINUX
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::close()
{
   return false; // TODO LINUX
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::play(U32 track)
{
   return false; // TODO LINUX
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::stop()
{
   return false; // TODO LINUX
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::getTrackCount(U32 * numTracks)
{
   return false; // TODO LINUX
}

template <class Type>
static inline Type max(Type v1, Type v2)
{
#if !defined(__FreeBSD__)
   if (v1 <= v2)
      return v2;
   else
      return v1;
#endif	// !defined(__FreeBSD__)
}
//------------------------------------------------------------------------------
bool UnixRedBookDevice::getVolume(F32 * volume)
{
#if !defined(__FreeBSD__)
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   if(!mVolumeInitialized)
   {
      setLastError("Volume failed to initialize");
      return(false);
   }

#if defined(__linux__)
   AssertFatal(0, "SDL CD not implemented");
   return true;
#else
   return(false);
#endif
#endif	// !defined(__FreeBSD__)
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::setVolume(F32 volume)
{
#if !defined(__FreeBSD__)
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   if(!mVolumeInitialized)
   {
      setLastError("Volume failed to initialize");
      return(false);
   }

#if defined(__linux__)
   AssertFatal(0, "SDL CD not implemented");
   return true;
#else
   return(false);
#endif
#endif	// !defined(__FreeBSD__)
}

//------------------------------------------------------------------------------
void UnixRedBookDevice::openVolume()
{
#if !defined(__FreeBSD__)
// Its unforunate that we have to do it this way, but SDL does not currently
// support setting CD audio volume
#if defined(__linux__)
   AssertFatal(0, "SDL CD not implemented");
#else
   setLastError("Volume failed to initialize");
#endif
#endif	// !defined(__FreeBSD__)
}

void UnixRedBookDevice::closeVolume()
{
#if !defined(__FreeBSD__)
   if(!mVolumeInitialized)
      return;

#if defined(__linux__)
   AssertFatal(0, "SDL CD not implemented");
#endif

   mVolumeInitialized = false;
#endif	// !defined(__FreeBSD__)
}

//------------------------------------------------------------------------------
void UnixRedBookDevice::setLastError(const char * error)
{
#if !defined(__FreeBSD__)
   RedBook::setLastError(error);
#endif	// !defined(__FreeBSD__)
}

//------------------------------------------------------------------------------
void InstallRedBookDevices()
{

}

//------------------------------------------------------------------------------
void PollRedbookDevices()
{
#if !defined(__FreeBSD__)
   UnixRedBookDevice *device = dynamic_cast<UnixRedBookDevice*>(RedBook::getCurrentDevice());

   if (device == NULL || !device->isPlaying())
      return;

   static const U32 PollDelay = 1000;

   static U32 lastPollTime = 0;
   U32 curTime = Platform::getVirtualMilliseconds();

   if (lastPollTime != 0 &&
      (curTime - lastPollTime) < PollDelay)
      return;

   lastPollTime = curTime;

   if (device->isPlaying())
   {
      device->updateStatus();
      if (!device->isPlaying())
         RedBook::handleCallback(RedBook::PlayFinished);
   }
#endif	// !defined(__FreeBSD__)
}
