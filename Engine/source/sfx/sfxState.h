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

#ifndef _SFXSTATE_H_
#define _SFXSTATE_H_

#ifndef _SIMDATABLOCK_H_
   #include "console/simDatablock.h"
#endif
#ifndef _CONSOLETYPES_H_
   #include "console/consoleTypes.h"
#endif
#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif
#ifndef _SFXCOMMON_H_
   #include "sfx/sfxCommon.h"
#endif


/// A boolean switch used to modify playlist behavior.
///
class SFXState : public SimDataBlock
{
   public:
   
      typedef SimDataBlock Parent;
      
      enum
      {
         MaxIncludedStates = 4,
         MaxExcludedStates = 4
      };
      
   protected:
   
      /// Reference count for activation.
      U32 mActiveCount;
      
      /// Reference count for disabling.
      U32 mDisableCount;
      
      /// States that will be activated when this state is activated.
      SFXState* mIncludedStates[ MaxIncludedStates ];
   
      /// States that will be disabled when this state is activated.
      SFXState* mExcludedStates[ MaxExcludedStates ];
                  
      /// Call when state has become active.
      void _onActivate();
      
      /// Call when state has gone back to being deactive.
      void _onDeactivate();
      
      /// @name Callbacks
      /// @{
      
      DECLARE_CALLBACK( void, onActivate, () );
      DECLARE_CALLBACK( void, onDeactivate, () );
      
      /// @}
      
   public:
   
      ///
      SFXState();
      
      /// Return true if the state is currently active (activated and not disabled).
      bool isActive() const { return ( !isDisabled() && mActiveCount > 0 ); }
      
      /// Return true if the state is currently disabled.
      bool isDisabled() const { return ( mDisableCount > 0 ); }
      
      /// Activate this state.  activate/deactivate calls balance each other.
      /// Activating a disabled state will not make it active.
      void activate();
      
      ///
      void deactivate();
      
      /// Re-enable this state.
      void enable();
      
      /// Disable this state so that it cannot be activated.  This is used
      /// by state exclusion.
      void disable();
      
      // SimDataBlock.
      virtual bool onAdd();
      virtual bool preload( bool server, String& errorStr );
      virtual void packData( BitStream* stream );
      virtual void unpackData( BitStream* stream );
      
      static void initPersistFields();
   
      DECLARE_CONOBJECT( SFXState );
      DECLARE_CATEGORY( "SFX" );
      DECLARE_DESCRIPTION( "A datablock describing a particular state for the SFX system." );
};

#endif // !_SFXSTATE_H_
