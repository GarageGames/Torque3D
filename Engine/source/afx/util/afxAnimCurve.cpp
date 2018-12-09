
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
#include "afx/util/afxAnimCurve.h"

afxAnimCurve::afxAnimCurve() : usable( false ), final_value( 0.0f ), start_value( 0.0f )
{
	evaluator = new afxHermiteEval();
}

afxAnimCurve::~afxAnimCurve()
{
	delete evaluator;
}

void afxAnimCurve::addKey( Point2F &v )
{
	Key k;
	k.time  = v.x;
	k.value = v.y;
	
	keys.push_back( k );

	usable = false;	
}	

void afxAnimCurve::addKey( F32 time, F32 value )
{
	Key k;
	k.time  = time;
	k.value = value;
	
	keys.push_back( k );

	usable = false;	
}	

void afxAnimCurve::setKeyTime( int index, F32 t )
{
	if( ( index < 0 ) || ( index >= keys.size() ) )
		return;

	Key &k = keys[index];
	k.time = t;

	usable = false;
}

void afxAnimCurve::setKeyValue( int index, F32 v )
{
	if( ( index < 0 ) || ( index >= keys.size() ) )
		return;

	Key &k  = keys[index];
	k.value = v;

	if( index == 0 )
		start_value = v;
	else if( index == keys.size()-1 )
		final_value = v;
}

//bool afxAnimCurve::compare_Key( const afxAnimCurve::Key &a, const afxAnimCurve::Key &b ) 
//{
//	return a.time < b.time;
//}

S32 QSORT_CALLBACK afxAnimCurve::compare_Key( const void* a, const void* b )
{
	const Key *key_a = (Key *)a;
	const Key *key_b = (Key *)b;

	//Con::printf( "*** %f %f", key_a->time, key_b->time  );

	//return key_a->time < key_b->time;

	
	if (key_a->time > key_b->time)
      return 1;
   else if (key_a->time < key_b->time)
      return -1;
   else
      return 0;
}

void afxAnimCurve::sort( )
{
	if( keys.size() == 0 )
		return;

	//std::sort( keys.begin(), keys.end(), afxAnimCurve::compare_Key );
	dQsort( keys.address(), keys.size(), sizeof(Key), afxAnimCurve::compare_Key );
	
	start_value = keys[0].value;
	final_value = keys[keys.size()-1].value;

	start_time = keys[0].time;
	final_time = keys[keys.size()-1].time;

	usable = true;
}

int afxAnimCurve::numKeys()
{
	return keys.size();
}

F32 afxAnimCurve::getKeyTime( int index )
{
	if( ( index < 0 ) || ( index >= keys.size() ) )
		return 0.0f;

	Key &k = keys[index];
	return k.time;
}

F32 afxAnimCurve::getKeyValue( int index )
{
	if( ( index < 0 ) || ( index >= keys.size() ) )
		return 0.0f;

	Key &k = keys[index];
	return k.value;
}

Point2F afxAnimCurve::getSegment( F32 time )
{
	Point2F segment( 0, 0 );

	if( keys.size() == 0 )
		return segment;

	int start_index = 0;
	for( ; start_index < keys.size()-1; start_index++ )
	{
		if( time < keys[start_index+1].time )
			break;
	}
	int end_index = start_index+1;

	segment.x = (F32)start_index;
	segment.y = (F32)end_index;

	return segment;
}

F32 afxAnimCurve::evaluate( F32 time )
{
	if( !usable )
		return 0.0f;
	
	if( time <= start_time )
		return start_value;
	
	if( time >= final_time )
		return final_value;

	if( keys.size() == 1 )
		return start_value;

	int start_index = 0;
	for( ; start_index < keys.size()-1; start_index++ )
	{
		if( time < keys[start_index+1].time )
			break;
	}
	int end_index = start_index+1;

	Key k0 = keys[start_index];
	Key k1 = keys[end_index];

	Point2F v0( (F32) k0.time, k0.value );
	Point2F v1( (F32) k1.time, k1.value );

	// Compute tangents
	Point2F tan0 = computeTangentK0( v0, v1, start_index );
	Point2F tan1 = computeTangentK1( v0, v1, end_index );

	F32 time_perc = (F32)( time - k0.time ) / (F32)( k1.time - k0.time );

	Point2F vnew = evaluator->evaluateCurve( v0,
																					 v1,
																					 tan0,
																					 tan1,
																					 time_perc );

	return vnew.y;
}

Point2F afxAnimCurve::computeTangentK0( Point2F &k0, Point2F &k1, int start_index )
{
	Point2F tan0;
	
	Point2F k_prev;
	Point2F k_next;

		// tangent for k0
	if( start_index == 0 )
	{
		k_prev = k0;  	// Setting previous point to k0, creating a hidden point in
										//  the same spot
		k_next = k1;
	}
	else
	{
		Key &k = keys[start_index-1];
		k_prev.set( k.time, k.value );
		k_next = k1;
	}
	tan0 = k_next-k_prev; //k_next.subtract( k_prev );
	tan0 *= .5f;

	return tan0;
}

Point2F afxAnimCurve::computeTangentK1( Point2F &k0, Point2F &k1, int end_index )
{
	Point2F tan1;
	
	Point2F k_prev;
	Point2F k_next;

		// tangent for k1
	if( end_index == keys.size()-1 )
	{
		k_prev = k0;  
		k_next = k1;	// Setting next point to k1, creating a hidden point in
									//  the same spot
	}
	else
	{
		k_prev = k0;
		Key &k = keys[end_index+1];
		k_next.set( k.time, k.value );
	}
	tan1 = k_next-k_prev; //k_next.subtract( k_prev );
	tan1 *= .5f;

	return tan1;
}

void afxAnimCurve::print()
{
	Con::printf( "afxAnimCurve -------------------------" );
	for( int i = 0; i < keys.size(); i++ )
	{
		Key &k = keys[i];
		Con::printf( "%f: %f", k.time, k.value );
	}
	Con::printf( "-----------------------------------" );
}	

void afxAnimCurve::printKey( int index )
{
	Key &k = keys[index];
	Con::printf( "%f: %f", k.time, k.value );
}