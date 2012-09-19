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

#ifndef _SFXPROVIDER_H_
#define _SFXPROVIDER_H_

#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif


class SFXDevice;



struct SFXDeviceInfo
{
   String   driver;
   String   name;
   bool     hasHardware;
   S32      maxBuffers;

   virtual ~SFXDeviceInfo() {}
};

typedef Vector<SFXDeviceInfo*> SFXDeviceInfoVector;

class SFXProvider
{
   friend class SFXSystem;

   private:

      /// The head of the linked list of avalible providers.
      static SFXProvider* smProviders;

      /// The next provider in the linked list of available providers. 
      SFXProvider* mNextProvider;

      /// The provider name which is passed by the concrete provider
      /// class to the SFXProvider constructor.
      String mName;

      static Vector<SFXProvider*> sAllProviders;

   protected:

      /// The array of avaIlable devices from this provider.  The
      /// concrete provider class will fill this on construction.
      SFXDeviceInfoVector  mDeviceInfo;

      /// This registers the provider to the available provider list.  It should be called
      /// for providers that are properly initialized and available for device enumeration and creation.
      /// the add and registration process is 2 steps to avoid issues when TGEA is used as a shared library (specifically on Windows)
      static void regProvider( SFXProvider* provider );

      virtual void init() = 0;

      SFXProvider( const String& name );
      ~SFXProvider();

      /// Look up the SFXDeviceInfo for the given device in mDeviceInfo.
      /// Return default device (first in list) if no other device matches (or null if device list is empty).
      SFXDeviceInfo* _findDeviceInfo( const String& deviceName );

      /// This is called from SFXSystem to create a new device.  Must be implemented
      /// by all contrete provider classes.
      ///
      /// @param deviceName      The case sensitive name of the device or NULL to create the 
      //                         default device.
      /// @param useHardware     Toggles the use of hardware processing when available.
      /// @param maxBuffers      The maximum buffers for this device to use or -1 
      ///                        for the device to pick a reasonable default for that device.
      ///
      /// @return Returns the created device or NULL for failure.
      ///
      virtual SFXDevice* createDevice( const String& deviceName, bool useHardware, S32 maxBuffers ) = 0;

   public:

      /// Returns a specific provider by searching the provider list
      /// for the first provider with the case sensitive name.
      static SFXProvider* findProvider( String providerName );

      /// Returns the first provider in the provider list.  Use 
      /// getNextProvider() to iterate over list.
      static SFXProvider* getFirstProvider() { return smProviders; }

      /// Returns the next provider in the provider list or NULL
      /// when the end of the list is reached.
      SFXProvider* getNextProvider() const { return mNextProvider; }

      /// The case sensitive name of this provider.
      const String& getName() const { return mName; }

      /// Returns a read only vector with device information for
      /// all creatable devices available from this provider.
      const SFXDeviceInfoVector& getDeviceInfo() const { return mDeviceInfo; }

      static void initializeAllProviders();

};


#endif // _SFXPROVIDER_H_