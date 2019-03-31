
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

#include "afx/arcaneFX.h"
#include "afx/util/afxCurveEval.h"

Point2F afxHermiteEval::evaluateCurve( Point2F &v0, Point2F &v1,
                                         Point2F &t0, Point2F &t1, F32 t )
{
  F32 t_3 = t*t*t;
  F32 t_2 = t*t;
  F32 h1 = ( 2.0f * t_3 ) - ( 3.0f * t_2 ) + 1;
  F32 h2 = (-2.0f * t_3 ) + ( 3.0f * t_2 );
  F32 h3 = t_3 - ( 2.0f * t_2 ) + t;
  F32 h4 = t_3 - t_2;

  Point2F v( 
    (h1*v0.x)+(h2*v1.x)+(h3*t0.x)+(h4*t1.x),
    (h1*v0.y)+(h2*v1.y)+(h3*t0.y)+(h4*t1.y) );

  return v;
}

Point2F afxHermiteEval::evaluateCurve( Point2F &v0, Point2F &v1, F32 t )
{
  Point2F tangent( 1, 0 );
  return( evaluateCurve( v0, v1, tangent, tangent, t ) );
}

Point2F afxHermiteEval::evaluateCurveTangent( Point2F &v0, Point2F &v1,
                                                Point2F &t0, Point2F &t1, F32 t )
{
  F32 t_2 = t*t;
  F32 h1_der = ( 6.0f * t_2 ) - ( 6.0f * t );
  F32 h2_der = (-6.0f * t_2 ) + ( 6.0f * t );
  F32 h3_der = ( 3.0f * t_2 ) - ( 4.0f * t ) + 1;
  F32 h4_der = ( 3.0f * t_2 ) - ( 2.0f * t );

  Point2F tangent( 
    (h1_der*v0.x)+(h2_der*v1.x)+(h3_der*t0.x)+(h4_der*t1.x),
    (h1_der*v0.y)+(h2_der*v1.y)+(h3_der*t0.y)+(h4_der*t1.y) );

  return tangent;
}

Point2F afxHermiteEval::evaluateCurveTangent( Point2F &v0, Point2F &v1, F32 t )
{
  Point2F tangent( 1, 0 );
  return( evaluateCurveTangent( v0, v1, tangent, tangent, t ) );
}

Point3F afxHermiteEval::evaluateCurve( Point3F &v0, Point3F &v1,
                                         Point3F &t0, Point3F &t1, F32 t )
{
  F32 t_3 = t*t*t;
  F32 t_2 = t*t;
  F32 h1 = ( 2.0f * t_3 ) - ( 3.0f * t_2 ) + 1;
  F32 h2 = (-2.0f * t_3 ) + ( 3.0f * t_2 );
  F32 h3 = t_3 - ( 2.0f * t_2 ) + t;
  F32 h4 = t_3 - t_2;

  Point3F v( 
    (h1*v0.x)+(h2*v1.x)+(h3*t0.x)+(h4*t1.x),
    (h1*v0.y)+(h2*v1.y)+(h3*t0.y)+(h4*t1.y),
    (h1*v0.z)+(h2*v1.z)+(h3*t0.z)+(h4*t1.z) );

  return v;
}

Point3F afxHermiteEval::evaluateCurve( Point3F &v0, Point3F &v1, F32 t )
{
  Point3F tangent( 1, 0, 0 );
  return( evaluateCurve( v0, v1, tangent, tangent, t ) );
}

Point3F afxHermiteEval::evaluateCurveTangent( Point3F &v0, Point3F &v1,
                                                Point3F &t0, Point3F &t1, F32 t )
{
  F32 t_2 = t*t;
  F32 h1_der = ( 6.0f * t_2 ) - ( 6.0f * t );
  F32 h2_der = (-6.0f * t_2 ) + ( 6.0f * t );
  F32 h3_der = ( 3.0f * t_2 ) - ( 4.0f * t ) + 1;
  F32 h4_der = ( 3.0f * t_2 ) - ( 2.0f * t );

  Point3F tangent( 
    (h1_der*v0.x)+(h2_der*v1.x)+(h3_der*t0.x)+(h4_der*t1.x),
    (h1_der*v0.y)+(h2_der*v1.y)+(h3_der*t0.y)+(h4_der*t1.y),
    (h1_der*v0.z)+(h2_der*v1.z)+(h3_der*t0.z)+(h4_der*t1.z) );

  return tangent;
}

Point3F afxHermiteEval::evaluateCurveTangent( Point3F &v0, Point3F &v1, F32 t )
{
  Point3F tangent( 1, 0, 0 );
  return( evaluateCurveTangent( v0, v1, tangent, tangent, t ) );
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

