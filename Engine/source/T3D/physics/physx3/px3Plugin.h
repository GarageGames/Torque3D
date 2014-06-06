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

#ifndef _PX3PLUGIN_H_
#define _PX3PLUGIN_H_

#ifndef _T3D_PHYSICS_PHYSICSPLUGIN_H_
#include "T3D/physics/physicsPlugin.h"
#endif

class Px3ClothShape;

class Px3Plugin : public PhysicsPlugin
{
public:

   Px3Plugin();
   ~Px3Plugin();

   /// Create function for factory.
   static PhysicsPlugin* create();

   // PhysicsPlugin
   virtual void destroyPlugin();
   virtual void reset();
   virtual PhysicsCollision* createCollision();
   virtual PhysicsBody* createBody();
   virtual PhysicsPlayer* createPlayer();
   virtual bool isSimulationEnabled() const;
   virtual void enableSimulation( const String &worldName, bool enable );
   virtual void setTimeScale( const F32 timeScale );
   virtual const F32 getTimeScale() const;
   virtual bool createWorld( const String &worldName );
   virtual void destroyWorld( const String &worldName );
   virtual PhysicsWorld* getWorld( const String &worldName ) const;
   virtual PhysicsWorld* getWorld() const;
   virtual U32 getWorldCount() const;
};

#endif  // _PX3PLUGIN_H_
