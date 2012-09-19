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
#include "console/compiler.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "core/util/tDictionary.h"
#include "app/mainLoop.h"

// External scripting cinterface, suitable for import into any scripting system which support "C" interfaces (C#, Python, Lua, Java, etc)

#ifdef TORQUE_OS_WIN32
#include "windowManager/win32/win32Window.h"
#include "windowManager/win32/winDispatch.h"
#endif

extern "C" {

   struct MarshalNativeEntry
   {
      const char* nameSpace;
      const char* name;
      Namespace::Entry* entry; 
      S32 minArgs;
      S32 maxArgs;
      S32 cbType;
   };


   static Namespace::Entry* GetEntry(const char* nameSpace, const char* name)                                          
   {
      Namespace* ns = NULL;

      if (!nameSpace || !dStrlen(nameSpace))
         ns = Namespace::mGlobalNamespace;
      else
      {
         nameSpace = StringTable->insert(nameSpace);
         ns = Namespace::find(nameSpace); //can specify a package here, maybe need, maybe not
      }

      if (!ns)
         return NULL;

      name = StringTable->insert(name);

      Namespace::Entry* entry = ns->lookupRecursive(name);

      return entry;
   }

   const char * script_getconsolexml()
   {
      Namespace::Entry* entry = GetEntry("", "consoleExportXML");

      if (!entry)
         return "";

      const char* argv[] = {"consoleExportXML", 0};

      return entry->cb.mStringCallbackFunc(NULL, 1, argv);      
   }

   MarshalNativeEntry* script_get_namespace_entry(const char* nameSpace, const char* name)
   {
      static MarshalNativeEntry mentry;

      Namespace::Entry* e = GetEntry(nameSpace, name);

      if (!e)
         return NULL;

      mentry.nameSpace = e->mNamespace->mName;
      mentry.name = e->mFunctionName;
      mentry.minArgs = e->mMinArgs;
      mentry.maxArgs = e->mMaxArgs;
      mentry.cbType = e->mType;
      mentry.entry = e;

      return &mentry;
   }

   void* script_get_stringtable_entry(const char* string)
   {
      return (void*)StringTable->insert(string);
   }

   // FIELD ACCESS

   // fieldNames must be from stringTable coming in! See Engine.stringTable

   const char* script_simobject_getfield_string(U32 id, const char* fieldName)
   {
      SimObject *object = Sim::findObject( id );
      if( object )
      {
         return (const char *) object->getDataField(fieldName, "");
      }
      return "";
   }

   void script_simobject_setfield_string(U32 objectId, const char* fieldName, const char* v)
   {
      SimObject *object = Sim::findObject( objectId );
      if( object )
      {
         object->setDataField(fieldName, "", v);
      }
   }


   bool script_simobject_getfield_bool(U32 objectId, const char* fieldName)
   {
      SimObject *object = Sim::findObject( objectId );
      if( object )
      {
         const char *v = object->getDataField(fieldName, "");

         return dAtob(v);
      }

      return false;
   }

   void script_simobject_setfield_bool(U32 objectId, const char* fieldName, bool v)
   {
      SimObject *object = Sim::findObject( objectId );
      if( object )
      {
         object->setDataField(fieldName, "", v ? "1" : "0");
      }
   }

   S32 script_simobject_getfield_int(U32 objectId, const char* fieldName)
   {
      SimObject *object = Sim::findObject( objectId );
      if( object )
      {
         const char *v = object->getDataField(fieldName, "");

         return dAtoi(v);
      }

      return false;
   }

   void script_simobject_setfield_int(U32 objectId, const char* fieldName, int v)
   {
      SimObject *object = Sim::findObject( objectId );
      if( object )
      {
         // this seems pretty lame, though it is how it is handled in consoleType.cpp
         char buf[256];
         dSprintf(buf, 256, "%d", v );
         object->setDataField(fieldName, "", buf);
      }
   }

   F32 script_simobject_getfield_float(U32 objectId, const char* fieldName)
   {
      SimObject *object = Sim::findObject( objectId );
      if( object )
      {
         const char *v = object->getDataField(fieldName, "");

         return dAtof(v);
      }

      return false;
   }

   void script_simobject_setfield_float(U32 objectId, const char* fieldName, F32 v)
   {
      SimObject *object = Sim::findObject( objectId );
      if( object )
      {
         char buf[256];
         dSprintf(buf, 256, "%g", v );
         object->setDataField(fieldName, "", buf);
      }
   }

   const char* script_call_namespace_entry_string(Namespace::Entry* entry, S32 argc, const char** argv)
   {
      // maxArgs improper on a number of console function/methods
      if (argc < entry->mMinArgs)// || argc > entry->mMaxArgs)
         return "";

      SimObject* o = NULL;

      if (entry->mNamespace && entry->mNamespace->isClass())
      {
         o = Sim::findObject(dAtoi(argv[1]));
         if (!o)
            return "";
      }

      return entry->cb.mStringCallbackFunc(o, argc, argv);      
   }

   bool script_call_namespace_entry_bool(Namespace::Entry* entry, S32 argc, const char** argv)
   {
      // maxArgs improper on a number of console function/methods
      if (argc < entry->mMinArgs)// || argc > entry->mMaxArgs)
         return "";

      SimObject* o = NULL;

      if (entry->mNamespace && entry->mNamespace->isClass())
      {
         o = Sim::findObject(dAtoi(argv[1]));
         if (!o)
            return "";
      }

      return entry->cb.mBoolCallbackFunc(o, argc, argv);      
   }

   S32 script_call_namespace_entry_int(Namespace::Entry* entry, S32 argc, const char** argv)
   {
      // maxArgs improper on a number of console function/methods
      if (argc < entry->mMinArgs)// || argc > entry->mMaxArgs)
         return 0;

      SimObject* o = NULL;

      if (entry->mNamespace && entry->mNamespace->isClass())
      {
         o = Sim::findObject(dAtoi(argv[1]));
         if (!o)
            return 0;
      }

      return entry->cb.mIntCallbackFunc(o, argc, argv);      
   }

   F32 script_call_namespace_entry_float(Namespace::Entry* entry, S32 argc, const char** argv)
   {
      // maxArgs improper on a number of console function/methods
      if (argc < entry->mMinArgs)// || argc > entry->mMaxArgs)
         return 0.0f;

      SimObject* o = NULL;

      if (entry->mNamespace && entry->mNamespace->isClass())
      {
         o = Sim::findObject(dAtoi(argv[1]));
         if (!o)
            return 0.0f;
      }

      return entry->cb.mFloatCallbackFunc(o, argc, argv);      
   }


   void script_call_namespace_entry_void(Namespace::Entry* entry, S32 argc, const char** argv)
   {
      // maxArgs improper on a number of console function/methods
      if (argc < entry->mMinArgs)// || argc > entry->mMaxArgs)
         return;

      SimObject* o = NULL;

      if (entry->mNamespace && entry->mNamespace->isClass())
      {
         Sim::findObject(dAtoi(argv[1]));
         if (!o)
            return;
      }

      entry->cb.mVoidCallbackFunc(o, argc, argv);      
   }

   int script_simobject_get_id(SimObject* so)
   {
      return so->getId();
   }

   int script_simobject_find(const char* classname, const char* name)
   {
      SimObject *object;
      if( Sim::findObject( name, object ) )
      {
         // if we specified a classname do type checking
         if (classname && dStrlen(classname))
         {
            AbstractClassRep* ocr = object->getClassRep();
            while (ocr)
            {
               if (!dStricmp(ocr->getClassName(), classname))
                  return object->getId();
               ocr = ocr->getParentClass();
            }

         }

         // invalid type
         return 0;
      }

      // didn't find object
      return 0;
   }

   void script_export_callback_string(StringCallback cb, const char *nameSpace, const char *funcName, const char* usage,  S32 minArgs, S32 maxArgs)
   {
      if (!nameSpace || !dStrlen(nameSpace))
         Con::addCommand(funcName, cb, usage, minArgs + 1, maxArgs + 1);
      else
         Con::addCommand(nameSpace, funcName, cb, usage, minArgs + 1, maxArgs + 1);
   }

   void script_export_callback_void(VoidCallback cb, const char *nameSpace, const char *funcName, const char* usage,  S32 minArgs, S32 maxArgs)
   {

      if (!nameSpace || !dStrlen(nameSpace))
         Con::addCommand(funcName, cb, usage, minArgs + 1, maxArgs + 1);
      else
         Con::addCommand(nameSpace, funcName, cb, usage, minArgs + 1, maxArgs + 1);



      // example of package support
      // note that Parent:: does not work with this, at least not yet anyway

      /*
      Namespace* ns;

      StringTableEntry nspace = NULL;

      if (nameSpace && dStrlen(nameSpace))
      nspace = StringTable->insert(nameSpace);

      Namespace::unlinkPackages();
      ns = Namespace::find(nspace, StringTable->insert("fps"));
      ns->addCommand(StringTable->insert(funcName), cb, StringTable->insert(usage), minArgs + 1, maxArgs + 1 );
      Namespace::relinkPackages();
      */
   }

   void script_export_callback_bool(BoolCallback cb, const char *nameSpace, const char *funcName, const char* usage,  S32 minArgs, S32 maxArgs)
   {
      if (!nameSpace || !dStrlen(nameSpace))
         Con::addCommand(funcName, cb, usage, minArgs + 1, maxArgs + 1);
      else
         Con::addCommand(nameSpace, funcName, cb, usage, minArgs + 1, maxArgs + 1);
   }

   void script_export_callback_int(IntCallback cb, const char *nameSpace, const char *funcName, const char* usage,  S32 minArgs, S32 maxArgs)
   {
      if (!nameSpace || !dStrlen(nameSpace))
         Con::addCommand(funcName, cb, usage, minArgs + 1, maxArgs + 1);
      else
         Con::addCommand(nameSpace, funcName, cb, usage, minArgs + 1, maxArgs + 1);
   }

   void script_export_callback_float(FloatCallback cb, const char *nameSpace, const char *funcName, const char* usage,  S32 minArgs, S32 maxArgs)
   {
      if (!nameSpace || !dStrlen(nameSpace))
         Con::addCommand(funcName, cb, usage, minArgs + 1, maxArgs + 1);
      else
         Con::addCommand(nameSpace, funcName, cb, usage, minArgs + 1, maxArgs + 1);
   }


#ifdef TORQUE_OS_WIN32

   void script_input_event(int type, int value1, int value2)
   {
      if (PlatformWindowManager::get() && PlatformWindowManager::get()->getFirstWindow())
      {
         Win32Window* w = (Win32Window*) PlatformWindowManager::get()->getFirstWindow();
         WindowId devId = w->getWindowId();

         switch (type)
         {
         case 0:
            w->mouseEvent.trigger(devId,0,value1,value2,w->isMouseLocked());
            break;
         case 1:
            if (value2)
               w->buttonEvent.trigger(devId,0,IA_MAKE,value1);
            else
               w->buttonEvent.trigger(devId,0,IA_BREAK,value1);
            break;

         }

      }      

   }
#endif

}


ConsoleFunction(TestFunction2Args, const char *, 3, 3, "testFunction(arg1, arg2)")
{
   return "Return Value";
}
