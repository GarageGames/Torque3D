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

#include "lighting/common/lightMapParams.h"
#include "core/stream/bitStream.h"
#include "core/module.h"

MODULE_BEGIN( LightMapParams )
MODULE_INIT_AFTER( ShadowMapParams )
MODULE_INIT
{
   LightMapParams::Type = "LightMapParams";
}
MODULE_END;

LightInfoExType LightMapParams::Type( "" );

LightMapParams::LightMapParams( LightInfo *light ) :
   representedInLightmap(false), 
   includeLightmappedGeometryInShadow(false), 
   shadowDarkenColor(0.0f, 0.0f, 0.0f, -1.0f)
{
   
}

LightMapParams::~LightMapParams()
{

}

void LightMapParams::set( const LightInfoEx *ex )
{
   // TODO: Do we even need this?
}

void LightMapParams::packUpdate( BitStream *stream ) const
{
   stream->writeFlag(representedInLightmap);
   stream->writeFlag(includeLightmappedGeometryInShadow);
   stream->write(shadowDarkenColor);
}

void LightMapParams::unpackUpdate( BitStream *stream )
{
   representedInLightmap = stream->readFlag();
   includeLightmappedGeometryInShadow = stream->readFlag();
   stream->read(&shadowDarkenColor);

   // Always make sure that the alpha value of the shadowDarkenColor is -1.0
   shadowDarkenColor.alpha = -1.0f;
}