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

#ifndef _SFX3DWORLD_H_
#define _SFX3DWORLD_H_

#ifndef _SCENETRACKER_H_
   #include "scene/sceneTracker.h"
#endif
#ifndef _SFXWORLD_H_
   #include "sfx/sfxWorld.h"
#endif
#ifndef _DATACHUNKER_H_
   #include "core/dataChunker.h"
#endif
#ifndef _OBJECTTYPES_H_
   #include "T3D/objectTypes.h"
#endif


class SFX3DWorld;


/// SFXObject implementation for the 3D system.
class SFX3DObject : public SceneObjectLink, public SFXObject< 3 >
{
   public:
   
      typedef SceneObjectLink Parent;
      
      ///
      SFX3DObject( SFX3DWorld* world, SceneObject* object );
      
      /// Return the transform for the ears on this object.
      void getEarTransform( MatrixF& transform ) const;
               
      // SFXObject.
      void getReferenceCenter( F32 position[ 3 ] ) const;
      void getBounds( F32 minBounds[ 3 ], F32 maxBounds[ 3 ] ) const;
      void getRealBounds( F32 minBounds[ 3 ], F32 maxBounds[ 3 ] ) const;
      SFXAmbience* getAmbience() const;
      bool containsPoint( const F32 point[ 3 ] ) const;
      String describeSelf() const;
};


/// Manager for the 3D sound world.
///
/// Any SceneObject in the world can be made the current listener.  It's ear position (if it's a ShapeBase)
/// will then be used as the reference center for attenuating 3D sounds, traversing ambient spaces, etc.
///
class SFX3DWorld : public SceneTracker
{
   public:
   
      typedef SceneTracker Parent;
      typedef SFXWorld< 3, SFX3DObject* > SFXWorldType;
      
      enum
      {
         /// The scene object type mask used to filter out the
         /// type of objects we are interested in.
         TYPEMASK = WaterObjectType       // Sound ambience.
                  | StaticShapeObjectType // Occlusion and sound ambience.
                  | StaticObjectType      // Portals and zones.
      };
      
   protected:
   
      /// The SFX world tracking system.
      SFXWorldType mSFXWorld;
      
      /// Allocator for the SFX3DObject SceneObjectLinks we attach to SceneObjects.
      FreeListChunker< SFX3DObject > mChunker;
      
      // SceneTracker.
      virtual bool _isTrackableObject( SceneObject* object ) const;
   
   public:
   
      ///
      SFX3DWorld();
      
      ///
      void update();
      
      /// Return the current listener object.
      SceneObject* getListener() const;
      
      /// Make the given object the current listener object.
      void setListener( SceneObject* object );
      
      /// Notify the SFX world that the given object had a (potential) change in its
      /// sound-related properties.
      void notifyChanged( SceneObject* object );
      
      ///
      void debugDump();
      
      // SceneTracker.
      virtual void registerObject( SceneObject* object );
      virtual void unregisterObject( SceneObject* object );
      virtual void updateObject( SceneObjectLink* object );
};


/// The singleton instance of SFX3DWorld, if there is one.
extern SFX3DWorld* gSFX3DWorld;

#endif // !_SFX3DWORLD_H_

