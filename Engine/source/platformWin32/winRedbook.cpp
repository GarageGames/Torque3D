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

#include "platformWin32/platformWin32.h"
#include "platform/platformRedBook.h"
#include "core/strings/unicode.h"
#include "core/strings/stringFunctions.h"

class Win32RedBookDevice : public RedBookDevice
{
   private:
      typedef RedBookDevice   Parent;

      U32         mDeviceId;

      void setLastError(const char *);
      void setLastError(U32);

      MIXERCONTROLDETAILS           mMixerVolumeDetails;
      MIXERCONTROLDETAILS_UNSIGNED  mMixerVolumeValue;

	  union {
		HMIXEROBJ mVolumeDeviceId;
		UINT mAuxVolumeDeviceId;
	  };

      U32   mOriginalVolume;
      bool  mVolumeInitialized;

      bool  mUsingMixer;

      void openVolume();
      void closeVolume();

   public:
      Win32RedBookDevice();
      ~Win32RedBookDevice();

      U32 getDeviceId();

      bool open();
      bool close();
      bool play(U32);
      bool stop();
      bool getTrackCount(U32 *);
      bool getVolume(F32 *);
      bool setVolume(F32);
};

//------------------------------------------------------------------------------
// Win32 specific
//------------------------------------------------------------------------------
void installRedBookDevices()
{
   U32 bufSize = ::GetLogicalDriveStrings(0,0);

   char * buf = new char[bufSize];

   ::GetLogicalDriveStringsA(bufSize, buf);

   char * str = buf;
   while(*str)
   {
      if(::GetDriveTypeA(str) == DRIVE_CDROM)
      {
         Win32RedBookDevice * device = new Win32RedBookDevice;
         device->mDeviceName = new char[dStrlen(str) + 1];
         dStrcpy(device->mDeviceName, str);

         RedBook::installDevice(device);
      }
      str += dStrlen(str) + 1;
   }

   delete [] buf;
}

void handleRedBookCallback(U32 code, U32 deviceId)
{
   if(code != MCI_NOTIFY_SUCCESSFUL)
      return;

   Win32RedBookDevice * device = dynamic_cast<Win32RedBookDevice*>(RedBook::getCurrentDevice());
   if(!device)
      return;

   if(device->getDeviceId() != deviceId)
      return;

   // only installed callback on play (no callback if play is aborted)
   RedBook::handleCallback(RedBook::PlayFinished);
}

//------------------------------------------------------------------------------
// Class: Win32RedBookDevice
//------------------------------------------------------------------------------
Win32RedBookDevice::Win32RedBookDevice()
{
   mVolumeInitialized = false;
}

Win32RedBookDevice::~Win32RedBookDevice()
{
   close();
}

U32 Win32RedBookDevice::getDeviceId()
{
   return(mDeviceId);
}

bool Win32RedBookDevice::open()
{
   if(mAcquired)
   {
      setLastError("Device is already open.");
      return(false);
   }

   U32 error;

   // open the device
   MCI_OPEN_PARMS openParms;
#ifdef UNICODE
   openParms.lpstrDeviceType = (LPCWSTR)MCI_DEVTYPE_CD_AUDIO;

   UTF16 buf[512];
   convertUTF8toUTF16((UTF8 *)mDeviceName, buf);
   openParms.lpstrElementName = buf;
#else
   openParms.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_CD_AUDIO;
   openParms.lpstrElementName = mDeviceName;
#endif

   error = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID, (DWORD_PTR)(LPMCI_OPEN_PARMS)&openParms);
   if(error)
   {
      // attempt to open as a shared device
      error = mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID|MCI_OPEN_SHAREABLE, (DWORD_PTR)(LPMCI_OPEN_PARMS)&openParms);
      if(error)
      {
         setLastError(error);
         return(false);
      }
   }

   // set time mode to milliseconds
   MCI_SET_PARMS setParms;
   setParms.dwTimeFormat = MCI_FORMAT_MILLISECONDS;

   error = mciSendCommand(openParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)(LPMCI_SET_PARMS)&setParms);
   if(error)
   {
      setLastError(error);
      return(false);
   }

   //
   mDeviceId = openParms.wDeviceID;
   mAcquired = true;

   openVolume();
   setLastError("");
   return(true);
}

bool Win32RedBookDevice::close()
{
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   stop();

   U32 error;

   MCI_GENERIC_PARMS closeParms;
   error = mciSendCommand(mDeviceId, MCI_CLOSE, 0, (DWORD_PTR)(LPMCI_GENERIC_PARMS)&closeParms);
   if(error)
   {
      setLastError(error);
      return(false);
   }

   mAcquired = false;
   closeVolume();
   setLastError("");
   return(true);
}

bool Win32RedBookDevice::play(U32 track)
{
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

   MCI_STATUS_PARMS statusParms;

   // get track start time
   statusParms.dwItem = MCI_STATUS_POSITION;
   statusParms.dwTrack = track + 1;

   U32 error;
   error = mciSendCommand(mDeviceId, MCI_STATUS, MCI_STATUS_ITEM|MCI_TRACK|MCI_WAIT,
      (DWORD_PTR)(LPMCI_STATUS_PARMS)&statusParms);

   if(error)
   {
      setLastError(error);
      return(false);
   }

   MCI_PLAY_PARMS playParms;
   playParms.dwFrom = statusParms.dwReturn;

   // get track end time
   statusParms.dwItem = MCI_STATUS_LENGTH;
   error = mciSendCommand(mDeviceId, MCI_STATUS, MCI_STATUS_ITEM|MCI_TRACK|MCI_WAIT,
      (DWORD_PTR)(LPMCI_STATUS_PARMS)&statusParms);

   if(error)
   {
      setLastError(error);
      return(false);
   }

   playParms.dwTo = playParms.dwFrom + statusParms.dwReturn;

   // play the track
   playParms.dwCallback = MAKELONG(getWin32WindowHandle(), 0);
   error = mciSendCommand(mDeviceId, MCI_PLAY, MCI_FROM|MCI_TO|MCI_NOTIFY,
      (DWORD_PTR)(LPMCI_PLAY_PARMS)&playParms);

   if(error)
   {
      setLastError(error);
      return(false);
   }

   setLastError("");
   return(true);
}

bool Win32RedBookDevice::stop()
{
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   MCI_GENERIC_PARMS genParms;

   U32 error = mciSendCommand(mDeviceId, MCI_STOP, 0, (DWORD_PTR)(LPMCI_GENERIC_PARMS)&genParms);
   if(error)
   {
      setLastError(error);
      return(false);
   }

   setLastError("");
   return(true);
}

bool Win32RedBookDevice::getTrackCount(U32 * numTracks)
{
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   MCI_STATUS_PARMS statusParms;

   statusParms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
   U32 error = mciSendCommand(mDeviceId, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD_PTR)(LPMCI_STATUS_PARMS)&statusParms);
   if(error)
   {
      setLastError(error);
      return(false);
   }

   *numTracks = statusParms.dwReturn;
   return(true);
}

bool Win32RedBookDevice::getVolume(F32 * volume)
{
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

   U32 vol = 0;
   if(mUsingMixer)
   {
      mixerGetControlDetails(mVolumeDeviceId, &mMixerVolumeDetails, MIXER_GETCONTROLDETAILSF_VALUE);
      vol = mMixerVolumeValue.dwValue;
   }
   else
      auxGetVolume(mAuxVolumeDeviceId, (unsigned long *)&vol);

   vol &= 0xffff;
   *volume = F32(vol) / 65535.f;

   setLastError("");
   return(true);
}

bool Win32RedBookDevice::setVolume(F32 volume)
{
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

   // move into a U32 - left/right U16 volumes
   U32 vol = U32(volume * 65536.f);
   if(vol > 0xffff)
      vol = 0xffff;

   if(mUsingMixer)
   {
      mMixerVolumeValue.dwValue = vol;
      mixerSetControlDetails(mVolumeDeviceId, &mMixerVolumeDetails, MIXER_SETCONTROLDETAILSF_VALUE);
   }
   else
   {
      vol |= vol << 16;
      auxSetVolume(mAuxVolumeDeviceId, vol);
   }

   setLastError("");
   return(true);
}

//------------------------------------------------------------------------------

void Win32RedBookDevice::openVolume()
{
   setLastError("");

   // first attempt to get the volume control through the mixer API
   S32 i;
   for(i = mixerGetNumDevs() - 1; i >= 0; i--)
   {
      // open the mixer
      if(mixerOpen((HMIXER*)&mVolumeDeviceId, i, 0, 0, 0) == MMSYSERR_NOERROR)
      {
         MIXERLINE lineInfo;
         memset(&lineInfo, 0, sizeof(lineInfo));
         lineInfo.cbStruct = sizeof(lineInfo);
         lineInfo.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC;

         // get the cdaudio line
         if(mixerGetLineInfo(mVolumeDeviceId, &lineInfo, MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR)
         {
            MIXERLINECONTROLS lineControls;
            MIXERCONTROL volumeControl;

            memset(&lineControls, 0, sizeof(lineControls));
            lineControls.cbStruct = sizeof(lineControls);
            lineControls.dwLineID = lineInfo.dwLineID;
            lineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
            lineControls.cControls = 1;
            lineControls.cbmxctrl = sizeof(volumeControl);
            lineControls.pamxctrl = &volumeControl;

            memset(&volumeControl, 0, sizeof(volumeControl));
            volumeControl.cbStruct = sizeof(volumeControl);

            // get the volume control
            if(mixerGetLineControls(mVolumeDeviceId, &lineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE) == MMSYSERR_NOERROR)
            {
               memset(&mMixerVolumeDetails, 0, sizeof(mMixerVolumeDetails));
               mMixerVolumeDetails.cbStruct = sizeof(mMixerVolumeDetails);
               mMixerVolumeDetails.dwControlID = volumeControl.dwControlID;
               mMixerVolumeDetails.cChannels = 1;
               mMixerVolumeDetails.cbDetails = sizeof(mMixerVolumeValue);
               mMixerVolumeDetails.paDetails = &mMixerVolumeValue;

               memset(&mMixerVolumeValue, 0, sizeof(mMixerVolumeValue));

               // query the current value
               if(mixerGetControlDetails(mVolumeDeviceId, &mMixerVolumeDetails, MIXER_GETCONTROLDETAILSF_VALUE) == MMSYSERR_NOERROR)
               {
                  mUsingMixer = true;
                  mVolumeInitialized = true;
                  mOriginalVolume = mMixerVolumeValue.dwValue;
                  return;
               }
            }
         }
      }

      mixerClose((HMIXER)mVolumeDeviceId);
   }

   // try aux
   for(i = auxGetNumDevs() - 1; i >= 0; i--)
   {
      AUXCAPS caps;
      auxGetDevCaps(i, &caps, sizeof(AUXCAPS));
      if((caps.wTechnology == AUXCAPS_CDAUDIO) && (caps.dwSupport & AUXCAPS_VOLUME))
      {
         mAuxVolumeDeviceId = i;
         mVolumeInitialized = true;
         mUsingMixer = false;
         auxGetVolume(i, (unsigned long *)&mOriginalVolume);
         return;
      }
   }

   setLastError("Volume failed to initialize");
}

void Win32RedBookDevice::closeVolume()
{
   setLastError("");
   if(!mVolumeInitialized)
      return;

   if(mUsingMixer)
   {
      mMixerVolumeValue.dwValue = mOriginalVolume;
      mixerSetControlDetails(mVolumeDeviceId, &mMixerVolumeDetails, MIXER_SETCONTROLDETAILSF_VALUE);
      mixerClose((HMIXER)mVolumeDeviceId);
   }
   else
      auxSetVolume(mAuxVolumeDeviceId, mOriginalVolume);

   mVolumeInitialized = false;
}

//------------------------------------------------------------------------------

void Win32RedBookDevice::setLastError(const char * error)
{
   RedBook::setLastError(error);
}

void Win32RedBookDevice::setLastError(U32 errorId)
{
   char buffer[256];
   if(!mciGetErrorStringA(errorId, buffer, sizeof(buffer) - 1))
      setLastError("Failed to get MCI error string!");
   else
      setLastError(buffer);
}

