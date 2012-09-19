/* -*- Mode: C; tab-width: 4; -*- */
/* npmngplg.c
 * MNG browser plugin
 * By Jason Summers
 * Based on libmng by Gerard Juyn
 */

#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>

#include "resource.h"
#include "libmng.h"
#include "jversion.h"      // part of libjpeg
#include "zlib.h"          // for zlibVersion
#include "npapidefs.h"

#define MNGPLG_CMS
//#define MNGPLG_TRACE

#define IDBASE           47000
#define ID_SAVEAS        (IDBASE+0)
#define ID_COPYIMAGE     (IDBASE+1)
#define ID_COPYURL       (IDBASE+2)
#define ID_VIEWIMAGE     (IDBASE+3)
#define ID_ABOUT         (IDBASE+4)
#define ID_FREEZE        (IDBASE+5)
#define ID_RESTARTANIM   (IDBASE+6)
#define ID_COPYLINKLOC   (IDBASE+7)
#define ID_STOPANIM      (IDBASE+8)
#define ID_SHOWERROR     (IDBASE+9)
#define ID_PROPERTIES    (IDBASE+10)

#define MNGPLGVERS    "1.0.1"

/* instance-specific data */
typedef struct pluginstruct_
{
	NPWindow*  fWindow;
	uint16     fMode;
	HWND       fhWnd;
	WNDPROC    fDefaultWindowProc;
	NPP        instance;

	mng_handle mng;

#define STATE_INIT        0
#define STATE_LOADING     1  // stream opened
#define STATE_VALIDFRAME  2  // at least one frame has been displayed
#define STATE_LOADED      3  // image loaded; stream closed


#define MAXLEN_TEXT   5000

#define MAXLEN_URL     300
#define MAXLEN_TARGET  100

	// I think I'm not doing this very well. Probably there really needs to be
	// two state variables, one for loading from the network, and one for
	// the libmng processing. (or use libmng's new getstate API?)
	int loadstate;

	int paintedyet;

	int scrolling;    // allow scrolling of the image?
	int xscrollpos, yscrollpos;
	int windowwidth, windowheight; // client size of current window

	int diblinesize;
	DWORD dibsize;
	DWORD filesize;
	DWORD libmngpos;  // count of bytes that have been sent to libmng
	DWORD byteswanted;   // libmng asked for this many more bytes (add to libmngpos)

	unsigned char *mngdata;    // stores the MNG file in memory
	DWORD bytesloaded;  // 
	DWORD bytesalloc;   // size of mngdata
	int needresume;   // if previous mng_readdisplay call returned NEEDMOREDATA

	char *textdata;

	int errorflag;    // set if an error occurs that prevents displaying the image
	char errormsg[256];

	unsigned char *lpdib;    // pointer to header section of dib
	unsigned char *lpdibbits;   // pointer to "bits" section of dib (follows the header)
	LPBITMAPINFOHEADER lpdibinfo;  // alias for lpdib
	int frozen;
	int timer_set;
	int timer2_set;
	int dynamicmng;
	int mouse_over_mng;
	int mouse_captured;

	int force_bgcolor;
	mng_uint16 bg_r,bg_g,bg_b;  // background color

	unsigned char url[MAX_PATH];  // the url of the stream

	int islink;
	HCURSOR linkcursor;
	HBRUSH bkgdbrush;
	unsigned char linkurl[MAXLEN_URL];
	unsigned char linktarget[MAXLEN_TARGET];

} PluginInstance;


/* global variables */

#ifdef MNGPLG_TRACE
static FILE *tracefile;
#endif

static const char* gInstanceLookupString = "pdata";
static HMODULE g_hInst = NULL;
static HCURSOR hcurHandNS;
static HFONT hfontMsg;

/* function prototypes */
LRESULT CALLBACK DlgProcAbout(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DlgProcProp(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PluginWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

void set_scrollbars(PluginInstance *This);


//////////////////////////// NPN_functions

static NPNetscapeFuncs* g_pNavigatorFuncs;
static NPPluginFuncs* g_pluginFuncs;

static const char* NPN_UserAgent(NPP instance)
{
    return g_pNavigatorFuncs->uagent(instance);
}

static NPError NPN_GetURL(NPP instance, const char *url, const char *target)
{
    return g_pNavigatorFuncs->geturl(instance, url, target);
}



/* ----------------------------------------------------------------------- */

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reason, LPVOID lpReserved)
{
    switch(reason) {
    case DLL_PROCESS_ATTACH:
		g_hInst=hModule;
		break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}


/* ----------------------------------------------------------------------- */

static void warn(PluginInstance *This, char *fmt, ...)
{
	va_list ap;
	char buf[2048];
	HWND hwnd;

	va_start(ap, fmt);
	wvsprintf(buf,fmt, ap);
	va_end(ap);

	if(This) hwnd= This->fhWnd;
	else hwnd=NULL;

	MessageBox(hwnd,buf,"MNG Plug-in",MB_OK|MB_ICONWARNING);
}


static void set_error(PluginInstance *This, char *fmt, ...)
{
	va_list ap;
	char buf[2048];
	HWND hwnd;

	va_start(ap, fmt);
	wvsprintf(buf,fmt, ap);
	va_end(ap);

	if(This) hwnd= This->fhWnd;
	else hwnd=NULL;

	This->errorflag=1;
	lstrcpyn(This->errormsg,buf,256);

	if(This->lpdib) {
		free(This->lpdib);
		This->lpdib=NULL;
	}
	This->xscrollpos = This->yscrollpos = 0;

	if(This->fhWnd)
		InvalidateRect(This->fhWnd,NULL,TRUE);
}

/* ----------------------------------------------------------------------- */
//    MNG callbacks
#define MNGPLG_CALLBACK MNG_DECL

static mng_ptr MNGPLG_CALLBACK memallocfunc(mng_size_t n)
{
	return (mng_ptr) calloc(n,1);
}


static void MNGPLG_CALLBACK memfreefunc(mng_ptr p, mng_size_t n)
{
	free((void*)p);
}


static mng_bool MNGPLG_CALLBACK callback_openstream (mng_handle mng)
{
//	PluginInstance *This;
//	This = (PluginInstance*) mng_get_userdata(mng);
	return MNG_TRUE;
}

static mng_bool MNGPLG_CALLBACK callback_closestream (mng_handle mng)
{
	PluginInstance *This;
	This = (PluginInstance*) mng_get_userdata(mng);
	This->loadstate = STATE_LOADED;  // this is probably redundant

	return MNG_TRUE;
}


static mng_bool MNGPLG_CALLBACK callback_readdata (mng_handle mng,mng_ptr pBuf,
                      mng_uint32 Buflen,mng_uint32 *pRead)
{
	int n;
	PluginInstance *This;
	This = (PluginInstance*) mng_get_userdata(mng);

#ifdef MNGPLG_TRACE
	fprintf(tracefile,"readdata callback buflen=%d loadstate=%d bytesloaded=%d libmngpos=%d\n",
		Buflen,This->loadstate,This->bytesloaded, This->libmngpos);
#endif


	// do we have enough data available?
	if(This->bytesloaded - This->libmngpos >= Buflen) {
		CopyMemory(pBuf,&This->mngdata[This->libmngpos],Buflen);
		(*pRead)= Buflen;
		This->libmngpos += Buflen;

#ifdef MNGPLG_TRACE
	fprintf(tracefile,"returning full: %d\n",Buflen);
#endif
		This->byteswanted=0;
		return MNG_TRUE;
	}
	else if(This->loadstate>=STATE_LOADED) {
		// We don't have the data it asked for, but we're at the end
		// of file, so send it anyway...?

		n=This->bytesloaded-This->libmngpos;

		if(n>0) {
			CopyMemory(pBuf,&This->mngdata[This->libmngpos],n);
			This->libmngpos+=n;
		}
		(*pRead)=n;
		// so what do we return?
#ifdef MNGPLG_TRACE
	fprintf(tracefile,"returning partial: %d\n",n);
#endif
		This->byteswanted=0;
		return MNG_TRUE;
	}

	// else we don't yet have the data it's requesting
#ifdef MNGPLG_TRACE
	fprintf(tracefile,"returning 0\n");
#endif
	(*pRead)=0;
	This->byteswanted=Buflen;
	return MNG_TRUE;
}


static mng_bool MNGPLG_CALLBACK callback_processheader(mng_handle mng,mng_uint32 iWidth,mng_uint32 iHeight)
{
	PluginInstance *This;
	This = (PluginInstance*) mng_get_userdata(mng);

	This->diblinesize = (((iWidth * 24)+31)/32)*4;
	This->dibsize = sizeof(BITMAPINFOHEADER) + This->diblinesize*iHeight;
	This->lpdib = calloc(This->dibsize,1);
	This->lpdibinfo = (LPBITMAPINFOHEADER)This->lpdib;
	This->lpdibbits = &This->lpdib[sizeof(BITMAPINFOHEADER)];
	ZeroMemory((void*)This->lpdib,sizeof(BITMAPINFOHEADER));
	This->lpdibinfo->biSize = sizeof(BITMAPINFOHEADER);
	This->lpdibinfo->biWidth = iWidth;
	This->lpdibinfo->biHeight = iHeight;
	This->lpdibinfo->biPlanes = 1;
	This->lpdibinfo->biBitCount = 24;

	mng_set_canvasstyle (mng, MNG_CANVAS_BGR8);

/*	if(This->fhWnd) {
		if((int)iWidth > This->windowwidth || (int)iHeight > This->windowheight) {
			This->scrolling=1;
		}
	} */

	set_scrollbars(This);
	return MNG_TRUE;
}


static mng_ptr MNGPLG_CALLBACK callback_getcanvasline (mng_handle mng, mng_uint32 iLinenr)
{
	unsigned char *pp;

	PluginInstance *This;
	This = (PluginInstance*) mng_get_userdata(mng);
	pp = (&This->lpdibbits[(This->lpdibinfo->biHeight-1-iLinenr)*This->diblinesize]);
	return (mng_ptr) pp;
}

static mng_bool MNGPLG_CALLBACK callback_refresh (mng_handle mng, mng_uint32 iLeft, mng_uint32 iTop,
                       mng_uint32 iRight, mng_uint32 iBottom)
{
	PluginInstance *This;
	RECT rect;

	This = (PluginInstance*) mng_get_userdata(mng);

	if(This->loadstate<STATE_VALIDFRAME) {
		This->loadstate=STATE_VALIDFRAME;
	}

	if(This->fhWnd) {
		if(This->paintedyet) {
			rect.left= iLeft      - This->xscrollpos;
			rect.top= iTop        - This->yscrollpos;
			rect.right= iLeft+iRight;
			rect.bottom= iTop+iBottom;

			InvalidateRect(This->fhWnd,&rect,FALSE);
		}
		else {
			// Make sure the first paint clears the whole plugin window
			InvalidateRect(This->fhWnd,NULL,TRUE);
			This->paintedyet=1;
		}
		UpdateWindow(This->fhWnd);
	}
	return MNG_TRUE;
}

static mng_uint32 MNGPLG_CALLBACK callback_gettickcount (mng_handle mng)
{
	return GetTickCount();
}


static mng_bool MNGPLG_CALLBACK callback_settimer (mng_handle mng,mng_uint32 iMsecs)
{
	PluginInstance *This;
	This = (PluginInstance*) mng_get_userdata(mng);

	if(This->fhWnd) {
		if(!SetTimer(This->fhWnd,1,(UINT)iMsecs,NULL)) {
			warn(This,"Unable to create a timer for animation");
			This->frozen=1;
			//return MNG_FALSE;
			return MNG_TRUE;
		}
		This->timer_set=1;
	}
	return MNG_TRUE;
}

static mng_bool MNGPLG_CALLBACK callback_processtext(mng_handle mng,
    mng_uint8 iType, mng_pchar zKeyword, mng_pchar zText,
    mng_pchar zLanguage, mng_pchar zTranslation)
{
	PluginInstance *This;
	int pos,i;

	This = (PluginInstance*) mng_get_userdata(mng);

	if(!This->textdata) {
		This->textdata=(char*)malloc(MAXLEN_TEXT+10);
		if(!This->textdata) return MNG_TRUE;
		lstrcpy(This->textdata,"");
	}

	pos=lstrlen(This->textdata);
	if(pos>=(MAXLEN_TEXT-10)) return MNG_TRUE;

	if(pos>0) {    /* separate items with a blank line */
		This->textdata[pos++]='\r';
		This->textdata[pos++]='\n';
		This->textdata[pos++]='\r';
		This->textdata[pos++]='\n';
	}

	for(i=0;zKeyword[i];i++) {
		if(pos<MAXLEN_TEXT)
			This->textdata[pos++]=zKeyword[i];
	}
	This->textdata[pos++]=':';
	This->textdata[pos++]=' ';

	for(i=0;zText[i];i++) {
		if(pos<MAXLEN_TEXT) {
			if(zText[i]=='\n') {
				This->textdata[pos++]='\r';
			}
			This->textdata[pos++]=zText[i];
		}
	}
	This->textdata[pos++]='\0';

	return MNG_TRUE;
}

#ifdef MNGPLG_TRACE
static mng_bool MNGPLG_CALLBACK callback_traceproc (mng_handle mng,
	mng_int32  iFuncnr,
	mng_int32  iFuncseq,
	mng_pchar  zFuncname)
{
	if(tracefile) {
		fprintf(tracefile,"%d\t%d\t%d\t%s\n",(int)mng,iFuncnr,iFuncseq,zFuncname);
	}
	return MNG_TRUE;
}
#endif

/* ----------------------------------------------------------------------- */
static int file_exists(const char *fn)
{
   HANDLE h;

   // try to open with no access
   h=CreateFile(fn,0,FILE_SHARE_READ,NULL,OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL,NULL);
   if(h == INVALID_HANDLE_VALUE) { return 0; }
   CloseHandle(h);
   return 1;
}

static void handle_read_error(PluginInstance *This, mng_retcode rv)
{
	mng_int8     iSeverity;
	mng_chunkid  iChunkname;
	mng_uint32   iChunkseq;
	mng_int32    iExtra1;
	mng_int32    iExtra2;
	mng_pchar    zErrortext;

#ifdef MNGPLG_TRACE
	fprintf(tracefile,"returned: %d\n",rv);
#endif

	switch(rv) {
	case MNG_NOERROR: case MNG_NEEDTIMERWAIT:
		break;

	case MNG_NEEDMOREDATA:
		if(This->loadstate>=STATE_LOADED) {
			set_error(This,"Unexpected end of file");
		}
		else {
			This->needresume=1;
		}
		break;

	case MNG_INVALIDSIG:
		set_error(This,"Invalid or missing MNG file (maybe a 404 Not Found error)");
		break;

	default:

		mng_getlasterror(This->mng, &iSeverity,&iChunkname,&iChunkseq,&iExtra1,
			&iExtra2,&zErrortext);

		if(zErrortext) {
			set_error(This,"Error reported by libmng (%d)\r\n\r\n%s",(int)rv,zErrortext);
		}
		else {
			set_error(This,"Error %d reported by libmng",(int)rv);
		}
	}
}

#ifdef MNGPLG_CMS

static int init_color_management(PluginInstance *This)
{
	mng_set_outputsrgb(This->mng);
	return 1;
}

#endif

// return 1 if okay
static int my_init_mng(PluginInstance *This)
{
	mng_retcode rv;
	int err;

	This->mng = mng_initialize((mng_ptr)This,memallocfunc,memfreefunc,NULL);
	//(mng_memalloc)  (mng_memfree)

#ifdef MNGPLG_CMS
	init_color_management(This);
#endif

	err=0;
	rv=mng_setcb_openstream    (This->mng, callback_openstream   ); if(rv) err++;
	rv=mng_setcb_closestream   (This->mng, callback_closestream  ); if(rv) err++;
	rv=mng_setcb_readdata      (This->mng, callback_readdata     ); if(rv) err++;
	rv=mng_setcb_processheader (This->mng, callback_processheader); if(rv) err++;
	rv=mng_setcb_getcanvasline (This->mng, callback_getcanvasline); if(rv) err++;
	rv=mng_setcb_refresh       (This->mng, callback_refresh      ); if(rv) err++;
	rv=mng_setcb_gettickcount  (This->mng, callback_gettickcount ); if(rv) err++;
	rv=mng_setcb_settimer      (This->mng, callback_settimer     ); if(rv) err++;
	rv=mng_setcb_processtext   (This->mng, callback_processtext  ); if(rv) err++;

#ifdef MNGPLG_TRACE
	rv=mng_setcb_traceproc     (This->mng, callback_traceproc    ); if(rv) err++;
#endif
	if(err) {
		warn(This,"Error setting libmng callback functions");
		return 0;
	}

	rv= mng_set_suspensionmode (This->mng,MNG_TRUE);
	if(rv) {
		warn(This,"Error setting suspension mode");
		return 0;
	}

	// if the web page author provided a bgcolor, use it
	if(This->force_bgcolor) {
		rv=mng_set_bgcolor (This->mng, This->bg_r, This->bg_g, This->bg_b);
	}

#ifdef MNGPLG_TRACE
	fprintf(tracefile,"initial readdisplay\n");
#endif

	handle_read_error(This, mng_readdisplay(This->mng) );
	return 1;
}

/* Global initialization */
static NPError NPP_Initialize(void)
{
	if(!g_hInst) {
		warn(NULL,"MNG plugin error: Cannot load resources");
	}

#ifdef MNGPLG_TRACE
	tracefile=fopen("c:\\temp\\mngtrace.txt","w");
#endif

#ifndef IDC_HAND
#define IDC_HAND MAKEINTRESOURCE(32649)
#endif
	hcurHandNS = LoadCursor(NULL,IDC_HAND);
	if(!hcurHandNS) {
		hcurHandNS=LoadCursor(g_hInst,"CURHAND_NS");
	}

	hfontMsg=CreateFont(-12,0,0,0,FW_DONTCARE,TRUE,0,0,ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DRAFT_QUALITY,
		VARIABLE_PITCH|FF_SWISS,"Arial");
	return NPERR_NO_ERROR;
}

/* Global shutdown */
static void NPP_Shutdown(void)
{
#ifdef MNGPLG_TRACE
	if(tracefile) {
		fclose(tracefile);
		tracefile=NULL;
	}
#endif
	if(hfontMsg) DeleteObject((HGDIOBJ)hfontMsg);
	return;
}


static unsigned char gethex(const char *s)
{
	int v[2];
	int i;

	v[0]=v[1]=0; 
	for(i=0;i<2;i++) {
		if(s[i]>='a' && s[i]<='f') v[i]=s[i]-87;
		if(s[i]>='A' && s[i]<='F') v[i]=s[i]-55;
		if(s[i]>='0' && s[i]<='9') v[i]=s[i]-48;
	}
	return (unsigned char)(v[0]*16+v[1]);
}

static void hexcolor2rgb(const char *s, mng_uint16 *r, mng_uint16 *g, mng_uint16 *b)
{
	if(lstrlen(s)!=7) return;
	if(s[0]!='#') return;
	(*r)= gethex(&s[1]); (*r)= ((*r)<<8)|(*r);
	(*g)= gethex(&s[3]); (*g)= ((*g)<<8)|(*g);
	(*b)= gethex(&s[5]); (*b)= ((*b)<<8)|(*b);
}

static void find_window_size(PluginInstance *This)
{
	RECT r;
	if(This->scrolling) {  // make sure scrollbars exist if needed
		ShowScrollBar(This->fhWnd,SB_BOTH,TRUE);
	}
	GetClientRect(This->fhWnd, &r);
	This->windowwidth=r.right;
	This->windowheight=r.bottom;
}

static void set_scrollbars(PluginInstance *This)
{
	SCROLLINFO si;
	int maxpos;

	if(!This->scrolling) return;
	if(!This->fhWnd) return;

	ZeroMemory(&si,sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);

	// horizontal
	if(This->lpdib) {
		maxpos=This->lpdibinfo->biWidth-This->windowwidth;
		if(maxpos<0) maxpos=0;
		if(This->xscrollpos>maxpos) This->xscrollpos=maxpos;
		if(This->xscrollpos<0) This->xscrollpos=0;
		
		si.fMask = SIF_ALL|SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = This->lpdibinfo->biWidth -1;
		si.nPage = This->windowwidth;
		si.nPos  = This->xscrollpos;
	}
	else {  // no image to display
		si.fMask = SIF_ALL|SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = 0;
		si.nPage = 1;
		si.nPos  = 0;
	}
	SetScrollInfo(This->fhWnd,SB_HORZ,&si,TRUE);

	// vertical
	if(This->lpdib) {
		maxpos=This->lpdibinfo->biHeight-This->windowheight;
		if(maxpos<0) maxpos=0;
		if(This->yscrollpos>maxpos) This->yscrollpos=maxpos;
		if(This->yscrollpos<0) This->yscrollpos=0;

		si.fMask = SIF_ALL|SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = This->lpdibinfo->biHeight -1;
		si.nPage = This->windowheight;
		si.nPos  = This->yscrollpos;
	}
	SetScrollInfo(This->fhWnd,SB_VERT,&si,TRUE);
}


#define SCROLLLINE  40

static void scrollmsg(PluginInstance *This, UINT msg,int code, short int pos)
{
	int page;
	int dx, dy;     // amount of scrolling
	int x_orig, y_orig;

	if(!This->scrolling) return;
	if(!This->lpdib) return;

	x_orig=This->xscrollpos;
	y_orig=This->yscrollpos;

	if(msg==WM_HSCROLL) {
		page=This->windowwidth-15;
		if(page<SCROLLLINE) page=SCROLLLINE;

		switch(code) {
		case SB_LINELEFT: This->xscrollpos-=SCROLLLINE; break;
		case SB_LINERIGHT: This->xscrollpos+=SCROLLLINE; break;
		case SB_PAGELEFT: This->xscrollpos-=page; break;
		case SB_PAGERIGHT: This->xscrollpos+=page; break;
		case SB_LEFT: This->xscrollpos=0; break;
		case SB_RIGHT: This->xscrollpos=This->lpdibinfo->biWidth; break;
		case SB_THUMBTRACK: This->xscrollpos=pos; break;
		default: return;
		}
		set_scrollbars(This);
	}
	else if(msg==WM_VSCROLL) {
		page=This->windowheight-15;
		if(page<SCROLLLINE) page=SCROLLLINE;

		switch(code) {
		case SB_LINEUP: This->yscrollpos-=SCROLLLINE; break;
		case SB_LINEDOWN: This->yscrollpos+=SCROLLLINE; break;
		case SB_PAGEUP: This->yscrollpos-=page; break;
		case SB_PAGEDOWN: This->yscrollpos+=page; break;
		case SB_TOP: This->yscrollpos=0; break;
		case SB_BOTTOM: This->yscrollpos=This->lpdibinfo->biHeight; break;
		case SB_THUMBTRACK: This->yscrollpos=pos; break;
		default: return;
		}
		set_scrollbars(This);
	}

	dx= x_orig - This->xscrollpos;
	dy= y_orig - This->yscrollpos;

	if(dx || dy) {  // if any change
		// GetClientRect(This->fhWnd,&cliprect);
		ScrollWindowEx(This->fhWnd,dx,dy,NULL,NULL /*&cliprect*/,NULL,NULL,SW_INVALIDATE);
	}
}

/* Once-per-instance initialization */
static NPError NPP_New(NPMIMEType pluginType,NPP instance,uint16 mode,
		int16 argc,char* argn[],char* argv[],NPSavedData* saved)
{
	PluginInstance* This;
	int i;
	
	if (instance == NULL) {
		return NPERR_INVALID_INSTANCE_ERROR;
	}
	instance->pdata = calloc(sizeof(PluginInstance),1);


	This = (PluginInstance*) instance->pdata;
	if (This == NULL) {
	    return NPERR_OUT_OF_MEMORY_ERROR;
	}

	This->force_bgcolor=1;
	This->bg_r = This->bg_g = This->bg_b = 0xffff;

	/* record some info for later lookup */
	This->fWindow = NULL;
	This->fMode = mode;
	
	This->fhWnd = NULL;
	This->fDefaultWindowProc = NULL;
	This->instance = instance;  /* save the instance id for reverse lookups */
	This->scrolling = (mode==NP_FULL);
	This->xscrollpos = This->yscrollpos = 0;
	This->windowwidth = This->windowheight = 0;

	This->loadstate = STATE_INIT;
	This->paintedyet = 0;
	This->mng=0;
	This->lpdib=NULL;
	lstrcpy(This->url,"");
	This->frozen=0;
	This->needresume=0;
	This->errorflag=0;
	lstrcpy(This->errormsg,"");

	This->dibsize = This->filesize = 0;

	lstrcpy(This->linkurl,"");
	lstrcpy(This->linktarget,"_self");
	This->islink=0;
	
	This->timer_set=0;
	This->timer2_set=0;
	This->dynamicmng= -1;
	This->mouse_over_mng=0;
	This->mouse_captured=0;

	This->linkcursor = hcurHandNS;

	// examine the <embed> tag arguments
	for(i=0;i<argc;i++) {
		if(!_stricmp(argn[i],"bgcolor")) {
			This->force_bgcolor=1;
			hexcolor2rgb(argv[i],&This->bg_r,&This->bg_g,&This->bg_b);
		}
		else if(!_stricmp(argn[i],"href")) {
			lstrcpyn(This->linkurl,argv[i],MAXLEN_URL);
			This->islink=1;
		}
		else if(!_stricmp(argn[i],"target")) {
			lstrcpyn(This->linktarget,argv[i],MAXLEN_TARGET);
		}
	}

	This->bkgdbrush=NULL;
	if(This->force_bgcolor)
		This->bkgdbrush=CreateSolidBrush(RGB(This->bg_r,This->bg_g,This->bg_b));

	return NPERR_NO_ERROR;
}

static void BeforeDestroyWindow(PluginInstance *This)
{
	if(This->timer_set) {
		KillTimer(This->fhWnd,1);
		This->timer_set=0;
	}
	if(This->timer2_set) {
		KillTimer(This->fhWnd,2);
		This->timer2_set=0;
	}
	if(This->mouse_captured) {
		ReleaseCapture();
		This->mouse_captured=0;
	}

	SetWindowLong( This->fhWnd, GWL_WNDPROC, (LONG)This->fDefaultWindowProc); // unsubclass
	This->fDefaultWindowProc = NULL;
	This->fhWnd = NULL;
}

static NPError NPP_Destroy(NPP instance, NPSavedData** save)
{
	PluginInstance* This;

	if (instance == NULL) return NPERR_INVALID_INSTANCE_ERROR;
	This = (PluginInstance*) instance->pdata;

	if(!This) return NPERR_INVALID_INSTANCE_ERROR;

	if(This->mng) {
		This->dynamicmng=0;
		mng_cleanup(&This->mng); 
		This->mng=0;
	}
	if(This->lpdib) {
		free(This->lpdib);
		This->lpdib=NULL;
	}
	if(This->mngdata) {
		free(This->mngdata);
		This->mngdata=NULL;
		This->bytesalloc=0;
	}
	if(This->textdata) {
		free(This->textdata);
		This->textdata=NULL;
	}

	if( This->fhWnd ) { // un-subclass the plugin window
		BeforeDestroyWindow(This);
	}

	if(This->bkgdbrush) DeleteObject((HGDIOBJ)This->bkgdbrush);

	if(This) {
		if(instance->pdata) free(instance->pdata);
		instance->pdata = NULL;
	}

	return NPERR_NO_ERROR;
}

/* Browser is providing us with a window */
static NPError NPP_SetWindow(NPP instance, NPWindow* window)
{
	NPError result = NPERR_NO_ERROR;
	PluginInstance* This;

	if (instance == NULL) return NPERR_INVALID_INSTANCE_ERROR;

	This = (PluginInstance*) instance->pdata;

	if( This->fhWnd != NULL ) {   /* If we already have a window... */

		if( (window == NULL) || ( window->window == NULL ) ) {
			/* There is now no window to use. get rid of the old
			 * one and exit. */
			BeforeDestroyWindow(This);
			This->fWindow=window;
			return NPERR_NO_ERROR;
		}

		else if ( This->fhWnd == (HWND) window->window ) {
			/* The new window is the same as the old one. Redraw and get out. */
			This->fWindow=window;

			InvalidateRect( This->fhWnd, NULL, FALSE );
			/* UpdateWindow( This->fhWnd ); */
			return NPERR_NO_ERROR;
		}
		else {
			/* Unsubclass the old window, so that we can subclass the new
			 * one later. */
			BeforeDestroyWindow(This);
		}
	}
	else if( (window == NULL) || ( window->window == NULL ) ) {
		/* We can just get out of here if there is no current
		 * window and there is no new window to use. */
		This->fWindow=window;

		return NPERR_NO_ERROR;
	}

	/* Subclass the new window so that we can begin drawing and
	 * receiving window messages. */
	This->fDefaultWindowProc = (WNDPROC)SetWindowLong( (HWND)window->window, GWL_WNDPROC, (LONG)PluginWindowProc);
	This->fhWnd = (HWND) window->window;
	SetProp( This->fhWnd, gInstanceLookupString, (HANDLE)This);

	This->fWindow = window;
	find_window_size(This);

	set_scrollbars(This);

	InvalidateRect( This->fhWnd, NULL, TRUE );
	UpdateWindow( This->fhWnd );

	return result;
}

// browser is announcing its intent to send data to us
static NPError NPP_NewStream(NPP instance,NPMIMEType type,NPStream *stream, 
	      NPBool seekable,uint16 *stype) {
	PluginInstance* This;

	if(instance==NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	This = (PluginInstance*) instance->pdata;
	if(!This)
		return NPERR_GENERIC_ERROR;

	/* save the URL for later */
	lstrcpyn(This->url,stream->url,MAX_PATH);


	This->libmngpos=0;
	This->bytesloaded=0;
	This->bytesalloc=0;
	This->byteswanted=0;
	This->mngdata=NULL;
	This->textdata=NULL;

	// if we know the total length of the stream in advance 
	// (most of the time we will, hopefully), allocate that amount.
	if(stream->end > 0) {
		This->mngdata = malloc(stream->end);
		This->bytesalloc= stream->end;
	}

	my_init_mng(This);

	This->loadstate=STATE_LOADING;

	(*stype)=NP_NORMAL;

	return NPERR_NO_ERROR;
}

static int32 NPP_WriteReady(NPP instance, NPStream *stream)
{
	/* Number of bytes ready to accept in NPP_Write() */
	/* We can handle any amount, so just return some really big number. */
	return (int32)0X0FFFFFFF;
}

#define ALLOC_CHUNK_SIZE 131072

static int32 NPP_Write(NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{
	PluginInstance* This;

#ifdef MNGPLG_TRACE
	fprintf(tracefile,"NPP_Write offs=%d len=%d\n",offset,len);
#endif


	if(!instance) return -1;
	This = (PluginInstance*) instance->pdata;
	if(!This) return -1;
	if(len<1) return len;

	if(offset+len > (int)This->bytesalloc) {  // oops, overflowed our memory buffer
		This->bytesalloc += ALLOC_CHUNK_SIZE;
		if(This->mngdata) {
			This->mngdata=realloc(This->mngdata, This->bytesalloc);
		}
		else {  // first time
			This->mngdata=malloc(This->bytesalloc);
		}
		if(!This->mngdata) {
			warn(This,"Cannot allocate memory for image (%d,%d,%p",offset,len,buffer);
			return -1;
		}
	}

	// now we should have enough room to copy the data to memory

	CopyMemory(&This->mngdata[offset],buffer,len);

	This->bytesloaded = offset+len;

	// now, check if it's time to call mng_read_resume
	if(This->needresume &&
		(This->bytesloaded >= (This->libmngpos + This->byteswanted)) )
	{
		This->needresume=0;
//		handle_read_error(This, mng_read_resume(This->mng) );

#ifdef MNGPLG_TRACE
	fprintf(tracefile,"NPP_Write display_resume bytesloaded=%d libmngpos=%d byteswanted=%d\n",
		This->bytesloaded,This->libmngpos,This->byteswanted);
#endif

		handle_read_error(This, mng_display_resume(This->mng) );
	}


	return len; // The number of bytes accepted -- we always accept them all.
}

/* DestroyStream gets called after the file has finished loading,
 */
static NPError NPP_DestroyStream(NPP instance, NPStream *stream, NPError reason)
{
	PluginInstance* This;
	if(!instance) return NPERR_INVALID_INSTANCE_ERROR;

	This = (PluginInstance*) instance->pdata;
//	if(reason==NPRES_DONE) {
	This->filesize = This->bytesloaded;
	This->loadstate = STATE_LOADED;
//	}

	if(reason!=NPRES_DONE) {
		set_error(This,"Image load failed or was canceled (%d)",(int)reason);
		This->needresume=0;
		if(This->timer_set) { KillTimer(This->fhWnd,1); This->timer_set=0; }
		return NPERR_NO_ERROR;
	}

#ifdef MNGPLG_TRACE
	fprintf(tracefile,"NPP_DestroyStream reason=%d needresume=%d\n",reason,This->needresume);
#endif

	if(This->needresume) {
		This->needresume=0;
//		handle_read_error(This, mng_read_resume(This->mng) );
		handle_read_error(This, mng_display_resume(This->mng) );
//		This->needresume=0;
	}

	return NPERR_NO_ERROR;
}


static void NPP_StreamAsFile(NPP instance, NPStream *stream, const char* fname)
{
	return;
}

// Print embedded plug-in (via the browser's Print command)
static void NPP_Print(NPP instance, NPPrint* printInfo)
{
	PluginInstance* This;
	if (instance == NULL) return;
	This = (PluginInstance*) instance->pdata;

	if(printInfo == NULL) {
		// Some browsers (Netscape) set printInfo to NULL to tell the plugin
		// to print in full page mode (this may be a bug).
		// PrintFullPage();  -- full page printing not implemented
		return;
	}
	
	if (printInfo->mode == NP_FULL) {
		/* the plugin is full-page, and the browser is giving it a chance
		 * to print in the manner of its choosing */
		void* platformPrint = printInfo->print.fullPrint.platformPrint;
		NPBool printOne =  printInfo->print.fullPrint.printOne;
		
		/* Setting this to FALSE and returning *should* cause the browser to
		 * call NPP_Print again, this time with mode=NP_EMBED.
		 * However, that doesn't happen with any browser I've ever seen :-(.
		 * Instead of the following line, you will probably need to implement
		 * printing yourself.  You also might as well set pluginPrinted to TRUE,
		 * though every browser I've tested ignores it. */
		printInfo->print.fullPrint.pluginPrinted = FALSE;
		/*  or */
		/*  PrintFullPage();
		 *  printInfo->print.fullPrint.pluginPrinted = TRUE;  */
	}
	else {	// we are embedded, and the browser had provided a printer context
		HDC pdc;
		int prevstretchmode;
		NPWindow* printWindow;
		
		if(This->loadstate < STATE_VALIDFRAME) return;

		printWindow= &(printInfo->print.embedPrint.window);

		/* embedPrint.platformPrint is a Windows device context in disguise */

		/* The definition of NPWindow changed between API verion 0.9 and 0.11,
		 * increasing in size from 28 to 32 bytes. This normally makes it
		 * impossible for version 0.9 browsers to print version 0.11 plugins
		 * (because the platformPrint field ends up at the wrong offset) --
		 * unless the plugin takes special care to detect this situation.
		 * To work around it, if we are compiled with API 0.11 or higher,
		 * and the browser is version 0.9 or earlier, we look for the HDC
		 * 4 bytes earlier, at offset 28 instead of 32 (of the embedPrint
		 * sub-structure).
		 */

		if(sizeof(NPWindow)>28 &&     /* i.e. is plugin API >= 0.11? */
			     HIBYTE(g_pNavigatorFuncs->version)==0 &&
		         LOBYTE(g_pNavigatorFuncs->version)<=9) {
			char *tmpc;
			HDC  *tmph;

			tmpc= (char*)&(printInfo->print.embedPrint);
			tmph= (HDC*)&tmpc[28];
			pdc=  *tmph;
		}
		else {
			pdc= (HDC) (printInfo->print.embedPrint.platformPrint);
		}

		if(!This->lpdib) return;

		prevstretchmode=SetStretchBltMode(pdc,COLORONCOLOR);
		StretchDIBits(pdc,
			printWindow->x,printWindow->y,
			printWindow->width,printWindow->height, /* dest coords */
			0,0,This->lpdibinfo->biWidth, This->lpdibinfo->biHeight,   /* source coords */
			This->lpdibbits, (LPBITMAPINFO)This->lpdib,
			DIB_RGB_COLORS,SRCCOPY);
		if(prevstretchmode) SetStretchBltMode(pdc,prevstretchmode);
	}
	return;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * NPP_URLNotify:
 * Notifies the instance of the completion of a URL request. 
 +++++++++++++++++++++++++++++++++++++++++++++++++*/
static void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{
	return;
}


/**********************************************************************/

/* Try to make a filename from the url. Caller must provide fn[MAX_PATH] buffer.
 * This function attempts to extract a bitmap filename from a URL,
 * but if it doesn't look like it contains an appropriate name,
 * it leaves it blank. */
static void url2filename(char *fn, char *url)
{
	int title,ext,i;

	lstrcpy(fn,"");
	ext=0;   /* position of the file extention */
	title=0; /* position of the base filename */
	for(i=0;url[i];i++) {
		if(url[i]=='.') ext=i+1;
		if(url[i]=='/') title=i+1;
		if(url[i]=='\\') title=i+1;  // handle Microsoft's bogus file: "URLs"
		if(url[i]==':') title=i+1;
		if(url[i]=='=') title=i+1;
	}

	if (!_stricmp(&url[ext],"mng") ||
		!_stricmp(&url[ext],"jng") ||
		!_stricmp(&url[ext],"png") )
	{
		lstrcpyn(fn,&url[title],MAX_PATH);
	}
}

// sanitize string and escape '&'s for use in a menu
static void escapeformenu(unsigned char *s1)
{
	int f, t, len;
	unsigned char s2[200];

	t=0;
	len=lstrlen(s1); if(len>50) len=50;
	for(f=0;f<len;f++) {
		if(s1[f]=='&') {
			s2[t++]='&';
			s2[t++]='&';
		}
		else if(s1[f]<32) {
			s2[t++]='_';
		}
		else {
			s2[t++]=s1[f];
		}
	}
	s2[t]='\0';
	lstrcpy(s1,s2);
}


static char *get_imagetype_name(mng_imgtype t)
{
	switch(t) {
	case mng_it_mng: return "MNG";
	case mng_it_png: return "PNG";
	case mng_it_jng: return "JNG";
	}
	return "Unknown";
}


/* Write the image to a local file  */
static void SaveImage(PluginInstance *This)
{
	OPENFILENAME ofn;
	char fn[MAX_PATH];
	HANDLE hfile;
	BOOL b;
	mng_imgtype t;
	DWORD byteswritten;

	if(!This->mng || This->loadstate<STATE_LOADED ||
		This->bytesloaded != This->filesize)
	{
		warn(This,"Image not loaded -- can't save");
		return;
	}

	if(lstrlen(This->url)) {
		url2filename(fn,This->url);
	}
	else {
		lstrcpy(fn,"");
	}

	ZeroMemory(&ofn,sizeof(OPENFILENAME));
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=This->fhWnd;
	ofn.nFilterIndex=1;
	ofn.lpstrTitle="Save Image As...";
	ofn.lpstrFile=fn;
	ofn.nMaxFile=MAX_PATH;
	ofn.Flags=OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;

	t=mng_get_sigtype(This->mng);
	if(t==mng_it_png) {
		ofn.lpstrDefExt="png";  // FIXME also give an option of MNG
		ofn.lpstrFilter="PNG (*.png)\0*.png\0\0";
	}
	else if(t==mng_it_jng) {
		ofn.lpstrDefExt="jng";  // FIXME also give an option of MNG
		ofn.lpstrFilter="JNG (*.jng)\0*.jng\0\0";
	}
	else {
		ofn.lpstrFilter="MNG (*.mng)\0*.mng\0\0";
		ofn.lpstrDefExt="mng";
	}

	if(GetSaveFileName(&ofn)) {
		// save to filename: ofn.lpstrFile
		hfile=CreateFile(ofn.lpstrFile,GENERIC_WRITE,FILE_SHARE_READ,
			NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hfile==INVALID_HANDLE_VALUE) {
			warn(This,"Unable to write file");
		}
		else {
			b=WriteFile(hfile, This->mngdata, This->filesize,
				&byteswritten,NULL);
			if(!b || byteswritten != This->filesize) {
				warn(This,"Error writing file");
			}
			CloseHandle(hfile);
		}
	}
}


static void CopyToClipboard(PluginInstance *This,unsigned char *mem,int size,UINT format)
{
	HGLOBAL hClip;
	LPVOID lpClip;

	if(!mem) return;

	if(!OpenClipboard(NULL)) {
		warn(This,"Can't open the clipboard");
		return;
	}

	if(EmptyClipboard()) {
		hClip=GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE,size);
		lpClip=GlobalLock(hClip);
		if(lpClip) {
			CopyMemory(lpClip,mem,size);
			GlobalUnlock(hClip);
			if(!SetClipboardData(format,hClip)) {
				warn(This,"Can't set clipboard data");
			}
		}
		else {
			warn(This,"Can't allocate memory for clipboard");
		}
	}
	else {
		warn(This,"Can't clear the clipboard");
	}
	CloseClipboard();
}

static void AboutDialog(PluginInstance *This)
{
	DialogBoxParam(g_hInst,"ABOUTDLG",This->fhWnd,(DLGPROC)DlgProcAbout,(LPARAM)This);
}

static void PropDialog(PluginInstance *This)
{
	//if(This->textdata)
	DialogBoxParam(g_hInst,"PROPDLG",This->fhWnd,(DLGPROC)DlgProcProp,(LPARAM)This);
}

static void display_last_error(PluginInstance *This)
{
	if(This->errorflag) {
		warn(This,"%s",This->errormsg);
	}
}

static void DynamicMNG_FireEvent(PluginInstance *This, mng_uint8 eventtype, POINTS pos)
{
	mng_retcode r;
	if(!This->mng) return;
	if(This->dynamicmng ==  0) return;
	if(This->dynamicmng == -1) {
		r=mng_status_dynamic(This->mng);
		if(r==MNG_FALSE) {
			return;
		}
		else {
			This->dynamicmng=1;
		}
	}
	mng_trapevent(This->mng, eventtype, pos.x+This->xscrollpos, pos.y+This->yscrollpos);
}


static void ContextMenu(PluginInstance *This, HWND hwnd)
{
	int cmd;
	HMENU menu;
	POINT pt;
	unsigned char buf[MAX_PATH], buf2[200];

	pt.x=0; pt.y=0;
	GetCursorPos(&pt);

	// create context menu dynamically
	menu=CreatePopupMenu();
	if(This->errorflag) {
		AppendMenu(menu,MF_ENABLED,ID_SHOWERROR,"SHOW ERROR MESSAGE");
		AppendMenu(menu,MF_SEPARATOR,0,NULL);
	}

	AppendMenu(menu,(This->loadstate>=STATE_LOADED?MF_ENABLED:MF_GRAYED),ID_SAVEAS,"Save Image &As...");
	AppendMenu(menu,(This->lpdib?MF_ENABLED:MF_GRAYED),ID_COPYIMAGE,"&Copy Image");
	AppendMenu(menu,MF_ENABLED,ID_COPYURL,"Cop&y Image Location");
	if(This->islink) {
		AppendMenu(menu,MF_ENABLED,ID_COPYLINKLOC,"Copy Link Location");
	}

	url2filename(buf,This->url);
	escapeformenu(buf);
	if(lstrlen(buf)) {
		wsprintf(buf2,"View Image (%s)",buf);
	}
	else {
		wsprintf(buf2,"View Image");
	}
	AppendMenu(menu,MF_ENABLED,ID_VIEWIMAGE,buf2);


	AppendMenu(menu,MF_SEPARATOR,0,NULL);
	// AppendMenu(menu,(This->mng?MF_ENABLED:MF_GRAYED),ID_STOPANIM,"Stop Animation");


	AppendMenu(menu,(This->mng?MF_ENABLED:MF_GRAYED)|
		(This->frozen?MF_CHECKED:MF_UNCHECKED),ID_FREEZE,"&Freeze Animation");

	// AppendMenu(menu,(This->mng?MF_ENABLED:MF_GRAYED),ID_RESTARTANIM,"Restart Animation");

	AppendMenu(menu,MF_SEPARATOR,0,NULL);

	AppendMenu(menu,MF_ENABLED,ID_PROPERTIES,"Properties...");

	AppendMenu(menu,MF_ENABLED,ID_ABOUT,"About MNG Plug-in...");

	cmd=TrackPopupMenuEx(menu, TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY|TPM_RETURNCMD|
		TPM_RIGHTBUTTON,pt.x,pt.y,hwnd,NULL);

	DestroyMenu(menu);

	switch(cmd) {

	case ID_STOPANIM:
		if(This->mng) {
			KillTimer(This->fhWnd,1);
			This->timer_set=0;
			mng_display_freeze(This->mng);
		}
		break;

	case ID_FREEZE:
		This->frozen = !This->frozen;
		if(This->frozen) {
			KillTimer(This->fhWnd,1);
			This->timer_set=0;
			mng_display_freeze(This->mng);
		}
		else {
			handle_read_error(This, mng_display_resume(This->mng) );

		}
		break;

	case ID_RESTARTANIM:
		if(!This->frozen) {
			KillTimer(This->fhWnd,1);
			This->timer_set=0;
			mng_display_freeze(This->mng);
		}
		This->frozen=1;
		mng_display_reset(This->mng);
		This->frozen=0;
		handle_read_error(This, mng_display_resume(This->mng) );
		break;

	case ID_SAVEAS:
		SaveImage(This);
		break;
	case ID_COPYIMAGE:
		if(This->lpdib) {
			CopyToClipboard(This,(unsigned char*)This->lpdib,This->dibsize,CF_DIB);
		}
		else {
			warn(This,"No image to copy");
		}
		break;
	case ID_COPYURL:
		CopyToClipboard(This,This->url,lstrlen(This->url)+1,CF_TEXT);
		break;
	case ID_COPYLINKLOC:
		if(This->islink) {
			CopyToClipboard(This,This->linkurl,lstrlen(This->linkurl)+1,CF_TEXT);
		}
		break;
	case ID_VIEWIMAGE:
		if(lstrlen(This->url)) 
			NPN_GetURL(This->instance,This->url,"_self");
		break;
	case ID_PROPERTIES:
		PropDialog(This);
		break;
	case ID_ABOUT:
		AboutDialog(This);
		break;
	case ID_SHOWERROR:
		display_last_error(This);
		break;
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * PluginWindowProc
 * Handle the Windows window-event loop.
 +++++++++++++++++++++++++++++++++++++++++++++++++*/
static LRESULT CALLBACK PluginWindowProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	PluginInstance* This;
	HDC hdc;
	RECT rect;
		
	This = (PluginInstance*) GetProp(hWnd, gInstanceLookupString);

	if(!This) return DefWindowProc( hWnd, Msg, wParam, lParam);

	switch(Msg) {

	case WM_ERASEBKGND:
		{
			HBRUSH br;
			hdc= (HDC)wParam;

			if(This->bkgdbrush)
				br=This->bkgdbrush;
			else
				br=GetStockObject(GRAY_BRUSH);

			GetClientRect(hWnd,&rect);
			FillRect(hdc,&rect,br);
			return 1;
		}

	case WM_HSCROLL:
	case WM_VSCROLL:
		scrollmsg(This,Msg,(int)(LOWORD(wParam)),(short int)(HIWORD(wParam)));
		return 0;

	case WM_SIZE:
		find_window_size(This);
		set_scrollbars(This);
		return 0;


	case WM_CONTEXTMENU: case WM_RBUTTONUP:
		ContextMenu(This, hWnd);
		return 0;


	case WM_SETCURSOR:
		if(LOWORD(lParam)==HTCLIENT) {
			if(This->islink) {
				SetCursor(This->linkcursor);
				return 1;
			}
		}
		break;

	case WM_LBUTTONDOWN:
		SetCapture(This->fhWnd);
		This->mouse_captured=1;

		if(This->dynamicmng && This->mng && !This->errorflag) {
			DynamicMNG_FireEvent(This,4,MAKEPOINTS(lParam));
		}
		return 0;
		
	case WM_LBUTTONUP:
		{
			RECT rc;
			POINT pt;

			if(This->mouse_captured) {
				ReleaseCapture();
				This->mouse_captured=0;
			}
			if(This->dynamicmng && This->mng && !This->errorflag) {
				DynamicMNG_FireEvent(This,5,MAKEPOINTS(lParam));
			}

			// if mouse is not over image, don't follow links, etc.
			GetWindowRect(This->fhWnd,&rc);
			GetCursorPos(&pt);
			if(!PtInRect(&rc,pt)) return 0;

			if(This->islink) {
				NPN_GetURL(This->instance,This->linkurl,This->linktarget);
				return 0;
			}
			else if(This->errorflag) {
				display_last_error(This);
			}
		}
		return 0;

	case WM_MOUSEMOVE:

		if(This->dynamicmng && This->mng && This->lpdib && !This->errorflag) {
			POINTS pos;
			int overimage;

			pos=MAKEPOINTS(lParam);
			overimage=0;
			if(pos.x>=0 && pos.x<This->lpdibinfo->biWidth && pos.y>=0 && pos.y<This->lpdibinfo->biHeight) {
				overimage=1;
			}

			if(overimage) {
				if(This->mouse_over_mng) {
					// mouse is still over image: mouse move event
					DynamicMNG_FireEvent(This,2,pos);  // 2=mouse move
				}
				else {
					// mouse wasn't over the image but now it is: mouse-enter event
					DynamicMNG_FireEvent(This,1,pos); // mouse enter
				}
			}
			else { // mouse not now over image
				if(This->mouse_over_mng) { // ... but it used to be
					pos.x=0; pos.y=0;
					DynamicMNG_FireEvent(This,3,pos); // 3=mouse leave
				}
			}

			This->mouse_over_mng=overimage; // remember for next time

			if(This->mouse_over_mng && (This->dynamicmng==1) ) {
#define MOUSE_POLL_INTERVAL  100    // milliseconds
				SetTimer(This->fhWnd,2,MOUSE_POLL_INTERVAL,NULL);
				This->timer2_set=0;
			}
		}
		return 0;

	case WM_PAINT:
		{
			PAINTSTRUCT paintStruct;
			HDC hdc;
			RECT rect2;

			hdc = BeginPaint( hWnd, &paintStruct );
			SetWindowOrgEx(hdc,This->xscrollpos,This->yscrollpos,NULL);

			GetClientRect(hWnd,&rect);
			if(This) {
				if(This->errorflag || !This->lpdib) {
					SelectObject(hdc,hfontMsg);
					Rectangle(hdc,rect.left,rect.top,rect.right,rect.bottom);
					rect2.left=rect.left+2;
					rect2.top=rect.top+2;
					rect2.right=rect.right-2;
					rect2.bottom=rect.bottom-2;
					if(This->errorflag) {
						DrawText(hdc,"MNG PLUG-IN ERROR!",-1,&rect2,DT_LEFT|DT_WORDBREAK);
					}
					else {
						if(This->loadstate>=STATE_LOADING) {
							DrawText(hdc,"MNG image loading...",-1,&rect2,DT_LEFT|DT_WORDBREAK);
						}
						else {
							DrawText(hdc,"MNG plug-in",-1,&rect2,DT_LEFT|DT_WORDBREAK);
						}
					}
				}
				else if(This->lpdib) {
					StretchDIBits(hdc,
						0,0,This->lpdibinfo->biWidth,This->lpdibinfo->biHeight,
						0,0,This->lpdibinfo->biWidth,This->lpdibinfo->biHeight,
						&((BYTE*)(This->lpdib))[sizeof(BITMAPINFOHEADER)],
						(LPBITMAPINFO)This->lpdib,DIB_RGB_COLORS,SRCCOPY);
				}
			}


			EndPaint( hWnd, &paintStruct );
		}
		return 0;

	case WM_TIMER:
		switch(wParam) {
		case 1: // the main animation timer
			KillTimer(hWnd,1);
			This->timer_set=0;

#ifdef MNGPLG_TRACE
			fprintf(tracefile,"WM_TIMER display_resume bytesloaded=%d\n",This->bytesloaded);
#endif

			if(This->mng) {
				if(!This->needresume) {
					handle_read_error(This, mng_display_resume(This->mng) );
				}
			}
			return 0;

		case 2: // timer for polling mouse position
			{
				RECT rc;
				POINT pt;
				POINTS pos;

				GetWindowRect(hWnd,&rc);
				GetCursorPos(&pt);
				if(!PtInRect(&rc,pt)) {
					KillTimer(hWnd,2);
					pos.x=0; pos.y=0;
					DynamicMNG_FireEvent(This,3,pos); // 3=mouse leave
					This->mouse_over_mng=0;
				}
			}
			return 0;
		}
		break;

	}

	/* Forward unprocessed messages on to their original destination
	 * (the window proc we replaced) */
	return This->fDefaultWindowProc(hWnd, Msg, wParam, lParam);
}

static LRESULT CALLBACK DlgProcProp(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	char buf[4096],buf2[1024];

	switch(Msg) {
	case WM_INITDIALOG:
		{
			DWORD tabs[1];

			PluginInstance *This=(PluginInstance*)lParam;

			tabs[0]= 60;
			SendDlgItemMessage(hWnd,IDC_IMGINFO,EM_SETTABSTOPS,(WPARAM)1,(LPARAM)tabs);

			wsprintf(buf,"URL:\t%s\r\n",This->url);

			if(This->lpdib) {
				wsprintf(buf2,"Dimensions:\t%d x %d\r\n",This->lpdibinfo->biWidth,
					This->lpdibinfo->biHeight);
				lstrcat(buf,buf2);
			}

			if(This->lpdib && This->fMode==NP_EMBED) {
				wsprintf(buf2,"Window:\t%d x %d\r\n",This->windowwidth,
					This->windowheight);
				lstrcat(buf,buf2);
			}

			if(This->filesize) {
				wsprintf(buf2,"File size:\t%u bytes\r\n",This->filesize);
				lstrcat(buf,buf2);
			}

#ifdef _DEBUG
			if(This->mngdata && This->lpdib && This->bytesalloc && This->dibsize) {
				// note this doesn't include memory used by libmng
				wsprintf(buf2,"Memory used:\t%u bytes\r\n",
					This->bytesalloc + This->dibsize);
				lstrcat(buf,buf2);
			}
#endif

			if(This->islink) {
				wsprintf(buf2,"Link to:\t%s\r\n",This->linkurl);
				lstrcat(buf,buf2);
				if(strcmp(This->linktarget,"_self")) {
					wsprintf(buf2,"Link target:\t%s\r\n",This->linktarget);
					lstrcat(buf,buf2);
				}
			}

			if(This->loadstate >= STATE_VALIDFRAME) {
				wsprintf(buf2,"Signature:\t%s\r\n",get_imagetype_name(mng_get_sigtype(This->mng)));
				lstrcat(buf,buf2);
				wsprintf(buf2,"Image type:\t%s\r\n",get_imagetype_name(mng_get_imagetype(This->mng)));
				lstrcat(buf,buf2);
				wsprintf(buf2,"Simplicity:\t0x%08x\r\n",mng_get_simplicity(This->mng));
				lstrcat(buf,buf2);
				wsprintf(buf2,"Frame count:\t%u\r\n",mng_get_framecount(This->mng));
				lstrcat(buf,buf2);
				wsprintf(buf2,"Layer count:\t%u\r\n",mng_get_layercount(This->mng));
				lstrcat(buf,buf2);
				wsprintf(buf2,"Play time:\t%u\r\n",mng_get_playtime(This->mng));
				lstrcat(buf,buf2);
			}

			SetDlgItemText(hWnd,IDC_IMGINFO,buf);

			if(This->textdata)
				SetDlgItemText(hWnd,IDC_MNGTEXT,This->textdata);
		}
		return(TRUE);
	case WM_CLOSE:
		EndDialog(hWnd,0);
		return(TRUE);
	case WM_COMMAND:
		switch(wParam) {
		case IDOK:
		case IDCANCEL:
			EndDialog(hWnd,0);
			return(TRUE);
		}
	}
	return(FALSE);
}

static LRESULT CALLBACK DlgProcAbout(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	char buf[4096],buf2[1024],buf3[300];

	switch(Msg) {
	case WM_INITDIALOG:
		{
			//DWORD tabs[1];

			PluginInstance *This=(PluginInstance*)lParam;

			//tabs[0]= 60;
			//SendDlgItemMessage(hWnd,IDC_IMGINFO,EM_SETTABSTOPS,(WPARAM)1,(LPARAM)tabs);

			wsprintf(buf,"MNGPLG Plug-in, Version %s\r\n%s"
#ifdef _DEBUG
				" DEBUG BUILD"
#endif
				"\r\nCopyright (C) 2000-2002 by Jason Summers\r\n\r\n",MNGPLGVERS,__DATE__);

			wsprintf(buf2,"Based on libmng by Gerard Juyn.\r\n");
			lstrcat(buf,buf2);

			wsprintf(buf2,"libmng version: %s\r\n\r\n",mng_version_text());
			lstrcat(buf,buf2);

			wsprintf(buf2,"Uses the zlib compression library.\r\n");
			lstrcat(buf,buf2);
			wsprintf(buf2,"zlib version: %s\r\n\r\n",zlibVersion());
			lstrcat(buf,buf2);

			wsprintf(buf2,"This software is based in part on the work of the "
				"Independent JPEG Group.\r\n");
			lstrcat(buf,buf2);
			// This really only gives the version of the libjpeg header used when
			// compiling this plugin, but I don't know how to query libjpeg for its
			// version.
			wsprintf(buf2,"IJG JPEG library version: %s\r\n%s\r\n\r\n",JVERSION,JCOPYRIGHT);
			lstrcat(buf,buf2);

#ifdef MNGPLG_CMS
			wsprintf(buf2,"Uses the lcms color management library by Martí Maria. "
				"lcms is distributed under the terms of the GNU LESSER GENERAL PUBLIC LICENSE. "
				"See the file COPYING-LCMS.\r\n\r\n");
			lstrcat(buf,buf2);
#endif

			if(GetModuleFileName(g_hInst,buf3,260)) {
				wsprintf(buf2,"MNGPLG location: %s\r\n",buf3);
				lstrcat(buf,buf2);
			}

			SetDlgItemText(hWnd,IDC_PRGINFO,buf);

		}
		return(TRUE);
	case WM_CLOSE:
		EndDialog(hWnd,0);
		return(TRUE);
	case WM_COMMAND:
		switch(wParam) {
		case IDOK:
		case IDCANCEL:
			EndDialog(hWnd,0);
			return(TRUE);
		}
	}
	return(FALSE);
}


/////////////////////
/////////////////////  low-level plug-in NPAPI functions

static JRIGlobalRef Private_GetJavaClass(void)
{
    return NULL;
}

NPError WINAPI NP_GetEntryPoints(NPPluginFuncs* pFuncs)
{
    if(!pFuncs) return NPERR_INVALID_FUNCTABLE_ERROR;

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
    pFuncs->event         = NULL;

	g_pluginFuncs		  = pFuncs;

    return NPERR_NO_ERROR;
}

NPError WINAPI NP_Initialize(NPNetscapeFuncs* pFuncs)
{
	int navMinorVers;

    if(!pFuncs) return NPERR_INVALID_FUNCTABLE_ERROR;

    g_pNavigatorFuncs = pFuncs; // save it for future reference 

    if(HIBYTE(pFuncs->version) > NP_VERSION_MAJOR)
        return NPERR_INCOMPATIBLE_VERSION_ERROR;

    navMinorVers = g_pNavigatorFuncs->version & 0xFF;
	if(navMinorVers>=NPVERS_HAS_NOTIFICATION)
		g_pluginFuncs->urlnotify = NPP_URLNotify;
	if( navMinorVers>=NPVERS_HAS_LIVECONNECT)
		g_pluginFuncs->javaClass = Private_GetJavaClass();

    return NPP_Initialize();
}

NPError WINAPI NP_Shutdown()
{
    NPP_Shutdown();

    g_pNavigatorFuncs = NULL;
    return NPERR_NO_ERROR;
}
