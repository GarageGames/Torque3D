
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

#ifndef _AFX_ANIM_CURVE_H_
#define _AFX_ANIM_CURVE_H_

#include "core/util/tVector.h"

#include "afx/util/afxCurveEval.h"

class afxAnimCurve
{
	class Key
	{
		public:
			F32 time;
			F32 value;
	};

	private:
		afxCurveEval* evaluator;

		F32  final_value;
		F32  start_value;
		F32  final_time;
		F32	 start_time;
		bool usable;

		//std::vector<Key> keys;
		Vector<Key> keys;

		//static bool compare_Key( const Key &a, const Key &b ); 
		static S32 QSORT_CALLBACK compare_Key( const void* a, const void* b );

	public:
		afxAnimCurve();
		~afxAnimCurve();

		void    addKey( Point2F &v );
		void    addKey( F32 time, F32 value );
		void    setKeyTime( int index, F32 t );
		void    setKeyValue( int index, F32 v );
		void    sort( );
		int     numKeys();
		F32     getKeyTime( int index );
		F32     getKeyValue( int index );
		Point2F getSegment( F32 time );
		F32     evaluate( F32 time );

		void print();
		void printKey( int index );

	private:
		Point2F computeTangentK0( Point2F &k0, Point2F &k1, int start_index );
		Point2F computeTangentK1( Point2F &k0, Point2F &k1, int end_index );
};

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#endif // _AFX_ANIM_CURVE_H_