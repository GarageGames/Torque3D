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
#include "core/strings/stringUnit.h"
#include "console/consoleInternal.h"
#include "core/color.h"
#include "console/consoleTypes.h"
#include "math/mPoint2.h"
#include "math/mPoint3.h"
#include "math/mPoint4.h"
#include "math/mRect.h"
#include "math/mBox.h"
#include "math/mQuat.h"
#include "math/mAngAxis.h"
#include "math/mMatrix.h"
// Property system includes:
#include "console/propertyParsing.h"

extern ExprEvalState gEvalState;

namespace PropertyInfo
{
   //-----------------------------------------------------------------------------
   // Bool
   //-----------------------------------------------------------------------------
   bool default_scan(const String &data, bool & result)
   {
      result = dAtob(data.c_str());
      return true;
   }

   bool default_print(String & result, bool const & data)
   {
      if(data)
         result = String("1");
      else
         result = String("0");
      return true;
   }

   //-----------------------------------------------------------------------------
   // F32/U32/S32
   //-----------------------------------------------------------------------------
   bool default_scan(const String &data, F32 & result)
   {
      result = dAtof(data.c_str());
      return true;
   }

   bool default_print(String & result, F32 const & data)
   {
      result = String::ToString(data);
      return true;
   }

   bool default_scan(const String &data, U32 & result)
   {
      result = dAtoi(data.c_str());
      return true;
   }

   bool default_print(String & result, U32 const & data)
   {
      result = String::ToString(data);
      return true;
   }

   bool default_scan(const String &data, S32 & result)
   {
      result = dAtoi(data.c_str());
      return true;
   }

   bool default_print(String & result, S32 const & data)
   {
      result = String::ToString(data);
      return true;
   }

   //-----------------------------------------------------------------------------
   // Basic Vector Types
   //-----------------------------------------------------------------------------
   template <typename T>
   inline void default_vector_scan(const String &data, Vector<T> & result)
   {
      result.clear();
      for(S32 i = 0; i < StringUnit::getUnitCount(data, " \t\n"); i++)
         result.push_back(dAtof(StringUnit::getUnit(data, i, " \t\n")));
   }

   template <typename T>
   inline void default_vector_print(String & result, Vector<T> const & data)
   {
      result = String("");
      S32 items = data.size();
      for(S32 i = 0; i < items; i++)
      {
         result += String::ToString(data[i]);
         if(i < items-1)
            result += String(" ");
      }
   }

   bool default_scan(const String &data, Vector<F32> & result)
   {
      default_vector_scan(data,result);
      return true;
   }

   bool default_print(String & result, Vector<F32> const & data)
   {
      default_vector_print<F32>(result,data);
      return true;
   }

   bool default_scan(const String &data, Vector<U32> & result)
   {
      default_vector_scan(data,result);
      return true;
   }

   bool default_print(String & result, Vector<U32> const & data)
   {
      default_vector_print<U32>(result,data);
      return true;
   }

   bool default_scan(const String &data, Vector<S32> & result)
   {
      default_vector_scan(data,result);
      return true;
   }

   bool default_print(String & result, Vector<S32> const & data)
   {
      default_vector_print<S32>(result,data);
      return true;
   }

   //-----------------------------------------------------------------------------
   // Math - Points
   //-----------------------------------------------------------------------------
   bool default_scan(const String &data, Point2F & result)
   {
      dSscanf(data.c_str(),"%g %g",&result.x,&result.y);
      return true;
   }

   bool default_print(String & result, Point2F const & data)
   {
      result = String::ToString("%g %g",data.x,data.y);
      return true;
   }

   bool default_scan(const String &data, Point2I & result)
   {
      // Handle passed as floating point from script
      if(data.find('.') != String::NPos)
      {
         Point2F tempResult;
         dSscanf(data.c_str(),"%f %f",&tempResult.x,&tempResult.y);
         result.x = mFloor(tempResult.x);
         result.y = mFloor(tempResult.y);
      }
      else
         dSscanf(data.c_str(),"%d %d",&result.x,&result.y);
      return true;
   }

   bool default_print(String & result, Point2I const & data)
   {
      result = String::ToString("%d %d",data.x,data.y);
      return true;
   }

   bool default_scan(const String &data, Point3F & result)
   {
      dSscanf(data.c_str(),"%g %g %g",&result.x,&result.y,&result.z);
      return true;
   }

   bool default_print(String & result, Point3F const & data)
   {
      result = String::ToString("%g %g %g",data.x,data.y,data.z);
      return true;
   }

   bool default_scan(const String &data, Point3I & result)
   {
      // Handle passed as floating point from script
      if(data.find('.') != String::NPos)
      {
         Point3F tempResult;
         dSscanf(data.c_str(),"%f %f %f",&tempResult.x,&tempResult.y,&tempResult.z);
         result.x = mFloor(tempResult.x);
         result.y = mFloor(tempResult.y);
         result.z = mFloor(tempResult.z);
      }
      else
         dSscanf(data.c_str(),"%d %d %d",&result.x,&result.y,&result.z);
      return true;
   }

   bool default_print(String & result, Point3I const & data)
   {
      result = String::ToString("%d %d %d",data.x,data.y,data.z);
      return true;
   }

   bool default_scan(const String &data, Point4F & result)
   {
      dSscanf(data.c_str(),"%g %g %g %g",&result.x,&result.y,&result.z,&result.w);
      return true;
   }

   bool default_print(String & result, Point4F const & data)
   {
      result = String::ToString("%g %g %g %g",data.x,data.y,data.z,data.w);
      return true;
   }

   bool default_scan(const String &data, Point4I & result)
   {
      // Handle passed as floating point from script
      if(data.find('.') != String::NPos)
      {
         Point4F tempResult;
         dSscanf(data.c_str(),"%f %f %f %f",&tempResult.x,&tempResult.y,&tempResult.z,&tempResult.w);
         result.x = mFloor(tempResult.x);
         result.y = mFloor(tempResult.y);
         result.z = mFloor(tempResult.z);
         result.w = mFloor(tempResult.w);
      }
      else
         dSscanf(data.c_str(),"%d %d %d %d",&result.x,&result.y,&result.z,&result.w);
      return true;      
   }

   bool default_print(String & result, const Point4I & data)
   {
      result = String::ToString("%d %d %d %d", data.x, data.y, data.z, data.w);
      return true;
   }

   //-----------------------------------------------------------------------------
   // Math - Rectangles and boxes
   //-----------------------------------------------------------------------------
   bool default_scan( const String &data, RectI & result )
   {
      // Handle passed as floating point from script
      if(data.find('.') != String::NPos)
      {
         RectF tempResult;
         dSscanf(data.c_str(),"%f %f %f %f",&tempResult.point.x,&tempResult.point.y,&tempResult.extent.x,&tempResult.extent.y);
         result.point.x = mFloor(tempResult.point.x);
         result.point.y = mFloor(tempResult.point.y);
         result.extent.x = mFloor(tempResult.extent.x);
         result.extent.y = mFloor(tempResult.extent.y);
      }
      else
         dSscanf(data.c_str(),"%d %d %d %d",&result.point.x,&result.point.y,&result.extent.x,&result.extent.y);
      return true;
   }
   bool default_print( String & result, const RectI & data )
   {
      result = String::ToString("%i %i %i %i",data.point.x,data.point.y,data.extent.x,data.extent.y);
      return true;
   }

   bool default_scan(const String &data, RectF & result)
   {
      dSscanf(data.c_str(),"%g %g %g %g",&result.point.x,&result.point.y,&result.extent.x,&result.extent.y);
      return true;
   }

   bool default_print(String & result, const RectF & data)
   {
      result = String::ToString("%g %g %g %g",data.point.x,data.point.y,data.extent.x,data.extent.y);
      return true;
   }

   bool default_scan(const String &data, Box3F & result)
   {
      dSscanf(data.c_str(),"%g %g %g %g %g %g",
         &result.minExtents.x,&result.minExtents.y,&result.minExtents.z,
         &result.maxExtents.x,&result.maxExtents.y,&result.maxExtents.z);
      return true;
   }

   bool default_print(String & result, const Box3F & data)
   {
      result = String::ToString("%g %g %g %g %g %g",
         data.minExtents.x,data.minExtents.y,data.minExtents.z,
         data.maxExtents.x,data.maxExtents.y,data.maxExtents.z);
      return true;
   }

   //-----------------------------------------------------------------------------

   bool default_scan( const String &data, AngAxisF & result )
   {
      if(StringUnit::getUnitCount(data," ") < 4)
         return false;

      dSscanf(data.c_str(),"%g %g %g %g", &result.axis.x,&result.axis.y,&result.axis.z,&result.angle);
      result.angle = mDegToRad(result.angle);
      return true;
   }

   bool default_print( String & result, const AngAxisF & data )
   {
      F32 angle = mRadToDeg(data.angle);
      angle = mFmod(angle + 360.0f,360.0f);
      result = String::ToString("%g %g %g %g", data.axis.x, data.axis.y, data.axis.z, angle);
      return true;
   }

   bool default_scan( const String &data, QuatF & result )
   {
      if(StringUnit::getUnitCount(data," ") < 4)
         return false;

      dSscanf(data.c_str(),"%g %g %g %g", &result.x,&result.y,&result.z,&result.w);
      return true;
   }

   bool default_print( String & result, const QuatF & data )
   {
      result = String::ToString("%g %g %g %g", data.x, data.y, data.z, data.w);
      return true;
   }

   bool default_scan( const String &data, MatrixF & result )
   {
      if(StringUnit::getUnitCount(data," ") < 16)
         return false;

      F32* m = result;
      dSscanf(data.c_str(),"%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g", 
         &m[result.idx(0,0)], &m[result.idx(0,1)], &m[result.idx(0,2)], &m[result.idx(0,3)], 
         &m[result.idx(1,0)], &m[result.idx(1,1)], &m[result.idx(1,2)], &m[result.idx(1,3)], 
         &m[result.idx(2,0)], &m[result.idx(2,1)], &m[result.idx(2,2)], &m[result.idx(2,3)], 
         &m[result.idx(3,0)], &m[result.idx(3,1)], &m[result.idx(3,2)], &m[result.idx(3,3)]);         
      return true;
   }

   bool default_print( String & result, const MatrixF & data )
   {
      const F32* m = data;
      result = String::ToString("%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
         m[data.idx(0,0)], m[data.idx(0,1)], m[data.idx(0,2)], m[data.idx(0,3)], 
         m[data.idx(1,0)], m[data.idx(1,1)], m[data.idx(1,2)], m[data.idx(1,3)], 
         m[data.idx(2,0)], m[data.idx(2,1)], m[data.idx(2,2)], m[data.idx(2,3)], 
         m[data.idx(3,0)], m[data.idx(3,1)], m[data.idx(3,2)], m[data.idx(3,3)]);
      return true;
   }

   //-----------------------------------------------------------------------------
   // Colors
   //-----------------------------------------------------------------------------
   bool default_scan(const String &data, ColorF & result)
   {
      if(StringUnit::getUnitCount(data," ") == 3)
      {
         dSscanf(data.c_str(),"%g %g %g",&result.red,&result.green,&result.blue);
         result.alpha = 1.0f;
      }
      else
         dSscanf(data.c_str(),"%g %g %g %g",&result.red,&result.green,&result.blue,&result.alpha);
      return true;
   }

   bool default_print(String & result, ColorF const & data)
   {
      if(data.alpha == 1.0f)
         result = String::ToString("%g %g %g",data.red,data.green,data.blue);
      else
         result = String::ToString("%g %g %g %g",data.red,data.green,data.blue,data.alpha);
      return true;
   }

   bool default_scan(const String &data, ColorI & result)
   {
      if(StringUnit::getUnitCount(data," ") == 3)
      {
         S32 r,g,b;
         dSscanf(data.c_str(),"%i %i %i",&r,&g,&b);
         result.set(r,g,b);
      }
      else
      {
         S32 r,g,b,a;
         dSscanf(data.c_str(),"%i %i %i %i",&r,&g,&b,&a);
         result.set(r,g,b,a);
      }
      return true;
   }

   bool default_print(String & result, const ColorI & data)
   {
      if(data.alpha == 255)
         result = String::ToString("%d %d %d",data.red,data.green,data.blue);
      else
         result = String::ToString("%d %d %d %d",data.red,data.green,data.blue,data.alpha);
      return true;
   }

   //-----------------------------------------------------------------------------
   // String
   //-----------------------------------------------------------------------------
   bool default_scan(const String &data, String & result)
   {
      result = data;
      return true;
   }

   bool default_print(String & result, const String & data)
   {
      result = data;
      return true;
   }

   //-----------------------------------------------------------------------------
   // FileName
   //-----------------------------------------------------------------------------
   bool default_scan(const String &data, FileName & result)
   {
      char buffer[1024];

      if(data.c_str()[0] == '$')
      {
         dStrncpy(buffer, data.c_str(), sizeof(buffer) - 1);
         buffer[sizeof(buffer)-1] = 0;
      }
      else if (!Con::expandScriptFilename(buffer, sizeof(buffer), data))
      {
         Con::warnf("(TypeFilename) illegal filename detected: %s", data.c_str());
         return false;
      }
      result = String(buffer);
      return true;
   }

   bool default_print(String & result, const FileName & data)
   {
      result = data;
      return true;
   }

   //-----------------------------------------------------------------------------
   // SimObject
   //-----------------------------------------------------------------------------
   bool default_scan(const String &data, SimObject * & result)
   {
      result = Sim::findObject(data);
      return result != NULL;
   }

   bool default_print(String & result, SimObject * const & data)
   {
      if(data)
      {
         if(String(data->getName()).isEmpty())
            result = data->getIdString();
         else
            result = data->getName();
         return true;
      }
      return false;
   }

   //-----------------------------------------------------------------------------
   // Print scan ints of various sizes as hex
   //-----------------------------------------------------------------------------

   //-------
   // 16 bit
   //-------

   bool hex_scan(const String & string, U32 & hex)
   {
      dSscanf(string.c_str(),"%i", &hex);
      return true;
   }

   bool hex_print(String & string, const U32 & hex)
   {
      string = String::ToString("0x%X",hex);
      return true;
   }

   bool hex_scan(const String & string, S32 & hex)
   {
      dSscanf(string.c_str(),"%i", &hex);
      return true;
   }

   bool hex_print(String & string, const S32 & hex)
   {
      string = String::ToString("0x%X",hex);
      return true;
   }

   //-------
   // 16 bit
   //-------

   bool hex_scan(const String & string, U16 & hex)
   {
      U32 tmp;
      bool ret = hex_scan(string,tmp);
      hex = tmp;
      return ret;
   }

   bool hex_print(String & string, const U16 & hex)
   {
      U32 tmp = hex;
      return hex_print(string,tmp);
   }

   bool hex_scan(const String & string, S16 & hex)
   {
      S32 tmp;
      bool ret = hex_scan(string,tmp);
      hex = tmp;
      return ret;
   }

   bool hex_print(String & string, const S16 & hex)
   {
      U32 tmp = hex;
      return hex_print(string,tmp);
   }

   //-------
   //  8 bit
   //-------

   bool hex_scan(const String & string, U8 & hex)
   {
      U32 tmp;
      bool ret = hex_scan(string,tmp);
      hex = tmp;
      return ret;
   }

   bool hex_print(String & string, const U8 & hex)
   {
      U32 tmp = hex;
      return hex_print(string,tmp);
   }

   bool hex_scan(const String & string, S8 & hex)
   {
      S32 tmp;
      bool ret = hex_scan(string,tmp);
      hex = tmp;
      return ret;
   }

   bool hex_print(String & string, const S8 & hex)
   {
      U32 tmp = hex;
      return hex_print(string,tmp);
   }

}
