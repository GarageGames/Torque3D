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

#include "core/strings/stringFunctions.h"
#include "console/consoleTypes.h"
#include "console/console.h"
#include "console/engineAPI.h"
#include "math/mPoint2.h"
#include "math/mPoint3.h"
#include "math/mMatrix.h"
#include "math/mQuat.h"
#include "math/mRect.h"
#include "math/mBox.h"
#include "math/mAngAxis.h"
#include "math/mTransform.h"
#include "math/mathTypes.h"
#include "math/mRandom.h"
#include "math/mEase.h"
#include "math/mathUtils.h"


IMPLEMENT_SCOPE( MathTypes, Math,, "" );

IMPLEMENT_STRUCT( Point2I,
   Point2I, MathTypes,
   "" )

      FIELD( x, x, 1, "X coordinate." )
      FIELD( y, y, 1, "Y coordinate." )

END_IMPLEMENT_STRUCT;
IMPLEMENT_STRUCT( Point2F,
   Point2F, MathTypes,
   "" )

      FIELD( x, x, 1, "X coordinate." )
      FIELD( y, y, 1, "Y coordinate." )

END_IMPLEMENT_STRUCT;
IMPLEMENT_STRUCT( Point3I,
   Point3I, MathTypes,
   "" )

      FIELD( x, x, 1, "X coordinate." )
      FIELD( y, y, 1, "Y coordinate." )
      FIELD( z, z, 1, "Z coordinate." )

END_IMPLEMENT_STRUCT;
IMPLEMENT_STRUCT( Point3F,
   Point3F, MathTypes,
   "" )

      FIELD( x, x, 1, "X coordinate." )
      FIELD( y, y, 1, "Y coordinate." )
      FIELD( z, z, 1, "Z coordinate." )

END_IMPLEMENT_STRUCT;
IMPLEMENT_STRUCT( Point4F,
   Point4F, MathTypes,
   "" )
   
      FIELD( x, x, 1, "X coordinate." )
      FIELD( y, y, 1, "Y coordinate." )
      FIELD( z, z, 1, "Z coordinate." )
      FIELD( w, w, 1, "W coordinate." )
      
END_IMPLEMENT_STRUCT;
IMPLEMENT_STRUCT( RectI,
   RectI, MathTypes,
   "" )
END_IMPLEMENT_STRUCT;
IMPLEMENT_STRUCT( RectF,
   RectF, MathTypes,
   "" )
END_IMPLEMENT_STRUCT;
IMPLEMENT_STRUCT( MatrixF,
   MatrixF, MathTypes,
   "" )
END_IMPLEMENT_STRUCT;
IMPLEMENT_STRUCT( AngAxisF,
   AngAxisF, MathTypes,
   "" )
END_IMPLEMENT_STRUCT;
IMPLEMENT_STRUCT( TransformF,
   TransformF, MathTypes,
   "" )
END_IMPLEMENT_STRUCT;
IMPLEMENT_STRUCT( Box3F,
   Box3F, MathTypes,
   "" )
END_IMPLEMENT_STRUCT;
IMPLEMENT_STRUCT( EaseF,
   EaseF, MathTypes,
   "" )
END_IMPLEMENT_STRUCT;


//-----------------------------------------------------------------------------
// TypePoint2I
//-----------------------------------------------------------------------------
ConsoleType( Point2I, TypePoint2I, Point2I )
ImplementConsoleTypeCasters( TypePoint2I, Point2I )

ConsoleGetType( TypePoint2I )
{
   Point2I *pt = (Point2I *) dptr;
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%d %d", pt->x, pt->y);
   return returnBuffer;
}

ConsoleSetType( TypePoint2I )
{
   if(argc == 1)
      dSscanf(argv[0], "%d %d", &((Point2I *) dptr)->x, &((Point2I *) dptr)->y);
   else if(argc == 2)
      *((Point2I *) dptr) = Point2I(dAtoi(argv[0]), dAtoi(argv[1]));
   else
      Con::printf("Point2I must be set as { x, y } or \"x y\"");
}

//-----------------------------------------------------------------------------
// TypePoint2F
//-----------------------------------------------------------------------------
ConsoleType( Point2F, TypePoint2F, Point2F )
ImplementConsoleTypeCasters( TypePoint2F, Point2F )

ConsoleGetType( TypePoint2F )
{
   Point2F *pt = (Point2F *) dptr;
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%g %g", pt->x, pt->y);
   return returnBuffer;
}

ConsoleSetType( TypePoint2F )
{
   if(argc == 1)
      dSscanf(argv[0], "%g %g", &((Point2F *) dptr)->x, &((Point2F *) dptr)->y);
   else if(argc == 2)
      *((Point2F *) dptr) = Point2F(dAtof(argv[0]), dAtof(argv[1]));
   else
      Con::printf("Point2F must be set as { x, y } or \"x y\"");
}

//-----------------------------------------------------------------------------
// TypePoint3I
//-----------------------------------------------------------------------------
ConsoleType( Point3I, TypePoint3I, Point3I )
ImplementConsoleTypeCasters(TypePoint3I, Point3I)

ConsoleGetType( TypePoint3I )
{
   Point3I *pt = (Point3I *) dptr;
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%d %d %d", pt->x, pt->y, pt->z);
   return returnBuffer;
}

ConsoleSetType( TypePoint3I )
{
   if(argc == 1)
      dSscanf(argv[0], "%d %d %d", &((Point3I *) dptr)->x, &((Point3I *) dptr)->y, &((Point3I *) dptr)->z);
   else if(argc == 3)
      *((Point3I *) dptr) = Point3I(dAtoi(argv[0]), dAtoi(argv[1]), dAtoi(argv[2]));
   else
      Con::printf("Point3I must be set as { x, y, z } or \"x y z\"");
}

//-----------------------------------------------------------------------------
// TypePoint3F
//-----------------------------------------------------------------------------
ConsoleType( Point3F, TypePoint3F, Point3F )
ImplementConsoleTypeCasters(TypePoint3F, Point3F)

ConsoleGetType( TypePoint3F )
{
   Point3F *pt = (Point3F *) dptr;
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%g %g %g", pt->x, pt->y, pt->z);
   return returnBuffer;
}

ConsoleSetType( TypePoint3F )
{
   if(argc == 1)
      dSscanf(argv[0], "%g %g %g", &((Point3F *) dptr)->x, &((Point3F *) dptr)->y, &((Point3F *) dptr)->z);
   else if(argc == 3)
      *((Point3F *) dptr) = Point3F(dAtof(argv[0]), dAtof(argv[1]), dAtof(argv[2]));
   else
      Con::printf("Point3F must be set as { x, y, z } or \"x y z\"");
}

//-----------------------------------------------------------------------------
// TypePoint4F
//-----------------------------------------------------------------------------
ConsoleType( Point4F, TypePoint4F, Point4F )
ImplementConsoleTypeCasters( TypePoint4F, Point4F )

ConsoleGetType( TypePoint4F )
{
   Point4F *pt = (Point4F *) dptr;
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%g %g %g %g", pt->x, pt->y, pt->z, pt->w);
   return returnBuffer;
}

ConsoleSetType( TypePoint4F )
{
   if(argc == 1)
      dSscanf(argv[0], "%g %g %g %g", &((Point4F *) dptr)->x, &((Point4F *) dptr)->y, &((Point4F *) dptr)->z, &((Point4F *) dptr)->w);
   else if(argc == 4)
      *((Point4F *) dptr) = Point4F(dAtof(argv[0]), dAtof(argv[1]), dAtof(argv[2]), dAtof(argv[3]));
   else
      Con::printf("Point4F must be set as { x, y, z, w } or \"x y z w\"");
}

//-----------------------------------------------------------------------------
// TypeRectI
//-----------------------------------------------------------------------------
ConsoleType( RectI, TypeRectI, RectI )
ImplementConsoleTypeCasters( TypeRectI, RectI )

ConsoleGetType( TypeRectI )
{
   RectI *rect = (RectI *) dptr;
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%d %d %d %d", rect->point.x, rect->point.y,
            rect->extent.x, rect->extent.y);
   return returnBuffer;
}

ConsoleSetType( TypeRectI )
{
   if(argc == 1)
      dSscanf(argv[0], "%d %d %d %d", &((RectI *) dptr)->point.x, &((RectI *) dptr)->point.y,
              &((RectI *) dptr)->extent.x, &((RectI *) dptr)->extent.y);
   else if(argc == 4)
      *((RectI *) dptr) = RectI(dAtoi(argv[0]), dAtoi(argv[1]), dAtoi(argv[2]), dAtoi(argv[3]));
   else
      Con::printf("RectI must be set as { x, y, w, h } or \"x y w h\"");
}

//-----------------------------------------------------------------------------
// TypeRectF
//-----------------------------------------------------------------------------
ConsoleType( RectF, TypeRectF, RectF )
ImplementConsoleTypeCasters( TypeRectF, RectF )

ConsoleGetType( TypeRectF )
{
   RectF *rect = (RectF *) dptr;
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%g %g %g %g", rect->point.x, rect->point.y,
            rect->extent.x, rect->extent.y);
   return returnBuffer;
}

ConsoleSetType( TypeRectF )
{
   if(argc == 1)
      dSscanf(argv[0], "%g %g %g %g", &((RectF *) dptr)->point.x, &((RectF *) dptr)->point.y,
              &((RectF *) dptr)->extent.x, &((RectF *) dptr)->extent.y);
   else if(argc == 4)
      *((RectF *) dptr) = RectF(dAtof(argv[0]), dAtof(argv[1]), dAtof(argv[2]), dAtof(argv[3]));
   else
      Con::printf("RectF must be set as { x, y, w, h } or \"x y w h\"");
}

//-----------------------------------------------------------------------------
// TypeMatrix
//-----------------------------------------------------------------------------
ConsoleType( MatrixF, TypeMatrixF, MatrixF )
ImplementConsoleTypeCasters( TypeMatrixF, MatrixF )

// Oh merry confusion.  Torque stores matrices in row-major order yet to TorqueScript
// matrices were passed in column-major order, so we need to stick to this here.

ConsoleGetType( TypeMatrixF )
{
   MatrixF* mat = ( MatrixF* ) dptr;

   Point3F col0, col1, col2;
   mat->getColumn(0, &col0);
   mat->getColumn(1, &col1);
   mat->getColumn(2, &col2);
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer,256,"%g %g %g %g %g %g %g %g %g",
            col0.x, col0.y, col0.z, col1.x, col1.y, col1.z, col2.x, col2.y, col2.z);
   return returnBuffer;
}

ConsoleSetType( TypeMatrixF )
{
   if( argc != 1 )
   {
      Con::errorf( "MatrixF must be set as \"c0x c0y c0z c1x c1y c1z c2x c2y c2z\"" );
      return;
   }
   
   Point3F col0, col1, col2;
   dSscanf( argv[ 0 ], "%g %g %g %g %g %g %g %g %g",
            &col0.x, &col0.y, &col0.z, &col1.x, &col1.y, &col1.z, &col2.x, &col2.y, &col2.z );

   MatrixF* mat = ( MatrixF* ) dptr;
   
   mat->setColumn( 0, col0 );
   mat->setColumn( 1, col1 );
   mat->setColumn( 2, col2 );
}

//-----------------------------------------------------------------------------
// TypeMatrixPosition
//-----------------------------------------------------------------------------
ConsoleType( MatrixPosition, TypeMatrixPosition, MatrixF )

ConsoleGetType( TypeMatrixPosition )
{
   F32 *col = (F32 *) dptr + 3;
   char* returnBuffer = Con::getReturnBuffer(256);
   if(col[12] == 1.f)
      dSprintf(returnBuffer, 256, "%g %g %g", col[0], col[4], col[8]);
   else
      dSprintf(returnBuffer, 256, "%g %g %g %g", col[0], col[4], col[8], col[12]);
   return returnBuffer;
}

ConsoleSetType( TypeMatrixPosition )
{
   F32 *col = ((F32 *) dptr) + 3;
   if (argc == 1)
   {
      col[0] = col[4] = col[8] = 0.f;
      col[12] = 1.f;
      dSscanf(argv[0], "%g %g %g %g", &col[0], &col[4], &col[8], &col[12]);
   }
   else if (argc <= 4) 
   {
      for (S32 i = 0; i < argc; i++)
         col[i << 2] = dAtof(argv[i]);
   }
   else
      Con::printf("Matrix position must be set as { x, y, z, w } or \"x y z w\"");
}

//-----------------------------------------------------------------------------
// TypeMatrixRotation
//-----------------------------------------------------------------------------
ConsoleType( MatrixRotation, TypeMatrixRotation, MatrixF )

ConsoleGetType( TypeMatrixRotation )
{
   AngAxisF aa(*(MatrixF *) dptr);
   aa.axis.normalize();
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer,256,"%g %g %g %g",aa.axis.x,aa.axis.y,aa.axis.z,mRadToDeg(aa.angle));
   return returnBuffer;
}

ConsoleSetType( TypeMatrixRotation )
{
   // DMM: Note that this will ONLY SET the ULeft 3x3 submatrix.
   //
   AngAxisF aa(Point3F(0,0,0),0);
   if (argc == 1)
   {
      dSscanf(argv[0], "%g %g %g %g", &aa.axis.x, &aa.axis.y, &aa.axis.z, &aa.angle);
      aa.angle = mDegToRad(aa.angle);
   }
   else if (argc == 4) 
   {
         for (S32 i = 0; i < argc; i++)
            ((F32*)&aa)[i] = dAtof(argv[i]);
         aa.angle = mDegToRad(aa.angle);
   }
   else
      Con::printf("Matrix rotation must be set as { x, y, z, angle } or \"x y z angle\"");

   //
   MatrixF temp;
   aa.setMatrix(&temp);

   F32* pDst = *(MatrixF *)dptr;
   const F32* pSrc = temp;
   for (U32 i = 0; i < 3; i++)
      for (U32 j = 0; j < 3; j++)
         pDst[i*4 + j] = pSrc[i*4 + j];
}

//-----------------------------------------------------------------------------
// TypeAngAxisF
//-----------------------------------------------------------------------------
ConsoleType( AngAxisF, TypeAngAxisF, AngAxisF )
ImplementConsoleTypeCasters( TypeAngAxisF, AngAxisF )

ConsoleGetType( TypeAngAxisF )
{
   AngAxisF* aa = ( AngAxisF* ) dptr;
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer,256,"%g %g %g %g",aa->axis.x,aa->axis.y,aa->axis.z,mRadToDeg(aa->angle));
   return returnBuffer;
}

ConsoleSetType( TypeAngAxisF )
{
   // DMM: Note that this will ONLY SET the ULeft 3x3 submatrix.
   //
   AngAxisF* aa = ( AngAxisF* ) dptr;
   if (argc == 1)
   {
      dSscanf(argv[0], "%g %g %g %g", &aa->axis.x, &aa->axis.y, &aa->axis.z, &aa->angle);
      aa->angle = mDegToRad(aa->angle);
   }
   else if (argc == 4) 
   {
      for (S32 i = 0; i < argc; i++)
         ((F32*)&aa)[i] = dAtof(argv[i]);
      aa->angle = mDegToRad(aa->angle);
   }
   else
      Con::printf("AngAxisF must be set as { x, y, z, angle } or \"x y z angle\"");
}


//-----------------------------------------------------------------------------
// TypeTransformF
//-----------------------------------------------------------------------------

const TransformF TransformF::Identity( Point3F::Zero, AngAxisF( Point3F( 0, 0, 1 ), 0) );

ConsoleType( TransformF, TypeTransformF, TransformF )
ImplementConsoleTypeCasters( TypeTransformF, TransformF )

ConsoleGetType( TypeTransformF )
{
   TransformF* aa = ( TransformF* ) dptr;
   char* returnBuffer = Con::getReturnBuffer( 256 );
   dSprintf( returnBuffer, 256, "%g %g %g %g %g %g %g",
             aa->mPosition.x, aa->mPosition.y, aa->mPosition.z,
             aa->mOrientation.axis.x, aa->mOrientation.axis.y, aa->mOrientation.axis.z, aa->mOrientation.angle );
   return returnBuffer;
}

ConsoleSetType( TypeTransformF )
{
   TransformF* aa = ( TransformF* ) dptr;
   if( argc == 1 )
   {
      U32 count = dSscanf( argv[ 0 ], "%g %g %g %g %g %g %g",
               &aa->mPosition.x, &aa->mPosition.y, &aa->mPosition.z,
               &aa->mOrientation.axis.x, &aa->mOrientation.axis.y, &aa->mOrientation.axis.z, &aa->mOrientation.angle );

      aa->mHasRotation = ( count == 7 );
   }
   else if( argc == 7 )
   {
      aa->mPosition.x = dAtof( argv[ 0 ] );
      aa->mPosition.y = dAtof( argv[ 1 ] );
      aa->mPosition.z = dAtof( argv[ 2 ] );
      aa->mOrientation.axis.x = dAtof( argv[ 3 ] );
      aa->mOrientation.axis.y = dAtof( argv[ 4 ] );
      aa->mOrientation.axis.z = dAtof( argv[ 5 ] );
      aa->mOrientation.angle = dAtof( argv[ 6 ] );
   }
   else
      Con::errorf( "TransformF must be set as { px, py, pz, x, y, z, angle } or \"px py pz x y z angle\"");
}



//-----------------------------------------------------------------------------
// TypeBox3F
//-----------------------------------------------------------------------------
ConsoleType( Box3F, TypeBox3F, Box3F )
ImplementConsoleTypeCasters( TypeBox3F, Box3F )

ConsoleGetType( TypeBox3F )
{
   const Box3F* pBox = (const Box3F*)dptr;

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%g %g %g %g %g %g",
            pBox->minExtents.x, pBox->minExtents.y, pBox->minExtents.z,
            pBox->maxExtents.x, pBox->maxExtents.y, pBox->maxExtents.z);

   return returnBuffer;
}

ConsoleSetType( TypeBox3F )
{
   Box3F* pDst = (Box3F*)dptr;

   if (argc == 1) 
   {
      U32 args = dSscanf(argv[0], "%g %g %g %g %g %g",
                         &pDst->minExtents.x, &pDst->minExtents.y, &pDst->minExtents.z,
                         &pDst->maxExtents.x, &pDst->maxExtents.y, &pDst->maxExtents.z);
      AssertWarn(args == 6, "Warning, box probably not read properly");
   } 
   else 
   {
      Con::printf("Box3F must be set as \"xMin yMin zMin xMax yMax zMax\"");
   }
}


//-----------------------------------------------------------------------------
// TypeEaseF
//-----------------------------------------------------------------------------
ConsoleType( EaseF, TypeEaseF, EaseF )
ImplementConsoleTypeCasters( TypeEaseF, EaseF )

ConsoleGetType( TypeEaseF )
{
   const EaseF* pEase = (const EaseF*)dptr;

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%d %d %g %g",
            pEase->dir, pEase->type, pEase->param[0], pEase->param[1]);

   return returnBuffer;
}

ConsoleSetType( TypeEaseF )
{
   EaseF* pDst = (EaseF*)dptr;

   // defaults...
   pDst->param[0] = -1.0f;
   pDst->param[1] = -1.0f;
   if (argc == 1) {
      U32 args = dSscanf(argv[0], "%d %d %f %f", // the two params are optional and assumed -1 if not present...
                         &pDst->dir, &pDst->type, &pDst->param[0],&pDst->param[1]);
      if( args < 2 )
         Con::warnf( "Warning, EaseF probably not read properly" );
   } else {
      Con::printf("EaseF must be set as \"dir type [param0 param1]\"");
   }
}


//-----------------------------------------------------------------------------

DefineConsoleFunction( VectorAdd, VectorF, ( VectorF a, VectorF b ),,
   "Add two vectors.\n"
   "@param a The first vector.\n"
   "@param b The second vector.\n"
   "@return The vector @a a + @a b.\n\n"
   "@tsexample\n"
      "//-----------------------------------------------------------------------------\n"
      "//\n"
      "// VectorAdd( %a, %b );\n"
      "//\n"
      "// The sum of vector a, (ax, ay, az), and vector b, (bx, by, bz) is:\n"
      "//\n"
      "//     a + b = ( ax + bx, ay + by, az + bz )\n"
      "//\n"
      "//-----------------------------------------------------------------------------\n"
      "%a = \"1 0 0\";\n"
      "%b = \"0 1 0\";\n\n"
      "// %r = \"( 1 + 0, 0 + 1, 0 + 0 )\";\n"
      "// %r = \"1 1 0\";\n"
      "%r = VectorAdd( %a, %b );\n"
   "@endtsexample\n\n"
   "@ingroup Vectors")
{
   return a + b;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( VectorSub, VectorF, ( VectorF a, VectorF b ),,
   "Subtract two vectors.\n"
   "@param a The first vector.\n"
   "@param b The second vector.\n"
   "@return The vector @a a - @a b.\n\n"
   "@tsexample\n"
	"//-----------------------------------------------------------------------------\n"
	"//\n"
	"// VectorSub( %a, %b );\n"
	"//\n"
	"// The difference of vector a, (ax, ay, az), and vector b, (bx, by, bz) is:\n"
	"//\n"
	"//     a - b = ( ax - bx, ay - by, az - bz )\n"
	"//\n"
	"//-----------------------------------------------------------------------------\n\n"

	"%a = \"1 0 0\";\n"
	"%b = \"0 1 0\";\n\n"

	"// %r = \"( 1 - 0, 0 - 1, 0 - 0 )\";\n"
	"// %r = \"1 -1 0\";\n"
	"%r = VectorSub( %a, %b );\n"
   "@endtsexample\n\n"
   "@ingroup Vectors" )
{
   return a - b;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( VectorScale, VectorF, ( VectorF a, F32 scalar ),,
   "Scales a vector by a scalar.\n"
   "@param a The vector to scale.\n"
   "@param scalar The scale factor.\n"
   "@return The vector @a a * @a scalar.\n\n"
   "@tsexample\n"
	"//-----------------------------------------------------------------------------\n"
	"//\n"
	"// VectorScale( %a, %v );\n"
	"//\n"
	"// Scaling vector a, (ax, ay, az), but the scalar, v, is:\n"
	"//\n"
	"//     a * v = ( ax * v, ay * v, az * v )\n"
	"//\n"
	"//-----------------------------------------------------------------------------\n\n"

	"%a = \"1 1 0\";\n"
	"%v = \"2\";\n\n"

	"// %r = \"( 1 * 2, 1 * 2, 0 * 2 )\";\n"
	"// %r = \"2 2 0\";\n"
	"%r = VectorScale( %a, %v );\n"
   "@endtsexample\n\n"
   "@ingroup Vectors" )
{
   return a * scalar;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( VectorNormalize, VectorF, ( VectorF v ),,
   "Brings a vector into its unit form, i.e. such that it has the magnitute 1.\n"
   "@param v The vector to normalize.\n"
   "@return The vector @a v scaled to length 1.\n\n"
   "@tsexample\n"
	"//-----------------------------------------------------------------------------\n"
	"//\n"
	"// VectorNormalize( %a );\n"
	"//\n"
	"// The normalized vector a, (ax, ay, az), is:\n"
	"//\n"
	"//     a^ = a / ||a||\n"
	"//        = ( ax / ||a||, ay / ||a||, az / ||a|| )\n"
	"//\n"
	"//-----------------------------------------------------------------------------\n\n"

	"%a = \"1 1 0\";\n"
	"%l = 1.414;\n\n"

	"// %r = \"( 1 / 1.141, 1 / 1.141, 0 / 1.141 )\";\n"
	"// %r = \"0.707 0.707 0\";\n"
	"%r = VectorNormalize( %a );\n"
   "@endtsexample\n\n"
   "@ingroup Vectors" )
{
   VectorF n( v );
   n.normalizeSafe();
   return n;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( VectorDot, F32, ( VectorF a, VectorF b ),,
   "Compute the dot product of two vectors.\n"
   "@param a The first vector.\n"
   "@param b The second vector.\n"
   "@return The dot product @a a * @a b.\n\n"
   "@tsexample\n"
	"//-----------------------------------------------------------------------------\n"
	"//\n"
	"// VectorDot( %a, %b );\n"
	"//\n"
	"// The dot product between vector a, (ax, ay, az), and vector b, (bx, by, bz), is:\n"
	"//\n"
	"//     a . b = ( ax * bx + ay * by + az * bz )\n"
	"//\n"
	"//-----------------------------------------------------------------------------\n\n"

	"%a = \"1 1 0\";\n"
	"%b = \"2 0 1\";\n\n"

	"// %r = \"( 1 * 2 + 1 * 0 + 0 * 1 )\";\n"
	"// %r = 2;\n"
	"%r = VectorDot( %a, %b );\n"
   "@endtsexample\n\n"
   "@ingroup Vectors" )
{
   return mDot( a, b );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( VectorCross, VectorF, ( VectorF a, VectorF b ),,
   "Calculcate the cross product of two vectors.\n"
   "@param a The first vector.\n"
   "@param b The second vector.\n"
   "@return The cross product @a x @a b.\n\n"
   "@tsexample\n"
	"//-----------------------------------------------------------------------------\n"
	"//\n"
	"// VectorCross( %a, %b );\n"
	"//\n"
	"// The cross product of vector a, (ax, ay, az), and vector b, (bx, by, bz), is\n"
	"//\n"
	"//     a x b = ( ( ay * bz ) - ( az * by ), ( az * bx ) - ( ax * bz ), ( ax * by ) - ( ay * bx ) )\n"
	"//\n"
	"//-----------------------------------------------------------------------------\n\n"

	"%a = \"1 1 0\";\n"
	"%b = \"2 0 1\";\n\n"

	"// %r = \"( ( 1 * 1 ) - ( 0 * 0 ), ( 0 * 2 ) - ( 1 * 1 ), ( 1 * 0 ) - ( 1 * 2 ) )\";\n"
	"// %r = \"1 -1 -2\";\n"
	"%r = VectorCross( %a, %b );\n"
   "@endtsexample\n\n"	
   "@ingroup Vectors" )
{
   VectorF v;
   mCross( a, b, &v );
   return v;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( VectorDist, F32, ( VectorF a, VectorF b ),,
   "Compute the distance between two vectors.\n"
   "@param a The first vector.\n"
   "@param b The second vector.\n"
   "@return The length( @a b - @a a ).\n\n"
   "@tsexample\n"
	"//-----------------------------------------------------------------------------\n"
	"//\n"
	"// VectorDist( %a, %b );\n"
	"//\n"
	"// The distance between vector a, (ax, ay, az), and vector b, (bx, by, bz), is\n"
	"//\n"
	"//     a -> b = ||( b - a )||\n"
	"//            = ||( bx - ax, by - ay, bz - az )||\n"
	"//            = mSqrt( ( bx - ax ) * ( bx - ax ) + ( by - ay ) * ( by - ay ) + ( bz - az ) * ( bz - az ) )\n"
	"//\n"
	"//-----------------------------------------------------------------------------\n\n"

	"%a = \"1 1 0\";\n"
	"%b = \"2 0 1\";\n\n"

	"// %r = mSqrt( ( 2 - 1 ) * ( 2 - 1) + ( 0 - 1 ) * ( 0 - 1 ) + ( 1 - 0 ) * ( 1 - 0 ) );\n"
	"// %r = mSqrt( 3 );\n"
	"%r = VectorDist( %a, %b );\n"
   "@endtsexample\n\n"
   "@ingroup Vectors" )
{
   VectorF v = b - a;
   return v.len();
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( VectorLen, F32, ( VectorF v ),,
   "Calculate the magnitude of the given vector.\n"
   "@param v A vector.\n"
   "@return The length of vector @a v.\n\n"
   "@tsexample\n"
	"//-----------------------------------------------------------------------------\n"
	"//\n"
	"// VectorLen( %a );\n"
	"//\n"
	"// The length or magnitude of  vector a, (ax, ay, az), is:\n"
	"//\n"
	"//     ||a|| = Sqrt( ax * ax + ay * ay + az * az )\n"
	"//\n"
	"//-----------------------------------------------------------------------------\n\n"

	"%a = \"1 1 0\";\n\n"

	"// %r = mSqrt( 1 * 1 + 1 * 1 + 0 * 0 );\n"
	"// %r = mSqrt( 2 );\n"
	"// %r = 1.414;\n"
	"%r = VectorLen( %a );\n"
   "@endtsexample\n\n"
   "@ingroup Vectors" )
{
   return v.len();
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( VectorOrthoBasis, MatrixF, ( AngAxisF aa ),,
   "Create an orthogonal basis from the given vector.\n"
   "@param aaf The vector to create the orthogonal basis from.\n"
   "@return A matrix representing the orthogonal basis.\n"
   "@ingroup Vectors" )
{
   MatrixF mat;
   aa.setMatrix(&mat);
   return mat;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( VectorLerp, VectorF, ( VectorF a, VectorF b, F32 t ),,
   "Linearly interpolate between two vectors by @a t.\n"
   "@param a Vector to start interpolation from.\n"
   "@param b Vector to interpolate to.\n"
   "@param t Interpolation factor (0-1).  At zero, @a a is returned and at one, @a b is returned.  In between, an interpolated vector "
      "between @a a and @a b is returned.\n"
   "@return An interpolated vector between @a a and @a b.\n\n"
   "@tsexample\n"
	"//-----------------------------------------------------------------------------\n"
	"//\n"
	"// VectorLerp( %a, %b );\n"
	"//\n"
	"// The point between vector a, (ax, ay, az), and vector b, (bx, by, bz), which is\n"
	"// weighted by the interpolation factor, t, is\n"
	"//\n"
	"//     r = a + t * ( b - a )\n"
	"//       = ( ax + t * ( bx - ax ), ay + t * ( by - ay ), az + t * ( bz - az ) )\n"
	"//\n"
	"//-----------------------------------------------------------------------------\n\n"

	"%a = \"1 1 0\";\n"
	"%b = \"2 0 1\";\n"
	"%v = \"0.25\";\n\n"

	"// %r = \"( 1 + 0.25 * ( 2 - 1 ), 1 + 0.25 * ( 0 - 1 ), 0 + 0.25 * ( 1 - 0 ) )\";\n"
	"// %r = \"1.25 0.75 0.25\";\n"
	"%r = VectorLerp( %a, %b );\n"
   "@endtsexample\n\n"
   "@ingroup Vectors" )
{
   VectorF c;
   c.interpolate( a, b, t );
   
   return c;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( MatrixCreate, TransformF, ( VectorF position, AngAxisF orientation ),,
   "Create a transform from the given translation and orientation.\n"
   "@param position The translation vector for the transform.\n"
   "@param orientation The axis and rotation that orients the transform.\n"
   "@return A transform based on the given position and orientation.\n"
   "@ingroup Matrices" )
{
   TransformF transform( position, orientation );
   return transform;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( MatrixCreateFromEuler, TransformF, ( Point3F angles ),,
   "@Create a matrix from the given rotations.\n\n"
   "@param Vector3F X, Y, and Z rotation in *radians*.\n"
   "@return A transform based on the given orientation.\n"
   "@ingroup Matrices" )
{
   QuatF rotQ( angles );
   AngAxisF aa;
   aa.set(rotQ);

   return TransformF( Point3F::Zero, aa );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( MatrixMultiply, TransformF, ( TransformF left, TransformF right ),,
   "@brief Multiply the two matrices.\n\n"
   "@param left First transform.\n"
   "@param right Right transform.\n"
   "@return Concatenation of the two transforms.\n"
   "@ingroup Matrices" )
{
   MatrixF m1 = left.getMatrix();
   MatrixF m2 = right.getMatrix();

   m1.mul( m2 );

   return TransformF( m1 );
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( MatrixMulVector, VectorF, ( TransformF transform, VectorF vector ),,
   "@brief Multiply the vector by the transform assuming that w=0.\n\n"
   "This function will multiply the given vector by the given transform such that translation will "
   "not affect the vector.\n\n"
   "@param transform A transform.\n"
   "@param vector A vector.\n"
   "@return The transformed vector.\n"
   "@ingroup Matrices")
{
   MatrixF m = transform.getMatrix();
   m.mulV( vector );
   return vector;
}

//-----------------------------------------------------------------------------

DefineConsoleFunction( MatrixMulPoint, Point3F, ( TransformF transform, Point3F point ),,
   "@brief Multiply the given point by the given transform assuming that w=1.\n\n"
   "This function will multiply the given vector such that translation with take effect.\n"
   "@param transform A transform.\n"
   "@param point A vector.\n"
   "@return The transformed vector.\n"
   "@ingroup Matrices")
{
   MatrixF m = transform.getMatrix();
   m.mulP( point );
   return point;
}

ConsoleFunctionGroupEnd(MatrixMath);

//------------------------------------------------------------------------------

DefineConsoleFunction( getBoxCenter, Point3F, ( Box3F box ),,
   "Get the center point of an axis-aligned box.\n\n"
   "@param b A Box3F, in string format using \"minExtentX minExtentY minExtentZ maxExtentX maxExtentY maxExtentZ\"\n"
   "@return Center of the box.\n"
   "@ingroup Math")
{
   return box.getCenter();
}

//------------------------------------------------------------------------------

DefineEngineFunction( setRandomSeed, void, ( S32 seed ), ( -1 ),
   "Set the current seed for the random number generator.\n"
   "Based on this seed, a repeatable sequence of numbers will be produced by getRandom().\n"
   "@param seed The seed with which to initialize the randon number generator with.  The same seed will always leed to"
      "the same sequence of pseudo-random numbers.\n"
      "If -1, the current timestamp will be used as the seed which is a good basis for randomization.\n"
   "@ingroup Random" )
{
   if( seed == -1 )
      seed = Platform::getRealMilliseconds();

	MRandomLCG::setGlobalRandSeed( seed );
}

//------------------------------------------------------------------------------

DefineEngineFunction( getRandomSeed, S32, (),,
   "Get the current seed used by the random number generator.\n"
   "@return The current random number generator seed value.\n"
   "@ingroup Random" )
{
   return gRandGen.getSeed();
}

//------------------------------------------------------------------------------

S32 mRandI(S32 i1, S32 i2)
{
   return gRandGen.randI(i1, i2);
}

F32 mRandF(F32 f1, F32 f2)
{
   return gRandGen.randF(f1, f2);
}

F32 mRandF()
{
   return gRandGen.randF();
}

ConsoleFunction( getRandom, F32, 1, 3,
   "( int a, int b ) "
   "@brief Returns a random number based on parameters passed in..\n\n"
   "If no parameters are passed in, getRandom() will return a float between 0.0 and 1.0. If one "
   "parameter is passed an integer between 0 and the passed in value will be returned. Two parameters will "
   "return an integer between the specified numbers.\n\n"
   "@param a If this is the only parameter, a number between 0 and a is returned. Elsewise represents the lower bound.\n"
   "@param b Upper bound on the random number.  The random number will be <= @a b.\n"
   "@return A pseudo-random integer between @a a and @a b, between 0 and a, or a "
   "float between 0.0 and 1.1 depending on usage.\n\n"
   "@note All parameters are optional."
   "@see setRandomSeed\n"
   "@ingroup Random" )
{
   if (argc == 2)
      return F32(gRandGen.randI(0,getMax( dAtoi(argv[1]), 0 )));
   else
   {
      if (argc == 3) 
      {
         S32 min = dAtoi(argv[1]);
         S32 max = dAtoi(argv[2]);
         if (min > max) 
         {
            S32 t = min;
            min = max;
            max = t;
         }
         return F32(gRandGen.randI(min,max));
      }
   }
   return gRandGen.randF();
}

//------------------------------------------------------------------------------
