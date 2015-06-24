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
#include "T3D/aiClient.h"

#include "math/mMatrix.h"
#include "T3D/shapeBase.h"
#include "T3D/player.h"
#include "T3D/gameBase/moveManager.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"


IMPLEMENT_CONOBJECT( AIClient );

ConsoleDocClass( AIClient,
   "@brief Simulated client driven by AI commands.\n\n"

   "This object is derived from the AIConnection class. It introduces its own Player object "
   "to solidify the purpose of this class: Simulated client connecting as a player\n\n"
   "To get more specific, if you want a strong alternative to AIPlayer (and wish to make use "
   "of the AIConnection structure), consider AIClient. AIClient inherits from AIConnection, "
   "contains quite a bit of functionality you will find in AIPlayer, and has its own Player "
   "object.\n\n"

   "@note This is a legacy class, which you are discouraged from using as it will "
   "most likely be deprecated in a future version. For now it has been left in for "
   "backwards compatibility with TGE and the old RTS Kit. Use AIPlayer instead.\n\n"

   "@see AIPlayer, AIConnection\n\n"

   "@ingroup AI\n"
   "@ingroup Networking\n"
);

IMPLEMENT_CALLBACK(AIClient, onConnect, void, (const char* idString), (idString),"");

/**
 * Constructor
 */
AIClient::AIClient() {
   mMoveMode = ModeStop;
   mMoveDestination.set( 0.0f, 0.0f, 0.0f );
   mAimLocation.set( 0.0f, 0.0f, 0.0f );
   mMoveSpeed = 1.0f;
   mMoveTolerance = 0.25f;

   // Clear the triggers
   for( S32 i = 0; i < MaxTriggerKeys; i++ )
      mTriggers[i] = false;
 
   mAimToDestination = true;
   mTargetInLOS = false;

   mLocation.set( 0.0f, 0.0f, 0.0f );
   mPlayer = NULL;
}

/**
 * Destructor
 */
AIClient::~AIClient() {
   // Blah
}

/**
 * Sets the object the bot is targeting
 *
 * @param targetObject The object to target
 */
void AIClient::setTargetObject( ShapeBase *targetObject ) {   
   if ( !targetObject || !bool( mTargetObject ) || targetObject->getId() != mTargetObject->getId() )
      mTargetInLOS = false;
   
   mTargetObject = targetObject;
}

/**
 * Returns the target object
 *
 * @return Object bot is targeting
 */
S32 AIClient::getTargetObject() const {
   if( bool( mTargetObject ) )
      return mTargetObject->getId();
   else
      return -1;
}

/**
 * Sets the speed at which this AI moves
 *
 * @param speed Speed to move, default player was 10
 */
void AIClient::setMoveSpeed( F32 speed ) {
   if( speed <= 0.0f )
      mMoveSpeed = 0.0f;
   else
      mMoveSpeed = getMin( 1.0f, speed );
}

/**
 * Sets the movement mode for this AI
 *
 * @param mode Movement mode, see enum
 */
void AIClient::setMoveMode( S32 mode ) {
   if( mode < 0 || mode >= ModeCount )
      mode = 0;

   if( mode != mMoveMode ) {
      switch( mode ) {
         case ModeStuck:
            throwCallback( "onStuck" );
            break;
         case ModeMove:
            if( mMoveMode == ModeStuck )
               throwCallback( "onUnStuck" );
            else
               throwCallback( "onMove" );
            break;
         case ModeStop:
            throwCallback( "onStop" );
            break;
      }
   }

   mMoveMode = mode;
}

/**
 * Sets how far away from the move location is considered
 * "on target"
 * 
 * @param tolerance Movement tolerance for error
 */
void AIClient::setMoveTolerance( const F32 tolerance ) {
   mMoveTolerance = getMax( 0.1f, tolerance );
}

/**
 * Sets the location for the bot to run to
 *
 * @param location Point to run to
 */
void AIClient::setMoveDestination( const Point3F &location ) {
   // Ok, here's the story...we're going to aim where we are going UNLESS told otherwise
   if( mAimToDestination ) {
      mAimLocation = location;
      mAimLocation.z = 0.0f;
   }

   mMoveDestination = location;
}

/**
 * Sets the location for the bot to aim at
 * 
 * @param location Point to aim at
 */
void AIClient::setAimLocation( const Point3F &location ) {
   mAimLocation = location;
   mAimToDestination = false;
}

/**
 * Clears the aim location and sets it to the bot's
 * current destination so he looks where he's going
 */
void AIClient::clearAim() {
   mAimLocation = Point3F( 0.0f, 0.0f, 0.0f );
   mAimToDestination = true;
}

/**
 * This method gets the move list for an object, in the case
 * of the AI, it actually calculates the moves, and then
 * sends them down the pipe.
 *
 * @param movePtr Pointer to move the move list into
 * @param numMoves Number of moves in the move list
 */
U32 AIClient::getMoveList( Move **movePtr,U32 *numMoves ) {
   //initialize the move structure and return pointers
   mMove = NullMove;
   *movePtr = &mMove;
   *numMoves = 1;

   // Check if we got a player
   mPlayer = NULL;
   mPlayer = dynamic_cast<Player *>( getControlObject() );

   // We got a something controling us?
   if( !mPlayer )
      return 1;

   
   // What is The Matrix?
   MatrixF moveMatrix;
   moveMatrix.set( EulerF( 0, 0, 0 ) );
   moveMatrix.setColumn( 3, Point3F( 0, 0, 0 ) );
   moveMatrix.transpose();
      
   // Position / rotation variables
   F32 curYaw, curPitch;
   F32 newYaw, newPitch;
   F32 xDiff, yDiff;

   
   F32 moveSpeed = mMoveSpeed;

   switch( mMoveMode ) {

   case ModeStop:
      return 1;     // Stop means no action
      break;

   case ModeStuck:
      // Fall through, so we still try to move
   case ModeMove:
   
      // Get my location
      MatrixF const& myTransform = mPlayer->getTransform();
      myTransform.getColumn( 3, &mLocation );
   
      // Set rotation variables
      Point3F rotation = mPlayer->getRotation();
      Point3F headRotation = mPlayer->getHeadRotation();
      curYaw = rotation.z;
      curPitch = headRotation.x;
      xDiff = mAimLocation.x - mLocation.x;
      yDiff = mAimLocation.y - mLocation.y;
   
      // first do Yaw
      if( !mIsZero( xDiff ) || !mIsZero( yDiff ) ) {
         // use the cur yaw between -Pi and Pi
         while( curYaw > M_2PI_F )
            curYaw -= M_2PI_F;
         while( curYaw < -M_2PI_F )
            curYaw += M_2PI_F;
      
         // find the new yaw
         newYaw = mAtan2( xDiff, yDiff );
      
         // find the yaw diff 
         F32 yawDiff = newYaw - curYaw;
      
         // make it between 0 and 2PI
         if( yawDiff < 0.0f )
            yawDiff += M_2PI_F;
         else if( yawDiff >= M_2PI_F )
            yawDiff -= M_2PI_F;
      
         // now make sure we take the short way around the circle
         if( yawDiff > M_2PI_F )
            yawDiff -= M_2PI_F;
         else if( yawDiff < -M_2PI_F )
            yawDiff += M_2PI_F;
      
         mMove.yaw = yawDiff;
      
         // set up the movement matrix
         moveMatrix.set( EulerF( 0.0f, 0.0f, newYaw ) );
      }
      else
         moveMatrix.set( EulerF( 0.0f, 0.0f, curYaw ) );
   
      // next do pitch
      F32 horzDist = Point2F( mAimLocation.x, mAimLocation.y ).len();

      if( !mIsZero( horzDist ) ) {
         //we shoot from the gun, not the eye...
         F32 vertDist = mAimLocation.z;
      
         newPitch = mAtan2( horzDist, vertDist ) - ( M_2PI_F / 2.0f );
      
         F32 pitchDiff = newPitch - curPitch;
         mMove.pitch = pitchDiff;
      }
   
      // finally, mMove towards mMoveDestination
      xDiff = mMoveDestination.x - mLocation.x;
      yDiff = mMoveDestination.y - mLocation.y;


      // Check if we should mMove, or if we are 'close enough'
      if( ( ( mFabs( xDiff ) > mMoveTolerance ) || 
            ( mFabs( yDiff ) > mMoveTolerance ) ) && ( !mIsZero( mMoveSpeed ) ) )
      {
         if( mIsZero( xDiff ) )
            mMove.y = ( mLocation.y > mMoveDestination.y ? -moveSpeed : moveSpeed );
         else if( mIsZero( yDiff ) )
            mMove.x = ( mLocation.x > mMoveDestination.x ? -moveSpeed : moveSpeed );
         else if( mFabs( xDiff ) > mFabs( yDiff ) ) {
            F32 value = mFabs( yDiff / xDiff ) * mMoveSpeed;
            mMove.y = ( mLocation.y > mMoveDestination.y ? -value : value );
            mMove.x = ( mLocation.x > mMoveDestination.x ? -moveSpeed : moveSpeed );
         }
         else {
            F32 value = mFabs( xDiff / yDiff ) * mMoveSpeed;
            mMove.x = ( mLocation.x > mMoveDestination.x ? -value : value );
            mMove.y = ( mLocation.y > mMoveDestination.y ? -moveSpeed : moveSpeed );
         }
      
         //now multiply the mMove vector by the transpose of the object rotation matrix
         moveMatrix.transpose();
         Point3F newMove;
         moveMatrix.mulP( Point3F( mMove.x, mMove.y, 0.0f ), &newMove );
      
         //and sub the result back in the mMove structure
         mMove.x = newMove.x;
         mMove.y = newMove.y;

         // We should check to see if we are stuck...
         if( mLocation.x == mLastLocation.x && 
             mLocation.y == mLastLocation.y &&
             mLocation.z == mLastLocation.z ) {

            // We're stuck...probably
            setMoveMode( ModeStuck );
         }
         else
            setMoveMode( ModeMove );
      }
      else {
         // Ok, we are close enough, lets stop

         // setMoveMode( ModeStop ); // DON'T use this, it'll throw the wrong callback
         mMoveMode = ModeStop;
         throwCallback(  "onReachDestination" ); // Callback

      }
      break;
   }

   // Test for target location in sight
   RayInfo dummy;
   Point3F targetLoc = mMoveDestination; // Change this

   if( mPlayer ) {
      if( !mPlayer->getContainer()->castRay( mLocation, targetLoc, 
                                                StaticShapeObjectType | StaticObjectType |
                                                TerrainObjectType, &dummy ) ) {
         if( !mTargetInLOS )
            throwCallback( "onTargetEnterLOS" );
      }
      else {
         if( mTargetInLOS )
            throwCallback( "onTargetExitLOS" );
            
      }
   }
   
   // Copy over the trigger status
   for( S32 i = 0; i < MaxTriggerKeys; i++ ) {
      mMove.trigger[i] = mTriggers[i];
      mTriggers[i] = false;
   }

   return 1;
}

/**
 * This method is just called to stop the bots from running amuck
 * while the mission cycles
 */
void AIClient::missionCycleCleanup() {
   setMoveMode( ModeStop );
}


/**
 * Utility function to throw callbacks
 */
void AIClient::throwCallback( const char *name ) {
   Con::executef( this, name );
}

/**
 * What gets called when this gets created, different from constructor
 */
void AIClient::onAdd( const char *nameSpace ) {

   // This doesn't work...
   //
   if( dStrcmp( nameSpace, mNameSpace->mName ) ) {
      Con::linkNamespaces( mNameSpace->mName, nameSpace );
      mNameSpace = Con::lookupNamespace( nameSpace );
   }

   throwCallback( "onAdd" );
}

// --------------------------------------------------------------------------------------------
// Console Functions
// --------------------------------------------------------------------------------------------

/**
 * Sets the move speed for an AI object
 */
DefineConsoleMethod( AIClient, setMoveSpeed, void, (F32 speed), , "ai.setMoveSpeed( float );" ) 
{
   AIClient *ai = static_cast<AIClient *>( object );
   ai->setMoveSpeed( speed );
}

/**
 * Stops all AI movement, halt!
 */
DefineConsoleMethod( AIClient, stop, void, (),, "ai.stop();" ) 
{
   AIClient *ai = static_cast<AIClient *>( object );
   ai->setMoveMode( AIClient::ModeStop );
}

/**
 * Tells the AI to aim at the location provided
 */
DefineConsoleMethod( AIClient, setAimLocation, void, (Point3F v), , "ai.setAimLocation( x y z );" ) 
{
   AIClient *ai = static_cast<AIClient *>( object );

   ai->setAimLocation( v );
}

/**
 * Tells the AI to move to the location provided
 */
DefineConsoleMethod( AIClient, setMoveDestination, void, (Point3F v), , "ai.setMoveDestination( x y z );" )
{
   AIClient *ai = static_cast<AIClient *>( object );

   ai->setMoveDestination( v );
}

/**
 * Returns the point the AI is aiming at
 */
DefineConsoleMethod( AIClient, getAimLocation, Point3F, (),, "ai.getAimLocation();" ) 
{
   AIClient *ai = static_cast<AIClient *>( object );
   return ai->getAimLocation();
}

/**
 * Returns the point the AI is set to move to
 */
DefineConsoleMethod( AIClient, getMoveDestination, Point3F, (),, "ai.getMoveDestination();" ) 
{
   AIClient *ai = static_cast<AIClient *>( object );
   return ai->getMoveDestination();
}

/**
 * Sets the bots target object
 */
DefineConsoleMethod( AIClient, setTargetObject, void, (const char * objName), , "ai.setTargetObject( obj );" ) 
{
   AIClient *ai = static_cast<AIClient *>( object );
   
   // Find the target
   ShapeBase *targetObject;
   if( Sim::findObject( objName, targetObject ) )
      ai->setTargetObject( targetObject );
   else
      ai->setTargetObject( NULL );
}

/**
 * Gets the object the AI is targeting
 */
DefineConsoleMethod( AIClient, getTargetObject, S32, (),, "ai.getTargetObject();" ) 
{
   AIClient *ai = static_cast<AIClient *>( object );

   return ai->getTargetObject();
}

/**
 * Tells the bot the mission is cycling
 */
DefineConsoleMethod( AIClient, missionCycleCleanup, void, (),, "ai.missionCycleCleanup();" ) 
{
   AIClient *ai = static_cast<AIClient*>( object );
   ai->missionCycleCleanup();
}

/**
 * Sets the AI to run mode
 */
DefineConsoleMethod( AIClient, move, void, (),, "ai.move();" ) 
{
   AIClient *ai = static_cast<AIClient *>( object );
   ai->setMoveMode( AIClient::ModeMove );
}

/**
 * Gets the AI's location in the world
 */
DefineConsoleMethod( AIClient, getLocation, Point3F, (),, "ai.getLocation();" ) 
{
   AIClient *ai = static_cast<AIClient *>( object );
   return ai->getLocation();
}

/**
 * Adds an AI Player to the game
 */
DefineConsoleFunction( aiAddPlayer, S32, (const char * name, const char * ns), (""), "'playerName'[, 'AIClassType'] );")
{
   // Create the player
   AIClient *aiPlayer = new AIClient();
   aiPlayer->registerObject();
   aiPlayer->setGhostFrom(false);
   aiPlayer->setGhostTo(false);
   aiPlayer->setSendingEvents(false);
   aiPlayer->setTranslatesStrings(true);
   aiPlayer->setEstablished();
   
   // Add the connection to the client group
   SimGroup *g = Sim::getClientGroup();
   g->addObject( aiPlayer );


   // Execute the connect console function, this is the same 
   // onConnect function invoked for normal client connections
   aiPlayer->onConnect_callback( name );

   // Now execute the onAdd command and feed it the namespace
   if(dStrcmp( ns,"" ) != 0 )
      aiPlayer->onAdd( ns );
   else
      aiPlayer->onAdd( "AIClient" );

   return aiPlayer->getId();
}


/**
 * Tells the AI to move forward 100 units...TEST FXN
 */
DefineConsoleMethod( AIClient, moveForward, void, (),, "ai.moveForward();" ) 
{
   
   AIClient *ai = static_cast<AIClient *>( object );
   ShapeBase *player = dynamic_cast<ShapeBase*>(ai->getControlObject());
   Point3F location;
   MatrixF const &myTransform = player->getTransform();
   myTransform.getColumn( 3, &location );

   location.y += 100.0f;
   
   ai->setMoveDestination( location );
} // *** /TEST FXN
