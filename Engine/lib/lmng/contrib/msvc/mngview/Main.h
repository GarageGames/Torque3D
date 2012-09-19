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

#ifndef _MAIN_H
#define _MAIN_H


//---------------------------------------------------------------------------------------------
// Includes:
//---------------------------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <commctrl.h>
#include "Resource.h"


//---------------------------------------------------------------------------------------------
// Defines:
//---------------------------------------------------------------------------------------------
#define TITLE "MNGView Sample Application 1.0"


//---------------------------------------------------------------------------------------------
// global Vars:
//---------------------------------------------------------------------------------------------
extern HWND			hPicWin;
extern HINSTANCE	hinst;
extern HDC			MemDC, hdc;
extern HBITMAP		MemImage, DefaultMemImage;
extern int			W, H, Bits;


typedef struct
{
	int			MaxFrame;
	int			CurFrame;
	int			Delay;
	int			isAnimation;
} ANIMFILE;
extern ANIMFILE	AnimFile;


//---------------------------------------------------------------------------------------------
// Function prototypes:
//---------------------------------------------------------------------------------------------
BOOL Error( const char *err );
BOOL Warning( const char *err );
VOID catpath( char *dst, const char *src );


// MNG.cpp specific:
VOID LoadMNG( LPSTR Filename, HWND hwnd, HDC hdc );
VOID SaveMNG( LPSTR Filename, HDC hdc, HBITMAP hBmp );

VOID UpdateMNG();
VOID CleanUpMNG();


#endif