/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <zlib.h>
#include <libmng.h>

#ifdef WIN32
#  include <getopt.h>
#else
#  include <unistd.h>
#endif


struct options
{
	char *inmask;
	char *inpath;
	char *outfile;
	int framerate;
	char verbose;
	char backimage;
	char deltamask;
	int sectorsize;
	int fullrects;
} opts;

// externals
char** make_file_list (const char* pattern, int* pnum_entries);
void free_file_list (char** list);

// internals
int error (int errn, const char* fmt, const char* s);
void verbose (const char* fmt, const char* s);
void verbose_d (const char* fmt, int val);
void parse_arguments (int argc, char *argv[], struct options *opts);
int read_file_list (void);
void calc_mng_dims (void);
void select_back_image (void);
void delta_images (void);
int write_mng_file (void);

typedef union
{
	struct
	{
		unsigned char r, g, b, a;
	} bchan;
	unsigned char channels[4];
	unsigned int value;
} RGBA;

typedef struct _file_info
{
	FILE* f;
	char* fname;
	char* fmode;
	int w, h;
	int x, y;
	RGBA* image;
	unsigned char* indimg;
	int delay;
	int identical;
	unsigned short objid;
	unsigned short cloneid;
	int clone;
	unsigned short precloneid;
	int preclone;
	struct _file_info* next;
} file_info;

void file_info_free ();

// MNG callbacks
mng_ptr MNG_DECL mng_alloc (mng_size_t iLen);
void MNG_DECL mng_free (mng_ptr pPtr, mng_size_t iLen);
mng_bool MNG_DECL mng_open_stream(mng_handle mng);
mng_bool MNG_DECL mng_close_stream(mng_handle mng);
mng_bool MNG_DECL mng_read_stream(mng_handle mng, mng_ptr buffer, mng_uint32 size, mng_uint32p bytes);
mng_bool MNG_DECL mng_write_stream (mng_handle mng, mng_ptr buffer, mng_uint32 size, mng_uint32p bytes);
mng_bool MNG_DECL mng_process_header(mng_handle mng, mng_uint32 width, mng_uint32 height);
mng_ptr MNG_DECL mng_get_canvasline_read(mng_handle mng, mng_uint32 line);
mng_ptr MNG_DECL mng_get_canvasline_write(mng_handle mng, mng_uint32 line);
mng_bool MNG_DECL mng_refresh_display(mng_handle mng, mng_uint32 x, mng_uint32 y, mng_uint32 w, mng_uint32 h);
mng_uint32 MNG_DECL mng_get_tickcount(mng_handle mng);
mng_bool MNG_DECL mng_set_timer(mng_handle mng, mng_uint32 msecs);

#define MAX_COLORS	0x100

// global png/mng data
char** Files = 0;
int cFiles = 0;
file_info* Infos = 0;
mng_palette8 Pal;
int cPalClr = 0;
int mngw = 0, mngh = 0;
int iback = 0;
int timerate = 100;		// default 100 ticks per second
int framedelay = 20;	// default 5 fps
int _curframe = -1;
int _curdeltaframe = -1;

int
main(int argc, char* argv[])
{
	int ret = 0;

	parse_arguments (argc, argv, &opts);

	if (opts.framerate) // update delay
		framedelay = timerate / opts.framerate;

	if (!opts.inpath && (strchr (opts.inmask, '/') || strchr (opts.inmask, '\\')))
	{
		char *pch1, *pch2;

		opts.inpath = (char*) calloc (strlen (opts.inmask), 1);
		if (!opts.inpath)
			return error (EXIT_FAILURE, "No memory", 0);

		strcpy (opts.inpath, opts.inmask);
		pch1 = strrchr (opts.inpath, '/');
		pch2 = strrchr (opts.inpath, '\\');
		if (pch2 > pch1)
			pch1 = pch2;

		pch1[1] = 0;	// term the path

		verbose ("Frame files in dir: %s\n", opts.inpath);
	}

	if (!opts.outfile)
	{
		char* pch;

		opts.outfile = (char*) calloc (strlen(opts.inmask) + 6, 1);
		if (!opts.outfile)
			return error (EXIT_FAILURE, "No memory", 0);

		strcpy (opts.outfile, opts.inmask);
		while ((pch = strchr (opts.outfile, '*')) != 0)
			strcpy (pch, pch + 1);
		
		pch = strstr (opts.outfile, ".png");
		if (!pch)
			pch = opts.outfile + strlen (opts.outfile);
		strcpy (pch, ".mng");

		if (pch == opts.outfile || pch[-1] == '/' || pch[-1] == '\\')
		{	// have to fix blank name
			memmove (pch + 1, pch, strlen(pch) + 1);
			*pch = '1';
		}

		verbose ("Output file: %s\n", opts.outfile);
	}

	fprintf (stderr, "using timerate of %d and framedelay of %d\n", timerate, framedelay);

	ret = read_file_list ();
	if (!ret)
	{
		calc_mng_dims ();
		if (opts.backimage)
			select_back_image ();
		delta_images ();

		ret = write_mng_file ();
	}

	file_info_free ();
	free_file_list (Files);

	return ret;
}

int
error(int errn, const char* fmt, const char* s)
{
	fprintf(stderr, fmt, s);
	return errn;
}

void
verbose(const char* fmt, const char* s)
{
	if (!opts.verbose)
		return;

	fprintf(stderr, fmt, s);
}

void verbose_d (const char* fmt, int val)
{
	if (!opts.verbose)
		return;

	fprintf(stderr, fmt, val);
}

void
usage()
{
	fprintf(stderr, "usage: makemng [-v] [-f rate] [-r] [-s size] [-o outputfile] <png-in-mask>\n");
	fprintf(stderr, "produces an MNG animation from a bunch of frame images\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "  -v\t\t : be verbose, explains things no human should know\n");
	fprintf(stderr, "  -f rate\t : sets the framerate; rate is 1..100 per second (default 5)\n");
	fprintf(stderr, "  -b\t\t : auto-select background frame (instead of frame0)\n");
	fprintf(stderr, "  -r\t\t : split delta frames into full rectangles only\n");
	fprintf(stderr, "  -s size\t : enable sector cleanup and set sector size (8..64)\n");
	fprintf(stderr, "diagnostical options:\n");
	fprintf(stderr, "  -d\t\t : generate delta-mask PNGs (form: mask_FRM1_FRM2.png)\n");
}

void
parse_arguments(int argc, char *argv[], struct options *opts)
{
	char ch;
	
	memset(opts, '\0', sizeof (struct options));
	while ((ch = getopt(argc, argv, "?hvbdrf:s:o:")) != -1)
	{
		switch(ch)
		{
		case 'o':
			opts->outfile = optarg;
			break;
		case 'f':
			opts->framerate = atoi(optarg);
			if (opts->framerate < 1 || opts->framerate > 100)
			{
				fprintf(stderr, "invalid -f option value\n");
				usage();
				exit(EXIT_FAILURE);
			}
			break;
		case 'd':
			opts->deltamask = 1;
			break;
		case 'b':
			opts->backimage = 1;
			break;
		case 'r':
			opts->fullrects = 1;
			break;
		case 's':
			opts->sectorsize = atoi(optarg);
			if (opts->sectorsize < 8 || opts->sectorsize > 64)
			{
				fprintf(stderr, "invalid -r option value\n");
				usage();
				exit(EXIT_FAILURE);
			}
			break;
		case 'v':
			opts->verbose = 1;
			break;
		case '?':
		case 'h':
		default:
			usage();
			exit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;
	if (argc != 1)
	{
		usage();
		exit(EXIT_FAILURE);
	}
	
	opts->inmask = argv[0];
}

void
make_file_name(int index, char* buf)
{
	if (opts.inpath)
		strcpy(buf, opts.inpath);
	else
		*buf = 0;

	strcat(buf, Files[index]);
}

void
file_info_init (file_info* ms)
{
	memset(ms, 0, sizeof(*ms));
	ms->identical = -1;
	ms->clone = -1;
	ms->preclone = -1;
}

void
file_info_cleanup (file_info* ms)
{
	file_info* fi = ms;

	while (fi)
	{
		file_info* tempi = fi;

		if (fi->image)
		{
			free(fi->image);
			fi->image = 0;
		}

		if (fi->indimg)
		{
			free(fi->indimg);
			fi->indimg = 0;
		}

		if (fi->f)
		{
			fclose(fi->f);
			fi->f = 0;
		}

		
		fi = fi->next;
		if (tempi != ms)
			free (tempi);
	}
}

void
file_info_free ()
{
	int i;

	if (Infos)
	{
		for (i = 0; i < cFiles; i++)
			file_info_cleanup (Infos + i);

		free (Infos);
		Infos = 0;
	}
}

int
equal_colors (RGBA rgba, mng_palette8e mng_clr)
{
	return rgba.bchan.r == mng_clr.iRed && 
			rgba.bchan.g == mng_clr.iGreen && 
			rgba.bchan.b == mng_clr.iBlue;
}

int
lookup_palette (RGBA rgba)
{
	int i;

	for (i = 0; i < cPalClr && !equal_colors(rgba, Pal[i]); i++)
		;
	
	return i < cPalClr ? i : -1;
}

int
update_palette (file_info* ms)
{
	int i;

	for (i = 0; i < ms->w * ms->h; i++)
	{
		RGBA rgba = ms->image[i];
		int ipal = lookup_palette (rgba);
		if (ipal == -1)
		{	// add color
			if (cPalClr >= MAX_COLORS)
				return 1;

			Pal[cPalClr].iRed = rgba.bchan.r;
			Pal[cPalClr].iGreen = rgba.bchan.g;
			Pal[cPalClr].iBlue = rgba.bchan.b;
			cPalClr++;
		}
	}
	return 0;
}

int
convert_image_indexed (file_info* ms)
{
	int i;

	ms->indimg = (unsigned char*) malloc (ms->w * ms->h);
	if (!ms->indimg)
		return 230;

	for (i = 0; i < ms->w * ms->h; i++)
	{
		int ipal = lookup_palette (ms->image[i]);

		if (ipal == -1)
			return 1;	// something is screwed

		ms->indimg[i] = ipal;
	}
	
	free (ms->image);
	ms->image = 0;

	return 0;
}

int
read_file_list (void)
{
	int ret = 0;
	mng_handle mng;
	char namebuf[260];
	int i;

	cFiles = 0;
	Files = make_file_list(opts.inmask, &cFiles);

	if (!Files || cFiles == 0)
	{
		fprintf (stderr, "No frame files found\n");
		return 1;
	}

	Infos = (file_info*) malloc (sizeof(file_info) * cFiles);
	if (!Infos)
		return 251;

	memset(Infos, 0, sizeof(file_info) * cFiles);

	mng = mng_initialize (MNG_NULL, mng_alloc, mng_free, MNG_NULL);
	if (mng == MNG_NULL)
		return 250;

	// set the callbacks
	mng_setcb_openstream(mng, mng_open_stream);
	mng_setcb_closestream(mng, mng_close_stream);
	mng_setcb_readdata(mng, mng_read_stream);
	mng_setcb_processheader(mng, mng_process_header);
	mng_setcb_getcanvasline(mng, mng_get_canvasline_read);
	mng_setcb_gettickcount(mng, mng_get_tickcount);
	mng_setcb_settimer(mng, mng_set_timer);
	mng_setcb_refresh(mng, mng_refresh_display);

	for (i = 0; i < cFiles && !ret; i++)
	{
		file_info* rf = Infos + i;

		file_info_init (rf);
		make_file_name (i, namebuf);
		rf->fname = namebuf;
		rf->fmode = "rb";

		verbose_d ("%03d ", i); verbose ("reading '%s'...", rf->fname);

		mng_reset (mng);
		mng_set_userdata (mng, rf);

		for (ret = mng_readdisplay (mng);
				ret == MNG_NEEDMOREDATA || ret == MNG_NEEDTIMERWAIT;
				ret = mng_display_resume (mng))
		{
			if (ret == MNG_NEEDTIMERWAIT)
				rf->delay = 0;
		}

		if (ret)
		{
			fprintf (stderr, "Could not read '%s'\n", rf->fname);
			ret = 2;
		}

		ret = update_palette (rf);
		if (ret)
		{
			fprintf (stderr, "Too many unique colors (%d processed), giving up\n", i);
			ret = 3;
		}

		ret = convert_image_indexed (rf);
		if (ret)
		{
			fprintf (stderr, "Image conversion failed on '%s'\n", rf->fname);
			ret = 4;
		}

		verbose (" done\n", 0);
	}

	mng_cleanup (&mng);

	if (ret == MNG_NOERROR)
		fprintf (stderr, "%d animation frames\n", cFiles);

	return ret;
}

void
calc_mng_dims (void)
{
	int i;

	mngw = mngh = -1;

	// get max dims
	for (i = 0; i < cFiles; i++)
	{
		if (Infos[i].w > mngw)
			mngw = Infos[i].w;
		
		if (Infos[i].h > mngh)
			mngh = Infos[i].h;
	}

	// adjust images - center
	for (i = 0; i < cFiles; i++)
	{
		if (Infos[i].w < mngw)
			Infos[i].x = (mngw - Infos[i].w) >> 1;
		
		if (Infos[i].h < mngh)
			Infos[i].y = (mngh - Infos[i].h) >> 1;
	}
}

int
compare_images (file_info* i1, file_info* i2)
{
	int cnt = 0;
	int w, h, x, y;
	int dx1, dx2, dy1, dy2;

	if (i1->w > i2->w)
	{
		w = i2->w;
		dx1 = i2->x;
		dx2 = 0;
	}
	else if (i1->w < i2->w)
	{
		w = i1->w;
		dx1 = 0;
		dx2 = i1->x;
	}
	else
	{
		w = i1->w;
		dx1 = dx2 = 0;
	}

	if (i1->h > i2->h)
	{
		h = i2->h;
		dy1 = i2->y;
		dy2 = 0;
	}
	else if (i1->h < i2->h)
	{
		h = i1->h;
		dy1 = 0;
		dy2 = i1->y;
	}
	else
	{
		h = i1->h;
		dy1 = dy2 = 0;
	}

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			if (i1->indimg[(y + dy1) * i1->w + x + dx1] != i2->indimg[(y + dy2) * i2->w + x + dx2])
				cnt++;
		}
	}
	return cnt;
}

void
select_back_image (void)
{
	int i;
	int* cdiff;
	int max;

	cdiff = (int*) calloc (cFiles, sizeof(int));
	if (!cdiff)
		return;

	verbose ("selecting optimal background image...", 0);

	for (i = 2; i < cFiles; i++)
	{
		if (Infos[i].w == mngw && Infos[i].h == mngh)
		{
			cdiff[i] = compare_images (Infos + i, Infos + i - 1) - 
					compare_images (Infos + i, Infos + 0);
		}
		else
		{
			// image is smaller than animation and cannot be background
			cdiff[i] = 0x7fffffff;
		}
	}

	// the difference has to be big enough
	// or it will be useless
	iback = 0;
	max = mngw * mngh / 32;

	for (i = 2; i < cFiles; i++)
	{
		if (cdiff[i] > max)
		{
			iback = i;
			max = cdiff[i];
		}
	}

	verbose (" done\n", 0);
	fprintf(stderr, "frame %03d selected as background\n", iback);
}

int
equal_images (int i1, int i2)
{
	// deference identical chain
	while (Infos[i1].identical != -1)
		i1 = Infos[i1].identical;
	while (Infos[i2].identical != -1)
		i2 = Infos[i2].identical;

	if (i1 == i2)
		return 1;

	if (Infos[i1].x != Infos[i2].x || Infos[i1].y != Infos[i2].y 
			|| Infos[i1].w != Infos[i2].w || Infos[i1].h != Infos[i2].h)
		return 0;

	return compare_images (Infos + i1, Infos + i2) == 0;
}

int
delta_adjust_positions (int* pos1, int* pos2)
{
	if (*pos1 <= *pos2)
		return 1;
	else
	{
		(*pos1)--;
		(*pos2)--;
		return -1;
	}
}

void
clean_expansion_horz (unsigned char* mask, int w, int x1, int y1, int x2, int y2, int threshold)
{
	int x, y, dx, dy;
	// assume anything out of bounds is cleared
	int prevclear = threshold + 1;

	dy = delta_adjust_positions (&y1, &y2);
	dx = delta_adjust_positions (&x1, &x2);

	for (y = y1; y != y2; y += dy)
	{
		int dcnt, ecnt;
		
		dcnt = ecnt = 0;

		for (x = x1; x != x2; x += dx)
		{
			if (mask[y * w + x] == 1)
				dcnt++;
			else if (mask[y * w + x] == 2 || mask[y * w + x] == 3)
				ecnt++;
		}
		
		if (dcnt == 0 && ecnt == 0)
		{	// line is clear
			prevclear++;
		}
		else if (dcnt == 0)
		{
			if (prevclear >= threshold)
			{	// it's not clear yet, but it will be in a moment ;)
				int lx, ly = y;
				
				if (prevclear == threshold)
				{	// need to clean everything we just checked
					ly = y - prevclear * dy;
				}

				for (ly = ly; ly != y + dy; ly += dy)
					for (lx = x1; lx != x2; lx += dx)
						mask[ly * w + lx] = 0;
			}
			prevclear++;
		}
		else
		{	// line is dirty
			prevclear = 0;
		}
	}
}

void
clean_expansion_vert (unsigned char* mask, int w, int x1, int y1, int x2, int y2, int threshold)
{
	int x, y, dx, dy;
	// assume anything out of bounds is cleared
	int prevclear = threshold + 1;

	dy = delta_adjust_positions (&y1, &y2);
	dx = delta_adjust_positions (&x1, &x2);

	for (x = x1; x != x2; x += dx)
	{
		int dcnt, ecnt;
		
		dcnt = ecnt = 0;

		for (y = y1; y != y2; y += dy)
		{
			if (mask[y * w + x] == 1)
				dcnt++;
			else if (mask[y * w + x] == 2 || mask[y * w + x] == 3)
				ecnt++;
		}
		
		if (dcnt == 0 && ecnt == 0)
		{	// line is clear
			prevclear++;
		}
		else if (dcnt == 0)
		{
			if (prevclear >= threshold)
			{	// it's not clear yet, but it will be in a moment ;)
				int ly, lx = x;

				if (prevclear == threshold)
				{	// need to clean everything we just checked
					lx = x - prevclear * dx;
				}

				for (lx = lx; lx != x + dx; lx += dx)
					for (ly = y1; ly != y2; ly += dy)
						mask[ly * w + lx] = 0;
			}
			prevclear++;
		}
		else
		{	// line is dirty
			prevclear = 0;
		}
	}
}

struct _expand_corner
{
	int x, y;
	int tx1, ty1;
	int tx2, ty2;
}
const expand_corner [] =
{
	{0, 0,  1, 0,  0, 1},	// top-left
	{1, 0,  0, 0,  0, 1},	// top-mid, from left
	{1, 0,  2, 0,  2, 1},	// top-mid, from right
	{2, 0,  1, 0,  2, 1},	// top-right
	{0, 1,  0, 0,  1, 0},	// mid-left, from top
	{0, 1,  0, 2,  1, 2},	// mid-left, from bottom
	{2, 1,  1, 0,  2, 0},	// mid-right, from top
	{2, 1,  1, 2,  2, 2},	// mid-right, from bottom
	{0, 2,  1, 2,  0, 1},	// bot-left
	{1, 2,  0, 1,  0, 2},	// bot-mid, from left
	{1, 2,  2, 1,  2, 2},	// bot-mid, from right
	{2, 2,  1, 2,  2, 1},	// bot-right

	{-1,-1, -1,-1, -1,-1}	// term
};

// this will recursively expand the missing corner pixels
// recursion is limited so we dont overflow the stack
int
expand_rect (char* mask, int x, int y, int w, int h)
{
	static int level = 0;
	int x1, y1, x2, y2, i, lx, ly;
	const struct _expand_corner* pc;
	signed char matrix[3][3];
	int cnt = 0;

	if (level > 99)
		return 1;	// make sure parent knows it failed

	level++;

	if (x > 0)
		x1 = x - 1;
	else
	{
		for (i = 0; i < 3; i++)
			matrix[0][i] = -1;
		
		x1 = x;
	}

	if (y > 0)
		y1 = y - 1;
	else
	{
		for (i = 0; i < 3; i++)
			matrix[i][0] = -1;
		
		y1 = y;
	}

	if (x + 1 < w)
		x2 = x + 2;
	else
	{
		for (i = 0; i < 3; i++)
			matrix[2][i] = -1;
		
		x2 = x + 1;
	}

	if (y + 1 < h)
		y2 = y + 2;
	else
	{
		for (i = 0; i < 3; i++)
			matrix[i][2] = -1;

		y2 = y + 1;
	}

	for (ly = y1; ly < y2; ly++)
		for (lx = x1; lx < x2; lx++)
			matrix[lx - x + 1][ly - y + 1] = mask[ly * w + lx];

	// check corner pixels
	for (pc = expand_corner; pc->x != -1; pc++)
	{
		if (matrix[pc->x][pc->y] == 0 && matrix[pc->tx1][pc->ty1] > 0 && matrix[pc->tx2][pc->ty2] > 0)
		{	// corner pixel missing
			int ofs = (y - 1 + pc->y) * w + (x - 1 + pc->x);
			
			matrix[pc->x][pc->y] = 3;

			// but it may already be present in the mask (recursive)
			if (mask[ofs] == 0)
			{
				mask[ofs] = 3;
				cnt += 1 + expand_rect (mask, x - 1 + pc->x, y - 1 + pc->y, w, h);
			}
		}
	}

	level--;

	return cnt;
}

file_info*
file_info_add_image (file_info* fi)
{
	file_info* ni;

	ni = (file_info*) malloc (sizeof(file_info));
	if (!ni)
		return 0;

	file_info_init (ni);

	while (fi->next)
		fi = fi->next;

	return fi->next = ni;
}

int
is_multi_delta_image (file_info* fi)
{
	return fi && fi->next;
}

#define MASK_COLORS 4
mng_palette8e mask_pal[MASK_COLORS] =
{
	{0x00, 0x00, 0x00},
	{0xff, 0xff, 0xff},
	{0x00, 0xff, 0x00},
	{0x00, 0x00, 0xff}
};

void
create_mask_png (char* mask, int w, int h)
{
	int ret = 0;
	mng_handle mng;
	file_info wf;
	char fname[260];
	mng_ptr imgdata;
	unsigned char* tempdata;
	unsigned char* p;
	uLong srcLen;
	uLong dstLen;
	int i;

	file_info_init (&wf);
	sprintf(fname, "mask_%03d_%03d.png", _curframe, _curdeltaframe);

	wf.fname = fname;
	wf.fmode = "wb";

	// extra byte in front of each line for filter type
	srcLen = w * h + h;
	tempdata = (mng_ptr) malloc(srcLen);
	if (!tempdata)
		return;

	// maximum necessary space
	// deflated data can be 100.1% + 12 bytes in worst case
	dstLen = srcLen + srcLen / 100 + 20;	// extra 8 for safety
	imgdata = (mng_ptr) malloc(dstLen);
	if (!imgdata)
		return;

	for (i = 0, p = tempdata; i < w * h; i++, p++)
	{
		if (i % w == 0)
		{	// write filter byte
			*p++ = 0;
		}
		
		*p = mask[i];
	}

	if (Z_OK != compress2(imgdata, &dstLen, tempdata, srcLen, 9))
		return;

	free(tempdata);

	mng = mng_initialize (&wf, mng_alloc, mng_free, MNG_NULL);
	if (mng == MNG_NULL)
		return;

	// set the callbacks
	mng_setcb_openstream(mng, mng_open_stream);
	mng_setcb_closestream(mng, mng_close_stream);
	mng_setcb_writedata(mng, mng_write_stream);

	ret = mng_create (mng);

	ret = mng_putchunk_ihdr (mng, w, h,
			MNG_BITDEPTH_8, MNG_COLORTYPE_INDEXED, MNG_COMPRESSION_DEFLATE,
			MNG_FILTER_ADAPTIVE, MNG_INTERLACE_NONE);

	if (ret == MNG_NOERROR)
		ret = mng_putchunk_plte (mng, 4, mask_pal);
	if (ret == MNG_NOERROR)
		ret = mng_putchunk_idat (mng, dstLen, imgdata);
	if (ret == MNG_NOERROR)
		ret = mng_putchunk_iend (mng);

	free (imgdata);

	if (ret == MNG_NOERROR)
		ret = mng_write (mng);

	mng_cleanup (&mng);

	file_info_cleanup (&wf);
}

int
build_delta (file_info* i1, file_info* i2)
{
	int w, h, x, y;
	int dx1, dx2, dy1, dy2;
	int cnt;
	char* mask = 0;

	if (i1->w > i2->w)
	{
		w = i2->w;
		dx1 = i2->x;
		dx2 = 0;
	}
	else if (i1->w < i2->w)
	{
		w = i1->w;
		dx1 = 0;
		dx2 = i1->x;
	}
	else
	{
		w = i1->w;
		dx1 = dx2 = 0;
	}

	if (i1->h > i2->h)
	{
		h = i2->h;
		dy1 = i2->y;
		dy2 = 0;
	}
	else if (i1->h < i2->h)
	{
		h = i1->h;
		dy1 = 0;
		dy2 = i1->y;
	}
	else
	{
		h = i1->h;
		dy1 = dy2 = 0;
	}

	mask = (char*) malloc (w * h);
	if (!mask)
		return 220;
	memset(mask, 0, w * h);

	// build diff mask first
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			if (i1->indimg[(y + dy1) * i1->w + x + dx1] != i2->indimg[(y + dy2) * i2->w + x + dx2])
				// diff pixel
				mask[y * w + x] = 1;
		}
	}

	// coarse expand the diff pixels
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			if (mask[y * w + x] == 1)
			{
				int x1 = x - 2;
				int x2 = x + 3;
				int y1 = y - 2;
				int y2 = y + 3;
				int lx;
				if (x1 < 0)
					x1 = 0;
				if (x2 > w)
					x2 = w;
				if (y1 < 0)
					y1 = 0;
				if (y2 > h)
					y2 = h;

				for (y1 = y1; y1 < y2; y1++)
					for (lx = x1; lx < x2; lx++)
						if (mask[y1 * w + lx] == 0)
							mask[y1 * w + lx] = 2;
			}
		}
	}

	// scan and remove extra expansion horizontally and vertically
	clean_expansion_vert (mask, w, 0, 0, w, h, 1);
	clean_expansion_vert (mask, w, w, 0, 0, h, 1);
	clean_expansion_horz (mask, w, 0, 0, w, h, 1);
	clean_expansion_horz (mask, w, 0, h, w, 0, 1);

	
	do	// coarse expand the diff pixels
	{	// merge would-be diff rectangles in the process
		cnt = 0;

		for (y = 0; y < h; y++)
		{
			for (x = 0; x < w; x++)
			{
				if (mask[y * w + x] != 0)
					cnt += expand_rect (mask, x, y, w, h);
			}
		}
		// repeat is something was expanded
	} while (cnt != 0);

	// at this point we should have guaranteed non-overlapping
	// rectangles that cover all of the delta areas

	if (opts.sectorsize)
	{	// final expansion cleanup
		for (y = 0; y < h; y += opts.sectorsize)
		{
			for (x = 0; x < w; x += opts.sectorsize)
			{
				int x2, y2;

				cnt = 0;
				for (y2 = y; y2 < y + opts.sectorsize && y2 < h; y2++)
					for (x2 = x; x2 < x + opts.sectorsize && x2 < w; x2++)
						if (mask[y2 * w + x2] == 1)
							cnt++;

				if (cnt > 0)
					continue;	// dirty sector

				// clean up sector
				for (y2 = y; y2 < y + opts.sectorsize && y2 < h; y2++)
					for (x2 = x; x2 < x + opts.sectorsize && x2 < w; x2++)
						mask[y2 * w + x2] = 0;

			}
		}
	}

	// check how muany pixels have to be replaced
	for (x = 0, cnt = 0; x < w * h; x++)
		if (mask[x])
			cnt++;

	if (opts.deltamask)
		create_mask_png (mask, w, h);

	// generate delta images
	if (cnt != w * h)
	{
		int ofs;

		for (y = 0, ofs = 0; y < h; y++)
		{
			for (x = 0; x < w; x++, ofs++)
			{
				if (mask[ofs] != 0)
				{	// copy masked rectangle into a new image
					// and clear the mask
					int i;
					int rw, rh;
					int x2, y2;
					unsigned char* src;
					unsigned char* dst;
					file_info* ni;
					
					ni = file_info_add_image (i1);
					if (!ni)
					{
						x = w;
						y = h;
						break;
					}
		

					// lookup delta rectangle
					for (i = x, src = mask + ofs; i < w && *src != 0; i++, src++)
						;
					ni->w = rw = i - x;

					if (opts.fullrects)
					{	// locate only complete rectangles
						y2 = y + 1;
						for (i = y + 1, src = mask + ofs; i < h && *src != 0; i++, src += w)
						{
							unsigned char* src2;

							y2 = i;
							for (x2 = x, src2 = src; x2 < x + rw && *src2 != 0; x2++, src2++)
								;

							if (x2 < x + rw)
								break;
						}
					}
					else
					{	// any rectangles
						for (y2 = y + 1, src = mask + ofs; y2 < h && *src != 0; y2++, src += w)
							;
					}
					
					ni->h = rh = y2 - y;

					ni->indimg = (unsigned char*) malloc (rw * rh);
					if (!ni->indimg)
					{
						x = w;
						y = h;
						break;
					}

					// copy the pixels
					for (i = 0, src = i1->indimg + (dy1 + y) * i1->w + dx1 + x, dst = ni->indimg;
							i < rh;
							i++, src += i1->w, dst += rw
						)
					{
						memcpy (dst, src, rw);
						memset (mask + ofs + i * w, 0, rw);
					}

					ni->x = i1->x + dx1 + x;
					ni->y = i1->y + dy1 + y;
				}
			}
		}

		if (i1->next)
		{	// dispose of the original
			file_info* ni = i1->next;
			free (i1->indimg);
			i1->indimg = ni->indimg;
			i1->x = ni->x;
			i1->y = ni->y;
			i1->w = ni->w;
			i1->h = ni->h;
			i1->next = ni->next;
			
			free (ni);
		}
	}
	else
	{	// break here
		cnt = 1;
	}

	if (mask)
		free (mask);

	return 0;
}

void
delta_images (void)
{
	int i;
	unsigned short nextid = 0x101;

	verbose ("calculating frame image deltas", 0);

	Infos[iback].objid = nextid++;
	if (iback != 0)
	{	// set the first frame objid different
		// from back id
		Infos[0].objid = nextid++;
	}

	// remove dupes
	for (i = 1; i < cFiles; i++)
	{
		int i2;
		
		if (i == iback)
			continue;

		Infos[i].objid = Infos[i - 1].objid;

		for (i2 = i - 1; i2 >= 0 && Infos[i].identical == -1; i2--)
		{
			int orgi2 = i2;

			// deference identical chain
			while (Infos[i2].identical != -1)
				i2 = Infos[i2].identical;

			if (equal_images (i, i2))
			{
				Infos[i].identical = i2;
				// dont need image data anymore
				if (Infos[i].indimg)
				{
					free (Infos[i].indimg);
					Infos[i].indimg = 0;
				}

				if (orgi2 != i - 1)
				{	// detached descendant
					// clone the object for it
					if (Infos[i2].clone == -1)
					{	// no clones yet
						Infos[i2].cloneid = nextid++;
						Infos[i2].clone = i;
						Infos[i].objid = Infos[i2].cloneid;
					}
					else
					{	// already cloned for another frame
						// tell the frame to preclone it for
						// this frame too
						// dereference preclone chain first
						for (i2 = Infos[i2].clone; Infos[i2].preclone != -1; i2 = Infos[i2].preclone)
							;
						Infos[i2].preclone = i;
						Infos[i2].precloneid = nextid++;
						Infos[i].objid = Infos[i2].precloneid;
					}
				}
			}
		}
		verbose (".", 0);
	}
	verbose ("|", 0);

	// compute deltas
	for (i = cFiles - 1; i >= 0; i--)
	{
		int i2;

		if (i == iback || Infos[i].identical != -1)
			// no delta needed
			continue;
		else
		{
			if (i == 0 && i != iback)
			{	// delta against original background
				i2 = iback;
			}
			else
			{	// deref indentical chain
				for (i2 = i - 1; i2 >= 0 && Infos[i2].identical != -1; i2 = Infos[i2].identical)
					;
				
				// sanity check
				if (Infos[i2].objid != Infos[i].objid)
				{
					fprintf (stderr, "delta_images: logical error 1\n");
					exit(EXIT_FAILURE);
				}
			}

			// debug info
			_curframe = i;
			_curdeltaframe = i2;

			build_delta (Infos + i, Infos + i2);
		}
		verbose (".", 0);
	}	
	
	verbose ("\n", 0);
}

int
get_png_image_data (file_info* ms, unsigned char* imgdata)
{
	int i;

	for (i = 0; i < ms->w * ms->h; i++, imgdata++)
	{
		if (i % ms->w == 0)
		{	// write filter byte
			*imgdata++ = 0;
		}
		
		*imgdata = ms->indimg[i];
	}
	return 0;
}

int
compress_png (file_info* ms, mng_ptr imgdata, mng_uint32 imglen, mng_uint32p bytes)
{
	int ret = 0;
	unsigned char* tempdata;
	uLong srcLen;

	*bytes = 0;

	// extra byte in front of each line for filter type
	srcLen = ms->w * ms->h + ms->h;
	tempdata = (mng_ptr) malloc(srcLen);
	if (!tempdata)
		return 241;

	ret = get_png_image_data (ms, tempdata);
	if (!ret)
	{
		uLong dstLen = imglen;

		if (Z_OK == compress2(imgdata, &dstLen, tempdata, srcLen, 9))
			*bytes = dstLen;
		else
			ret = 253;
	}
	
	free(tempdata);

	return ret;
}

int
output_png (mng_handle mng, file_info* rf, int delta)
{
	int ret = 0;
	mng_ptr imgdata;
	mng_uint32 imglen;
	mng_uint32 cbcomp;
	unsigned short objid = rf->objid;

	// maximum necessary space
	// deflated data can be 100.1% + 12 bytes in worst case
	imglen = mngw * mngh + mngh;
	imglen += imglen / 100 + 20;	// extra 8 for safety
	imgdata = (mng_ptr) malloc(imglen);
	if (!imgdata)
		return 252;

	do
	{
		if (delta)
		{	// output delta
			ret = mng_putchunk_dhdr (mng, objid,
					MNG_IMAGETYPE_PNG, MNG_DELTATYPE_BLOCKPIXELREPLACE,
					rf->w, rf->h, rf->x, rf->y);
		}
		else
		{	// output image verbatim
			ret = mng_putchunk_ihdr (mng, rf->w, rf->h,
					MNG_BITDEPTH_8, MNG_COLORTYPE_INDEXED, MNG_COMPRESSION_DEFLATE,
					MNG_FILTER_ADAPTIVE, MNG_INTERLACE_NONE);

			if (ret == MNG_NOERROR)
			{	// write empty PLTE to use the global PLTE
				ret = mng_putchunk_plte (mng, 0, Pal);
				//ret = mng_putchunk_plte (mng, cPalClr, Pal);	// enable to write plain PNG
			}
		}

		if (ret == MNG_NOERROR)
			ret = compress_png (rf, imgdata, imglen, &cbcomp);
		
		if (ret == MNG_NOERROR)
			ret = mng_putchunk_idat (mng, cbcomp, imgdata);
		
		if (ret == MNG_NOERROR)
			ret = mng_putchunk_iend (mng);

	} while ((rf = rf->next) != 0 && ret == MNG_NOERROR);

	free (imgdata);

	return ret;
}

int
write_mng_file (void)
{
	int ret = 0;
	mng_handle mng;
	file_info wf;
	file_info rf;
	file_info backf;
	int i;
	unsigned short lastobjid;
	char curframemode, newframemode;

	mng = mng_initialize (MNG_NULL, mng_alloc, mng_free, MNG_NULL);
	if (mng == MNG_NULL)
	{
		fprintf (stderr, "libmng did not init properly\n");
		return 250;
	}

	// set the callbacks
	mng_setcb_openstream(mng, mng_open_stream);
	mng_setcb_closestream(mng, mng_close_stream);
	mng_setcb_writedata(mng, mng_write_stream);

	file_info_init (&wf);
	wf.fname = opts.outfile;
	wf.fmode = "wb";
	mng_set_userdata (mng, &wf);

	ret = mng_create (mng);
	if (ret != MNG_NOERROR)
		fprintf (stderr, "Could not create '%s'\n", wf.fname);
	else
		verbose ("writing MNG file '%s'", wf.fname);

	ret = mng_putchunk_mhdr (mng, mngw, mngh, timerate, 0, 0, 0,
			MNG_SIMPLICITY_VALID | MNG_SIMPLICITY_SIMPLEFEATURES |
			MNG_SIMPLICITY_COMPLEXFEATURES | MNG_SIMPLICITY_DELTAPNG | 0x240);
	
	//ret = mng_putchunk_term (mng, MNG_TERMACTION_LASTFRAME, MNG_ITERACTION_LASTFRAME, 0, 0);

	ret = mng_putchunk_plte (mng, cPalClr, Pal);

	ret = mng_putchunk_back (mng, 0,0,0, 0, 0, MNG_BACKGROUNDIMAGE_NOTILE);

	curframemode = MNG_FRAMINGMODE_1;
	ret = mng_putchunk_fram (mng, MNG_FALSE, curframemode, 0,MNG_NULL,
			MNG_CHANGEDELAY_DEFAULT, MNG_CHANGETIMOUT_NO, MNG_CHANGECLIPPING_NO, MNG_CHANGESYNCID_NO,
			framedelay, 0,0,0,0,0,0, MNG_NULL,0);

	// define the staring image/object
	backf = Infos[iback];
	ret = mng_putchunk_defi (mng, backf.objid, MNG_DONOTSHOW_NOTVISIBLE, MNG_CONCRETE, MNG_FALSE, 0,0, MNG_FALSE, 0,0,0,0);
	ret = output_png (mng, &backf, 0);

	//ret = mng_putchunk_save (mng, MNG_TRUE, 0,0);
	//ret = mng_putchunk_seek (mng, 5, "start");

	if (iback != 0)
	{	// clone the starting object for the first frame
		ret = mng_putchunk_clon (mng, backf.objid, Infos[0].objid,
				MNG_FULL_CLONE, MNG_DONOTSHOW_NOTVISIBLE, MNG_CONCRETE_ASPARENT,
				MNG_FALSE, 0,0,0);
	}

	lastobjid = 0;

	for (i = 0; i < cFiles && ret == MNG_NOERROR; i++)
	{
		rf = Infos[i];

		if (rf.precloneid != 0)
		{	// pre-clone the object for another frame
			ret = mng_putchunk_clon (mng, rf.objid, rf.precloneid,
					MNG_FULL_CLONE, MNG_DONOTSHOW_NOTVISIBLE, MNG_CONCRETE_ASPARENT,
					MNG_FALSE, 0,0,0);
		}

		if (is_multi_delta_image (&rf))
			// multi-delta png; frame mode: 0-delay for subframe
			newframemode = MNG_FRAMINGMODE_2;
		else
			// frame mode: 1 image per frame
			newframemode = MNG_FRAMINGMODE_1;


		if (newframemode != curframemode)
		{	// change framing mode only
			ret = mng_putchunk_fram (mng, MNG_FALSE, newframemode, 0,MNG_NULL,
					MNG_CHANGEDELAY_NO, MNG_CHANGETIMOUT_NO, MNG_CHANGECLIPPING_NO, MNG_CHANGESYNCID_NO,
					0,0,0,0,0,0,0, MNG_NULL,0);
			curframemode = newframemode;
		}
		else if (curframemode == MNG_FRAMINGMODE_2)
		{	// start new subframe
			ret = mng_putchunk_fram (mng, MNG_TRUE, 0,0,MNG_NULL,0,0,0,0,0,0,0,0,0,0,0,0,0);
		}

		if (rf.indimg != 0 && i != iback)
		{	// display a delta png
			ret = output_png (mng, &rf, 1);
		}

		if (rf.cloneid != 0)
		{	// post-clone the object for another frame
			ret = mng_putchunk_clon (mng, rf.objid, rf.cloneid,
					MNG_FULL_CLONE, MNG_DONOTSHOW_NOTVISIBLE, MNG_CONCRETE_ASPARENT,
					MNG_FALSE, 0,0,0);
		}

		if (rf.objid != lastobjid || rf.identical != -1)
		{	// show the object for this frame
			ret = mng_putchunk_show (mng, MNG_FALSE, rf.objid, rf.objid, MNG_SHOWMODE_0);
			lastobjid = rf.objid;
		}

		verbose (".", 0);
	}

	//ret = mng_putchunk_seek (mng, 3, "end");
	ret = mng_putchunk_mend (mng);

	ret = mng_write (mng);

	file_info_cleanup (&wf);

	mng_cleanup (&mng);

	if (ret == MNG_NOERROR)
		verbose ("finished.\n", 0);
	else
		fprintf (stderr, "Could not create MNG file\n");

	return ret;
}

mng_ptr MNG_DECL
mng_alloc (mng_size_t iLen)
{
	mng_ptr ptr;

	if (iLen & 0x80000000)
		return 0;	// MNG error!

	ptr = malloc (iLen);
	if (ptr)
		memset(ptr, 0, iLen);
	
	return ptr;
}

void MNG_DECL
mng_free (mng_ptr pPtr, mng_size_t iLen)
{
	if (iLen)
		free (pPtr);
}

mng_bool MNG_DECL
mng_open_stream (mng_handle mng)
{
	file_info* ms;

	ms = (file_info*) mng_get_userdata (mng);
	
	ms->f = fopen (ms->fname, ms->fmode);
	if (!ms->f)
	{
		fprintf(stderr, "unable to open '%s'\n", ms->fname);
		return MNG_FALSE;
	}

	return MNG_TRUE;
}

mng_bool MNG_DECL
mng_close_stream (mng_handle mng)
{
	file_info* ms;

	ms = (file_info*) mng_get_userdata (mng);

	fclose(ms->f);
	ms->f = NULL;

	return MNG_TRUE;
}

mng_bool MNG_DECL
mng_read_stream (mng_handle mng, mng_ptr buffer, mng_uint32 size, mng_uint32p bytes)
{
	file_info* ms;

	ms = (file_info*) mng_get_userdata (mng);

	*bytes = fread(buffer, 1, size, ms->f);

	return MNG_TRUE;
}

mng_bool MNG_DECL
mng_write_stream (mng_handle mng, mng_ptr buffer, mng_uint32 size, mng_uint32p bytes)
{
	file_info* ms;

	ms = (file_info*) mng_get_userdata (mng);

	*bytes = fwrite(buffer, 1, size, ms->f);

	return MNG_TRUE;
}

mng_bool MNG_DECL
mng_process_header (mng_handle mng, mng_uint32 width, mng_uint32 height)
{
	file_info* ms;

 	ms = (file_info*) mng_get_userdata (mng);
	ms->w = width;
	ms->h = height;
	ms->image = (RGBA*) malloc(sizeof(RGBA) * width * height);
	if (!ms->image)
		return MNG_FALSE;

	mng_set_canvasstyle(mng, MNG_CANVAS_RGBA8);

	return MNG_TRUE;
}

mng_ptr MNG_DECL
mng_get_canvasline_read (mng_handle mng, mng_uint32 line)
{
	file_info* ms;
	mng_ptr row;

 	ms = (file_info*) mng_get_userdata (mng);

	row = ms->image + ms->w * line;
 
	return row;
}

mng_ptr MNG_DECL
mng_get_canvasline_write (mng_handle mng, mng_uint32 line)
{
	file_info* ms;

 	ms = (file_info*) mng_get_userdata (mng);

	//if (!ms->rowdata)
	//	ms->rowdata = (unsigned char*) malloc (ms->w);
	//if (!ms->rowdata)
	//	return MNG_NULL;

	//make_pal_row (ms, line, ms->rowdata);

	// satisfying compiler
	line = 0;

	return MNG_NULL;
}

mng_bool MNG_DECL
mng_refresh_display (mng_handle mng, mng_uint32 x, mng_uint32 y, mng_uint32 w, mng_uint32 h)
{
	// not implemented
	file_info* ms;

 	ms = (file_info*) mng_get_userdata (mng);

	// satisfying compiler
	x = y = w = h = 0;

	return MNG_TRUE;
}

mng_uint32 MNG_DECL
mng_get_tickcount (mng_handle mng)
{
	// not implemented
	file_info* ms;
	static int tick = 0;

 	ms = (file_info*) mng_get_userdata (mng);

	return tick += 50;
}

mng_bool MNG_DECL
mng_set_timer (mng_handle mng, mng_uint32 msecs)
{
	// not implemented
	file_info* ms;

 	ms = (file_info*) mng_get_userdata (mng);
	ms->delay = msecs;

	return MNG_TRUE;
}
