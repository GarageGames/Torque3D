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

#ifndef _AICLIENT_H_
#define _AICLIENT_H_

#ifndef _AICONNECTION_H_
#include "T3D/aiConnection.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

class ShapeBase;
class Player;

class AIClient : public AIConnection {

	typedef AIConnection Parent;

	private:
		enum {
			FireTrigger = 0,
			JumpTrigger = 2,
			JetTrigger = 3,
			GrenadeTrigger = 4,
			MineTrigger = 5
		};

		F32 mMoveSpeed;
		S32 mMoveMode;
		F32 mMoveTolerance; // How close to the destination before we stop

		bool mTriggers[MaxTriggerKeys];

		Player *mPlayer;

		Point3F mMoveDestination;
		Point3F mLocation;
      Point3F mLastLocation; // For stuck check

		bool mAimToDestination;
		Point3F mAimLocation;
      bool mTargetInLOS;
		
		SimObjectPtr<ShapeBase> mTargetObject;

      // Utility Methods
      void throwCallback( const char *name );
	public:
		
		DECLARE_CONOBJECT( AIClient );

	   DECLARE_CALLBACK( void, onConnect, (const char* idString) );

		enum {
			ModeStop = 0,
			ModeMove,
			ModeStuck,
			ModeCount		// This is in there as a max index value
		};

		AIClient();
      ~AIClient();

		U32 getMoveList( Move **movePtr,U32 *numMoves );

		// ---Targeting and aiming sets/gets
		void setTargetObject( ShapeBase *targetObject );
		S32 getTargetObject() const;

		// ---Movement sets/gets
		void setMoveSpeed( const F32 speed );
		F32 getMoveSpeed() const { return mMoveSpeed; }

		void setMoveMode( S32 mode );
		S32 getMoveMode() const { return mMoveMode; }

		void setMoveTolerance( const F32 tolerance );
		F32 getMoveTolerance() const { return mMoveTolerance; }

		void setMoveDestination( const Point3F &location );
		Point3F getMoveDestination() const { return mMoveDestination; }

      Point3F getLocation() const { return mLocation; }

		// ---Facing(Aiming) sets/gets
		void setAimLocation( const Point3F &location );
		Point3F getAimLocation() const { return mAimLocation; }
		void clearAim();

		// ---Other
		void missionCycleCleanup();
      void onAdd( const char *nameSpace );
};

#endif
