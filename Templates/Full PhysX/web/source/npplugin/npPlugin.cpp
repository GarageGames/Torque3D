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

#include <string>
#include <vector>

#include "npWebGamePlugin.h"
#include "../common/webCommon.h"

// Functions exported from the browser to our plugin
static NPNetscapeFuncs NPNFuncs;

// Set once both the web page and plugin are fully loaded (including any possible Javascript <-> TorqueScript exports/imports)
static BOOL gInitialized = false;

#ifndef WIN32
// Entry points into our plugin for Safari 
extern "C" {

#pragma GCC visibility push(default)

NPError NP_Initialize (NPNetscapeFuncs *browser);
NPError NP_GetEntryPoints (NPPluginFuncs *plugin);
NPError NP_Shutdown();

#pragma GCC visibility pop

}
#endif

// Converts a NPVARIANT variable to a C string representation
const char* MY_NPVARIANT_TO_STRING(const NPVariant& f)
{
   static std::string result;
   char r[1024] = {0};

   if (NPVARIANT_IS_VOID(f) || NPVARIANT_IS_NULL(f))
      return "";

   if (NPVARIANT_IS_STRING(f))
   {
      NPString str = NPVARIANT_TO_STRING(f);
      result = std::string(str.UTF8Characters, str.UTF8Length);
      return result.c_str();
   }

   if (NPVARIANT_IS_BOOLEAN(f))
   {
      if (NPVARIANT_TO_BOOLEAN(f))
         return "1";
      return "0";
   }

   if (NPVARIANT_IS_INT32(f))
   {
      sprintf(r, "%i", NPVARIANT_TO_INT32(f)); 
      result = r;
      return result.c_str();
   }

   if (NPVARIANT_IS_DOUBLE(f))
   {
      sprintf(r, "%f", NPVARIANT_TO_DOUBLE(f));
      result = r;
      return result.c_str();
   }

   return "";

}

// Javascript -> TorqueScript function calling (with parser)
const char* CallScript(const char* code)
{
   std::string scode = code;
   const char* sig = scode.c_str();

   // do not allow large strings which could be used maliciously
   if (scode.length() > 255 || !gInitialized)
   {
      return "";
   }

   // data buffers for laying out data in a Torque 3D console friendly manner
   char  nameSpace[256];
   char  fname[256];
   char  argv[256][256];
   char* argvv[256];
   int argc = 0;
   int argBegin = 0;

   memset(nameSpace, 0, 256);
   memset(fname, 0, 256);
   memset(argv, 0, 256 * 256);

   for (int i = 0; i < scode.length(); i++)
   {
      if (sig[i] == ')' || sig[i] == ';')
      {
         //scan out last arg is any
         char dummy[256];
         memset(dummy, 0, 256);

         WebCommon::StringCopy(dummy, &sig[argBegin], i - argBegin);

         if (strlen(dummy))
         {
            strcpy(argv[argc], dummy);
            argvv[argc] = argv[argc];
            argc++;
         }

         break; // done
      }

      // handle namespace
      if (sig[i]==':')
      {
         if (nameSpace[0] || fname[0])
         {
            return "";
         }

         if (i > 0 && sig[i-1] == ':')
         {
            if (i - 2 > 0)
               WebCommon::StringCopy(nameSpace, sig, i - 1);
         }

         continue;
      }

      // arguments begin
      if (sig[i] == '(' )
      {
         if (fname[0] || i < 1)
         {
            return "";
         }

         //everything before this is function name, minus nameSpace
         if (nameSpace[0])
         {
            int nlen = strlen(nameSpace);
            WebCommon::StringCopy(fname, &sig[nlen + 2], i - nlen - 2);
         }
         else
         {
            WebCommon::StringCopy(fname, sig, i);
         }

         WebCommon::StringCopy(argv[0], fname, strlen(fname)+1);
         argvv[0] = argv[0];
         argc++;

         argBegin = i + 1;
      }

      // argument
      if (sig[i] == ',' )
      {
         if (argBegin >= i || argc == 255) 
         {
            return "";
         }

         WebCommon::StringCopy(argv[argc], &sig[argBegin], i - argBegin);
         argvv[argc] = argv[argc];

         argc++;
         argBegin = i + 1;
      }

   }

   static std::string retVal;

   if (fname[0])
   {
      // call into the Torque 3D shared library (console system) and get return value
      retVal = torque_callsecurefunction(nameSpace, fname, argc, (const char **) argvv);
      return retVal.c_str();
   }

   return "";
}


// TorqueScript -> JavaScript 
const char* CallJavaScriptFunction(const char* name, int numArguments, const char* argv[])
{
   // our plugin instance
   NPP pNPInstance = NPWebGamePlugin::sInstance->mInstance;

   // holds the generated Javascript encoded as a NPString
   NPString npScript;

   // retrieve our plugin object from the browser
   NPObject* pluginObject;

   if (NPERR_NO_ERROR != NPNFuncs.getvalue(pNPInstance, NPNVPluginElementNPObject, &pluginObject)) 
   {
      return NULL;
   }

   // generate Javascript to be evaluated
   std::string script = name;
   script += "(";
   for (int i = 1; i < numArguments; i++)
   {
      script += "\"";
      script += argv[i];
      script += "\"";
      if ( i + 1 < numArguments)
         script += ", ";
   }
   script += ");";

   //encode as a NPString
   npScript.UTF8Characters = script.c_str();
   npScript.UTF8Length = script.length();
   
   // finally, have browser evaluate our script and get the return value
   NPVariant result;
   NPNFuncs.evaluate(pNPInstance,pluginObject,&npScript,&result); 

   return MY_NPVARIANT_TO_STRING(result);

}

// the sole entry point for Torque 3D console system into our browser plugin (handed over as a function pointer)
static const char * MyStringCallback(void *obj, int argc, const char* argv[])
{
   static char ret[4096];
   strcpy(ret,CallJavaScriptFunction(argv[0], argc, argv));
   return ret;
}


// these can be added on the page before we're initialized, so we cache until we're ready for them
typedef struct 
{ 
   std::string jsCallback; //javascript function name
   unsigned int numArguments;  //the number of arguments it takes
} JavasScriptExport;

static std::vector<JavasScriptExport> gJavaScriptExports;

// this actually exports the function to the Torque 3D console system
// we do this in two steps as we can't guarantee that Torque 3D is initialized on web page
// before JavaScript calls are made
void ExportFunctionInternal(const JavasScriptExport& jsexport)
{
   torque_exportstringcallback(MyStringCallback,"JS",jsexport.jsCallback.c_str(),"",jsexport.numArguments,jsexport.numArguments);
}

// invoked via the Javascript plugin object startup() method once the page/plugin are fully loaded
void Startup()
{
   if (gInitialized)
      return;

   // actually do the export on any cached functions
   gInitialized = true;
   std::vector<JavasScriptExport>::iterator i;
   for (i = gJavaScriptExports.begin(); i != gJavaScriptExports.end();i++)
   {
      ExportFunctionInternal(*i);
   }

   // setup the secure TorqueScript function calls we can call from Javascript (see webConfig.h)
   WebCommon::AddSecureFunctions();
}

// Export a Javascript function to the Torque 3D console system, possibly caching it 
void ExportFunction(const char* callback, int numArguments)
{
   JavasScriptExport jsexport;
   jsexport.jsCallback = callback;
   jsexport.numArguments = numArguments;

   if (!gInitialized)
   {
      //queue it up
      gJavaScriptExports.push_back(jsexport);
   }
   else
   {
      ExportFunctionInternal(jsexport);
   }
}

// NP Plugin Interface


// Our plugin object structure "inherited" from NPObject
typedef struct
{
   // NPObject fields
   NPClass *_class;
   uint32_t referenceCount;

   // Here begins our custom fields (well, field)

   // Platform specific game plugin class (handles refresh, sizing, initialization of Torque 3D, etc)
   NPWebGamePlugin* webPlugin;

} PluginObject;

static PluginObject* gPluginObject = NULL;

// interface exports for our plugin that are expected to the browser

void pluginInvalidate ();
bool pluginHasProperty (NPClass *theClass, NPIdentifier name);
bool pluginHasMethod (NPObject *npobj, NPIdentifier name);
bool pluginGetProperty (PluginObject *obj, NPIdentifier name, NPVariant *variant);
bool pluginSetProperty (PluginObject *obj, NPIdentifier name, const NPVariant *variant);
bool pluginInvoke (PluginObject *obj, NPIdentifier name, NPVariant *args, uint32_t argCount, NPVariant *result);
bool pluginInvokeDefault (PluginObject *obj, NPVariant *args, uint32_t argCount, NPVariant *result);
NPObject *pluginAllocate (NPP npp, NPClass *theClass);
void pluginDeallocate (PluginObject *obj);

static NPClass _pluginFunctionPtrs = { 
   NP_CLASS_STRUCT_VERSION,
   (NPAllocateFunctionPtr)       pluginAllocate, 
   (NPDeallocateFunctionPtr)     pluginDeallocate, 
   (NPInvalidateFunctionPtr)     pluginInvalidate,
   (NPHasMethodFunctionPtr)      pluginHasMethod,
   (NPInvokeFunctionPtr)         pluginInvoke,
   (NPInvokeDefaultFunctionPtr)  pluginInvokeDefault,
   (NPHasPropertyFunctionPtr)    pluginHasProperty,
   (NPGetPropertyFunctionPtr)    pluginGetProperty,
   (NPSetPropertyFunctionPtr)    pluginSetProperty,
};

// versioning information

static bool identifiersInitialized = false;

#define ID_VERSION_PROPERTY        0
#define NUM_PROPERTY_IDENTIFIERS   1

static NPIdentifier pluginPropertyIdentifiers[NUM_PROPERTY_IDENTIFIERS];
static const NPUTF8 *pluginPropertyIdentifierNames[NUM_PROPERTY_IDENTIFIERS] = {
   "version",
};

// methods that are callable on the plugin from Javascript

#define ID_SETVARIABLE_METHOD        0
#define ID_GETVARIABLE_METHOD        1
#define ID_EXPORTFUNCTION_METHOD     2
#define ID_CALLSCRIPT_METHOD         3
#define ID_STARTUP_METHOD            4
#define NUM_METHOD_IDENTIFIERS       5

static NPIdentifier pluginMethodIdentifiers[NUM_METHOD_IDENTIFIERS];
static const NPUTF8 *pluginMethodIdentifierNames[NUM_METHOD_IDENTIFIERS] = {
   "setVariable",
   "getVariable",
   "exportFunction",
   "callScript",
   "startup",
};

NPClass *getPluginClass(void)
{
   return &_pluginFunctionPtrs;
}

// Sets up the property and method identifier arrays used by the browser
// via the hasProperty and hasMethod fuction pointers
static void initializeIdentifiers()
{
   // fill the property identifier array
   NPNFuncs.getstringidentifiers(pluginPropertyIdentifierNames, 
      NUM_PROPERTY_IDENTIFIERS,
      pluginPropertyIdentifiers);

   // fill the method identifier array
   NPNFuncs.getstringidentifiers(pluginMethodIdentifierNames,
      NUM_METHOD_IDENTIFIERS,
      pluginMethodIdentifiers);
};

bool pluginHasProperty (NPClass *theClass, NPIdentifier name)
{   
   for (int i = 0; i < NUM_PROPERTY_IDENTIFIERS; i++) {
      if (name == pluginPropertyIdentifiers[i]) {
         return true;
      }
   }
   return false;
}

bool pluginHasMethod(NPObject *npobj, NPIdentifier name)
{
   for (int i = 0; i < NUM_METHOD_IDENTIFIERS; i++) {
      if (name == pluginMethodIdentifiers[i]) {
         return true;
      }
   }
   return false;
}

// utility function that sets up a NPVariant from a std::string
void FillString(const std::string& src, NPVariant* variant)
{
   variant->type = NPVariantType_String;
   variant->value.stringValue.UTF8Length = static_cast<uint32_t>(src.length());
   variant->value.stringValue.UTF8Characters = reinterpret_cast<NPUTF8 *>(NPNFuncs.memalloc(src.size()));
   memcpy((void*)variant->value.stringValue.UTF8Characters, src.c_str(), src.size());   
}


bool pluginGetProperty (PluginObject *obj, NPIdentifier name, NPVariant *variant)
{
   VOID_TO_NPVARIANT(*variant);

   if (name == pluginPropertyIdentifiers[ID_VERSION_PROPERTY]) {
      FillString(std::string("1.0"), variant);
      return true;
   }

   //unknown property
   return false;
}

bool pluginSetProperty (PluginObject *obj, NPIdentifier name, const NPVariant *variant)
{

   return false;
}

// handle our plugin methods using standard np plugin conventions.
bool pluginInvoke (PluginObject *obj, NPIdentifier name, NPVariant *args, unsigned argCount, NPVariant *result)
{
   VOID_TO_NPVARIANT(*result);

   // sanity check
   if (argCount > 16)
      return false;

   // plugin.startup(); - called once web page is fully loaded and plugin (including Torque 3D) is initialized
   if (name == pluginMethodIdentifiers[ID_STARTUP_METHOD]) {
      result->type = NPVariantType_Void;

      Startup();
      return true;
   }
   // plugin.setVariable("$MyVariable", 42); - set a Torque 3D console variable
   else if (name == pluginMethodIdentifiers[ID_SETVARIABLE_METHOD]) {
      result->type = NPVariantType_Void;
      if (argCount != 2)
         return false;

      std::string arg0(MY_NPVARIANT_TO_STRING(args[0]));
      std::string arg1(MY_NPVARIANT_TO_STRING(args[1]));
      WebCommon::SetVariable(arg0.c_str(), arg1.c_str());
      return true;
   }
   // plugin.getVariable("$MyVariable"); - get a Torque 3D console variable
   else if (name == pluginMethodIdentifiers[ID_GETVARIABLE_METHOD]) {
      if (argCount != 1)
         return false;

      std::string value;
      std::string arg0(MY_NPVARIANT_TO_STRING(args[0]));
      value = WebCommon::GetVariable(arg0.c_str());
      FillString(value, result);

      return true;
   }
   // plugin.exportFunction("MyJavascriptFunction",3); - export a Javascript function to the Torque 3D console system via its name and argument count
   else if (name == pluginMethodIdentifiers[ID_EXPORTFUNCTION_METHOD]) {
      result->type = NPVariantType_Void;
      if (argCount != 2)
         return false;

      std::string fname(MY_NPVARIANT_TO_STRING(args[0]));

      int argCount = 0;

      if (NPVARIANT_IS_DOUBLE(args[1]))
         argCount = NPVARIANT_TO_DOUBLE(args[1]);
      else
         argCount = NPVARIANT_TO_INT32(args[1]);

      ExportFunction(fname.c_str(), argCount);
      return true;
   }
   // var result = plugin.callScript("mySecureFunction('one', 'two', 'three');"); - call a TorqueScript function marked as secure in webConfig.h with supplied arguments
   else if (name == pluginMethodIdentifiers[ID_CALLSCRIPT_METHOD]) {

      if (argCount != 1)
         return false;

      std::string value;
      std::string code(MY_NPVARIANT_TO_STRING(args[0]));
      value = CallScript(code.c_str());
      FillString(value, result);
      return true;
   }


   return false;
}

bool pluginInvokeDefault (PluginObject *obj, NPVariant *args, unsigned argCount, NPVariant *result)
{
   VOID_TO_NPVARIANT(*result);
   return false;
}

void pluginInvalidate ()
{
   // Make sure we've released any remaining references to JavaScript
   // objects.
}

NPObject *pluginAllocate (NPP npp, NPClass *theClass)
{

   PluginObject *newInstance = new PluginObject;

   gPluginObject = newInstance;

   if (!identifiersInitialized)
   {
      identifiersInitialized = true;
      initializeIdentifiers();
   }

   // platform specific NPWebGamePlugin instantiation
   newInstance->webPlugin = new NPWebGamePlugin(npp);

   gInitialized = false;
   gJavaScriptExports.clear();

   return (NPObject *)newInstance;
}

void pluginDeallocate (PluginObject *obj) 
{
   delete obj;
   gPluginObject = NULL;
}

int32 NPP_Write (NPP instance, NPStream *stream, int32_t offset, int32_t len, void *buffer);


NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* pFuncs)
{
   if (pFuncs == NULL) {
      return NPERR_INVALID_FUNCTABLE_ERROR;
   }

   // Safari sets the size field of pFuncs to 0
   if (pFuncs->size == 0)
      pFuncs->size = sizeof(NPPluginFuncs);
   if (pFuncs->size < sizeof(NPPluginFuncs)) {
      return NPERR_INVALID_FUNCTABLE_ERROR;
   }

   pFuncs->version       = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
   pFuncs->newp          = NPP_New;
   pFuncs->destroy       = NPP_Destroy;
   pFuncs->setwindow     = NPP_SetWindow;
   pFuncs->newstream     = NPP_NewStream;
   pFuncs->destroystream = NPP_DestroyStream;
   pFuncs->asfile        = NPP_StreamAsFile;
   pFuncs->writeready    = NPP_WriteReady;
   pFuncs->write         = NPP_Write;
   pFuncs->print         = NPP_Print;
   pFuncs->event         = NPP_HandleEvent;
   pFuncs->urlnotify     = NPP_URLNotify;
   pFuncs->getvalue      = NPP_GetValue;
   pFuncs->setvalue      = NPP_SetValue;
   pFuncs->javaClass     = NULL;

   return NPERR_NO_ERROR;
}

NPError OSCALL NP_Initialize(NPNetscapeFuncs* pFuncs)
{
   static bool _initialized = false;
   if (!_initialized) {
      _initialized = true;
   }

   if (pFuncs == NULL)
      return NPERR_INVALID_FUNCTABLE_ERROR;

   if ((pFuncs->version >> 8) > NP_VERSION_MAJOR)
      return NPERR_INCOMPATIBLE_VERSION_ERROR;

   // Safari sets the pfuncs size to 0
   if (pFuncs->size == 0)
      pFuncs->size = sizeof(NPNetscapeFuncs);
   if (pFuncs->size < sizeof (NPNetscapeFuncs))
      return NPERR_INVALID_FUNCTABLE_ERROR;

   NPNFuncs = *pFuncs;

   return NPERR_NO_ERROR;
}

NPError OSCALL NP_Shutdown()
{
   if (WebCommon::gTorque3DModule)
      WebCommon::ShutdownTorque3D();
      
   return NPERR_NO_ERROR;
}


NPError NPP_New(NPMIMEType    pluginType,
                NPP instance, uint16 mode,
                int16 argc,   char *argn[],
                char *argv[], NPSavedData *saved)
{
   WebCommon::gPluginMIMEType = pluginType;

   if (gPluginObject)
   {
      WebCommon::MessageBox( 0, "This plugin allows only one instance", "Error");
      return NPERR_GENERIC_ERROR;
   }

   // Get the location we're loading the plugin from (http://, file://) including address
   // this is used by the domain locking feature to ensure that your plugin is only
   // being used from your web site

   NPObject* windowObject = NULL;
   NPNFuncs.getvalue( instance, NPNVWindowNPObject, &windowObject );

   NPVariant variantValue;
   NPIdentifier identifier = NPNFuncs.getstringidentifier( "location" );

   if (!NPNFuncs.getproperty( instance, windowObject, identifier, &variantValue ))
      return NPERR_GENERIC_ERROR;

   NPObject *locationObj = variantValue.value.objectValue;

   identifier = NPNFuncs.getstringidentifier( "href" );

   if (!NPNFuncs.getproperty( instance, locationObj, identifier, &variantValue ))
      return NPERR_GENERIC_ERROR;

   std::string url = MY_NPVARIANT_TO_STRING(variantValue);

   if (!WebCommon::CheckDomain(url.c_str()))
      return NPERR_GENERIC_ERROR;

   // everything checks out, let's rock

   if (NPNFuncs.version >= 14) {
      // this calls pluginAllocate
      instance->pdata = NPNFuncs.createobject(instance, getPluginClass());
   }
   
#ifndef WIN32
   // On Mac, make sure we're using CoreGraphics (otherwise 3D rendering fails)
   NPNFuncs.setvalue(instance, (NPPVariable)NPNVpluginDrawingModel, (void *) NPDrawingModelCoreGraphics);
#endif

   //PluginObject *plugin = (PluginObject*)instance->pdata;

   return NPERR_NO_ERROR;
}

// here is the place to clean up and destroy the object
NPError NPP_Destroy (NPP instance, NPSavedData** save)
{
   if (instance == NULL)
      return NPERR_INVALID_INSTANCE_ERROR;

   if (instance->pdata != NULL) {
      PluginObject *plugin = (PluginObject *)instance->pdata;
      delete plugin->webPlugin;

      NPNFuncs.releaseobject((NPObject*) instance->pdata);
      instance->pdata = NULL;
   }
   return NPERR_NO_ERROR;
}

NPError NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
   if (variable == NPPVpluginScriptableNPObject) {

      if (instance->pdata == NULL) {
         instance->pdata = NPNFuncs.createobject(instance, getPluginClass());
      }

      NPObject* obj = reinterpret_cast<NPObject*>(instance->pdata);
      NPNFuncs.retainobject(obj);
      void **v = (void **)value;
      *v = obj;
      return NPERR_NO_ERROR;
   }

   return NPERR_GENERIC_ERROR;
}

NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
   return NPERR_GENERIC_ERROR;
}

NPError NPP_NewStream(NPP instance,
                      NPMIMEType type,
                      NPStream* stream, 
                      NPBool seekable,
                      uint16* stype)
{
   if(instance == NULL)
      return NPERR_INVALID_INSTANCE_ERROR;

   return NPERR_NO_ERROR;
}

int32 NPP_WriteReady (NPP instance, NPStream *stream)
{
   if (instance == NULL)
      return NPERR_INVALID_INSTANCE_ERROR;

   return 0x0fffffff;
}

int32 NPP_Write (NPP instance, NPStream *stream, int32_t offset, int32_t len, void *buffer)
{   
   if (instance == NULL)
      return NPERR_INVALID_INSTANCE_ERROR;

   return NPERR_NO_ERROR;
}

NPError NPP_DestroyStream (NPP instance, NPStream *stream, NPError reason)
{
   if(instance == NULL)
      return NPERR_INVALID_INSTANCE_ERROR;

   return NPERR_NO_ERROR;
}


void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{

}

void NPP_StreamAsFile (NPP instance, NPStream* stream, const char* fname)
{
}

void NPP_Print (NPP instance, NPPrint* printInfo)
{
}

int16 NPP_HandleEvent(NPP instance, void* event)
{
   return 0;
}
// browser communicated window changes (creation, etc) here
NPError NPP_SetWindow(NPP instance, NPWindow* window)
{    
   // strange...
   if (!window || !window->window)
      return NPERR_GENERIC_ERROR;
   // strange...
   if (!instance)
      return  NPERR_INVALID_INSTANCE_ERROR;

   // get back the plugin instance object
   PluginObject *plugin = (PluginObject *)instance->pdata;
   NPWebGamePlugin* webPlugin = plugin->webPlugin;
   if (webPlugin) {
      if (!window->window) {

      }
      else {

         // handle platform specific window and Torque 3D initialization
         webPlugin->Open(window);
      }

      return NPERR_NO_ERROR;
   }

   // return an error if no object defined
   return NPERR_GENERIC_ERROR;
}
