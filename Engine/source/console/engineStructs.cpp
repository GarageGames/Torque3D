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
#include "math/mPolyhedron.h"

IMPLEMENT_STRUCT(PlaneF,
   PlaneF, ,
   "")

   FIELD(x, x, 1, "")
   FIELD(y, y, 1, "")
   FIELD(z, z, 1, "")
   FIELD(d, d, 1, "")

END_IMPLEMENT_STRUCT;

IMPLEMENT_STRUCT( PolyhedronData::Edge,
   Edge,,
   "" )

   FIELD_AS(U32, face, face, 2, "")
   FIELD_AS(U32, vertex, vertex, 2, "")

END_IMPLEMENT_STRUCT;


IMPLEMENT_STRUCT(Torque::UUID,
   UUID, ,
   "")

   Torque::UUIDEngineExport::getAField(),
   Torque::UUIDEngineExport::getBField(),
   Torque::UUIDEngineExport::getCField(),
   Torque::UUIDEngineExport::getDField(),
   Torque::UUIDEngineExport::getEField(),
   Torque::UUIDEngineExport::getFField(),

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

// Vectors
IMPLEMENT_STRUCT( Vector< bool >,
   BoolVector,,
   "" )

   VectorFieldEngineExport::getElementCountField< bool >(),
   VectorFieldEngineExport::getArraySizeField< bool >(),
   VectorFieldEngineExport::getArrayField< bool >(),

END_IMPLEMENT_STRUCT;

IMPLEMENT_STRUCT( Vector< S32 >,
   IntVector,,
   "" )

   VectorFieldEngineExport::getElementCountField< S32 >(),
   VectorFieldEngineExport::getArraySizeField< S32 >(),
   VectorFieldEngineExport::getArrayField< S32 >(),

END_IMPLEMENT_STRUCT;

IMPLEMENT_STRUCT( Vector< F32 >,
   FloatVector,,
   "" )

   VectorFieldEngineExport::getElementCountField< F32 >(),
   VectorFieldEngineExport::getArraySizeField< F32 >(),
   VectorFieldEngineExport::getArrayField< F32 >(),

END_IMPLEMENT_STRUCT;

IMPLEMENT_STRUCT( Vector< Point3F >,
   Point3FVector,,
   "" )

   VectorFieldEngineExport::getElementCountField< Point3F >(),
   VectorFieldEngineExport::getArraySizeField< Point3F >(),
   VectorFieldEngineExport::getArrayField< Point3F >(),

END_IMPLEMENT_STRUCT;

IMPLEMENT_STRUCT(Vector< PlaneF >,
   PlaneFVector, ,
   "")

   VectorFieldEngineExport::getElementCountField< PlaneF >(),
   VectorFieldEngineExport::getArraySizeField< PlaneF >(),
   VectorFieldEngineExport::getArrayField< PlaneF >(),

END_IMPLEMENT_STRUCT;

IMPLEMENT_STRUCT(Vector< PolyhedronData::Edge >,
   EdgeVector, ,
   "")

   VectorFieldEngineExport::getElementCountField< PolyhedronData::Edge >(),
   VectorFieldEngineExport::getArraySizeField< PolyhedronData::Edge >(),
   VectorFieldEngineExport::getArrayField< PolyhedronData::Edge >(),

END_IMPLEMENT_STRUCT;

IMPLEMENT_STRUCT(Vector< const char* >,
   StringVector, ,
   "")

   VectorFieldEngineExport::getElementCountField< const char* >(),
   VectorFieldEngineExport::getArraySizeField< const char* >(),
   VectorFieldEngineExport::getArrayField< const char* >(),

END_IMPLEMENT_STRUCT;
