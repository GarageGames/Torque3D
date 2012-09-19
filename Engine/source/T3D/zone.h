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

#ifndef _ZONE_H_
#define _ZONE_H_

#ifndef _SCENEPOLYHEDRALZONE_H_
#include "scene/zones/scenePolyhedralZone.h"
#endif

#ifndef _SCENEAMBIENTSOUNDOBJECT_H_
#include "scene/mixin/sceneAmbientSoundObject.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif


/// A volume in space that encloses objects.
///
/// Zones do not physically contain objects in the scene.  Rather, any object
/// that has its world box coincide with the world box of a zone is considered
/// to be part of that zone.  As such, objects can be in multiple zones at
/// the same time.
class Zone : public SceneAmbientSoundObject< ScenePolyhedralZone >
{
   public:

      typedef SceneAmbientSoundObject< ScenePolyhedralZone > Parent;

   protected:

      // SceneVolume.
      virtual ColorI _getDefaultEditorSolidColor() const { return ColorI( 255, 0, 0, 45 ); }

   public:

      Zone() {}
      Zone( const Polyhedron& polyhedron )
      {
         mPolyhedron = polyhedron;
      }

      // SimObject
      DECLARE_CONOBJECT( Zone );
      DECLARE_DESCRIPTION( "A volume that encloses objects for visibility culling." );
      DECLARE_CATEGORY( "3D" );

      static void consoleInit();
};

#endif // _ZONE_H_
