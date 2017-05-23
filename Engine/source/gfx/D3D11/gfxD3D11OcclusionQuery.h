//-----------------------------------------------------------------------------
// Copyright (c) 2015 GarageGames, LLC
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

#ifndef _GFX_D3D11_OCCLUSIONQUERY_H_
#define _GFX_D3D11_OCCLUSIONQUERY_H_

#include "gfx/D3D11/gfxD3D11Device.h"
#include "gfx/gfxOcclusionQuery.h"

#ifdef TORQUE_GATHER_METRICS
   #include "platform/platformTimer.h"
#endif

class GFXD3D11OcclusionQuery : public GFXOcclusionQuery
{
private:
   mutable ID3D11Query *mQuery;

#ifdef TORQUE_GATHER_METRICS
   U32 mBeginFrame;
   U32 mTimeSinceEnd;
   PlatformTimer *mTimer;
#endif

public:
   GFXD3D11OcclusionQuery(GFXDevice *device);
   virtual ~GFXD3D11OcclusionQuery();

   virtual bool begin();
   virtual void end();   
   virtual OcclusionQueryStatus getStatus(bool block, U32 *data = NULL);

   // GFXResource
   virtual void zombify();   
   virtual void resurrect();
   virtual const String describeSelf() const;
};

#endif