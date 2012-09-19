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

#ifndef _SCENESIMPLEZONE_H_
#define _SCENESIMPLEZONE_H_

#ifndef _SCENEZONESPACE_H_
#include "scene/zones/sceneZoneSpace.h"
#endif

#ifndef _MORIENTEDBOX_H_
#include "math/mOrientedBox.h"
#endif


class SceneRenderState;
class BaseMatInstance;


/// Abstract base class for a zone space that contains only a single zone.
///
/// Simple zones are required to be convex.
class SceneSimpleZone : public SceneZoneSpace
{
   public:

      typedef SceneZoneSpace Parent;

   protected:

      enum
      {
         AmbientMask   = Parent::NextFreeMask << 0,   ///< Ambient light color setting has changed.
         NextFreeMask  = Parent::NextFreeMask << 1,
      };

      /// @name Ambient Lighting
      /// @{

      /// If this is true, then the zone defines its own ambient
      /// light color in #mAmbientColor.
      bool mUseAmbientLightColor;

      /// Ambient light color in this zone.
      ColorF mAmbientLightColor;

      /// @}

      /// @name Transforms
      /// @{

      /// Whether the zone has been rotated.  This is used to fasttrack overlap
      /// tests to avoid doing an OBB/AABB intersection test when we can simply
      /// use a much quicker AABB/AABB intersection test on the world boxes of
      /// both objects.
      bool mIsRotated;

      /// The OBB for the zone.
      OrientedBox3F mOrientedWorldBox;

      /// @}

      /// Return the oriented bounding box for the zone.
      const OrientedBox3F& _getOrientedWorldBox() const { return mOrientedWorldBox; }

      /// Update the OBB for the zone.
      virtual void _updateOrientedWorldBox() { mOrientedWorldBox.set( getTransform(), getScale() ); }

      // SceneObject.
      virtual bool onSceneAdd();

   public:

      SceneSimpleZone();

      /// @name Ambient Lighting
      /// @{

      /// Return true if the zone defines its own ambient light color.
      bool useAmbientLightColor() const { return mUseAmbientLightColor; }

      /// Set whether a custom ambient light color is active in this zone.
      void setUseAmbientLightColor( bool value );

      /// Return the ambient light color for this zone.
      ColorF getAmbientLightColor() const { return mAmbientLightColor; }

      /// Set the ambient light color for the zone.
      /// @note This only takes effect if useAmbientLightColor() return true.
      /// @see setUseAmbientLightColor
      void setAmbientLightColor( const ColorF& color );

      /// @}

      /// @name Inherited
      /// @{

      // SimObject.
      virtual String describeSelf() const;

      static void initPersistFields();

      // NetObject
      virtual U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
      virtual void unpackUpdate( NetConnection *conn, BitStream *stream );

      // SceneObject
      virtual void prepRenderImage( SceneRenderState* state );
      virtual void setTransform( const MatrixF& mat );

      // SceneZoneSpace.
      virtual U32 getPointZone( const Point3F &p );
      virtual bool getOverlappingZones( const Box3F& aabb, U32* outZones, U32& outNumZones );
      virtual void traverseZones( SceneTraversalState* state );
      virtual bool getZoneAmbientLightColor( U32 zone, ColorF& outColor ) const;
      virtual void traverseZones( SceneTraversalState* state, U32 startZoneId );

      /// @}

   private:

      static bool _setUseAmbientLightColor( void* object, const char* index, const char* data );
      static bool _setAmbientLightColor( void* object, const char* index, const char* data );
};

#endif // !_SCENESIMPLEZONE_H_
