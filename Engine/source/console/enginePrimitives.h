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

#ifndef _ENGINEPRIMITIVES_H_
#define _ENGINEPRIMITIVES_H_

#ifndef _ENGINETYPES_H_
   #include "console/engineTypes.h"
#endif

#include "math/mPlane.h"
#include "math/mPolyhedron.h"

/// @file
/// Definitions for the core primitive types used in the
/// exposed engine API.



DECLARE_PRIMITIVE_R( bool );
DECLARE_PRIMITIVE_R(S8);
DECLARE_PRIMITIVE_R(U8);
DECLARE_PRIMITIVE_R(S16);
DECLARE_PRIMITIVE_R(U16);
DECLARE_PRIMITIVE_R(S32);
DECLARE_PRIMITIVE_R(U32);
DECLARE_PRIMITIVE_R(F32);
DECLARE_PRIMITIVE_R(F64);
DECLARE_PRIMITIVE_R(U64);
DECLARE_PRIMITIVE_R(S64);
DECLARE_PRIMITIVE_R(void*);

DECLARE_PRIMITIVE_R(bool*);
DECLARE_PRIMITIVE_R(U8*);
DECLARE_PRIMITIVE_R(S32*);
DECLARE_PRIMITIVE_R(U32*);
DECLARE_PRIMITIVE_R(F32*);
DECLARE_PRIMITIVE_R(Point3F*);
DECLARE_PRIMITIVE_R(PlaneF*);
DECLARE_PRIMITIVE_R(PolyhedronData::Edge*);
DECLARE_PRIMITIVE_R(const char**);

//FIXME: this allows String to be used as a struct field type

// String is special in the way its data is exchanged through the API.  Through
// calls, strings are passed as plain, null-terminated UTF-8 character strings.
// In addition, strings passed back as return values from engine API functions
// are considered to be owned by the API layer itself.  The rule here is that such
// a string is only valid until the next API call is made.  Usually, control layers
// will immediately copy and convert strings to their own string type.
_DECLARE_TYPE_R(String);
template<>
struct EngineTypeTraits< String > : public _EnginePrimitiveTypeTraits< String >
{
   typedef const UTF8* ArgumentValueType;
   typedef const UTF8* ReturnValueType;

   //FIXME: this needs to be sorted out; for now, we store default value literals in ASCII
   typedef const char* DefaultArgumentValueStoreType;
   
   static const UTF8* ReturnValue( const String& str )
   {
      static String sTemp;      
      sTemp = str;
      return sTemp.utf8();
   }
};

// For struct fields, String cannot be used directly but "const UTF16*" must be used
// instead.  Make sure this works with the template machinery by redirecting the type
// back to String.
template<> struct EngineTypeTraits< const UTF16* > : public EngineTypeTraits< String > {};
template<> inline const EngineTypeInfo* TYPE< const UTF16* >() { return TYPE< String >(); }
inline const EngineTypeInfo* TYPE( const UTF16*& ) { return TYPE< String >(); }

// Temp support for allowing const char* to remain in the API functions as long as we
// still have the console system around.  When that is purged, these definitions should
// be deleted and all const char* uses be replaced with String.
_DECLARE_TYPE_R(const UTF8*);
template<>
struct EngineTypeTraits< const UTF8* > : public _EnginePrimitiveTypeTraits< const UTF8* >
{
   static const UTF8* ReturnValue(const String& str)
   {
      static String sTemp;
      sTemp = str;
      return sTemp.utf8();
   }
};


#endif // !_ENGINEPRIMITIVES_H_
