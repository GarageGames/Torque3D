
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

#ifndef _AFX_CURVE_EVAL_BASE_H_
#define _AFX_CURVE_EVAL_BASE_H_

#include "math/mPoint2.h"
#include "math/mPoint3.h"

class afxCurveEval
{
public:
  virtual Point2F evaluateCurve(Point2F& v0, Point2F& v1, F32 t)=0;
  virtual Point2F evaluateCurve(Point2F& v0, Point2F& v1, Point2F& t0, Point2F& t1, F32 t)=0;
  virtual Point2F evaluateCurveTangent(Point2F& v0, Point2F& v1, F32 t)=0;
  virtual Point2F evaluateCurveTangent(Point2F& v0, Point2F& v1, Point2F& t0, Point2F& t1, F32 t)=0;
  
  virtual Point3F evaluateCurve(Point3F& v0, Point3F& v1, F32 t)=0;
  virtual Point3F evaluateCurve(Point3F& v0, Point3F& v1, Point3F& t0, Point3F& t1, F32 t)=0;
  virtual Point3F evaluateCurveTangent(Point3F& v0, Point3F& v1, F32 t)=0;
  virtual Point3F evaluateCurveTangent(Point3F& v0, Point3F& v1, Point3F& t0, Point3F& t1, F32 t)=0;
};

class afxHermiteEval : public afxCurveEval
{
public:
  Point2F evaluateCurve(Point2F& v0, Point2F& v1, F32 t);
  Point2F evaluateCurve(Point2F& v0, Point2F& v1, Point2F& t0, Point2F& t1, F32 t);
  Point2F evaluateCurveTangent(Point2F& v0, Point2F& v1, F32 t);
  Point2F evaluateCurveTangent(Point2F& v0, Point2F& v1, Point2F& t0, Point2F& t1, F32 t);
  
  Point3F evaluateCurve(Point3F& v0, Point3F& v1, F32 t);
  Point3F evaluateCurve(Point3F& v0, Point3F& v1, Point3F& t0, Point3F& t1, F32 t);
  Point3F evaluateCurveTangent(Point3F& v0, Point3F& v1, F32 t);
  Point3F evaluateCurveTangent(Point3F& v0, Point3F& v1, Point3F& t0, Point3F& t1, F32 t);
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_CURVE_EVAL_BASE_H_
