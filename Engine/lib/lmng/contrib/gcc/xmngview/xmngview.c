/* Built with libmng-1.0.9
 * Compiled on linux with gcc-3.3.4
 * james@blastwave.org suggested the single step mode and wrote:
 * "xmngview works on Solaris both Sparc and Intel and compiles with Sun's cc"
 *
 * <szukw000@students.uni-mainz.de> 
 *    This program my be redistributed under the terms of the
 *    GNU General Public Licence, version 2, or at your preference,
 *    any later version.
 *           
 * For more information about libmng please visit:
 * 
 * The official libmng web-site:
 *   http://www.libmng.com
 * 
 * Libmng on SourceForge:
 *   http://libmng.sourceforge.net
 * 
 * The official MNG homepage:
 *   http://www.libpng.org/pub/mng
 * 
 * The official PNG homepage:
 *   http://www.libpng.org/pub/png
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <ctype.h>
#include <libmng.h>

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <X11/Xutil.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/FileSB.h>
#include <Xm/Label.h>
#include <X11/extensions/XShm.h>

#include "xmng.h"

#define DEFAULT_BACKGROUND "grey77"
static char version[]={"0.6"};

static void run_viewer(FILE *reader, char *read_idf);

static mng_handle user_handle;
static ImageInfo img;
static struct timeval start_tv, now_tv;
static XtIntervalId timeout_ID;
static char *prg_idf;

static XtAppContext app_context;
static  Widget toplevel, main_form, canvas, file_label;
static XmFontList file_font;
static Dimension start_width;

#define SLASH '/'
/*
 * Cnf: XQueryColor(3X11)
*/
static char *parse_rgb_color(char *val)
{
    char *s, *d;
    int ch;
    char status, rgb_type;
    char r[6], g[6], b[6], rgb[24];

    rgb_type = 0;
    status = 1;
    s = val;
    memset(r, 0, 6);
    memset(g, 0, 6);
    memset(b, 0, 6);

    if(strncasecmp(s, "rgb:", 4) == 0)
   {
    rgb_type = 1;
    s += 4;
    if((d = strchr(s, SLASH)))
  {
    *d = 0;

    if(d - s > 4)
      s[4] = 0;
    strcpy(r, s);

    s = ++d;

    if((d = strchr(s, SLASH)))
 {
    *d = 0;
    if(d - s > 4)
      s[4] = 0;
    strcpy(g, s);

    s = d + 1;
    while((ch = *++d) && isxdigit(ch));
    *d = 0;
    if(d - s > 4)
      s[4] = 0;
    strcpy(b, s);

 }
    if(*r == 0 || *g == 0 || *b == 0)
      return NULL;

    s = r - 1;
    while((ch = *++s))
 {
    if(isxdigit(ch)) continue;
    status = rgb_type = 0;
    break;
 }
    s = g - 1;
    while((ch = *++s))
 {
    if(isxdigit(ch)) continue;
    status = rgb_type = 0;
    break;
 }
    s = b - 1;
    while((ch = *++s))
 {
    if(isxdigit(ch)) continue;
    status = rgb_type = 0;
    break;
 }
    if(status)
 {
    strcpy(rgb, "rgb:");
    d = rgb + 4;
    s = r;
    while(*s) *d++ = *s++;
    *d++ = SLASH;
    s = g;
    while(*s) *d++ = *s++;
    *d++ = SLASH;
    s = b;
    while(*s) *d++ = *s++;
    *d = 0;

    return strdup(rgb);
 }
  } /* if((slash = strchr(s, SLASH))) */
    return NULL;
   }

    s = val;
    if(*s == '#' || isdigit(*s))
   {
    if(*s != '#')
      --s;
    while((ch = *++s))
  {
    if(isxdigit(ch)) continue;
    status = 0;
    break;
  }
    if(status)
  {
    d = rgb;
    s = val;
    if(*s == '#')
      ++s;
/*
 * #RGB                (4 bits each)
 * #RRGGBB             (8 bits each)
 * #RRRGGGBBB          (12 bits each)
 * #RRRRGGGGBBBB       (16 bits each)
*/
    if(strlen(s) > 12)
      s[12] = 0;
    *d++ = '#';
    strcpy(d, s);
    return strdup(rgb);
  }
    return NULL;
   }

/*
 * 'white', 'LavenderBlush', 'dark slate gray', 'grey12'
*/
    s = val - 1;
    while((ch = *++s))
   {
    if(isalnum(ch) || isspace(ch)) continue;
    status = 0;
    break;
   }
    if(!status)
      return NULL;
    return strdup(val);

}/* parse_rgb_color() */

static void set_bg_pixel(ImageInfo *img)
{
    XColor xcolor;
    Widget w;
    char *s, *d;
    int found;

    w = img->canvas;

    if(!img->has_bg_pixel)
   {
    if(img->has_bg_color)
  {
    s = strdup(img->bg_color);

    d = parse_rgb_color(s);

    free(s);

    if(d)
 {
    strcpy(img->bg_color, d);
    free(d);
 }
    else
      img->has_bg_color = 0;
  }
    if(!img->has_bg_color)
  {
    strcpy(img->bg_color, DEFAULT_BACKGROUND);
    img->has_bg_color = 1;
  }

    found = XParseColor(img->dpy,
      DefaultColormap(img->dpy, DefaultScreen(img->dpy)),
      img->bg_color, &xcolor);

    if(!found)
  {
    strcpy(img->bg_color, DEFAULT_BACKGROUND);

    found = XParseColor(img->dpy,
      DefaultColormap(img->dpy, DefaultScreen(img->dpy)),
      img->bg_color, &xcolor);

  }
    xcolor.flags = DoRed | DoGreen | DoBlue;

    XAllocColor(img->dpy,
      DefaultColormap(img->dpy, DefaultScreen(img->dpy)),
      &xcolor);
   }
    else
   {
    xcolor.pixel = img->bg_pixel;
    xcolor.flags = DoRed|DoGreen|DoBlue;

    found = XQueryColor(img->dpy,
      DefaultColormap(img->dpy, DefaultScreen(img->dpy)),
      &xcolor);
   }
    img->bg_pixel = xcolor.pixel;
    img->xbg_red = xcolor.red;
    img->xbg_green = xcolor.green;
    img->xbg_blue = xcolor.blue;
    img->bg_red = (unsigned char)xcolor.red&0xff;
    img->bg_green = (unsigned char)xcolor.green&0xff;
    img->bg_blue = (unsigned char)xcolor.blue&0xff;
    img->has_bg_pixel = 1;

}/* set_bg_pixel() */

static void fsb_cancel_cb(Widget w, XtPointer client, XtPointer call)
{
    XtUnmanageChild(w);
}

void create_file_dialog(Widget w, char *button_text, char *title_text,
    void(*fsb_select_cb)(Widget,XtPointer,XtPointer))
{
    Arg args[4];
    int cnt;
    Widget dialog;
    XmString button_str, title_str, filter;
    Widget child;

    cnt = 0;
    dialog = XmCreateFileSelectionDialog(w, "Files", args, cnt);

	XtUnmanageChild(XmFileSelectionBoxGetChild(dialog,XmDIALOG_HELP_BUTTON));
    XtAddCallback(dialog, XmNcancelCallback, fsb_cancel_cb, NULL);
    XtAddCallback(dialog, XmNokCallback, fsb_select_cb, NULL);
    button_str = XmStringCreateLocalized(button_text);
    title_str = XmStringCreateLocalized(title_text);
	filter = XmStringCreateLocalized("*.[jmp]ng");
    XtVaSetValues(dialog,
        XmNokLabelString, button_str,
        XmNdialogTitle,   title_str,
		XmNpattern, filter,
		XmNfileFilterStyle, XmFILTER_NONE,
        NULL);
    XmStringFree(button_str);
    XmStringFree(title_str);
	XmStringFree(filter);
    child = XmFileSelectionBoxGetChild(dialog, XmDIALOG_FILTER_TEXT);
    XtVaSetValues(child, XmNfontList, file_font, NULL);
    child = XmFileSelectionBoxGetChild(dialog, XmDIALOG_DIR_LIST);
    XtVaSetValues(child, XmNfontList, file_font, NULL);
    child = XmFileSelectionBoxGetChild(dialog, XmDIALOG_LIST);
    XtVaSetValues(child, XmNfontList, file_font, NULL);
    child = XmFileSelectionBoxGetChild(dialog, XmDIALOG_TEXT);
    XtVaSetValues(child, XmNfontList, file_font, NULL);

    XtManageChild(dialog);
    XMapRaised(XtDisplay (dialog), XtWindow (XtParent (dialog)));
}

void run_mng_file_cb(Widget w, XtPointer client, XtPointer call)
{
    XmFileSelectionBoxCallbackStruct *fsb;
    char *read_idf;
	FILE *reader;

    XtUnmanageChild(w);
    fsb = (XmFileSelectionBoxCallbackStruct *)call;
    XmStringGetLtoR(fsb->value, XmSTRING_DEFAULT_CHARSET, &read_idf);

    if(read_idf == NULL || *read_idf == 0) return;

	reader = fopen(read_idf, "r");
    if(reader == NULL)
   {
    perror(read_idf);
    fprintf(stderr, "\n\n%s: cannot open file '%s'\n\n", prg_idf, read_idf);
    return;
   }
	run_viewer(reader, read_idf);

    free(read_idf);
}

static void user_reset_data(void)
{
	if(timeout_ID) XtRemoveTimeOut(timeout_ID);
	timeout_ID = 0;
	mng_cleanup(&img.user_handle);

	img.read_pos = 0;
	free(img.read_buf);
	img.read_buf = NULL;
	img.read_len = 0;
	img.img_width = 0;
	img.img_height = 0;
	img.mng_bytes_per_line = 0;
	img.read_idf = NULL;
	img.frozen = 0;
	img.restarted = 0;
    img.single_step_wanted = 0;
    img.single_step_served = 0;

	XClearWindow(img.dpy, img.win);
}

void browse_file_cb(Widget w, XtPointer client, XtPointer call)
{
	if(img.user_handle)
	  user_reset_data();

	img.stopped = 0;
	img.frozen = 0;
	img.restarted = 0;
    create_file_dialog(w, "Select", "Select MNG file", run_mng_file_cb);
}

void Viewer_postlude(void)
{
	if(timeout_ID) XtRemoveTimeOut(timeout_ID);
	mng_cleanup(&img.user_handle);
	if(img.reader) fclose(img.reader);
	if(img.ximage) XDestroyImage(img.ximage);
	if(img.read_buf) free(img.read_buf);
	if(img.mng_buf) free(img.mng_buf);
	if(img.dither_line) free(img.dither_line);
	if(!img.external_win && img.dpy) XtCloseDisplay(img.dpy);
	fputc('\n', stderr);
}

static void user_init_data(ImageInfo *img)
{
	unsigned int depth;
	int screen;
	Display *dpy;

	dpy = img->dpy;
    screen = DefaultScreen(dpy);
    depth = DefaultDepth(dpy, screen);
	img->depth = depth;

	if(!img->visual)
   {
	img->visual = DefaultVisual(dpy, screen);
	img->gc = DefaultGC(dpy, DefaultScreen(dpy));
   }
	else
   {
	if(img->mng_buf) free(img->mng_buf);
	if(img->dither_line) free(img->dither_line);

	x11_destroy_ximage(img);
   }

	set_bg_pixel(img);

	mng_set_bgcolor(img->user_handle, 
	  img->xbg_red, img->xbg_green, img->xbg_blue);

	img->mng_bytes_per_line = img->img_width * img->mng_rgb_size;
	img->mng_buf = (unsigned char*)
	  calloc(1, img->mng_bytes_per_line * img->img_height);
	img->dither_line = (unsigned char*)
	  calloc(1, img->mng_bytes_per_line);

	if(!img->x11_init)
   {
	x11_init_color(img);

	img->x11_init = 1;
   }
	img->ximage = x11_create_ximage(img);
	
	if(img->ximage == NULL)
   {
	Viewer_postlude();
	exit(0);
   }
}

static void player_exit_cb(Widget w, XtPointer client, XtPointer call)
{
    Viewer_postlude();
    exit(0);
}

static void player_stop_cb(Widget w, XtPointer client, XtPointer call)
{
    if(img.type != MNG_TYPE) return;
    if(!img.user_handle) return;
    if(img.stopped) return;

    user_reset_data();
    img.stopped = 1;
}

static void player_single_step_cb(Widget w, XtPointer client, XtPointer call)
{
    if(img.type != MNG_TYPE) return;
    if(!img.user_handle) return;
    if(img.stopped) return;

    if(img.single_step_served)
   {
    img.single_step_served = 0;
    img.frozen = 0;

    img.single_step_wanted = 1;
    return;
   }
	if(timeout_ID) XtRemoveTimeOut(timeout_ID);
	timeout_ID = 0;
    img.single_step_wanted = 1;
    mng_display_resume(img.user_handle);
}

static void player_pause_cb(Widget w, XtPointer client, XtPointer call)
{
    if(img.type != MNG_TYPE) return;
    if(!img.user_handle) return;
    if(img.stopped) return;
    if(img.frozen) return;

    if(timeout_ID) XtRemoveTimeOut(timeout_ID);
    timeout_ID = 0;
    img.frozen = 1;
    img.single_step_served = 0;
    img.single_step_wanted = 0;
}

static void player_resume_cb(Widget w, XtPointer client, XtPointer call)
{
    if(img.type != MNG_TYPE) return;
    if(!img.user_handle) return;
    if(img.stopped) return;

    if(!img.frozen
    && !img.single_step_served)
      return;
    img.frozen = 0;

    if(img.single_step_served
    || img.single_step_wanted)
   {
    img.single_step_served = 0;
    img.single_step_wanted = 0;

    if(timeout_ID) XtRemoveTimeOut(timeout_ID);
    timeout_ID = 0;
   }
    mng_display_resume(img.user_handle);
}

static void player_restart_cb(Widget w, XtPointer client, XtPointer call)
{
    if(img.type != MNG_TYPE) return;
    if(!img.user_handle) return;
    if(img.stopped) return;

    img.frozen = 1;
	if(timeout_ID) XtRemoveTimeOut(timeout_ID);
	timeout_ID = 0;

    img.frozen = 0;
    img.single_step_served = 0;
    img.single_step_wanted = 0;

    img.read_pos = 0;
    mng_reset(img.user_handle);
    img.restarted = 1;
	gettimeofday(&start_tv, NULL);
	mng_read(img.user_handle);
	mng_display(img.user_handle);
}

static void release_event_cb(Widget w, XtPointer client, XEvent *event,
	 Boolean *cont)
{
    Viewer_postlude();
    exit(0);
}

static void redraw(int type)
{
	if((type == Expose || type == GraphicsExpose)
	&& img.ximage)
   {
	  XPutImage(img.dpy, img.win, img.gc, img.ximage,
	    0, 0, 0, 0, img.img_width, img.img_height);
   }
}

static void exposures_cb(Widget w, XtPointer client,
    XmDrawingAreaCallbackStruct *cbs)
{

	redraw(cbs->event->xany.type);
}

static mng_ptr user_alloc(mng_size_t len)
{
    return calloc(1, len + 2);
}

static void user_free(mng_ptr buf, mng_size_t len)
{
    free(buf);
}

static mng_bool user_read(mng_handle user_handle, mng_ptr out_buf, 
	mng_uint32  req_len, mng_uint32 *out_len)
{
    mng_uint32 more;
    ImageInfo *img;

    img = (ImageInfo *)mng_get_userdata(user_handle);

	more = img->read_len - img->read_pos;

	if(more > 0
	&& img->read_buf != NULL)
   {
	if(req_len < more) 
	  more = req_len;
	memcpy(out_buf, img->read_buf + img->read_pos, more);
	img->read_pos += more;
	*out_len = more;

    return MNG_TRUE;
   }
	return MNG_FALSE;
}

static mng_bool user_open_stream(mng_handle user_handle)
{
    return MNG_TRUE;
}

static mng_bool user_close_stream(mng_handle user_handle)
{
	return MNG_TRUE;
}

static void create_widgets(mng_uint32 width, mng_uint32 height)
{
	Widget but_rc, but_frame, canvas_frame;
	Widget but1, but2, but3, but4, but5, but6, but7;

    toplevel = XtAppInitialize(&app_context, "xmngview", NULL, 0, 
	img.argc_ptr, img.argv,
      0, 0, 0);

    main_form = XtVaCreateManagedWidget("main_form",
      xmFormWidgetClass, toplevel,
	  XmNhorizontalSpacing, SPACE_X, 
	  XmNverticalSpacing, SPACE_Y,
	  XmNresizable, True,
      NULL);
	but_frame = XtVaCreateManagedWidget("but_frame",
	  xmFrameWidgetClass, main_form,
	  XmNshadowType, XmSHADOW_ETCHED_OUT,
	  XmNtopAttachment, XmATTACH_FORM,
	  XmNleftAttachment, XmATTACH_FORM,
	  XmNrightAttachment, XmATTACH_FORM,
	  XmNshadowThickness, FRAME_SHADOW_WIDTH,
	  NULL);
    but_rc = XtVaCreateManagedWidget("but_rc",
      xmRowColumnWidgetClass,  but_frame,
      XmNentryAlignment, XmALIGNMENT_CENTER,
      XmNorientation, XmHORIZONTAL,
      XmNpacking, XmPACK_COLUMN,
      XmNnumColumns, 1,
	  XmNresizeWidth, True,
      XmNentryBorder, BUT_ENTRY_BORDER,
      NULL);
    but1 = XtVaCreateManagedWidget("Exit",
      xmPushButtonWidgetClass, but_rc,
      NULL);
    XtAddCallback(but1, XmNactivateCallback,
      player_exit_cb, (XtPointer)toplevel);

    but2 = XtVaCreateManagedWidget("Pause",
      xmPushButtonWidgetClass, but_rc,
      NULL);
    XtAddCallback(but2, XmNactivateCallback,
      player_pause_cb, (XtPointer)toplevel);

    but3 = XtVaCreateManagedWidget("GoOn",
      xmPushButtonWidgetClass, but_rc,
      NULL);
    XtAddCallback(but3, XmNactivateCallback,
      player_resume_cb, NULL);

    but4 = XtVaCreateManagedWidget("Restart",
      xmPushButtonWidgetClass, but_rc,
      NULL);
    XtAddCallback(but4, XmNactivateCallback,
      player_restart_cb, NULL);

    but5 = XtVaCreateManagedWidget("Step",
      xmPushButtonWidgetClass, but_rc,
      NULL);
    XtAddCallback(but5, XmNactivateCallback,
      player_single_step_cb, NULL);

    but6 = XtVaCreateManagedWidget("Finish",
      xmPushButtonWidgetClass, but_rc,
      NULL);
    XtAddCallback(but6, XmNactivateCallback,
      player_stop_cb, NULL);

    but7 = XtVaCreateManagedWidget("Browse",
      xmPushButtonWidgetClass, but_rc,
      NULL);
    XtAddCallback(but7, XmNactivateCallback,
      browse_file_cb, NULL);

	file_label = XtVaCreateManagedWidget("FILE: ",
	  xmLabelWidgetClass, main_form,
	  XmNalignment, XmALIGNMENT_BEGINNING,
	  XmNtopAttachment, XmATTACH_WIDGET,
	  XmNtopWidget, but_frame,
	  XmNleftAttachment, XmATTACH_FORM,
	  XmNrightAttachment, XmATTACH_FORM,
	  NULL);

	canvas_frame = XtVaCreateManagedWidget("canvas_frame",
	  xmFrameWidgetClass, main_form,
	  XmNshadowType, XmSHADOW_ETCHED_OUT,
      XmNtopAttachment, XmATTACH_WIDGET,
      XmNtopWidget, file_label,
	  XmNbottomAttachment, XmATTACH_FORM,
	  XmNleftAttachment, XmATTACH_FORM,
	  XmNrightAttachment, XmATTACH_FORM,
	  NULL);

    canvas = XtVaCreateManagedWidget("canvas",
      xmDrawingAreaWidgetClass, canvas_frame,
      XmNheight, height,
      XmNwidth, width,
      NULL);

    XtAddEventHandler(canvas,
      ButtonReleaseMask|ButtonPressMask,
      False, release_event_cb, (XtPointer)toplevel);

	XtAddCallback(canvas, 
	  XmNexposeCallback, (XtCallbackProc)exposures_cb, (XtPointer)&img);
	
    XtRealizeWidget(toplevel);

	if(start_width == 0)
   {
	width = height = 0;

	start_width = (FRAME_SHADOW_WIDTH<<1);
	XtVaGetValues(but1, XmNwidth, &width, NULL);
	start_width += width + (BUT_ENTRY_BORDER<<1) + ANY_WIDTH;
	XtVaGetValues(but2, XmNwidth, &width, NULL);
	start_width += width + (BUT_ENTRY_BORDER<<1) + ANY_WIDTH;
	XtVaGetValues(but3, XmNwidth, &width, NULL);
	start_width += width + (BUT_ENTRY_BORDER<<1) + ANY_WIDTH;
	XtVaGetValues(but4, XmNwidth, &width, NULL);
	start_width += width + (BUT_ENTRY_BORDER<<1) + ANY_WIDTH;
	XtVaGetValues(but5, XmNwidth, &width, NULL);
	start_width += width + (BUT_ENTRY_BORDER<<1) + ANY_WIDTH;
	XtVaGetValues(but6, XmNwidth, &width, NULL);
	start_width += width + (BUT_ENTRY_BORDER<<1);
	XtVaGetValues(but7, XmNwidth, &width, NULL);
	start_width += width + (BUT_ENTRY_BORDER<<1);
   }
	img.canvas = canvas;
    img.dpy = XtDisplay(img.canvas);
    img.win = XtWindow(img.canvas);
    file_font = XmFontListAppendEntry(NULL,
        XmFontListEntryCreate(XmFONTLIST_DEFAULT_TAG,
        XmFONT_IS_FONT,
        XLoadQueryFont(img.dpy,
        "-*-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-1")));
}

static mng_bool user_process_header(mng_handle user_handle,
    mng_uint32 width, mng_uint32 height)
{
    ImageInfo *img;
	Dimension cw, ch, tw, th, dh, dw, fw, fh;
	XmString xmstr;
	char *s, buf[128];

    img = (ImageInfo*)mng_get_userdata(user_handle);

	if(img->restarted)
   {
	img->restarted = 0;
	return MNG_TRUE;
   }
	img->img_width = width;
	img->img_height = height;

	if(!img->external_win)
   {
	if(!img->canvas)
	  create_widgets(width, height);
	else
  {
	tw = th = fw = fh = cw = ch = 0;

	XtVaGetValues(toplevel, XmNwidth, &tw, XmNheight, &th, NULL);
	XtVaGetValues(main_form, XmNwidth, &fw, XmNheight, &fh, NULL);
	XtVaGetValues(img->canvas, XmNwidth, &cw, XmNheight, &ch, NULL);

	if(height > ch)
 {
	dh = height - ch;
	th += dh;
	fh += dh;
 }	else
	if(ch > height)
 {
	dh = ch - height;
	th -= dh;
	fh -= dh;
 }
	if(width > cw)
 {
	dw = width - cw;
	tw += dw;
	fw += dw;
 }	else
	if(cw > width)
 {
	if(width > start_width)
	  dw = cw - width;
	else
	  dw = cw - start_width;
	tw -= dw;
	fw -= dw;
 }
	if(fw < start_width)
 {
	tw = start_width + (SPACE_X<<1);
	fw = start_width;
 }
	XtVaSetValues(toplevel, XmNwidth,tw  , XmNheight,th , NULL);
	XtVaSetValues(main_form, XmNwidth,fw  , XmNheight,fh , NULL);
	XtVaSetValues(img->canvas, XmNwidth,width  , XmNheight,height , NULL);
  }
   }
	else
	if(img->external_win)
   {
	Display *dpy;

	XtToolkitInitialize();
	app_context = XtCreateApplicationContext();
	dpy = XtOpenDisplay(app_context, NULL,NULL,"xmngview",
		NULL, 0, img->argc_ptr, img->argv);
	img->dpy = dpy;
    img->win = img->external_win;
	
	XSelectInput(dpy, img->win, ExposureMask);
   }
	user_init_data(img);

	if(img->canvas)
   {
	s = strrchr(img->read_idf, '/');
	if(s == NULL) s = img->read_idf; else ++s;
	s = strdup(s);
	if(strlen(s) > 64) s[64] = 0;
	sprintf(buf, "%s (%d x %d)", s, img->img_width, img->img_height);
	xmstr = XmStringCreateLtoR((char*)buf, XmSTRING_DEFAULT_CHARSET);
	XtVaSetValues(file_label, XmNlabelString, xmstr, NULL);
	XmStringFree(xmstr);
	free(s);
   }
	gettimeofday(&start_tv, NULL);
	return MNG_TRUE;
}

static void wait_cb(XtPointer client, XtIntervalId * id)
{
	timeout_ID = 0;

	if(img.frozen
	|| img.single_step_served)
   {
//	gettimeofday(&start_tv, NULL);

	timeout_ID = XtAppAddTimeOut(app_context,
	  img.delay, wait_cb, NULL);
   }
	else
   {
	mng_display_resume(img.user_handle);
   }
}

static mng_bool user_set_timer(mng_handle user_handle, mng_uint32 delay)
{
	ImageInfo *img;

	img = (ImageInfo*)mng_get_userdata(user_handle);
	img->delay = delay;

	timeout_ID = XtAppAddTimeOut(app_context,
	  delay, wait_cb, NULL);

	return MNG_TRUE;
}

static mng_uint32 user_get_tick_count(mng_handle user_handle)
{
	double sec, usec;
	mng_uint32 ticks;

	gettimeofday(&now_tv, NULL);

	sec = (double)(now_tv.tv_sec - start_tv.tv_sec);
	usec = (double)now_tv.tv_usec - (double)start_tv.tv_usec;
	ticks = (mng_uint32)(sec * 1000.0 + usec/1000.0);
//fprintf(stderr,"TICKS %u (%f:%f)\n", ticks, sec, usec);
	return ticks;
}

static mng_ptr user_get_canvas_line(mng_handle user_handle, mng_uint32 line)
{
	ImageInfo *img;

	img = (ImageInfo*)mng_get_userdata(user_handle);

	return img->mng_buf + img->mng_bytes_per_line * line;
}

static mng_bool user_refresh(mng_handle user_handle, mng_uint32 x,
    mng_uint32 y, mng_uint32 width, mng_uint32 height)
{
    ImageInfo *img;
    mng_uint32 src_len;
    unsigned char *src_start, *src_buf;
    int row, max_row;
    Display *dpy;
    GC gc;
    Window win;
    XImage *ximage;
    Visual *visual;
	int have_shmem;

    img = (ImageInfo*)mng_get_userdata(user_handle);

	if(img->single_step_wanted)
	  img->single_step_served = 1;

    win = img->win;
    gc = img->gc;
    dpy = img->dpy;
    ximage = img->ximage;
    visual = img->visual;
	have_shmem = img->have_shmem;

    max_row = y + height;
    row = y;
    src_len = img->mng_bytes_per_line;
    src_buf = src_start = img->mng_buf + img->mng_rgb_size * x + y * src_len;

    while(row < max_row)
  {
	viewer_renderline(img, src_start, row, x, width);

    ++row;
    src_start += src_len;
  }
	XPUTIMAGE(dpy, win, gc, ximage, x, y, x, y, width, height);
	XSync(dpy, False);	
    return MNG_TRUE;
}

static mng_bool user_error(mng_handle user_handle, mng_int32 code, 
	mng_int8 severity,
    mng_chunkid chunktype, mng_uint32 chunkseq,
    mng_int32 extra1, mng_int32 extra2, mng_pchar text)
{
    ImageInfo *img;
    unsigned char chunk[5];

	img = (ImageInfo*)mng_get_userdata(user_handle);

    chunk[0] = (char)((chunktype >> 24) & 0xFF);
    chunk[1] = (char)((chunktype >> 16) & 0xFF);
    chunk[2] = (char)((chunktype >>  8) & 0xFF);
    chunk[3] = (char)((chunktype      ) & 0xFF);
    chunk[4] = '\0';

    fprintf(stderr, "\n\n%s: error playing(%s) chunk[%d]'%s':\n",
        prg_idf, img->read_idf, chunkseq, chunk);
    fprintf(stderr, "code(%d) severity(%d) extra1(%d) extra2(%d)"
      "\ntext:'%s'\n\n", code, severity, extra1, extra2, text);
    return 0;
}

static mng_bool prelude(void)
{
#define MAXBUF 8
    unsigned char buf[MAXBUF];
	
    if(fread(buf, 1, MAXBUF, img.reader) != MAXBUF)
   {
	fprintf(stderr,"\n%s:prelude\n\tcannot read signature \n",
	  prg_idf);
    return MNG_FALSE;
   }
	
	if(memcmp(buf, MNG_MAGIC, 8) == 0)
	  img.type = MNG_TYPE;
	else
	if(memcmp(buf, JNG_MAGIC, 8) == 0)
	  img.type = JNG_TYPE;
	else
	if(memcmp(buf, PNG_MAGIC, 8) == 0)
	  img.type = PNG_TYPE;
	if(!img.type)
   {
	fprintf(stderr,"\n%s:'%s' is no MNG / JNG / PNG file\n", 
	prg_idf, img.read_idf);
    return MNG_FALSE;
   }
    fseek(img.reader, 0, SEEK_SET);
    fseek(img.reader, 0, SEEK_END);
    img.read_len = ftell(img.reader);
    fseek(img.reader, 0, SEEK_SET);

	if(!img.user_handle)
   {
    user_handle = mng_initialize(&img, user_alloc, user_free, MNG_NULL);

    if(user_handle == MNG_NULL)
  {
    fprintf(stderr, "\n%s: cannot initialize libmng.\n", prg_idf);
    return MNG_FALSE;
  }
	img.user_handle = user_handle;

	mng_set_canvasstyle(user_handle, MNG_CANVAS_RGB8);
	img.mng_rgb_size = CANVAS_RGB8_SIZE;

    if(mng_setcb_openstream(user_handle, user_open_stream) != OK
    || mng_setcb_closestream(user_handle, user_close_stream) != OK
    || mng_setcb_readdata(user_handle, user_read) != OK
	|| mng_setcb_settimer(user_handle, user_set_timer) != OK
	|| mng_setcb_gettickcount(user_handle, user_get_tick_count) != OK
    || mng_setcb_processheader(user_handle, user_process_header) != OK
	|| mng_setcb_getcanvasline(user_handle, user_get_canvas_line) != OK
	|| mng_setcb_refresh(user_handle, user_refresh) != OK
	|| mng_setcb_errorproc(user_handle, user_error) != OK
      )
  {
    fprintf(stderr,"\n%s: cannot set callbacks for libmng.\n",
	  prg_idf);
    return MNG_FALSE;
  }
   }
	img.read_buf = (unsigned char*)calloc(1, img.read_len + 2);
	fread(img.read_buf, 1, img.read_len, img.reader);
	fclose(img.reader);
	img.reader = NULL;

    return MNG_TRUE;
}

static void run_viewer(FILE *reader, char *read_idf)
{
	XEvent event;

	img.read_idf = read_idf;
	img.reader = reader;

	if(read_idf != NULL)
   {
	if(prelude() == MNG_FALSE)
	  return ;

	gettimeofday(&start_tv, NULL);

	mng_read(img.user_handle);
	mng_display(img.user_handle);
   }

	if(!img.external_win)
  {
	XtAppMainLoop(app_context);
  }
	else
	while(1)
  {
	XtAppNextEvent(app_context, &event);

	redraw(event.type);
  }
}

static void usage(const char *prg)
{
	const char *bar=
"\n------------------------------------------------------------------------\n";

	fputs(bar, stderr);
	fprintf(stderr,"%s version %s\n"
	  "USAGE: %s [--w WINDOW] [--bg BACKGROUND_COLOR] [FILE]\n", 
	  prg, version, prg);
	fputs("\twith BACKGROUND_COLOR = "
	"(\"TEXT\" | \"#RGB\" | \"rgb:R/G/B\" | \"PIXEL\")\n"
	  "\te.g.\n\t(--bg \"red\" | --bg \"#ff0000\" "
	  "| --bg \"rgb:ff/00/00\" | --bg \"0xf800\")\n"
	  "\twith FILE=(idf.mng | idf.jng | idf.png)",stderr);
	fputs(bar, stderr);
}

static void shrink_name(char *buf)
{
    char *s, *d;
    int ch;

    s = d = buf;
    while((ch = *s++))
  {
    if(isspace(ch)) continue;
    *d++ = tolower(ch);
  }
    *d = 0;
}

int main(int argc, char **argv)
{
	FILE *reader;
	char *read_idf, *s;
	char *ok;
	int i;
	unsigned char has_bg_color, has_bg_pixel;
	Window external_win;
	Pixel bg_pixel;

    if((prg_idf = strrchr(argv[0], '/')) == NULL)
      prg_idf = argv[0];
    else
      ++prg_idf;

	memset(&img, 0, sizeof(ImageInfo));
	external_win = 0; read_idf = NULL; reader = NULL;
	has_bg_color = has_bg_pixel = 0;
	bg_pixel = 0;
	i = 0;

    while(++i < argc)
   {
    s = argv[i];

	if(strcmp(s, "--help") == 0
	|| strcmp(s, "-help") == 0
	|| *s == '?')
  {
	usage(prg_idf);
	return 0;
  }
    if(strcasecmp(s, "--w") == 0)
  {
	++i;
	s = argv[i];
    external_win = strtoul(s, &ok, 10);
	if(*ok)
	  return 0;
	continue;
  }
    if(strcasecmp(s, "--bg") == 0)
  {
	++i;
	s = argv[i];
    if(*s == '#' || strncasecmp(s, "rgb:", 4) == 0 || isalpha(*s))
 {
    strncpy(img.bg_color, s, MAX_COLORBUF);
    img.bg_color[MAX_COLORBUF] = 0;
    has_bg_color = 1;

    if(*s != '#')
      shrink_name(img.bg_color);
    continue;
 }
    bg_pixel = strtoul(s, &ok, 16);

    if(*ok == 0)
      has_bg_pixel = 1;
    continue;
  }
    if(*s != '-')
  {
    read_idf = s; continue;
  }
   }
	if(read_idf != NULL)
   {
	reader = fopen(read_idf, "rb");
	if(reader == NULL)
  {
	perror(read_idf);
	fprintf(stderr, "\n\n%s: cannot open file '%s'\n\n", prg_idf, read_idf);
	return 0;
  }
   }
	img.argv = argv;
	img.argc_ptr = &argc;
	img.external_win = external_win;
    img.has_bg_pixel = has_bg_pixel;
    img.bg_pixel = bg_pixel;
    img.has_bg_color = has_bg_color;

    if(!has_bg_pixel && !has_bg_color)
   {
	strcpy(img.bg_color, DEFAULT_BACKGROUND);
	img.has_bg_color = 1;
   }

	if(read_idf == NULL && external_win == 0)
	  create_widgets(5,5);

	run_viewer(reader, read_idf);

	Viewer_postlude();
	return 0;
}
