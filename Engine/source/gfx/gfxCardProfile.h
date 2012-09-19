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

#ifndef _GFXCARDPROFILE_H_
#define _GFXCARDPROFILE_H_

#ifndef _TDICTIONARY_H_
#include "core/util/tDictionary.h"
#endif
#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif
#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif


/// GFXCardProfiler provides a device independent wrapper around both the 
/// capabilities reported by the card/drivers and the exceptions recorded
/// in various scripts.
///
/// See the Torque Scripting Manual for more details.
///
class GFXCardProfiler
{
   /// @name icpi Internal Card Profile Interface
   ///
   /// This is the interface implemented by subclasses of this class in order
   /// to provide implementation-specific information about the current
   /// card/drivers.
   ///
   /// Basically, the implementation needs to provide some unique strings:
   ///      - mVersionString indicating the current driver version of the
   ///        card in question. (For instance, "53.36")
   ///      - mCardDescription indicating the name of the card ("Radeon 8500")
   ///      - getRendererString() indicating the name of the renderer ("DX9", "GL1.2").
   ///        Each card profiler subclass must return a unique constant so we can keep
   ///        data separate. Bear in mind that punctuation is stripped from filenames.
   ///        
   /// The profiler also needs to implement setupCardCapabilities(), which is responsible
   /// for querying the active device and setting defaults based on the reported capabilities,
   /// and _queryCardCap, which is responsible for recognizing and responding to
   /// device-specific capability queries.
   ///
   /// @{

public:

   ///
   const String &getVersionString() const { return mVersionString; }
   const String &getCardString() const { return mCardDescription; }
   const String &getChipString() const { return mChipSet; }
   U32 getVideoMemoryInMB() const { return mVideoMemory; }

   virtual const String &getRendererString() const = 0;

protected:

   String mVersionString;
   String mCardDescription;
   String mChipSet;
   U32 mVideoMemory;

   virtual void setupCardCapabilities()=0;

   /// Implementation specific query code. 
   ///
   /// This function is meant to be overridden by the specific implementation class.
   ///
   /// Some query strings are handled by the external implementation while others must
   /// be done by the specific implementation. This is given first chance to return
   /// a result, then the generic rules are applied.
   ///
   /// @param  query       Capability being queried.
   /// @param  foundResult Result to return to the caller. If the function returns true
   ///                     then this value is returned as the result of the query.
   virtual bool _queryCardCap(const String &query, U32 &foundResult)=0;

   virtual bool _queryFormat( const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips ) = 0;
   /// @}

   /// @name helpergroup Helper Functions
   ///
   /// Various helper functions.

   /// Load a specified script file from the profiles directory, if it exists.
   void loadProfileScript(const char* scriptName);
   
   /// Load the script files in order for the specified card profile tuple.
   void loadProfileScripts(const String& render, const String& vendor, const String& card, const String& version);

   String strippedString(const char*string);

   /// @}
   
   /// Capability dictionary.
   Map<String, U32> mCapDictionary;

public:
   
   /// @name ecpi External Card Profile Interface
   ///
   /// @{
   
   /// Called for a profile for a given device.
   GFXCardProfiler();
   virtual ~GFXCardProfiler();

   /// Set load script files and generally initialize things.
   virtual void init()=0;

   /// Called to query a capability. Given a query string it returns a 
   /// bool indicating whether or not the capability holds. If you call
   /// this and cap isn't recognized then it returns false and prints
   /// a console error.
   U32 queryProfile(const String &cap);

   /// Same as queryProfile(), but a default can be specified to indicate
   /// what value should be returned if the profiler doesn't know anything
   /// about it. If cap is not recognized, defaultValue is returned and
   /// no error is reported.
   U32 queryProfile(const String &cap, U32 defaultValue);

   /// Set the specified capability to the specified value.
   void setCapability(const String &cap, U32 value);

   /// Queries support for the specified texture format, and texture profile
   bool checkFormat( const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips );

   /// @}
};

#endif

