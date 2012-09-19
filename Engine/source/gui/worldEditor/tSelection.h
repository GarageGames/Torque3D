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

#ifndef _TSELECTION_H_
#define _TSELECTION_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

#ifndef _MMATRIX_H_
#include "math/mMatrix.h"
#endif


template <class T>
class Selection : public Vector<T>
{
public:

   // Use explicit specialization to define these for your type.
   MatrixF getOrientation() { return MatrixF(); }
   Point3F getOrigin() { return Point3F(); }
   Point3F getScale() { return Point3F(); }

   void offset( const Point3F &delta );
   void rotate( const EulerF &delta );
   void scale( const Point3F &delta );

protected:
   
   // Use explicit specialization to define these for your type.
   virtual void offsetObject( T &object, const Point3F &delta ) {}
   virtual void rotateObject( T &object, const EulerF &delta, const Point3F &origin ) {}
   virtual void scaleObject( T &object, const Point3F &delta ) {}

protected:

   //Point3F        mCentroid;
   //Point3F        mBoxCentroid;
   //Box3F          mBoxBounds;
   //bool           mCentroidValid;
};


template<class T> inline void Selection<T>::offset( const Point3F &delta )
{
   typename Selection<T>::iterator itr = this->begin();

   for ( ; itr != this->end(); itr++ )   
      offsetObject( *itr, delta );      
}

template<class T> inline void Selection<T>::rotate( const EulerF &delta )
{
   typename Selection<T>::iterator itr = this->begin();
   Point3F origin = getOrigin();

   for ( ; itr != this->end(); itr++ )   
      rotateObject( *itr, delta, origin );
}

template<class T> inline void Selection<T>::scale( const Point3F &delta )
{
   // Can only scale a single selection.
   if ( this->size() != 1 )
      return;

   scaleObject( this->mArray[0], delta );   
}

#endif // _TSELECTION_H_