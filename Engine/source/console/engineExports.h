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

#ifndef _ENGINEEXPORTS_H_
#define _ENGINEEXPORTS_H_

#ifndef _ENGINEOBJECT_H_
   #include "console/engineObject.h"
#endif


/// @file
/// Foundation for the engine API export system.
///
/// The engine DLL exposes a well-defined API that the control layer can
/// use to interface with the engine.  The structure of this API is accessible
/// through 
///
/// The system is primarily meant to allow mechanical extraction and processing
/// of the API.  It is not meant to be used as a direct means to actually interface
/// with the engine.
///
/// All export classes are themselves EngineObjects so they can be used in the
/// API.  They are, however, all declared as non-instantiable classes.


class EngineExportScope;


DECLARE_SCOPE( ReflectionAPI );


/// Kind of entity being exported.
enum EngineExportKind
{
   EngineExportKindScope,     ///< A collection of exports grouped in a separate named scope.
   EngineExportKindFunction,  ///< A function call across the interop border going either in or out.
   EngineExportKindType       ///< A data type for data exchange between the engine and its control layer.  Note that types are also scopes.
};


/// Information about an entity exported by the engine API.  This is an abstract base
/// class.
class EngineExport : public StaticEngineObject
{
   public:

      DECLARE_ABSTRACT_CLASS( EngineExport, StaticEngineObject );
      DECLARE_INSCOPE( ReflectionAPI );
      friend class EngineExportScope; // Default constructor.
   
   protected:
   
      /// Name of the export.  Never NULL but will be an empty string for anonymous
      /// exports such as function types.
      const char* mExportName;

      /// Kind of export.
      EngineExportKind mExportKind;
   
      /// The scope in which this export is defined.
      EngineExportScope* mExportScope;
      
      /// Documentation string.
      const char* mDocString;

      /// Next export in the link chain of the export's scope.
      EngineExport* mNextExport;
            
      /// Protected constructor as this is an abstract class.
      ///
      /// @param name Export name.
      /// @param kind Export kind.
      /// @param scope Scope to export to.
      /// @param docString Documentation string.
      EngineExport( const char* name, EngineExportKind kind, EngineExportScope* scope, const char* docString );
            
   public:
   
      /// Return the name of the export.
      const char* getExportName() const { return mExportName; }
      
      /// Return the fully qualified name of this export starting from the global export scope.
      /// Qualifiers are separated with "::".
      String getFullyQualifiedExportName() const;
      
      /// Return the kind of entity being exported.
      EngineExportKind getExportKind() const { return mExportKind; }
      
      /// Return the scope that contains this export.  All exports except the global scope
      /// itself are contained in a scope.
      EngineExportScope* getExportScope() const { return mExportScope; }

      /// Get the next export in the link chain of the export's associated scope.
      EngineExport* getNextExport() const { return mNextExport; }

      /// Return the documentation string for this type.
      const char* getDocString() const { return mDocString; }

   private:
   
      /// Special constructor for the global scope instance.
      EngineExport()
         : mExportName( "" ),
           mExportKind( EngineExportKindScope ),
           mExportScope( NULL ),
           mNextExport( NULL ) {}
};


/// A group of engine exports.
class EngineExportScope : public EngineExport
{
   public:
   
      DECLARE_CLASS( EngineExportScope, EngineExport );
      friend class EngineExport; // mExports
      friend struct _GLOBALSCOPE; // smGlobalScope
      template< typename T > friend T* constructInPlace( T* );
   
   protected:
      
      /// Head of the link chain of exports for this scope.
      EngineExport* mExports;
      
      /// The global export scope singleton.
      static EngineExportScope smGlobalScope;
   
   public:
   
      /// Construct a new export scope.
      ///
      /// @param name Name of the scope inside its parent scope.
      /// @param scope Parent scope.
      /// @param docString Documentation string.
      EngineExportScope( const char* name, EngineExportScope* scope, const char* docString );
   
      /// Return the global export scope singleton.  This is the root of the
      /// export hierarchy and thus directly or indirectly contains all
      /// entities exported by the engine.
      static EngineExportScope* getGlobalScope() { return &smGlobalScope; }
            
      /// Return the chain of exports associated with this scope.
      EngineExport* getExports() const { return mExports; }
      
   private:
   
      /// Constructor for the global scope.
      EngineExportScope() {}
};


/// @}

#endif // !_ENGINEEXPORTS_H_
