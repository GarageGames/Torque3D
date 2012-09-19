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

#include "platform/platform.h"
#include "environment/timeOfDay.h"

#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "T3D/gameBase/gameConnection.h"
#include "environment/sun.h"
#include "console/engineAPI.h"


TimeOfDayUpdateSignal TimeOfDay::smTimeOfDayUpdateSignal;

IMPLEMENT_CO_NETOBJECT_V1(TimeOfDay);

ConsoleDocClass( TimeOfDay,
   "@brief Environmental object that triggers a day/night cycle in level.\n\n"

   "@note TimeOfDay only works in Advanced Lighting with a Sub object or ScatterSky\n\n"

   "@tsexample\n"
   "new TimeOfDay(tod)\n"
   "{\n"
   "   axisTilt = \"23.44\";\n"
   "   dayLength = \"120\";\n"
   "   startTime = \"0.15\";\n"
   "   time = \"0.15\";\n"
   "   play = \"0\";\n"
   "   azimuthOverride = \"572.958\";\n"
   "   dayScale = \"1\";\n"
   "   nightScale = \"1.5\";\n"
   "   position = \"598.399 550.652 196.297\";\n"
   "   rotation = \"1 0 0 0\";\n"
   "   scale = \"1 1 1\";\n"
   "   canSave = \"1\";\n"
   "   canSaveDynamicFields = \"1\";\n"
   "};\n"
   "@endtsexample\n\n"
   "@ingroup enviroMisc"
);

TimeOfDay::TimeOfDay() 
   :  mElevation( 0.0f ),
      mAzimuth( 0.0f ),
      mAxisTilt( 23.44f ),       // 35 degree tilt
      mDayLen( 120.0f ),         // 2 minutes
      mStartTimeOfDay( 0.5f ),   // High noon
      mTimeOfDay( 0.0f ),        // initialized to StartTimeOfDay in onAdd
      mPlay( true ),
      mDayScale( 1.0f ),
      mNightScale( 1.5f ),
      mAnimateTime( 0.0f ),
      mAnimateSpeed( 0.0f ),
      mAnimate( false )
{
   mNetFlags.set( Ghostable | ScopeAlways );
   mTypeMask = EnvironmentObjectType;

   // Sets the sun vector directly overhead for lightmap generation
   // The value of mSunVector is grabbed by the terrain lighting stuff.
   /*
   F32 ele, azi;
   ele = azi = TORADIANS(90);
   MathUtils::getVectorFromAngles(mSunVector, azi, ele);
   */
   mPrevElevation = 0;
   mNextElevation = 0;
   mAzimuthOverride = 1.0f;

   _initColors();
}

TimeOfDay::~TimeOfDay()
{
}

bool TimeOfDay::setTimeOfDay( void *object, const char *index, const char *data )
{
   TimeOfDay *tod = static_cast<TimeOfDay*>(object);
   tod->setTimeOfDay( dAtof( data ) );

   return false;
}

bool TimeOfDay::setPlay( void *object, const char *index, const char *data )
{
   TimeOfDay *tod = static_cast<TimeOfDay*>(object);
   tod->setPlay( dAtob( data ) );

   return false;
}

bool TimeOfDay::setDayLength( void *object, const char *index, const char *data )
{
   TimeOfDay *tod = static_cast<TimeOfDay*>(object);
   F32 length = dAtof( data );
   if( length != 0 )
      tod->setDayLength( length );

   return false;

}

void TimeOfDay::initPersistFields()
{
	  addGroup( "TimeOfDay" );

      addField( "axisTilt", TypeF32, Offset( mAxisTilt, TimeOfDay ),
            "The angle in degrees between global equator and tropic." );

      addProtectedField( "dayLength", TypeF32, Offset( mDayLen, TimeOfDay ), &setDayLength, &defaultProtectedGetFn,
            "The length of a virtual day in real world seconds." );

      addField( "startTime", TypeF32, Offset( mStartTimeOfDay, TimeOfDay ),
         "" );

      addProtectedField( "time", TypeF32, Offset( mTimeOfDay, TimeOfDay ), &setTimeOfDay, &defaultProtectedGetFn, "Current time of day." );

      addProtectedField( "play", TypeBool, Offset( mPlay, TimeOfDay ), &setPlay, &defaultProtectedGetFn, "True when the TimeOfDay object is operating." );

      addField( "azimuthOverride", TypeF32, Offset( mAzimuthOverride, TimeOfDay ), "" );

      addField( "dayScale", TypeF32, Offset( mDayScale, TimeOfDay ), "Scalar applied to time that elapses while the sun is up." );

      addField( "nightScale", TypeF32, Offset( mNightScale, TimeOfDay ), "Scalar applied to time that elapses while the sun is down." );

   endGroup( "TimeOfDay" );

	Parent::initPersistFields();
}

void TimeOfDay::consoleInit()
{
   Parent::consoleInit();

   //addVariable( "$TimeOfDay::currentTime", &TimeOfDay::smCurrentTime );
   //addVariable( "$TimeOfDay::timeScale", TypeF32, &TimeOfDay::smTimeScale );
}

void TimeOfDay::inspectPostApply()
{
   _updatePosition();
   setMaskBits( OrbitMask );
}

void TimeOfDay::_onGhostAlwaysDone()
{
   _updatePosition();
}

bool TimeOfDay::onAdd()
{
   if ( !Parent::onAdd() )
      return false;
   
   // The server initializes to the specified starting values.
   // The client initializes itself to the server time from
   // unpackUpdate.
   if ( isServerObject() )
   {
      mTimeOfDay = mStartTimeOfDay;
      _updatePosition();
   }

   // We don't use a bounds.
   setGlobalBounds();
   resetWorldBox();
   addToScene();

   // Lets receive ghost events so we can resolve
   // the sun object.
   if ( isClientObject() )
      NetConnection::smGhostAlwaysDone.notify( this, &TimeOfDay::_onGhostAlwaysDone );

   if ( isServerObject() )   
      Con::executef( this, "onAdd" );   

   setProcessTick( true );

   return true;
}

void TimeOfDay::onRemove()
{
   if ( isClientObject() )
      NetConnection::smGhostAlwaysDone.remove( this, &TimeOfDay::_onGhostAlwaysDone );

   removeFromScene();
   Parent::onRemove();
}

U32 TimeOfDay::packUpdate(NetConnection *conn, U32 mask, BitStream *stream )
{
   U32 retMask = Parent::packUpdate( conn, mask, stream );

   if ( stream->writeFlag( mask & OrbitMask ) )
   {
      stream->write( mStartTimeOfDay );
      stream->write( mDayLen );
      stream->write( mTimeOfDay );
      stream->write( mAxisTilt );
      stream->write( mAzimuthOverride );

      stream->write( mDayScale );
      stream->write( mNightScale );

      stream->writeFlag( mPlay );
   }

   if ( stream->writeFlag( mask & AnimateMask ) )
   {
      stream->write( mAnimateTime );
      stream->write( mAnimateSpeed );
   }

   return retMask;
}

void TimeOfDay::unpackUpdate( NetConnection *conn, BitStream *stream )
{
   Parent::unpackUpdate( conn, stream );

   if ( stream->readFlag() ) // OrbitMask
   {
      stream->read( &mStartTimeOfDay );
      stream->read( &mDayLen );
      stream->read( &mTimeOfDay );
      stream->read( &mAxisTilt );
      stream->read( &mAzimuthOverride );

      stream->read( &mDayScale );
      stream->read( &mNightScale );

      mPlay = stream->readFlag();

      _updatePosition();
   }

   if ( stream->readFlag() ) // AnimateMask
   {
      F32 time, speed;
      stream->read( &time );
      stream->read( &speed );

      if( isProperlyAdded() )
         animate( time, speed );
   }
}

void TimeOfDay::processTick( const Move *move )
{
   if ( mAnimate )
   {
      F32 current = mTimeOfDay * 360.0f;      
      F32 next = current + (mAnimateSpeed * TickSec);

      // Protect for wrap around.
      while ( next > 360.0f )
         next -= 360.0f;

      // Clamp to make sure we don't pass the target time.
      if ( next >= mAnimateTime )
      {
         next = mAnimateTime;
         mAnimate = false;
      }

      // Set the new time of day.
      mTimeOfDay = next / 360.0f;

      _updatePosition();
      _updateTimeEvents();

      if ( !mAnimate && isServerObject() )
         Con::executef( this, "onAnimateDone" );
   }
   else if ( mPlay )
   {
      F32 dt = TickSec;
      F32 current = mRadToDeg( mNextElevation );

      if ( current > 350.0f || ( 0.0f <= current && current < 190.0f ) )
         dt *= mDayScale;
      else
         dt *= mNightScale;

      mTimeOfDay += dt / mDayLen;

      // It could be possible for more than a full day to 
      // pass in a single advance time, so I put this inside a loop
      // but timeEvents will not actually be called for the
      // skipped day.
      while ( mTimeOfDay > 1.0f )
         mTimeOfDay -= 1.0f;

      _updatePosition();
      _updateTimeEvents();
   }
   else
      _updatePosition();
}

void TimeOfDay::_updatePosition()
{
   mPrevElevation = mNextElevation;

   if ( mFabs( mAzimuthOverride ) )
   {
      mElevation = mDegToRad( mTimeOfDay * 360.0f );
      mAzimuth = mAzimuthOverride;

      mNextElevation = mElevation;  // already normalized
   }
   else
   {
      //// Full azimuth/elevation calculation.
      //// calculate sun decline and meridian angle (in radians)
      //F32 sunDecline = mSin( M_2PI * mTimeOfYear ) * mDegToRad( mAxisTilt );
      //F32 meridianAngle = mTimeOfDay * M_2PI - mDegToRad( mLongitude );

      //// calculate the elevation and azimuth (in radians)
      //mElevation = _calcElevation( mDegToRad( mLatitude ), sunDecline, meridianAngle );
      //mAzimuth = _calcAzimuth( mDegToRad( mLatitude ), sunDecline, meridianAngle );

      // Simplified azimuth/elevation calculation.
      // calculate sun decline and meridian angle (in radians)
      F32 sunDecline = mDegToRad( mAxisTilt );
      F32 meridianAngle = mTimeOfDay * M_2PI;

      // calculate the elevation and azimuth (in radians)
      mElevation = _calcElevation( 0.0f, sunDecline, meridianAngle );
      mAzimuth = _calcAzimuth( 0.0f, sunDecline, meridianAngle );

      // calculate 'normalized' elevation (0=sunrise, PI/2=zenith, PI=sunset, 3PI/4=nadir)
      F32 normElevation = M_PI_F * mElevation / ( 2 * _calcElevation( 0.0f, sunDecline, 0.0f ) );
      if ( mAzimuth > M_PI_F )
         normElevation = M_PI_F - normElevation;
      else if ( mElevation < 0 )
         normElevation = M_2PI_F + normElevation;

      mNextElevation = normElevation;
   }

   // Only the client updates the sun position!
   if ( isClientObject() )
      smTimeOfDayUpdateSignal.trigger( this, mTimeOfDay );
}

F32 TimeOfDay::_calcElevation( F32 lat, F32 dec, F32 mer )
{
   return mAsin( mSin(lat) * mSin(dec) + mCos(lat) * mCos(dec) * mCos(mer) );
}

F32 TimeOfDay::_calcAzimuth( F32 lat, F32 dec, F32 mer )
{
   // Add PI to normalize this from the range of -PI/2 to PI/2 to 0 to 2 * PI;
	  return mAtan2( mSin(mer), mCos(mer) * mSin(lat) - mTan(dec) * mCos(lat) ) + M_PI_F;
}

void TimeOfDay::_getSunColor( ColorF *outColor ) const
{
	  const COLOR_TARGET *ct = NULL;

   F32 ele = mClampF( M_2PI_F - mNextElevation, 0.0f, M_PI_F );
	  F32 phase = -1.0f;
	  F32 div;

   if (!mColorTargets.size())
   {
      outColor->set(1.0f,1.0f,1.0f);
      return;
   }

   if (mColorTargets.size() == 1)
   {
      ct = &mColorTargets[0];
      outColor->set(ct->color.red, ct->color.green, ct->color.blue);
      return;
   }

   //simple check
   if ( mColorTargets[0].elevation != 0.0f )
   {
      AssertFatal(0, "TimeOfDay::GetColor() - First elevation must be 0.0 radians")
      outColor->set(1.0f, 1.0f, 1.0f);
      //mBandMod = 1.0f;
      //mCurrentBandColor = color;
      return;
   }

   if ( mColorTargets[mColorTargets.size()-1].elevation != M_PI_F )
   {
      AssertFatal(0, "Celestails::GetColor() - Last elevation must be PI")
      outColor->set(1.0f, 1.0f, 1.0f);
      //mBandMod = 1.0f;
      //mCurrentBandColor = color;
      return;
   }

   //we need to find the phase and interp... also loop back around
   U32 count=0;
   for (;count < mColorTargets.size() - 1; count++)
   {
      const COLOR_TARGET *one = &mColorTargets[count];
      const COLOR_TARGET *two = &mColorTargets[count+1];

      if (ele >= one->elevation && ele <= two->elevation)
      {
			      div = two->elevation - one->elevation;
			
         //catch bad input divide by zero
         if ( mFabs( div ) < 0.01f )
            div = 0.01f;
			
			      phase = (ele - one->elevation) / div;
			      outColor->interpolate( one->color, two->color, phase );

			      //mCurrentBandColor.interpolate(one->bandColor, two->bandColor, phase);
			      //mBandMod = one->bandMod * (1.0f - phase) + two->bandMod * phase;

			      return;
		    }
	  }

	  AssertFatal(0,"This isn't supposed to happen");
}

void TimeOfDay::_initColors()
{
   // NOTE: The elevation targets represent distances 
   // from PI/2 radians (strait up).

   ColorF c;
   ColorF bc;

   // e is for elevation
   F32 e = M_PI_F / 13.0f; // (semicircle in radians)/(number of color target entries);

   // Day
   c.set(1.0f,1.0f,1.0f);
   _addColorTarget(0, c, 1.0f, c); // High noon at equanox
   c.set(.9f,.9f,.9f);
   _addColorTarget(e * 1.0f, c, 1.0f, c);
   c.set(.9f,.9f,.9f);
   _addColorTarget(e * 2.0f, c, 1.0f, c);
   c.set(.8f,.75f,.75f);
   _addColorTarget(e * 3.0f, c, 1.0f, c);
   c.set(.7f,.65f,.65f);
   _addColorTarget(e * 4.0f, c, 1.0f, c);

   //Dawn and Dusk (3 entries)
   c.set(.7f,.65f,.65f);
   bc.set(.8f,.6f,.3f);
   _addColorTarget(e * 5.0f, c, 3.0f, bc);
   c.set(.65f,.54f,.4f);
   bc.set(.75f,.5f,.4f);
   _addColorTarget(e * 6.0f, c, 2.75f, bc);
   c.set(.55f,.45f,.25f);
   bc.set(.65f,.3f,.3f);
   _addColorTarget(e * 7.0f, c, 2.5f, bc);

   //NIGHT
   c.set(.3f,.3f,.3f);
   bc.set(.7f,.4f,.2f);
   _addColorTarget(e * 8.0f, c, 1.25f, bc);
   c.set(.25f,.25f,.3f);
   bc.set(.8f,.3f,.2f);
   _addColorTarget(e * 9.0f, c, 1.00f, bc);
   c.set(.25f,.25f,.4f);
   _addColorTarget(e * 10.0f, c, 1.0f, c);
   c.set(.2f,.2f,.35f);
   _addColorTarget(e * 11.0f, c, 1.0f, c);
   c.set(.15f,.15f,.2f);
   _addColorTarget(M_PI_F, c, 1.0f, c); // Midnight at equanox.
}

void TimeOfDay::_addColorTarget( F32 ele, const ColorF &color, F32 bandMod, const ColorF &bandColor )
{
   COLOR_TARGET  newTarget;

   newTarget.elevation = ele;
   newTarget.color = color;
   newTarget.bandMod = bandMod;
   newTarget.bandColor = bandColor;

   mColorTargets.push_back(newTarget);
}

void TimeOfDay::_updateTimeEvents()
{
   if ( mTimeEvents.empty() )
      return;

   // Get the prev, next elevation in degrees since TimeOfDayEvent is specified
   // in degrees.
   F32 prevElevation = mRadToDeg( mPrevElevation );
   F32 nextElevation = mRadToDeg( mNextElevation );

   // If prevElevation is less than nextElevation then its the next day.
   // Unroll it so we can just loop forward in time and simplify our loop.
   if ( nextElevation < prevElevation )
      nextElevation += 360.0f;

   const U32 evtCount = mTimeEvents.size();

   // Find where in the event list we need to start...
   // The first timeEvent with elevation greater than our previous elevation.
   
   U32 start = 0;   
   for ( ; start < evtCount; start++ )
   {
      if ( mTimeEvents[start].triggerElevation > prevElevation )
         break;
   }

   bool onNextDay = false;

   // Nothing between prevElevation and the end of the day...
   // Check between start of the day and nextElevation...
   if ( start == evtCount )
   {
      start = 0;
      for ( ; start < evtCount; start++ )
      {
         if ( mTimeEvents[start].triggerElevation <= nextElevation )
         {
            onNextDay = true;
            break;
         }
      }
   }

   // No events were hit...
   if ( start == evtCount )
      return;

   U32 itr = start;
   while ( true )
   {
      TimeOfDayEvent &timeEvent = mTimeEvents[itr];
      
      F32 elev = timeEvent.triggerElevation;
      if ( onNextDay )
         elev += 360.0f;

      // Hit an event that happens later after nextElevation so we
      // have checked everything within the range and are done.
      if ( elev > nextElevation )
         break;

      // If its not greater than the nextElevation it must be less, and if
      // we are here we already know its greater than prevElevation.
      
      AssertFatal( elev >= prevElevation && elev <= nextElevation, "TimeOfDay::_updateTimeEvents - Logical error in here!" );
      AssertFatal( !timeEvent.deleteMe, "TimeOfDay::_updateTimeEvents - tried to fire the same event twice!" );

      _onTimeEvent( timeEvent.identifier );

      if ( timeEvent.oneShot )
         timeEvent.deleteMe = true;

      // On to the next time event...
      itr++;

      // We hit the end of the day?
      if ( itr == evtCount )
      {
         // We are already on the next day so we have checked everything.
         if ( onNextDay )
            break;
         // Check events for the next day
         else
         {            
            itr = 0;
            onNextDay = true;
         }         
      }
   }

   // Cleanup one-shot events that fired...

   for ( S32 i = 0; i < mTimeEvents.size(); i++ )
   {
      if ( mTimeEvents[i].deleteMe )
      {
         // Don't use erase_fast, there are ordered.
         mTimeEvents.erase( i );
         i--;
      }
   }
}

void TimeOfDay::addTimeEvent( F32 triggerElevation, const UTF8 *identifier )
{
   // Insert in ascending order of elevation.
   // Note that having more than one TimeEvent with the same triggerElevation
   // may cause undefined behavior.

   TimeOfDayEvent *pEvent = NULL;
   
   if ( mTimeEvents.empty() || mTimeEvents.last().triggerElevation <= triggerElevation )
   {
      mTimeEvents.increment();
      pEvent = &mTimeEvents.last();
   }
   else 
   {   
      for ( S32 i = 0; i < mTimeEvents.size(); i++ )
      {
         if ( mTimeEvents[i].triggerElevation > triggerElevation )
         {
            mTimeEvents.insert( i );
            pEvent = &mTimeEvents[i];
            break;
         }
      }
   }

   AssertFatal( pEvent, "TimeOfDay::addTimeEvent - could not find place to insert event." );

   pEvent->triggerElevation = triggerElevation;
   pEvent->identifier = identifier;
   pEvent->oneShot = false;
      
   pEvent->deleteMe = false;
}

void TimeOfDay::setTimeOfDay( F32 time )
{ 
   mTimeOfDay = time;

   while ( mTimeOfDay > 1.0f )
      mTimeOfDay -= 1.0f;
   while ( mTimeOfDay < 0.0f )
      mTimeOfDay += 1.0f;

   _updatePosition();

   //if ( isServerObject() )
   _updateTimeEvents();

   setMaskBits( OrbitMask ); 
}

void TimeOfDay::_onTimeEvent( const String &identifier )
{
   // Client doesn't do onTimeEvent callbacks.
   if ( isClientObject() )
      return;

   String strCurrentTime = String::ToString( "%g", mTimeOfDay );

   F32 elevation = mRadToDeg( mNextElevation );
   while( elevation < 0 )
      elevation += 360.0f;
   while( elevation > 360.0f )
      elevation -= 360.0f;

   String strCurrentElevation = String::ToString( "%g", elevation );

   Con::executef( this, "onTimeEvent", identifier.c_str(), strCurrentTime.c_str(), strCurrentElevation.c_str() );
}

void TimeOfDay::animate( F32 time, F32 speed )
{
   // Stop any existing animation... this one
   // becomes the new one.
   mAnimate = false;

   // Set the target time to hit.
   mAnimateTime = mClamp(time, 0.0f, 360.0f);

   F32 current = mTimeOfDay * 360.0f;
   F32 target = mAnimateTime;
   if ( target < current )
      target += 360.0f;

   // If we're already at the current time then
   // we have nothing more to do... the animation is here.
   F32 dif = target - current;
   if ( mIsZero( dif ) )
      return;

   // Start playback.
   mAnimateSpeed = speed;
   mAnimate = true;

   if ( isServerObject() )
   {
      Con::executef( this, "onAnimateStart" );
      setMaskBits( AnimateMask );
   }
}

DefineEngineMethod( TimeOfDay, addTimeOfDayEvent, void, (F32 elevation, const char *identifier ),,
   "" )
{
   object->addTimeEvent( elevation, identifier );
}

DefineEngineMethod( TimeOfDay, setTimeOfDay, void, ( F32 time ),,
   "" )
{
   object->setTimeOfDay( time );
}

DefineEngineMethod( TimeOfDay, setPlay, void, ( bool enabled ),,
   "")
{
   object->setPlay( enabled );
}

DefineEngineMethod( TimeOfDay, setDayLength, void, ( F32 seconds ),,
   "" )
{
   if ( seconds > 0.0f )
      object->setDayLength( seconds );
}

DefineEngineMethod( TimeOfDay, animate, void, ( F32 elevation, F32 degreesPerSecond ),,
   "")
{
   object->animate( elevation, degreesPerSecond );
}
