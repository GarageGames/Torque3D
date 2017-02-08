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

#ifndef _CONTAINERQUERY_H_
#define _CONTAINERQUERY_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _STRINGTABLE_H_
#include "core/stringTable.h"
#endif
#ifndef _MBOX_H_
#include "math/mBox.h"
#endif

class SceneObject;
class WaterObject;

struct ContainerQueryInfo
{
   ContainerQueryInfo()
      : box(-1,-1,-1,1,1,1),
        mass(1.0f),
        waterCoverage(0.0f),
        waterHeight(0.0f),                
        waterDensity(0.0f),
        waterViscosity(0.0f),
        gravityScale(1.0f),
        appliedForce(0,0,0),
        waterObject(NULL)
   {        
   }

   //SceneObject *sceneObject;
   Box3F box;
   F32 mass;
   F32 waterCoverage;
   F32 waterHeight;
   F32 waterDensity;
   F32 waterViscosity;
   String liquidType;
   F32 gravityScale;
   Point3F appliedForce;
   WaterObject *waterObject;
};

extern void findRouter( SceneObject *obj, void *key );
extern void waterFind( SceneObject *obj, void *key );
extern void physicalZoneFind( SceneObject *obj, void *key );

#endif // _CONTAINERQUERY_H_