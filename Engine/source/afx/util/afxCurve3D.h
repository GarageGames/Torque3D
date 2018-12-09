
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

#ifndef _AFX_CURVE_3D_H_
#define _AFX_CURVE_3D_H_

#include "core/util/tVector.h"
#include "math/mPoint3.h"

class afxCurveEval;

class afxCurve3D
{
	class CurvePoint
	{
		public:
			F32			parameter;
			Point3F point;
		
		   // new:
         Point3F tangent;
	};

	private:
		afxCurveEval* evaluator;
		Point3F start_value;
		Point3F final_value;
		Point3F start_tangent;
		Point3F final_tangent;
		bool	  usable;

		//std::vector<CurvePoint> points;
		Vector<CurvePoint> points;

		Point3F default_vector;

		//static bool compare_CurvePoint( const CurvePoint &a, const CurvePoint &b ); 
		static S32 QSORT_CALLBACK compare_CurvePoint( const void* a, const void* b );

      // new
      Point3F last_tangent;
      bool flip;

	public:
		afxCurve3D();
		~afxCurve3D();

		void    addPoint( F32 param, Point3F &v );
		void    setPoint( int index, Point3F &v );
		void    sort( );
		int     numPoints();
		F32     getParameter( int index );
		Point3F getPoint( int index );
		Point3F evaluate( F32 param );
		Point3F evaluateTangent( F32 param );

		void print();

      void computeTangents();

      //MatrixF createOrientFromDir( Point3F &direction );

	private:
		Point3F computeTangentP0( Point3F &p0, Point3F &p1, int start_index );
		Point3F computeTangentP1( Point3F &p0, Point3F &p1, int end_index );
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_CURVE_3D_H_