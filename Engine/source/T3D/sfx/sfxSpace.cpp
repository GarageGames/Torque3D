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
#include "T3D/sfx/sfxSpace.h"

#include "scene/mixin/sceneAmbientSoundObject.impl.h"
#include "scene/mixin/scenePolyhedralObject.impl.h"
#include "console/engineAPI.h"


IMPLEMENT_CO_NETOBJECT_V1( SFXSpace );

ConsoleDocClass( SFXSpace,
   "@brief A volume in space that defines an ambient sound zone.\n\n"

   "@ingroup SFX"
);

//-----------------------------------------------------------------------------

SFXSpace::SFXSpace()
{
   mObjScale.set( 10, 10, 10 );
   mObjBox.set(
      Point3F( -0.5f, -0.5f, -0.5f ),
      Point3F( 0.5f, 0.5f, 0.5f )
   );
}

//-----------------------------------------------------------------------------

void SFXSpace::consoleInit()
{
   // Disable rendering of SFXSpaces by default.
   getStaticClassRep()->mIsRenderEnabled = false;
}
