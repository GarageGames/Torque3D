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

// "Universal" script methods for projectile damage handling.  You can easily
// override these support functions with an equivalent namespace method if your
// weapon needs a unique solution for applying damage.

function ProjectileData::onCollision(%data, %proj, %col, %fade, %pos, %normal)
{
   //echo("ProjectileData::onCollision("@%data.getName()@", "@%proj@", "@%col.getClassName()@", "@%fade@", "@%pos@", "@%normal@")");

   // Apply damage to the object all shape base objects
   if (%data.directDamage > 0)
   {
      if (%col.getType() & ($TypeMasks::ShapeBaseObjectType))
         %col.damage(%proj, %pos, %data.directDamage, %data.damageType);
   }
}

function ProjectileData::onExplode(%data, %proj, %position, %mod)
{
   //echo("ProjectileData::onExplode("@%data.getName()@", "@%proj@", "@%position@", "@%mod@")");

   // Damage objects within the projectiles damage radius
   if (%data.damageRadius > 0)
      radiusDamage(%proj, %position, %data.damageRadius, %data.radiusDamage, %data.damageType, %data.areaImpulse);
}
