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

#include "sfx/sfxParameter.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "console/simSet.h"
#include "math/mMathFn.h"
#include "math/mathTypes.h"
#include "math/mathIO.h"
#include "core/stream/bitStream.h"
#include "platform/typetraits.h"


IMPLEMENT_CONOBJECT( SFXParameter );

ConsoleDocClass( SFXParameter,
   "@brief A sound channel value that can be bound to multiple sound sources.\n\n"
   
   "Parameters are special objects that isolate a specific property that sound sources can have and allows to bind "
   "this isolated instance to multiple sound sources such that when the value of the parameter changes, all sources "
   "bound to the parameter will have their respective property changed.\n\n"
   
   "Parameters are identified and referenced by their #internalName.  This means that the SFXDescription::parameters and "
   "SFXTrack::parameters fields should contain the #internalNames of the SFXParameter objects which should be attached to "
   "the SFXSources when they are created.  No two SFXParameters should have the same #internalName.\n\n"
   
   "All SFXParameter instances are automatically made children of the SFXParameterGroup.\n\n"
   
   "@note To simply control the volume and/or pitch levels of a group of sounds, it is easier and more efficient to use "
      "a sound source group rather than SFXParameters (see @ref SFXSource_hierarchies).  Simply create a SFXSource object representing the group, assign "
      "SFXDescription::sourceGroup of the sounds appropriately, and then set the volume and/or pitch level of the group to "
      "modulate the respective properties of all children.\n\n"
      
   "@section SFXParameter_updates Parameter Updates\n"
   
   "Parameters are periodically allowed to update their own values.  This makes it possible to attach custom logic to a parameter "
   "and have individual parameters synchronize their values autonomously.  Use the onUpdate() callback to attach "
   "script code to a parameter update.\n\n"
   
   "@tsexample\n"
      "new SFXParameter( EngineRPMLevel )\n"
      "{\n"
      "   // Set the name by which this parameter is identified.\n"
      "   internalName = \"EngineRPMLevel\";\n"
      "\n"
      "   // Let this parameter control the pitch of attached sources to simulate engine RPM ramping up and down.\n"
      "   channel = \"Pitch\";\n"
      "\n"
      "   // Start out with unmodified pitch.\n"
      "   defaultValue = 1;\n"
      "\n"
      "   // Add a texture description of what this parameter does.\n"
      "   description = \"Engine RPM Level\";\n"
      "};\n"
      "\n"
      "// Create a description that automatically attaches the engine RPM parameter.\n"
      "singleton SFXDescription( EngineRPMSound : AudioLoop2D )\n"
      "{\n"
      "   parameters[ 0 ] = \"EngineRPMLevel\";\n"
      "};\n"
      "\n"
      "// Create sound sources for the engine.\n"
      "sfxCreateSource( EngineRPMSound, \"art/sound/engine/enginePrimary\" );\n"
      "sfxCreateSource( EngineRPMSound, \"art/sound/engine/engineSecondary\" );\n"
      "\n"
      "// Setting the parameter value will now affect the pitch level of both sound sources.\n"
      "EngineRPMLevel.value = 0.5;\n"
      "EngineRPMLevel.value = 1.5;\n"
   "@endtsexample\n\n"
   
   "@ref SFX_interactive\n\n"
   "@ingroup SFX"
);


IMPLEMENT_CALLBACK( SFXParameter, onUpdate, void, (), (),
   "Called when the sound system triggers an update on the parameter.\n"
   "This occurs periodically during system operation." );


//-----------------------------------------------------------------------------

SFXParameter::SFXParameter()
   : mValue( 1.f ),
     mRange( 0.f, 1.f ),
     mChannel( SFXChannelVolume ),
     mDefaultValue( 1.f )
{
}

//-----------------------------------------------------------------------------

SFXParameter::~SFXParameter()
{
}

//-----------------------------------------------------------------------------

void SFXParameter::initPersistFields()
{
   addGroup( "Sound" );
   
      addProtectedField( "value",         TypeF32,       Offset( mValue, SFXParameter ),
         &SFXParameter::_setValue, &defaultProtectedGetFn,
         "Current value of the audio parameter.\n"
         "All attached sources are notified when this value changes." );
      addProtectedField( "range",         TypePoint2F,   Offset( mRange, SFXParameter ),
         &SFXParameter::_setRange, &defaultProtectedGetFn,
         "Permitted range for #value.\n"
         "Minimum and maximum allowed value for the parameter.  Both inclusive.\n\n"
         "For all but the User0-3 channels, this property is automatically set up by SFXParameter." );
      addProtectedField( "channel",       TYPEID< SFXChannel >(), Offset( mChannel, SFXParameter ),
         &SFXParameter::_setChannel, &defaultProtectedGetFn,
         "Channel that the parameter controls.\n"
         "This controls which property of the sources it is attached to the parameter controls." );
      addProtectedField( "defaultValue",  TypeF32,       Offset( mDefaultValue, SFXParameter ),
         &SFXParameter::_setDefaultValue, &defaultProtectedGetFn,
         "Value to which the parameter is initially set.\n"
         "When the parameter is first added to the system, #value will be set to #defaultValue." );
      addField( "description",            TypeRealString,Offset( mDescription, SFXParameter ),
         "Textual description of the parameter.\n"
         "Primarily for use in the Audio Parameters dialog of the editor to allow for easier identification "
         "of parameters." );
      
   endGroup( "Sound" );
   
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

bool SFXParameter::_setValue( void *object, const char *index, const char *data )
{
   reinterpret_cast< SFXParameter* >( object )->setValue( dAtof( data ) );
   return false;
}

//-----------------------------------------------------------------------------

bool SFXParameter::_setRange( void *object, const char *index, const char *data )
{
   Point2F range = EngineUnmarshallData< Point2F >()( data );
   reinterpret_cast< SFXParameter* >( object )->setRange( range );
   return false;
}

//-----------------------------------------------------------------------------

bool SFXParameter::_setChannel( void *object, const char *index, const char *data )
{
   SFXChannel channel = EngineUnmarshallData< SFXChannel >()( data );
   reinterpret_cast< SFXParameter* >( object )->setChannel( channel );
   return false;
}

//-----------------------------------------------------------------------------

bool SFXParameter::_setDefaultValue( void *object, const char *index, const char *data )
{
   reinterpret_cast< SFXParameter* >( object )->setDefaultValue( dAtof( data ) );
   return false;
}

//-----------------------------------------------------------------------------

bool SFXParameter::onAdd()
{
   if( !Parent::onAdd() )
      return false;
      
   mValue = mDefaultValue;
      
   // Make sure the parameter has a name.
      
   if( !getInternalName() || !getInternalName()[ 0 ] )
   {
      Con::errorf( "SFXParameter::onAdd - %i (%s): parameter object does not have a name", getId(), getName() );
      return false;
   }
      
   // Make sure the parameter has a unique name.
      
   if( find( getInternalName() ) )
   {
      Con::errorf( "SFXParameter::onAdd - %i (%s): a parameter called '%s' already exists", getId(), getName(), getInternalName() );
      return false;
   }
      
   // Add us to the SFXParameter group.
   
   Sim::getSFXParameterGroup()->addObject( this );
      
   return true;
}

//-----------------------------------------------------------------------------

void SFXParameter::onRemove()
{
   mEventSignal.trigger( this, SFXParameterEvent_Deleted );
   Parent::onRemove();
}

//-----------------------------------------------------------------------------

void SFXParameter::update()
{
   onUpdate_callback();
}

//-----------------------------------------------------------------------------

void SFXParameter::reset()
{
   setValue( mDefaultValue );
}

//-----------------------------------------------------------------------------

void SFXParameter::setValue( F32 value )
{
   if( value == mValue )
      return;
      
   mValue = mClampF( value, mRange.x, mRange.y );
   mEventSignal.trigger( this, SFXParameterEvent_ValueChanged );
}

//-----------------------------------------------------------------------------

void SFXParameter::setDefaultValue( F32 value )
{
   mDefaultValue = mClampF( value, mRange.x, mRange.y );
}

//-----------------------------------------------------------------------------

void SFXParameter::setRange( const Point2F& range )
{
   if( range == mRange )
      return;
      
   mRange = range;
   
   F32 value = mClampF( mValue, mRange.x, mRange.y );
   if( value != mValue )
      setValue( value );
      
   mDefaultValue = mClampF( mDefaultValue, mRange.x, mRange.y );
}

//-----------------------------------------------------------------------------

void SFXParameter::setChannel( SFXChannel channel )
{
   if( mChannel == channel )
      return;
      
   mChannel = channel;
   
   F32 value = mValue;
   switch( channel )
   {
      case SFXChannelVolume:
      case SFXChannelConeOutsideVolume:
         setRange( 0.f, 1.0f );
         break;
         
      case SFXChannelConeInsideAngle:
      case SFXChannelConeOutsideAngle:
         setRange( 0.f, 360.f );
         break;
         
      case SFXChannelPitch:
      case SFXChannelMinDistance:
      case SFXChannelMaxDistance:
      case SFXChannelCursor:
         setRange( 0.f, TypeTraits< F32 >::MAX );
         break;
         
      case SFXChannelStatus:
         setRange( F32( SFXStatusPlaying ), F32( SFXStatusStopped ) );
         break;
                     
      default:
         setRange( TypeTraits< F32 >::MIN, TypeTraits< F32 >::MAX );
         break;
   }
   
   // If the range setting did not result in the value already
   // being changed, fire a value-change signal now so that sources
   // can catch on to the new semantics.  Unfortunately, we can't
   // do something about the old semantic's value having been
   // changed by us.
   
   if( mValue == value )
      mEventSignal.trigger( this, SFXParameterEvent_ValueChanged );
}

//-----------------------------------------------------------------------------

SFXParameter* SFXParameter::find( StringTableEntry name )
{
   return dynamic_cast< SFXParameter* >(
      Sim::getSFXParameterGroup()->findObjectByInternalName( name )
   );
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXParameter, getParameterName, String, (),,
   "Get the name of the parameter.\n"
   "@return The paramete name." )
{
   return object->getInternalName();
}

//-----------------------------------------------------------------------------

DefineEngineMethod( SFXParameter, reset, void, (),,
   "Reset the parameter's value to its default.\n"
   "@see SFXParameter::defaultValue\n" )
{
   object->reset();
}
