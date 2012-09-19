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

#ifndef _DECALSPHERE_H_
#define _DECALSPHERE_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _MSPHERE_H_
#include "math/mSphere.h"
#endif


class DecalInstance;
class SceneZoneSpaceManager;


/// A bounding sphere in world space and a list of DecalInstance(s)
/// contained by it. DecalInstance(s) are organized/binned in this fashion
/// as a lookup and culling optimization.
class DecalSphere
{
   public:

      static F32 smDistanceTolerance;
      static F32 smRadiusTolerance;

      DecalSphere()
      {
         VECTOR_SET_ASSOCIATION( mItems );
         VECTOR_SET_ASSOCIATION( mZones );
      }
      DecalSphere( const Point3F &position, F32 radius )
      {
         VECTOR_SET_ASSOCIATION( mItems );
         VECTOR_SET_ASSOCIATION( mZones );

         mWorldSphere.center = position;
         mWorldSphere.radius = radius;
      }

      /// Recompute #mWorldSphere from the current instance list.
      void updateWorldSphere();

      /// Recompute the zoning information from the current bounds.
      void updateZoning( SceneZoneSpaceManager* zoneManager );

      /// Decal instances in this sphere.
      Vector< DecalInstance* > mItems;

      /// Zones that intersect with this sphere.  If this is empty, the zoning
      /// state of the sphere is uninitialized.
      Vector< U32 > mZones;

      /// World-space sphere corresponding to this DecalSphere.
      SphereF mWorldSphere;

      ///
      bool tryAddItem( DecalInstance* inst );
};

#endif // !_DECALSPHERE_H_