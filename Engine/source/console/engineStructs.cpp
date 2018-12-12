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

#include "console/engineStructs.h"
#include "console/engineAPI.h"
#include "core/util/tVector.h"
#include "core/util/uuid.h"
#include "core/color.h"

// Compute the offsets for vectors
const U32 Vector<bool>::offset[] = { Offset(mElementCount, Vector<bool>), Offset(mArraySize, Vector<bool>), Offset(mArray, Vector<bool>)};
const U32 Vector<S32>::offset[] = { Offset(mElementCount, Vector<S32>), Offset(mArraySize, Vector<S32>), Offset(mArray, Vector<S32>)};
const U32 Vector<F32>::offset[] = { Offset(mElementCount, Vector<F32>), Offset(mArraySize, Vector<F32>), Offset(mArray, Vector<F32>)};
const U32 Vector<Point3F>::offset[] = { Offset(mElementCount, Vector<Point3F>), Offset(mArraySize, Vector<Point3F>), Offset(mArray, Vector<Point3F>)};
const U32 Vector<PlaneF>::offset[] = { Offset(mElementCount, Vector<PlaneF>), Offset(mArraySize, Vector<PlaneF>), Offset(mArray, Vector<PlaneF>)};
const U32 Vector<PolyhedronData::Edge>::offset[] = { Offset(mElementCount, Vector<PolyhedronData::Edge>), Offset(mArraySize, Vector<PolyhedronData::Edge>), Offset(mArray, Vector<PolyhedronData::Edge>)};
const U32 Vector<const char*>::offset[] = { Offset(mElementCount, Vector<const char*>), Offset(mArraySize, Vector<const char*>), Offset(mArray, Vector<const char*>)};

IMPLEMENT_STRUCT( Vector< bool >,
   BoolVector,,
   "" )
   {"elementCount", "", 1, TYPE<U32>(), Vector<bool>::offset[0]},
   {"arraySize", "", 1, TYPE<U32>(), Vector<bool>::offset[1]},
   {"array", "", 1, TYPE<bool*>(), Vector<bool>::offset[2]},
END_IMPLEMENT_STRUCT;


IMPLEMENT_STRUCT( Vector< S32 >,
   IntVector,,
   "" )
   {"elementCount", "", 1, TYPE<U32>(), Vector<S32>::offset[0]},
   {"arraySize", "", 1, TYPE<U32>(), Vector<S32>::offset[1]},
   {"array", "", 1, TYPE<S32*>(), Vector<S32>::offset[2]},
END_IMPLEMENT_STRUCT;

IMPLEMENT_STRUCT( Vector< F32 >,
   FloatVector,,
   "" )
   {"elementCount", "", 1, TYPE<U32>(), Vector<F32>::offset[0]},
   {"arraySize", "", 1, TYPE<U32>(), Vector<F32>::offset[1]},
   {"array", "", 1, TYPE<F32*>(), Vector<F32>::offset[2]},
END_IMPLEMENT_STRUCT;

IMPLEMENT_STRUCT( Vector< Point3F >,
   Point3FVector,,
   "" )
   {"elementCount", "", 1, TYPE<U32>(), Vector<Point3F>::offset[0]},
   {"arraySize", "", 1, TYPE<U32>(), Vector<Point3F>::offset[1]},
   {"array", "", 1, TYPE<Point3F*>(), Vector<Point3F>::offset[2]},
END_IMPLEMENT_STRUCT;

IMPLEMENT_STRUCT(PlaneF,
   PlaneF, ,
   "")
   FIELD(x, x, 1, "")
   FIELD(y, y, 1, "")
   FIELD(z, z, 1, "")
   FIELD(d, d, 1, "")
   END_IMPLEMENT_STRUCT;

IMPLEMENT_STRUCT( Vector< PlaneF >,
   PlaneFVector,,
   "" )
   {"elementCount", "", 1, TYPE<U32>(), Vector<PlaneF>::offset[0]},
   {"arraySize", "", 1, TYPE<U32>(), Vector<PlaneF>::offset[1]},
   {"array", "", 1, TYPE<PlaneF*>(), Vector<PlaneF>::offset[2]},
END_IMPLEMENT_STRUCT;

IMPLEMENT_STRUCT( PolyhedronData::Edge,
   Edge,,
   "" )
   { "face", "", 2, TYPE<U32>(), (U32)FIELDOFFSET( face ) },
   { "vertex", "", 2, TYPE<U32>(), (U32)FIELDOFFSET( vertex ) },
END_IMPLEMENT_STRUCT;

IMPLEMENT_STRUCT( Vector< PolyhedronData::Edge >,
   EdgeVector,,
   "" )
   {"elementCount", "", 1, TYPE<U32>(), Vector<PolyhedronData::Edge>::offset[0]},
   {"arraySize", "", 1, TYPE<U32>(), Vector<PolyhedronData::Edge>::offset[1]},
   {"array", "", 1, TYPE<PolyhedronData::Edge*>(), Vector<PolyhedronData::Edge>::offset[2]},
END_IMPLEMENT_STRUCT;


IMPLEMENT_STRUCT( Vector< const char* >,
   StringVector,,
   "" )
   {"elementCount", "", 1, TYPE<U32>(), Vector<const char*>::offset[0]},
   {"arraySize", "", 1, TYPE<U32>(), Vector<const char*>::offset[1]},
   {"array", "", 1, TYPE<const char**>(), Vector<const char*>::offset[2]},
END_IMPLEMENT_STRUCT;


// Compute the offsets for UUID
const U32 Torque::UUID::offset[] = { 
   Offset(a, Torque::UUID), 
   Offset(b, Torque::UUID), 
   Offset(c, Torque::UUID), 
   Offset(d, Torque::UUID), 
   Offset(e, Torque::UUID), 
   Offset(f, Torque::UUID) 
};

IMPLEMENT_STRUCT( Torque::UUID,
   UUID,,
   "" )
   {"a", "", 1, TYPE<U32>(), Torque::UUID::offset[0]},
   {"b", "", 1, TYPE<U16>(), Torque::UUID::offset[1]},
   {"c", "", 1, TYPE<U16>(), Torque::UUID::offset[2]},
   {"d", "", 1, TYPE<U8>(), Torque::UUID::offset[3]},
   {"e", "", 1, TYPE<U8>(), Torque::UUID::offset[4]},
   {"f", "", 6, TYPE<U8>(), Torque::UUID::offset[5]},
END_IMPLEMENT_STRUCT;


IMPLEMENT_STRUCT( ColorI,
   ColorI,,
   "RGBA color quadruple in 8bit integer precision per channel." )
   
   FIELD( red, red, 1, "Red channel value." )
   FIELD( green, green, 1, "Green channel value." )
   FIELD( blue, blue, 1, "Blue channel value." )
   FIELD( alpha, alpha, 1, "Alpha channel value." )
   
END_IMPLEMENT_STRUCT;


IMPLEMENT_STRUCT( LinearColorF,
   LinearColorF,,
   "RGBA color quadruple in 32bit floating-point precision per channel." )

   FIELD( red, red, 1, "Red channel value." )
   FIELD( green, green, 1, "Green channel value." )
   FIELD( blue, blue, 1, "Blue channel value." )
   FIELD( alpha, alpha, 1, "Alpha channel value." )

END_IMPLEMENT_STRUCT;
