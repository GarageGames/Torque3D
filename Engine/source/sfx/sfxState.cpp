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

#include "sfx/sfxState.h"
#include "sfx/sfxTypes.h"
#include "core/stream/bitStream.h"
#include "console/engineAPI.h"


//#define DEBUG_SPEW


IMPLEMENT_CO_DATABLOCK_V1( SFXState );


ConsoleDocClass( SFXState,
   "@brief A boolean switch used to modify playlist behavior.\n\n"
   
   "Sound system states are used to allow playlist controllers to make decisions based on global state.  This is useful, for "
   "example, to couple audio playback to gameplay state.  Certain states may, for example, represent different locations that the "
   "listener can be in, like underwater, in open space, or indoors.  Other states could represent moods of the current gameplay "
   "situation, like, for example, an aggressive mood during combat.\n\n"
   
   "By activating and deactivating sound states according to gameplay state, a set of concurrently running playlists may "
   "react and adapt to changes in the game.\n\n"
   
   "@section SFXState_activation Activation and Deactivation\n"
   
   "At any time, a given state can be either active or inactive.  Calling activate() on a state increases an internal "
   "counter and calling deactivate() decreases the counter.  Only when the count reaches zero will the state be "
   "deactivated.\n\n"
   
   "In addition to the activation count, states also maintain a disabling count.  Calling disable() increases this count "
   "and calling enable() decreases it.  As long as this count is greater than zero, a given state will not be activated "
   "even if its activation count is non-zero.  Calling disable() on an active state will not only increase the disabling "
   "count but also deactivate the state.  Calling enable() on a state with a positive activation count will re-activate "
   "the state when the disabling count reaches zero.\n\n"
   
   "@section SFXState_dependencies State Dependencies\n"
   
   "By listing other states in in its #includedStates and #excludedStates fields, a state may automatically trigger the "
   "activation or disabling of other states in the sytem when it is activated.  This allows to form dependency chains "
   "between individual states.\n\n"
   
   "@tsexample\n"
   "// State indicating that the listener is submerged.\n"
   "singleton SFXState( AudioLocationUnderwater )\n"
   "{\n"
   "   parentGroup = AudioLocation;\n"
   "   // AudioStateExclusive is a class defined in the core scripts that will automatically\n"
   "   // ensure for a state to deactivate all the sibling SFXStates in its parentGroup when it\n"
   "   // is activated.\n"
   "   className = \"AudioStateExclusive\";\n"
   "};\n"
   "\n"
   "// State suitable e.g. for combat.\n"
   "singleton SFXState( AudioMoodAggressive )\n"
   "{\n"
   "   parentGroup = AudioMood;\n"
   "   className = \"AudioStateExclusive\";\n"
   "};\n"
   "@endtsexample\n\n"
   
   "@see SFXPlayList\n"
   "@see SFXController\n"
   "@see SFXPlayList::state\n"
   "@see SFXPlayList::stateMode\n\n"
   "@ref SFX_interactive\n\n"
   
   "@ingroup SFX\n"
   "@ingroup Datablocks"
);


IMPLEMENT_CALLBACK( SFXState, onActivate, void, (), (),
   "Called when the state goes from inactive to active." );
IMPLEMENT_CALLBACK( SFXState, onDeactivate, void, (), (),
   "called when the state goes from active to deactive." );
   

static Vector< SFXState* > sgActiveStates( __FILE__, __LINE__ );


//-----------------------------------------------------------------------------

SFXState::SFXState()
   : mActiveCount( 0 ),
     mDisableCount( 0 )
{
   dMemset( mIncludedStates, 0, sizeof( mIncludedStates ) );
   dMemset( mExcludedStates, 0, sizeof( mExcludedStates ) );
}

//-----------------------------------------------------------------------------

void SFXState::initPersistFields()
{
   addGroup( "State" );
   
      addField( "includedStates", TypeSFXStateName, Offset( mIncludedStates, SFXState ),
         MaxIncludedStates,
         "States that will automatically be activated when this state is activated.\n\n"
         "@ref SFXState_activation" );
      addField( "excludedStates", TypeSFXStateName, Offset( mExcludedStates, SFXState ),
         MaxExcludedStates,
         "States that will automatically be disabled when this state is activated.\n\n"
         "@ref SFXState_activation" );
   
   endGroup( "State" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

void SFXState::activate()
{
   mActiveCount ++;
   if( mActiveCount == 1 && !isDisabled() )
      _onActivate();  
}

//-----------------------------------------------------------------------------

void SFXState::deactivate()
{
   if( !mActiveCount )
      return;
      
   mActiveCount --;
   if( !mActiveCount && !isDisabled() )
      _onDeactivate();
}

//-----------------------------------------------------------------------------

void SFXState::enable()
{
   if( !mDisableCount )
      return;
      
   mDisableCount --;
   
   if( !mDisableCount && mActiveCount > 0 )
      _onActivate();
}

//-----------------------------------------------------------------------------

void SFXState::disable()
{
   mDisableCount ++;
   
   if( mDisableCount == 1 && mActiveCount > 0 )
      _onDeactivate();
}

//-----------------------------------------------------------------------------

bool SFXState::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   Sim::getSFXStateSet()->addObject( this );
      
   return true;
}

//-----------------------------------------------------------------------------

bool SFXState::preload( bool server, String& errorStr )
{
   if( !Parent::preload( server, errorStr ) )
      return false;

   if( !server )
   {
      for( U32 i = 0; i < MaxIncludedStates; ++ i )
         if( !sfxResolve( &mIncludedStates[ i ], errorStr ) )
            return false;
         
      for( U32 i = 0; i < MaxExcludedStates; ++ i )
         if( !sfxResolve( &mExcludedStates[ i ], errorStr ) )
            return false;
   }
            
   return true;
}

//-----------------------------------------------------------------------------

void SFXState::packData( BitStream* stream )
{
   Parent::packData( stream );
      
   for( U32 i = 0; i < MaxIncludedStates; ++ i )
      sfxWrite( stream, mIncludedStates[ i ] );
   for( U32 i = 0; i < MaxExcludedStates; ++ i )
      sfxWrite( stream, mExcludedStates[ i ] );
}

//-----------------------------------------------------------------------------

void SFXState::unpackData( BitStream* stream )
{
   Parent::unpackData( stream );
   
   for( U32 i = 0; i < MaxIncludedStates; ++ i )
      sfxRead( stream, &mIncludedStates[ i ] );
   for( U32 i = 0; i < MaxExcludedStates; ++ i )
      sfxRead( stream, &mExcludedStates[ i ] );
}

//-----------------------------------------------------------------------------

void SFXState::_onActivate()
{
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXState] Activating '%s'", getName() );
   #endif
   
   onActivate_callback();
      
   // Add the state to the list.
      
   sgActiveStates.push_back( this );

   // Activate included states.
      
   for( U32 i = 0; i < MaxIncludedStates; ++ i )
      if( mIncludedStates[ i ] )
         mIncludedStates[ i ]->activate();
         
   // Disable excluded states.
         
   for( U32 i = 0; i < MaxExcludedStates; ++ i )
      if( mExcludedStates[ i ] )
         mExcludedStates[ i ]->disable();
}

//-----------------------------------------------------------------------------

void SFXState::_onDeactivate()
{
   #ifdef DEBUG_SPEW
   Platform::outputDebugString( "[SFXState] Deactivating '%s'", getName() );
   #endif

   onDeactivate_callback();
      
   // Remove the state from the list.
      
   for( U32 i = 0; i < sgActiveStates.size(); ++ i )
      if( sgActiveStates[ i ] == this )
      {
         sgActiveStates.erase_fast( i );
         break;
      }

   // Deactivate included states.
      
   for( U32 i = 0; i < MaxIncludedStates; ++ i )
      if( mIncludedStates[ i ] )
         mIncludedStates[ i ]->deactivate();
         
   // Enable excluded states.
         
   for( U32 i = 0; i < MaxExcludedStates; ++ i )
      if( mExcludedStates[ i ] )
         mExcludedStates[ i ]->enable();
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXState, isActive, bool, (),,
   "Test whether the state is currently active.\n"
   "This is true when the activation count is >0 and the disabling count is =0.\n"
   "@return True if the state is currently active.\n"
   "@see activate" )
{
   return object->isActive();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXState, activate, void, (),,
   "Increase the activation count on the state.\n"
   "If the state isn't already active and it is not disabled, the state will be activated.\n"
   "@see isActive\n"
   "@see deactivate\n" )
{
   object->activate();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXState, deactivate, void, (),,
   "Decrease the activation count on the state.\n"
   "If the count reaches zero and the state was not disabled, the state will be deactivated.\n"
   "@see isActive\n"
   "@see activate\n" )
{
   object->deactivate();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXState, isDisabled, bool, (),,
   "Test whether the state is currently disabled.\n"
   "This is true when the disabling count of the state is non-zero.\n"
   "@return True if the state is disabled.\n\n"
   "@see disable\n" )
{
   return object->isDisabled();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXState, disable, void, (),,
   "Increase the disabling count of the state.\n"
   "If the state is currently active, it will be deactivated.\n"
   "@see isDisabled\n" )
{
   object->disable();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXState, enable, void, (),,
   "Decrease the disabling count of the state.\n"
   "If the disabling count reaches zero while the activation count is still non-zero, "
      "the state will be reactivated again.\n"
   "@see isDisabled\n" )
{
   object->enable();
}

//=============================================================================
//    Console Functions.
//=============================================================================
// MARK: ---- Console Functions ----

//-----------------------------------------------------------------------------

DefineEngineFunction( sfxGetActiveStates, const char*, (),,
   "Return a newline-separated list of all active states.\n"
   "@return A list of the form\n"
   "@verbatim\n"
      "stateName1 NL stateName2 NL stateName3 ...\n"
   "@endverbatim\n"
   "where each element is the name of an active state object.\n\n"
   "@tsexample\n"
      "// Disable all active states.\n"
      "foreach$( %state in sfxGetActiveStates() )\n"
      "   %state.disable();\n"
   "@endtsexample\n\n"
   "@ingroup SFX" )
{
   StringBuilder str;
   
   bool isFirst = true;
   for( U32 i = 0; i < sgActiveStates.size(); ++ i )
   {
      if( !isFirst )
         str.append( ' ' );
         
      str.append( sgActiveStates[ i ]->getName() );
      isFirst = false;
   }
   
   return Con::getReturnBuffer( str );
}
