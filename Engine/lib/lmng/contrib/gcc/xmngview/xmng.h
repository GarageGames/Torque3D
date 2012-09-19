#ifndef _XMNG_H_
#define _XMNG_H
#define RGB_SIZE                3
#define CANVAS_RGB8_SIZE		3
#define CANVAS_RGBA8_SIZE 		4
#define CANVAS_ARGB8_SIZE 		4
#define CANVAS_RGB8_A8_SIZE 	4
#define CANVAS_BGR8_SIZE 		3
#define CANVAS_BGRA8_SIZE 		4
#define CANVAS_BGRA8PM_SIZE 	4
#define CANVAS_ABGR8_SIZE 		4

#define MNG_MAGIC "\x8aMNG\x0d\x0a\x1a\x0a"
#define JNG_MAGIC "\x8bJNG\x0d\x0a\x1a\x0a"
#define PNG_MAGIC "\x89PNG\x0d\x0a\x1a\x0a"
#define PSEUDOCOLOR 1
#define TRUECOLOR   2

#define MNG_TYPE 1
#define JNG_TYPE 2
#define PNG_TYPE 3

#define SPACE_X	10
#define SPACE_Y 10
#define BUT_ENTRY_BORDER 0
#define FRAME_SHADOW_WIDTH 2
#define ANY_WIDTH 4

#define OK MNG_NOERROR 
#define MAX_COLORBUF 64

typedef struct
{
    unsigned int frozen:1;
    unsigned int restarted:1;
    unsigned int stopped:1;
    unsigned int single_step_wanted:1;
    unsigned int single_step_served:1;
	unsigned int has_bg_color:1;
	unsigned int has_bg_pixel:1;
	unsigned int x11_init:1;
	unsigned int timer_active:1;

	mng_handle user_handle;
	Widget canvas;
	int type;
	XtIntervalId timeout_ID;
	mng_uint32 counter;
    mng_uint32 delay;
    mng_uint32 img_width, img_height;
    mng_uint32 read_len;
	mng_uint32 read_pos;
	unsigned char *read_buf;
	unsigned char *mng_buf;
	unsigned char *dither_line;

	Window external_win;
	Window frame_win;
	Window control_win;
	GC gc;
	Display *dpy;
	Window win;
	unsigned short mng_rgb_size;
	unsigned short mng_bytes_per_line;
	XImage *ximage;
	int src_x, src_y;
	int dst_x, dst_y;
	unsigned int frame_w, frame_h;

	void *shm;
	int gray;
	int display_depth, display_type;
	int have_shmem;
	Pixel bg_pixel;
	unsigned short xbg_red, xbg_green, xbg_blue;
	unsigned char bg_red, bg_green, bg_blue;
	Visual *visual;
	unsigned int depth;
/* do not free */
	struct timeval timer_start;
	struct timeval timer_end;
	
    char *read_idf;
    FILE *reader;
	int *argc_ptr;
	char **argv;
	char bg_color[MAX_COLORBUF];
} ImageInfo;

#define XPUTIMAGE(dpy,dr,gc,xi,a,b,c,d,w,h)                          \
    if (have_shmem)                                                  \
    XShmPutImage(dpy,dr,gc,xi,a,b,c,d,w,h,True);                 \
    else                                                             \
    XPutImage(dpy,dr,gc,xi,a,b,c,d,w,h)

extern void Viewer_postlude(void);
extern XImage *x11_create_ximage(ImageInfo *data);
extern void x11_destroy_ximage(ImageInfo *data);
extern void x11_init_color(ImageInfo *data);
extern void viewer_renderline(ImageInfo *data, unsigned char *scanline, 
	unsigned int row, unsigned int x, unsigned int width);

#endif
