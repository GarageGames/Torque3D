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

// Not needed on dedicated (SDL is not not linked against when dedicated)
#ifndef TORQUE_DEDICATED
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

#include <SDL/SDL.h>

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
#if !defined(__FreeBSD__)
   AssertFatal(mCD, "mCD is NULL");

   CDstatus status = SDL_CDStatus(mCD);
   if (status == CD_ERROR)
   {
      setLastError("Error accessing device");
      return(false);
   }
   else if (status == CD_TRAYEMPTY)
   {
      setLastError("CD tray empty");
      return false;
   }

   mPlaying = (status == CD_PLAYING);
   return true;
#endif	// !defined(__FreeBSD__)
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
#if !defined(__FreeBSD__)
    if(mAcquired)
    {
       setLastError("Device is already open.");
       return(false);
    }

    // open the device
    mCD = SDL_CDOpen(mDeviceId);
    if (mCD == NULL)
    {
       setLastError(SDL_GetError());
       return false;
    }

    mAcquired = true;

    openVolume();
    setLastError("");
    return(true);
#endif	// !defined(__FreeBSD__)
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::close()
{
#if !defined(__FreeBSD__)
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   stop();
   closeVolume();

   if (mCD != NULL)
   {
      SDL_CDClose(mCD);
      mCD = NULL;
   }

   mAcquired = false;
   setLastError("");
   return(true);
#endif	// !defined(__FreeBSD__)
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::play(U32 track)
{
#if !defined(__FreeBSD__)
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   U32 numTracks;
   if(!getTrackCount(&numTracks))
      return(false);

   if(track >= numTracks)
   {
      setLastError("Track index is out of range");
      return(false);
   }

   if (!updateStatus())
      return false;

   AssertFatal(mCD, "mCD is NULL");
   if (SDL_CDPlayTracks(mCD, track, 0, 1, 0) == -1)
   {
      setLastError(SDL_GetError());
      return false;
   }

   mPlaying = true;

   setLastError("");
   return(true);
#endif	// !defined(__FreeBSD__)
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::stop()
{
#if !defined(__FreeBSD__)
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   AssertFatal(mCD, "mCD is NULL");

   if (SDL_CDStop(mCD) == -1)
   {
      setLastError(SDL_GetError());
      return(false);
   }

   mPlaying = false;

   setLastError("");
   return(true);
#endif	// !defined(__FreeBSD__)
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::getTrackCount(U32 * numTracks)
{
#if !defined(__FreeBSD__)
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   if (!updateStatus())
      return false;

   AssertFatal(mCD, "mCD is NULL");
   *numTracks = mCD->numtracks;

   return(true);
#endif	// !defined(__FreeBSD__)
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
   AssertFatal(mCD, "mCD is NULL");

   setLastError("");
   cdrom_volctrl sysvol;
   if (ioctl(mCD->id, CDROMVOLREAD, &sysvol) == -1)
   {
      setLastError(strerror(errno));
      return(false);
   }
   U8 maxVol = max(sysvol.channel0, sysvol.channel1);
   // JMQTODO: support different left/right channel volumes?
   *volume = static_cast<F32>(maxVol) / 255.f;
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
   AssertFatal(mCD, "mCD is NULL");

   setLastError("");
   cdrom_volctrl sysvol;
   volume = volume * 255.f;
   if (volume > 255)
      volume = 255;
   if (volume < 0)
      volume = 0;
   sysvol.channel0 = sysvol.channel1 = static_cast<__u8>(volume);
   if (ioctl(mCD->id, CDROMVOLCTRL, &sysvol) == -1)
   {
      setLastError(strerror(errno));
      return(false);
   }
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
   AssertFatal(mCD, "mCD is NULL");

   setLastError("");

   if (ioctl(mCD->id, CDROMVOLREAD, &mOriginalVolume) == -1)
   {
      setLastError(strerror(errno));
      return;
   }

   mVolumeInitialized = true;
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
   AssertFatal(mCD, "mCD is NULL");

   setLastError("");

   if (ioctl(mCD->id, CDROMVOLCTRL, &mOriginalVolume) == -1)
   {
      setLastError(strerror(errno));
      return;
   }
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
#if !defined(__FreeBSD__)
   Con::printf("CD Audio Init:");
   if (SDL_InitSubSystem(SDL_INIT_CDROM) == -1)
   {
      Con::printf("   Unable to initialize CD Audio: %s", SDL_GetError());
      return;
   }

   S32 numDrives = SDL_CDNumDrives();
   if (numDrives == 0)
   {
      Con::printf("   No drives found.");
      return;
   }

   for (int i = 0; i < numDrives; ++i)
   {
      const char * deviceName = SDL_CDName(i);
      Con::printf("   Installing CD Audio device: %s", deviceName);

      UnixRedBookDevice * device = new UnixRedBookDevice;
      device->setDeviceInfo(i, deviceName);
      RedBook::installDevice(device);
   }

   Con::printf(" ");
#endif	// !defined(__FreeBSD__)
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
#endif
