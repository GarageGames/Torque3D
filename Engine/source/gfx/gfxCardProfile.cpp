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

#include "platform/platform.h"
#include "gfx/gfxCardProfile.h"

#include "console/console.h"
#include "console/engineAPI.h"
#include "core/volume.h"

// NOTE: The script class docs are in 
// Documentation\scriptDocs\docs\classGFXCardProfiler.txt


void GFXCardProfiler::loadProfileScript(const char* aScriptName)
{
   const String   profilePath = Con::getVariable( "$pref::Video::profilePath" );
   String scriptName = !profilePath.isEmpty() ? profilePath.c_str() : "profile";
   scriptName += "/";
   scriptName += aScriptName;
   
   void  *data = NULL;
   U32   dataSize = 0;

   Torque::FS::ReadFile( scriptName.c_str(), data, dataSize, true );

   if(data == NULL)
   {
      Con::warnf("      - No card profile %s exists", scriptName.c_str());
      return;
   }

   const char  *script = static_cast<const char *>(data);

   Con::printf("      - Loaded card profile %s", scriptName.c_str());

   Con::evaluate(script, false, NULL);
   delete[] script;
}

void GFXCardProfiler::loadProfileScripts(const String& render, const String& vendor, const String& card, const String& version)
{
   String script = render + ".cs";
   loadProfileScript(script);

   script = render + "." + vendor + ".cs";
   loadProfileScript(script);

   script = render + "." + vendor + "." + card + ".cs";
   loadProfileScript(script);

   script = render + "." + vendor + "." + card + "." + version + ".cs";
   loadProfileScript(script);
}

GFXCardProfiler::GFXCardProfiler() : mVideoMemory( 0 )
{
}

GFXCardProfiler::~GFXCardProfiler()
{
   mCapDictionary.clear();
}

String GFXCardProfiler::strippedString(const char *string)
{
   String res = "";

   // And fill it with the stripped string...
   const char *a=string;
   while(*a)
   {
      if(isalnum(*a))
      {
         res += *a;
      }
      a++;
   }

   return res;
}

void GFXCardProfiler::init()
{
   // Spew a bit...
   Con::printf("Initializing GFXCardProfiler (%s)", getRendererString().c_str());
   Con::printf("   o Chipset : '%s'", getChipString().c_str());
   Con::printf("   o Card    : '%s'", getCardString().c_str());
   Con::printf("   o Version : '%s'", getVersionString().c_str());
   Con::printf("   o VRAM    : %d MB", getVideoMemoryInMB());

   // Do card-specific setup...
   Con::printf("   - Scanning card capabilities...");

   setupCardCapabilities();

   // And finally, load stuff up...
   String render  = strippedString(getRendererString());
   String chipset  = strippedString(getChipString());
   String card    = strippedString(getCardString());
   String version = strippedString(getVersionString());

   Con::printf("   - Loading card profiles...");
   loadProfileScripts(render, chipset, card, version);
}

U32 GFXCardProfiler::queryProfile(const String &cap)
{
   U32 res;
   if( _queryCardCap( cap, res ) )
      return res;

   if(mCapDictionary.contains(cap))
      return mCapDictionary[cap];

   Con::errorf( "GFXCardProfiler (%s) - Unknown capability '%s'.", getRendererString().c_str(), cap.c_str() );
   return 0;
}

U32 GFXCardProfiler::queryProfile(const String &cap, U32 defaultValue)
{
   PROFILE_SCOPE( GFXCardProfiler_queryProfile );

   U32 res;
   if( _queryCardCap( cap, res ) )
      return res;

   if( mCapDictionary.contains( cap ) )
      return mCapDictionary[cap];
   else
      return defaultValue;
}

void GFXCardProfiler::setCapability(const String &cap, U32 value)
{
   // Check for dups.
   if( mCapDictionary.contains( cap ) )
   {
      Con::warnf( "GFXCardProfiler (%s) - Setting capability '%s' multiple times.", getRendererString().c_str(), cap.c_str() );
      mCapDictionary[cap] = value;
      return;
   }

   // Insert value as necessary.
   Con::printf( "GFXCardProfiler (%s) - Setting capability '%s' to %d.", getRendererString().c_str(), cap.c_str(), value );
   mCapDictionary.insert( cap, value );
}

bool GFXCardProfiler::checkFormat( const GFXFormat fmt, const GFXTextureProfile *profile, bool &inOutAutogenMips )
{
   return _queryFormat( fmt, profile, inOutAutogenMips );
}

DECLARE_SCOPE( GFXCardProfilerAPI );
IMPLEMENT_SCOPE( GFXCardProfilerAPI, GFXCardProfiler,, "");
ConsoleDoc(
   "@class GFXCardProfilerAPI\n"
   "@brief This class is the interface between TorqueScript and GFXCardProfiler.\n\n"
   "You will not actually declare GFXCardProfilerAPI in TorqueScript. It exists solely "
   "to give access to the GFXCardProfiler's querying functions, such as GFXCardProfiler::getRenderer.\n\n"
   "@tsexample\n"
   "// Example of accessing GFXCardProfiler function from script\n"
   "// Notice you are not using the API version\n"
   "%videoMem = GFXCardProfiler::getVideoMemoryMB();\n"
   "@endtsexample\n\n"
   "@see GFXCardProfiler for more information\n\n"
   "@ingroup GFX\n"
);

DefineEngineStaticMethod( GFXCardProfilerAPI, getVersion, String, (),, 
   "Returns the driver version string." )
{
	return GFX->getCardProfiler()->getVersionString();
}

DefineEngineStaticMethod( GFXCardProfilerAPI, getCard, String, (),,
   "Returns the card name." )
{
	return GFX->getCardProfiler()->getCardString();
}

DefineEngineStaticMethod( GFXCardProfilerAPI, getVendor, String, (),,
   "Returns the card vendor name." )
{
   // TODO: Fix all of this vendor crap, it's not consistent
	return GFX->getCardProfiler()->getChipString();
}

DefineEngineStaticMethod( GFXCardProfilerAPI, getRenderer, String, (),,
   "Returns the renderer name.  For example D3D9 or OpenGL." )
{
	return GFX->getCardProfiler()->getRendererString();
}

DefineEngineStaticMethod( GFXCardProfilerAPI, getVideoMemoryMB, S32, (),,
   "Returns the amount of video memory in megabytes." )
{
   return GFX->getCardProfiler()->getVideoMemoryInMB();
}

DefineEngineStaticMethod( GFXCardProfilerAPI, setCapability, void, ( const char *name, S32 value ),,
   "Used to set the value for a specific card capability.\n"
   "@param name The name of the capability being set.\n"
   "@param value The value to set for that capability." )
{
	GFX->getCardProfiler()->setCapability( name, (U32)value );
}

DefineEngineStaticMethod( GFXCardProfilerAPI, queryProfile, S32, ( const char *name, S32 defaultValue ),,
   "Used to query the value of a specific card capability.\n"
   "@param name The name of the capability being queried.\n"
   "@param defaultValue The value to return if the capability is not defined." )
{
	return (S32)GFX->getCardProfiler()->queryProfile( name, (U32)defaultValue );
}


DefineEngineStaticMethod( GFXCardProfilerAPI, getBestDepthFormat, String, (),,
                         "Returns the card name." )
{
    if (GFX->getCardProfiler()->queryProfile("GL::Workaround::intel_mac_depth", false))
        return "GFXFormatD16";
    else
        return "GFXFormatD24S8";
}

