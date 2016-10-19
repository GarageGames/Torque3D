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

#include "platform/platform.h"
#include "console/dynamicTypes.h"
#include "core/stringTable.h"
#include "core/strings/stringFunctions.h"


ConsoleBaseType* ConsoleBaseType::smListHead = NULL;
S32 ConsoleBaseType::smConsoleTypeCount = 0;

// And, we also privately store the types lookup table.
static VectorPtr< ConsoleBaseType* > gConsoleTypeTable( __FILE__, __LINE__ );


//-----------------------------------------------------------------------------

ConsoleBaseType::ConsoleBaseType( const S32 size, S32 *idPtr, const char *aTypeName )
   :  mTypeSize( size ),
      mInspectorFieldType( NULL ),
      mTypeInfo( NULL )
{
   mTypeName = StringTable->insert( aTypeName );
   mTypeID = smConsoleTypeCount++;
   *idPtr = mTypeID;

   // Link ourselves into the list.
   mListNext = smListHead;
   smListHead = this;
}

//-----------------------------------------------------------------------------

ConsoleBaseType *ConsoleBaseType::getListHead()
{
   return smListHead;
}

//-----------------------------------------------------------------------------

void ConsoleBaseType::initialize()
{
   // Prep and empty the vector.
   gConsoleTypeTable.setSize(smConsoleTypeCount+1);
   dMemset(gConsoleTypeTable.address(), 0, sizeof(ConsoleBaseType*) * gConsoleTypeTable.size());

   // Walk the list and register each one with the console system.
   ConsoleBaseType *walk = getListHead();
   while(walk)
   {
      const S32 id = walk->getTypeID();
      AssertFatal(gConsoleTypeTable[id]==NULL, "ConsoleBaseType::initialize - encountered a table slot that contained something!");
      gConsoleTypeTable[id] = walk;
  
      walk = walk->getListNext();
   }
}

//-----------------------------------------------------------------------------

ConsoleBaseType* ConsoleBaseType::getType(const S32 typeID)
{
   if( typeID == -1 || gConsoleTypeTable.size() <= typeID )
      return NULL;
      
   return gConsoleTypeTable[ typeID ];
}

//-----------------------------------------------------------------------------

ConsoleBaseType* ConsoleBaseType::getTypeByName(const char *typeName)
{
   ConsoleBaseType* walk = getListHead();
   while( walk != NULL )
   {
      if( dStrcmp( walk->getTypeName(), typeName ) == 0 )
         return walk;

      walk = walk->getListNext();
   }

   return NULL;
}

//-----------------------------------------------------------------------------

ConsoleBaseType * ConsoleBaseType::getTypeByClassName(const char *typeName)
{
   ConsoleBaseType *walk = getListHead();
   while( walk != NULL )
   {
      if( dStrcmp( walk->getTypeClassName(), typeName ) == 0 )
         return walk;

      walk = walk->getListNext();
   }

   return NULL;
}
