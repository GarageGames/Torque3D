//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
//
// MNGView Sample Application for VC6:
// Loads all MNG/JNG/PNG Files LibMNG can do
// Can save a single Frame to PNG Format.
//
// This code is public domain.
// Created by Nikolaus Brennig, November 14th, 2000.
// virtualnik@nol.at
// http://cust.nol.at/ppee
//
// Tab: 4
//
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include "Main.h"
#include <libmng.h>


//---------------------------------------------------------------------------------------------
// VARS:
//---------------------------------------------------------------------------------------------
typedef struct 
{
	FILE			*file;	   
	LPSTR			filename; 
	mng_uint32		delay;     
} mngstuff;

int					lineWidth;
BYTE				*mngdestbuffer;
mngstuff			*mymngstuff;
mng_handle			Curmng;



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------------
// callbacks for the mng decoder:
//---------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------
// memory allocation; data must be zeroed
//---------------------------------------------------------------------------------------------
mng_ptr mymngalloc( mng_uint32 size )
{
	return (mng_ptr)calloc(1, size);
}


//---------------------------------------------------------------------------------------------
// memory deallocation
//---------------------------------------------------------------------------------------------
void mymngfree(mng_ptr p, mng_uint32 size)
{
	free(p);
}


//---------------------------------------------------------------------------------------------
// Stream open:
//---------------------------------------------------------------------------------------------
mng_bool mymngopenstream(mng_handle mng)
{
	mngstuff	*mymng;

	// look up our stream struct
    mymng = (mngstuff*)mng_get_userdata(mng);
	
	// open the file
	mymng->file = fopen( mymng->filename, "rb" );
	if( mymng->file == NULL ) 
	{
		char temp[100];
		sprintf( temp, "Unable to open File: %s", mymng->filename );
		Warning( temp );
		return MNG_FALSE;
	}

	return MNG_TRUE;
}


//---------------------------------------------------------------------------------------------
// Stream open for Writing:
//---------------------------------------------------------------------------------------------
mng_bool mymngopenstreamwrite(mng_handle mng)
{
	mngstuff	*mymng;

	// look up our stream struct
    mymng = (mngstuff*)mng_get_userdata(mng);
	
	// open the file
	mymng->file = fopen( mymng->filename, "wb" );
	if( mymng->file == NULL ) 
	{
		Warning( "unable to open file!" );
		return MNG_FALSE;
	}

	return MNG_TRUE;
}


//---------------------------------------------------------------------------------------------
// Stream close:
//---------------------------------------------------------------------------------------------
mng_bool mymngclosestream(mng_handle mng)
{
	return MNG_TRUE; // We close the file ourself, mng_cleanup doesnt seem to do it...
}


//---------------------------------------------------------------------------------------------
// feed data to the decoder
//---------------------------------------------------------------------------------------------
mng_bool mymngreadstream( mng_handle mng, mng_ptr buffer, mng_uint32 size, mng_uint32 *bytesread )
{
	mngstuff *mymng;

	// look up our stream struct
	mymng = (mngstuff*)mng_get_userdata(mng);

	// read the requested amount of data from the file
	*bytesread = fread( buffer, sizeof(BYTE), size, mymng->file );

	return MNG_TRUE;
}


//---------------------------------------------------------------------------------------------
// the header's been read. set up the display stuff
//---------------------------------------------------------------------------------------------
mng_bool mymngprocessheader( mng_handle mng, mng_uint32 width, mng_uint32 height )
{
	// Store values:
	W = width; H = height;
	Bits = 24;
	lineWidth = ((((W * Bits) + 31) >> 5) << 2);

	// Create decoderbuffer:
	mngdestbuffer = new BYTE[lineWidth*H];
	if( mngdestbuffer == 0 ) Warning( "Out of Memory!" );

	// Create the MemoryBitmap now, we store there the image...
	if( DefaultMemImage ) SelectObject( MemDC, DefaultMemImage );
	if( MemDC ) DeleteDC( MemDC );
	if( MemImage ) DeleteObject( MemImage );
	hdc = GetDC( hPicWin );
    MemDC = CreateCompatibleDC( 0 );
	MemImage = CreateCompatibleBitmap( hdc, W, H );
	DefaultMemImage = (HBITMAP)SelectObject( MemDC, MemImage );
	ReleaseDC( hPicWin, hdc );

	// Set output style:
	mng_set_canvasstyle( mng, MNG_CANVAS_BGR8 );

	return MNG_TRUE;
}


//---------------------------------------------------------------------------------------------
// return a row pointer for the decoder to fill
//---------------------------------------------------------------------------------------------
mng_ptr mymnggetcanvasline( mng_handle mng, mng_uint32 line )
{
	return (mng_ptr)(mngdestbuffer + (lineWidth*(H-1-line)));
}


//---------------------------------------------------------------------------------------------
// timer
//---------------------------------------------------------------------------------------------
mng_uint32 mymnggetticks(mng_handle mng)
{
	return (mng_uint32)GetTickCount();
}


//---------------------------------------------------------------------------------------------
// Refresh:
//---------------------------------------------------------------------------------------------
mng_bool mymngrefresh( mng_handle mng, mng_uint32 x, mng_uint32 y, mng_uint32 w, mng_uint32 h )
{
    PBITMAPINFO bmpi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
	bmpi->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	bmpi->bmiHeader.biWidth			= W;
	bmpi->bmiHeader.biHeight		= H;
	bmpi->bmiHeader.biPlanes		= 1;
	bmpi->bmiHeader.biCompression   = BI_RGB; 
	bmpi->bmiHeader.biBitCount		= Bits;
	bmpi->bmiHeader.biSizeImage		= 0;
	bmpi->bmiHeader.biClrUsed		= 0; 
	bmpi->bmiHeader.biClrImportant	= 0; 

	// Now blt the Image onto our MemDC...
	StretchDIBits( MemDC, 0, 0, W, H, 0, 0, W, H, mngdestbuffer, bmpi, 0, SRCCOPY );
	LocalFree((PBITMAPINFO)bmpi);

	return MNG_TRUE;
}


//---------------------------------------------------------------------------------------------
// interframe delay callback
//---------------------------------------------------------------------------------------------
mng_bool mymngsettimer(mng_handle mng, mng_uint32 msecs)
{
	mngstuff	*mymng;

	// look up our stream struct
    mymng = (mngstuff*)mng_get_userdata(mng);

	// set the timer for when the decoder wants to be woken
	mymng->delay = msecs;

	return MNG_TRUE;
}


//---------------------------------------------------------------------------------------------
// Error Callback;
//---------------------------------------------------------------------------------------------
mng_bool mymngerror(
	mng_handle mng, mng_int32 code, mng_int8 severity,
	mng_chunkid chunktype, mng_uint32 chunkseq,
	mng_int32 extra1, mng_int32 extra2, mng_pchar text
	)
{
	char	chunk[5];

	// pull out the chuck type as a string
	// FIXME: does this assume unsigned char?
	chunk[0] = (char)((chunktype >> 24) & 0xFF);
	chunk[1] = (char)((chunktype >> 16) & 0xFF);
	chunk[2] = (char)((chunktype >>  8) & 0xFF);
	chunk[3] = (char)((chunktype      ) & 0xFF);
	chunk[4] = '\0';

	// output the error:
	char temp[1000];
	sprintf( temp, "error playing chunk %s (%d)", chunk, chunkseq );
	Warning( temp );

	// No need for anymore decoding:
	KillTimer( hPicWin, 2 );

	// error occured;
	return MNG_FALSE;
}


//---------------------------------------------------------------------------------------------
// Load a MNG/JNG/PNG file:
//---------------------------------------------------------------------------------------------
VOID LoadMNG( LPSTR Filename, HWND hwnd, HDC hdc )
{
	// allocate our stream data structure
	mymngstuff = (mngstuff*)calloc(1, sizeof(*mymngstuff));
	if( mymngstuff == NULL ) 
	{
		Warning( "Unable to allocate MNG struct!" );
		return;
	}

	// pass the name of the file we want to play
	mymngstuff->filename = Filename;

	// set up the mng decoder for our stream
    Curmng = mng_initialize(mymngstuff, mymngalloc, mymngfree, MNG_NULL);
    if(Curmng == MNG_NULL) 
	{
		free(mymngstuff);
		Warning( "MNG Init Error!" );
		return;
    }

	// No need to store chunks:
	mng_set_storechunks(Curmng, MNG_FALSE);

	// Set the colorprofile, lcms uses this:
	mng_set_srgb( Curmng, MNG_TRUE );
	char DestDir[2048];
	SearchPath( NULL, "MNGVIEW.EXE", NULL, sizeof(DestDir), DestDir, NULL );
	lstrcpyn( DestDir, DestDir, lstrlen(DestDir)-lstrlen("MNGVIEW.EXE") );
	catpath( DestDir, "sRGB.icm" );
	FILE *RGBfile = fopen( DestDir, "rb" );
	if( RGBfile == 0 )
	{
		mng_cleanup(&Curmng);
		free(mymngstuff);
		Warning( "Need file \"sRGB.icm\" !" );
		return;
    }
	fclose(RGBfile);
	mng_set_outputprofile(Curmng, DestDir);

	// Set white as background color:
	WORD Red   = (255 << 8) + 255;
	WORD Green = (255 << 8) + 255;
	WORD Blue  = (255 << 8) + 255;
	mng_set_bgcolor( Curmng, Red, Green, Blue );

	// If PNG Background is available, use it:
	mng_set_usebkgd( Curmng, MNG_TRUE );

	// set the callbacks
	mng_setcb_errorproc(Curmng, mymngerror);
    mng_setcb_openstream(Curmng, mymngopenstream);
    mng_setcb_closestream(Curmng, mymngclosestream);
    mng_setcb_readdata(Curmng, mymngreadstream);
	mng_setcb_gettickcount(Curmng, mymnggetticks);
	mng_setcb_settimer(Curmng, mymngsettimer);
	mng_setcb_processheader(Curmng, mymngprocessheader);
	mng_setcb_getcanvasline(Curmng, mymnggetcanvasline);
	mng_setcb_refresh(Curmng, mymngrefresh);

	// Read the stuff:
	mng_readdisplay(Curmng);

	AnimFile.CurFrame	 = mng_get_layercount( Curmng );
	AnimFile.MaxFrame	 = mng_get_framecount( Curmng );
	AnimFile.isAnimation = 1;
	AnimFile.Delay		 = mymngstuff->delay;

	// Start the whole thing:
	SetTimer( hPicWin, 2, mymngstuff->delay, 0 );
}


//---------------------------------------------------------------------------------------------
// Called when loading a new file or Appquit:
//---------------------------------------------------------------------------------------------
void CleanUpMNG()
{
	KillTimer( hPicWin, 2 );
    mng_cleanup(&Curmng);
	fclose( mymngstuff->file );
	free(mymngstuff);
	delete [] mngdestbuffer;
}


//---------------------------------------------------------------------------------------------
// Called when timer says next frame/layer/update is needed:
//---------------------------------------------------------------------------------------------
void UpdateMNG()
{
	mymngstuff->delay = 0;
	if( MNG_NEEDTIMERWAIT == mng_display_resume(Curmng) )
	{
		KillTimer( hPicWin, 2 );
		SetTimer( hPicWin, 2, mymngstuff->delay, 0 );
	}
	else
	{
		CleanUpMNG();
		AnimFile.CurFrame	 = -1;
		AnimFile.MaxFrame	 = -1;
		AnimFile.isAnimation = -1;
		AnimFile.Delay       = -1;
	}
}



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------------
// MNG WRITING STUFF:
//---------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
int		OffsetX=0,OffsetY=0,OffsetW=0,OffsetH=0;
BYTE	*srcbuffer=0, *tmpbuffer;


//---------------------------------------------------------------------------------------------
// Callback for writing data:
//---------------------------------------------------------------------------------------------
mng_bool mymngwritedata( mng_handle hMNG, mng_ptr pBuf, mng_uint32 iSize, mng_uint32 *iWritten )
{
	mngstuff *pMydata = (mngstuff*)mng_get_userdata(hMNG);
 
	*iWritten = fwrite( pBuf, sizeof(BYTE), iSize, pMydata->file );

	if( *iWritten < iSize )
	{
		Warning( "write error" );
		return MNG_FALSE;
	}

	return MNG_TRUE;
}


//---------------------------------------------------------------------------------------------
// swap Rs and Bs...
//---------------------------------------------------------------------------------------------
BOOL RGBFromBGR( BYTE *buf, UINT widthPix, UINT height )
{
	UINT   col, row;
	LPBYTE pRed, pBlu;
	BYTE   tmp;

    if (buf==NULL)return FALSE;

	INT TmpRow = 0;
	INT WidthBytes = widthPix*3;
	if(WidthBytes & 0x003) WidthBytes = (WidthBytes | 3) + 1;
	INT OurCol = 0;
    for( row=0; row<height; row++ )
	{
		for( col=0; col<widthPix; col++ )
        {
			pRed = buf + TmpRow + OurCol;
            pBlu = pRed + 2;

            // swap red and blue
            tmp = *pBlu;
            *pBlu = *pRed;
            *pRed = tmp;

			OurCol += 3;
		}
		TmpRow += WidthBytes;
		OurCol = 0;
	}
  
	return TRUE;
}


//---------------------------------------------------------------------------------------------
// Creates the srcuffer filled with data for saving:
//---------------------------------------------------------------------------------------------
VOID CreateSrcBuffer( int Frame, int FrameCount, HBITMAP hBmp2 )
{
	PBITMAPINFO		pbmi;
	BITMAP			bmp;
	
	srcbuffer=0;

	// Get WidthBytes...
	INT WidthBytes = W*3;

	INT LineWidth = W*3;
	if(LineWidth & 0x003) LineWidth = (LineWidth | 3) + 1;

	// Bitmapstruct init...
	GetObject( hBmp2, sizeof(BITMAP), (LPSTR)&bmp );
	pbmi = (PBITMAPINFO) LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
   
	// Initialize the fields in the BITMAPINFO structure.
	pbmi->bmiHeader.biSize         = sizeof(BITMAPINFOHEADER); 
	pbmi->bmiHeader.biWidth        = bmp.bmWidth; 
	pbmi->bmiHeader.biHeight       = -bmp.bmHeight; 
	pbmi->bmiHeader.biPlanes       = bmp.bmPlanes; 
	pbmi->bmiHeader.biBitCount     = 24; 
	pbmi->bmiHeader.biCompression  = BI_RGB; 
	pbmi->bmiHeader.biSizeImage    = LineWidth * H * FrameCount; 
	pbmi->bmiHeader.biClrImportant = 0; 

	// Alloc Memory...
	srcbuffer = 0;
	srcbuffer = new BYTE[LineWidth*H*FrameCount];
	if( srcbuffer == 0 )
		Warning( "srcbuffer == 0!" );

	// get the buffer and modify the format:
	if( 0 == GetDIBits( MemDC, hBmp2, 0, (WORD) (H*FrameCount), srcbuffer, pbmi, 0 ) )
		Warning( "no GetDIBits!!!" );
	RGBFromBGR( srcbuffer, W, H*FrameCount );
	if( srcbuffer == 0 )
		Warning( "srcbuffer == 0!" );

	// Freee.
	LocalFree((PBITMAPINFO)pbmi);
}


//---------------------------------------------------------------------------------------------
// Writes a single PNG datastream
//---------------------------------------------------------------------------------------------
VOID WritePNG( mng_handle hMNG, int Frame, int FrameCount )
{
	BYTE			*dstbuffer;
	INT				LineWidth;
	INT				WidthBytes;
	
	OffsetX=0; OffsetY=0; OffsetW=W; OffsetH=H;

	// Get WidthBytes...
	WidthBytes = W*3;
	LineWidth = W*3;
	if(LineWidth & 0x003) LineWidth = (LineWidth | 3) + 1;

	tmpbuffer = new BYTE[(WidthBytes+1)*OffsetH];
	if( tmpbuffer == 0 ) Warning( "Out of Memory!" );

	// Write DEFI chunk.
	mng_putchunk_defi( hMNG, 0, 0, 0, MNG_TRUE, OffsetX, OffsetY, MNG_FALSE, 0, 0, 0, 0 );
 		 
	// Write Header:
	mng_putchunk_ihdr(
		hMNG, 
		OffsetW, OffsetH, 
		MNG_BITDEPTH_8/*iBitdepth*/, 
		MNG_COLORTYPE_RGB/*iColortype*/, 
		MNG_COMPRESSION_DEFLATE/*iCompression*/, 
		MNG_FILTER_ADAPTIVE/*iFilter*/, 
		MNG_INTERLACE_NONE /*iInterlace*/
	);

	// transfer data, add Filterbyte:
	for( int Row=0; Row<OffsetH; Row++ )
	{
		// First Byte in each Scanline is Filterbyte: Currently 0 -> No Filter.
		tmpbuffer[Row*(WidthBytes+1)]=0; 

		// Copy the scanline:
		memcpy( 
			tmpbuffer+Row*(WidthBytes+1)+1, 
			srcbuffer+((OffsetY+Row)*(LineWidth))+OffsetX, 
			WidthBytes
		);
	} 

	// Free srcbuffer if not animated GIF:
	delete [] srcbuffer;

	// Compress data with ZLib (Deflate):
	dstbuffer = new BYTE[(WidthBytes+1)*OffsetH];
	if( dstbuffer == 0 ) Warning( "Out of Memory!" );
	DWORD dstbufferSize=(WidthBytes+1)*OffsetH;

	// Compress data:
	if( Z_OK != compress2( 
		(Bytef *)dstbuffer, (ULONG *)&dstbufferSize,	
		(const Bytef*)tmpbuffer, (ULONG) (WidthBytes+1)*OffsetH, 
		9 
	)) Warning( "Unable to compress imagedata!" );

	// Write Data into MNG File:
	mng_putchunk_idat( hMNG, dstbufferSize, (mng_ptr*)dstbuffer);
	mng_putchunk_iend(hMNG);

	// Free the stuff:
	delete [] tmpbuffer;
	delete [] dstbuffer;
}


//---------------------------------------------------------------------------------------------
// Writes a MNG (24bit)
//---------------------------------------------------------------------------------------------
VOID SaveMNG( LPSTR Filename, HDC hdc, HBITMAP hBmp )
{
	mng_handle	hMNG;

	// check if currently a MNG file is loaded:
	if( AnimFile.isAnimation == 1 )	
		CleanUpMNG();

	// Creates the srcbuffer for imagedata:
	CreateSrcBuffer( 0, 1, hBmp );

	// allocate our stream data structure
	mymngstuff = (mngstuff*)calloc(1, sizeof(*mymngstuff));
	if( mymngstuff == NULL ) 
	{
		Warning( "Cannot allocate data buffer." );
		return;
	}

	// pass the name of the file we want to play
	mymngstuff->filename = Filename;

	// init the lib:
	hMNG = mng_initialize((mng_ptr)mymngstuff, mymngalloc, mymngfree, MNG_NULL);
	if( !hMNG )
	{
	    Warning( "Cannot initialize libmng." );
		return;
	}
	else
	{
		mng_setcb_openstream(hMNG, mymngopenstreamwrite );
		mng_setcb_closestream(hMNG, mymngclosestream);
		mng_setcb_writedata(hMNG, mymngwritedata);

		// Write File:
   		mng_create(hMNG);

		// Just a single Frame (save a normal PNG):
		WritePNG( hMNG, 0, 1 );

		// Now write file:
		mng_write(hMNG);

		// Free the stuff:
		fclose( mymngstuff->file );
		mng_cleanup(&hMNG); 
	}

	free( mymngstuff );
}


