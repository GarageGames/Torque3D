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
#include "core/util/str.h"
#include "gfx/gfxDevice.h"
#include "langElement.h"

//**************************************************************************
// Language element
//**************************************************************************
Vector<LangElement*> LangElement::elementList( __FILE__, __LINE__ );

//--------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------
LangElement::LangElement()
{
   elementList.push_back( this );

   static U32 tempNum = 0;
   dSprintf( (char*)name, sizeof(name), "tempName%d", tempNum++ );
}

//--------------------------------------------------------------------------
// Find element of specified name
//--------------------------------------------------------------------------
LangElement * LangElement::find( const char *name )
{
   for( U32 i=0; i<elementList.size(); i++ )
   {
      if( !dStrcmp( (char*)elementList[i]->name, name ) )
      {
         return elementList[i];
      }
   }
   
   return NULL;
}

//--------------------------------------------------------------------------
// Delete existing elements
//--------------------------------------------------------------------------
void LangElement::deleteElements()
{
   for( U32 i=0; i<elementList.size(); i++ )
   {
      delete elementList[i];
   }
   
   elementList.setSize( 0 );

}

//--------------------------------------------------------------------------
// Set name
//--------------------------------------------------------------------------
void LangElement::setName(const char* newName )
{
   dStrncpy( ( char* ) name, newName, sizeof( name ) );
   name[ sizeof( name ) - 1 ] = '\0';
}

//**************************************************************************
// Variable
//**************************************************************************
U32 Var::texUnitCount = 0;

Var::Var()
{
   dStrcpy( (char*)type, "float4" );
   structName[0] = '\0';
   connectName[0] = '\0';
   constSortPos = cspUninit;
   constNum = 0;
   texCoordNum = 0;
   uniform = false;
   vertData = false;
   connector = false;
   sampler = false;
   mapsToSampler = false;
   arraySize = 1;
   texture = false;
   rank = 0;
}

Var::Var( const char *inName, const char *inType )
{
   structName[0] = '\0';
   connectName[0] = '\0';
   uniform = false;
   vertData = false;
   connector = false;
   sampler = false;
   mapsToSampler = false;
   texCoordNum = 0;
   constSortPos = cspUninit;
   arraySize = 1;
   texture = false;
   rank = 0;

   setName( inName );
   setType( inType );
}

void Var::setUniform(const String& constType, const String& constName, ConstantSortPosition sortPos)
{ 
   uniform = true;
   setType(constType.c_str());
   setName(constName.c_str());   
   constSortPos = cspPass;      
}

//--------------------------------------------------------------------------
// Set struct name
//--------------------------------------------------------------------------
void Var::setStructName(const char* newName )
{
   dStrncpy( ( char* ) structName, newName, sizeof( structName ) );
   structName[ sizeof( structName ) - 1 ] = '\0';
}

//--------------------------------------------------------------------------
// Set connect name
//--------------------------------------------------------------------------
void Var::setConnectName(const char* newName )
{
   dStrncpy( ( char* ) connectName, newName, sizeof( connectName ) );
   connectName[ sizeof( connectName ) - 1 ] = '\0';
}

//--------------------------------------------------------------------------
// Set type
//--------------------------------------------------------------------------
void Var::setType(const char *newType )
{
   dStrncpy( ( char* ) type, newType, sizeof( type ) );
   type[ sizeof( type ) - 1 ] = '\0';
}

//--------------------------------------------------------------------------
// print
//--------------------------------------------------------------------------
void Var::print( Stream &stream )
{
   if( structName[0] != '\0' )
   {
      stream.write( dStrlen((char*)structName), structName );
      if(GFX->getAdapterType() == OpenGL)
         stream.write( 1, "_" );
      else
      stream.write( 1, "." );
   }

   stream.write( dStrlen((char*)name), name );
}

//--------------------------------------------------------------------------
// Get next available texture unit number
//--------------------------------------------------------------------------
U32 Var::getTexUnitNum(U32 numElements)
{
   U32 ret = texUnitCount;
   texUnitCount += numElements;
   return ret;
}

//--------------------------------------------------------------------------
// Reset
//--------------------------------------------------------------------------
void Var::reset()
{
   texUnitCount = 0;
}

//**************************************************************************
// Multi line statement
//**************************************************************************
void MultiLine::addStatement( LangElement *elem )
{
   AssertFatal( elem, "Attempting to add empty statement" );

   mStatementList.push_back( elem );
}

//--------------------------------------------------------------------------
// Print
//--------------------------------------------------------------------------
void MultiLine::print( Stream &stream )
{
   for( U32 i=0; i<mStatementList.size(); i++ )
   {
      mStatementList[i]->print( stream );
   }
} 