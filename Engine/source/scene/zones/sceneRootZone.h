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

#ifndef _SCENEROOTZONE_H_
#define _SCENEROOTZONE_H_

#ifndef _SCENESIMPLEZONE_H_
#include "scene/zones/sceneSimpleZone.h"
#endif



/// Root zone in a scene.
///
/// The root zone is an infinite zone that contains all others zones and
/// objects in the scene.
///
/// This class is not declared as a ConsoleObject and is not visible
/// from TorqueScript.  SceneRootZone objects are not registered with the
/// Sim system.
class SceneRootZone : public SceneSimpleZone
{
   public:

      typedef SceneSimpleZone Parent;

   protected:

      // SceneObject.
      virtual bool onSceneAdd();
      virtual void onSceneRemove();

   public:

      SceneRootZone();

      // SimObject.
      virtual bool onAdd();
      virtual void onRemove();
      virtual String describeSelf() const;

      // SceneObject.
      virtual bool containsPoint( const Point3F &point ) const;

      // SceneZoneSpace.
      virtual U32 getPointZone( const Point3F &p ) const;
      virtual bool getOverlappingZones( const Box3F& aabb, U32* outZones, U32& outNumZones );
};

#endif //_SCENEROOTZONE_H_
