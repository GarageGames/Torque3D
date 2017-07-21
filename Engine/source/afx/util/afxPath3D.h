
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
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
//
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#ifndef _AFX_PATH3D_UTIL_H_
#define _AFX_PATH3D_UTIL_H_

#include "afx/util/afxCurve3D.h"
#include "afx/util/afxAnimCurve.h"

#include "math/mMatrix.h"

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

class afxPath3D : public EngineObject
{
private:
    // Path-related data
  afxCurve3D   curve;
  afxAnimCurve curve_parameters;
  int       num_points;
  
  // Time data
  F32       start_time;
  F32       end_time;
  
public:
  /*C*/     afxPath3D( );
  /*D*/     ~afxPath3D();
  
  void      sortAll();
  
  void      setStartTime(F32 time);
  
  F32       getEndTime();
  int       getNumPoints();
  Point3F   getPointPosition(int index);
  F32       getPointTime(int index);
  F32       getPointParameter(int index);
  Point2F   getParameterSegment(F32 time);
  
  void      setPointPosition(int index, Point3F &p);
  
  Point3F   evaluateAtTime(F32 time);
  Point3F   evaluateAtTime(F32 t0, F32 t1);  // returns delta
  Point3F   evaluateTangentAtTime(F32 time);
  Point3F   evaluateTangentAtPoint(int index);
  
  void      buildPath(int num_points, Point3F curve_points[], F32 start_time, F32 end_time);
  void      buildPath(int num_points, Point3F curve_points[], F32 speed);
  void      buildPath(int num_points, Point3F curve_points[], F32 point_times[], F32 time_offset, F32 time_factor);
  void      buildPath(int num_points, Point3F curve_points[], Point2F curve_params[]);
  
  void      reBuildPath();
  
  void      print();
   
  enum LoopType
  {      
    LOOP_CONSTANT,
    LOOP_CYCLE,
    LOOP_OSCILLATE
  };
  
  U32       loop_type;
  void      setLoopType(U32);
  
private:
  void      initPathParameters(Point3F curve_points[], F32 speed);
  void      initPathParametersNEW(Point3F curve_points[], F32 start_time, F32 end_time);
    
  F32       calcCurveTime(F32 time);
};

typedef afxPath3D::LoopType afxPath3DLoopType;
DefineEnumType( afxPath3DLoopType );

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_PATH3D_UTIL_H_
