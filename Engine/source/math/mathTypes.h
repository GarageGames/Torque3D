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

#ifndef _MATHTYPES_H_
#define _MATHTYPES_H_

#ifndef _DYNAMIC_CONSOLETYPES_H_
   #include "console/dynamicTypes.h"
#endif


void RegisterMathFunctions(void);


class Point2I;
class Point2F;
class Point3I;
class Point3F;
class Point4F;
class RectI;
class RectF;
class MatrixF;
class Box3F;
class EaseF;
class AngAxisF;
class TransformF;
class RotationF;

DECLARE_SCOPE( MathTypes );


DECLARE_STRUCT( Point2I );
DECLARE_STRUCT( Point2F );
DECLARE_STRUCT( Point3I );
DECLARE_STRUCT( Point3F );
DECLARE_STRUCT( Point4F );
DECLARE_STRUCT( RectI );
DECLARE_STRUCT( RectF );
DECLARE_STRUCT( MatrixF );
DECLARE_STRUCT( AngAxisF );
DECLARE_STRUCT( TransformF );
DECLARE_STRUCT( Box3F );
DECLARE_STRUCT( EaseF );
DECLARE_STRUCT(RotationF);


// Legacy console types.
DefineConsoleType( TypePoint2I, Point2I )
DefineConsoleType( TypePoint2F, Point2F )
DefineConsoleType( TypePoint3I, Point3I )
DefineConsoleType( TypePoint3F, Point3F )
DefineConsoleType( TypePoint4F, Point4F )
DefineConsoleType( TypeRectI, RectI )
DefineConsoleType( TypeRectF, RectF )
DefineConsoleType( TypeMatrixF, MatrixF )
DefineConsoleType( TypeMatrixPosition, MatrixF)
DefineConsoleType( TypeMatrixRotation, MatrixF )
DefineConsoleType( TypeAngAxisF, AngAxisF )
DefineConsoleType( TypeTransformF, TransformF )
DefineConsoleType( TypeBox3F, Box3F )
DefineConsoleType( TypeEaseF, EaseF )
DefineConsoleType(TypeRotationF, RotationF)

#endif
