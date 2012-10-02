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

#ifndef _LIGHTMAPPARAMS_H_
#define _LIGHTMAPPARAMS_H_

#ifndef _LIGHTINFO_H_
#include "lighting/lightInfo.h"
#endif

class LightMapParams : public LightInfoEx
{
public:
   LightMapParams( LightInfo *light );
   virtual ~LightMapParams();

   /// The LightInfoEx hook type.
   static LightInfoExType Type;

   // LightInfoEx
   virtual void set( const LightInfoEx *ex );
   virtual const LightInfoExType& getType() const { return Type; }
   virtual void packUpdate( BitStream *stream ) const;
   virtual void unpackUpdate( BitStream *stream );

public:
   // We're leaving these public for easy access 
   // for console protected fields.

   bool representedInLightmap;   ///< This light is represented in lightmaps (static light, default: false)
   ColorF shadowDarkenColor;     ///< The color that should be used to multiply-blend dynamic shadows onto lightmapped geometry (ignored if 'representedInLightmap' is false)
   bool includeLightmappedGeometryInShadow; ///< This light should render lightmapped geometry during its shadow-map update (ignored if 'representedInLightmap' is false)
};

#endif
