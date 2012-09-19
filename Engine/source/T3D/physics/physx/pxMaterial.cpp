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
#include "T3D/physics/physX/px.h"

#include "T3D/physics/physX/pxMaterial.h"

#include "T3D/physics/physX/pxWorld.h"
#include "T3D/physics/physicsPlugin.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"


IMPLEMENT_CO_DATABLOCK_V1( PxMaterial );

ConsoleDocClass( PxMaterial,
   
   "@brief Defines a PhysX material assignable to a PxMaterial.\n\n"

   "When two actors collide, the collision behavior that results depends on the material properties "
   "of the actors' surfaces. For example, the surface properties determine if the actors will or will "
   "not bounce, or if they will slide or stick. Currently, the only special feature supported by materials "
   "is anisotropic friction, but according to Nvidia, other effects such as moving surfaces and more types "
   "of friction are slotted for future release.\n\n"

   "For more information, refer to Nvidia's PhysX docs.\n\n"

   "@ingroup Physics"
);

PxMaterial::PxMaterial()
: mNxMat( NULL ),
  mNxMatId( -1 ),
  restitution( 0.0f ),
  staticFriction( 0.1f ),
  dynamicFriction( 0.95f ),
  mServer( false )
{
}

PxMaterial::~PxMaterial()
{
}

void PxMaterial::consoleInit()
{
   Parent::consoleInit();
}

void PxMaterial::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("PxMaterial");		

      addField( "restitution", TypeF32, Offset( restitution, PxMaterial ),
         "@brief Coeffecient of a bounce applied to the shape in response to a collision.\n\n"
         "A value of 0 makes the object bounce as little as possible, while higher values up to 1.0 result in more bounce.\n\n"
         "@note Values close to or above 1.0 may cause stability problems and/or increasing energy.");
      addField( "staticFriction", TypeF32, Offset( staticFriction, PxMaterial ),
         "@brief Coefficient of static %friction to be applied.\n\n" 
         "Static %friction determines the force needed to start moving an at-rest object in contact with a surface. "
         "If the force applied onto shape cannot overcome the force of static %friction, the shape will remain at rest. "
         "A higher coefficient will require a larger force to start motion. "
         "@note This value should be larger than 0.\n\n");
      addField( "dynamicFriction", TypeF32,	Offset( dynamicFriction, PxMaterial ),
         "@brief Coefficient of dynamic %friction to be applied.\n\n" 
         "Dynamic %friction reduces the velocity of a moving object while it is in contact with a surface. "
         "A higher coefficient will result in a larger reduction in velocity. "
         "A shape's dynamicFriction should be equal to or larger than 0.\n\n");

   endGroup("PxMaterial");		
}

void PxMaterial::onStaticModified( const char *slotName, const char *newValue )
{
   if ( isProperlyAdded() && mNxMat != NULL )
   {      
      mNxMat->setRestitution( restitution );
      mNxMat->setStaticFriction( staticFriction );
      mNxMat->setDynamicFriction( dynamicFriction );
   }
}

bool PxMaterial::preload( bool server, String &errorBuffer )
{
   mServer = server;

   PxWorld *world = dynamic_cast<PxWorld*>( PHYSICSMGR->getWorld( server ? "server" : "client" ) );   

   if ( !world )
   {
      // TODO: Error... in error buffer?
      return false;
   }   

   NxMaterialDesc	material;
   material.restitution = restitution;
   material.staticFriction	= staticFriction;
   material.dynamicFriction	= dynamicFriction;

   mNxMat = world->createMaterial( material );
   mNxMatId = mNxMat->getMaterialIndex();

   if ( mNxMatId == -1 )
   {
      errorBuffer = "PxMaterial::preload() - unable to create material!";
      return false;
   }

   return Parent::preload( server, errorBuffer );
}

void PxMaterial::packData( BitStream* stream )
{
   Parent::packData( stream );
   
   stream->write( restitution );
   stream->write( staticFriction );
   stream->write( dynamicFriction );
}

void PxMaterial::unpackData( BitStream* stream )
{
   Parent::unpackData( stream );

   stream->read( &restitution );
   stream->read( &staticFriction );
   stream->read( &dynamicFriction );
}
