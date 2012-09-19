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


//---------------------------------------------------------------------------------------------
// Libs (its up to you to make VC find the libs; set the paths in the options):
//---------------------------------------------------------------------------------------------
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"libmng.lib")
#pragma comment(lib,"libjpeg.lib")
#pragma comment(lib,"libz.lib")
#pragma comment(lib,"lcmsstat.lib")


//---------------------------------------------------------------------------------------------
// Vars:
//---------------------------------------------------------------------------------------------
HWND			hPicWin;
HINSTANCE		hinst;
HMENU			hMenu;
HDC				MemDC, hdc;
HBITMAP			MemImage, DefaultMemImage;
ANIMFILE		AnimFile;
RECT			rcRect;
OPENFILENAME	ofn, sfn;
int				W, H, Bits;
int				dx, dy;
char			OFNFile[1024];
char			CurDir[1024];


//---------------------------------------------------------------------------------------------
// Loads a file:
//---------------------------------------------------------------------------------------------
VOID LoadOFN( HWND hwnd )
{
	GetCurrentDirectory( sizeof(CurDir), CurDir );

	ofn.lStructSize     = sizeof(OPENFILENAME);
	ofn.hwndOwner       = hwnd;
	ofn.lpstrFile       = OFNFile;
	ofn.nMaxFile        = sizeof(OFNFile);
	ofn.lpstrFilter     = "Network Graphics (*.mng;*.jng:*.png)\0*.mng;*.jng;*.png\0";
	ofn.nFilterIndex    = 1;
	ofn.lpstrFileTitle  = NULL;
	ofn.nMaxFileTitle   = NULL;
	ofn.lpstrInitialDir = CurDir;
	ofn.lpstrTitle      = "Load:"; 
	ofn.Flags           = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
						  OFN_HIDEREADONLY | OFN_NONETWORKBUTTON;

	// Display the Open dialog box. 
	if( GetOpenFileName(&ofn) )
	{
		dx = dy = W = H = 0;
		if( AnimFile.isAnimation == 1 ) CleanUpMNG();
		LoadMNG( OFNFile, hwnd, hdc );
	} 
}


//---------------------------------------------------------------------------------------------
// Saves a file:
//---------------------------------------------------------------------------------------------
VOID SaveSFN( HWND hwnd )
{
	GetCurrentDirectory( sizeof(CurDir), CurDir );

	// Initialize OPENFILENAME
	sfn.lStructSize     = sizeof(OPENFILENAME);
	sfn.hwndOwner       = hwnd;
	sfn.lpstrFile       = OFNFile;
	sfn.nMaxFile        = sizeof(OFNFile);
	sfn.lpstrFilter     = "PNG\0*.png\0";
	sfn.nFilterIndex    = 1;
	sfn.lpstrFileTitle  = 0;
	sfn.nMaxFileTitle   = 0;
	sfn.lpstrInitialDir = CurDir;
	sfn.lpstrTitle      = "Save an Image"; 
	sfn.Flags           = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON;
	
	// Display the Open dialog box.
	if( GetSaveFileName(&sfn) )
	{
		SaveMNG( OFNFile, MemDC, MemImage );
	}
}


//---------------------------------------------------------------------------------------------
// For stringhandling...
//---------------------------------------------------------------------------------------------
VOID catpath( char *dst, const char *src )
{
    int len = lstrlen(dst);
    if( len > 0 && (dst[len-1] != '\\' && dst[len-1] != '/') ) lstrcat( dst, "\\" );
    lstrcat( dst, src );
}


//---------------------------------------------------------------------------------------------
// MainWindow WindowProc
//---------------------------------------------------------------------------------------------
long WINAPI WindowProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		case WM_TIMER:
		{
			int wTimerID = wParam;
			if( AnimFile.isAnimation == 1 && wTimerID == 2 )
			{
				UpdateMNG();
				SendMessage( hPicWin, WM_PAINT, 0, 0 );
			}
		}
		break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
				case FILE_OPEN:
					LoadOFN(hwnd);
					break;

				case FILE_SAVE:
					SaveSFN(hwnd);
					break;

				case FILE_EXIT:
					DestroyWindow( hPicWin );
					break;

				case HELP_ABOUT:
					Warning( 
						"MNGView Sample Application for VC6.\n" \
						"This Code is Public Domain.\n" \
						"Created by Nikolaus Brennig."
					);
					break;
			}
			break;

	    case WM_ERASEBKGND:
			return 0L;

        case WM_PAINT:
			// GetDC:
			GetClientRect( hPicWin, &rcRect );
			hdc = GetDC( hPicWin );

			if( MemDC == 0 ) 
			{
				BitBlt( hdc, 0, 0, rcRect.right, rcRect.bottom, MemDC, 0, 0, BLACKNESS );
				ReleaseDC( hPicWin, hdc );
				break;
			}

			// Erase:
			// Upper area...
			BitBlt( hdc, 0, 0, rcRect.right, (0+dy), MemDC, 0, 0, BLACKNESS );
			// Lower area...
			BitBlt( hdc, 0, (0+dy)+H, rcRect.right, rcRect.bottom - ((0+dy)+H), MemDC, 0, 0, BLACKNESS );
			// Left area...
			BitBlt( hdc, 0, 0, (0+dx), rcRect.bottom, MemDC, 0, 0, BLACKNESS );
			// Right area...
			BitBlt( hdc, (0+dx)+W, 0, rcRect.right, rcRect.bottom, MemDC, 0, 0, BLACKNESS );
	
			// Show Imageframe:
			BitBlt( hdc, dx, dy, W, H, MemDC, 0, 0, SRCCOPY );

			// Release DC...
			ReleaseDC( hPicWin, hdc );
            break;

		case WM_HSCROLL:
		{
			int nScrollCode = (int) LOWORD(wParam);  // scroll bar value 
			if( nScrollCode == SB_LINELEFT	) dx += 10;
			if( nScrollCode == SB_LINERIGHT ) dx -= 10;
			SendMessage( hwnd, WM_PAINT, 0, 0 );
		}
		break;

		case WM_VSCROLL:
		{
			int nScrollCode = (int) LOWORD(wParam);  // scroll bar value 
			if( nScrollCode == SB_LINEUP	) dy += 10;
			if( nScrollCode == SB_LINEDOWN  ) dy -= 10;
			SendMessage( hwnd, WM_PAINT, 0, 0 );
		}
		break;

        case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 550;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 450;
            return 0L;

		case WM_DESTROY:
			if( AnimFile.isAnimation == 1 ) 
				CleanUpMNG();
			if( hMenu ) DestroyMenu( hMenu );
            PostQuitMessage(0);
            return 0L;
	}
    
	return DefWindowProc( hwnd, message, wParam, lParam );
}


//---------------------------------------------------------------------------------------------
// ok, initen wir mal das Window mit den Styleparametern...
//---------------------------------------------------------------------------------------------
BOOL InitApplication( HINSTANCE hInstance, int nCmdShow, LPSTR lpCommandLine )
{
	WNDCLASSEX          wcex;

	ZeroMemory( &wcex, sizeof(wcex) );
	wcex.cbSize             = sizeof(wcex);
	wcex.style              = 0;
	wcex.lpfnWndProc        = WindowProc;
	wcex.cbClsExtra         = 0;
	wcex.cbWndExtra         = 0;
	wcex.hInstance          = hInstance;
	wcex.hIcon              = LoadIcon(hInstance, MAKEINTRESOURCE(APPICON));
	wcex.hCursor            = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground      = HBRUSH(GetStockObject(BLACK_BRUSH));
	wcex.lpszMenuName       = 0;
	wcex.lpszClassName      = "MNGViewClass";
	wcex.hIconSm            = 0;
	
	if( !RegisterClassEx(&wcex) ) 
		return Error( "RegisterClass failed!" );

	// Init:
	W = 0;
	H = 0;
	AnimFile.isAnimation = 0;
	MemDC = 0;
	MemImage = 0;
	DefaultMemImage = 0;

    // Create the Window:
    hPicWin = CreateWindowEx( 
		0, 
		"MNGViewClass", 
		TITLE, 
		WS_OVERLAPPEDWINDOW|WS_SYSMENU|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_HSCROLL|WS_VSCROLL, 
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		NULL, NULL, 
		hInstance, 
		NULL 
	);
    if( !hPicWin ) 
		return Error( "Couldn't create Window!" );

	// Load our menu...
    hMenu = LoadMenu( GetModuleHandle(NULL), MAKEINTRESOURCE(THEMENU) );
	if( hMenu == 0 ) Warning( "Unable to load Menu!" );
	SetMenu( hPicWin, hMenu );

    InitCommonControls();

	UpdateWindow(hPicWin);
	ShowWindow(hPicWin, SW_NORMAL);

	return true;   
}


//---------------------------------------------------------------------------------------------
// Ok. Das ist die Startfunktion, die wird als erstes von Windows augerufen...
//---------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	MSG     msg;

	hinst = hInstance;

	if( !InitApplication(hInstance, nCmdShow, lpCmdLine) )
		return -1;

	while( GetMessage(&msg, NULL, 0, 0) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg); 
	}
 
	return msg.wParam;  
}


//---------------------------------------------------------------------------------------------
// Error
//---------------------------------------------------------------------------------------------
BOOL Error( const char *err )
{
	MessageBox( hPicWin, err, TITLE, MB_ICONHAND+MB_OK );
	DestroyWindow( hPicWin );

 	return FALSE;
}


//---------------------------------------------------------------------------------------------
// Warning
//---------------------------------------------------------------------------------------------
BOOL Warning( const char *err )
{
	MessageBox( hPicWin, err, TITLE, MB_ICONHAND+MB_OK );

	return FALSE;
} 

