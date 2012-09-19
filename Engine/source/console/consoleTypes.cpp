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

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/simPersistID.h"
#include "core/stringTable.h"
#include "core/util/str.h"
#include "core/util/uuid.h"
#include "core/color.h"
#include "console/simBase.h"
#include "math/mRect.h"

//-----------------------------------------------------------------------------
// TypeString
//-----------------------------------------------------------------------------
ConsoleType( string, TypeString, const char* )
ImplementConsoleTypeCasters( TypeString, const char* );

ConsoleGetType( TypeString )
{
   return *((const char **)(dptr));
}

ConsoleSetType( TypeString )
{
   if(argc == 1)
      *((const char **) dptr) = StringTable->insert(argv[0]);
   else
      Con::printf("(TypeString) Cannot set multiple args to a single string.");
}

//-----------------------------------------------------------------------------
// TypeCaseString
//-----------------------------------------------------------------------------
ConsoleType( caseString, TypeCaseString, const char* )

ConsoleSetType( TypeCaseString )
{
   if(argc == 1)
      *((const char **) dptr) = StringTable->insert(argv[0], true);
   else
      Con::printf("(TypeCaseString) Cannot set multiple args to a single string.");
}

ConsoleGetType( TypeCaseString )
{
   return *((const char **)(dptr));
}

//-----------------------------------------------------------------------------
// TypeRealString
//-----------------------------------------------------------------------------
ConsoleType( string, TypeRealString, String )
ImplementConsoleTypeCasters( TypeRealString, String )

ConsoleGetType( TypeRealString )
{
   const String *theString = static_cast<const String*>(dptr);

   return theString->c_str();
}

ConsoleSetType( TypeRealString )
{
   String *theString = static_cast<String*>(dptr);

   if(argc == 1)
      *theString = argv[0];
   else
      Con::printf("(TypeRealString) Cannot set multiple args to a single string.");
}

//-----------------------------------------------------------------------------
// TypeCommand
//-----------------------------------------------------------------------------
ConsoleType( string, TypeCommand, String )

ConsoleGetType( TypeCommand )
{
   const String *theString = static_cast<const String*>(dptr);

   return theString->c_str();
}

ConsoleSetType( TypeCommand )
{
   String *theString = static_cast<String*>(dptr);

   if(argc == 1)
      *theString = argv[0];
   else
      Con::printf("(TypeCommand) Cannot set multiple args to a single command.");
}

//-----------------------------------------------------------------------------
// TypeFileName
//-----------------------------------------------------------------------------
ConsolePrepType( filename, TypeFilename, const char * )

ConsoleSetType( TypeFilename )
{
   if(argc == 1)
   {
      char buffer[1024];
      if(argv[0][0] == '$')
      {
         dStrncpy(buffer, argv[0], sizeof(buffer) - 1);
         buffer[sizeof(buffer)-1] = 0;
      }
      else if (! Con::expandScriptFilename(buffer, 1024, argv[0]))
      {
         Con::warnf("(TypeFilename) illegal filename detected: %s", argv[0]);
         return;
      }

      *((const char **) dptr) = StringTable->insert(buffer);
   }
   else
      Con::printf("(TypeFilename) Cannot set multiple args to a single filename.");
}

ConsoleGetType( TypeFilename )
{
   return *((const char **)(dptr));
}

ConsoleProcessData( TypeFilename )
{
   if( Con::expandScriptFilename( buffer, bufferSz, data ) )
      return buffer;
   else
   {
      Con::warnf("(TypeFilename) illegal filename detected: %s", data);
      return data;
   }
}

//-----------------------------------------------------------------------------
// TypeStringFilename
//-----------------------------------------------------------------------------
ConsolePrepType( filename, TypeStringFilename, String )

ConsoleSetType( TypeStringFilename )
{
   if(argc == 1)
   {
      char buffer[1024];
      if(argv[0][0] == '$')
      {
         dStrncpy(buffer, argv[0], sizeof(buffer) - 1);
         buffer[sizeof(buffer)-1] = 0;
      }
      else if (! Con::expandScriptFilename(buffer, 1024, argv[0]))
      {
         Con::warnf("(TypeStringFilename) illegal filename detected: %s", argv[0]);
         return;
      }

      *((String*)dptr) = String(buffer);
   }
   else
      Con::printf("(TypeStringFilename) Cannot set multiple args to a single filename.");
}

ConsoleGetType( TypeStringFilename )
{
   return *((String*)dptr);
}

ConsoleProcessData( TypeStringFilename )
{
   if( Con::expandScriptFilename( buffer, bufferSz, data ) )
   {
      return buffer;
   }
   else
   {
      Con::warnf("(TypeFilename) illegal filename detected: %s", data);
      return data;
   }
}

//-----------------------------------------------------------------------------
// TypePrefabFilename
//-----------------------------------------------------------------------------
ConsolePrepType( filename, TypePrefabFilename, String )

ConsoleSetType( TypePrefabFilename )
{
   Con::setData(TypeStringFilename, dptr, 0, argc, argv, tbl, flag);
}

ConsoleGetType( TypePrefabFilename )
{
   return *((String*)dptr);
}

ConsoleProcessData( TypePrefabFilename )
{
   if( Con::expandScriptFilename( buffer, bufferSz, data ) )
   {
      return buffer;
   }
   else
   {
      Con::warnf("(TypePrefabFilename) illegal filename detected: %s", data);
      return data;
   }
}

//-----------------------------------------------------------------------------
// TypeImageFilename
//-----------------------------------------------------------------------------
ConsolePrepType( filename, TypeImageFilename, String )

ConsoleSetType( TypeImageFilename )
{
   Con::setData(TypeStringFilename, dptr, 0, argc, argv, tbl, flag);
}

ConsoleGetType( TypeImageFilename )
{
   return *((String*)dptr);
}

ConsoleProcessData( TypeImageFilename )
{
   if( Con::expandScriptFilename( buffer, bufferSz, data ) )
      return buffer;
   else
   {
      Con::warnf("(TypeImageFilename) illegal filename detected: %s", data);
      return data;
   }
}

//-----------------------------------------------------------------------------
// TypeShapeFilename
//-----------------------------------------------------------------------------
ConsolePrepType( filename, TypeShapeFilename, const char* )

ConsoleSetType( TypeShapeFilename )
{
   Con::setData(TypeFilename, dptr, 0, argc, argv, tbl, flag);
}

ConsoleGetType( TypeShapeFilename )
{
   return *((const char **)(dptr));
}

ConsoleProcessData( TypeShapeFilename )
{
   if( Con::expandScriptFilename( buffer, bufferSz, data ) )
      return buffer;
   else
   {
      Con::warnf("(TypeShapeFilename) illegal filename detected: %s", data);
      return data;
   }
}

//-----------------------------------------------------------------------------
// TypeS8
//-----------------------------------------------------------------------------
ConsoleType( char, TypeS8, S8 )
ImplementConsoleTypeCasters( TypeS8, S8 )

ConsoleGetType( TypeS8 )
{
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%d", *((U8 *) dptr) );
   return returnBuffer;
}

ConsoleSetType( TypeS8 )
{
   if(argc == 1)
      *((U8 *) dptr) = dAtoi(argv[0]);
   else
      Con::printf("(TypeU8) Cannot set multiple args to a single S8.");
}

//-----------------------------------------------------------------------------
// TypeS32
//-----------------------------------------------------------------------------
ConsoleType( int, TypeS32, S32 )
ImplementConsoleTypeCasters(TypeS32, S32)

ConsoleGetType( TypeS32 )
{
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%d", *((S32 *) dptr) );
   return returnBuffer;
}

ConsoleSetType( TypeS32 )
{
   if(argc == 1)
      *((S32 *) dptr) = dAtoi(argv[0]);
   else
      Con::printf("(TypeS32) Cannot set multiple args to a single S32.");
}


//-----------------------------------------------------------------------------
// TypeS32Vector
//-----------------------------------------------------------------------------
ConsoleType( intList, TypeS32Vector, Vector<S32> )
ImplementConsoleTypeCasters( TypeS32Vector, Vector< S32 > )

ConsoleGetType( TypeS32Vector )
{
   Vector<S32> *vec = (Vector<S32> *)dptr;
   S32 buffSize = ( vec->size() * 15 ) + 16 ;
   char* returnBuffer = Con::getReturnBuffer( buffSize );
   S32 maxReturn = buffSize;
   returnBuffer[0] = '\0';
   S32 returnLeng = 0;
   for (Vector<S32>::iterator itr = vec->begin(); itr != vec->end(); itr++)
   {
      // concatenate the next value onto the return string
      dSprintf(returnBuffer + returnLeng, maxReturn - returnLeng, "%d ", *itr);
      // update the length of the return string (so far)
      returnLeng = dStrlen(returnBuffer);
   }
   // trim off that last extra space
   if (returnLeng > 0 && returnBuffer[returnLeng - 1] == ' ')
      returnBuffer[returnLeng - 1] = '\0';
   return returnBuffer;
}

ConsoleSetType( TypeS32Vector )
{
   Vector<S32> *vec = (Vector<S32> *)dptr;
   // we assume the vector should be cleared first (not just appending)
   vec->clear();
   if(argc == 1)
   {
      const char *values = argv[0];
      const char *endValues = values + dStrlen(values);
      S32 value;
      // advance through the string, pulling off S32's and advancing the pointer
      while (values < endValues && dSscanf(values, "%d", &value) != 0)
      {
         vec->push_back(value);
         const char *nextValues = dStrchr(values, ' ');
         if (nextValues != 0 && nextValues < endValues)
            values = nextValues + 1;
         else
            break;
      }
   }
   else if (argc > 1)
   {
      for (S32 i = 0; i < argc; i++)
         vec->push_back(dAtoi(argv[i]));
   }
   else
      Con::printf("Vector<S32> must be set as { a, b, c, ... } or \"a b c ...\"");
}

//-----------------------------------------------------------------------------
// TypeF32
//-----------------------------------------------------------------------------
ConsoleType( float, TypeF32, F32 )
ImplementConsoleTypeCasters(TypeF32, F32)

ConsoleGetType( TypeF32 )
{
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%g", *((F32 *) dptr) );
   return returnBuffer;
}
ConsoleSetType( TypeF32 )
{
   if(argc == 1)
      *((F32 *) dptr) = dAtof(argv[0]);
   else
      Con::printf("(TypeF32) Cannot set multiple args to a single F32.");
}

//-----------------------------------------------------------------------------
// TypeF32Vector
//-----------------------------------------------------------------------------
ConsoleType( floatList, TypeF32Vector, Vector<F32> )
ImplementConsoleTypeCasters( TypeF32Vector, Vector< F32 > )

ConsoleGetType( TypeF32Vector )
{
   Vector<F32> *vec = (Vector<F32> *)dptr;
   S32 buffSize = ( vec->size() * 15 ) + 16 ;
   char* returnBuffer = Con::getReturnBuffer( buffSize );
   S32 maxReturn = buffSize;
   returnBuffer[0] = '\0';
   S32 returnLeng = 0;
   for (Vector<F32>::iterator itr = vec->begin(); itr != vec->end(); itr++)
   {
      // concatenate the next value onto the return string
      dSprintf(returnBuffer + returnLeng, maxReturn - returnLeng, "%g ", *itr);
      // update the length of the return string (so far)
      returnLeng = dStrlen(returnBuffer);
   }
   // trim off that last extra space
   if (returnLeng > 0 && returnBuffer[returnLeng - 1] == ' ')
      returnBuffer[returnLeng - 1] = '\0';
   return returnBuffer;
}

ConsoleSetType( TypeF32Vector )
{
   Vector<F32> *vec = (Vector<F32> *)dptr;
   // we assume the vector should be cleared first (not just appending)
   vec->clear();
   if(argc == 1)
   {
      const char *values = argv[0];
      const char *endValues = values + dStrlen(values);
      F32 value;
      // advance through the string, pulling off F32's and advancing the pointer
      while (values < endValues && dSscanf(values, "%g", &value) != 0)
      {
         vec->push_back(value);
         const char *nextValues = dStrchr(values, ' ');
         if (nextValues != 0 && nextValues < endValues)
            values = nextValues + 1;
         else
            break;
      }
   }
   else if (argc > 1)
   {
      for (S32 i = 0; i < argc; i++)
         vec->push_back(dAtof(argv[i]));
   }
   else
      Con::printf("Vector<F32> must be set as { a, b, c, ... } or \"a b c ...\"");
}

//-----------------------------------------------------------------------------
// TypeBool
//-----------------------------------------------------------------------------
ConsoleType( bool, TypeBool, bool )
ImplementConsoleTypeCasters( TypeBool, bool )

ConsoleGetType( TypeBool )
{
   return *((bool *) dptr) ? "1" : "0";
}

ConsoleSetType( TypeBool )
{
   if(argc == 1)
      *((bool *) dptr) = dAtob(argv[0]);
   else
      Con::printf("(TypeBool) Cannot set multiple args to a single bool.");
}


//-----------------------------------------------------------------------------
// TypeBoolVector
//-----------------------------------------------------------------------------
ConsoleType( boolList, TypeBoolVector, Vector<bool> )
ImplementConsoleTypeCasters( TypeBoolVector, Vector< bool > )

ConsoleGetType( TypeBoolVector )
{
   Vector<bool> *vec = (Vector<bool>*)dptr;
   char* returnBuffer = Con::getReturnBuffer(1024);
   S32 maxReturn = 1024;
   returnBuffer[0] = '\0';
   S32 returnLeng = 0;
   for (Vector<bool>::iterator itr = vec->begin(); itr < vec->end(); itr++)
   {
      // concatenate the next value onto the return string
      dSprintf(returnBuffer + returnLeng, maxReturn - returnLeng, "%d ", (*itr == true ? 1 : 0));
      returnLeng = dStrlen(returnBuffer);
   }
   // trim off that last extra space
   if (returnLeng > 0 && returnBuffer[returnLeng - 1] == ' ')
      returnBuffer[returnLeng - 1] = '\0';
   return(returnBuffer);
}

ConsoleSetType( TypeBoolVector )
{
   Vector<bool> *vec = (Vector<bool>*)dptr;
   // we assume the vector should be cleared first (not just appending)
   vec->clear();
   if (argc == 1)
   {
      const char *values = argv[0];
      const char *endValues = values + dStrlen(values);
      S32 value;
      // advance through the string, pulling off bool's and advancing the pointer
      while (values < endValues && dSscanf(values, "%d", &value) != 0)
      {
         vec->push_back(value == 0 ? false : true);
         const char *nextValues = dStrchr(values, ' ');
         if (nextValues != 0 && nextValues < endValues)
            values = nextValues + 1;
         else
            break;
      }
   }
   else if (argc > 1)
   {
      for (S32 i = 0; i < argc; i++)
         vec->push_back(dAtob(argv[i]));
   }
   else
      Con::printf("Vector<bool> must be set as { a, b, c, ... } or \"a b c ...\"");
}


//-----------------------------------------------------------------------------
// TypeFlag
//-----------------------------------------------------------------------------
ConsoleType( flag, TypeFlag, S32 )

ConsoleGetType( TypeFlag )
{
   BitSet32 tempFlags = *(BitSet32 *)dptr;
   if (tempFlags.test(flag)) return "true";
   else return "false";
}

ConsoleSetType( TypeFlag )
{
   bool value = true;
   if (argc != 1)
   {
      Con::printf("flag must be true or false");
   }
   else
   {
      value = dAtob(argv[0]);
   }
   ((BitSet32 *)dptr)->set(flag, value);
}

//-----------------------------------------------------------------------------
// TypeColorF
//-----------------------------------------------------------------------------
ConsoleType( ColorF, TypeColorF, ColorF )
ImplementConsoleTypeCasters( TypeColorF, ColorF )

ConsoleGetType( TypeColorF )
{
   ColorF * color = (ColorF*)dptr;
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%g %g %g %g", color->red, color->green, color->blue, color->alpha);
   return(returnBuffer);
}

ConsoleSetType( TypeColorF )
{
   ColorF *tmpColor = (ColorF *) dptr;
   if(argc == 1)
   {
      tmpColor->set(0, 0, 0, 1);
      F32 r,g,b,a;
      S32 args = dSscanf(argv[0], "%g %g %g %g", &r, &g, &b, &a);
      tmpColor->red = r;
      tmpColor->green = g;
      tmpColor->blue = b;
      if (args == 4)
         tmpColor->alpha = a;
   }
   else if(argc == 3)
   {
      tmpColor->red    = dAtof(argv[0]);
      tmpColor->green  = dAtof(argv[1]);
      tmpColor->blue   = dAtof(argv[2]);
      tmpColor->alpha  = 1.f;
   }
   else if(argc == 4)
   {
      tmpColor->red    = dAtof(argv[0]);
      tmpColor->green  = dAtof(argv[1]);
      tmpColor->blue   = dAtof(argv[2]);
      tmpColor->alpha  = dAtof(argv[3]);
   }
   else
      Con::printf("Color must be set as { r, g, b [,a] }");
}

//-----------------------------------------------------------------------------
// TypeColorI
//-----------------------------------------------------------------------------
ConsoleType( ColorI, TypeColorI, ColorI )
ImplementConsoleTypeCasters( TypeColorI, ColorI )

ConsoleGetType( TypeColorI )
{
   ColorI *color = (ColorI *) dptr;
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%d %d %d %d", color->red, color->green, color->blue, color->alpha);
   return returnBuffer;
}

ConsoleSetType( TypeColorI )
{
   ColorI *tmpColor = (ColorI *) dptr;
   if(argc == 1)
   {
      tmpColor->set(0, 0, 0, 255);
      S32 r,g,b,a;
      S32 args = dSscanf(argv[0], "%d %d %d %d", &r, &g, &b, &a);
      tmpColor->red = r;
      tmpColor->green = g;
      tmpColor->blue = b;
      if (args == 4)
         tmpColor->alpha = a;
   }
   else if(argc == 3)
   {
      tmpColor->red    = dAtoi(argv[0]);
      tmpColor->green  = dAtoi(argv[1]);
      tmpColor->blue   = dAtoi(argv[2]);
      tmpColor->alpha  = 255;
   }
   else if(argc == 4)
   {
      tmpColor->red    = dAtoi(argv[0]);
      tmpColor->green  = dAtoi(argv[1]);
      tmpColor->blue   = dAtoi(argv[2]);
      tmpColor->alpha  = dAtoi(argv[3]);
   }
   else
      Con::printf("Color must be set as { r, g, b [,a] }");
}

//-----------------------------------------------------------------------------
// TypeSimObjectName
//-----------------------------------------------------------------------------
ConsoleType( SimObject, TypeSimObjectName, SimObject* )

ConsoleSetType( TypeSimObjectName )
{
   if(argc == 1)
   {
      SimObject **obj = (SimObject **)dptr;
      *obj = Sim::findObject(argv[0]);
   }
   else
      Con::printf("(TypeSimObjectName) Cannot set multiple args to a single S32.");
}

ConsoleGetType( TypeSimObjectName )
{
   SimObject **obj = (SimObject**)dptr;
   char* returnBuffer = Con::getReturnBuffer(128);
   dSprintf(returnBuffer, 128, "%s", *obj && (*obj)->getName() ? (*obj)->getName() : "");
   return returnBuffer;
}

//-----------------------------------------------------------------------------
// TypeName
//-----------------------------------------------------------------------------
ConsoleType( string, TypeName, const char* )

ConsoleGetType( TypeName )
{
   return *((const char **)(dptr));
}

ConsoleSetType( TypeName )
{   
   Con::warnf( "ConsoleSetType( TypeName ) should not be called. A ProtectedSetMethod does this work!" );
}

//------------------------------------------------------------------------------
// TypeParticleParameterString
//------------------------------------------------------------------------------
ConsoleType( string, TypeParticleParameterString, const char* )

ConsoleGetType( TypeParticleParameterString )
{
   return *((const char **)(dptr));
}

ConsoleSetType( TypeParticleParameterString )
{
   if(argc == 1)
      *((const char **) dptr) = StringTable->insert(argv[0]);
   else
      Con::printf("(TypeParticleParameterString) Cannot set multiple args to a single string.");
}

//-----------------------------------------------------------------------------
// TypeMaterialName
//-----------------------------------------------------------------------------

ConsoleType( string, TypeMaterialName, String )

ConsoleGetType( TypeMaterialName )
{
   const String *theString = static_cast<const String*>(dptr);
   return theString->c_str();
}

ConsoleSetType( TypeMaterialName )
{
   String *theString = static_cast<String*>(dptr);

   if(argc == 1)
      *theString = argv[0];
   else
      Con::printf("(TypeMaterialName) Cannot set multiple args to a single string.");
}

//-----------------------------------------------------------------------------
// TypeTerrainMaterialIndex
//-----------------------------------------------------------------------------

ConsoleType( int, TypeTerrainMaterialIndex, S32 )

ConsoleGetType( TypeTerrainMaterialIndex )
{
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%d", *((S32 *) dptr) );
   return returnBuffer;
}

ConsoleSetType( TypeTerrainMaterialIndex )
{
   if(argc == 1)
      *((S32 *) dptr) = dAtoi(argv[0]);
   else
      Con::printf("(TypeTerrainMaterialIndex) Cannot set multiple args to a single S32.");
}

//-----------------------------------------------------------------------------
// TypeTerrainMaterialName
//-----------------------------------------------------------------------------

ConsoleType( string, TypeTerrainMaterialName, const char* )

ConsoleGetType( TypeTerrainMaterialName )
{
   return *((const char **)(dptr));
}

ConsoleSetType( TypeTerrainMaterialName )
{
   if(argc == 1)
      *((const char **) dptr) = StringTable->insert(argv[0]);
   else
      Con::printf("(TypeTerrainMaterialName) Cannot set multiple args to a single string.");
}

//-----------------------------------------------------------------------------
// TypeCubemapName
//-----------------------------------------------------------------------------

ConsoleType( string, TypeCubemapName, String )

ConsoleGetType( TypeCubemapName )
{
   const String *theString = static_cast<const String*>(dptr);
   return theString->c_str();
}

ConsoleSetType( TypeCubemapName )
{
   String *theString = static_cast<String*>(dptr);

   if(argc == 1)
      *theString = argv[0];
   else
      Con::printf("(TypeCubemapName) Cannot set multiple args to a single string.");
}

//-----------------------------------------------------------------------------
// TypeRectUV
//-----------------------------------------------------------------------------
ConsoleType( RectF, TypeRectUV, RectF )

ConsoleGetType( TypeRectUV )
{
   RectF *rect = (RectF *) dptr;
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%g %g %g %g", rect->point.x, rect->point.y,
            rect->extent.x, rect->extent.y);
   return returnBuffer;
}

ConsoleSetType( TypeRectUV )
{
   if(argc == 1)
      dSscanf(argv[0], "%g %g %g %g", &((RectF *) dptr)->point.x, &((RectF *) dptr)->point.y,
              &((RectF *) dptr)->extent.x, &((RectF *) dptr)->extent.y);
   else if(argc == 4)
      *((RectF *) dptr) = RectF(dAtof(argv[0]), dAtof(argv[1]), dAtof(argv[2]), dAtof(argv[3]));
   else
      Con::printf("RectUV must be set as { x, y, w, h } or \"x y w h\"");
}

//-----------------------------------------------------------------------------
// TypeUUID
//-----------------------------------------------------------------------------
ConsoleType( uuid, TypeUUID, Torque::UUID )
ImplementConsoleTypeCasters( TypeUUID, Torque::UUID )

ConsoleGetType( TypeUUID )
{
   Torque::UUID* uuid = ( Torque::UUID* ) dptr;
   return Con::getReturnBuffer( uuid->toString() );
}

ConsoleSetType( TypeUUID )
{
   if( argc == 1 )
   {
      Torque::UUID* uuid = ( Torque::UUID* ) dptr;
      if( !uuid->fromString( argv[ 0 ] ) )
         Con::errorf( "Error parsing UUID: '%s'", argv[ 0 ] );
   }
   else
      Con::printf( "(TypeUUID) Cannot set multiple args to a single uuid." );
}

//-----------------------------------------------------------------------------
// TypePID
//-----------------------------------------------------------------------------
ConsoleType( pid, TypePID, SimPersistID* )
ImplementConsoleTypeCasters( TypePID, SimPersistID* )

ConsoleGetType( TypePID )
{
   SimPersistID* pid = *( ( SimPersistID** ) dptr );
   if( !pid )
      return "";
      
   return Con::getReturnBuffer( pid->getUUID().toString() );
}

ConsoleSetType( TypePID )
{
   if( argc == 1 )
   {
      SimPersistID** pid = ( SimPersistID** ) dptr;
      
      if( !argv[ 0 ][ 0 ] )
         *pid = NULL;
      else
      {
         Torque::UUID uuid;
         if( !uuid.fromString( argv[ 0 ] ) )
         {
            Con::errorf( "Error parsing UUID in PID: '%s'", argv[ 0 ] );
            *pid = NULL;
         }
         else
            *pid = SimPersistID::findOrCreate( uuid );
      }
   }
   else
      Con::printf( "(TypePID) Cannot set multiple args to a single pid." );
}

//-----------------------------------------------------------------------------
// TypeSimPersistId
//-----------------------------------------------------------------------------
ConsoleType( SimPersistId, TypeSimPersistId, SimPersistID* )

ConsoleGetType( TypeSimPersistId )
{
   SimPersistID* pid = *( ( SimPersistID** ) dptr );
   if( !pid )
      return "";
      
   return Con::getReturnBuffer( pid->getUUID().toString() );
}

ConsoleSetType( TypeSimPersistId )
{
   if( argc == 1 )
   {
      SimPersistID** pid = ( SimPersistID** ) dptr;
      
      if( !argv[ 0 ][ 0 ] )
         *pid = NULL;
      else
      {
         Torque::UUID uuid;
         if( !uuid.fromString( argv[ 0 ] ) )
         {
            Con::errorf( "Error parsing UUID in PID: '%s'", argv[ 0 ] );
            *pid = NULL;
         }
         else
            *pid = SimPersistID::find( uuid );
      }
   }
   else
      Con::printf( "(TypeSimPersistId) Cannot set multiple args to a single SimPersistId." );
}
