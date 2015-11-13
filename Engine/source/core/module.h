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

#ifndef _MODULE_H_
#define _MODULE_H_

#ifndef _TSINGLETON_H_
   #include "core/util/tSingleton.h"
#endif
#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif


/// @file
/// A system for keeping initialization and shutdown modular while
/// avoiding non-trivial global constructors/destructors.


/// An engine component that requires initialization and/or cleanup.
class Module
{
   public:
   
      typedef void Parent;
      friend struct EngineModuleManager;
      
   protected:
   
      struct Dependency;
      friend struct Dependency;
      
      enum Mode
      {
         ModeInitialize,
         ModeShutdown
      };
      
      /// Direction of a dependency edge.
      enum DependencyType
      {
         DependencyBefore,
         DependencyAfter
      };
   
      /// Entry in the list of dependencies.
      struct Dependency
      {
         /// Direction of dependence.  A "before" dependence goes the reverse direction.
         DependencyType mType;
         
         /// Name of the module that this module depends on.
         const char* mModuleName;
         
         /// Pointer to module.  Filled by init code.
         Module* mModule;
         
         /// Next dependency or NULL.
         Dependency* mNext;
         
         Dependency( Mode mode, DependencyType type, Module* parentModule, const char* moduleName )
            : mType( type ),
              mModuleName( moduleName ),
              mModule( NULL ),
              mNext( mode == ModeInitialize ? parentModule->mInitDependencies : parentModule->mShutdownDependencies )
         {
            if( mode == ModeInitialize )
               parentModule->mInitDependencies = this;
            else
               parentModule->mShutdownDependencies = this;
         }
      };
      
      /// Record for module that this module overrides.
      struct Override
      {
         /// Name of module being overridden.
         const char* mModuleName;
         
         /// Next override or NULL.
         Override* mNext;
         
         Override( Module* parentModule, const char* moduleName )
            : mModuleName( moduleName ),
              mNext( parentModule->mOverrides )
         {
            parentModule->mOverrides = this;
         }
      };
      
      /// Flag to make sure we don't shutdown modules that have not been initialized.
      bool mIsInitialized;
      
      /// Next module in the global module list.
      Module* mNext;
      
      /// List of modules to which the initialization of this module has dependency relations.
      Dependency* mInitDependencies;
            
      /// List of modules to which the shutdown of this module has dependency relations.
      Dependency* mShutdownDependencies;
      
      /// List of modules being overriden by this module.
      Override* mOverrides;
                  
      /// Global list of modules.
      static Module* smFirst;
      
      /// Return true if this module is constrained to precede "module" in the given "mode".
      bool _constrainedToComeBefore( Module* module, Mode mode );

      /// Return true if this module is constrained to follow "module" in the given "mode".
      bool _constrainedToComeAfter( Module* module, Mode mode );
      
      ///
      Dependency* _getDependencies( Mode mode )
      {
         if( mode == ModeInitialize )
            return mInitDependencies;
         else
            return mShutdownDependencies;
      }
            
      Module()
         : mIsInitialized( false ),
           mNext( smFirst ),
           mInitDependencies( NULL ),
           mShutdownDependencies( NULL ),
           mOverrides( NULL )

      {
         smFirst = this;
      }
      
   public:
   
      /// Return the module name.
      virtual const char* getName() const = 0;
      
      /// Initialize the module.  This is only called after all modules that this
      /// module depends on have been initialized.
      virtual void initialize() {}
      
      /// Shut down the module.  This is called before any module that this module
      /// depends on have been shut down.
      virtual void shutdown() {}
};


/// Begin a module definition.
///
/// @code
/// MODULE_BEGIN( MyModule )
///
///   MODULE_INIT_AFTER( Sim )
///   MODULE_INIT_BEFORE( 3D )
///   MODULE_SHUTDOWN_BEFORE( Sim )
///
///   MODULE_INIT
///   {
///      // Init code...
///   }
///
///   MODULE_SHUTDOWN
///   {
///      // Cleanup code...
///   }
///
/// MODULE_END;
/// @endcode
#define MODULE_BEGIN( name )                                                     \
   namespace { namespace _ ## name {                                             \
      class _ModuleInst : public ::Module {                                      \
         public:                                                                 \
            typedef ::Module Parent;                                             \
            static _ModuleInst smInstance;                                       \
            virtual const char* getName() const { return #name; }

/// Make sure this module is initialized before the module called "name".
///
/// @code
/// MODULE_BEGIN( MyModule )
///    MODULE_INIT_BEFORE( MyOtherModule )
/// MODULE_END;
/// @endcode
#define MODULE_INIT_BEFORE( name )                                                                 \
            struct _DepInitBefore ## name : public Parent::Dependency                              \
            {                                                                                      \
               _DepInitBefore ## name()                                                            \
                  : Parent::Dependency( ModeInitialize, DependencyBefore, &smInstance, #name ) {}  \
            } mDepInitBefore ## name;

/// Make sure this module is initialized after the module called "name".
///
/// @code
/// MODULE_BEGIN( MyModule )
///    MODULE_INIT_AFTER( MyOtherModule )
/// MODULE_END;
/// @endcode
#define MODULE_INIT_AFTER( name )                                                                  \
            struct _DepInitAfter ## name : public Parent::Dependency                               \
            {                                                                                      \
               _DepInitAfter ## name()                                                             \
                  : Parent::Dependency( ModeInitialize, DependencyAfter, &smInstance, #name ) {}   \
            } mDepInitAfter ## name;

/// Make sure this module is initialized before the module called "name".
///
/// @code
/// MODULE_BEGIN( MyModule )
///    MODULE_SHUTDOWN_BEFORE( MyOtherModule )
/// MODULE_END;
/// @endcode
#define MODULE_SHUTDOWN_BEFORE( name )                                                             \
            struct _DepShutdownBefore ## name : public Parent::Dependency                          \
            {                                                                                      \
               _DepShutdownBefore ## name()                                                        \
                  : Parent::Dependency( ModeShutdown, DependencyBefore, &smInstance, #name ) {}    \
            } mDepShutdownBefore ## name;

/// Make sure this module is initialized after the module called "name".
///
/// @code
/// MODULE_BEGIN( MyModule )
///    MODULE_SHUTDOWN_AFTER( MyOtherModule )
/// MODULE_END;
/// @endcode
#define MODULE_SHUTDOWN_AFTER( name )                                                              \
            struct _DepShutdownAfter ## name : public Parent::Dependency                           \
            {                                                                                      \
               _DepShutdownAfter ## name()                                                         \
                  : Parent::Dependency( ModeShutdown, DependencyAfter, &smInstance, #name ) {}     \
            } mDepShutdownAfter ## name;
            
/// Replace the given module in both the init and the shutdown sequence.
///
/// @code
/// MODULE_BEGIN( MyMoveManager )
///   MODULE_OVERRIDE( MoveManager )
/// MODULE_END;
/// @endcode
#define MODULE_OVERRIDE( name )                                                                    \
            struct _Override ## name : public Parent::Override                                     \
            {                                                                                      \
               _Override ## name()                                                                 \
                  : Parent::Override( &smInstance, #name ) {}                                      \
            } mOverride ## name;

/// Define initialization code for the module.
///
/// @code
/// MODULE_BEGIN( MyModule )
///    MODULE_INIT
///    {
///       // Init code goes here.
///    }
/// MODULE_END;
/// @endcode
#define MODULE_INIT                                                              \
            virtual void initialize()

/// Define cleanup code for the module.
///
/// @code
/// MODULE_BEGIN( MyModule )
///    MODULE_SHUTDOWN
///    {
///       // Cleanup code goes here.
///    }
/// MODULE_END;
/// @endcode
#define MODULE_SHUTDOWN                                                          \
            virtual void shutdown()

/// Terminate a module definition.
///
/// @code
/// MODULE_BEGIN( MyModule )
/// MODULE_END;
/// @endcode
#define MODULE_END                                                               \
      };                                                                         \
      _ModuleInst _ModuleInst::smInstance;                                       \
   } }
   
   

/// Used to define a function which will be called right 
/// after the named module is initialized.
///
/// @code
/// AFTER_MODULE_INIT( Sim )
/// {
///    Con::addVariable( "$myBool", TypeBool, &smMyBool );
/// }
/// @endcode
///
#define AFTER_MODULE_INIT( name ) \
   namespace { \
      class _AfterModuleInit : public ::Module { \
         public: \
            typedef ::Module Parent; \
            static _AfterModuleInit smInstance; \
            virtual const char* getName() const { return "AFTER_MODULE_INIT( " #name " ) in " __FILE__; } \
            struct _DepInitAfter : public Parent::Dependency \
            { \
               _DepInitAfter() \
               : Parent::Dependency( ModeInitialize, DependencyAfter, &smInstance, #name ) {} \
            } mDepInitAfter; \
            virtual void initialize(); \
      }; \
      _AfterModuleInit _AfterModuleInit::smInstance; \
   } \
   void _AfterModuleInit::initialize()


struct EngineModuleManager
{
      /// Initialize all modules registered with the system.
      static void initializeSystem();
      
      /// Shutdown all modules registered with the system.
      static void shutdownSystem();
   
      /// Return the instance of the module called "name" or NULL if no such module is defined.
      static Module* findModule( const char* name );

   private:
   
      static Module* _findOverrideFor( Module* module );
      static String _moduleListToString( Vector< Module* >& moduleList );
      static void _printModuleList( Vector< Module* >& moduleList );
      static void _insertIntoModuleList( Module::Mode mode, Vector< Module* >& moduleList, Module* module );
      static S32 _getIndexOfModuleInList( Vector< Module* >& moduleList, Module* module );
      static S32 _getIndexOfModuleInList( Vector< Module* >& moduleList, const char* moduleName );
      static void _createModuleList( Module::Mode mode, Vector< Module* >& moduleList );
};

#endif // !_MODULE_H_
