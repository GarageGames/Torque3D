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

beginModule( 'nxCharacter' );
	
   global $PHYSX_SDK_PATH;

   // Defines
   addProjectDefines( 'WIN32' );
   addProjectDefines( 'NXCHARACTER_STATIC' );

   // Source
   addSrcDir( $PHYSX_SDK_PATH . '/SDKs/NxCharacter/src' );
   addSrcDir( $PHYSX_SDK_PATH . '/SDKs/NxCharacter/include' );

   // Additional includes
   addIncludePath( $PHYSX_SDK_PATH . '/SDKs/NxCharacter/include' );
   addIncludePath( $PHYSX_SDK_PATH . '/SDKs/Physics/include' );
   addIncludePath( $PHYSX_SDK_PATH . '/SDKs/Foundation/include' );
   addIncludePath( $PHYSX_SDK_PATH . '/SDKs/PhysXLoader/include' );
   addIncludePath( $PHYSX_SDK_PATH . '/SDKs/Cooking/include' );
   
endModule();

?>