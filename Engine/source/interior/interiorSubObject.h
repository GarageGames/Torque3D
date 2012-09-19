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

#ifndef _INTERIORSUBOBJECT_H_
#define _INTERIORSUBOBJECT_H_

#ifndef _SCENERENDERSTATE_H_
#include "scene/sceneRenderState.h"
#endif
#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif

class InteriorInstance;

//class SubObjectRenderImage : public SceneRenderImage
//{
//  public:
//   U32 mDetailLevel;
//};

class InteriorSubObject : public SceneObject
{
   typedef SceneObject Parent;

  protected:
   InteriorInstance* mInteriorInstance;   // Should NOT be set by derived except in clone

  protected:
   enum SubObjectKeys {
      TranslucentSubObjectKey = 0,
      MirrorSubObjectKey      = 1
   };

   virtual U32  getSubObjectKey() const  = 0;
   virtual bool _readISO(Stream&);
   virtual bool _writeISO(Stream&) const;

   InteriorInstance* getInstance();
   const MatrixF&    getSOTransform() const;
   const Point3F&    getSOScale() const;

  public:
   InteriorSubObject();
   virtual ~InteriorSubObject();

   // Render control.  A sub-object should return false from renderDetailDependant if
   //  it exists only at the level-0 detail level, ie, doors, elevators, etc., true
   //  if should only render at the interiors detail, ie, translucencies.
   //virtual SubObjectRenderImage* getRenderImage(SceneState*, const Point3F& osPoint) = 0;
   virtual bool                  renderDetailDependant() const = 0;
   virtual U32                   getZone() const               = 0;

   virtual void                  noteTransformChange();
   virtual InteriorSubObject*    clone(InteriorInstance*) const = 0;

   static InteriorSubObject* readISO(Stream&);
   bool                      writeISO(Stream&) const;
};

#endif  // _H_INTERIORSUBOBJECT_
