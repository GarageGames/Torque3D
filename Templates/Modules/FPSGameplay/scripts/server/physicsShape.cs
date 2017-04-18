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

function PhysicsShapeData::damage(%this, %obj, %sourceObject, %position, %amount, %damageType)
{
   // Order of operations is extremely important here!
   // Verify that any changes will not cause this method to overflow the stack
   // recursively calling itself.
      
   // Note that invulerable, damageRadius, areaImpulse, radiusDamage, and damageType
   // are only dynamic fields... This is fine so long as you are only calling 
   // this method server-side, just keep in mind these fields are NOT networked.
   
   if (  %this.invulnerable ||
         %amount < 0 || 
         ( %this.minDamageAmount != 0 && %amount < %this.minDamageAmount ) )
      return;
      
   // We cannot destroy things twice.
   if ( %obj.isDestroyed() )
      return;
                        
   // This sets a maskbit on the server PhysicsShape which will cause the
   // client object to destroy ( spawn debris ) during the next ghost update.
   %obj.destroy();   
   
   // Single-player hack...
   // In a single-player situation the radial impulse NetEvent will
   // be applied client-side immediately when we call it, which means it will
   // happen before the next ghost update and the debris won't even exist yet!
   //
   // So we are explicitly calling destroy on the client-side object first,
   // before sending the event.
   //   
   if ( %obj.getClientObject() )
      %obj.getClientObject().destroy();
         
   if ( %this.damageRadius > 0 )
   {
      // Send impulse event to affect objects from the explosion of this object.
      // Happens server-side and client-side.
      if ( %this.areaImpulse > 0 )
         RadialImpulseEvent::send( %position, %this.damageRadius, %this.areaImpulse );
      
      // Apply damage to objects from the explosion of this object.   
      if ( %this.radiusDamage > 0 )
         radiusDamage( %obj, %position, %this.damageRadius, %this.radiusDamage, %this.damageType );
   }
}