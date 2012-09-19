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

#ifndef _WINDEMITTER_H_
#define _WINDEMITTER_H_

#ifndef _MPOINT3_H_
   #include "math/mPoint3.h"
#endif
#ifndef _MSPHERE_H_
   #include "math/mSphere.h"
#endif
#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif

class WindEmitter;

/// A vector of WindEmitter pointers.
typedef Vector<WindEmitter*> WindEmitterList;


class WindEmitter
{
public:
   WindEmitter();
   ~WindEmitter();

   operator const SphereF&() const { return mSphere; }

   void update( const Point3F& pos, const VectorF& velocity );

   void setPosition( const Point3F& pos );

   void setRadius( F32 radius );

   void setStrength( F32 strength );

   void setTurbulency( F32 frequency, F32 strength );

   const Point3F& getCenter() const { return mSphere.center; }

   F32 getRadius() const { return mSphere.radius; }

   F32 getStrength() const { return mStrength; }

   F32 getTurbulenceFrequency() const { return mTurbulenceFrequency; }

   F32 getTurbulenceStrength() const { return mTurbulenceStrength; }

   const VectorF& getVelocity() const { return mVelocity; }


   static bool findBest( const Point3F& cameraPos, 
                         const VectorF& cameraDir,
                         F32 viewDistance,
                         U32 maxResults,
                         WindEmitterList* results );

protected:
   SphereF mSphere;

   VectorF mVelocity;

   F32 mStrength;
   F32 mTurbulenceFrequency;
   F32 mTurbulenceStrength;
   F32 mScore;

   bool mEnabled;

   static WindEmitterList smAllEmitters;

   static S32 QSORT_CALLBACK _sortByScore( const void* a, const void* b );
};

#endif // _WINDEMITTER_H_
