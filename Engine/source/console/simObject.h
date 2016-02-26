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

#ifndef _SIMOBJECT_H_
#define _SIMOBJECT_H_

#ifndef _SIM_H_
   #include "console/sim.h"
#endif
#ifndef _CONSOLEOBJECT_H_
   #include "console/consoleObject.h"
#endif
#ifndef _BITSET_H_
   #include "core/bitSet.h"
#endif

#ifndef _TAML_CALLBACKS_H_
#include "persistence/taml/tamlCallbacks.h"
#endif

class Stream;
class LightManager;
class SimFieldDictionary;
class SimPersistID;


/// Base class for objects involved in the simulation.
///
/// @section simobject_intro Introduction
///
/// SimObject is a base class for most of the classes you'll encounter
/// working in Torque. It provides fundamental services allowing "smart"
/// object referencing, creation, destruction, organization, and location.
/// Along with SimEvent, it gives you a flexible event-scheduling system,
/// as well as laying the foundation for the in-game editors, GUI system,
/// and other vital subsystems.
///
/// @section simobject_subclassing Subclassing
///
/// You will spend a lot of your time in Torque subclassing, or working
/// with subclasses of, SimObject. SimObject is designed to be easy to
/// subclass.
///
/// You should not need to override anything in a subclass except:
///     - The constructor/destructor.
///     - processArguments()
///     - onAdd()/onRemove()
///     - onGroupAdd()/onGroupRemove()
///     - onNameChange()
///     - onStaticModified()
///     - onDeleteNotify()
///     - onEditorEnable()/onEditorDisable()
///     - inspectPreApply()/inspectPostApply()
///     - things from ConsoleObject (see ConsoleObject docs for specifics)
///
/// Of course, if you know what you're doing, go nuts! But in most cases, you
/// shouldn't need to touch things not on that list.
///
/// When you subclass, you should define a typedef in the class, called Parent,
/// that references the class you're inheriting from.
///
/// @code
/// class mySubClass : public SimObject {
///     typedef SimObject Parent;
///     ...
/// @endcode
///
/// Then, when you override a method, put in:
///
/// @code
/// bool mySubClass::onAdd()
/// {
///     if(!Parent::onAdd())
///         return false;
///
///     // ... do other things ...
/// }
/// @endcode
///
/// Of course, you want to replace onAdd with the appropriate method call.
///
/// @section simobject_lifecycle A SimObject's Life Cycle
///
/// SimObjects do not live apart. One of the primary benefits of using a
/// SimObject is that you can uniquely identify it and easily find it (using
/// its ID). Torque does this by keeping a global hierarchy of SimGroups -
/// a tree - containing every registered SimObject. You can then query
/// for a given object using Sim::findObject() (or SimSet::findObject() if
/// you want to search only a specific set).
///
/// @code
///        // Three examples of registering an object.
///
///        // Method 1:
///        AIClient *aiPlayer = new AIClient();
///        aiPlayer->registerObject();
///
///        // Method 2:
///        ActionMap* globalMap = new ActionMap;
///        globalMap->registerObject("GlobalActionMap");
///
///        // Method 3:
///        bool reg = mObj->registerObject(id);
/// @endcode
///
/// Registering a SimObject performs these tasks:
///     - Marks the object as not cleared and not removed.
///     - Assigns the object a unique SimObjectID if it does not have one already.
///     - Adds the object to the global name and ID dictionaries so it can be found
///       again.
///     - Calls the object's onAdd() method. <b>Note:</b> SimObject::onAdd() performs
///       some important initialization steps. See @ref simobject_subclassing "here
///       for details" on how to properly subclass SimObject.
///     - If onAdd() fails (returns false), it calls unregisterObject().
///     - Checks to make sure that the SimObject was properly initialized (and asserts
///       if not).
///
/// Calling registerObject() and passing an ID or a name will cause the object to be
/// assigned that name and/or ID before it is registered.
///
/// Congratulations, you have now registered your object! What now?
///
/// Well, hopefully, the SimObject will have a long, useful life. But eventually,
/// it must die.
///
/// There are a two ways a SimObject can die.
///         - First, the game can be shut down. This causes the root SimGroup
///           to be unregistered and deleted. When a SimGroup is unregistered,
///           it unregisters all of its member SimObjects; this results in everything
///           that has been registered with Sim being unregistered, as everything
///           registered with Sim is in the root group.
///         - Second, you can manually kill it off, either by calling unregisterObject()
///           or by calling deleteObject().
///
/// When you unregister a SimObject, the following tasks are performed:
///     - The object is flagged as removed.
///     - Notifications are cleaned up.
///     - If the object is in a group, then it removes itself from the group.
///     - Delete notifications are sent out.
///     - Finally, the object removes itself from the Sim globals, and tells
///       Sim to get rid of any pending events for it.
///
/// If you call deleteObject(), all of the above tasks are performed, in addition
/// to some sanity checking to make sure the object was previously added properly,
/// and isn't in the process of being deleted. After the object is unregistered, it
/// deallocates itself.
///
/// @section simobject_editor Torque Editors
///
/// SimObjects are one of the building blocks for the in-game editors. They
/// provide a basic interface for the editor to be able to list the fields
/// of the object, update them safely and reliably, and inform the object
/// things have changed.
///
/// This interface is implemented in the following areas:
///     - onNameChange() is called when the object is renamed.
///     - onStaticModified() is called whenever a static field is modified.
///     - inspectPreApply() is called before the object's fields are updated,
///                     when changes are being applied.
///     - inspectPostApply() is called after the object's fields are updated.
///     - onEditorEnable() is called whenever an editor is enabled (for instance,
///                     when you hit F11 to bring up the world editor).
///     - onEditorDisable() is called whenever the editor is disabled (for instance,
///                     when you hit F11 again to close the world editor).
///
/// (Note: you can check the variable gEditingMission to see if the mission editor
/// is running; if so, you may want to render special indicators. For instance, the
/// fxFoliageReplicator renders inner and outer radii when the mission editor is
/// runnning.)
///
/// @section simobject_console The Console
///
/// SimObject extends ConsoleObject by allowing you to
/// to set arbitrary dynamic fields on the object, as well as
/// statically defined fields. This is done through two methods,
/// setDataField and getDataField, which deal with the complexities of
/// allowing access to two different types of object fields.
///
/// Static fields take priority over dynamic fields. This is to be
/// expected, as the role of dynamic fields is to allow data to be
/// stored in addition to the predefined fields.
///
/// The fields in a SimObject are like properties (or fields) in a class.
///
/// Some fields may be arrays, which is what the array parameter is for; if it's non-null,
/// then it is parsed with dAtoI and used as an index into the array. If you access something
/// as an array which isn't, then you get an empty string.
///
/// <b>You don't need to read any further than this.</b> Right now,
/// set/getDataField are called a total of 6 times through the entire
/// Torque codebase. Therefore, you probably don't need to be familiar
/// with the details of accessing them. You may want to look at Con::setData
/// instead. Most of the time you will probably be accessing fields directly,
/// or using the scripting language, which in either case means you don't
/// need to do anything special.
///
/// The functions to get/set these fields are very straightforward:
///
/// @code
///  setDataField(StringTable->insert("locked", false), NULL, b ? "true" : "false" );
///  curObject->setDataField(curField, curFieldArray, STR.getStringValue());
///  setDataField(slotName, array, value);
/// @endcode
///
/// <i>For advanced users:</i> There are two flags which control the behavior
/// of these functions. The first is ModStaticFields, which controls whether
/// or not the DataField functions look through the static fields (defined
/// with addField; see ConsoleObject for details) of the class. The second
/// is ModDynamicFields, which controls dynamically defined fields. They are
/// set automatically by the console constructor code.
///
/// @nosubgrouping
class SimObject: public ConsoleObject, public TamlCallbacks
{
   public:
   
      typedef ConsoleObject Parent;

      friend class SimManager;
      friend class SimGroup;
      friend class SimNameDictionary;
      friend class SimManagerNameDictionary;
      friend class SimIdDictionary;

      /// @name Notification
      /// @{
      
      struct Notify
      {
         enum Type
         {
            ClearNotify,   ///< Notified when the object is cleared.
            DeleteNotify,  ///< Notified when the object is deleted.
            ObjectRef,     ///< Cleverness to allow tracking of references.
            Invalid        ///< Mark this notification as unused (used in freeNotify).
         } type;
         
         void *ptr;        ///< Data (typically referencing or interested object).
         Notify *next;     ///< Next notification in the linked list.
      };

      /// @}

      /// Flags passed to SimObject::write 
      enum WriteFlags
      {
         SelectedOnly         = BIT( 0 ), ///< Indicates that only objects marked as selected should be outputted. Used in SimSet.
         NoName               = BIT( 1 ), ///< Indicates that the object name should not be saved.
         IgnoreCanSave        = BIT( 2 ), ///< Write out even if CannotSave=true.
      };

   private:

      /// Flags for use in mFlags
      enum
      {
         Deleted           = BIT( 0 ),    ///< This object is marked for deletion.
         Removed           = BIT( 1 ),    ///< This object has been unregistered from the object system.
         Added             = BIT( 3 ),    ///< This object has been registered with the object system.
         Selected          = BIT( 4 ),    ///< This object has been marked as selected. (in editor)
         Expanded          = BIT( 5 ),    ///< This object has been marked as expanded. (in editor)
         ModStaticFields   = BIT( 6 ),    ///< The object allows you to read/modify static fields
         ModDynamicFields  = BIT( 7 ),    ///< The object allows you to read/modify dynamic fields
         AutoDelete        = BIT( 8 ),    ///< Delete this object when the last ObjectRef is gone.
         CannotSave        = BIT( 9 ),    ///< Object should not be saved.
         EditorOnly        = BIT( 10 ),   ///< This object is for use by the editor only.
         NoNameChange      = BIT( 11 ),   ///< Whether changing the name of this object is allowed.
         Hidden            = BIT( 12 ),   ///< Object is hidden in editors.
         Locked            = BIT( 13 ),   ///< Object is locked in editors.
      };
      
      // dictionary information stored on the object
      StringTableEntry objectName;
      StringTableEntry mOriginalName;
      SimObject*       nextNameObject;
      SimObject*       nextManagerNameObject;
      SimObject*       nextIdObject;

      /// SimGroup we're contained in, if any.
      SimGroup*   mGroup;
      
      /// Flags internal to the object management system.
      BitSet32    mFlags;

      StringTableEntry    mProgenitorFile;

      /// Object we are copying fields from.
      SimObject* mCopySource;

      /// Table of dynamic fields assigned to this object.
      SimFieldDictionary *mFieldDictionary;

      /// Buffer to store textual representation of this object's numeric ID in.
      char mIdString[ 11 ];
      
      /// @name Serialization
      /// @{

      /// Path to file this SimObject was loaded from.
      StringTableEntry mFilename;

      /// The line number that the object was declared on if it was loaded from a file.
      S32 mDeclarationLine;
      
      /// @}

      /// @name Notification
      /// @{
      
      /// List of notifications added to this object.
      Notify* mNotifyList;

      static SimObject::Notify *mNotifyFreeList;
      static SimObject::Notify *allocNotify();     ///< Get a free Notify structure.
      static void freeNotify(SimObject::Notify*);  ///< Mark a Notify structure as free.

      /// @}

      static bool _setCanSave( void* object, const char* index, const char* data );
      static const char* _getCanSave( void* object, const char* data );
      
      static const char* _getHidden( void* object, const char* data )
         { if( static_cast< SimObject* >( object )->isHidden() ) return "1"; return "0"; }
      static const char* _getLocked( void* object, const char* data )
         { if( static_cast< SimObject* >( object )->isLocked() ) return "1"; return "0"; }
      static bool _setHidden( void* object, const char* index, const char* data )
         { static_cast< SimObject* >( object )->setHidden( dAtob( data ) ); return false; }
      static bool _setLocked( void* object, const char* index, const char* data )
         { static_cast< SimObject* >( object )->setLocked( dAtob( data ) ); return false; }

      // Namespace protected set methods
      static bool setClass( void *object, const char *index, const char *data )
         { static_cast<SimObject*>(object)->setClassNamespace(data); return false; };
      static bool setSuperClass(void *object, const char *index, const char *data)     
         { static_cast<SimObject*>(object)->setSuperClassNamespace(data); return false; };

            static bool writeObjectName(void* obj, StringTableEntry pFieldName)
         { SimObject* simObject = static_cast<SimObject*>(obj); return simObject->objectName != NULL && simObject->objectName != StringTable->EmptyString(); }
      static bool writeCanSaveDynamicFields(void* obj, StringTableEntry pFieldName)  
         { return static_cast<SimObject*>(obj)->mCanSaveFieldDictionary == false; }
      static bool writeInternalName(void* obj, StringTableEntry pFieldName)          
         { SimObject* simObject = static_cast<SimObject*>(obj); return simObject->mInternalName != NULL && simObject->mInternalName != StringTable->EmptyString(); }
      static bool setParentGroup(void* obj, const char* data);
      static bool writeParentGroup(void* obj, StringTableEntry pFieldName)           
         { return static_cast<SimObject*>(obj)->mGroup != NULL; }
      static bool writeSuperclass(void* obj, StringTableEntry pFieldName)            
         { SimObject* simObject = static_cast<SimObject*>(obj); return simObject->mSuperClassName != NULL && simObject->mSuperClassName != StringTable->EmptyString(); }
      static bool writeClass(void* obj, StringTableEntry pFieldName)                 
         { SimObject* simObject = static_cast<SimObject*>(obj); return simObject->mClassName != NULL && simObject->mClassName != StringTable->EmptyString(); }
      static bool writeClassName(void* obj, StringTableEntry pFieldName)
         { SimObject* simObject = static_cast<SimObject*>(obj); return simObject->mClassName != NULL && simObject->mClassName != StringTable->EmptyString(); }

      
      // Group hierarchy protected set method 
      static bool setProtectedParent(void *object, const char *index, const char *data);

      // Object name protected set method
      static bool setProtectedName(void *object, const char *index, const char *data);

   public:
      inline void setProgenitorFile(const char* pFile) { mProgenitorFile = StringTable->insert(pFile); }
      inline StringTableEntry getProgenitorFile(void) const { return mProgenitorFile; }

   protected:
      /// Taml callbacks.
      virtual void onTamlPreWrite(void) {}
      virtual void onTamlPostWrite(void) {}
      virtual void onTamlPreRead(void) {}
      virtual void onTamlPostRead(const TamlCustomNodes& customNodes) {}
      virtual void onTamlAddParent(SimObject* pParentObject) {}
      virtual void onTamlCustomWrite(TamlCustomNodes& customNodes) {}
      virtual void onTamlCustomRead(const TamlCustomNodes& customNodes);
   
      /// Id number for this object.
      SimObjectId mId;
      
      /// Internal name assigned to the object.  Not set by default.
      StringTableEntry mInternalName;
      
      static bool          smForceId;   ///< Force a registered object to use the given Id.  Cleared upon use.
      static SimObjectId   smForcedId;  ///< The Id to force upon the object.  Poor object.
      
      /// @name Serialization
      /// @{
      
      /// Whether dynamic fields should be saved out in serialization.  Defaults to true.
      bool mCanSaveFieldDictionary;
      
      /// @}

      /// @name Persistent IDs
      /// @{
      
      /// Persistent ID assigned to this object.  Allows to unambiguously refer to this
      /// object in serializations regardless of stream object ordering.
      SimPersistID* mPersistentId;
      
      static bool _setPersistentID( void* object, const char* index, const char* data );
         
      /// @}
      
      /// @name Namespace management
      /// @{
      
      /// The namespace in which method lookup for this object begins.
      Namespace* mNameSpace;

      /// Name of namespace to use as class namespace.
      StringTableEntry mClassName;
      
      /// Name of namespace to use as class super namespace.
      StringTableEntry mSuperClassName;

      /// Perform namespace linking on this object.
      void linkNamespaces();
      
      /// Undo namespace linking on this object.
      void unlinkNamespaces();
      
      /// @}

      /// Called when the object is selected in the editor.
      virtual void _onSelected() {}

      /// Called when the object is unselected in the editor.
      virtual void _onUnselected() {}
   
      /// We can provide more detail, like object name and id.
      virtual String _getLogMessage(const char* fmt, va_list args) const;
   
      DEFINE_CREATE_METHOD
      {
         T* object = new T;
         object->incRefCount();
         object->registerObject();
         return object;
      }

      
      // EngineObject.
      virtual void _destroySelf();

   public:
      
      /// @name Cloning
      /// @{
      
      /// Return a shallow copy of this object.
      virtual SimObject* clone();
      
      /// Return a deep copy of this object.
      virtual SimObject* deepClone();
      
      /// @}

      /// @name Accessors
      /// @{

      /// Get the value of a field on the object.
      ///
      /// See @ref simobject_console "here" for a detailed discussion of what this
      /// function does.
      ///
      /// @param   slotName    Field to access.
      /// @param   array       String containing index into array
      ///                      (if field is an array); if NULL, it is ignored.
      const char *getDataField(StringTableEntry slotName, const char *array);

      /// Set the value of a field on the object.
      ///
      /// See @ref simobject_console "here" for a detailed discussion of what this
      /// function does.
      ///
      /// @param   slotName    Field to access.
      /// @param   array       String containing index into array; if NULL, it is ignored.
      /// @param   value       Value to store.
      void setDataField(StringTableEntry slotName, const char *array, const char *value);

      const char *getPrefixedDataField(StringTableEntry fieldName, const char *array);

      void setPrefixedDataField(StringTableEntry fieldName, const char *array, const char *value);

      const char *getPrefixedDynamicDataField(StringTableEntry fieldName, const char *array, const S32 fieldType = -1);

      void setPrefixedDynamicDataField(StringTableEntry fieldName, const char *array, const char *value, const S32 fieldType = -1);

      StringTableEntry getDataFieldPrefix(StringTableEntry fieldName);

      /// Get the type of a field on the object.
      ///
      /// @param   slotName    Field to access.
      /// @param   array       String containing index into array
      ///                      (if field is an array); if NULL, it is ignored.
      U32 getDataFieldType(StringTableEntry slotName, const char *array);

      /// Set the type of a *dynamic* field on the object.
      ///
      /// @param   typeName/Id Console base type name/id to assign to a dynamic field.
      /// @param   slotName    Field to access.
      /// @param   array       String containing index into array
      ///                      (if field is an array); if NULL, it is ignored.
      void setDataFieldType(const U32 fieldTypeId, StringTableEntry slotName, const char *array);
      void setDataFieldType(const char *typeName, StringTableEntry slotName, const char *array);

      /// Get reference to the dictionary containing dynamic fields.
      ///
      /// See @ref simobject_console "here" for a detailed discussion of what this
      /// function does.
      ///
      /// This dictionary can be iterated over using a SimFieldDictionaryIterator.
      SimFieldDictionary * getFieldDictionary() {return(mFieldDictionary);}

      // Component Information
      inline virtual StringTableEntry  getComponentName() { return StringTable->insert( getClassName() ); };

      /// These functions support internal naming that is not namespace
      /// bound for locating child controls in a generic way.
      ///
      /// Set the internal name of this control (Not linked to a namespace)
      void setInternalName(const char* newname);

      /// Get the internal name of this control
      StringTableEntry getInternalName() const { return mInternalName; }

      /// Set the original name of this control
      void setOriginalName(const char* originalName);

      /// Get the original name of this control
      StringTableEntry getOriginalName() const { return mOriginalName; }

      /// These functions allow you to set and access the filename
      /// where this object was created.
      ///
      /// Set the filename
      void setFilename(const char* file);

      /// Get the filename
      StringTableEntry getFilename() const { return mFilename; }

      /// These functions are used to track the line number (1-based)
      /// on which the object was created if it was loaded from script
      ///
      /// Set the declaration line number
      void setDeclarationLine(U32 lineNumber);

      /// Get the declaration line number
      S32 getDeclarationLine() const { return mDeclarationLine; }

      /// Save object as a TorqueScript File.
      virtual bool save( const char* pcFilePath, bool bOnlySelected = false, const char *preappend = NULL );

      /// Check if a method exists in the objects current namespace.
      virtual bool isMethod( const char* methodName );
      
      /// Return true if the field is defined on the object
      virtual bool isField( const char* fieldName, bool includeStatic = true, bool includeDynamic = true );
      
      /// @}

      /// @name Initialization
      /// @{

      ///
      SimObject();
      
      virtual ~SimObject();

      virtual bool processArguments(S32 argc, ConsoleValueRef *argv);  ///< Process constructor options. (ie, new SimObject(1,2,3))

      /// @}

      /// @name Events
      /// @{
      
      /// Called when the object is added to the sim.
      virtual bool onAdd();
      
      /// Called when the object is removed from the sim.
      virtual void onRemove();
      
      /// Called when the object is added to a SimGroup.
      virtual void onGroupAdd();
      
      /// Called when the object is removed from a SimGroup.
      virtual void onGroupRemove();
      
      /// Called when the object's name is changed.
      virtual void onNameChange(const char *name);
      
      ///
      ///  Specifically, these are called by setDataField
      ///  when a static or dynamic field is modified, see
      ///  @ref simobject_console "the console details".
      virtual void onStaticModified(const char* slotName, const char*newValue = NULL); ///< Called when a static field is modified.
      virtual void onDynamicModified(const char* slotName, const char*newValue = NULL); ///< Called when a dynamic field is modified.

      /// Called before any property of the object is changed in the world editor.
      ///
      /// The calling order here is:
      ///  - inspectPreApply()
      ///  - ...
      ///  - calls to setDataField()
      ///  - ...
      ///  - inspectPostApply()
      virtual void inspectPreApply();

      /// Called after any property of the object is changed in the world editor.
      ///
      /// @see inspectPreApply
      virtual void inspectPostApply();

      /// Called when a SimObject is deleted.
      ///
      /// When you are on the notification list for another object
      /// and it is deleted, this method is called.
      virtual void onDeleteNotify(SimObject *object);

      /// Called when the editor is activated.
      virtual void onEditorEnable(){};

      /// Called when the editor is deactivated.
      virtual void onEditorDisable(){};

      /// @}

      /// Find a named sub-object of this object.
      ///
      /// This is subclassed in the SimGroup and SimSet classes.
      ///
      /// For a single object, it just returns NULL, as normal objects cannot have children.
      virtual SimObject *findObject(const char *name);

      /// @name Notification
      /// @{
      
      Notify *removeNotify(void *ptr, Notify::Type);   ///< Remove a notification from the list.
      void deleteNotify(SimObject* obj);               ///< Notify an object when we are deleted.
      void clearNotify(SimObject* obj);                ///< Notify an object when we are cleared.
      void clearAllNotifications();                    ///< Remove all notifications for this object.
      void processDeleteNotifies();                    ///< Send out deletion notifications.

      /// Register a reference to this object.
      ///
      /// You pass a pointer to your reference to this object.
      ///
      /// When the object is deleted, it will null your
      /// pointer, ensuring you don't access old memory.
      ///
      /// @param obj   Pointer to your reference to the object.
      void registerReference(SimObject **obj);

      /// Unregister a reference to this object.
      ///
      /// Remove a reference from the list, so that it won't
      /// get nulled inappropriately.
      ///
      /// Call this when you're done with your reference to
      /// the object, especially if you're going to free the
      /// memory. Otherwise, you may erroneously get something
      /// overwritten.
      ///
      /// @see registerReference
      void unregisterReference(SimObject **obj);

      /// @}

      /// @name Registration
      ///
      /// SimObjects must be registered with the object system.
      /// @{

      /// Register an object with the object system.
      ///
      /// This must be called if you want to keep the object around.
      /// In the rare case that you will delete the object immediately, or
      /// don't want to be able to use Sim::findObject to locate it, then
      /// you don't need to register it.
      ///
      /// registerObject adds the object to the global ID and name dictionaries,
      /// after first assigning it a new ID number. It calls onAdd(). If onAdd fails,
      /// it unregisters the object and returns false.
      ///
      /// If a subclass's onAdd doesn't eventually call SimObject::onAdd(), it will
      /// cause an assertion.
      bool registerObject();

      /// Register the object, forcing the id.
      ///
      /// @see registerObject()
      /// @param   id  ID to assign to the object.
      bool registerObject(U32 id);

      /// Register the object, assigning the name.
      ///
      /// @see registerObject()
      /// @param   name  Name to assign to the object.
      bool registerObject(const char *name);

      /// Register the object, assigning a name and ID.
      ///
      /// @see registerObject()
      /// @param   name  Name to assign to the object.
      /// @param   id  ID to assign to the object.
      bool registerObject(const char *name, U32 id);

      /// Unregister the object from Sim.
      ///
      /// This performs several operations:
      ///  - Sets the removed flag.
      ///  - Call onRemove()
      ///  - Clear out notifications.
      ///  - Remove the object from...
      ///      - its group, if any. (via getGroup)
      ///      - Sim::gNameDictionary
      ///      - Sim::gIDDictionary
      ///  - Finally, cancel any pending events for this object (as it can't receive them now).
      void unregisterObject();

      /// Unregister, mark as deleted, and free the object.
      void deleteObject();

      /// Performs a safe delayed delete of the object using a sim event.
      void safeDeleteObject();

      /// @}

      /// @name Accessors
      /// @{
      
      /// Return the unique numeric object ID.
      SimObjectId getId() const { return mId; }
      
      /// Return the object ID as a string.
      const char* getIdString() const { return mIdString; }
                  
      /// Return the name of this object.
      StringTableEntry getName() const { return objectName; }

      /// Return the SimGroup that this object is contained in.  Never NULL except for
      /// RootGroup and unregistered objects.
      SimGroup* getGroup() const { return mGroup; }

      /// Assign the given name to this object.
      void assignName( const char* name );

      void setId(SimObjectId id);
      static void setForcedId(SimObjectId id) { smForceId = true; smForcedId = id; } ///< Force an Id on the next registered object.
      bool isChildOfGroup(SimGroup* pGroup);
      bool isProperlyAdded() const { return mFlags.test(Added); }
      bool isDeleted() const { return mFlags.test(Deleted); }
      bool isRemoved() const { return mFlags.test(Deleted | Removed); }
      
      virtual bool isLocked() const { return mFlags.test( Locked ); }
      virtual void setLocked( bool b );
      virtual bool isHidden() const { return mFlags.test( Hidden ); }
      virtual void setHidden(bool b);

      /// @}

      /// @name Sets
      ///
      /// The object must be properly registered before you can add/remove it to/from a set.
      ///
      /// All these functions accept either a name or ID to identify the set you wish
      /// to operate on. Then they call addObject or removeObject on the set, which
      /// sets up appropriate notifications.
      ///
      /// An object may be in multiple sets at a time.
      /// @{
      bool addToSet(SimObjectId);
      bool addToSet(const char *);
      bool removeFromSet(SimObjectId);
      bool removeFromSet(const char *);

      /// @}

      /// @name Serialization
      /// @{

      /// Determine whether or not a field should be written.
      ///
      /// @param   fiedname The name of the field being written.
      /// @param   value The value of the field.
      virtual bool writeField(StringTableEntry fieldname, const char* value);

      /// Output the TorqueScript to recreate this object.
      ///
      /// This calls writeFields internally.
      /// @param   stream  Stream to output to.
      /// @param   tabStop Indentation level for this object.
      /// @param   flags   If SelectedOnly is passed here, then
      ///                  only objects marked as selected (using setSelected)
      ///                  will output themselves.
      virtual void write(Stream &stream, U32 tabStop, U32 flags = 0);

      /// Write the fields of this object in TorqueScript.
      ///
      /// @param   stream  Stream for output.
      /// @param   tabStop Indentation level for the fields.
      virtual void writeFields(Stream &stream, U32 tabStop);

      virtual bool writeObject(Stream *stream);
      virtual bool readObject(Stream *stream);
      
      /// Set whether fields created at runtime should be saved. Default is true.
      void setCanSaveDynamicFields( bool bCanSave ) { mCanSaveFieldDictionary	=	bCanSave; }
      
      /// Get whether fields created at runtime should be saved. Default is true.
      bool getCanSaveDynamicFields( ) { return mCanSaveFieldDictionary;}

      /// Return the object that this object is copying fields from.
      SimObject* getCopySource() const { return mCopySource; }
      
      /// Set the object that this object should be copying fields from.
      void setCopySource( SimObject* object );

      /// Copy fields from another object onto this one.
      ///
      /// Objects must be of same type. Everything from obj
      /// will overwrite what's in this object; extra fields
      /// in this object will remain. This includes dynamic
      /// fields.
      ///
      /// @param   obj Object to copy from.
      void assignFieldsFrom(SimObject *obj);

      /// Copy dynamic fields from another object onto this one.
      ///
      /// Everything from obj will overwrite what's in this
      /// object.
      ///
      /// @param   obj Object to copy from.
      void assignDynamicFieldsFrom(SimObject *obj);

      /// @}

      /// Return the object's namespace.
      Namespace* getNamespace() { return mNameSpace; }

      /// Get next matching item in namespace.
      ///
      /// This wraps a call to Namespace::tabComplete; it gets the
      /// next thing in the namespace, given a starting value
      /// and a base length of the string. See
      /// Namespace::tabComplete for details.
      const char *tabComplete(const char *prevText, S32 baseLen, bool);

      /// @name Accessors
      /// @{
      
      bool isSelected() const { return mFlags.test(Selected); }
      bool isExpanded() const { return mFlags.test(Expanded); }
      bool isEditorOnly() const { return mFlags.test( EditorOnly ); }
      bool isNameChangeAllowed() const { return !mFlags.test( NoNameChange ); }
      bool isAutoDeleted() const { return mFlags.test( AutoDelete ); }
      void setSelected(bool sel);
      void setExpanded(bool exp) { if(exp) mFlags.set(Expanded); else mFlags.clear(Expanded); }
      void setModDynamicFields(bool dyn) { if(dyn) mFlags.set(ModDynamicFields); else mFlags.clear(ModDynamicFields); }
      void setModStaticFields(bool sta) { if(sta) mFlags.set(ModStaticFields); else mFlags.clear(ModStaticFields); }
      bool canModDynamicFields() { return mFlags.test(ModDynamicFields); }
      bool canModStaticFields() { return mFlags.test(ModStaticFields); }
      void setAutoDelete( bool val ) { if( val ) mFlags.set( AutoDelete ); else mFlags.clear( AutoDelete ); }
      void setEditorOnly( bool val ) { if( val ) mFlags.set( EditorOnly ); else mFlags.clear( EditorOnly ); }
      void setNameChangeAllowed( bool val ) { if( val ) mFlags.clear( NoNameChange ); else mFlags.set( NoNameChange ); }

      /// Returns boolean specifying if the object can be serialized.
      bool getCanSave() const { return !mFlags.test( CannotSave ); }
      
      /// Set serialization flag.
      virtual void setCanSave( bool val ) { if( !val ) mFlags.set( CannotSave ); else mFlags.clear( CannotSave ); }

      /// Returns true if this object is selected or any group it is a member of is.
      bool isSelectedRecursive() const;

      /// @}
      
      /// @name Namespace management
      /// @{
      
      /// Return name of class namespace set on this object.
      StringTableEntry getClassNamespace() const { return mClassName; };
      
      /// Return name of superclass namespace set on this object.
      StringTableEntry getSuperClassNamespace() const { return mSuperClassName; };
      
      ///
      void setClassNamespace( const char* classNamespace );
      
      ///
      void setSuperClassNamespace( const char* superClassNamespace );
            
      /// @}

      /// @name Persistent IDs
      /// @{

      /// Return the persistent ID assigned to this object or NULL.
      SimPersistID* getPersistentId() const { return mPersistentId; }
      
      /// Return the persistent ID assigned to this object or assign one to it if it has none.
      SimPersistID* getOrCreatePersistentId();
      
      /// @}
      
      /// @name Debugging
      /// @{

      /// Return a textual description of the object.
      virtual String describeSelf() const;

      /// Dump the contents of this object to the console.  Use the Torque Script dump() and dumpF() functions to 
      /// call this.  
      void dumpToConsole( bool includeFunctions=true );
      
      ///added this so that you can print the entire class hierarchy, including script objects, 
      //from the console or C++.
      
      /// Print the AbstractClassRep hierarchy of this object to the console.
      virtual void dumpClassHierarchy();
      
      /// Print the SimGroup hierarchy of this object to the console.
      virtual void dumpGroupHierarchy();

      /// @}

      static void initPersistFields();

      /// Copy SimObject to another SimObject (Originally designed for T2D).
      virtual void copyTo(SimObject* object);

      // Component Console Overrides
      virtual bool handlesConsoleMethod(const char * fname, S32 * routingId) { return false; }
      virtual void getConsoleMethodData(const char * fname, S32 routingId, S32 * type, S32 * minArgs, S32 * maxArgs, void ** callback, const char ** usage) {}
      
      DECLARE_CONOBJECT( SimObject );
      
      static SimObject* __findObject( const char* id ) { return Sim::findObject( id ); }
      static const char* __getObjectId( ConsoleObject* object )
      {
         SimObject* simObject = static_cast< SimObject* >( object );
         if( !simObject )
            return "";
         else if( simObject->getName() )
            return simObject->getName();
         return simObject->getIdString();
      }

      // EngineObject.
      virtual void destroySelf();
};


/// Smart SimObject pointer.
///
/// This class keeps track of the book-keeping necessary
/// to keep a registered reference to a SimObject or subclass
/// thereof.
///
/// Normally, if you want the SimObject to be aware that you
/// have a reference to it, you must call SimObject::registerReference()
/// when you create the reference, and SimObject::unregisterReference() when
/// you're done. If you change the reference, you must also register/unregister
/// it. This is a big headache, so this class exists to automatically
/// keep track of things for you.
///
/// @code
///     // Assign an object to the
///     SimObjectPtr<GameBase> mOrbitObject = Sim::findObject("anObject");
///
///     // Use it as a GameBase*.
///     mOrbitObject->getWorldBox().getCenter(&mPosition);
///
///     // And reassign it - it will automatically update the references.
///     mOrbitObject = Sim::findObject("anotherObject");
/// @endcode
template< typename T >
class SimObjectPtr : public WeakRefPtr< T >
{
   public:
   
      typedef WeakRefPtr< T > Parent;
   
      SimObjectPtr() {}
      SimObjectPtr(T *ptr) { this->mReference = NULL; set(ptr); }
      SimObjectPtr( const SimObjectPtr& ref ) { this->mReference = NULL; set(ref.mReference); }

      T* getObject() const { return Parent::getPointer(); }

      ~SimObjectPtr() { set((WeakRefBase::WeakReference*)NULL); }

      SimObjectPtr<T>& operator=(const SimObjectPtr ref)
      {
         set(ref.mReference);
         return *this;
      }
      SimObjectPtr<T>& operator=(T *ptr)
      {
         set(ptr);
         return *this;
      }

   protected:
      void set(WeakRefBase::WeakReference * ref)
      {
         if( ref == this->mReference )
            return;

         if( this->mReference )
         {
            // Auto-delete
            T* obj = this->getPointer();
            if ( this->mReference->getRefCount() == 2 && obj && obj->isAutoDeleted() )
               obj->deleteObject();

            this->mReference->decRefCount();
         }
         this->mReference = NULL;
         if( ref )
         {
            this->mReference = ref;
            this->mReference->incRefCount();
         }
      }

      void set(T * obj)
      {
         set(obj ? obj->getWeakReference() : (WeakRefBase::WeakReference *)NULL);
      }
};

#endif // _SIMOBJECT_H_
