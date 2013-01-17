
/*

NvStanHull.cpp : A convex hull generator written by Stan Melax

*/
/*!
**
** Copyright (c) 2009 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
** Portions of this source has been released with the PhysXViewer application, as well as
** Rocket, CreateDynamics, ODF, and as a number of sample code snippets.
**
** If you find this code useful or you are feeling particularily generous I would
** ask that you please go to http://www.amillionpixels.us and make a donation
** to Troy DeMolay.
**
** DeMolay is a youth group for young men between the ages of 12 and 21.
** It teaches strong moral principles, as well as leadership skills and
** public speaking.  The donations page uses the 'pay for pixels' paradigm
** where, in this case, a pixel is only a single penny.  Donations can be
** made for as small as $4 or as high as a $100 block.  Each person who donates
** will get a link to their own site as well as acknowledgement on the
** donations blog located here http://www.amillionpixels.blogspot.com/
**
** If you wish to contact me you can use the following methods:
**
** Skype ID: jratcliff63367
** Yahoo: jratcliff63367
** AOL: jratcliff1961
** email: jratcliffscarab@gmail.com
**
**
** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>


#include <stdarg.h>
#include <setjmp.h>

#include "NvStanHull.h"

namespace CONVEX_DECOMPOSITION
{

//*****************************************************
//*** DARRAY.H
//*****************************************************

template <class Type> class ArrayRet;
template <class Type> class Array
{
	public:
				Array(NxI32 s=0);
				Array(Array<Type> &array);
				Array(ArrayRet<Type> &array);
				~Array();
	void		allocate(NxI32 s);
	void		SetSize(NxI32 s);
	void		Pack();
	Type&		Add(Type);
	void		AddUnique(Type);
	NxI32 		Contains(Type);
	void		Insert(Type,NxI32);
	NxI32			IndexOf(Type);
	void		Remove(Type);
	void		DelIndex(NxI32 i);
	Type *		element;
	NxI32			count;
	NxI32			array_size;
	const Type	&operator[](NxI32 i) const { assert(i>=0 && i<count);  return element[i]; }
	Type		&operator[](NxI32 i)  { assert(i>=0 && i<count);  return element[i]; }
	Type		&Pop() { assert(count); count--;  return element[count]; }
	Array<Type> &operator=(Array<Type> &array);
	Array<Type> &operator=(ArrayRet<Type> &array);
	// operator ArrayRet<Type> &() { return *(ArrayRet<Type> *)this;} // this worked but i suspect could be dangerous
};

template <class Type> class ArrayRet:public Array<Type>
{
};

template <class Type> Array<Type>::Array(NxI32 s)
{
	count=0;
	array_size = 0;
	element = NULL;
	if(s)
	{
		allocate(s);
	}
}


template <class Type> Array<Type>::Array(Array<Type> &array)
{
	count=0;
	array_size = 0;
	element = NULL;
	for(NxI32 i=0;i<array.count;i++)
	{
		Add(array[i]);
	}
}


template <class Type> Array<Type>::Array(ArrayRet<Type> &array)
{
	*this = array;
}
template <class Type> Array<Type> &Array<Type>::operator=(ArrayRet<Type> &array)
{
	count=array.count;
	array_size = array.array_size;
	element = array.element;
	array.element=NULL;
	array.count=0;
	array.array_size=0;
	return *this;
}


template <class Type> Array<Type> &Array<Type>::operator=(Array<Type> &array)
{
	count=0;
	for(NxI32 i=0;i<array.count;i++)
	{
		Add(array[i]);
	}
	return *this;
}

template <class Type> Array<Type>::~Array()
{
	if (element != NULL)
	{
	  MEMALLOC_FREE(element);
	}
	count=0;array_size=0;element=NULL;
}

template <class Type> void Array<Type>::allocate(NxI32 s)
{
	assert(s>0);
	assert(s>=count);
	Type *old = element;
	array_size =s;
	element = (Type *) MEMALLOC_MALLOC( sizeof(Type)*array_size );
	assert(element);
	for(NxI32 i=0;i<count;i++)
	{
		element[i]=old[i];
	}
	if(old)
	{
		MEMALLOC_FREE(old);
	}
}

template <class Type> void Array<Type>::SetSize(NxI32 s)
{
	if(s==0)
	{
		if(element)
		{
			MEMALLOC_FREE(element);
			element = NULL;
		}
 	  array_size = s;
	}
	else
	{
		allocate(s);
	}
	count=s;
}

template <class Type> void Array<Type>::Pack()
{
	allocate(count);
}

template <class Type> Type& Array<Type>::Add(Type t)
{
	assert(count<=array_size);
	if(count==array_size)
	{
		allocate((array_size)?array_size *2:16);
	}
	element[count++] = t;
	return element[count-1];
}

template <class Type> NxI32 Array<Type>::Contains(Type t)
{
	NxI32 i;
	NxI32 found=0;
	for(i=0;i<count;i++)
	{
		if(element[i] == t) found++;
	}
	return found;
}

template <class Type> void Array<Type>::AddUnique(Type t)
{
	if(!Contains(t)) Add(t);
}


template <class Type> void Array<Type>::DelIndex(NxI32 i)
{
	assert(i<count);
	count--;
	while(i<count)
	{
		element[i] = element[i+1];
		i++;
	}
}

template <class Type> void Array<Type>::Remove(Type t)
{
	NxI32 i;
	for(i=0;i<count;i++)
	{
		if(element[i] == t)
		{
			break;
		}
	}
	assert(i<count); // assert object t is in the array.
	DelIndex(i);
	for(i=0;i<count;i++)
	{
		assert(element[i] != t);
	}
}

template <class Type> void Array<Type>::Insert(Type t,NxI32 k)
{
	NxI32 i=count;
	Add(t); // to allocate space
	while(i>k)
	{
		element[i]=element[i-1];
		i--;
	}
	assert(i==k);
	element[k]=t;
}


template <class Type> NxI32 Array<Type>::IndexOf(Type t)
{
	NxI32 i;
	for(i=0;i<count;i++)
	{
		if(element[i] == t)
		{
			return i;
		}
	}
	assert(0);
	return -1;
}

//****************************************************
//** VECMATH.H
//****************************************************
#define PI (3.1415926535897932384626433832795f)

#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (180.0f / PI)
#define SQRT_OF_2 (1.4142135f)
#define OFFSET(Class,Member)  (((char*) (&(((Class*)NULL)-> Member )))- ((char*)NULL))



NxI32    argmin(NxF32 a[],NxI32 n);
NxF32  sqr(NxF32 a); 
NxF32  clampf(NxF32 a) ;
NxF32  Round(NxF32 a,NxF32 precision);
NxF32  Interpolate(const NxF32 &f0,const NxF32 &f1,NxF32 alpha) ;

template <class T>
void Swap(T &a,T &b) 
{
	T tmp = a;
	a=b;
	b=tmp;
}



template <class T>
T Max(const T &a,const T &b) 
{
	return (a>b)?a:b;
}

template <class T>
T Min(const T &a,const T &b) 
{
	return (a<b)?a:b;
}

//----------------------------------

class int3  : public Memalloc
{
public:
	NxI32 x,y,z;
	int3(){};
	int3(NxI32 _x,NxI32 _y, NxI32 _z){x=_x;y=_y;z=_z;}
	const NxI32& operator[](NxI32 i) const {return (&x)[i];}
	NxI32& operator[](NxI32 i) {return (&x)[i];}
};


//-------- 2D --------

class float2  : public Memalloc
{
public:
	NxF32 x,y;
	float2(){x=0;y=0;};
	float2(NxF32 _x,NxF32 _y){x=_x;y=_y;}
	NxF32& operator[](NxI32 i) {assert(i>=0&&i<2);return ((NxF32*)this)[i];}
	const NxF32& operator[](NxI32 i) const {assert(i>=0&&i<2);return ((NxF32*)this)[i];}
};
inline float2 operator-( const float2& a, const float2& b ){return float2(a.x-b.x,a.y-b.y);}
inline float2 operator+( const float2& a, const float2& b ){return float2(a.x+b.x,a.y+b.y);}

//--------- 3D ---------

class float3  : public Memalloc // 3D
{
	public:
	NxF32 x,y,z;
	float3(){x=0;y=0;z=0;};
	float3(NxF32 _x,NxF32 _y,NxF32 _z){x=_x;y=_y;z=_z;};
	//operator NxF32 *() { return &x;};
	NxF32& operator[](NxI32 i) {assert(i>=0&&i<3);return ((NxF32*)this)[i];}
	const NxF32& operator[](NxI32 i) const {assert(i>=0&&i<3);return ((NxF32*)this)[i];}
};


float3& operator+=( float3 &a, const float3& b );
float3& operator-=( float3 &a ,const float3& b );
float3& operator*=( float3 &v ,const NxF32 s );
float3& operator/=( float3 &v, const NxF32 s );

NxF32  magnitude( const float3& v );
float3 normalize( const float3& v );
float3 safenormalize(const float3 &v);
float3 vabs(const float3 &v);
float3 operator+( const float3& a, const float3& b );
float3 operator-( const float3& a, const float3& b );
float3 operator-( const float3& v );
float3 operator*( const float3& v, const NxF32 s );
float3 operator*( const NxF32 s, const float3& v );
float3 operator/( const float3& v, const NxF32 s );
inline NxI32 operator==( const float3 &a, const float3 &b ) { return (a.x==b.x && a.y==b.y && a.z==b.z); }
inline NxI32 operator!=( const float3 &a, const float3 &b ) { return (a.x!=b.x || a.y!=b.y || a.z!=b.z); }
// due to ambiguity and inconsistent standards ther are no overloaded operators for mult such as va*vb.
NxF32  dot( const float3& a, const float3& b );
float3 cmul( const float3 &a, const float3 &b);
float3 cross( const float3& a, const float3& b );
float3 Interpolate(const float3 &v0,const float3 &v1,NxF32 alpha);
float3 Round(const float3& a,NxF32 precision);
float3	VectorMax(const float3 &a, const float3 &b);
float3	VectorMin(const float3 &a, const float3 &b);



class float3x3  : public Memalloc
{
	public:
	float3 x,y,z;  // the 3 rows of the Matrix
	float3x3(){}
	float3x3(NxF32 xx,NxF32 xy,NxF32 xz,NxF32 yx,NxF32 yy,NxF32 yz,NxF32 zx,NxF32 zy,NxF32 zz):x(xx,xy,xz),y(yx,yy,yz),z(zx,zy,zz){}
	float3x3(float3 _x,float3 _y,float3 _z):x(_x),y(_y),z(_z){}
	float3&       operator[](NxI32 i)       {assert(i>=0&&i<3);return (&x)[i];}
	const float3& operator[](NxI32 i) const {assert(i>=0&&i<3);return (&x)[i];}
	NxF32&        operator()(NxI32 r, NxI32 c)       {assert(r>=0&&r<3&&c>=0&&c<3);return ((&x)[r])[c];}
	const NxF32&  operator()(NxI32 r, NxI32 c) const {assert(r>=0&&r<3&&c>=0&&c<3);return ((&x)[r])[c];}
}; 
float3x3 Transpose( const float3x3& m );
float3   operator*( const float3& v  , const float3x3& m  );
float3   operator*( const float3x3& m , const float3& v   );
float3x3 operator*( const float3x3& m , const NxF32& s   );
float3x3 operator*( const float3x3& ma, const float3x3& mb );
float3x3 operator/( const float3x3& a, const NxF32& s ) ;
float3x3 operator+( const float3x3& a, const float3x3& b );
float3x3 operator-( const float3x3& a, const float3x3& b );
float3x3 &operator+=( float3x3& a, const float3x3& b );
float3x3 &operator-=( float3x3& a, const float3x3& b );
float3x3 &operator*=( float3x3& a, const NxF32& s );
NxF32    Determinant(const float3x3& m );
float3x3 Inverse(const float3x3& a);  // its just 3x3 so we simply do that cofactor method


//-------- 4D Math --------

class float4  : public Memalloc
{
public:
	NxF32 x,y,z,w;
	float4(){x=0;y=0;z=0;w=0;};
	float4(NxF32 _x,NxF32 _y,NxF32 _z,NxF32 _w){x=_x;y=_y;z=_z;w=_w;}
	float4(const float3 &v,NxF32 _w){x=v.x;y=v.y;z=v.z;w=_w;}
	//operator NxF32 *() { return &x;};
	NxF32& operator[](NxI32 i) {assert(i>=0&&i<4);return ((NxF32*)this)[i];}
	const NxF32& operator[](NxI32 i) const {assert(i>=0&&i<4);return ((NxF32*)this)[i];}
	const float3& xyz() const { return *((float3*)this);}
	float3&       xyz()       { return *((float3*)this);}
};


struct D3DXMATRIX; 

class float4x4  : public Memalloc
{
	public:
	float4 x,y,z,w;  // the 4 rows
	float4x4(){}
	float4x4(const float4 &_x, const float4 &_y, const float4 &_z, const float4 &_w):x(_x),y(_y),z(_z),w(_w){}
	float4x4(NxF32 m00, NxF32 m01, NxF32 m02, NxF32 m03, 
						NxF32 m10, NxF32 m11, NxF32 m12, NxF32 m13, 
				NxF32 m20, NxF32 m21, NxF32 m22, NxF32 m23, 
				NxF32 m30, NxF32 m31, NxF32 m32, NxF32 m33 )
			:x(m00,m01,m02,m03),y(m10,m11,m12,m13),z(m20,m21,m22,m23),w(m30,m31,m32,m33){}
	NxF32&       operator()(NxI32 r, NxI32 c)       {assert(r>=0&&r<4&&c>=0&&c<4);return ((&x)[r])[c];}
	const NxF32& operator()(NxI32 r, NxI32 c) const {assert(r>=0&&r<4&&c>=0&&c<4);return ((&x)[r])[c];}
		operator       NxF32* ()       {return &x.x;}
		operator const NxF32* () const {return &x.x;}
	operator       struct D3DXMATRIX* ()       { return (struct D3DXMATRIX*) this;}
	operator const struct D3DXMATRIX* () const { return (struct D3DXMATRIX*) this;}
};


NxI32     operator==( const float4 &a, const float4 &b );
float4 Homogenize(const float3 &v3,const NxF32 &w=1.0f); // Turns a 3D float3 4D vector4 by appending w
float4 cmul( const float4 &a, const float4 &b);
float4 operator*( const float4 &v, NxF32 s);
float4 operator*( NxF32 s, const float4 &v);
float4 operator+( const float4 &a, const float4 &b);
float4 operator-( const float4 &a, const float4 &b);
float4x4 operator*( const float4x4& a, const float4x4& b );
float4 operator*( const float4& v, const float4x4& m );
float4x4 Inverse(const float4x4 &m);
float4x4 MatrixRigidInverse(const float4x4 &m);
float4x4 MatrixTranspose(const float4x4 &m);
float4x4 MatrixPerspectiveFov(NxF32 fovy, NxF32 Aspect, NxF32 zn, NxF32 zf );
float4x4 MatrixTranslation(const float3 &t);
float4x4 MatrixRotationZ(const NxF32 angle_radians);
float4x4 MatrixLookAt(const float3& eye, const float3& at, const float3& up);
NxI32     operator==( const float4x4 &a, const float4x4 &b );


//-------- Quaternion ------------

class Quaternion :public float4
{
 public:
	Quaternion() { x = y = z = 0.0f; w = 1.0f; }
	Quaternion( float3 v, NxF32 t ) { v = normalize(v); w = cosf(t/2.0f); v = v*sinf(t/2.0f); x = v.x; y = v.y; z = v.z; }
	Quaternion(NxF32 _x, NxF32 _y, NxF32 _z, NxF32 _w){x=_x;y=_y;z=_z;w=_w;}
	NxF32 angle() const { return acosf(w)*2.0f; }
	float3 axis() const { float3 a(x,y,z); if(fabsf(angle())<0.0000001f) return float3(1,0,0); return a*(1/sinf(angle()/2.0f)); }
	float3 xdir() const { return float3( 1-2*(y*y+z*z),  2*(x*y+w*z),  2*(x*z-w*y) ); }
	float3 ydir() const { return float3(   2*(x*y-w*z),1-2*(x*x+z*z),  2*(y*z+w*x) ); }
	float3 zdir() const { return float3(   2*(x*z+w*y),  2*(y*z-w*x),1-2*(x*x+y*y) ); }
	float3x3 getmatrix() const { return float3x3( xdir(), ydir(), zdir() ); }
	operator float3x3() { return getmatrix(); }
	void Normalize();
};

Quaternion& operator*=(Quaternion& a, NxF32 s );
Quaternion	operator*( const Quaternion& a, NxF32 s );
Quaternion	operator*( const Quaternion& a, const Quaternion& b);
Quaternion	operator+( const Quaternion& a, const Quaternion& b );
Quaternion	normalize( Quaternion a );
NxF32		dot( const Quaternion &a, const Quaternion &b );
float3		operator*( const Quaternion& q, const float3& v );
float3		operator*( const float3& v, const Quaternion& q );
Quaternion	slerp( Quaternion a, const Quaternion& b, NxF32 interp );
Quaternion  Interpolate(const Quaternion &q0,const Quaternion &q1,NxF32 alpha); 
Quaternion  RotationArc(float3 v0, float3 v1 );  // returns quat q where q*v0=v1
Quaternion  Inverse(const Quaternion &q);
float4x4     MatrixFromQuatVec(const Quaternion &q, const float3 &v);


//------ Euler Angle -----

Quaternion YawPitchRoll( NxF32 yaw, NxF32 pitch, NxF32 roll );
NxF32 Yaw( const Quaternion& q );
NxF32 Pitch( const Quaternion& q );
NxF32 Roll( Quaternion q );
NxF32 Yaw( const float3& v );
NxF32 Pitch( const float3& v );


//------- Plane ----------

class Plane 
{
	public:
	float3	normal;
	NxF32	dist;   // distance below origin - the D from plane equasion Ax+By+Cz+D=0
			Plane(const float3 &n,NxF32 d):normal(n),dist(d){}
			Plane():normal(),dist(0){}
	void	Transform(const float3 &position, const Quaternion &orientation);
};

inline Plane PlaneFlip(const Plane &plane){return Plane(-plane.normal,-plane.dist);}
inline NxI32 operator==( const Plane &a, const Plane &b ) { return (a.normal==b.normal && a.dist==b.dist); }
inline NxI32 coplanar( const Plane &a, const Plane &b ) { return (a==b || a==PlaneFlip(b)); }


//--------- Utility Functions ------

float3  PlaneLineIntersection(const Plane &plane, const float3 &p0, const float3 &p1);
float3  PlaneProject(const Plane &plane, const float3 &point);
float3  LineProject(const float3 &p0, const float3 &p1, const float3 &a);  // projects a onto infinite line p0p1
NxF32   LineProjectTime(const float3 &p0, const float3 &p1, const float3 &a);
float3  ThreePlaneIntersection(const Plane &p0,const Plane &p1, const Plane &p2);
NxI32     PolyHit(const float3 *vert,const NxI32 n,const float3 &v0, const float3 &v1, float3 *impact=NULL, float3 *normal=NULL);
NxI32     BoxInside(const float3 &p,const float3 &bmin, const float3 &bmax) ;
NxI32     BoxIntersect(const float3 &v0, const float3 &v1, const float3 &bmin, const float3 &bmax, float3 *impact);
NxF32   DistanceBetweenLines(const float3 &ustart, const float3 &udir, const float3 &vstart, const float3 &vdir, float3 *upoint=NULL, float3 *vpoint=NULL);
float3  TriNormal(const float3 &v0, const float3 &v1, const float3 &v2);
float3  NormalOf(const float3 *vert, const NxI32 n);
Quaternion VirtualTrackBall(const float3 &cop, const float3 &cor, const float3 &dir0, const float3 &dir1);




//*****************************************************
// ** VECMATH.CPP
//*****************************************************


NxF32   sqr(NxF32 a) {return a*a;}
NxF32   clampf(NxF32 a) {return Min(1.0f,Max(0.0f,a));}


NxF32 Round(NxF32 a,NxF32 precision)
{
	return floorf(0.5f+a/precision)*precision;
}


NxF32 Interpolate(const NxF32 &f0,const NxF32 &f1,NxF32 alpha) 
{
	return f0*(1-alpha) + f1*alpha;
}


NxI32     argmin(NxF32 a[],NxI32 n)
{
	NxI32 r=0;
	for(NxI32 i=1;i<n;i++) 
		{
		if(a[i]<a[r]) 
				{
			r = i;			
		}
	}
	return r;
}



//------------ float3 (3D) --------------



float3 operator+( const float3& a, const float3& b ) 
{
	return float3(a.x+b.x, a.y+b.y, a.z+b.z); 
}


float3 operator-( const float3& a, const float3& b )
{
	return float3( a.x-b.x, a.y-b.y, a.z-b.z ); 
}


float3 operator-( const float3& v )                     
{
	return float3( -v.x, -v.y, -v.z ); 
}


float3 operator*( const float3& v, NxF32 s )      
{
	return float3( v.x*s, v.y*s, v.z*s ); 
}


float3 operator*( NxF32 s, const float3& v )      
{
	return float3( v.x*s, v.y*s, v.z*s ); 
}


float3 operator/( const float3& v, NxF32 s )
{ 
	return v*(1.0f/s); 
}

NxF32  dot( const float3& a, const float3& b )    
{
	return a.x*b.x + a.y*b.y + a.z*b.z; 
}

float3 cmul( const float3 &v1, const float3 &v2) 
{ 
	return float3(v1.x*v2.x, v1.y*v2.y, v1.z*v2.z); 
}


float3 cross( const float3& a, const float3& b )
{
		return float3( a.y*b.z - a.z*b.y,
									 a.z*b.x - a.x*b.z,
									 a.x*b.y - a.y*b.x );
}




float3& operator+=( float3& a , const float3& b )
{
		a.x += b.x;
		a.y += b.y;
		a.z += b.z;
		return a;
}


float3& operator-=( float3& a , const float3& b )
{
		a.x -= b.x;
		a.y -= b.y;
		a.z -= b.z;
		return a;
}


float3& operator*=(float3& v , NxF32 s )
{
		v.x *= s;
		v.y *= s;
		v.z *= s;
		return v;
}


float3& operator/=(float3& v , NxF32 s )
{
		NxF32 sinv = 1.0f / s;
		v.x *= sinv;
		v.y *= sinv;
		v.z *= sinv;
		return v;
}

float3 vabs(const float3 &v)
{
	return float3(fabsf(v.x),fabsf(v.y),fabsf(v.z));
}


NxF32 magnitude( const float3& v )
{
		return sqrtf(sqr(v.x) + sqr( v.y)+ sqr(v.z));
}



float3 normalize( const float3 &v )
{
	// this routine, normalize, is ok, provided magnitude works!!
		NxF32 d=magnitude(v);
		if (d==0) 
		{
		printf("Cant normalize ZERO vector\n");
		assert(0);// yes this could go here
		d=0.1f;
	}
	d = 1/d;
	return float3(v.x*d,v.y*d,v.z*d);
}

float3 safenormalize(const float3 &v)
{
	if(magnitude(v)<=0.0f)
	{
		return float3(1,0,0);
	}
	return normalize(v);
}

float3 Round(const float3 &a,NxF32 precision)
{
	return float3(Round(a.x,precision),Round(a.y,precision),Round(a.z,precision));
}


float3 Interpolate(const float3 &v0,const float3 &v1,NxF32 alpha) 
{
	return v0*(1-alpha) + v1*alpha;
}

float3 VectorMin(const float3 &a,const float3 &b)
{
	return float3(Min(a.x,b.x),Min(a.y,b.y),Min(a.z,b.z));
}
float3 VectorMax(const float3 &a,const float3 &b)
{
	return float3(Max(a.x,b.x),Max(a.y,b.y),Max(a.z,b.z));
}

// the statement v1*v2 is ambiguous since there are 3 types
// of vector multiplication
//  - componantwise (for example combining colors)
//  - dot product
//  - cross product
// Therefore we never declare/implement this function.
// So we will never see:  float3 operator*(float3 a,float3 b) 




//------------ float3x3 ---------------
NxF32 Determinant(const float3x3 &m)
{
	return  m.x.x*m.y.y*m.z.z + m.y.x*m.z.y*m.x.z + m.z.x*m.x.y*m.y.z 
			 -m.x.x*m.z.y*m.y.z - m.y.x*m.x.y*m.z.z - m.z.x*m.y.y*m.x.z ;
}

float3x3 Inverse(const float3x3 &a)
{
	float3x3 b;
	NxF32 d=Determinant(a);
	assert(d!=0);
	for(NxI32 i=0;i<3;i++) 
		{
		for(NxI32 j=0;j<3;j++) 
				{
			NxI32 i1=(i+1)%3;
			NxI32 i2=(i+2)%3;
			NxI32 j1=(j+1)%3;
			NxI32 j2=(j+2)%3;
			// reverse indexs i&j to take transpose
			b[j][i] = (a[i1][j1]*a[i2][j2]-a[i1][j2]*a[i2][j1])/d;
		}
	}
	// Matrix check=a*b; // Matrix 'check' should be the identity (or close to it)
	return b;
}


float3x3 Transpose( const float3x3& m )
{
	return float3x3( float3(m.x.x,m.y.x,m.z.x),
					float3(m.x.y,m.y.y,m.z.y),
					float3(m.x.z,m.y.z,m.z.z));
}


float3 operator*(const float3& v , const float3x3 &m ) {
	return float3((m.x.x*v.x + m.y.x*v.y + m.z.x*v.z), 
					(m.x.y*v.x + m.y.y*v.y + m.z.y*v.z), 
					(m.x.z*v.x + m.y.z*v.y + m.z.z*v.z));
}
float3 operator*(const float3x3 &m,const float3& v  ) { 
	return float3(dot(m.x,v),dot(m.y,v),dot(m.z,v));
}


float3x3 operator*( const float3x3& a, const float3x3& b )  
{ 
	return float3x3(a.x*b,a.y*b,a.z*b);
}

float3x3 operator*( const float3x3& a, const NxF32& s )  
{ 
	return float3x3(a.x*s, a.y*s ,a.z*s); 
}
float3x3 operator/( const float3x3& a, const NxF32& s )  
{ 
	NxF32 t=1/s;
	return float3x3(a.x*t, a.y*t ,a.z*t); 
}
float3x3 operator+( const float3x3& a, const float3x3& b )
{
	return float3x3(a.x+b.x, a.y+b.y, a.z+b.z);
}
float3x3 operator-( const float3x3& a, const float3x3& b )
{
	return float3x3(a.x-b.x, a.y-b.y, a.z-b.z);
}
float3x3 &operator+=( float3x3& a, const float3x3& b )
{
	a.x+=b.x;
	a.y+=b.y;
	a.z+=b.z;
	return a;
}
float3x3 &operator-=( float3x3& a, const float3x3& b )
{
	a.x-=b.x;
	a.y-=b.y;
	a.z-=b.z;
	return a;
}
float3x3 &operator*=( float3x3& a, const NxF32& s )
{
	a.x*=s;
	a.y*=s;
	a.z*=s;
	return a;
}



float3 ThreePlaneIntersection(const Plane &p0,const Plane &p1, const Plane &p2){
	float3x3 mp =Transpose(float3x3(p0.normal,p1.normal,p2.normal));
	float3x3 mi = Inverse(mp);
	float3 b(p0.dist,p1.dist,p2.dist);
	return -b * mi;
}


//--------------- 4D ----------------

float4   operator*( const float4&   v, const float4x4& m )
{
	return v.x*m.x + v.y*m.y + v.z*m.z + v.w*m.w; // yes this actually works
}

NxI32 operator==( const float4 &a, const float4 &b ) 
{
	return (a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w);
}


//  Dont implement m*v for now, since that might confuse us
//  All our transforms are based on multiplying the "row" vector on the left
//float4   operator*(const float4x4& m , const float4&   v )
//{
//	return float4(dot(v,m.x),dot(v,m.y),dot(v,m.z),dot(v,m.w));
//}



float4 cmul( const float4 &a, const float4 &b) 
{
	return float4(a.x*b.x,a.y*b.y,a.z*b.z,a.w*b.w);
}


float4 operator*( const float4 &v, NxF32 s) 
{
	return float4(v.x*s,v.y*s,v.z*s,v.w*s);
}


float4 operator*( NxF32 s, const float4 &v) 
{
	return float4(v.x*s,v.y*s,v.z*s,v.w*s);
}


float4 operator+( const float4 &a, const float4 &b) 
{
	return float4(a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w);
}



float4 operator-( const float4 &a, const float4 &b) 
{
	return float4(a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w);
}


float4 Homogenize(const float3 &v3,const NxF32 &w)
{
	return float4(v3.x,v3.y,v3.z,w);
}



float4x4 operator*( const float4x4& a, const float4x4& b )
{
	return float4x4(a.x*b,a.y*b,a.z*b,a.w*b);
}

float4x4 MatrixTranspose(const float4x4 &m)
{
	return float4x4(
		m.x.x, m.y.x, m.z.x, m.w.x,
		m.x.y, m.y.y, m.z.y, m.w.y,
		m.x.z, m.y.z, m.z.z, m.w.z,
		m.x.w, m.y.w, m.z.w, m.w.w );
}

float4x4 MatrixRigidInverse(const float4x4 &m)
{
	float4x4 trans_inverse = MatrixTranslation(-m.w.xyz());
	float4x4 rot   = m;
	rot.w = float4(0,0,0,1);
	return trans_inverse * MatrixTranspose(rot);
}


float4x4 MatrixPerspectiveFov(NxF32 fovy, NxF32 aspect, NxF32 zn, NxF32 zf )
{
	NxF32 h = 1.0f/tanf(fovy/2.0f); // view space height
	NxF32 w = h / aspect ;  // view space width
	return float4x4(
		w, 0, 0             ,   0,
		0, h, 0             ,   0,
		0, 0, zf/(zn-zf)    ,  -1,
		0, 0, zn*zf/(zn-zf) ,   0 );
}



float4x4 MatrixLookAt(const float3& eye, const float3& at, const float3& up)
{
	float4x4 m;
	m.w.w = 1.0f;
	m.w.xyz() = eye;
	m.z.xyz() = normalize(eye-at);
	m.x.xyz() = normalize(cross(up,m.z.xyz()));
	m.y.xyz() = cross(m.z.xyz(),m.x.xyz());
	return MatrixRigidInverse(m);
}


float4x4 MatrixTranslation(const float3 &t)
{
	return float4x4(
		1,  0,  0,  0,
		0,  1,  0,  0,
		0,  0,  1,  0,
		t.x,t.y,t.z,1 );
}


float4x4 MatrixRotationZ(const NxF32 angle_radians)
{
	NxF32 s =  sinf(angle_radians);
	NxF32 c =  cosf(angle_radians);
	return float4x4(
		c,  s,  0,  0,
		-s, c,  0,  0,
		0,  0,  1,  0,
		0,  0,  0,  1 );
}



NxI32 operator==( const float4x4 &a, const float4x4 &b )
{
	return (a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w);
}


float4x4 Inverse(const float4x4 &m)
{
	float4x4 d;
	NxF32 *dst = &d.x.x;
	NxF32 tmp[12]; /* temp array for pairs */
	NxF32 src[16]; /* array of transpose source matrix */
	NxF32 det; /* determinant */
	/* transpose matrix */
	for ( NxI32 i = 0; i < 4; i++) {
		src[i] = m(i,0) ;
		src[i + 4] = m(i,1);
		src[i + 8] = m(i,2);
		src[i + 12] = m(i,3); 
	}
	/* calculate pairs for first 8 elements (cofactors) */
	tmp[0]  = src[10] * src[15];
	tmp[1]  = src[11] * src[14];
	tmp[2]  = src[9] * src[15];
	tmp[3]  = src[11] * src[13];
	tmp[4]  = src[9] * src[14];
	tmp[5]  = src[10] * src[13];
	tmp[6]  = src[8] * src[15];
	tmp[7]  = src[11] * src[12];
	tmp[8]  = src[8] * src[14];
	tmp[9]  = src[10] * src[12];
	tmp[10] = src[8] * src[13];
	tmp[11] = src[9] * src[12];
	/* calculate first 8 elements (cofactors) */
	dst[0]  = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
	dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
	dst[1]  = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
	dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
	dst[2]  = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
	dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
	dst[3]  = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
	dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
	dst[4]  = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
	dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
	dst[5]  = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
	dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
	dst[6]  = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
	dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
	dst[7]  = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
	dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];
	/* calculate pairs for second 8 elements (cofactors) */
	tmp[0]  = src[2]*src[7];
	tmp[1]  = src[3]*src[6];
	tmp[2]  = src[1]*src[7];
	tmp[3]  = src[3]*src[5];
	tmp[4]  = src[1]*src[6];
	tmp[5]  = src[2]*src[5];
	tmp[6]  = src[0]*src[7];
	tmp[7]  = src[3]*src[4];
	tmp[8]  = src[0]*src[6];
	tmp[9]  = src[2]*src[4];
	tmp[10] = src[0]*src[5];
	tmp[11] = src[1]*src[4];
	/* calculate second 8 elements (cofactors) */
	dst[8]  = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
	dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
	dst[9]  = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
	dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
	dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
	dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
	dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
	dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
	dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
	dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
	dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
	dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
	dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
	dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
	dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
	dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];
	/* calculate determinant */
	det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];
	/* calculate matrix inverse */
	det = 1/det;
	for ( NxI32 j = 0; j < 16; j++)
	dst[j] *= det;
	return d;
}


//--------- Quaternion --------------
	
Quaternion operator*( const Quaternion& a, const Quaternion& b )
{
	Quaternion c;
	c.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z; 
	c.x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y; 
	c.y = a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x; 
	c.z = a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w; 
	return c;
}


Quaternion operator*( const Quaternion& a, NxF32 b )
{
	return Quaternion(a.x*b, a.y*b, a.z*b ,a.w*b);
}

Quaternion  Inverse(const Quaternion &q)
{
	return Quaternion(-q.x,-q.y,-q.z,q.w);
}

Quaternion& operator*=( Quaternion& q, const NxF32 s )
{
		q.x *= s;
		q.y *= s;
		q.z *= s;
		q.w *= s;
		return q;
}
void Quaternion::Normalize()
{
	NxF32 m = sqrtf(sqr(w)+sqr(x)+sqr(y)+sqr(z));
	if(m<0.000000001f) {
		w=1.0f;
		x=y=z=0.0f;
		return;
	}
	(*this) *= (1.0f/m);
}

float3 operator*( const Quaternion& q, const float3& v )
{
	// The following is equivalent to:   
	//return (q.getmatrix() * v);  
	NxF32 qx2 = q.x*q.x;
	NxF32 qy2 = q.y*q.y;
	NxF32 qz2 = q.z*q.z;

	NxF32 qxqy = q.x*q.y;
	NxF32 qxqz = q.x*q.z;
	NxF32 qxqw = q.x*q.w;
	NxF32 qyqz = q.y*q.z;
	NxF32 qyqw = q.y*q.w;
	NxF32 qzqw = q.z*q.w;
	return float3(
		(1-2*(qy2+qz2))*v.x + (2*(qxqy-qzqw))*v.y + (2*(qxqz+qyqw))*v.z ,
		(2*(qxqy+qzqw))*v.x + (1-2*(qx2+qz2))*v.y + (2*(qyqz-qxqw))*v.z ,
		(2*(qxqz-qyqw))*v.x + (2*(qyqz+qxqw))*v.y + (1-2*(qx2+qy2))*v.z  );
}

Quaternion operator+( const Quaternion& a, const Quaternion& b )
{
	return Quaternion(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
}

NxF32 dot( const Quaternion &a,const Quaternion &b )
{
	return  (a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z); 
}

Quaternion normalize( Quaternion a )
{
	NxF32 m = sqrtf(sqr(a.w)+sqr(a.x)+sqr(a.y)+sqr(a.z));
	if(m<0.000000001) 
		{    
		a.w=1;
		a.x=a.y=a.z=0;
		return a;
	}
	return a * (1/m);
}

Quaternion slerp( Quaternion a, const Quaternion& b, NxF32 interp )
{
	if(dot(a,b) <0.0) 
		{
		a.w=-a.w;
		a.x=-a.x;
		a.y=-a.y;
		a.z=-a.z;
	}
	NxF32 d = dot(a,b);
	if(d>=1.0) {
		return a;
	}
	NxF32 theta = acosf(d);
	if(theta==0.0f) { return(a);}
	return a*(sinf(theta-interp*theta)/sinf(theta)) + b*(sinf(interp*theta)/sinf(theta));
}


Quaternion Interpolate(const Quaternion &q0,const Quaternion &q1,NxF32 alpha) {
	return slerp(q0,q1,alpha);
}


Quaternion YawPitchRoll( NxF32 yaw, NxF32 pitch, NxF32 roll ) 
{
	roll  *= DEG2RAD;
	yaw   *= DEG2RAD;
	pitch *= DEG2RAD;
	return Quaternion(float3(0.0f,0.0f,1.0f),yaw)*Quaternion(float3(1.0f,0.0f,0.0f),pitch)*Quaternion(float3(0.0f,1.0f,0.0f),roll);
}

NxF32 Yaw( const Quaternion& q )
{
	static float3 v;
	v=q.ydir();
	return (v.y==0.0&&v.x==0.0) ? 0.0f: atan2f(-v.x,v.y)*RAD2DEG;
}

NxF32 Pitch( const Quaternion& q )
{
	static float3 v;
	v=q.ydir();
	return atan2f(v.z,sqrtf(sqr(v.x)+sqr(v.y)))*RAD2DEG;
}

NxF32 Roll( Quaternion q )
{
	q = Quaternion(float3(0.0f,0.0f,1.0f),-Yaw(q)*DEG2RAD)  *q;
	q = Quaternion(float3(1.0f,0.0f,0.0f),-Pitch(q)*DEG2RAD)  *q;
	return atan2f(-q.xdir().z,q.xdir().x)*RAD2DEG;
}

NxF32 Yaw( const float3& v )
{
	return (v.y==0.0&&v.x==0.0) ? 0.0f: atan2f(-v.x,v.y)*RAD2DEG;
}

NxF32 Pitch( const float3& v )
{
	return atan2f(v.z,sqrtf(sqr(v.x)+sqr(v.y)))*RAD2DEG;
}


//------------- Plane --------------


void Plane::Transform(const float3 &position, const Quaternion &orientation) {
	//   Transforms the plane to the space defined by the 
	//   given position/orientation.
	static float3 newnormal;
	static float3 origin;

	newnormal = Inverse(orientation)*normal;
	origin = Inverse(orientation)*(-normal*dist - position);

	normal = newnormal;
	dist = -dot(newnormal, origin);
}




//--------- utility functions -------------

//        RotationArc()
// Given two vectors v0 and v1 this function
// returns quaternion q where q*v0==v1.
// Routine taken from game programming gems.
Quaternion RotationArc(float3 v0,float3 v1){
	static Quaternion q;
	v0 = normalize(v0);  // Comment these two lines out if you know its not needed.
	v1 = normalize(v1);  // If vector is already unit length then why do it again?
	float3  c = cross(v0,v1);
	NxF32   d = dot(v0,v1);
	if(d<=-1.0f) { return Quaternion(1,0,0,0);} // 180 about x axis
	NxF32   s = sqrtf((1+d)*2);
	q.x = c.x / s;
	q.y = c.y / s;
	q.z = c.z / s;
	q.w = s /2.0f;
	return q;
}


float4x4 MatrixFromQuatVec(const Quaternion &q, const float3 &v) 
{
	// builds a 4x4 transformation matrix based on orientation q and translation v 
	NxF32 qx2 = q.x*q.x;
	NxF32 qy2 = q.y*q.y;
	NxF32 qz2 = q.z*q.z;

	NxF32 qxqy = q.x*q.y;
	NxF32 qxqz = q.x*q.z;
	NxF32 qxqw = q.x*q.w;
	NxF32 qyqz = q.y*q.z;
	NxF32 qyqw = q.y*q.w;
	NxF32 qzqw = q.z*q.w;

	return float4x4(
		1-2*(qy2+qz2),  
		2*(qxqy+qzqw),  
		2*(qxqz-qyqw),  
		0            ,  
		2*(qxqy-qzqw),  
		1-2*(qx2+qz2),  
		2*(qyqz+qxqw),  
		0            ,  
		2*(qxqz+qyqw),  
		2*(qyqz-qxqw),  
		1-2*(qx2+qy2),  
		0    , 
		 v.x ,
		 v.y ,
		 v.z ,
		 1.0f );
}


float3 PlaneLineIntersection(const Plane &plane, const float3 &p0, const float3 &p1)
{
	// returns the point where the line p0-p1 intersects the plane n&d
				static float3 dif;
		dif = p1-p0;
				NxF32 dn= dot(plane.normal,dif);
				NxF32 t = -(plane.dist+dot(plane.normal,p0) )/dn;
				return p0 + (dif*t);
}

float3 PlaneProject(const Plane &plane, const float3 &point)
{
	return point - plane.normal * (dot(point,plane.normal)+plane.dist);
}

float3 LineProject(const float3 &p0, const float3 &p1, const float3 &a)
{
	float3 w;
	w = p1-p0;
	NxF32 t= dot(w,(a-p0)) / (sqr(w.x)+sqr(w.y)+sqr(w.z));
	return p0+ w*t;
}


NxF32 LineProjectTime(const float3 &p0, const float3 &p1, const float3 &a)
{
	float3 w;
	w = p1-p0;
	NxF32 t= dot(w,(a-p0)) / (sqr(w.x)+sqr(w.y)+sqr(w.z));
	return t;
}



float3 TriNormal(const float3 &v0, const float3 &v1, const float3 &v2)
{
	// return the normal of the triangle
	// inscribed by v0, v1, and v2
	float3 cp=cross(v1-v0,v2-v1);
	NxF32 m=magnitude(cp);
	if(m==0) return float3(1,0,0);
	return cp*(1.0f/m);
}



NxI32 BoxInside(const float3 &p, const float3 &bmin, const float3 &bmax) 
{
	return (p.x >= bmin.x && p.x <=bmax.x && 
			p.y >= bmin.y && p.y <=bmax.y && 
			p.z >= bmin.z && p.z <=bmax.z );
}


NxI32 BoxIntersect(const float3 &v0, const float3 &v1, const float3 &bmin, const float3 &bmax,float3 *impact)
{
	if(BoxInside(v0,bmin,bmax))
		{
				*impact=v0;
				return 1;
		}
	if(v0.x<=bmin.x && v1.x>=bmin.x) 
		{
		NxF32 a = (bmin.x-v0.x)/(v1.x-v0.x);
		//v.x = bmin.x;
		NxF32 vy =  (1-a) *v0.y + a*v1.y;
		NxF32 vz =  (1-a) *v0.z + a*v1.z;
		if(vy>=bmin.y && vy<=bmax.y && vz>=bmin.z && vz<=bmax.z) 
				{
			impact->x = bmin.x;
			impact->y = vy;
			impact->z = vz;
			return 1;
		}
	}
	else if(v0.x >= bmax.x  &&  v1.x <= bmax.x) 
		{
		NxF32 a = (bmax.x-v0.x)/(v1.x-v0.x);
		//v.x = bmax.x;
		NxF32 vy =  (1-a) *v0.y + a*v1.y;
		NxF32 vz =  (1-a) *v0.z + a*v1.z;
		if(vy>=bmin.y && vy<=bmax.y && vz>=bmin.z && vz<=bmax.z) 
				{
			impact->x = bmax.x;
			impact->y = vy;
			impact->z = vz;
			return 1;
		}
	}
	if(v0.y<=bmin.y && v1.y>=bmin.y) 
		{
		NxF32 a = (bmin.y-v0.y)/(v1.y-v0.y);
		NxF32 vx =  (1-a) *v0.x + a*v1.x;
		//v.y = bmin.y;
		NxF32 vz =  (1-a) *v0.z + a*v1.z;
		if(vx>=bmin.x && vx<=bmax.x && vz>=bmin.z && vz<=bmax.z) 
				{
			impact->x = vx;
			impact->y = bmin.y;
			impact->z = vz;
			return 1;
		}
	}
	else if(v0.y >= bmax.y  &&  v1.y <= bmax.y) 
		{
		NxF32 a = (bmax.y-v0.y)/(v1.y-v0.y);
		NxF32 vx =  (1-a) *v0.x + a*v1.x;
		// vy = bmax.y;
		NxF32 vz =  (1-a) *v0.z + a*v1.z;
		if(vx>=bmin.x && vx<=bmax.x && vz>=bmin.z && vz<=bmax.z) 
				{
			impact->x = vx;
			impact->y = bmax.y;
			impact->z = vz;
			return 1;
		}
	}
	if(v0.z<=bmin.z && v1.z>=bmin.z) 
		{
		NxF32 a = (bmin.z-v0.z)/(v1.z-v0.z);
		NxF32 vx =  (1-a) *v0.x + a*v1.x;
		NxF32 vy =  (1-a) *v0.y + a*v1.y;
		// v.z = bmin.z;
		if(vy>=bmin.y && vy<=bmax.y && vx>=bmin.x && vx<=bmax.x) 
				{
			impact->x = vx;
			impact->y = vy;
			impact->z = bmin.z;
			return 1;
		}
	}
	else if(v0.z >= bmax.z  &&  v1.z <= bmax.z) 
		{
		NxF32 a = (bmax.z-v0.z)/(v1.z-v0.z);
		NxF32 vx =  (1-a) *v0.x + a*v1.x;
		NxF32 vy =  (1-a) *v0.y + a*v1.y;
		// v.z = bmax.z;
		if(vy>=bmin.y && vy<=bmax.y && vx>=bmin.x && vx<=bmax.x) 
				{
			impact->x = vx;
			impact->y = vy;
			impact->z = bmax.z;
			return 1;
		}
	}
	return 0;
}


NxF32 DistanceBetweenLines(const float3 &ustart, const float3 &udir, const float3 &vstart, const float3 &vdir, float3 *upoint, float3 *vpoint)
{
	static float3 cp;
	cp = normalize(cross(udir,vdir));

	NxF32 distu = -dot(cp,ustart);
	NxF32 distv = -dot(cp,vstart);
	NxF32 dist = (NxF32)fabs(distu-distv);
	if(upoint) 
		{
		Plane plane;
		plane.normal = normalize(cross(vdir,cp));
		plane.dist = -dot(plane.normal,vstart);
		*upoint = PlaneLineIntersection(plane,ustart,ustart+udir);
	}
	if(vpoint) 
		{
		Plane plane;
		plane.normal = normalize(cross(udir,cp));
		plane.dist = -dot(plane.normal,ustart);
		*vpoint = PlaneLineIntersection(plane,vstart,vstart+vdir);
	}
	return dist;
}


Quaternion VirtualTrackBall(const float3 &cop, const float3 &cor, const float3 &dir1, const float3 &dir2) 
{
	// routine taken from game programming gems.
	// Implement track ball functionality to spin stuf on the screen
	//  cop   center of projection
	//  cor   center of rotation
	//  dir1  old mouse direction 
	//  dir2  new mouse direction
	// pretend there is a sphere around cor.  Then find the points
	// where dir1 and dir2 intersect that sphere.  Find the
	// rotation that takes the first point to the second.
	NxF32 m;
	// compute plane 
	float3 nrml = cor - cop;
	NxF32 fudgefactor = 1.0f/(magnitude(nrml) * 0.25f); // since trackball proportional to distance from cop
	nrml = normalize(nrml);
	NxF32 dist = -dot(nrml,cor);
	float3 u= PlaneLineIntersection(Plane(nrml,dist),cop,cop+dir1);
	u=u-cor;
	u=u*fudgefactor;
	m= magnitude(u);
	if(m>1) 
		{
				u/=m;
		}
	else 
		{
		u=u - (nrml * sqrtf(1-m*m));
	}
	float3 v= PlaneLineIntersection(Plane(nrml,dist),cop,cop+dir2);
	v=v-cor;
	v=v*fudgefactor;
	m= magnitude(v);
	if(m>1) 
		{
				v/=m;
		}
	else
		{
		v=v - (nrml * sqrtf(1-m*m));
	}
	return RotationArc(u,v);
}


NxI32 countpolyhit=0;
NxI32 PolyHit(const float3 *vert, const NxI32 n, const float3 &v0, const float3 &v1, float3 *impact, float3 *normal)
{
	countpolyhit++;
	NxI32 i;
	float3 nrml(0,0,0);
	for(i=0;i<n;i++) 
		{
		NxI32 i1=(i+1)%n;
		NxI32 i2=(i+2)%n;
		nrml = nrml + cross(vert[i1]-vert[i],vert[i2]-vert[i1]);
	}

	NxF32 m = magnitude(nrml);
	if(m==0.0)
		{
				return 0;
		}
	nrml = nrml * (1.0f/m);
	NxF32 dist = -dot(nrml,vert[0]);
	NxF32 d0,d1;
	if((d0=dot(v0,nrml)+dist) <0  ||  (d1=dot(v1,nrml)+dist) >0) 
		{        
				return 0;
		}

	static float3 the_point; 
	// By using the cached plane distances d0 and d1
	// we can optimize the following:
	//     the_point = planelineintersection(nrml,dist,v0,v1);
	NxF32 a = d0/(d0-d1);
	the_point = v0*(1-a) + v1*a;


	NxI32 inside=1;
	for(NxI32 j=0;inside && j<n;j++) 
		{
			// let inside = 0 if outside
			float3 pp1,pp2,side;
			pp1 = vert[j] ;
			pp2 = vert[(j+1)%n];
			side = cross((pp2-pp1),(the_point-pp1));
			inside = (dot(nrml,side) >= 0.0);
	}
	if(inside) 
		{
		if(normal){*normal=nrml;}
		if(impact){*impact=the_point;}
	}
	return inside;
}

//****************************************************
// HULL.H source code goes here
//****************************************************
class PHullResult
{
public:

	PHullResult(void)
	{
		mVcount = 0;
		mIndexCount = 0;
		mFaceCount = 0;
		mVertices = 0;
		mIndices  = 0;
	}

	NxU32 mVcount;
	NxU32 mIndexCount;
	NxU32 mFaceCount;
	NxF32       *mVertices;
	NxU32 *mIndices;
};

bool ComputeHull(NxU32 vcount,const NxF32 *vertices,PHullResult &result,NxU32 maxverts,NxF32 inflate);
void ReleaseHull(PHullResult &result);

//*****************************************************
// HULL.cpp source code goes here
//*****************************************************


#define REAL3 float3
#define REAL  NxF32

#define COPLANAR   (0)
#define UNDER      (1)
#define OVER       (2)
#define SPLIT      (OVER|UNDER)
#define PAPERWIDTH (0.001f)
#define VOLUME_EPSILON (1e-20f)

NxF32 planetestepsilon = PAPERWIDTH;

class ConvexH : public Memalloc
{
  public:
	class HalfEdge
	{
	  public:
		short ea;         // the other half of the edge (index into edges list)
		NxU8 v;  // the vertex at the start of this edge (index into vertices list)
		NxU8 p;  // the facet on which this edge lies (index into facets list)
		HalfEdge(){}
		HalfEdge(short _ea,NxU8 _v, NxU8 _p):ea(_ea),v(_v),p(_p){}
	};
	Array<REAL3> vertices;
	Array<HalfEdge> edges;
	Array<Plane>  facets;
	ConvexH(NxI32 vertices_size,NxI32 edges_size,NxI32 facets_size);
};

typedef ConvexH::HalfEdge HalfEdge;

ConvexH::ConvexH(NxI32 vertices_size,NxI32 edges_size,NxI32 facets_size)
	:vertices(vertices_size)
	,edges(edges_size)
	,facets(facets_size)
{
	vertices.count=vertices_size;
	edges.count   = edges_size;
	facets.count  = facets_size;
}

ConvexH *ConvexHDup(ConvexH *src)
{
	ConvexH *dst = MEMALLOC_NEW(ConvexH)(src->vertices.count,src->edges.count,src->facets.count);

	memcpy(dst->vertices.element,src->vertices.element,sizeof(float3)*src->vertices.count);
	memcpy(dst->edges.element,src->edges.element,sizeof(HalfEdge)*src->edges.count);
	memcpy(dst->facets.element,src->facets.element,sizeof(Plane)*src->facets.count);
	return dst;
}


NxI32 PlaneTest(const Plane &p, const REAL3 &v) {
	REAL a  = dot(v,p.normal)+p.dist;
	NxI32   flag = (a>planetestepsilon)?OVER:((a<-planetestepsilon)?UNDER:COPLANAR);
	return flag;
}

NxI32 SplitTest(ConvexH &convex,const Plane &plane) {
	NxI32 flag=0;
	for(NxI32 i=0;i<convex.vertices.count;i++) {
		flag |= PlaneTest(plane,convex.vertices[i]);
	}
	return flag;
}

class VertFlag 
{
public:
	NxU8 planetest;
	NxU8 junk;
	NxU8 undermap;
	NxU8 overmap;
};
class EdgeFlag 
{
public:
	NxU8 planetest;
	NxU8 fixes;
	short undermap;
	short overmap;
};
class PlaneFlag 
{
public:
	NxU8 undermap;
	NxU8 overmap;
};
class Coplanar{
public:
	unsigned short ea;
	NxU8 v0;
	NxU8 v1;
};

NxI32 AssertIntact(ConvexH &convex) {
	NxI32 i;
	NxI32 estart=0;
	for(i=0;i<convex.edges.count;i++) {
		if(convex.edges[estart].p!= convex.edges[i].p) {
			estart=i;
		}
		NxI32 inext = i+1;
		if(inext>= convex.edges.count || convex.edges[inext].p != convex.edges[i].p) {
			inext = estart;
		}
		assert(convex.edges[inext].p == convex.edges[i].p);
		NxI32 nb = convex.edges[i].ea;
		assert(nb!=255);
		if(nb==255 || nb==-1) return 0;
		assert(nb!=-1);
		assert(i== convex.edges[nb].ea);
	}
	for(i=0;i<convex.edges.count;i++) {
		assert(COPLANAR==PlaneTest(convex.facets[convex.edges[i].p],convex.vertices[convex.edges[i].v]));
		if(COPLANAR!=PlaneTest(convex.facets[convex.edges[i].p],convex.vertices[convex.edges[i].v])) return 0;
		if(convex.edges[estart].p!= convex.edges[i].p) {
			estart=i;
		}
		NxI32 i1 = i+1;
		if(i1>= convex.edges.count || convex.edges[i1].p != convex.edges[i].p) {
			i1 = estart;
		}
		NxI32 i2 = i1+1;
		if(i2>= convex.edges.count || convex.edges[i2].p != convex.edges[i].p) {
			i2 = estart;
		}
		if(i==i2) continue; // i sliced tangent to an edge and created 2 meaningless edges
		REAL3 localnormal = TriNormal(convex.vertices[convex.edges[i ].v],
			                           convex.vertices[convex.edges[i1].v],
			                           convex.vertices[convex.edges[i2].v]);
		//assert(dot(localnormal,convex.facets[convex.edges[i].p].normal)>0);//Commented out on Stan Melax' advice
		if(dot(localnormal,convex.facets[convex.edges[i].p].normal)<=0)return 0;
	}
	return 1;
}

ConvexH *ConvexHCrop(ConvexH &convex,const Plane &slice)
{
	NxI32 i;
	NxI32 vertcountunder=0;
	NxI32 vertcountover =0;
	static Array<NxI32> vertscoplanar;  // existing vertex members of convex that are coplanar
	vertscoplanar.count=0;
	static Array<NxI32> edgesplit;  // existing edges that members of convex that cross the splitplane
	edgesplit.count=0;

	assert(convex.edges.count<480);

	EdgeFlag  edgeflag[512];
	VertFlag  vertflag[256];
	PlaneFlag planeflag[128];
	HalfEdge  tmpunderedges[512];
	Plane	  tmpunderplanes[128];
	Coplanar coplanaredges[512];
	NxI32 coplanaredges_num=0;

	Array<REAL3> createdverts;
	// do the side-of-plane tests
	for(i=0;i<convex.vertices.count;i++) {
		vertflag[i].planetest = (NxU8)PlaneTest(slice,convex.vertices[i]);
		if(vertflag[i].planetest == COPLANAR) {
			// ? vertscoplanar.Add(i);
			vertflag[i].undermap = (NxU8)vertcountunder++;
			vertflag[i].overmap  = (NxU8)vertcountover++;
		}
		else if(vertflag[i].planetest == UNDER)	{
			vertflag[i].undermap = (NxU8)vertcountunder++;
		}
		else {
			assert(vertflag[i].planetest == OVER);
			vertflag[i].overmap  = (NxU8)vertcountover++;
			vertflag[i].undermap = (NxU8)-1; // for debugging purposes
		}
	}

	NxI32 under_edge_count =0;
	NxI32 underplanescount=0;
	NxI32 e0=0;

	for(NxI32 currentplane=0; currentplane<convex.facets.count; currentplane++) {
		NxI32 estart =e0;
		NxI32 enextface=0;
		NxI32 planeside = 0;
		NxI32 e1 = e0+1;
		NxI32 vout=-1;
		NxI32 vin =-1;
		NxI32 coplanaredge = -1;
		do{

			if(e1 >= convex.edges.count || convex.edges[e1].p!=currentplane) {
				enextface = e1;
				e1=estart;
			}
			HalfEdge &edge0 = convex.edges[e0];
			HalfEdge &edge1 = convex.edges[e1];
			HalfEdge &edgea = convex.edges[edge0.ea];


			planeside |= vertflag[edge0.v].planetest;
			//if((vertflag[edge0.v].planetest & vertflag[edge1.v].planetest)  == COPLANAR) {
			//	assert(ecop==-1);
			//	ecop=e;
			//}


			if(vertflag[edge0.v].planetest == OVER && vertflag[edge1.v].planetest == OVER){
				// both endpoints over plane
				edgeflag[e0].undermap  = -1;
			}
			else if((vertflag[edge0.v].planetest | vertflag[edge1.v].planetest)  == UNDER) {
				// at least one endpoint under, the other coplanar or under
				
				edgeflag[e0].undermap = (short)under_edge_count;
				tmpunderedges[under_edge_count].v = (NxU8)vertflag[edge0.v].undermap;
				tmpunderedges[under_edge_count].p = (NxU8)underplanescount;
				if(edge0.ea < e0) {
					// connect the neighbors
					assert(edgeflag[edge0.ea].undermap !=-1);
					tmpunderedges[under_edge_count].ea = edgeflag[edge0.ea].undermap;
					tmpunderedges[edgeflag[edge0.ea].undermap].ea = (short)under_edge_count;
				}
				under_edge_count++;
			}
			else if((vertflag[edge0.v].planetest | vertflag[edge1.v].planetest)  == COPLANAR) {
				// both endpoints coplanar 
				// must check a 3rd point to see if UNDER
				NxI32 e2 = e1+1; 
				if(e2>=convex.edges.count || convex.edges[e2].p!=currentplane) {
					e2 = estart;
				}
				assert(convex.edges[e2].p==currentplane);
				HalfEdge &edge2 = convex.edges[e2];
				if(vertflag[edge2.v].planetest==UNDER) {
					
					edgeflag[e0].undermap = (short)under_edge_count;
					tmpunderedges[under_edge_count].v = (NxU8)vertflag[edge0.v].undermap;
					tmpunderedges[under_edge_count].p = (NxU8)underplanescount;
					tmpunderedges[under_edge_count].ea = -1;
					// make sure this edge is added to the "coplanar" list
					coplanaredge = under_edge_count;
					vout = vertflag[edge0.v].undermap;
					vin  = vertflag[edge1.v].undermap;
					under_edge_count++;
				}
				else {
					edgeflag[e0].undermap = -1;
				}
			}
			else if(vertflag[edge0.v].planetest == UNDER && vertflag[edge1.v].planetest == OVER) {
				// first is under 2nd is over 
				
				edgeflag[e0].undermap = (short) under_edge_count;
				tmpunderedges[under_edge_count].v = (NxU8)vertflag[edge0.v].undermap;
				tmpunderedges[under_edge_count].p = (NxU8)underplanescount;
				if(edge0.ea < e0) {
					assert(edgeflag[edge0.ea].undermap !=-1);
					// connect the neighbors
					tmpunderedges[under_edge_count].ea = edgeflag[edge0.ea].undermap;
					tmpunderedges[edgeflag[edge0.ea].undermap].ea = (short)under_edge_count;
					vout = tmpunderedges[edgeflag[edge0.ea].undermap].v;
				}
				else {
					Plane &p0 = convex.facets[edge0.p];
					Plane &pa = convex.facets[edgea.p];
					createdverts.Add(ThreePlaneIntersection(p0,pa,slice));
					//createdverts.Add(PlaneProject(slice,PlaneLineIntersection(slice,convex.vertices[edge0.v],convex.vertices[edgea.v])));
					//createdverts.Add(PlaneLineIntersection(slice,convex.vertices[edge0.v],convex.vertices[edgea.v]));
					vout = vertcountunder++;
				}
				under_edge_count++;
				/// hmmm something to think about: i might be able to output this edge regarless of 
				// wheter or not we know v-in yet.  ok i;ll try this now:
				tmpunderedges[under_edge_count].v = (NxU8)vout;
				tmpunderedges[under_edge_count].p = (NxU8)underplanescount;
				tmpunderedges[under_edge_count].ea = -1;
				coplanaredge = under_edge_count;
				under_edge_count++;

				if(vin!=-1) {
					// we previously processed an edge  where we came under
					// now we know about vout as well

					// ADD THIS EDGE TO THE LIST OF EDGES THAT NEED NEIGHBOR ON PARTITION PLANE!!
				}

			}
			else if(vertflag[edge0.v].planetest == COPLANAR && vertflag[edge1.v].planetest == OVER) {
				// first is coplanar 2nd is over 
				
				edgeflag[e0].undermap = -1;
				vout = vertflag[edge0.v].undermap;
				// I hate this but i have to make sure part of this face is UNDER before ouputting this vert
				NxI32 k=estart;
				assert(edge0.p == currentplane);
				while(!(planeside&UNDER) && k<convex.edges.count && convex.edges[k].p==edge0.p) {
					planeside |= vertflag[convex.edges[k].v].planetest;
					k++;
				}
				if(planeside&UNDER){
					tmpunderedges[under_edge_count].v = (NxU8)vout;
					tmpunderedges[under_edge_count].p = (NxU8)underplanescount;
					tmpunderedges[under_edge_count].ea = -1;
					coplanaredge = under_edge_count; // hmmm should make a note of the edge # for later on
					under_edge_count++;
					
				}
			}
			else if(vertflag[edge0.v].planetest == OVER && vertflag[edge1.v].planetest == UNDER) {
				// first is over next is under 
				// new vertex!!!
				if (vin!=-1) return NULL;
				if(e0<edge0.ea) {
					Plane &p0 = convex.facets[edge0.p];
					Plane &pa = convex.facets[edgea.p];
					createdverts.Add(ThreePlaneIntersection(p0,pa,slice));
					//createdverts.Add(PlaneLineIntersection(slice,convex.vertices[edge0.v],convex.vertices[edgea.v]));
					//createdverts.Add(PlaneProject(slice,PlaneLineIntersection(slice,convex.vertices[edge0.v],convex.vertices[edgea.v])));
					vin = vertcountunder++;
				}
				else {
					// find the new vertex that was created by edge[edge0.ea]
					NxI32 nea = edgeflag[edge0.ea].undermap;
					assert(tmpunderedges[nea].p==tmpunderedges[nea+1].p);
					vin = tmpunderedges[nea+1].v;
					assert(vin < vertcountunder);
				}
				if(vout!=-1) {
					// we previously processed an edge  where we went over
					// now we know vin too
					// ADD THIS EDGE TO THE LIST OF EDGES THAT NEED NEIGHBOR ON PARTITION PLANE!!
				}
				// output edge
				tmpunderedges[under_edge_count].v = (NxU8)vin;
				tmpunderedges[under_edge_count].p = (NxU8)underplanescount;
				edgeflag[e0].undermap = (short)under_edge_count;
				if(e0>edge0.ea) {
					assert(edgeflag[edge0.ea].undermap !=-1);
					// connect the neighbors
					tmpunderedges[under_edge_count].ea = edgeflag[edge0.ea].undermap;
					tmpunderedges[edgeflag[edge0.ea].undermap].ea = (short)under_edge_count;
				}
				assert(edgeflag[e0].undermap == under_edge_count);
				under_edge_count++;
			}
			else if(vertflag[edge0.v].planetest == OVER && vertflag[edge1.v].planetest == COPLANAR) {
				// first is over next is coplanar 
				
				edgeflag[e0].undermap = -1;
				vin = vertflag[edge1.v].undermap;
				if (vin==-1) return NULL;
				if(vout!=-1) {
					// we previously processed an edge  where we came under
					// now we know both endpoints
					// ADD THIS EDGE TO THE LIST OF EDGES THAT NEED NEIGHBOR ON PARTITION PLANE!!
				}

			}
			else {
				assert(0);
			}
			

			e0=e1;
			e1++; // do the modulo at the beginning of the loop

		} while(e0!=estart) ;
		e0 = enextface;
		if(planeside&UNDER) {
			planeflag[currentplane].undermap = (NxU8)underplanescount;
			tmpunderplanes[underplanescount] = convex.facets[currentplane];
			underplanescount++;
		}
		else {
			planeflag[currentplane].undermap = 0;
		}
		if(vout>=0 && (planeside&UNDER)) {
			assert(vin>=0);
			assert(coplanaredge>=0);
			assert(coplanaredge!=511);
			coplanaredges[coplanaredges_num].ea = (short)coplanaredge;
			coplanaredges[coplanaredges_num].v0 = (NxU8)vin;
			coplanaredges[coplanaredges_num].v1 = (NxU8)vout;
			coplanaredges_num++;
		}
	}

	// add the new plane to the mix:
	if(coplanaredges_num>0) {
		tmpunderplanes[underplanescount++]=slice;
	}
	for(i=0;i<coplanaredges_num-1;i++) {
		if(coplanaredges[i].v1 != coplanaredges[i+1].v0) {
			NxI32 j = 0;
			for(j=i+2;j<coplanaredges_num;j++) {
				if(coplanaredges[i].v1 == coplanaredges[j].v0) {
					Coplanar tmp = coplanaredges[i+1];
					coplanaredges[i+1] = coplanaredges[j];
					coplanaredges[j] = tmp;
					break;
				}
			}
			if(j>=coplanaredges_num)
			{
				// assert(j<coplanaredges_num);
				return NULL;
			}
		}
	}
	ConvexH *punder = MEMALLOC_NEW(ConvexH)(vertcountunder,under_edge_count+coplanaredges_num,underplanescount);

	ConvexH &under = *punder;
	NxI32 k=0;
	for(i=0;i<convex.vertices.count;i++) {
		if(vertflag[i].planetest != OVER){
			under.vertices[k++] = convex.vertices[i];
		}
	}
	i=0;
	while(k<vertcountunder) {
		under.vertices[k++] = createdverts[i++];
	}
	assert(i==createdverts.count);

	for(i=0;i<coplanaredges_num;i++) {
		under.edges[under_edge_count+i].p  = (NxU8)(underplanescount-1);
		under.edges[under_edge_count+i].ea = coplanaredges[i].ea;
		tmpunderedges[coplanaredges[i].ea].ea = (short)(under_edge_count+i);
		under.edges[under_edge_count+i].v  = coplanaredges[i].v0;
	}

	memcpy(under.edges.element,tmpunderedges,sizeof(HalfEdge)*under_edge_count);
	memcpy(under.facets.element,tmpunderplanes,sizeof(Plane)*underplanescount);
	return punder;
}


NxF32 minadjangle = 3.0f;  // in degrees  - result wont have two adjacent facets within this angle of each other.
static NxI32 candidateplane(Plane *planes,NxI32 planes_count,ConvexH *convex,NxF32 epsilon)
{
	NxI32 p =-1;
	REAL md=0;
	NxI32 i,j;
	NxF32 maxdot_minang = cosf(DEG2RAD*minadjangle);
	for(i=0;i<planes_count;i++)
	{
		NxF32 d=0;
		NxF32 dmax=0;
		NxF32 dmin=0;
		for(j=0;j<convex->vertices.count;j++)
		{
			dmax = Max(dmax,dot(convex->vertices[j],planes[i].normal)+planes[i].dist);
			dmin = Min(dmin,dot(convex->vertices[j],planes[i].normal)+planes[i].dist);
		}
		NxF32 dr = dmax-dmin;
		if(dr<planetestepsilon) dr=1.0f; // shouldn't happen.
		d = dmax /dr;
		if(d<=md) continue;
		for(j=0;j<convex->facets.count;j++)
		{
			if(planes[i]==convex->facets[j]) 
			{
				d=0;continue;
			}
			if(dot(planes[i].normal,convex->facets[j].normal)>maxdot_minang)
			{
				for(NxI32 k=0;k<convex->edges.count;k++)
				{
					if(convex->edges[k].p!=j) continue;
					if(dot(convex->vertices[convex->edges[k].v],planes[i].normal)+planes[i].dist<0)
					{
						d=0; // so this plane wont get selected.
						break;
					}
				}
			}
		}
		if(d>md)
		{
			p=i;
			md=d;
		}
	}
	return (md>epsilon)?p:-1;
}



template<class T>
inline NxI32 maxdir(const T *p,NxI32 count,const T &dir)
{
	assert(count);
	NxI32 m=0;
	for(NxI32 i=1;i<count;i++)
	{
		if(dot(p[i],dir)>dot(p[m],dir)) m=i;
	}
	return m;
}


template<class T>
NxI32 maxdirfiltered(const T *p,NxI32 count,const T &dir,Array<NxI32> &allow)
{
	assert(count);
	NxI32 m=-1;
	for(NxI32 i=0;i<count;i++) if(allow[i])
	{
		if(m==-1 || dot(p[i],dir)>dot(p[m],dir)) m=i;
	}
	assert(m!=-1);
	return m;
} 

float3 orth(const float3 &v)
{
	float3 a=cross(v,float3(0,0,1));
	float3 b=cross(v,float3(0,1,0));
	return normalize((magnitude(a)>magnitude(b))?a:b);
}


template<class T>
NxI32 maxdirsterid(const T *p,NxI32 count,const T &dir,Array<NxI32> &allow)
{
	NxI32 m=-1;
	while(m==-1)
	{
		m = maxdirfiltered(p,count,dir,allow);
		if(allow[m]==3) return m;
		T u = orth(dir);
		T v = cross(u,dir);
		NxI32 ma=-1;
		for(NxF32 x = 0.0f ; x<= 360.0f ; x+= 45.0f)
		{
			NxF32 s = sinf(DEG2RAD*(x));
			NxF32 c = cosf(DEG2RAD*(x));
			NxI32 mb = maxdirfiltered(p,count,dir+(u*s+v*c)*0.025f,allow);
			if(ma==m && mb==m)
			{
				allow[m]=3;
				return m;
			}
			if(ma!=-1 && ma!=mb)  // Yuck - this is really ugly
			{
				NxI32 mc = ma;
				for(NxF32 xx = x-40.0f ; xx <= x ; xx+= 5.0f)
				{
					NxF32 s = sinf(DEG2RAD*(xx));
					NxF32 c = cosf(DEG2RAD*(xx));
					NxI32 md = maxdirfiltered(p,count,dir+(u*s+v*c)*0.025f,allow);
					if(mc==m && md==m)
					{
						allow[m]=3;
						return m;
					}
					mc=md;
				}
			}
			ma=mb;
		}
		allow[m]=0;
		m=-1;
	}
	assert(0);
	return m;
} 




NxI32 operator ==(const int3 &a,const int3 &b) 
{
	for(NxI32 i=0;i<3;i++) 
	{
		if(a[i]!=b[i]) return 0;
	}
	return 1;
}

int3 roll3(int3 a) 
{
	NxI32 tmp=a[0];
	a[0]=a[1];
	a[1]=a[2];
	a[2]=tmp;
	return a;
}
NxI32 isa(const int3 &a,const int3 &b) 
{
	return ( a==b || roll3(a)==b || a==roll3(b) );
}
NxI32 b2b(const int3 &a,const int3 &b) 
{
	return isa(a,int3(b[2],b[1],b[0]));
}
NxI32 above(float3* vertices,const int3& t, const float3 &p, NxF32 epsilon) 
{
	float3 n=TriNormal(vertices[t[0]],vertices[t[1]],vertices[t[2]]);
	return (dot(n,p-vertices[t[0]]) > epsilon); // EPSILON???
}
NxI32 hasedge(const int3 &t, NxI32 a,NxI32 b)
{
	for(NxI32 i=0;i<3;i++)
	{
		NxI32 i1= (i+1)%3;
		if(t[i]==a && t[i1]==b) return 1;
	}
	return 0;
}
NxI32 hasvert(const int3 &t, NxI32 v)
{
	return (t[0]==v || t[1]==v || t[2]==v) ;
}
NxI32 shareedge(const int3 &a,const int3 &b)
{
	NxI32 i;
	for(i=0;i<3;i++)
	{
		NxI32 i1= (i+1)%3;
		if(hasedge(a,b[i1],b[i])) return 1;
	}
	return 0;
}

class Tri;

static Array<Tri*> tris; // djs: For heaven's sake!!!!

class Tri : public int3
{
public:
	int3 n;
	NxI32 id;
	NxI32 vmax;
	NxF32 rise;
	Tri(NxI32 a,NxI32 b,NxI32 c):int3(a,b,c),n(-1,-1,-1)
	{
		id = tris.count;
		tris.Add(this);
		vmax=-1;
		rise = 0.0f;
	}
	~Tri()
	{
		assert(tris[id]==this);
		tris[id]=NULL;
	}
	NxI32 &neib(NxI32 a,NxI32 b);
};


NxI32 &Tri::neib(NxI32 a,NxI32 b)
{
	static NxI32 er=-1;
	NxI32 i;
	for(i=0;i<3;i++) 
	{
		NxI32 i1=(i+1)%3;
		NxI32 i2=(i+2)%3;
		if((*this)[i]==a && (*this)[i1]==b) return n[i2];
		if((*this)[i]==b && (*this)[i1]==a) return n[i2];
	}
	assert(0);
	return er;
}
void b2bfix(Tri* s,Tri*t)
{
	NxI32 i;
	for(i=0;i<3;i++) 
	{
		NxI32 i1=(i+1)%3;
		NxI32 i2=(i+2)%3;
		NxI32 a = (*s)[i1];
		NxI32 b = (*s)[i2];
		assert(tris[s->neib(a,b)]->neib(b,a) == s->id);
		assert(tris[t->neib(a,b)]->neib(b,a) == t->id);
		tris[s->neib(a,b)]->neib(b,a) = t->neib(b,a);
		tris[t->neib(b,a)]->neib(a,b) = s->neib(a,b);
	}
}

void removeb2b(Tri* s,Tri*t)
{
	b2bfix(s,t);
	delete s;
	delete t;
}

void extrude(Tri *t0,NxI32 v)
{
	int3 t= *t0;
	NxI32 n = tris.count;
	Tri* ta = MEMALLOC_NEW(Tri)(v,t[1],t[2]);
	ta->n = int3(t0->n[0],n+1,n+2);
	tris[t0->n[0]]->neib(t[1],t[2]) = n+0;
	Tri* tb = MEMALLOC_NEW(Tri)(v,t[2],t[0]);
	tb->n = int3(t0->n[1],n+2,n+0);
	tris[t0->n[1]]->neib(t[2],t[0]) = n+1;
	Tri* tc = MEMALLOC_NEW(Tri)(v,t[0],t[1]);
	tc->n = int3(t0->n[2],n+0,n+1);
	tris[t0->n[2]]->neib(t[0],t[1]) = n+2;
	if(hasvert(*tris[ta->n[0]],v)) removeb2b(ta,tris[ta->n[0]]);
	if(hasvert(*tris[tb->n[0]],v)) removeb2b(tb,tris[tb->n[0]]);
	if(hasvert(*tris[tc->n[0]],v)) removeb2b(tc,tris[tc->n[0]]);
	delete t0;

}

Tri *extrudable(NxF32 epsilon)
{
	NxI32 i;
	Tri *t=NULL;
	for(i=0;i<tris.count;i++)
	{
		if(!t || (tris[i] && t->rise<tris[i]->rise))
		{
			t = tris[i];
		}
	}
	return (t->rise >epsilon)?t:NULL ;
}

class int4
{
public:
	NxI32 x,y,z,w;
	int4(){};
	int4(NxI32 _x,NxI32 _y, NxI32 _z,NxI32 _w){x=_x;y=_y;z=_z;w=_w;}
	const NxI32& operator[](NxI32 i) const {return (&x)[i];}
	NxI32& operator[](NxI32 i) {return (&x)[i];}
};



bool hasVolume(float3 *verts, NxI32 p0, NxI32 p1, NxI32 p2, NxI32 p3)
{
	float3 result3 = cross(verts[p1]-verts[p0], verts[p2]-verts[p0]);
	if (magnitude(result3) < VOLUME_EPSILON && magnitude(result3) > -VOLUME_EPSILON) // Almost collinear or otherwise very close to each other
		return false;
	NxF32 result = dot(normalize(result3), verts[p3]-verts[p0]);
	return (result > VOLUME_EPSILON || result < -VOLUME_EPSILON); // Returns true iff volume is significantly non-zero
}

int4 FindSimplex(float3 *verts,NxI32 verts_count,Array<NxI32> &allow)
{
	float3 basis[3];
	basis[0] = float3( 0.01f, 0.02f, 1.0f );      
	NxI32 p0 = maxdirsterid(verts,verts_count, basis[0],allow);
	NxI32	p1 = maxdirsterid(verts,verts_count,-basis[0],allow);
	basis[0] = verts[p0]-verts[p1];
	if(p0==p1 || basis[0]==float3(0,0,0)) 
		return int4(-1,-1,-1,-1);
	basis[1] = cross(float3(     1, 0.02f, 0),basis[0]);
	basis[2] = cross(float3(-0.02f,     1, 0),basis[0]);
	basis[1] = normalize( (magnitude(basis[1])>magnitude(basis[2])) ? basis[1]:basis[2]);
	NxI32 p2 = maxdirsterid(verts,verts_count,basis[1],allow);
	if(p2 == p0 || p2 == p1)
	{
		p2 = maxdirsterid(verts,verts_count,-basis[1],allow);
	}
	if(p2 == p0 || p2 == p1) 
		return int4(-1,-1,-1,-1);
	basis[1] = verts[p2] - verts[p0];
	basis[2] = normalize(cross(basis[1],basis[0]));
	NxI32 p3 = maxdirsterid(verts,verts_count,basis[2],allow);
	if(p3==p0||p3==p1||p3==p2||!hasVolume(verts, p0, p1, p2, p3)) p3 = maxdirsterid(verts,verts_count,-basis[2],allow);
	if(p3==p0||p3==p1||p3==p2) 
		return int4(-1,-1,-1,-1);
	assert(!(p0==p1||p0==p2||p0==p3||p1==p2||p1==p3||p2==p3));
	if(dot(verts[p3]-verts[p0],cross(verts[p1]-verts[p0],verts[p2]-verts[p0])) <0) {Swap(p2,p3);}
	return int4(p0,p1,p2,p3);
}
#pragma warning(push)
#pragma warning(disable:4706)
NxI32 calchullgen(float3 *verts,NxI32 verts_count, NxI32 vlimit) 
{
	if(verts_count <4) return 0;
	if(vlimit==0) vlimit=1000000000;
	NxI32 j;
	float3 bmin(*verts),bmax(*verts);
	Array<NxI32> isextreme(verts_count);
	Array<NxI32> allow(verts_count);
	for(j=0;j<verts_count;j++) 
	{
		allow.Add(1);
		isextreme.Add(0);
		bmin = VectorMin(bmin,verts[j]);
		bmax = VectorMax(bmax,verts[j]);
	}
	NxF32 epsilon = magnitude(bmax-bmin) * 0.001f;


	int4 p = FindSimplex(verts,verts_count,allow);
	if(p.x==-1) return 0; // simplex failed



	float3 center = (verts[p[0]]+verts[p[1]]+verts[p[2]]+verts[p[3]]) /4.0f;  // a valid interior point
	Tri *t0 = MEMALLOC_NEW(Tri)(p[2],p[3],p[1]); t0->n=int3(2,3,1);
	Tri *t1 = MEMALLOC_NEW(Tri)(p[3],p[2],p[0]); t1->n=int3(3,2,0);
	Tri *t2 = MEMALLOC_NEW(Tri)(p[0],p[1],p[3]); t2->n=int3(0,1,3);
	Tri *t3 = MEMALLOC_NEW(Tri)(p[1],p[0],p[2]); t3->n=int3(1,0,2);
	isextreme[p[0]]=isextreme[p[1]]=isextreme[p[2]]=isextreme[p[3]]=1;

	for(j=0;j<tris.count;j++)
	{
		Tri *t=tris[j];
		assert(t);
		assert(t->vmax<0);
		float3 n=TriNormal(verts[(*t)[0]],verts[(*t)[1]],verts[(*t)[2]]);
		t->vmax = maxdirsterid(verts,verts_count,n,allow);
		t->rise = dot(n,verts[t->vmax]-verts[(*t)[0]]);
	}
	Tri *te;
	vlimit-=4;
	while(vlimit >0 && (te=extrudable(epsilon)))
	{
		int3 ti=*te;
		NxI32 v=te->vmax;
		assert(!isextreme[v]);  // wtf we've already done this vertex
		isextreme[v]=1;
		//if(v==p0 || v==p1 || v==p2 || v==p3) continue; // done these already
		j=tris.count;
		while(j--) {
			if(!tris[j]) continue;
			int3 t=*tris[j];
			if(above(verts,t,verts[v],0.01f*epsilon))
			{
				extrude(tris[j],v);
			}
		}
		// now check for those degenerate cases where we have a flipped triangle or a really skinny triangle
		j=tris.count;
		while(j--)
		{
			if(!tris[j]) continue;
			if(!hasvert(*tris[j],v)) break;
			int3 nt=*tris[j];
			if(above(verts,nt,center,0.01f*epsilon)  || magnitude(cross(verts[nt[1]]-verts[nt[0]],verts[nt[2]]-verts[nt[1]]))< epsilon*epsilon*0.1f )
			{
				Tri *nb = tris[tris[j]->n[0]];
				assert(nb);assert(!hasvert(*nb,v));assert(nb->id<j);
				extrude(nb,v);
				j=tris.count;
			}
		}
		j=tris.count;
		while(j--)
		{
			Tri *t=tris[j];
			if(!t) continue;
			if(t->vmax>=0) break;
			float3 n=TriNormal(verts[(*t)[0]],verts[(*t)[1]],verts[(*t)[2]]);
			t->vmax = maxdirsterid(verts,verts_count,n,allow);
			if(isextreme[t->vmax]) 
			{
				t->vmax=-1; // already done that vertex - algorithm needs to be able to terminate.
			}
			else
			{
				t->rise = dot(n,verts[t->vmax]-verts[(*t)[0]]);
			}
		}
		vlimit --;
	}
	return 1;
}
#pragma warning(pop)

NxI32 calchull(float3 *verts,NxI32 verts_count, NxI32 *&tris_out, NxI32 &tris_count,NxI32 vlimit) 
{
	NxI32 rc=calchullgen(verts,verts_count,  vlimit) ;
	if(!rc) return 0;
	Array<NxI32> ts;
	for(NxI32 i=0;i<tris.count;i++)if(tris[i])
	{
		for(NxI32 j=0;j<3;j++)ts.Add((*tris[i])[j]);
		delete tris[i];
	}
	tris_count = ts.count/3;
	tris_out   = ts.element;
	ts.element=NULL; ts.count=ts.array_size=0;
	// please reset here, otherwise, we get a nice virtual function call (R6025) error with NxCooking library
	tris.SetSize( 0 );
	return 1;
}

static NxF32 area2(const float3 &v0,const float3 &v1,const float3 &v2)
{
	float3 cp = cross(v0-v1,v2-v0);
	return dot(cp,cp);
}
NxI32 calchullpbev(float3 *verts,NxI32 verts_count,NxI32 vlimit, Array<Plane> &planes,NxF32 bevangle) 
{
	NxI32 i,j;
	Array<Plane> bplanes;
	planes.count=0;
	NxI32 rc = calchullgen(verts,verts_count,vlimit);
	if(!rc) return 0;
	extern NxF32 minadjangle; // default is 3.0f;  // in degrees  - result wont have two adjacent facets within this angle of each other.
	NxF32 maxdot_minang = cosf(DEG2RAD*minadjangle);
	for(i=0;i<tris.count;i++)if(tris[i])
	{
		Plane p;
		Tri *t = tris[i];
		p.normal = TriNormal(verts[(*t)[0]],verts[(*t)[1]],verts[(*t)[2]]);
		p.dist   = -dot(p.normal, verts[(*t)[0]]);
		for(j=0;j<3;j++)
		{
			if(t->n[j]<t->id) continue;
			Tri *s = tris[t->n[j]];
			REAL3 snormal = TriNormal(verts[(*s)[0]],verts[(*s)[1]],verts[(*s)[2]]);
			if(dot(snormal,p.normal)>=cos(bevangle*DEG2RAD)) continue;
			REAL3 e = verts[(*t)[(j+2)%3]] - verts[(*t)[(j+1)%3]];
			REAL3 n = (e!=REAL3(0,0,0))? cross(snormal,e)+cross(e,p.normal) : snormal+p.normal;
			assert(n!=REAL3(0,0,0));
			if(n==REAL3(0,0,0)) return 0;  
			n=normalize(n);
			bplanes.Add(Plane(n,-dot(n,verts[maxdir(verts,verts_count,n)])));
		}
	}
	for(i=0;i<tris.count;i++)if(tris[i])for(j=i+1;j<tris.count;j++)if(tris[i] && tris[j])
	{
		Tri *ti = tris[i];
		Tri *tj = tris[j];
		REAL3 ni = TriNormal(verts[(*ti)[0]],verts[(*ti)[1]],verts[(*ti)[2]]);
		REAL3 nj = TriNormal(verts[(*tj)[0]],verts[(*tj)[1]],verts[(*tj)[2]]);
		if(dot(ni,nj)>maxdot_minang)
		{
			// somebody has to die, keep the biggest triangle
			if( area2(verts[(*ti)[0]],verts[(*ti)[1]],verts[(*ti)[2]]) < area2(verts[(*tj)[0]],verts[(*tj)[1]],verts[(*tj)[2]]))
			{
				delete tris[i];
			}
			else
			{
				delete tris[j];
			}
		}
	}
	for(i=0;i<tris.count;i++)if(tris[i])
	{
		Plane p;
		Tri *t = tris[i];
		p.normal = TriNormal(verts[(*t)[0]],verts[(*t)[1]],verts[(*t)[2]]);
		p.dist   = -dot(p.normal, verts[(*t)[0]]);
		planes.Add(p);
	}
	for(i=0;i<bplanes.count;i++)
	{
		for(j=0;j<planes.count;j++)
		{
			if(dot(bplanes[i].normal,planes[j].normal)>maxdot_minang) break;
		}
		if(j==planes.count)
		{
			planes.Add(bplanes[i]);
		}
	}
	for(i=0;i<tris.count;i++)if(tris[i])
	{
		delete tris[i];
	}
	tris.count = 0; //bad place to do the tris.SetSize(0) fix, this line is executed many times, and will result in a whole lot of allocations if the array is totally cleared here
	return 1;
}

ConvexH *test_cube()
{
	ConvexH *convex = MEMALLOC_NEW(ConvexH)(8,24,6);
	convex->vertices[0] = REAL3(0,0,0);
	convex->vertices[1] = REAL3(0,0,1);
	convex->vertices[2] = REAL3(0,1,0);
	convex->vertices[3] = REAL3(0,1,1);
	convex->vertices[4] = REAL3(1,0,0);
	convex->vertices[5] = REAL3(1,0,1);
	convex->vertices[6] = REAL3(1,1,0);
	convex->vertices[7] = REAL3(1,1,1);

	convex->facets[0] = Plane(REAL3(-1,0,0),0);
	convex->facets[1] = Plane(REAL3(1,0,0),-1);
	convex->facets[2] = Plane(REAL3(0,-1,0),0);
	convex->facets[3] = Plane(REAL3(0,1,0),-1);
	convex->facets[4] = Plane(REAL3(0,0,-1),0);
	convex->facets[5] = Plane(REAL3(0,0,1),-1);

	convex->edges[0 ] = HalfEdge(11,0,0);
	convex->edges[1 ] = HalfEdge(23,1,0);
	convex->edges[2 ] = HalfEdge(15,3,0);
	convex->edges[3 ] = HalfEdge(16,2,0);

	convex->edges[4 ] = HalfEdge(13,6,1);
	convex->edges[5 ] = HalfEdge(21,7,1);
	convex->edges[6 ] = HalfEdge( 9,5,1);
	convex->edges[7 ] = HalfEdge(18,4,1);

	convex->edges[8 ] = HalfEdge(19,0,2);
	convex->edges[9 ] = HalfEdge( 6,4,2);
	convex->edges[10] = HalfEdge(20,5,2);
	convex->edges[11] = HalfEdge( 0,1,2);

	convex->edges[12] = HalfEdge(22,3,3);
	convex->edges[13] = HalfEdge( 4,7,3);
	convex->edges[14] = HalfEdge(17,6,3);
	convex->edges[15] = HalfEdge( 2,2,3);

	convex->edges[16] = HalfEdge( 3,0,4);
	convex->edges[17] = HalfEdge(14,2,4);
	convex->edges[18] = HalfEdge( 7,6,4);
	convex->edges[19] = HalfEdge( 8,4,4);

	convex->edges[20] = HalfEdge(10,1,5);
	convex->edges[21] = HalfEdge( 5,5,5);
	convex->edges[22] = HalfEdge(12,7,5);
	convex->edges[23] = HalfEdge( 1,3,5);


	return convex;
}

ConvexH *ConvexHMakeCube(const REAL3 &bmin, const REAL3 &bmax)
{
	ConvexH *convex = test_cube();
	convex->vertices[0] = REAL3(bmin.x,bmin.y,bmin.z);
	convex->vertices[1] = REAL3(bmin.x,bmin.y,bmax.z);
	convex->vertices[2] = REAL3(bmin.x,bmax.y,bmin.z);
	convex->vertices[3] = REAL3(bmin.x,bmax.y,bmax.z);
	convex->vertices[4] = REAL3(bmax.x,bmin.y,bmin.z);
	convex->vertices[5] = REAL3(bmax.x,bmin.y,bmax.z);
	convex->vertices[6] = REAL3(bmax.x,bmax.y,bmin.z);
	convex->vertices[7] = REAL3(bmax.x,bmax.y,bmax.z);

	convex->facets[0] = Plane(REAL3(-1,0,0), bmin.x);
	convex->facets[1] = Plane(REAL3(1,0,0), -bmax.x);
	convex->facets[2] = Plane(REAL3(0,-1,0), bmin.y);
	convex->facets[3] = Plane(REAL3(0,1,0), -bmax.y);
	convex->facets[4] = Plane(REAL3(0,0,-1), bmin.z);
	convex->facets[5] = Plane(REAL3(0,0,1), -bmax.z);
	return convex;
}


static NxI32 overhull(Plane *planes,NxI32 planes_count,float3 *verts, NxI32 verts_count,NxI32 maxplanes,
			 float3 *&verts_out, NxI32 &verts_count_out,  NxI32 *&faces_out, NxI32 &faces_count_out ,NxF32 inflate)
{
	NxI32 i,j;
	if(verts_count <4) return NULL;
	maxplanes = Min(maxplanes,planes_count);
	float3 bmin(verts[0]),bmax(verts[0]);
	for(i=0;i<verts_count;i++)
	{
		bmin = VectorMin(bmin,verts[i]);
		bmax = VectorMax(bmax,verts[i]);
	}
	NxF32 diameter = magnitude(bmax-bmin);
//	inflate *=diameter;   // RELATIVE INFLATION
	bmin -= float3(inflate*2.5f,inflate*2.5f,inflate*2.5f);
	bmax += float3(inflate*2.5f,inflate*2.5f,inflate*2.5f);
	// 2 is from the formula:
	// D = d*|n1+n2|/(1-n1 dot n2), where d is "inflate" and
	// n1 and n2 are the normals of two planes at bevelAngle to each other
	// for 120 degrees, D is 2d

	//bmin -= float3(inflate,inflate,inflate);
	//bmax += float3(inflate,inflate,inflate);
	for(i=0;i<planes_count;i++)
	{
		planes[i].dist -= inflate;
	}
	float3 emin = bmin; // VectorMin(bmin,float3(0,0,0));
	float3 emax = bmax; // VectorMax(bmax,float3(0,0,0));
	NxF32 epsilon  = 0.01f; // size of object is taken into account within candidate plane function.  Used to multiply here by magnitude(emax-emin)
	planetestepsilon = magnitude(emax-emin) * PAPERWIDTH;
	// todo: add bounding cube planes to force bevel. or try instead not adding the diameter expansion ??? must think.
	// ConvexH *convex = ConvexHMakeCube(bmin - float3(diameter,diameter,diameter),bmax+float3(diameter,diameter,diameter));
	NxF32 maxdot_minang = cosf(DEG2RAD*minadjangle);
	for(j=0;j<6;j++)
	{
		float3 n(0,0,0);
		n[j/2] = (j%2)? 1.0f : -1.0f;
		for(i=0;i<planes_count;i++)
		{
			if(dot(n,planes[i].normal)> maxdot_minang)
			{
				(*((j%2)?&bmax:&bmin)) += n * (diameter*0.5f);
				break;
			}
		}
	}
	ConvexH *c = ConvexHMakeCube(REAL3(bmin),REAL3(bmax));
	NxI32 k;
	while(maxplanes-- && (k=candidateplane(planes,planes_count,c,epsilon))>=0)
	{
		ConvexH *tmp = c;
		c = ConvexHCrop(*tmp,planes[k]);
		if(c==NULL) {c=tmp; break;} // might want to debug this case better!!!
		if(!AssertIntact(*c)) {c=tmp; break;} // might want to debug this case better too!!!
		delete tmp;
	}

	assert(AssertIntact(*c));
	//return c;
	faces_out = (NxI32*)MEMALLOC_MALLOC(sizeof(NxI32)*(1+c->facets.count+c->edges.count));     // new NxI32[1+c->facets.count+c->edges.count];
	faces_count_out=0;
	i=0;
	faces_out[faces_count_out++]=-1;
	k=0;
	while(i<c->edges.count)
	{
		j=1;
		while(j+i<c->edges.count && c->edges[i].p==c->edges[i+j].p) { j++; }
		faces_out[faces_count_out++]=j;
		while(j--)
		{
			faces_out[faces_count_out++] = c->edges[i].v;
			i++;
		}
		k++;
	}
	faces_out[0]=k; // number of faces.
	assert(k==c->facets.count);
	assert(faces_count_out == 1+c->facets.count+c->edges.count);
	verts_out = c->vertices.element; // new float3[c->vertices.count];
	verts_count_out = c->vertices.count;
	for(i=0;i<c->vertices.count;i++)
	{
		verts_out[i] = float3(c->vertices[i]);
	}
	c->vertices.count=c->vertices.array_size=0;	c->vertices.element=NULL;
	delete c;
	return 1;
}

static NxI32 overhullv(float3 *verts, NxI32 verts_count,NxI32 maxplanes,
			 float3 *&verts_out, NxI32 &verts_count_out,  NxI32 *&faces_out, NxI32 &faces_count_out ,NxF32 inflate,NxF32 bevangle,NxI32 vlimit)
{
	if(!verts_count) return 0;
	extern NxI32 calchullpbev(float3 *verts,NxI32 verts_count,NxI32 vlimit, Array<Plane> &planes,NxF32 bevangle) ;
	Array<Plane> planes;
	NxI32 rc=calchullpbev(verts,verts_count,vlimit,planes,bevangle) ;
	if(!rc) return 0;
	return overhull(planes.element,planes.count,verts,verts_count,maxplanes,verts_out,verts_count_out,faces_out,faces_count_out,inflate);
}


//*****************************************************
//*****************************************************


bool ComputeHull(NxU32 vcount,const NxF32 *vertices,PHullResult &result,NxU32 vlimit,NxF32 inflate)
{

	NxI32 index_count;
	NxI32 *faces;
	float3 *verts_out;
	NxI32     verts_count_out;

	if(inflate==0.0f)
	{
		NxI32  *tris_out;
		NxI32    tris_count;
		NxI32 ret = calchull( (float3 *) vertices, (NxI32) vcount, tris_out, tris_count, vlimit );
		if(!ret) return false;
		result.mIndexCount = (NxU32) (tris_count*3);
		result.mFaceCount  = (NxU32) tris_count;
		result.mVertices   = (NxF32*) vertices;
		result.mVcount     = (NxU32) vcount;
		result.mIndices    = (NxU32 *) tris_out;
		return true;
	}

	NxI32 ret = overhullv((float3*)vertices,vcount,35,verts_out,verts_count_out,faces,index_count,inflate,120.0f,vlimit);
	if(!ret) {
		tris.SetSize(0); //have to set the size to 0 in order to protect from a "pure virtual function call" problem
		return false;
	}

	Array<int3> tris;
	NxI32 n=faces[0];
	NxI32 k=1;
	for(NxI32 i=0;i<n;i++)
	{
		NxI32 pn = faces[k++];
		for(NxI32 j=2;j<pn;j++) tris.Add(int3(faces[k],faces[k+j-1],faces[k+j]));
		k+=pn;
	}
	assert(tris.count == index_count-1-(n*3));
	MEMALLOC_FREE(faces);	// PT: I added that. Is it ok ?

	result.mIndexCount = (NxU32) (tris.count*3);
	result.mFaceCount  = (NxU32) tris.count;
	result.mVertices   = (NxF32*) verts_out;
	result.mVcount     = (NxU32) verts_count_out;
	result.mIndices    = (NxU32 *) tris.element;
	tris.element=NULL; tris.count = tris.array_size=0;
	CONVEX_DECOMPOSITION::tris.SetSize(0); //have to set the size to 0 in order to protect from a "pure virtual function call" problem

	return true;
}


void ReleaseHull(PHullResult &result)
{
  MEMALLOC_FREE(result.mIndices);	// PT: I added that. Is it ok ?
  MEMALLOC_FREE(result.mVertices);	// PT: I added that. Is it ok ?
	result.mVcount = 0;
	result.mIndexCount = 0;
	result.mIndices = 0;
	result.mVertices = 0;
	result.mIndices  = 0;
}



//****** HULLLIB source code


HullError HullLibrary::CreateConvexHull(const HullDesc       &desc,           // describes the input request
																				HullResult           &result)         // contains the resulst
{
	HullError ret = QE_FAIL;


	PHullResult hr;

	NxU32 vcount = desc.mVcount;
	if ( vcount < 8 ) vcount = 8;

	NxF32 *vsource  = (NxF32 *) MEMALLOC_MALLOC( sizeof(NxF32)*vcount*3 );


	NxF32 scale[3];

	NxU32 ovcount;

	bool ok = CleanupVertices(desc.mVcount,desc.mVertices, desc.mVertexStride, ovcount, vsource, desc.mNormalEpsilon, scale ); // normalize point cloud, remove duplicates!

	if ( ok )
	{


		{
			for (NxU32 i=0; i<ovcount; i++)
			{
				NxF32 *v = &vsource[i*3];
				v[0]*=scale[0];
				v[1]*=scale[1];
				v[2]*=scale[2];
			}
		}

		NxF32 skinwidth = 0;
		if ( desc.HasHullFlag(QF_SKIN_WIDTH) ) 
			skinwidth = desc.mSkinWidth;

		ok = ComputeHull(ovcount,vsource,hr,desc.mMaxVertices,skinwidth);

		if ( ok )
		{

			// re-index triangle mesh so it refers to only used vertices, rebuild a new vertex table.
			NxF32 *vscratch = (NxF32 *) MEMALLOC_MALLOC( sizeof(NxF32)*hr.mVcount*3);
			BringOutYourDead(hr.mVertices,hr.mVcount, vscratch, ovcount, hr.mIndices, hr.mIndexCount );

			ret = QE_OK;

			if ( desc.HasHullFlag(QF_TRIANGLES) ) // if he wants the results as triangle!
			{
				result.mPolygons          = false;
				result.mNumOutputVertices = ovcount;
				result.mOutputVertices    = (NxF32 *)MEMALLOC_MALLOC( sizeof(NxF32)*ovcount*3);
				result.mNumFaces          = hr.mFaceCount;
				result.mNumIndices        = hr.mIndexCount;

				result.mIndices           = (NxU32 *) MEMALLOC_MALLOC( sizeof(NxU32)*hr.mIndexCount);

				memcpy(result.mOutputVertices, vscratch, sizeof(NxF32)*3*ovcount );

  			if ( desc.HasHullFlag(QF_REVERSE_ORDER) )
				{

					const NxU32 *source = hr.mIndices;
								NxU32 *dest   = result.mIndices;

					for (NxU32 i=0; i<hr.mFaceCount; i++)
					{
						dest[0] = source[2];
						dest[1] = source[1];
						dest[2] = source[0];
						dest+=3;
						source+=3;
					}

				}
				else
				{
					memcpy(result.mIndices, hr.mIndices, sizeof(NxU32)*hr.mIndexCount);
				}
			}
			else
			{
				result.mPolygons          = true;
				result.mNumOutputVertices = ovcount;
				result.mOutputVertices    = (NxF32 *)MEMALLOC_MALLOC( sizeof(NxF32)*ovcount*3);
				result.mNumFaces          = hr.mFaceCount;
				result.mNumIndices        = hr.mIndexCount+hr.mFaceCount;
				result.mIndices           = (NxU32 *) MEMALLOC_MALLOC( sizeof(NxU32)*result.mNumIndices);
				memcpy(result.mOutputVertices, vscratch, sizeof(NxF32)*3*ovcount );

				{
					const NxU32 *source = hr.mIndices;
								NxU32 *dest   = result.mIndices;
					for (NxU32 i=0; i<hr.mFaceCount; i++)
					{
						dest[0] = 3;
						if ( desc.HasHullFlag(QF_REVERSE_ORDER) )
						{
							dest[1] = source[2];
							dest[2] = source[1];
							dest[3] = source[0];
						}
						else
						{
							dest[1] = source[0];
							dest[2] = source[1];
							dest[3] = source[2];
						}

						dest+=4;
						source+=3;
					}
				}
			}
			// ReleaseHull frees memory for hr.mVertices, which can be the
			// same pointer as vsource, so be sure to set it to NULL if necessary
			if ( hr.mVertices == vsource) vsource = NULL;

			ReleaseHull(hr);

			if ( vscratch )
			{
				MEMALLOC_FREE(vscratch);
			}
		}
	}

	// this pointer is usually freed in ReleaseHull()
	if ( vsource )
	{
		MEMALLOC_FREE(vsource);
	}


	return ret;
}



HullError HullLibrary::ReleaseResult(HullResult &result) // release memory allocated for this result, we are done with it.
{
	if ( result.mOutputVertices )
	{
		MEMALLOC_FREE(result.mOutputVertices);
		result.mOutputVertices = 0;
	}
	if ( result.mIndices )
	{
		MEMALLOC_FREE(result.mIndices);
		result.mIndices = 0;
	}
	return QE_OK;
}


static void AddPoint(NxU32 &vcount,NxF32 *p,NxF32 x,NxF32 y,NxF32 z)
{
	NxF32 *dest = &p[vcount*3];
	dest[0] = x;
	dest[1] = y;
	dest[2] = z;
	vcount++;
}


NxF32 GetDist(NxF32 px,NxF32 py,NxF32 pz,const NxF32 *p2)
{

	NxF32 dx = px - p2[0];
	NxF32 dy = py - p2[1];
	NxF32 dz = pz - p2[2];

	return dx*dx+dy*dy+dz*dz;
}



bool  HullLibrary::CleanupVertices(NxU32 svcount,
																const NxF32 *svertices,
																NxU32 stride,
																NxU32 &vcount,       // output number of vertices
																NxF32 *vertices,                 // location to store the results.
																NxF32  normalepsilon,
																NxF32 *scale)
{
	if ( svcount == 0 ) return false;


	#define EPSILON 0.000001f // close enough to consider two floating point numbers to be 'the same'.

	vcount = 0;

	NxF32 recip[3];

	if ( scale )
	{
		scale[0] = 1;
		scale[1] = 1;
		scale[2] = 1;
	}

	NxF32 bmin[3] = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
	NxF32 bmax[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	const char *vtx = (const char *) svertices;

	{
		for (NxU32 i=0; i<svcount; i++)
		{
			const NxF32 *p = (const NxF32 *) vtx;

			vtx+=stride;

			for (NxI32 j=0; j<3; j++)
			{
				if ( p[j] < bmin[j] ) bmin[j] = p[j];
				if ( p[j] > bmax[j] ) bmax[j] = p[j];
			}
		}
	}

	NxF32 dx = bmax[0] - bmin[0];
	NxF32 dy = bmax[1] - bmin[1];
	NxF32 dz = bmax[2] - bmin[2];

	NxF32 center[3];

	center[0] = dx*0.5f + bmin[0];
	center[1] = dy*0.5f + bmin[1];
	center[2] = dz*0.5f + bmin[2];

	if ( dx < EPSILON || dy < EPSILON || dz < EPSILON || svcount < 3 )
	{

		NxF32 len = FLT_MAX;

		if ( dx > EPSILON && dx < len ) len = dx;
		if ( dy > EPSILON && dy < len ) len = dy;
		if ( dz > EPSILON && dz < len ) len = dz;

		if ( len == FLT_MAX )
		{
			dx = dy = dz = 0.01f; // one centimeter
		}
		else
		{
			if ( dx < EPSILON ) dx = len * 0.05f; // 1/5th the shortest non-zero edge.
			if ( dy < EPSILON ) dy = len * 0.05f;
			if ( dz < EPSILON ) dz = len * 0.05f;
		}

		NxF32 x1 = center[0] - dx;
		NxF32 x2 = center[0] + dx;

		NxF32 y1 = center[1] - dy;
		NxF32 y2 = center[1] + dy;

		NxF32 z1 = center[2] - dz;
		NxF32 z2 = center[2] + dz;

		AddPoint(vcount,vertices,x1,y1,z1);
		AddPoint(vcount,vertices,x2,y1,z1);
		AddPoint(vcount,vertices,x2,y2,z1);
		AddPoint(vcount,vertices,x1,y2,z1);
		AddPoint(vcount,vertices,x1,y1,z2);
		AddPoint(vcount,vertices,x2,y1,z2);
		AddPoint(vcount,vertices,x2,y2,z2);
		AddPoint(vcount,vertices,x1,y2,z2);

		return true; // return cube


	}
	else
	{
		if ( scale )
		{
			scale[0] = dx;
			scale[1] = dy;
			scale[2] = dz;

			recip[0] = 1 / dx;
			recip[1] = 1 / dy;
			recip[2] = 1 / dz;

			center[0]*=recip[0];
			center[1]*=recip[1];
			center[2]*=recip[2];

		}

	}



	vtx = (const char *) svertices;

	for (NxU32 i=0; i<svcount; i++)
	{

		const NxF32 *p = (const NxF32 *)vtx;
		vtx+=stride;

		NxF32 px = p[0];
		NxF32 py = p[1];
		NxF32 pz = p[2];

		if ( scale )
		{
			px = px*recip[0]; // normalize
			py = py*recip[1]; // normalize
			pz = pz*recip[2]; // normalize
		}

		{
			NxU32 j;

			for (j=0; j<vcount; j++)
			{
				NxF32 *v = &vertices[j*3];

				NxF32 x = v[0];
				NxF32 y = v[1];
				NxF32 z = v[2];

				NxF32 dx = fabsf(x - px );
				NxF32 dy = fabsf(y - py );
				NxF32 dz = fabsf(z - pz );

				if ( dx < normalepsilon && dy < normalepsilon && dz < normalepsilon )
				{
					// ok, it is close enough to the old one
					// now let us see if it is further from the center of the point cloud than the one we already recorded.
					// in which case we keep this one instead.

					NxF32 dist1 = GetDist(px,py,pz,center);
					NxF32 dist2 = GetDist(v[0],v[1],v[2],center);

					if ( dist1 > dist2 )
					{
						v[0] = px;
						v[1] = py;
						v[2] = pz;
					}

					break;
				}
			}

			if ( j == vcount )
			{
				NxF32 *dest = &vertices[vcount*3];
				dest[0] = px;
				dest[1] = py;
				dest[2] = pz;
				vcount++;
			}
		}
	}

	// ok..now make sure we didn't prune so many vertices it is now invalid.
	{
		NxF32 bmin[3] = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
		NxF32 bmax[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

		for (NxU32 i=0; i<vcount; i++)
		{
			const NxF32 *p = &vertices[i*3];
			for (NxI32 j=0; j<3; j++)
			{
				if ( p[j] < bmin[j] ) bmin[j] = p[j];
				if ( p[j] > bmax[j] ) bmax[j] = p[j];
			}
		}

		NxF32 dx = bmax[0] - bmin[0];
		NxF32 dy = bmax[1] - bmin[1];
		NxF32 dz = bmax[2] - bmin[2];

		if ( dx < EPSILON || dy < EPSILON || dz < EPSILON || vcount < 3)
		{
			NxF32 cx = dx*0.5f + bmin[0];
			NxF32 cy = dy*0.5f + bmin[1];
			NxF32 cz = dz*0.5f + bmin[2];

			NxF32 len = FLT_MAX;

			if ( dx >= EPSILON && dx < len ) len = dx;
			if ( dy >= EPSILON && dy < len ) len = dy;
			if ( dz >= EPSILON && dz < len ) len = dz;

			if ( len == FLT_MAX )
			{
				dx = dy = dz = 0.01f; // one centimeter
			}
			else
			{
				if ( dx < EPSILON ) dx = len * 0.05f; // 1/5th the shortest non-zero edge.
				if ( dy < EPSILON ) dy = len * 0.05f;
				if ( dz < EPSILON ) dz = len * 0.05f;
			}

			NxF32 x1 = cx - dx;
			NxF32 x2 = cx + dx;

			NxF32 y1 = cy - dy;
			NxF32 y2 = cy + dy;

			NxF32 z1 = cz - dz;
			NxF32 z2 = cz + dz;

			vcount = 0; // add box

			AddPoint(vcount,vertices,x1,y1,z1);
			AddPoint(vcount,vertices,x2,y1,z1);
			AddPoint(vcount,vertices,x2,y2,z1);
			AddPoint(vcount,vertices,x1,y2,z1);
			AddPoint(vcount,vertices,x1,y1,z2);
			AddPoint(vcount,vertices,x2,y1,z2);
			AddPoint(vcount,vertices,x2,y2,z2);
			AddPoint(vcount,vertices,x1,y2,z2);

			return true;
		}
	}

	return true;
}

void HullLibrary::BringOutYourDead(const NxF32 *verts,NxU32 vcount, NxF32 *overts,NxU32 &ocount,NxU32 *indices,NxU32 indexcount)
{
	NxU32 *used = (NxU32 *)MEMALLOC_MALLOC(sizeof(NxU32)*vcount);
	memset(used,0,sizeof(NxU32)*vcount);

	ocount = 0;

	for (NxU32 i=0; i<indexcount; i++)
	{
		NxU32 v = indices[i]; // original array index

		assert( v < vcount );

		if ( used[v] ) // if already remapped
		{
			indices[i] = used[v]-1; // index to new array
		}
		else
		{

			indices[i] = ocount;      // new index mapping

			overts[ocount*3+0] = verts[v*3+0]; // copy old vert to new vert array
			overts[ocount*3+1] = verts[v*3+1];
			overts[ocount*3+2] = verts[v*3+2];

			ocount++; // increment output vert count

			assert( ocount <= vcount );

			used[v] = ocount; // assign new index remapping
		}
	}

	MEMALLOC_FREE(used);
}

//==================================================================================
HullError HullLibrary::CreateTriangleMesh(HullResult &answer,ConvexHullTriangleInterface *iface)
{
	HullError ret = QE_FAIL;


	const NxF32 *p            = answer.mOutputVertices;
	const NxU32   *idx = answer.mIndices;
	NxU32 fcount       = answer.mNumFaces;

	if ( p && idx && fcount )
	{
		ret = QE_OK;

		for (NxU32 i=0; i<fcount; i++)
		{
			NxU32 pcount = *idx++;

			NxU32 i1 = *idx++;
			NxU32 i2 = *idx++;
			NxU32 i3 = *idx++;

			const NxF32 *p1 = &p[i1*3];
			const NxF32 *p2 = &p[i2*3];
			const NxF32 *p3 = &p[i3*3];

			AddConvexTriangle(iface,p1,p2,p3);

			pcount-=3;
			while ( pcount )
			{
				i3 = *idx++;
				p2 = p3;
				p3 = &p[i3*3];

				AddConvexTriangle(iface,p1,p2,p3);
				pcount--;
			}

		}
	}

	return ret;
}

//==================================================================================
void HullLibrary::AddConvexTriangle(ConvexHullTriangleInterface *callback,const NxF32 *p1,const NxF32 *p2,const NxF32 *p3)
{
	ConvexHullVertex v1,v2,v3;

	#define TSCALE1 (1.0f/4.0f)

	v1.mPos[0] = p1[0];
	v1.mPos[1] = p1[1];
	v1.mPos[2] = p1[2];

	v2.mPos[0] = p2[0];
	v2.mPos[1] = p2[1];
	v2.mPos[2] = p2[2];

	v3.mPos[0] = p3[0];
	v3.mPos[1] = p3[1];
	v3.mPos[2] = p3[2];

	NxF32 n[3];
	ComputeNormal(n,p1,p2,p3);

	v1.mNormal[0] = n[0];
	v1.mNormal[1] = n[1];
	v1.mNormal[2] = n[2];

	v2.mNormal[0] = n[0];
	v2.mNormal[1] = n[1];
	v2.mNormal[2] = n[2];

	v3.mNormal[0] = n[0];
	v3.mNormal[1] = n[1];
	v3.mNormal[2] = n[2];

	const NxF32 *tp1 = p1;
	const NxF32 *tp2 = p2;
	const NxF32 *tp3 = p3;

	NxI32 i1 = 0;
	NxI32 i2 = 0;

	NxF32 nx = fabsf(n[0]);
	NxF32 ny = fabsf(n[1]);
	NxF32 nz = fabsf(n[2]);

	if ( nx <= ny && nx <= nz )
		i1 = 0;
	if ( ny <= nx && ny <= nz )
		i1 = 1;
	if ( nz <= nx && nz <= ny )
		i1 = 2;

	switch ( i1 )
	{
		case 0:
			if ( ny < nz )
				i2 = 1;
			else
				i2 = 2;
			break;
		case 1:
			if ( nx < nz )
				i2 = 0;
			else
				i2 = 2;
			break;
		case 2:
			if ( nx < ny )
				i2 = 0;
			else
				i2 = 1;
			break;
	}

	v1.mTexel[0] = tp1[i1]*TSCALE1;
	v1.mTexel[1] = tp1[i2]*TSCALE1;

	v2.mTexel[0] = tp2[i1]*TSCALE1;
	v2.mTexel[1] = tp2[i2]*TSCALE1;

	v3.mTexel[0] = tp3[i1]*TSCALE1;
	v3.mTexel[1] = tp3[i2]*TSCALE1;

	callback->ConvexHullTriangle(v3,v2,v1);
}

//==================================================================================
NxF32 HullLibrary::ComputeNormal(NxF32 *n,const NxF32 *A,const NxF32 *B,const NxF32 *C)
{
	NxF32 vx,vy,vz,wx,wy,wz,vw_x,vw_y,vw_z,mag;

	vx = (B[0] - C[0]);
	vy = (B[1] - C[1]);
	vz = (B[2] - C[2]);

	wx = (A[0] - B[0]);
	wy = (A[1] - B[1]);
	wz = (A[2] - B[2]);

	vw_x = vy * wz - vz * wy;
	vw_y = vz * wx - vx * wz;
	vw_z = vx * wy - vy * wx;

	mag = sqrtf((vw_x * vw_x) + (vw_y * vw_y) + (vw_z * vw_z));

	if ( mag < 0.000001f )
	{
		mag = 0;
	}
	else
	{
		mag = 1.0f/mag;
	}

	n[0] = vw_x * mag;
	n[1] = vw_y * mag;
	n[2] = vw_z * mag;

	return mag;
}

}; // End of namespace
