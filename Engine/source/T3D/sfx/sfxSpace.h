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

#ifndef _SFXSPACE_H_
#define _SFXSPACE_H_

#ifndef _SCENESPACE_H_
#include "scene/sceneSpace.h"
#endif

#ifndef _SCENEAMBIENTSOUNDOBJECT_H_
#include "scene/mixin/sceneAmbientSoundObject.h"
#endif

#ifndef _SCENEPOLYHEDRALOBJECT_H_
#include "scene/mixin/scenePolyhedralObject.h"
#endif


/// A convex space that defines a custom ambient sound space.
class SFXSpace : public SceneAmbientSoundObject< ScenePolyhedralObject< SceneSpace > >
{
   public:

      typedef SceneAmbientSoundObject< ScenePolyhedralObject< SceneSpace > > Parent;

   protected:

      // SceneSpace.
      virtual ColorI _getDefaultEditorSolidColor() const { return ColorI( 244, 135, 18, 45 ); }

   public:

      SFXSpace();

      // SimObject.
      DECLARE_CONOBJECT( SFXSpace );
      DECLARE_DESCRIPTION( "A box volume that defines an ambient sound space." );
      DECLARE_CATEGORY( "3D Sound" );

      static void consoleInit();
};

#endif // !_SFXSPACE_H_
