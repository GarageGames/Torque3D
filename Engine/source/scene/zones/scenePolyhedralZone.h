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

#ifndef _SCENEPOLYHEDRALZONE_H_
#define _SCENEPOLYHEDRALZONE_H_

#ifndef _SCENESIMPLEZONE_H_
#include "scene/zones/sceneSimpleZone.h"
#endif

#ifndef _SCENEPOLYHEDRALOBJECT_H_
#include "scene/mixin/scenePolyhedralObject.h"
#endif

#ifndef _MINTERSECTOR_H_
#include "math/mIntersector.h"
#endif


/// A simple zone space that is described by a polyhedron.
///
/// By default, if no other polyhedron is assigned to a polyhedral zone, the
/// polyhedron is initialized from the zone's object box.
class ScenePolyhedralZone : public ScenePolyhedralObject< SceneSimpleZone >
{
   public:

      typedef ScenePolyhedralObject< SceneSimpleZone > Parent;

   protected:

      typedef PolyhedronBoxIntersector< PolyhedronType > IntersectorType;

      /// Fast polyhedron/AABB intersector used for testing SceneObject AABBs
      /// for overlap with the zone.
      IntersectorType mIntersector;

      /// Precompute polyhedron/AABB intersection data.
      void _updateIntersector()
      {
         mIntersector = IntersectorType( getPolyhedron(), getTransform(), getScale(), getWorldBox() );
      }

      // SceneSimpleZone.
      virtual void _updateOrientedWorldBox();

   public:

      ScenePolyhedralZone() {}
      ScenePolyhedralZone( const PolyhedronType& polyhedron );

      // SimObject.
      virtual bool onAdd();

      // SceneObject.
      virtual void setTransform( const MatrixF& mat );

      // SceneZoneSpace.
      virtual bool getOverlappingZones( const Box3F& aabb, U32* outZones, U32& outNumZones );
};

#endif // !_SCENEPOLYHEDRALZONE_H_
