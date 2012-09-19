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

#ifndef _MSPLINEPATCH_H_
#define _MSPLINEPATCH_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif
#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif
#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

//------------------------------------------------------------------------------
// Spline control points
//------------------------------------------------------------------------------

/// Class for spline control points.
/// @see SplinePatch
class SplCtrlPts
{
private:
   /// Vector of points in the spline
   Vector <Point3F > mPoints;

public:

   SplCtrlPts();
   virtual ~SplCtrlPts();

   /// Gets the number of points in the spline
   U32               getNumPoints(){ return mPoints.size(); }
   /// Gets the point at the given index
   /// @param pointNum index of the point in question
   const Point3F *   getPoint( U32 pointNum );
   /// Sets a point at the given index to the point given
   /// @param point New value for the given point
   /// @param pointNum index of the given point
   void              setPoint( Point3F &point, U32 pointNum );
   /// Adds a point to the end of the spline
   /// @param point New point to be added
   void              addPoint( Point3F &point );
   /// Clears existing points and enters new points
   /// @param pts List of points to be added
   /// @param num Number of points to be added
   void              submitPoints( Point3F *pts, U32 num );
};

//------------------------------------------------------------------------------
// Base class for spline patches
//------------------------------------------------------------------------------

/// Base class for spline patches.  The only child of this class is QuadPatch.
///
/// Spline utility class for drawing nice pretty splines.  In order to draw a spline,
/// you need to create a SplCtrlPts data structure, which contains all control
/// points on the spline.  See SplCtrlPts for more information on how to submit
/// points to the spline utility.  Next, submit the SplCtrlPts structure to the
/// spline utility.
/// @code
/// SplinePatch patch;
/// patch.submitControlPoints(ctrlPts);
/// @endcode
/// Next, use the SplineUtil namespace to draw your spline.
/// @code
/// SplineUtil::drawSplineBeam(camPos, numSegments, width, patch[, uvOffset, numTexRep]);
/// @endcode
///
/// You can also create a SplineBeamInfo structure (SplineUtil::SplineBeamInfo)
/// and just pass the SplineBeamInfo structure to the SplineUtil::drawSplineBeam function.
/// @see SplCtrlPts
/// @see SplineUtil
class SplinePatch
{
private:
   U32         mNumReqControlPoints;
   SplCtrlPts  mControlPoints;

protected:
   void     setNumReqControlPoints( U32 numPts ){ mNumReqControlPoints = numPts; }

public:

   SplinePatch();

   U32                  getNumReqControlPoints(){ return mNumReqControlPoints; }
   const SplCtrlPts *   getControlPoints(){ return &mControlPoints; }
   const Point3F *      getControlPoint( U32 index ){ return mControlPoints.getPoint( index ); }

   // virtuals
   virtual void         setControlPoint( Point3F &point, int index );
   /// If you have a preconstructed "SplCtrlPts" class, submit it with this function.
   /// @see SplCtrlPts
   virtual void         submitControlPoints( SplCtrlPts &points ){ mControlPoints = points; }
   /// Recalc function.  Do not call this ever - only SplineUtil needs this.
   /// @see SplineUtil
   virtual void         calc( F32 t, Point3F &result) = 0;
   /// Recalc function.  Do not call this ever - only SplineUtil needs this.
   /// @see SplineUtil
   virtual void         calc( Point3F *points, F32 t, Point3F &result ) = 0;

};


#endif
