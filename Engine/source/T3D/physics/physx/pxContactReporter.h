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

#ifndef _PXCONTACTREPORTER_H_
#define _PXCONTACTREPORTER_H_

#ifndef _PHYSX_H_
#include "T3D/physics/physX/px.h"
#endif


class PxContactReporter : public NxUserContactReport
{
protected:

   virtual void onContactNotify( NxContactPair& pair, NxU32 events );

public:

   PxContactReporter();
   virtual ~PxContactReporter();
};



class PxUserNotify : public NxUserNotify
{
public:
   virtual bool onJointBreak( NxReal breakingForce, NxJoint &brokenJoint );   
   virtual void onWake( NxActor **actors, NxU32 count ) {}
   virtual void onSleep ( NxActor **actors, NxU32 count ) {}
};


#endif // _PXCONTACTREPORTER_H_
