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

#ifndef _SFXFMODPLUGIN_H_
#define _SFXFMODPLUGIN_H_

#ifndef _SFXSYSTEM_H_
   #include "sfx/sfxSystem.h"
#endif


/// SFXSystem plugin that adds the capability to create SFXSources for
/// Designer SFXFMODEvents.
///
/// The plugin will only be installed if an FMOD device has been created.
/// While SFXFMODEvents may be constructed without an FMOD device, trying
/// to play such an event then will result in an error.
///
class SFXFMODPlugin : public SFXSystemPlugin
{
   public:
   
      typedef SFXSystemPlugin Parent;
      
      ///
      virtual SFXSource* createSource( SFXTrack* track );
};

#endif // !_SFXFMODPLUGIN_H_
