
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
#include "afx/util/afxCurve3D.h"

afxCurve3D::afxCurve3D() : usable( false ), default_vector( 0, 0, 0 )
{
	evaluator = new afxHermiteEval();
}

afxCurve3D::~afxCurve3D()
{
	delete evaluator;
}

void afxCurve3D::addPoint( F32 param, Point3F &v )
{
	if( param < 0.0f || param > 1.0f )
		return;

	CurvePoint p;
	p.parameter = param;
	p.point.set( v );
	
	points.push_back( p );	

	usable = false;	
}	

void afxCurve3D::setPoint( int index, Point3F &v )
{
	if( ( index < 0 ) || ( index >= points.size() ) )
		return;

	CurvePoint &p = points[index];
	p.point = v;

	if( index == 0 )
		start_value = v;
	else if( index == points.size()-1 )
		final_value = v;
}

//bool afxCurve3D::compare_CurvePoint( const afxCurve3D::CurvePoint &a, const afxCurve3D::CurvePoint &b ) 
//{
//	return a.parameter < b.parameter;
//}

S32 QSORT_CALLBACK afxCurve3D::compare_CurvePoint( const void* a, const void* b )
{
	//CurvePoint *cp_a = *((CurvePoint **)a);
  //CurvePoint *cp_b = *((CurvePoint **)b);
  
	const CurvePoint *cp_a = (CurvePoint *)a;
	const CurvePoint *cp_b = (CurvePoint *)b;

  //Con::printf( "*** %f %f", cp_a->parameter, cp_b->parameter  );

	//return cp_a->parameter < cp_b->parameter;
	//return 1;

	if (cp_a->parameter > cp_b->parameter)
      return 1;
   else if (cp_a->parameter < cp_b->parameter)
      return -1;
   else
      return 0;
}

void afxCurve3D::sort( )
{
	if( points.size() == 0 )
		return;

	if( points.size() == 1 )
	{
		start_value   = points[0].point;
		final_value   = start_value;
		usable = true;
		return;
	}

	//Con::printf( "*** pre-sort" );
	//std::sort( points.begin(), points.end(), afxCurve3D::compare_CurvePoint );
	dQsort( points.address(), points.size(), sizeof(CurvePoint), afxCurve3D::compare_CurvePoint );
	//Con::printf( "*** post-sort" );

	start_value = points[0].point;
	final_value = points[points.size()-1].point;

	usable = true;

	start_tangent = evaluateTangent( 0.0f );
	final_tangent = evaluateTangent( 1.0f );
}	

int afxCurve3D::numPoints()
{
	return points.size();
}

F32 afxCurve3D::getParameter( int index )
{
	if( ( index < 0 ) || ( index >= points.size() ) )
		return 0.0f;

	return points[index].parameter;
}

Point3F afxCurve3D::getPoint( int index )
{
	if( ( index < 0 ) || ( index >= points.size() ) )
		return default_vector;

	return points[index].point;
}

Point3F afxCurve3D::evaluate( F32 param )
{
	if( !usable )
		return default_vector;

	if( param <= 0.0f )
		return start_value;
	
	if( param >= 1.0f )
		return final_value;

	if( points.size() == 1 )
		return start_value;

	int start_index = 0;
	for( ; start_index < points.size()-1; start_index++ )
	{
		if( param < points[start_index+1].parameter )
			break;
	}
	int end_index = start_index+1;

	CurvePoint p0 = points[start_index];
	CurvePoint p1 = points[end_index];

	// Compute tangents
	//Point3F tan0 = computeTangentP0( p0.point, p1.point, start_index );
	//Point3F tan1 = computeTangentP1( p0.point, p1.point, end_index );
	

	F32 local_param = ( param - p0.parameter ) / ( p1.parameter - p0.parameter );

	//Point3F vnew = evaluator->evaluateCurve( p0.point,
	//																				p1.point,
	//																				tan0,
	//																				tan1,
	//																				local_param );

   Point3F vnew = evaluator->evaluateCurve( p0.point,   p1.point,
														  p0.tangent, p1.tangent,
														  local_param );
	return vnew;
}

Point3F afxCurve3D::evaluateTangent( F32 param )
{
	if( !usable )
		return default_vector;

	if( param < 0.0f )
		return start_tangent;
	
	if( param > 1.0f )
		return final_tangent;

	if( points.size() == 1 )
		return start_tangent;

	int start_index = 0;
	for( ; start_index < points.size()-1; start_index++ )
	{
		if( param < points[start_index+1].parameter )
			break;
	}
	int end_index = start_index+1;

	if( param == 1.0f )
	{
		end_index = points.size()-1;
		start_index = end_index - 1;
	}

	CurvePoint p0 = points[start_index];
	CurvePoint p1 = points[end_index];
	
	// Compute tangents
	//Point3F tan0 = computeTangentP0( p0.point, p1.point, start_index );
	//Point3F tan1 = computeTangentP1( p0.point, p1.point, end_index );

	F32 local_param = ( param - p0.parameter ) / ( p1.parameter - p0.parameter );

	//Point3F vnew = evaluator->evaluateCurveTangent( p0.point,
	//																								p1.point,
	//																								tan0,
	//																								tan1,
	//																								local_param );
   Point3F vnew = evaluator->evaluateCurveTangent( p0.point,   p1.point,
														         p0.tangent, p1.tangent,
														         local_param );

	return vnew;
}

Point3F afxCurve3D::computeTangentP0( Point3F &p0, Point3F &p1, int start_index )
{
	Point3F tan0;
	
	Point3F p_prev;
	Point3F p_next;

		// tangent for p0
	if( start_index == 0 )
	{
		p_prev = p0;  	// Setting previous point to p0, creating a hidden point in
										//  the same spot
		p_next = p1;
	}
	else
	{
		CurvePoint &p = points[start_index-1];
		p_prev = p.point;
		p_next = p1;
	}
	tan0 = p_next-p_prev; //p_next.subtract( p_prev );
	tan0 *= .5f; //= tan0.scale( .5f );

	return tan0;
}

Point3F afxCurve3D::computeTangentP1( Point3F &p0, Point3F &p1, int end_index )
{
	Point3F tan1;
	
	Point3F p_prev;
	Point3F p_next;

		// tangent for p1
	if( end_index == points.size()-1 )
	{
		p_prev = p0;  
		p_next = p1;	// Setting next point to p1, creating a hidden point in
									//  the same spot
	}
	else
	{
		p_prev = p0;
		CurvePoint &p = points[end_index+1];
		p_next = p.point;
	}
	tan1 = p_next-p_prev; //p_next.subtract( p_prev );
	tan1 *= .5f; //= tan1.scale( .5f );

	//Con::printf("UPDATE");
	return tan1;
}

void afxCurve3D::computeTangents()
{
	CurvePoint *p_prev;
	CurvePoint *p_next;

   for( int i = 0; i < points.size(); i++ )
	{
		CurvePoint *p = &points[i];
		
      if( i == 0 )
      {
         p_prev = p;  	// Setting previous point to p0, creating a hidden point in
								//  the same spot
		   p_next = &points[i+1];
      }
      else if( i == points.size()-1 )
	   {
		   p_prev = &points[i-1];  
		   p_next = p;	   // Setting next point to p1, creating a hidden point in
	   						//  the same spot
	   }
      else
	   {		   
		   p_prev = &points[i-1];
		   p_next = &points[i+1];
	   }

      p->tangent = p_next->point - p_prev->point;
      //(p->tangent).normalize();
      p->tangent *= .5f;

      //Con::printf( "%d: %f %f %f", i, p->tangent.x, p->tangent.y, p->tangent.z );
   }
}

void afxCurve3D::print()
{
	Con::printf( "afxCurve3D -------------------------" );
	for( int i = 0; i < points.size(); i++ )
	{
		CurvePoint &p = points[i];
		Con::printf( "%f: %f %f %f", p.parameter, p.point.x, p.point.y, p.point.z );
	}
	Con::printf( "---------------------------------" );
}