// npapidefs.h
// minimal version of the defs from the NS plugin SDK

#ifndef NPAPIDEFS_H
#define NPAPIDEFS_H

#define NP_VERSION_MAJOR 0
#define NP_VERSION_MINOR 11

#define NPVERS_HAS_STREAMOUTPUT   8
#define NPVERS_HAS_NOTIFICATION   9
#define NPVERS_HAS_LIVECONNECT    9
#define NPVERS_HAS_WINDOWLESS     11

#define NPERR_NO_ERROR                    0
#define NPERR_GENERIC_ERROR               1
#define NPERR_INVALID_INSTANCE_ERROR      2
#define NPERR_INVALID_FUNCTABLE_ERROR     3
#define NPERR_MODULE_LOAD_FAILED_ERROR    4
#define NPERR_OUT_OF_MEMORY_ERROR         5
#define NPERR_INVALID_PLUGIN_ERROR        6
#define NPERR_INVALID_PLUGIN_DIR_ERROR    7
#define NPERR_INCOMPATIBLE_VERSION_ERROR  8
#define NPERR_INVALID_PARAM               9
#define NPERR_INVALID_URL                 10
#define NPERR_FILE_NOT_FOUND              11
#define NPERR_NO_DATA                     12
#define NPERR_STREAM_NOT_SEEKABLE         13

#define NP_EMBED  1
#define NP_FULL   2

#define NP_NORMAL      1
#define NP_SEEK        2
#define NP_ASFILE      3
#define NP_ASFILEONLY  4

#define NPRES_DONE         0
#define NPRES_NETWORK_ERR  1
#define NPRES_USER_BREAK   2

typedef unsigned short uint16;
typedef unsigned long uint32;
typedef short int16;
typedef long int32;

typedef unsigned char NPBool;
typedef int16   NPError;
typedef int16   NPReason;
typedef char*   NPMIMEType;
typedef HRGN NPRegion;

typedef void*  JRIGlobalRef;
struct JRIEnvInterface;
typedef struct JRIEnvInterface	JRIEnvInterface;
typedef const JRIEnvInterface*	JRIEnv;
struct _jobject;
typedef struct _jobject *jobject;
typedef jobject jref;

typedef struct _NPRect
{
    uint16  top;
    uint16  left;
    uint16  bottom;
    uint16  right;
} NPRect;

typedef struct _NPP
{
    void* pdata;
    void* ndata;
} NPP_t;

typedef NPP_t* NPP;

typedef struct _NPStream
{
    void* pdata;
    void* ndata;
    const char* url;
    uint32 end;
    uint32 lastmodified;
    void* notifyData;
} NPStream;

typedef enum {
  NPPVpluginNameString = 1,
  NPPVpluginDescriptionString,
  NPPVpluginWindowBool,
  NPPVpluginTransparentBool
} NPPVariable;

typedef enum {
  NPNVxDisplay = 1,
  NPNVxtAppContext,
    NPNVnetscapeWindow,
  NPNVjavascriptEnabledBool,
  NPNVasdEnabledBool,
  NPNVisOfflineBool
} NPNVariable;

typedef enum {
    NPWindowTypeWindow = 1,
    NPWindowTypeDrawable
} NPWindowType;

typedef struct _NPSavedData
{
    int32 len;
    void* buf;
} NPSavedData;

typedef struct _NPByteRange
{
    int32 offset;
    uint32  length;
    struct _NPByteRange* next;
} NPByteRange;

typedef struct _NPFullPrint
{
    NPBool  pluginPrinted;
    NPBool  printOne;
    void* platformPrint;
} NPFullPrint;

typedef struct _NPWindow
{
    void* window;
    int32 x;
    int32 y;
    uint32  width;
    uint32  height;
    NPRect  clipRect;
    NPWindowType type;
} NPWindow;

typedef struct _NPEmbedPrint
{
    NPWindow  window;
    void* platformPrint;
} NPEmbedPrint;

typedef struct _NPPrint
{
    uint16  mode;
    union
    {
      NPFullPrint   fullPrint;
      NPEmbedPrint  embedPrint;
    } print;
} NPPrint;

typedef struct _NPEvent
{
    uint16   event;
    uint32   wParam;
    uint32   lParam;
} NPEvent;


typedef NPError (*NPP_NewUPP)(NPMIMEType,NPP,uint16,int16,char* argn[],char* argv[],NPSavedData*);
typedef NPError (*NPP_DestroyUPP)(NPP instance, NPSavedData** save);
typedef NPError (*NPP_SetWindowUPP)(NPP,NPWindow*);
typedef NPError (*NPP_NewStreamUPP)(NPP,NPMIMEType,NPStream*,NPBool,uint16*);
typedef NPError (*NPP_DestroyStreamUPP)(NPP,NPStream*,NPReason);
typedef int32 (*NPP_WriteReadyUPP)(NPP instance,NPStream*);
typedef int32 (*NPP_WriteUPP)(NPP,NPStream*,int32,int32,void*);
typedef void (*NPP_StreamAsFileUPP)(NPP,NPStream*,const char*);
typedef void (*NPP_PrintUPP)(NPP,NPPrint*);
typedef int16 (*NPP_HandleEventUPP)(NPP,void*);
typedef void (*NPP_URLNotifyUPP)(NPP,const char*,NPReason,void*);
typedef NPError (*NPP_GetValueUPP)(NPP,NPPVariable,void*);
typedef NPError (*NPP_SetValueUPP)(NPP,NPNVariable,void*);
typedef NPError (*NPN_GetValueUPP)(NPP,NPNVariable,void*);
typedef NPError (*NPN_SetValueUPP)(NPP,NPPVariable,void*);
typedef NPError (*NPN_GetURLNotifyUPP)(NPP,const char*,const char*,void*);
typedef NPError (*NPN_PostURLNotifyUPP)(NPP,const char*,const char*,uint32,const char*,NPBool,void*);
typedef NPError (*NPN_GetURLUPP)(NPP,const char*,const char*);
typedef NPError (*NPN_PostURLUPP)(NPP,const char*,const char*,uint32,const char*,NPBool);
typedef NPError (*NPN_RequestReadUPP)(NPStream*,NPByteRange*);
typedef NPError (*NPN_NewStreamUPP)(NPP,NPMIMEType,const char*,NPStream**);
typedef int32 (*NPN_WriteUPP)(NPP,NPStream*,int32,void*);
typedef NPError (*NPN_DestroyStreamUPP)(NPP,NPStream*,NPReason);
typedef void (*NPN_StatusUPP)(NPP instance, const char*);
typedef const char* (*NPN_UserAgentUPP)(NPP);
typedef void* (*NPN_MemAllocUPP)(uint32);
typedef void (*NPN_MemFreeUPP)(void*);
typedef uint32 (*NPN_MemFlushUPP)(uint32);
typedef void (*NPN_ReloadPluginsUPP)(NPBool);
typedef JRIEnv* (*NPN_GetJavaEnvUPP)(void);
typedef jref (*NPN_GetJavaPeerUPP)(NPP);
typedef void (*NPN_InvalidateRectUPP)(NPP,NPRect*);
typedef void (*NPN_InvalidateRegionUPP)(NPP,NPRegion);
typedef void (*NPN_ForceRedrawUPP)(NPP);


typedef struct _NPPluginFuncs {
    uint16 size;
    uint16 version;
    NPP_NewUPP newp;
    NPP_DestroyUPP destroy;
    NPP_SetWindowUPP setwindow;
    NPP_NewStreamUPP newstream;
    NPP_DestroyStreamUPP destroystream;
    NPP_StreamAsFileUPP asfile;
    NPP_WriteReadyUPP writeready;
    NPP_WriteUPP write;
    NPP_PrintUPP print;
    NPP_HandleEventUPP event;
    NPP_URLNotifyUPP urlnotify;
    JRIGlobalRef javaClass;
    NPP_GetValueUPP getvalue;
    NPP_SetValueUPP setvalue;
} NPPluginFuncs;

typedef struct _NPNetscapeFuncs {
    uint16 size;
    uint16 version;
    NPN_GetURLUPP geturl;
    NPN_PostURLUPP posturl;
    NPN_RequestReadUPP requestread;
    NPN_NewStreamUPP newstream;
    NPN_WriteUPP write;
    NPN_DestroyStreamUPP destroystream;
    NPN_StatusUPP status;
    NPN_UserAgentUPP uagent;
    NPN_MemAllocUPP memalloc;
    NPN_MemFreeUPP memfree;
    NPN_MemFlushUPP memflush;
    NPN_ReloadPluginsUPP reloadplugins;
    NPN_GetJavaEnvUPP getJavaEnv;
    NPN_GetJavaPeerUPP getJavaPeer;
    NPN_GetURLNotifyUPP geturlnotify;
    NPN_PostURLNotifyUPP posturlnotify;
    NPN_GetValueUPP getvalue;
    NPN_SetValueUPP setvalue;
    NPN_InvalidateRectUPP invalidaterect;
    NPN_InvalidateRegionUPP invalidateregion;
    NPN_ForceRedrawUPP forceredraw;
} NPNetscapeFuncs;


#endif // NPAPIDEFS_H
