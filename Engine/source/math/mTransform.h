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

#ifndef _MTRANSFORM_H_
#define _MTRANSFORM_H_

#ifndef _MPOINT3_H_
   #include "math/mPoint3.h"
#endif
#ifndef _MANGAXIS_H_
   #include "math/mAngAxis.h"
#endif
#ifndef _MMATRIX_H_
   #include "math/mMatrix.h"
#endif

/// A transform expressed as a combination of a position vector and an angular
/// orientation.
class TransformF
{
   public:
   
      Point3F mPosition;
      AngAxisF mOrientation;
      bool mHasRotation;

      static const TransformF Identity;

      TransformF()
         :  mPosition( Point3F::Zero ),
            mOrientation( Point3F( 0, 0, 1 ), 0 ),
            mHasRotation(true)
      {
      }

      TransformF( const Point3F& position, const AngAxisF& orientation )
      {
         set( position, orientation );
         mHasRotation = true;
      }

      TransformF( const MatrixF& mat )
      {
         set( mat );
         mHasRotation = true;
      }

      bool hasRotation() const { return mHasRotation; }

      void set( const Point3F& position, const AngAxisF& orientation )
      {
         mPosition = position;
         mOrientation = orientation;
      }

      void set( const MatrixF& mat )
      {
         mPosition = mat.getPosition();
         mOrientation.set( mat );
      }

      /// Return the position vector of the transform.
      const Point3F& getPosition() const { return mPosition; }
      
      /// REturn the orientation of the transform.
      const AngAxisF& getOrientation() const { return mOrientation; }

      MatrixF getMatrix() const
      {
         MatrixF mat;
         mOrientation.setMatrix( &mat );
         mat.setPosition( mPosition );

         return mat;
      }
};

#endif // !_MTRANSFORM_H_
