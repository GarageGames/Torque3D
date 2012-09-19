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

#include "console/engineExports.h"
#include "console/engineTypeInfo.h"
#include "console/engineAPI.h"


IMPLEMENT_SCOPE( ReflectionAPI, Reflection,,
   "Metadata for the exported engine API." );
   
IMPLEMENT_NONINSTANTIABLE_CLASS( EngineExport,
   "Abstract base class of entities exported through the engine API." )
END_IMPLEMENT_CLASS;
IMPLEMENT_NONINSTANTIABLE_CLASS( EngineExportScope,
   "A scope contained a collection of exported engine API entities." )
END_IMPLEMENT_CLASS;
   
EngineExportScope EngineExportScope::smGlobalScope;


//-----------------------------------------------------------------------------

EngineExport::EngineExport( const char* name, EngineExportKind kind, EngineExportScope* scope, const char* docString )
   : mExportName( name ),
     mExportKind( kind ),
     mExportScope( scope ),
     mNextExport( NULL ),
     mDocString( docString )
{
   AssertFatal( name != NULL, "EngineExport - export without name!" );
   AssertFatal( scope != NULL, avar( "EngineExport - export '%s' is in no scope" ) );
   
   // Link to scope's export chain.
   
   mNextExport = scope->mExports;
   scope->mExports = this;
}

//-----------------------------------------------------------------------------

String EngineExport::getFullyQualifiedExportName() const
{
   if( getExportScope() )
   {
      String parentQualifiedName = getExportScope()->getFullyQualifiedExportName();
      if( parentQualifiedName.isEmpty() )
         return getExportName();
         
      return String::ToString( "%s::%s", parentQualifiedName.c_str(), getExportName() );
   }
      
   return getExportName();
}

//-----------------------------------------------------------------------------

EngineExportScope::EngineExportScope( const char* name, EngineExportScope* scope, const char* docString )
   : SuperType( name, EngineExportKindScope, scope, docString )
{
   // Do *NOT* initialize mExports here.  EngineExportScopes should be
   // instantiated globally and by not initializing the field, we allow
   // exports to link themselves to their scope without being order dependent
   // on our constructor.
}
