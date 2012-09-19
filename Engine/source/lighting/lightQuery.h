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

#ifndef _LIGHTQUERY_H_
#define _LIGHTQUERY_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _MSPHERE_H_
#include "math/mSphere.h"
#endif

#ifndef _MBOX_H_
#include "math/mBox.h"
#endif


class LightManager;
class LightInfo;


/// Used to gather an score lights for rendering.
class LightQuery
{
public:

   LightQuery( U32 maxLights = 4 );
   ~LightQuery();

   /// Set the query volume from a camera position and direction.
   void init(  const Point3F &cameraPos,
               const Point3F &cameraDir, 
               F32 viewDist );

   /// Set the query volume from a sphere.
   void init( const SphereF &bounds );

   /// Set the query volume from a box.
   void init( const Box3F &bounds );

   /// This returns the best lights based on the query volume.
   void getLights( LightInfo** outLights, U32 maxLights );

protected:

   void _scoreLights();

   static S32 _lightScoreCmp( LightInfo* const *a, LightInfo* const *b );

   /// The maximum lights to return from the query.
   const U32 mMaxLights;

   /// The sorted list of best lights.
	Vector<LightInfo*> mLights;

   /// The sphere used to query for lights.
   SphereF mVolume;
};


#endif // _LIGHTQUERY_H_
