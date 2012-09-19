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
#include "T3D/physics/physicsEvents.h"

#include "math/mathIO.h"
#include "core/stream/bitStream.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsWorld.h"
#include "scene/sceneObject.h"
#include "T3D/gameBase/gameConnection.h"
#include "console/engineAPI.h"


RadialImpulseEvent::RadialImpulseEvent()
 : mPosition( 0, 0, 0 ),
   mRadius( 0 ),
   mMagnitude( 0 )
{            
}

RadialImpulseEvent::RadialImpulseEvent( const Point3F &pos, F32 radius, F32 magnitude )
 :  mPosition( pos ),
    mRadius( radius ),
    mMagnitude( magnitude )
{
}

RadialImpulseEvent::~RadialImpulseEvent()
{ 
}

void RadialImpulseEvent::pack( NetConnection* /*ps*/, BitStream *bstream )
{       
   mathWrite( *bstream, mPosition );
   bstream->write( mRadius );
   bstream->write( mMagnitude );
}

void RadialImpulseEvent::write( NetConnection*, BitStream *bstream )
{ 
   mathWrite( *bstream, mPosition );
   bstream->write( mRadius );
   bstream->write( mMagnitude );
}
   
void RadialImpulseEvent::unpack( NetConnection *ps, BitStream *bstream )
{      
   mathRead( *bstream, &mPosition );
   bstream->read( &mRadius );
   bstream->read( &mMagnitude );
}
   
void RadialImpulseEvent::process(NetConnection *con)
{ 
   impulse( &gClientContainer, mPosition, mRadius, mMagnitude );   
}

void RadialImpulseEvent::_impulseCallback( SceneObject *obj, void *key )
{
   ImpulseInfo *info = (ImpulseInfo*)key; 
   obj->applyRadialImpulse( info->pos, info->radius, info->magnitude );
}

void RadialImpulseEvent::impulse( SceneContainer *con, const Point3F &position, F32 radius, F32 magnitude )
{
   Point3F offset( radius, radius, radius );   
   Box3F bounds( position - offset, position + offset );      
   
   ImpulseInfo info;
   info.pos = position;
   info.radius = radius;
   info.magnitude = magnitude;

   con->findObjects( bounds, -1, _impulseCallback, &info );
}

IMPLEMENT_CO_NETEVENT_V1( RadialImpulseEvent );

ConsoleDocClass( RadialImpulseEvent,
   "@brief Creates a physics-based impulse effect from a defined central point and magnitude.\n\n"
   "@see RadialImpulseEvent::send\n"
   "@ingroup Physics\n"
);


DefineEngineStaticMethod(RadialImpulseEvent, send, void, (const char* inPosition, F32 radius, F32 magnitude), ("1.0 1.0 1.0", 10.0f, 20.0f),
					 "@brief Applies a radial impulse to any SceneObjects within the area of effect.\n\n"
					 "This event is performed both server and client-side.\n\n"
					 "@param position Center point for this radial impulse.\n"
					 "@param radius Distance from the position for this radial impulse to affect.\n"
					 "@param magnitude The force applied to objects within the radius from the position of this radial impulse effect.\n\n"
                "@tsexample\n"
                  "// Define the Position\n"
                  "%position = \"10.0 15.0 10.0\";\n\n"
                  "// Define the Radius\n"
                  "%radius = \"25.0\";\n\n"
                  "// Define the Magnitude\n"
                  "%magnitude = \"30.0\"\n\n"
                  "// Create a globalRadialImpulse physics effect.\n"
                  "RadialImpulseEvent::send(%position,%radius,%magnitude);\n"
                "@endtsexample\n\n")
{
   // Scan out arguments...
   Point3F position;

   dSscanf( inPosition, "%f %f %f", &position.x, &position.y, &position.z );

   // Apply server-side.
   RadialImpulseEvent::impulse( &gServerContainer, position, radius, magnitude );

   // Transmit event to each client to perform client-side...

   SimGroup *pClientGroup = Sim::getClientGroup();
   if ( !pClientGroup )
   {
      Con::errorf( "globalRadialImpulse() - Client group not found!" );
      return;
   }
   
   SimGroup::iterator itr = pClientGroup->begin();
   for ( ; itr != pClientGroup->end(); itr++ )
   {
      GameConnection* gc = static_cast<GameConnection*>(*itr);
      if ( gc )
         gc->postNetEvent( new RadialImpulseEvent( position, radius, magnitude ) );
   }
}
