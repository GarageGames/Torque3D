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

#ifndef _SCENEAMBIENTSOUNDOBJECT_H_
#define _SCENEAMBIENTSOUNDOBJECT_H_


class SFXAmbience;


/// Template mixin to add ability to hold a custom SFXAmbience to
/// a SceneObject.
template< typename Base >
class SceneAmbientSoundObject : public Base
{
   public:

      typedef Base Parent;

   protected:

      enum
      {
         SoundMask     = Parent::NextFreeMask << 0,   ///< Ambient sound properties have changed.
         NextFreeMask  = Parent::NextFreeMask << 1,
      };

      /// Ambient sound properties for this space.
      SFXAmbience* mSoundAmbience;

   public:

      SceneAmbientSoundObject();

      /// Set the ambient sound properties for the space.
      void setSoundAmbience( SFXAmbience* ambience );

      // SimObject.
      static void initPersistFields();

      // NetObject.
      virtual U32 packUpdate( NetConnection* connection, U32 mask, BitStream* stream );
      virtual void unpackUpdate( NetConnection* connection, BitStream* stream );

      // SceneObject.
      virtual SFXAmbience* getSoundAmbience() const { return mSoundAmbience; }

   private:

      // Console field getters/setters.
      static bool _setSoundAmbience( void* object, const char* index, const char* data );
};

#endif // !_SCENEAMBIENTSOUNDOBJECT_H_
