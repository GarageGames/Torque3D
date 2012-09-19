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

#ifndef _PROPERTYPARSING_H_
#define _PROPERTYPARSING_H_

class ColorI;
class ColorF;
class Point2I;
class Point2F;
class Point3F;
class Point4F;
class Point3I;
class Point4I;
class RectI;
class RectF;
class Box3I;
class Box3F;
class MatrixF;
class AngAxisF;
class QuatF;
class String;
class FileName;
class SimObject;
class SimObjectType;
template<class T> class Vector;

//-----------------------------------------------------------------------------
// String scan/print methods
//
// Macros in propertyImplementation.h point the ConcretePropertyDefinition scan/print delegates to these functions.
//
namespace PropertyInfo
{
   // String
   bool default_scan(const String &data, String & result);
   bool default_print(String & result, const String & data);

   // Bool
   bool default_scan(const String &data, bool & result);
   bool default_print(String & result, const bool & data);

   // S32/F32/U32
   bool default_scan(const String &data, F32 & result);
   bool default_print(String & result, const F32 & data);
   bool default_scan(const String &data, U32 & result);
   bool default_print(String & result, const U32 & data);
   bool default_scan(const String &data, S32 & result);
   bool default_print(String & result, const S32 & data);

   // Vector<S32/F32/U32>
   bool default_scan(const String &data, Vector<F32> & result);
   bool default_print(String & result, const Vector<F32> & data);
   bool default_scan(const String &data, Vector<U32> & result);
   bool default_print(String & result, const Vector<U32> & data);
   bool default_scan(const String &data, Vector<S32> & result);
   bool default_print(String & result, const Vector<S32> & data);

   // Math Points
   bool default_scan(const String &data, Point2F & result);
   bool default_print(String & result, const Point2F & data);
   bool default_scan(const String &data, Point2I & result);
   bool default_print(String & result, const Point2I & data);
   bool default_scan(const String &data, Point3F & result);
   bool default_print(String & result, const Point3F & data);
   bool default_scan(const String &data, Point3I & result);
   bool default_print(String & result, const Point3I & data);
   bool default_scan(const String &data, Point4F & result);
   bool default_print(String & result, const Point4F & data);
   bool default_scan(const String &data, Point4I & result);
   bool default_print(String & result, const Point4I & data);

   // Math Boxs & Rectangles
   bool default_scan(const String &data, RectI & result);
   bool default_print(String & result, const RectI & data);
   bool default_scan(const String &data, RectF & result);
   bool default_print(String & result, const RectF & data);
   bool default_scan(const String &data, Box3I & result);
   bool default_print(String & result, const Box3I & data);
   bool default_scan(const String &data, Box3F & result);
   bool default_print(String & result, const Box3F & data);

   //-----------------------------------------------------------------------------
   bool default_scan( const String &data, AngAxisF & result );
   bool default_print( String & result, const AngAxisF & data );

   bool default_scan( const String &data, QuatF & result );
   bool default_print( String & result, const QuatF & data );

   bool default_scan( const String &data, MatrixF & result );
   bool default_print( String & result, const MatrixF & data );

   // Colors
   bool default_scan(const String &data, ColorF & result);
   bool default_print(String & result, const ColorF & data);
   bool default_scan(const String &data, ColorI & result);
   bool default_print(String & result, const ColorI & data);

   // filename handler
   bool default_scan(const String &data, FileName & result);
   bool default_print(String & result, const FileName & data);

   // SimObjectType
   bool default_scan(const String &data, SimObjectType & result);
   bool default_print(String & result, SimObjectType const & data);

   // SimObject
   bool default_scan(const String &data, SimObject * & result);
   bool default_print(String & result, SimObject * const & data);

   template<class T>
   inline bool typed_simobject_scan(const String &data, T * & result)
   {
      SimObject * obj;
      result = default_scan(data,obj)? dynamic_cast<T*>(obj) : NULL;
      return result;
   }

   template<class T>
   inline bool typed_simobject_print(String & result, T * const & data)
   {
      return default_print(result,data);
   }

   bool hex_scan(const String & string, U32 & hex);
   bool hex_print(String & string, const U32 & hex);
   bool hex_scan(const String & string, S32 & hex);
   bool hex_print(String & string, const S32 & hex);

   bool hex_scan(const String & string, U16 & hex);
   bool hex_print(String & string, const U16 & hex);
   bool hex_scan(const String & string, S16 & hex);
   bool hex_print(String & string, const S16 & hex);

   bool hex_scan(const String & string, U8 & hex);
   bool hex_print(String & string, const U8 & hex);
   bool hex_scan(const String & string, S8 & hex);
   bool hex_print(String & string, const S8 & hex);

   bool default_print(String & result, SimObjectType * const & data);
}

// Default Scan/print definition
#define DEFINE_PROPERTY_DEFAULT_PRINT(dataType) bool PropertyInfo::default_print(String & resultString, dataType const & dataTyped)
#define DEFINE_PROPERTY_DEFAULT_SCAN(dataType) bool PropertyInfo::default_scan(const String &dataString, dataType & resultTyped)

#endif // _PROPERTYPARSING_H_
