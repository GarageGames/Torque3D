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

#ifndef _CONSOLEINTERNAL_H_
#define _CONSOLEINTERNAL_H_

#ifndef _STRINGFUNCTIONS_H_
   #include "core/strings/stringFunctions.h"
#endif
#ifndef _STRINGTABLE_H_
   #include "core/stringTable.h"
#endif
#ifndef _CONSOLETYPES_H
   #include "console/consoleTypes.h"
#endif
#ifndef _CONSOLEOBJECT_H_
   #include "console/simObject.h"
#endif
#ifndef _DATACHUNKER_H_
   #include "core/dataChunker.h"
#endif


/// @ingroup console_system Console System
/// @{


class ExprEvalState;
struct FunctionDecl;
class CodeBlock;
class AbstractClassRep;


/// A dictionary of function entries.
///
/// Namespaces are used for dispatching calls in the console system.
class Namespace
{
      enum {
        MaxActivePackages = 512,
      };

      static U32 mNumActivePackages;
      static U32 mOldNumActivePackages;
      static StringTableEntry mActivePackages[MaxActivePackages];

   public:
      StringTableEntry mName;
      StringTableEntry mPackage;

      Namespace *mParent;
      Namespace *mNext;
      AbstractClassRep *mClassRep;
      U32 mRefCountToParent;
      
      const char* mUsage;
      
      /// Script defined usage strings need to be cleaned up. This
      /// field indicates whether or not the usage was set from script.
      bool mCleanUpUsage;

      /// A function entry in the namespace.
      struct Entry
      {
         enum 
         {
            ScriptCallbackType           = -3,
            GroupMarker                  = -2,
            InvalidFunctionType          = -1,
            ConsoleFunctionType,
            StringCallbackType,
            IntCallbackType,
            FloatCallbackType,
            VoidCallbackType,
            BoolCallbackType
         };

         /// Link back to the namespace to which the entry belongs.
         Namespace* mNamespace;
         
         /// Next function entry in the hashtable link chain of the namespace.
         Entry* mNext;
         
         /// Name of this function.
         StringTableEntry mFunctionName;
         
         ///
         S32 mType;
         
         /// Min number of arguments expected by this function.
         S32 mMinArgs;
         
         /// Max number of arguments expected by this function.  If zero,
         /// function takes an arbitrary number of arguments.
         S32 mMaxArgs;
         
         /// Name of the package to which this function belongs.
         StringTableEntry mPackage;
         
         /// Whether this function is included only in TORQUE_TOOLS builds.
         bool mToolOnly;

         /// Usage string for documentation.
         const char* mUsage;
         
         /// Extended console function information.
         ConsoleFunctionHeader* mHeader;

         /// The compiled script code if this is a script function.
         CodeBlock* mCode;
         
         /// The offset in the compiled script code at which this function begins.
         U32 mFunctionOffset;

         /// If it's a script function, this is the line of the declaration in code.
         /// @note 0 for functions read from legacy DSOs that have no line number information.
         U32 mFunctionLineNumber;
         
         union CallbackUnion {
            StringCallback mStringCallbackFunc;
            IntCallback mIntCallbackFunc;
            VoidCallback mVoidCallbackFunc;
            FloatCallback mFloatCallbackFunc;
            BoolCallback mBoolCallbackFunc;
            const char *mGroupName;
            const char *mCallbackName;
         } cb;
         
         Entry();
         
         ///
         void clear();

         ///
         const char *execute( S32 argc, const char** argv, ExprEvalState* state );
         
         /// Return a one-line documentation text string for the function.
         String getBriefDescription( String* outRemainingDocText = NULL ) const;
         
         /// Get the auto-doc string for this function.  This string does not included prototype information.
         String getDocString() const;
         
         /// Return a string describing the arguments the function takes including default argument values.
         String getArgumentsString() const;

         /// Return a full prototype string for the function including return type, function name,
         /// and arguments.
         String getPrototypeString() const;
      };
      
      Entry* mEntryList;

      Entry** mHashTable;
      
      U32 mHashSize;
      U32 mHashSequence;   ///< @note The hash sequence is used by the autodoc console facility
                           ///        as a means of testing reference state.

      Namespace();
      ~Namespace();

      void addFunction( StringTableEntry name, CodeBlock* cb, U32 functionOffset, const char* usage = NULL, U32 lineNumber = 0 );
      void addCommand( StringTableEntry name, StringCallback, const char *usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL );
      void addCommand( StringTableEntry name, IntCallback, const char *usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL );
      void addCommand( StringTableEntry name, FloatCallback, const char *usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL );
      void addCommand( StringTableEntry name, VoidCallback, const char *usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL );
      void addCommand( StringTableEntry name, BoolCallback, const char *usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL );

      void addScriptCallback( const char *funcName, const char *usage, ConsoleFunctionHeader* header = NULL );

      void markGroup(const char* name, const char* usage);
      char * lastUsage;

      /// Returns true if this namespace represents an engine defined
      /// class and is not just a script defined class namespace.
      bool isClass() const { return mClassRep && mClassRep->getNameSpace() == this; }

      void getEntryList(VectorPtr<Entry *> *);

      /// Return the name of this namespace.
      StringTableEntry getName() const { return mName; }

      /// Return the superordinate namespace to this namespace. Symbols are inherited from
      /// this namespace.
      Namespace* getParent() const { return mParent; }

      /// Return the topmost package in the parent hierarchy of this namespace
      /// that still refers to the same namespace.  If packages are active and
      /// adding to this namespace, then they will be linked in-between the namespace
      /// they are adding to and its real parent namespace.
      Namespace* getPackageRoot()
      {
         Namespace* walk = this;
         while( walk->mParent && walk->mParent->mName == mName )
            walk = walk->mParent;

         return walk;
      }

      /// Return the package in which this namespace is defined.
      StringTableEntry getPackage() const { return mPackage; }

      /// Increase the count on the reference that this namespace
      /// holds to its parent.
      /// @note Must not be called on namespaces coming from packages.
      void incRefCountToParent()
      {
         AssertFatal( mPackage == NULL, "Namespace::incRefCountToParent - Must not be called on a namespace coming from a package!" );
         mRefCountToParent ++;
      }

      /// Decrease the count on the reference that this namespace
      /// holds to its parent.
      /// @note Must not be called on namespaces coming from packages.
      void decRefCountToParent()
      {
         unlinkClass( NULL );
      }

      Entry *lookup(StringTableEntry name);
      Entry *lookupRecursive(StringTableEntry name);
      Entry *createLocalEntry(StringTableEntry name);
      void buildHashTable();
      void clearEntries();
      bool classLinkTo(Namespace *parent);
      bool unlinkClass(Namespace *parent);
      void getUniqueEntryLists( Namespace *other, VectorPtr<Entry *> *outThisList, VectorPtr<Entry *> *outOtherList );

      const char *tabComplete(const char *prevText, S32 baseLen, bool fForward);

      static U32 mCacheSequence;
      static DataChunker mCacheAllocator;
      static DataChunker mAllocator;
      static void trashCache();
      static Namespace *mNamespaceList;
      static Namespace *mGlobalNamespace;

      static void init();
      static void shutdown();
      static Namespace *global();

      static Namespace *find(StringTableEntry name, StringTableEntry package=NULL);

      static void activatePackage(StringTableEntry name);
      static void deactivatePackage(StringTableEntry name);
      static void deactivatePackageStack(StringTableEntry name);
      static void dumpClasses( bool dumpScript = true, bool dumpEngine = true );
      static void dumpFunctions( bool dumpScript = true, bool dumpEngine = true );
      static void printNamespaceEntries(Namespace * g, bool dumpScript = true, bool dumpEngine = true);
      static void unlinkPackages();
      static void relinkPackages();
      static bool isPackage(StringTableEntry name);
      static U32 getActivePackagesCount();
      static StringTableEntry getActivePackage(U32 index);
};

typedef VectorPtr<Namespace::Entry *>::iterator NamespaceEntryListIterator;

extern char *typeValueEmpty;

class Dictionary
{
public:

   struct Entry
   {
      enum
      {
         TypeInternalInt = -3,
         TypeInternalFloat = -2,
         TypeInternalString = -1,
      };

      StringTableEntry name;
      Entry *nextEntry;
      S32 type;

      typedef Signal<void()> NotifySignal;

      /// The optional notification signal called when
      /// a value is assigned to this variable.
      NotifySignal *notify;
      
      /// Usage doc string.
      const char* mUsage;
      
      /// Whether this is a constant that cannot be assigned to.
      bool mIsConstant;

   protected:

      // NOTE: This is protected to ensure no one outside
      // of this structure is messing with it.

      #pragma warning( push )
      #pragma warning( disable : 4201 ) // warning C4201: nonstandard extension used : nameless struct/union

      // An variable is either a real dynamic type or
      // its one exposed from C++ using a data pointer.
      //
      // We use this nameless union and struct setup
      // to optimize the memory usage.
      union
      {
         struct
         {
            char *sval;
            U32 ival;  // doubles as strlen when type is TypeInternalString
            F32 fval;
            U32 bufferLen;
         };

         struct
         {
            /// The real data pointer.
            void *dataPtr;

            /// The enum lookup table for enumerated types.
            const EnumTable *enumTable;
         };
      };

      #pragma warning( pop ) // C4201

   public:

      Entry(StringTableEntry name);
      ~Entry();

      U32 getIntValue()
      {
         if(type <= TypeInternalString)
            return ival;
         else
            return dAtoi(Con::getData(type, dataPtr, 0, enumTable));
      }

      F32 getFloatValue()
      {
         if(type <= TypeInternalString)
            return fval;
         else
            return dAtof(Con::getData(type, dataPtr, 0, enumTable));
      }

      const char *getStringValue()
      {
         if(type == TypeInternalString)
            return sval;
         if(type == TypeInternalFloat)
            return Con::getData(TypeF32, &fval, 0);
         else if(type == TypeInternalInt)
            return Con::getData(TypeS32, &ival, 0);
         else
            return Con::getData(type, dataPtr, 0, enumTable);
      }

      void setIntValue(U32 val)
      {
         if( mIsConstant )
         {
            Con::errorf( "Cannot assign value to constant '%s'.", name );
            return;
         }
            
         if(type <= TypeInternalString)
         {
            fval = (F32)val;
            ival = val;
            if(sval != typeValueEmpty)
            {
               dFree(sval);
               sval = typeValueEmpty;
            }
            type = TypeInternalInt;
         }
         else
         {
            const char *dptr = Con::getData(TypeS32, &val, 0);
            Con::setData(type, dataPtr, 0, 1, &dptr, enumTable);
         }

         // Fire off the notification if we have one.
         if ( notify )
            notify->trigger();
      }

      void setFloatValue(F32 val)
      {
         if( mIsConstant )
         {
            Con::errorf( "Cannot assign value to constant '%s'.", name );
            return;
         }

         if(type <= TypeInternalString)
         {
            fval = val;
            ival = static_cast<U32>(val);
            if(sval != typeValueEmpty)
            {
               dFree(sval);
               sval = typeValueEmpty;
            }
            type = TypeInternalFloat;
         }
         else
         {
            const char *dptr = Con::getData(TypeF32, &val, 0);
            Con::setData(type, dataPtr, 0, 1, &dptr, enumTable);
         }

         // Fire off the notification if we have one.
         if ( notify )
            notify->trigger();
      }

      void setStringValue(const char *value);
   };

    struct HashTableData
    {
        Dictionary* owner;
        S32 size;
        S32 count;
        Entry **data;
        FreeListChunker< Entry > mChunker;
        
        HashTableData( Dictionary* owner )
           : owner( owner ), size( 0 ), count( 0 ), data( NULL ) {}
    };

    HashTableData* hashTable;
    HashTableData ownHashTable;
    ExprEvalState *exprState;
    
    StringTableEntry scopeName;
    Namespace *scopeNamespace;
    CodeBlock *code;
    U32 ip;

    Dictionary();
    ~Dictionary();

    Entry *lookup(StringTableEntry name);
    Entry *add(StringTableEntry name);
    void setState(ExprEvalState *state, Dictionary* ref=NULL);
    void remove(Entry *);
    void reset();

    void exportVariables( const char *varString, const char *fileName, bool append );
    void exportVariables( const char *varString, Vector<String> *names, Vector<String> *values );
    void deleteVariables( const char *varString );

    void setVariable(StringTableEntry name, const char *value);
    const char *getVariable(StringTableEntry name, bool *valid = NULL);
    
    U32 getCount() const
    {
      return hashTable->count;
    }
    bool isOwner() const
    {
      return hashTable->owner;
    }

    /// @see Con::addVariable
    Entry* addVariable(    const char *name, 
                           S32 type, 
                           void *dataPtr, 
                           const char* usage );

    /// @see Con::removeVariable
    bool removeVariable(StringTableEntry name);

    /// @see Con::addVariableNotify
    void addVariableNotify( const char *name, const Con::NotifyDelegate &callback );

    /// @see Con::removeVariableNotify
    void removeVariableNotify( const char *name, const Con::NotifyDelegate &callback );

    /// Return the best tab completion for prevText, with the length
    /// of the pre-tab string in baseLen.
    const char *tabComplete(const char *prevText, S32 baseLen, bool);
    
    /// Run integrity checks for debugging.
    void validate();
};

class ExprEvalState
{
public:
    /// @name Expression Evaluation
    /// @{

    ///
    SimObject *thisObject;
    Dictionary::Entry *currentVariable;
    bool traceOn;
    
    U32 mStackDepth;

    ExprEvalState();
    ~ExprEvalState();

    /// @}

    /// @name Stack Management
    /// @{

    /// The stack of callframes.  The extra redirection is necessary since Dictionary holds
    /// an interior pointer that will become invalid when the object changes address.
    Vector< Dictionary* > stack;

    ///
    Dictionary globalVars;
    
    void setCurVarName(StringTableEntry name);
    void setCurVarNameCreate(StringTableEntry name);
    S32 getIntVariable();
    F64 getFloatVariable();
    const char *getStringVariable();
    void setIntVariable(S32 val);
    void setFloatVariable(F64 val);
    void setStringVariable(const char *str);

    void pushFrame(StringTableEntry frameName, Namespace *ns);
    void popFrame();

    /// Puts a reference to an existing stack frame
    /// on the top of the stack.
    void pushFrameRef(S32 stackIndex);
    
    U32 getStackDepth() const
    {
       return mStackDepth;
    }
    
    Dictionary& getCurrentFrame()
    {
      return *( stack[ mStackDepth - 1 ] );
    }

    /// @}
    
    /// Run integrity checks for debugging.
    void validate();
};

namespace Con
{
   /// The current $instantGroup setting.
   extern String gInstantGroup;
}

/// @}

#endif
