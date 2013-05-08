<?php
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

// 3D
addEngineSrcDir('collision');
addEngineSrcDir('materials');
addEngineSrcDir('lighting');
addEngineSrcDir('lighting/common');
addEngineSrcDir('renderInstance');
addEngineSrcDir('scene');
addEngineSrcDir('scene/culling');
addEngineSrcDir('scene/zones');
addEngineSrcDir('scene/mixin');
addEngineSrcDir('shaderGen');
addEngineSrcDir('terrain');
addEngineSrcDir('environment');

addEngineSrcDir('forest');
addEngineSrcDir('forest/ts');
if(getToolBuild())
   addEngineSrcDir('forest/editor');

addEngineSrcDir('ts');
addEngineSrcDir('ts/arch');
addEngineSrcDir('physics');
addEngineSrcDir('gui/3d');
addEngineSrcDir('postFx' );

// 3D game
addEngineSrcDir('T3D');
addEngineSrcDir('T3D/examples');
addEngineSrcDir('T3D/fps');
addEngineSrcDir('T3D/fx');
addEngineSrcDir('T3D/vehicles');
addEngineSrcDir('T3D/physics');
addEngineSrcDir('T3D/decal');
addEngineSrcDir('T3D/sfx');
addEngineSrcDir('T3D/gameBase');
addEngineSrcDir('T3D/turret');

global $TORQUE_HIFI_NET;
global $TORQUE_EXTENDED_MOVE;
if ( $TORQUE_HIFI_NET == true )
{
   addProjectDefines( 'TORQUE_HIFI_NET' );
   addEngineSrcDir('T3D/gameBase/hifi');
}
elseif ( $TORQUE_EXTENDED_MOVE == true )
{
   addProjectDefines( 'TORQUE_EXTENDED_MOVE' );
   addEngineSrcDir('T3D/gameBase/extended');
}
else
	addEngineSrcDir('T3D/gameBase/std');

// Plstform specific stuff.
switch( Generator::$platform )
{
   case "360":
      addEngineSrcDir('ts/arch/360');
      // Fall through
      
   case "win32":
      addEngineSrcDir( 'terrain/hlsl' );
      addEngineSrcDir( 'forest/hlsl' );
      break;

   case "ps3":
      addEngineSrcDir('ts/arch/ps3');
      // Fall through
      
   case "mac":
   case "linux":
   case "linux_dedicated":
      addEngineSrcDir( 'terrain/glsl' );
      addEngineSrcDir( 'forest/glsl' );
      break;
}

?>
