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

#ifndef _SFXPARAMETER_H_
#define _SFXPARAMETER_H_

#ifndef _SIMOBJECT_H_
   #include "console/simObject.h"
#endif
#ifndef _SFXCOMMON_H_
   #include "sfx/sfxCommon.h"
#endif
#ifndef _TSIGNAL_H_
   #include "core/util/tSignal.h"
#endif
#ifndef _MPOINT2_H_
   #include "math/mPoint2.h"
#endif


/// Enumeration of events triggered by SFXParameters.
enum SFXParameterEvent
{
   /// The parameter value has changed.
   SFXParameterEvent_ValueChanged,
   
   /// The parameter is about to be deleted.
   SFXParameterEvent_Deleted,
};


/// Parameter for interactive audio.
///
/// Parameters are tied to sound sources and will signal value changes so that
/// sound sources may react.
///
/// All parameters are global.  The name of a parameter is its internal object name.
///
/// Like sources, parameters are exclusively client-side.
///
class SFXParameter : public SimObject
{
   public:
   
      typedef SimObject Parent;
      typedef Signal< void( SFXParameter* parameter, SFXParameterEvent event ) > EventSignal;
            
   protected:
   
      /// The current value.
      F32 mValue;
      
      /// The min/max range of the parameter's value.  Both inclusive.
      Point2F mRange;
      
      /// The channel being controlled by this parameter.
      SFXChannel mChannel;
      
      /// Value assigned to the parameter on creation and reset.
      F32 mDefaultValue;
      
      /// Help text.
      String mDescription;
   
      /// The signal used to notify attached sources of parameter events.
      EventSignal mEventSignal;
      
      /// @name Callbacks
      /// @{
      
      DECLARE_CALLBACK( void, onUpdate, () );
      
      /// @}
      
      static bool _setValue( void *object, const char *index, const char *data );
      static bool _setRange( void *object, const char *index, const char *data );
      static bool _setChannel( void *object, const char *index, const char *data );
      static bool _setDefaultValue( void *object, const char *index, const char *data );
   
   public:
   
      SFXParameter();
      
      ~SFXParameter();
      
      /// Look up a parameter by the given name.
      static SFXParameter* find( StringTableEntry name );
      
      /// Update the parameter's value.  The default implementation will invoke a script
      /// 'onUpdate' method if it is defined and do nothing otherwise.
      virtual void update();
      
      /// Reset the parameter's value to its default.
      void reset();
      
      /// Return the current value of this parameter.
      F32 getValue() const { return mValue; }
      
      /// Set the parameter's current value.  Will be clamped against the parameter's valid
      /// value range.  If a value change occurs, a SFXParameterEvent_ValueChange event
      /// is fired.
      void setValue( F32 value );
      
      /// Return the default value of this parameter.  This is the value the parameter
      /// will be set to when it is added to the system.
      F32 getDefaultValue() const { return mDefaultValue; }
      
      /// Set the default value of this parameter.  This is the value the parameter
      /// is set to when it is added to the system.
      void setDefaultValue( F32 value );
      
      /// Return the range of valid values that this parameter may take.
      const Point2F& getRange() const { return mRange; }
      
      /// Set the valid range for the value of this parameter.  Note that both min
      /// and max are inclusive.
      void setRange( const Point2F& range );

      /// Set the valid range for the value of this parameter.  Note that both min
      /// and max are inclusive.
      void setRange( F32 minValue, F32 maxValue ) { setRange( Point2F( minValue, maxValue ) ); }
      
      /// Return the parameter channel that is being affected by this parameter.
      SFXChannel getChannel() const { return mChannel; }
      
      /// Set the parameter channel that is being affected by this parameter.
      void setChannel( SFXChannel channel );
      
      /// Return the description text supplied for this parameter.  This is used to help
      /// identify the purpose of a parameter.
      const String& getDescription() const { return mDescription; }
      
      /// Set the description text for this parameter.  This may be used to help identify
      /// the purpose of a parameter.
      void setDescription( const String& str ) { mDescription = str; }
      
      /// Return the event signal for this parameter.
      EventSignal& getEventSignal() { return mEventSignal; }
      
      // SimObject.
      virtual bool onAdd();
      virtual void onRemove();
      
      static void initPersistFields();
   
      DECLARE_CONOBJECT( SFXParameter );
      DECLARE_CATEGORY( "SFX" );
      DECLARE_DESCRIPTION( "" );
};

#endif // !_SFXPARAMETER_H_

