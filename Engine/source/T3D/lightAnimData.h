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

#ifndef _LIGHTANIMDATA_H_
#define _LIGHTANIMDATA_H_

#ifndef _SIMDATABLOCK_H_
#include "console/simDatablock.h"
#endif
#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif
#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _COLOR_H_
#include "core/color.h"
#endif

class LightInfo;


/// The light animation state required by LightAnimData.
struct LightAnimState
{  
   /// Constructor.
   LightAnimState()
      :  active( true ),
         animationPhase( 1.0f ),
         animationPeriod( 1.0f ),
         brightness( 1.0f ),
         transform( true ),
         color( 0, 0, 0, 0 )
   {
   }

   /// If true light animation should be performed.
   bool active;

   /// The phase used to offset the animation start 
   /// time to vary the animation of nearby lights.
   F32 animationPhase;

   /// The length of time in seconds for a single playback
   /// of the light animation.
   F32 animationPeriod;   

   /// The set light brightness before animation occurs.
   F32 brightness;

   /// The set light transform before animation occurs.
   MatrixF transform;

   /// The set light color before animation occurs.
   ColorF color;
};


/// A datablock which defines and performs light animation.
/// @see LightBase, LightDescription
class LightAnimData : public SimDataBlock
{
   typedef SimDataBlock Parent;

protected:

   /// Called internally to update the key data.
   void _updateKeys();

public:

   LightAnimData();
   virtual ~LightAnimData();

   DECLARE_CONOBJECT( LightAnimData );

   // SimObject
   static void initPersistFields();
   virtual void inspectPostApply();

   // SimDataBlock
   virtual bool preload( bool server, String &errorStr );
   virtual void packData( BitStream *stream );
   virtual void unpackData( BitStream *stream );

   /// Animates parameters on the passed Light's LightInfo object.
   virtual void animate( LightInfo *light, LightAnimState *state );

   /// Helper class used to keyframe light parameters.  It is templatized
   /// on the number of parameters to store.
   template<U32 COUNT>
   struct AnimValue
   {
      /// Constructor.
      AnimValue()
      {
         dMemset( value1, 0, sizeof( value1 ) );
         dMemset( value2, 0, sizeof( value2 ) );
         dMemset( period, 0, sizeof( period ) );
         dMemset( keys, 0, sizeof( keys ) );
         dMemset( smooth, 0, sizeof( smooth ) );
         dMemset( keyLen, 0, sizeof( keyLen ) );
         dMemset( timeScale, 0, sizeof( timeScale ) );
      }

      /// The first value associated with the A keyframe.
      F32 value1[COUNT];

      /// The second value associated with the Z keyframe.
      F32 value2[COUNT];

      /// The period of the full keyframe sequence.
      F32 period[COUNT];

      /// The keyframe keys as a string of letters A to Z.
      StringTableEntry keys[COUNT];

      /// If true the transition between keyframes will be smooth.
      bool smooth[COUNT];

      /// The calculated length of the keyframe string.
      /// @see updateKey
      U32 keyLen[COUNT];

      /// The scale used to convert time into a keyframe position.
      /// @see updateKey
      F32 timeScale[COUNT];

      /// Performs the animation returning the results in the output if
      /// the time scale is greater than zero.
      /// @return Returns true if the animation was performed.
      bool animate(F32 time, F32 *output, bool multiply = false);

      /// Called when the key string is changed to update the
      /// key length and time scale.
      void updateKey();

      /// Write the animation data to the bitstream.
      void write( BitStream *stream ) const;

      /// Read the animation data from the bitstream.
      void read( BitStream *stream );
   };

   /// The positional animation parameters for x, y, and z.
   AnimValue<3> mOffset;

   /// The rotational animation parameters for x, y, and z.
   AnimValue<3> mRot;

   /// The color animation parameters for r, g, and b.
   AnimValue<3> mColor;

   /// The brightness animation parameter.
   AnimValue<1> mBrightness;
};

#endif // _LIGHTANIMDATA_H_