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

#ifndef _SPOTLIGHT_H_
#define _SPOTLIGHT_H_

#ifndef _LIGHTBASE_H_
#include "T3D/lightBase.h"
#endif


class SpotLight : public LightBase
{
   typedef LightBase Parent;

protected:

   F32 mRange;

   F32 mInnerConeAngle;

   F32 mOuterConeAngle;

   // LightBase
   void _conformLights();
   void _renderViz( SceneRenderState *state );

public:

   SpotLight();
   virtual ~SpotLight();

   // ConsoleObject
   DECLARE_CONOBJECT( SpotLight );
   static void initPersistFields();

   // SceneObject
   virtual void setScale( const VectorF &scale );

   // NetObject
   U32 packUpdate( NetConnection *conn, U32 mask, BitStream *stream );
   void unpackUpdate( NetConnection *conn, BitStream *stream );  
};

#endif // _SPOTLIGHT_H_
