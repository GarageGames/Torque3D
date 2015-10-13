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

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#ifndef _PLATFORM_H_
   #include "platform/platform.h"
#endif
#ifndef _BITSET_H_
   #include "core/bitSet.h"
#endif
#ifndef _REFBASE_H_
   #include "core/util/refBase.h"
#endif
#include <stdarg.h>

#include "core/util/str.h"
#include "core/util/journal/journaledSignal.h"

class SimObject;
class Namespace;
struct ConsoleFunctionHeader;

class EngineEnumTable;
typedef EngineEnumTable EnumTable;

typedef U32 StringStackPtr;

template< typename T > S32 TYPEID();


/// @defgroup console_system Console System
/// The Console system is the basis for logging, SimObject, and TorqueScript itself.
///
/// @{

/// Indicates that warnings about undefined script variables should be displayed.
///
/// @note This is set and controlled by script.
extern bool gWarnUndefinedScriptVariables;

enum StringTableConstants
{
   StringTagPrefixByte = 0x01 ///< Magic value prefixed to tagged strings.
};

/// Represents an entry in the log.
struct ConsoleLogEntry
{
   /// This field indicates the severity of the log entry.
   ///
   /// Log entries are filtered and displayed differently based on
   /// their severity. Errors are highlighted red, while normal entries
   /// are displayed as normal text. Often times, the engine will be
   /// configured to hide all log entries except warnings or errors,
   /// or to perform a special notification when it encounters an error.
   enum Level
   {
      Normal = 0,
      Warning,
      Error,
      NUM_CLASS
   } mLevel;

   /// Used to associate a log entry with a module.
   ///
   /// Log entries can come from different sources; for instance,
   /// the scripting engine, or the network code. This allows the
   /// logging system to be aware of where different log entries
   /// originated from.
   enum Type
   {
      General = 0,
      Assert,
      Script,
      GUI,
      Network,
	  GGConnect,
	  NUM_TYPE
   } mType;

   /// Indicates the actual log entry.
   ///
   /// This contains a description of the event being logged.
   /// For instance, "unable to access file", or "player connected
   /// successfully", or nearly anything else you might imagine.
   ///
   /// Typically, the description should contain a concise, descriptive
   /// string describing whatever is being logged. Whenever possible,
   /// include useful details like the name of the file being accessed,
   /// or the id of the player or GuiControl, so that if a log needs
   /// to be used to locate a bug, it can be done as painlessly as
   /// possible.
   const char *mString;
};

typedef const char *StringTableEntry;
extern char *typeValueEmpty;

class ConsoleValue
{
public:
   
   enum
   {
      TypeInternalInt = -5,
      TypeInternalFloat = -4,
      TypeInternalStringStackPtr = -3,
      TypeInternalStackString = -2,
      TypeInternalString = -1,
   };
   
   S32 type;
   
public:
   
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
   
   U32 getIntValue();
   S32 getSignedIntValue();
   F32 getFloatValue();
   const char *getStringValue();
   StringStackPtr getStringStackPtr();
   bool getBoolValue();
   
   void setIntValue(U32 val);
   void setIntValue(S32 val);
   void setFloatValue(F32 val);
   void setStringValue(const char *value);
   void setStackStringValue(const char *value);
   void setStringStackPtrValue(StringStackPtr ptr);
   void setBoolValue(bool val);
   
   void init()
   {
      ival = 0;
      fval = 0;
      sval = typeValueEmpty;
      bufferLen = 0;
      type = TypeInternalString;
   }
   
   void cleanup()
   {
      if ((type <= TypeInternalString) && (bufferLen > 0))
      {
         dFree(sval);
         bufferLen = 0;
      }
      sval = typeValueEmpty;
      type = ConsoleValue::TypeInternalString;
      ival = 0;
      fval = 0;
   }
   ConsoleValue(){ init(); };
   ~ConsoleValue(){ cleanup(); };
};

// Proxy class for console variables
// Can point to existing console variables,
// or act like a free floating value.
class ConsoleValueRef
{
public:
   ConsoleValue *value;

   ConsoleValueRef() : value(0) { ; }
   ~ConsoleValueRef() { ; }

   ConsoleValueRef(const ConsoleValueRef &ref);

   static ConsoleValueRef fromValue(ConsoleValue *value) { ConsoleValueRef ref; ref.value = value; return ref; }

   const char *getStringValue() { return value ? value->getStringValue() : ""; }
   StringStackPtr getStringStackPtrValue() { return value ? value->getStringStackPtr() : 0; }

   inline U32 getIntValue() { return value ? value->getIntValue() : 0; }
   inline S32 getSignedIntValue() { return value ? value->getSignedIntValue() : 0; }
   inline F32 getFloatValue() { return value ? value->getFloatValue() : 0.0f; }
   inline bool getBoolValue() { return value ? value->getBoolValue() : false; }

   inline operator const char*() { return getStringValue(); }
   inline operator String() { return String(getStringValue()); }
   inline operator U32() { return getIntValue(); }
   inline operator S32() { return getSignedIntValue(); }
   inline operator F32() { return getFloatValue(); }
   inline operator bool() { return getBoolValue(); }
   
   inline bool isStringStackPtr() { return value ? value->type == ConsoleValue::TypeInternalStringStackPtr : false; }
   inline bool isString() { return value ? value->type >= ConsoleValue::TypeInternalStringStackPtr : true; }
   inline bool isInt() { return value ? value->type == ConsoleValue::TypeInternalInt : false; }
   inline bool isFloat() { return value ? value->type == ConsoleValue::TypeInternalFloat : false; }
   inline S32 getType() { return value ? value->type : -1; }

   // Note: operators replace value
   ConsoleValueRef& operator=(const ConsoleValueRef &other);
   ConsoleValueRef& operator=(const char *newValue);
   ConsoleValueRef& operator=(U32 newValue);
   ConsoleValueRef& operator=(S32 newValue);
   ConsoleValueRef& operator=(F32 newValue);
   ConsoleValueRef& operator=(F64 newValue);
};

// Overrides to allow ConsoleValueRefs to be directly converted to S32&F32

inline S32 dAtoi(ConsoleValueRef &ref)
{
   return ref.getSignedIntValue();
}

inline F32 dAtof(ConsoleValueRef &ref)
{
   return ref.getFloatValue();
}

inline bool dAtob(ConsoleValue &ref)
{
   return ref.getBoolValue();
}


// Transparently converts ConsoleValue[] to const char**
class StringStackWrapper
{
public:
   const char **argv;
   int argc;

   StringStackWrapper(int targc, ConsoleValueRef targv[]);
   ~StringStackWrapper();

   const char* operator[](int idx) { return argv[idx]; }
   operator const char**() { return argv; }

   int count() { return argc; }
};

// Transparently converts const char** to ConsoleValue
class StringStackConsoleWrapper
{
public:
   ConsoleValue *argvValue;
   ConsoleValueRef *argv;
   int argc;

   StringStackConsoleWrapper(int targc, const char **targv);
   ~StringStackConsoleWrapper();

   ConsoleValueRef& operator[](int idx) { return argv[idx]; }
   operator ConsoleValueRef*() { return argv; }

   int count() { return argc; }
};

/// @defgroup console_callbacks Scripting Engine Callbacks
///
/// The scripting engine makes heavy use of callbacks to represent
/// function exposed to the scripting language. StringCallback,
/// IntCallback, FloatCallback, VoidCallback, and BoolCallback all
/// represent exposed script functions returning different types.
///
/// ConsumerCallback is used with the function Con::addConsumer; functions
/// registered with Con::addConsumer are called whenever something is outputted
/// to the console. For instance, the TelnetConsole registers itself with the
/// console so it can echo the console over the network.
///
/// @note Callbacks to the scripting language - for instance, onExit(), which is
///       a script function called when the engine is shutting down - are handled
///       using Con::executef() and kin.
/// @{

///
typedef const char * (*StringCallback)(SimObject *obj, S32 argc, ConsoleValueRef argv[]);
typedef S32             (*IntCallback)(SimObject *obj, S32 argc, ConsoleValueRef argv[]);
typedef F32           (*FloatCallback)(SimObject *obj, S32 argc, ConsoleValueRef argv[]);
typedef void           (*VoidCallback)(SimObject *obj, S32 argc, ConsoleValueRef argv[]); // We have it return a value so things don't break..
typedef bool           (*BoolCallback)(SimObject *obj, S32 argc, ConsoleValueRef argv[]);

typedef void (*ConsumerCallback)(U32 level, const char *consoleLine);
/// @}

/// @defgroup console_types Scripting Engine Type Functions
///
/// @see Con::registerType
/// @{
typedef const char* (*GetDataFunction)(void *dptr, EnumTable *tbl, BitSet32 flag);
typedef void        (*SetDataFunction)(void *dptr, S32 argc, const char **argv, EnumTable *tbl, BitSet32 flag);
/// @}

/// This namespace contains the core of the console functionality.
///
/// @section con_intro Introduction
///
/// The console is a key part of Torque's architecture. It allows direct run-time control
/// of many aspects of the engine.
///
/// @nosubgrouping
namespace Con
{
   /// Various configuration constants.
   enum Constants 
   {
      /// This is the version number associated with DSO files.
      ///
      /// If you make any changes to the way the scripting language works
      /// (such as DSO format changes, adding/removing op-codes) that would
      /// break compatibility, then you should increment this.
      ///
      /// If you make a really major change, increment it to the next multiple
      /// of ten.
      ///
      /// 12/29/04 - BJG - 33->34 Removed some opcodes, part of namespace upgrade.
      /// 12/30/04 - BJG - 34->35 Reordered some things, further general shuffling.
      /// 11/03/05 - BJG - 35->36 Integrated new debugger code.
      //  09/08/06 - THB - 36->37 New opcode for internal names
      //  09/15/06 - THB - 37->38 Added unit conversions
      //  11/23/06 - THB - 38->39 Added recursive internal name operator
      //  02/15/07 - THB - 39->40 Bumping to 40 for TGB since the console has been
      //                          majorly hacked without the version number being bumped
      //  02/16/07 - THB - 40->41 newmsg operator
      //  06/15/07 - THB - 41->42 script types
      /// 07/31/07 - THB - 42->43 Patch from Andreas Kirsch: Added opcode to support nested new declarations.
      /// 09/12/07 - CAF - 43->44 remove newmsg operator
      /// 09/27/07 - RDB - 44->45 Patch from Andreas Kirsch: Added opcode to support correct void return
      /// 01/13/09 - TMS - 45->46 Added script assert
      /// 09/07/14 - jamesu - 46->47 64bit support
      /// 10/14/14 - jamesu - 47->48 Added opcodes to reduce reliance on strings in function calls
      DSOVersion = 48,

      MaxLineLength = 512,  ///< Maximum length of a line of console input.
      MaxDataTypes = 256    ///< Maximum number of registered data types.
   };

   /// @name Control Functions
   ///
   /// The console must be initialized and shutdown appropriately during the
   /// lifetime of the app. These functions are used to manage this behavior.
   ///
   /// @note Torque deals with this aspect of console management, so you don't need
   ///       to call these functions in normal usage of the engine.
   /// @{

   /// Initializes the console.
   ///
   /// This performs the following steps:
   ///   - Calls Namespace::init() to initialize the scripting namespace hierarchy.
   ///   - Calls ConsoleConstructor::setup() to initialize globally defined console
   ///     methods and functions.
   ///   - Registers some basic global script variables.
   ///   - Calls AbstractClassRep::init() to initialize Torque's class database.
   ///   - Registers some basic global script functions that couldn't usefully
   ///     be defined anywhere else.
   void init();

   /// Shuts down the console.
   ///
   /// This performs the following steps:
   ///   - Closes the console log file.
   ///   - Calls Namespace::shutdown() to shut down the scripting namespace hierarchy.
   void shutdown();

   /// Is the console active at this time?
   bool isActive();

   /// @}

   /// @name Console Consumers
   ///
   /// The console distributes its output through Torque by using
   /// consumers. Every time a new line is printed to the console,
   /// all the ConsumerCallbacks registered using addConsumer are
   /// called, in order.
   ///
   /// @note The GuiConsole control, which provides the on-screen
   ///       in-game console, uses a different technique to render
   ///       the console. It calls getLockLog() to lock the Vector
   ///       of on-screen console entries, then it renders them as
   ///       needed. While the Vector is locked, the console will
   ///       not change the Vector. When the GuiConsole control is
   ///       done with the console entries, it calls unlockLog()
   ///       to tell the console that it is again safe to modify
   ///       the Vector.
   ///
   /// @see TelnetConsole
   /// @see TelnetDebugger
   /// @see WinConsole
   /// @see MacCarbConsole
   /// @see StdConsole
   /// @see ConsoleLogger
   ///
   /// @{

   ///
   void addConsumer(ConsumerCallback cb);
   void removeConsumer(ConsumerCallback cb);

   typedef JournaledSignal<void(RawData)> ConsoleInputEvent;

   /// Called from the native consoles to provide lines of console input
   /// to process. This will schedule it for execution ASAP.
   extern ConsoleInputEvent smConsoleInput;

   /// @}

   /// @name Miscellaneous
   /// @{

   /// Remove color marking information from a string.
   ///
   /// @note It does this in-place, so be careful! It may
   ///       potentially blast data if you're not careful.
   ///       When in doubt, make a copy of the string first.
   void stripColorChars(char* line);

   /// Convert from a relative script path to an absolute script path.
   ///
   /// This is used in (among other places) the exec() script function, which
   /// takes a parameter indicating a script file and executes it. Script paths
   /// can be one of:
   ///      - <b>Absolute:</b> <i>fps/foo/bar.cs</i> Paths of this sort are passed
   ///        through.
   ///      - <b>Mod-relative:</b> <i>~/foo/bar.cs</i> Paths of this sort have their
   ///        replaced with the name of the current mod.
   ///      - <b>File-relative:</b> <i>./baz/blip.cs</i> Paths of this sort are
   ///        calculated relative to the path of the current scripting file.
   ///
   /// @note This function determines paths relative to the currently executing
   ///       CodeBlock. Calling it outside of script execution will result in
   ///       it directly copying src to filename, since it won't know to what the
   ///       path is relative!
   ///
   /// @param  filename    Pointer to string buffer to fill with absolute path.
   /// @param  size        Size of buffer pointed to by filename.
   /// @param  src         Original, possibly relative script path.
   bool expandScriptFilename(char *filename, U32 size, const char *src);
   bool expandGameScriptFilename(char *filename, U32 size, const char *src);
   bool expandToolScriptFilename(char *filename, U32 size, const char *src);
   bool collapseScriptFilename(char *filename, U32 size, const char *src);

   bool expandPath(char* pDstPath, U32 size, const char* pSrcPath, const char* pWorkingDirectoryHint = NULL, const bool ensureTrailingSlash = false);
   void collapsePath(char* pDstPath, U32 size, const char* pSrcPath, const char* pWorkingDirectoryHint = NULL);
   bool isBasePath(const char* SrcPath, const char* pBasePath);
   void ensureTrailingSlash(char* pDstPath, const char* pSrcPath);
   bool stripRepeatSlashes(char* pDstPath, const char* pSrcPath, S32 dstSize);

   void addPathExpando(const char* pExpandoName, const char* pPath);
   void removePathExpando(const char* pExpandoName);
   bool isPathExpando(const char* pExpandoName);
   StringTableEntry getPathExpando(const char* pExpandoName);
   U32 getPathExpandoCount(void);
   StringTableEntry getPathExpandoKey(U32 expandoIndex);
   StringTableEntry getPathExpandoValue(U32 expandoIndex);

   bool isCurrentScriptToolScript();

   StringTableEntry getModNameFromPath(const char *path);

   /// Returns true if fn is a global scripting function.
   ///
   /// This looks in the global namespace. It also checks to see if fn
   /// is in the StringTable; if not, it returns false.
   bool isFunction(const char *fn);

   /// This is the basis for tab completion in the console.
   ///
   /// @note This is an internally used function. You probably don't
   ///       care much about how this works.
   ///
   /// This function does some basic parsing to try to ascertain the namespace in which
   /// we are attempting to do tab completion, then bumps control off to the appropriate
   /// tabComplete function, either in SimObject or Namespace.
   ///
   /// @param  inputBuffer     Pointer to buffer containing starting data, or last result.
   /// @param  cursorPos       Location of cursor in this buffer. This is used to indicate
   ///                         what part of the string should be kept and what part should
   ///                         be advanced to the next match if any.
   /// @param  maxResultLength Maximum amount of result data to put into inputBuffer. This
   ///                         is capped by MaxCompletionBufferSize.
   /// @param  forwardTab      Should we go forward to next match or backwards to previous
   ///                         match? True indicates forward.
   U32 tabComplete(char* inputBuffer, U32 cursorPos, U32 maxResultLength, bool forwardTab);

   /// @}


   /// @name Variable Management
   /// @{

   /// The delegate signature for the variable assignment notifications.
   ///
   /// @see addVariableNotify, removeVariableNotify
   typedef Delegate<void()> NotifyDelegate;

   /// Add a console variable that references the value of a variable in C++ code.
   ///
   /// If a value is assigned to the console variable the C++ variable is updated,
   /// and vice-versa.
   ///
   /// @param name      Global console variable name to create.
   /// @param type      The type of the C++ variable; see the ConsoleDynamicTypes enum for a complete list.
   /// @param pointer   Pointer to the variable.
   /// @param usage     Documentation string.
   ///
   /// @see ConsoleDynamicTypes
   void addVariable( const char *name, 
                     S32 type, 
                     void *pointer, 
                     const char* usage = NULL );
                     
   /// Add a console constant that references the value of a constant in C++ code.
   ///
   /// @param name      Global console constant name to create.
   /// @param type      The type of the C++ constant; see the ConsoleDynamicTypes enum for a complete list.
   /// @param pointer   Pointer to the constant.
   /// @param usage     Documentation string.
   ///
   /// @see ConsoleDynamicTypes
   void addConstant( const char *name, 
                     S32 type, 
                     const void *pointer, 
                     const char* usage = NULL );
                     
   /// Remove a console variable.
   ///
   /// @param name   Global console variable name to remove
   /// @return       true if variable existed before removal.
   bool removeVariable(const char *name);

   /// Add a callback for notification when a variable
   /// is assigned a new value.
   ///
   /// @param name      An existing global console variable name.
   /// @param callback  The notification delegate function.
   ///
   void addVariableNotify( const char *name, const NotifyDelegate &callback );

   /// Remove an existing variable assignment notification callback.
   ///
   /// @param name      An existing global console variable name.
   /// @param callback  The notification delegate function.
   ///
   void removeVariableNotify( const char *name, const NotifyDelegate &callback );

   /// Assign a string value to a locally scoped console variable
   ///
   /// @note The context of the variable is determined by gEvalState; that is,
   ///       by the currently executing code.
   ///
   /// @param name   Local console variable name to set
   /// @param value  String value to assign to name
   void setLocalVariable(const char *name, const char *value);

   /// Retrieve the string value to a locally scoped console variable
   ///
   /// @note The context of the variable is determined by gEvalState; that is,
   ///       by the currently executing code.
   ///
   /// @param name   Local console variable name to get
   const char* getLocalVariable(const char* name);

   /// @}

   /// @name Global Variable Accessors
   /// @{
   /// Assign a string value to a global console variable
   /// @param name   Global console variable name to set
   /// @param value  String value to assign to this variable.
   void setVariable(const char *name, const char *value);

   /// Retrieve the string value of a global console variable
   /// @param name   Global Console variable name to query
   /// @return       The string value of the variable or "" if the variable does not exist.
   const char* getVariable(const char* name);

   /// Retrieve the string value of an object field
   /// @param name   "object.field" string to query
   /// @return       The string value of the variable or NULL if no object is specified
   const char* getObjectField(const char* name);

   /// Same as setVariable(), but for bools.
   void setBoolVariable (const char* name,bool var);

   /// Same as getVariable(), but for bools.
   ///
   /// @param  name  Name of the variable.
   /// @param  def   Default value to supply if no matching variable is found.
   bool getBoolVariable (const char* name,bool def = false);

   /// Same as setVariable(), but for ints.
   void setIntVariable  (const char* name,S32 var);

   /// Same as getVariable(), but for ints.
   ///
   /// @param  name  Name of the variable.
   /// @param  def   Default value to supply if no matching variable is found.
   S32  getIntVariable  (const char* name,S32 def = 0);

   /// Same as setVariable(), but for floats.
   void setFloatVariable(const char* name,F32 var);

   /// Same as getVariable(), but for floats.
   ///
   /// @param  name  Name of the variable.
   /// @param  def   Default value to supply if no matching variable is found.
   F32  getFloatVariable(const char* name,F32 def = .0f);

   /// @}

   /// @name Global Function Registration
   /// @{

   /// Register a C++ function with the console making it a global function callable from the scripting engine.
   ///
   /// @param name      Name of the new function.
   /// @param cb        Pointer to the function implementing the scripting call; a console callback function returning a specific type value.
   /// @param usage     Documentation for this function. @ref console_autodoc
   /// @param minArgs   Minimum number of arguments this function accepts
   /// @param maxArgs   Maximum number of arguments this function accepts
   /// @param toolOnly  Wether this is a TORQUE_TOOLS only function.
   /// @param header    The extended function header.
   void addCommand( const char* name, StringCallback cb, const char* usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL );

   void addCommand( const char* name, IntCallback    cb, const char* usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL ); ///< @copydoc addCommand( const char *, StringCallback, const char *, S32, S32, bool, ConsoleFunctionHeader* )
   void addCommand( const char* name, FloatCallback  cb, const char* usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL ); ///< @copydoc addCommand( const char *, StringCallback, const char *, S32, S32, bool, ConsoleFunctionHeader* )
   void addCommand( const char* name, VoidCallback   cb, const char* usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL ); ///< @copydoc addCommand( const char *, StringCallback, const char *, S32, S32, bool, ConsoleFunctionHeader* )
   void addCommand( const char* name, BoolCallback   cb, const char* usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL ); ///< @copydoc addCommand( const char *, StringCallback, const char *, S32, S32, bool, ConsoleFunctionHeader* )
   
   /// @}

   /// @name Namespace Function Registration
   /// @{

   /// Register a C++ function with the console making it callable
   /// as a method of the given namespace from the scripting engine.
   ///
   /// @param nameSpace Name of the namespace to associate the new function with; this is usually the name of a class.
   /// @param name      Name of the new function.
   /// @param cb        Pointer to the function implementing the scripting call; a console callback function returning a specific type value.
   /// @param usage     Documentation for this function. @ref console_autodoc
   /// @param minArgs   Minimum number of arguments this function accepts
   /// @param maxArgs   Maximum number of arguments this function accepts
   /// @param toolOnly  Wether this is a TORQUE_TOOLS only function.
   /// @param header    The extended function header.
   void addCommand(const char *nameSpace, const char *name,StringCallback cb, const char *usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL );

   void addCommand(const char *nameSpace, const char *name,IntCallback cb,    const char *usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL ); ///< @copydoc addCommand( const char*, const char *, StringCallback, const char *, S32, S32, bool, ConsoleFunctionHeader* )
   void addCommand(const char *nameSpace, const char *name,FloatCallback cb,  const char *usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL ); ///< @copydoc addCommand( const char*, const char *, StringCallback, const char *, S32, S32, bool, ConsoleFunctionHeader* )
   void addCommand(const char *nameSpace, const char *name,VoidCallback cb,   const char *usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL ); ///< @copydoc addCommand( const char*, const char *, StringCallback, const char *, S32, S32, bool, ConsoleFunctionHeader* )
   void addCommand(const char *nameSpace, const char *name,BoolCallback cb,   const char *usage, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL ); ///< @copydoc addCommand( const char*, const char *, StringCallback, const char *, S32, S32, bool, ConsoleFunctionHeader* )

   /// @}

   /// @name Special Purpose Registration
   ///
   /// These are special-purpose functions that exist to allow commands to be grouped, so
   /// that when we generate console docs, they can be more meaningfully presented.
   ///
   /// @ref console_autodoc "Click here for more information about console docs and grouping."
   ///
   /// @{

   void markCommandGroup (const char * nsName, const char *name, const char* usage=NULL);
   void beginCommandGroup(const char * nsName, const char *name, const char* usage);
   void endCommandGroup  (const char * nsName, const char *name);

   void noteScriptCallback( const char *className, const char *funcName, const char *usage, ConsoleFunctionHeader* header = NULL );

   /// @}

   /// @name Console Output
   ///
   /// These functions process the formatted string and pass it to all the ConsumerCallbacks that are
   /// currently registered. The console log file and the console window callbacks are installed by default.
   ///
   /// @see addConsumer()
   /// @see removeConsumer()
   /// @{

   /// @param _format   A stdlib printf style formatted out put string
   /// @param ...       Variables to be written
   void printf(const char *_format, ...);

   /// @note The console window colors warning text as LIGHT GRAY.
   /// @param _format   A stdlib printf style formatted out put string
   /// @param ...       Variables to be written
   void warnf(const char *_format, ...);

   /// @note The console window colors warning text as RED.
   /// @param _format   A stdlib printf style formatted out put string
   /// @param ...       Variables to be written
   void errorf(const char *_format, ...);

   /// @note The console window colors warning text as LIGHT GRAY.
   /// @param type      Allows you to associate the warning message with an internal module.
   /// @param _format   A stdlib printf style formatted out put string
   /// @param ...       Variables to be written
   /// @see Con::warnf()
   void warnf(ConsoleLogEntry::Type type, const char *_format, ...);

   /// @note The console window colors warning text as RED.
   /// @param type      Allows you to associate the warning message with an internal module.
   /// @param _format   A stdlib printf style formatted out put string
   /// @param ...       Variables to be written
   /// @see Con::errorf()
   void errorf(ConsoleLogEntry::Type type, const char *_format, ...);

   //some additions from t2d
   /// Prints a separator to the console.
   inline void printSeparator(void) { printf("--------------------------------------------------------------------------------"); }

   /// Prints a separator to the console.
   inline void printBlankLine(void) { printf(""); }

   /// @}

   /// Returns true when called from the main thread, false otherwise
   bool isMainThread();


   /// @name Console Execution
   ///
   /// These are functions relating to the execution of script code.
   ///
   /// @{

   /// Call a script function from C/C++ code.
   ///
   /// @param argc      Number of elements in the argv parameter
   /// @param argv      A character string array containing the name of the function
   ///                  to call followed by the arguments to that function.
   /// @code
   /// // Call a Torque script function called mAbs, having one parameter.
   /// char* argv[] = {"abs", "-9"};
   /// char* result = execute(2, argv);
   /// @endcode
   /// NOTE: this function restores the console stack on return.
   ConsoleValueRef execute(S32 argc, const char* argv[]);
   ConsoleValueRef execute(S32 argc, ConsoleValueRef argv[]);

   /// Call a Torque Script member function of a SimObject from C/C++ code.
   /// @param object    Object on which to execute the method call.
   /// @param argc      Number of elements in the argv parameter (must be >2, see argv)
   /// @param argv      A character string array containing the name of the member function
   ///                  to call followed by an empty parameter (gets filled with object ID)
   ///                  followed by arguments to that function.
   /// @code
   /// // Call the method setMode() on an object, passing it one parameter.
   ///
   /// char* argv[] = {"setMode", "", "2"};
   /// char* result = execute(mysimobject, 3, argv);
   /// @endcode
   /// NOTE: this function restores the console stack on return.
   ConsoleValueRef execute(SimObject *object, S32 argc, const char* argv[], bool thisCallOnly = false);
   ConsoleValueRef execute(SimObject *object, S32 argc, ConsoleValueRef argv[], bool thisCallOnly = false);

   /// Evaluate an arbitrary chunk of code.
   ///
   /// @param  string   Buffer containing code to execute.
   /// @param  echo     Should we echo the string to the console?
   /// @param  fileName Indicate what file this code is coming from; used in error reporting and such.
   /// NOTE: This function restores the console stack on return.
   ConsoleValueRef evaluate(const char* string, bool echo = false, const char *fileName = NULL);

   /// Evaluate an arbitrary line of script.
   ///
   /// This wraps dVsprintf(), so you can substitute parameters into the code being executed.
   /// NOTE: This function restores the console stack on return.
   ConsoleValueRef evaluatef(const char* string, ...);

   /// @}

   /// @name Console Function Implementation Helpers
   ///
   /// The functions Con::getIntArg, Con::getFloatArg and Con::getArgBuffer(size) are used to
   /// allocate on the console stack string variables that will be passed into the next console
   //  function called.  This allows the console to avoid copying some data.
   ///
   /// getReturnBuffer lets you allocate stack space to return data in.
   /// @{

   ///
   char* getReturnBuffer(U32 bufferSize);
   char* getReturnBuffer(const char *stringToCopy);
   char* getReturnBuffer( const String& str );
   char* getReturnBuffer( const StringBuilder& str );

   char* getArgBuffer(U32 bufferSize);
   char* getFloatArg(F64 arg);
   char* getIntArg  (S32 arg);
   char* getBoolArg(bool arg);
   char* getStringArg( const char* arg );
   char* getStringArg( const String& arg );
   /// @}

   /// @name Namespaces
   /// @{

   Namespace *lookupNamespace(const char *nsName);
   bool linkNamespaces(const char *parentName, const char *childName);
   bool unlinkNamespaces(const char *parentName, const char *childName);

   /// @note This should only be called from consoleObject.h
   bool classLinkNamespaces(Namespace *parent, Namespace *child);

   const char *getNamespaceList(Namespace *ns);
   /// @}

   /// @name Logging
   /// @{

   void getLockLog(ConsoleLogEntry * &log, U32 &size);
   void unlockLog(void);
   void setLogMode(S32 mode);

   /// @}

   /// @name Instant Group
   /// @{

   void pushInstantGroup( String name = String() );
   void popInstantGroup();

   /// @}

   /// @name Dynamic Type System
   /// @{

   void setData(S32 type, void *dptr, S32 index, S32 argc, const char **argv, const EnumTable *tbl = NULL, BitSet32 flag = 0);
   const char *getData(S32 type, void *dptr, S32 index, const EnumTable *tbl = NULL, BitSet32 flag = 0);
   const char *getFormattedData(S32 type, const char *data, const EnumTable *tbl = NULL, BitSet32 flag = 0);

   /// @}
};

struct _EngineConsoleCallbackHelper;
template<typename P1> struct _EngineConsoleExecCallbackHelper;

namespace Con
{
	/// @name Console Execution - executef
	/// {
	///
	/// Implements a script function thunk which automatically converts parameters to relevant console types.
	/// Can be used as follows:
	/// - Con::executef("functionName", ...);
	/// - Con::executef(mySimObject, "functionName", ...);
	/// 
	/// NOTE: if you get a rather cryptic template error coming through here, most likely you are trying to 
	/// convert a parameter which EngineMarshallType does not have a specialization for.
	/// Another problem can occur if you do not include "console/simBase.h" and "console/engineAPI.h" 
	/// since _EngineConsoleExecCallbackHelper and SimConsoleThreadExecCallback are required.
	///
	/// @see _EngineConsoleExecCallbackHelper
	///
	template<typename A> ConsoleValueRef executef(A a) { _EngineConsoleExecCallbackHelper<A> callback( a ); return callback.template call<ConsoleValueRef>(); }
	template<typename A, typename B> ConsoleValueRef executef(A a, B b) { _EngineConsoleExecCallbackHelper<A> callback( a ); return callback.template call<ConsoleValueRef>(b); }
	template<typename A, typename B, typename C> ConsoleValueRef executef(A a, B b, C c) { _EngineConsoleExecCallbackHelper<A> callback( a ); return callback.template call<ConsoleValueRef>(b, c); }
	template<typename A, typename B, typename C, typename D> ConsoleValueRef executef(A a, B b, C c, D d) { _EngineConsoleExecCallbackHelper<A> callback( a ); return callback.template call<ConsoleValueRef>(b, c, d); }
	template<typename A, typename B, typename C, typename D, typename E> ConsoleValueRef executef(A a, B b, C c, D d, E e) { _EngineConsoleExecCallbackHelper<A> callback( a ); return callback.template call<ConsoleValueRef>(b, c, d, e); }
	template<typename A, typename B, typename C, typename D, typename E, typename F> ConsoleValueRef executef(A a, B b, C c, D d, E e, F f) { _EngineConsoleExecCallbackHelper<A> callback( a ); return callback.template call<ConsoleValueRef>(b, c, d, e, f); }
	template<typename A, typename B, typename C, typename D, typename E, typename F, typename G> ConsoleValueRef executef(A a, B b, C c, D d, E e, F f, G g) { _EngineConsoleExecCallbackHelper<A> callback( a ); return callback.template call<ConsoleValueRef>(b, c, d, e, f, g); }
	template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H> ConsoleValueRef executef(A a, B b, C c, D d, E e, F f, G g, H h) { _EngineConsoleExecCallbackHelper<A> callback( a ); return callback.template call<ConsoleValueRef>(b, c, d, e, f, g, h); }
	template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I> ConsoleValueRef executef(A a, B b, C c, D d, E e, F f, G g, H h, I i) { _EngineConsoleExecCallbackHelper<A> callback( a ); return callback.template call<ConsoleValueRef>(b, c, d, e, f, g, h, i); }
	template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J> ConsoleValueRef executef(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) { _EngineConsoleExecCallbackHelper<A> callback( a ); return callback.template call<ConsoleValueRef>(b, c, d, e, f, g, h, i, j); }
	template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K> ConsoleValueRef executef(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) { _EngineConsoleExecCallbackHelper<A> callback( a ); return callback.template call<ConsoleValueRef>(b, c, d, e, f, g, h, i, j, k); }
	template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L> ConsoleValueRef executef(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) { _EngineConsoleExecCallbackHelper<A> callback( a ); return callback.template call<ConsoleValueRef>(b, c, d, e, f, g, h, i, j, k, l); }
	/// }
};

extern void expandEscape(char *dest, const char *src);
extern bool collapseEscape(char *buf);
extern U32 HashPointer(StringTableEntry ptr);


/// Extended information about a console function.
struct ConsoleFunctionHeader
{
   /// Return type string.
   const char* mReturnString;
   
   /// List of arguments taken by the function.  Used for documentation.
   const char* mArgString;
   
   /// List of default argument values.  Used for documentation.
   const char* mDefaultArgString;
   
   /// Whether this is a static method in a class.
   bool mIsStatic;
   
   ConsoleFunctionHeader(
      const char* returnString,
      const char* argString,
      const char* defaultArgString,
      bool isStatic = false )
      : mReturnString( returnString ),
        mArgString( argString ),
        mDefaultArgString( defaultArgString ),
        mIsStatic( isStatic ) {}
};


/// This is the backend for the ConsoleMethod()/ConsoleFunction() macros.
///
/// See the group ConsoleConstructor Innards for specifics on how this works.
///
/// @see @ref console_autodoc
/// @nosubgrouping
class ConsoleConstructor
{
public:
   /// @name Entry Type Fields
   ///
   /// One of these is set based on the type of entry we want
   /// inserted in the console.
   ///
   /// @ref console_autodoc
   /// @{
   
   StringCallback sc;   ///< A function/method that returns a string.
   IntCallback ic;      ///< A function/method that returns an int.
   FloatCallback fc;    ///< A function/method that returns a float.
   VoidCallback vc;     ///< A function/method that returns nothing.
   BoolCallback bc;     ///< A function/method that returns a bool.
   bool group;          ///< Indicates that this is a group marker.
   bool ns;             ///< Indicates that this is a namespace marker.
                        ///  @deprecated Unused.
   bool callback;       ///< Is this a callback into script?
   
   /// @}

   /// Minimum number of arguments expected by the function.
   S32 mina;
   
   /// Maximum number of arguments accepted by the funtion.  Zero for varargs.
   S32 maxa;
      
   /// Name of the function/method.
   const char* funcName;
   
   /// Name of the class namespace to which to add the method.
   const char* className;

   /// Usage string for documentation.
   const char* usage;

   /// Whether this is a TORQUE_TOOLS only function.
   bool toolOnly;
   
   /// The extended function header.
   ConsoleFunctionHeader* header;
   
   /// @name ConsoleConstructor Innards
   ///
   /// The ConsoleConstructor class is used as the backend for the ConsoleFunction() and
   /// ConsoleMethod() macros. The way it works takes advantage of several properties of
   /// C++.
   ///
   /// The ConsoleFunction()/ConsoleMethod() macros wrap the declaration of a ConsoleConstructor.
   ///
   /// @code
   ///      // The definition of a ConsoleFunction using the macro
   ///      ConsoleFunction(ExpandFilename, const char*, 2, 2, "(string filename)")
   ///      {
   ///         argc;
   ///         char* ret = Con::getReturnBuffer( 1024 );
   ///         Con::expandScriptFilename(ret, 1024, argv[1]);
   ///         return ret;
   ///      }
   ///
   ///      // Resulting code
   ///      static const char* cExpandFilename(SimObject *, S32, const char **argv);
   ///      static ConsoleConstructor
   ///            gExpandFilenameobj(NULL,"ExpandFilename", cExpandFilename,
   ///            "(string filename)", 2, 2);
   ///      static const char* cExpandFilename(SimObject *, S32 argc, const char **argv)
   ///      {
   ///         argc;
   ///         char* ret = Con::getReturnBuffer( 1024 );
   ///         Con::expandScriptFilename(ret, 1024, argv[1]);
   ///         return ret;
   ///      }
   ///
   ///      // A similar thing happens when you do a ConsoleMethod.
   /// @endcode
   ///
   /// As you can see, several global items are defined when you use the ConsoleFunction method.
   /// The macro constructs the name of these items from the parameters you passed it. Your
   /// implementation of the console function is is placed in a function with a name based on
   /// the actual name of the console funnction. In addition, a ConsoleConstructor is declared.
   ///
   /// Because it is defined as a global, the constructor for the ConsoleConstructor is called
   /// before execution of main() is started. The constructor is called once for each global
   /// ConsoleConstructor variable, in the order in which they were defined (this property only holds true
   /// within file scope).
   ///
   /// We have ConsoleConstructor create a linked list at constructor time, by storing a static
   /// pointer to the head of the list, and keeping a pointer to the next item in each instance
   /// of ConsoleConstructor. init() is a helper function in this process, automatically filling
   /// in commonly used fields and updating first and next as needed. In this way, a list of
   /// items to add to the console is assemble in memory, ready for use, before we start
   /// execution of the program proper.
   ///
   /// In Con::init(), ConsoleConstructor::setup() is called to process this prepared list. Each
   /// item in the list is iterated over, and the appropriate Con namespace functions (usually
   /// Con::addCommand) are invoked to register the ConsoleFunctions and ConsoleMethods in
   /// the appropriate namespaces.
   ///
   /// @see Namespace
   /// @see Con
   /// @{

   ///
   ConsoleConstructor *next;
   static ConsoleConstructor *first;

   void init( const char* cName, const char* fName, const char *usg, S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL );

   static void setup();

   /// Validate there are no duplicate entries for this item.
   void validate();

   /// @}

   /// @name Basic Console Constructors
   /// @{

   ConsoleConstructor( const char* className, const char* funcName, StringCallback sfunc, const char* usage,  S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL );
   ConsoleConstructor( const char* className, const char* funcName, IntCallback    ifunc, const char* usage,  S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL );
   ConsoleConstructor( const char* className, const char* funcName, FloatCallback  ffunc, const char* usage,  S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL );
   ConsoleConstructor( const char* className, const char* funcName, VoidCallback   vfunc, const char* usage,  S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL );
   ConsoleConstructor( const char* className, const char* funcName, BoolCallback   bfunc, const char* usage,  S32 minArgs, S32 maxArgs, bool toolOnly = false, ConsoleFunctionHeader* header = NULL );
   
   /// @}

   /// @name Magic Console Constructors
   ///
   /// These perform various pieces of "magic" related to consoleDoc functionality.
   /// @ref console_autodoc
   /// @{

   /// Indicates a group marker. (A doxygen illusion)
   ///
   /// @see Con::markCommandGroup
   /// @ref console_autodoc
   ConsoleConstructor( const char *className, const char *groupName, const char* usage );

   /// Indicates a callback declared with the DECLARE_SCRIPT_CALLBACK macro and friends.
   ConsoleConstructor( const char *className, const char *callbackName, const char *usage, ConsoleFunctionHeader* header );

   /// @}
};


/// An arbitrary fragment of auto-doc text for the script reference.
struct ConsoleDocFragment
{
   /// The class in which to put the fragment.  If NULL, the fragment
   /// will be placed globally.
   const char* mClass;
   
   /// The definition to output for this fragment.  NULL for fragments
   /// not associated with a definition.
   const char* mDefinition;
   
   /// The documentation text.
   const char* mText;
   
   /// Next fragment in the global link chain.
   ConsoleDocFragment* mNext;
   
   /// First fragment in the global link chain.
   static ConsoleDocFragment* smFirst;
   
   ConsoleDocFragment( const char* text, const char* inClass = NULL, const char* definition = NULL )
      : mClass( inClass ),
        mDefinition( definition ),
        mText( text ),
        mNext( smFirst )
   {
      smFirst = this;
   }
};


/// Utility class to save and restore the current console stack frame
///
class ConsoleStackFrameSaver
{
public:

	bool mSaved;

	ConsoleStackFrameSaver() : mSaved(false)
	{
	}

	~ConsoleStackFrameSaver()
	{
		restore();
	}

	void save();
	void restore();
};


/// @name Global Console Definition Macros
///
/// @note If TORQUE_DEBUG is defined, then we gather documentation information, and
///       do some extra sanity checks.
///
/// @see ConsoleConstructor
/// @ref console_autodoc
/// @{


/// Define a C++ method that calls back to script on an object.
/// 
/// @see consoleCallback.h
#define DECLARE_CALLBACK( returnType, name, args )  \
   virtual returnType name ## _callback args

// O hackery of hackeries
#define conmethod_return_const              return (const
#define conmethod_return_S32                return (S32
#define conmethod_return_F32                return (F32
#define conmethod_nullify(val)
#define conmethod_return_void               conmethod_nullify(void
#define conmethod_return_bool               return (bool

#if !defined(TORQUE_SHIPPING)

// Console function macros
#  define ConsoleFunctionGroupBegin(groupName, usage) \
      static ConsoleConstructor cfg_ConsoleFunctionGroup_##groupName##_GroupBegin(NULL,#groupName,usage)

#  define ConsoleFunction(name,returnType,minArgs,maxArgs,usage1) \
   returnType cf_##name(SimObject *, S32, ConsoleValueRef *argv); \
   ConsoleConstructor cc_##name##_obj(NULL,#name,cf_##name,usage1,minArgs,maxArgs); \
      returnType cf_##name(SimObject *, S32 argc, ConsoleValueRef *argv)

#  define ConsoleToolFunction(name,returnType,minArgs,maxArgs,usage1) \
   returnType ctf_##name(SimObject *, S32, ConsoleValueRef *argv); \
   ConsoleConstructor cc_##name##_obj(NULL,#name,ctf_##name,usage1,minArgs,maxArgs, true); \
   returnType ctf_##name(SimObject *, S32 argc, ConsoleValueRef *argv)

#  define ConsoleFunctionGroupEnd(groupName) \
      static ConsoleConstructor cfg_##groupName##_GroupEnd(NULL,#groupName,NULL)

// Console method macros
#  define ConsoleNamespace(className, usage) \
   ConsoleConstructor cc_##className##_Namespace(#className, usage)

#  define ConsoleMethodGroupBegin(className, groupName, usage) \
   static ConsoleConstructor cc_##className##_##groupName##_GroupBegin(#className,#groupName,usage)

#  define ConsoleMethod(className,name,returnType,minArgs,maxArgs,usage1) \
   inline returnType cm_##className##_##name(className *, S32, ConsoleValueRef *argv); \
   returnType cm_##className##_##name##_caster(SimObject *object, S32 argc, ConsoleValueRef *argv) { \
         AssertFatal( dynamic_cast<className*>( object ), "Object passed to " #name " is not a " #className "!" ); \
         conmethod_return_##returnType ) cm_##className##_##name(static_cast<className*>(object),argc,argv); \
      };                                                                                              \
      ConsoleConstructor cc_##className##_##name##_obj(#className,#name,cm_##className##_##name##_caster,usage1,minArgs,maxArgs); \
      inline returnType cm_##className##_##name(className *object, S32 argc, ConsoleValueRef *argv)

#  define ConsoleStaticMethod(className,name,returnType,minArgs,maxArgs,usage1) \
   inline returnType cm_##className##_##name(S32, ConsoleValueRef *); \
   returnType cm_##className##_##name##_caster(SimObject *object, S32 argc, ConsoleValueRef *argv) { \
   conmethod_return_##returnType ) cm_##className##_##name(argc,argv); \
   }; \
   ConsoleConstructor \
   cc_##className##_##name##_obj(#className,#name,cm_##className##_##name##_caster,usage1,minArgs,maxArgs); \
   inline returnType cm_##className##_##name(S32 argc, ConsoleValueRef *argv)

#  define ConsoleMethodGroupEnd(className, groupName) \
   static ConsoleConstructor cc_##className##_##groupName##_GroupEnd(#className,#groupName,NULL)
   
/// Add a fragment of auto-doc text to the console API reference.
/// @note There can only be one ConsoleDoc per source file.
#  define ConsoleDoc( text )                                \
      namespace {                                           \
         ConsoleDocFragment _sDocFragment( text );           \
      }

#else

// These do nothing if we don't want doc information.
#  define ConsoleFunctionGroupBegin(groupName, usage)
#  define ConsoleFunctionGroupEnd(groupName)
#  define ConsoleNamespace(className, usage)
#  define ConsoleMethodGroupBegin(className, groupName, usage)
#  define ConsoleMethodGroupEnd(className, groupName)

// These are identical to what's above, we just want to null out the usage strings.
#  define ConsoleFunction(name,returnType,minArgs,maxArgs,usage1)                   \
      static returnType c##name(SimObject *, S32, ConsoleValueRef*);                   \
      static ConsoleConstructor g##name##obj(NULL,#name,c##name,"",minArgs,maxArgs);\
      static returnType c##name(SimObject *, S32 argc, ConsoleValueRef *argv)

#  define ConsoleToolFunction(name,returnType,minArgs,maxArgs,usage1)                   \
   static returnType c##name(SimObject *, S32, ConsoleValueRef*);                   \
   static ConsoleConstructor g##name##obj(NULL,#name,c##name,"",minArgs,maxArgs, true);\
   static returnType c##name(SimObject *, S32 argc, ConsoleValueRef *argv)

#  define ConsoleMethod(className,name,returnType,minArgs,maxArgs,usage1)                             \
      static inline returnType c##className##name(className *, S32, ConsoleValueRef *argv);               \
      static returnType c##className##name##caster(SimObject *object, S32 argc, ConsoleValueRef *argv) {  \
         conmethod_return_##returnType ) c##className##name(static_cast<className*>(object),argc,argv);              \
      };                                                                                              \
      static ConsoleConstructor                                                                       \
         className##name##obj(#className,#name,c##className##name##caster,"",minArgs,maxArgs);        \
      static inline returnType c##className##name(className *object, S32 argc, ConsoleValueRef *argv)

#  define ConsoleStaticMethod(className,name,returnType,minArgs,maxArgs,usage1)                       \
      static inline returnType c##className##name(S32, ConsoleValueRef*);                                \
      static returnType c##className##name##caster(SimObject *object, S32 argc, ConsoleValueRef *argv) {  \
         conmethod_return_##returnType ) c##className##name(argc,argv);                                                        \
      };                                                                                              \
      static ConsoleConstructor                                                                       \
         className##name##obj(#className,#name,c##className##name##caster,"",minArgs,maxArgs);        \
      static inline returnType c##className##name(S32 argc, ConsoleValueRef *argv)

#define ConsoleDoc( text )

#endif

/// @}

/// @}

#endif // _CONSOLE_H_
