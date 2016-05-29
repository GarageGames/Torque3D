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

#ifndef RENDER_COMPONENT_INTERFACE_H
#define RENDER_COMPONENT_INTERFACE_H

#ifndef _TSSHAPE_H_
#include "ts/tsShape.h"
#endif
#ifndef _TSSHAPEINSTANCE_H_
#include "ts/tsShapeInstance.h"
#endif
#ifndef CORE_INTERFACES_H
#include "T3D/components/coreInterfaces.h"
#endif

class RenderComponentInterface : public Interface < RenderComponentInterface >
{
public:
   virtual void prepRenderImage(SceneRenderState *state) = 0;

   virtual TSShape* getShape() = 0;

   Signal< void(RenderComponentInterface*) > onShapeChanged;

   virtual TSShapeInstance* getShapeInstance() = 0;

   virtual MatrixF getNodeTransform(S32 nodeIdx) = 0;

   virtual Vector<MatrixF> getNodeTransforms() = 0;

   virtual void setNodeTransforms(Vector<MatrixF> transforms) = 0;

   Signal< void(RenderComponentInterface*) > onShapeInstanceChanged;
};

class CastRayRenderedInterface// : public Interface<CastRayRenderedInterface>
{
public:
   virtual bool castRayRendered(const Point3F &start, const Point3F &end, RayInfo* info)=0;
};

#endif