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

datablock PhysicsShapeData( PhysicsCube )
{	
   category = "Physics";
   shapeName = "art/shapes/cube/cube.dae";
   emap = true;
   
   //physics properties
   mass = "0.5";   
   friction = "0.4";
   staticFriction = "0.5";
   restitution = "0.3";
   linearDamping = "0.1";
   angularDamping = "0.2";
   linearSleepThreshold = "1.0";
   angularSleepThreshold = "1.0";
   buoyancyDensity = "0.9";      
   waterDampingScale = "10";   
   
   //damage - dynamic fields
   radiusDamage = 0;
   damageRadius = 0;
   areaImpulse = 0;   
   invulnerable = true;   
};

datablock PhysicsShapeData( PhysicsBoulder )
{	
   category = "Physics";
   shapeName = "art/shapes/rocks/boulder.dts";
   emap = true;
   
   //physics properties
   mass = "20";   
   friction = "0.2";
   staticFriction = "0.3";
   restitution = "0.8";
   linearDamping = "0.1";
   angularDamping = "0.2";
   linearSleepThreshold = "1.0";
   angularSleepThreshold = "1.0";
   buoyancyDensity = "0.9";      
   waterDampingScale = "10";   
   
   //damage - dynamic fields
   radiusDamage = 0;
   damageRadius = 0;
   areaImpulse = 0;   
   invulnerable = false;   
};
