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

#ifndef _PHYSICSEVENTS_H_
#define _PHYSICSEVENTS_H_

#ifndef _NETCONNECTION_H_
#include "sim/netConnection.h"
#endif

class SceneObject;
class SceneContainer;


/// When this NetEvent is processed on the client-side it 
/// applies a radial impulse to objects in the physics 
/// simulation.
class RadialImpulseEvent : public NetEvent
{
   typedef NetEvent Parent;

protected:

   struct ImpulseInfo
   {
      Point3F pos;
      F32 radius;
      F32 magnitude;
   };

   Point3F mPosition;
   F32 mRadius;
   F32 mMagnitude;

   static void _impulseCallback( SceneObject *obj, void *key );

public:

   RadialImpulseEvent();      
   RadialImpulseEvent( const Point3F &pos, F32 radius, F32 magnitude );
   ~RadialImpulseEvent();   

   virtual void pack( NetConnection* /*ps*/, BitStream *bstream );   
   virtual void write( NetConnection*, BitStream *bstream );   
   virtual void unpack( NetConnection *ps, BitStream *bstream );   
   virtual void process(NetConnection *);
   
   static void impulse( SceneContainer *con, const Point3F &position, F32 radius, F32 magnitude );

   DECLARE_CONOBJECT( RadialImpulseEvent );
};


#endif // _PHYSICSEVENTS_H_