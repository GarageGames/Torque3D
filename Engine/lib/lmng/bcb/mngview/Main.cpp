//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <stdlib.h>  // for : malloc & free
#include "Main.h"

/****************************************************************************}
{*              For conditions of distribution and use,                     *}
{*                 see copyright notice in libmng.pas                       *}
{****************************************************************************}
{*                                                                          *}
{*  project   : libmng                                                      *}
{*  file      : main.pas                  copyright (c) 2000 G.Juyn         *}
{*  version   : 1.0.1                                                       *}
{*                                                                          *}
{*  purpose   : Main form for mngview application                           *}
{*                                                                          *}
{*  author    : G.Juyn                                                      *}
{*  web       : http://www.3-t.com                                          *}
{*  email     : mailto:info@3-t.com                                         *}
{*                                                                          *}
{*  comment   : this is the heart of the mngview applciation                *}
{*                                                                          *}
{*  changes   : This project is a converted version of "mngview" - AP       *}
{*              - AP - 15/9/2000 - revisions ...                            *}
{*              - made the callbacks calling convention explicit            *}
(*              - Moved the defines from "project options" to "main.h"      *}
{*              - Added Readme.txt to the project - Please READ IT !        *}
(*                                                                          *}
{*              0.5.1 - 05/02/2000 - G.Juyn                                 *}
{*              - added this version block                                  *}
{*              - made the initialization part more robust                  *}
{*                eg. program aborts on initialization errors               *}
{*              - B002(105797) - added check for existence of default sRGB  *}
{*                profile (now included in distribution)                    *}
{*              - added mng_cleanup to program exit                         *}
{*              0.5.1 - 05/08/2000 - G.Juyn                                 *}
{*              - changed to stdcall convention                             *}
{*              0.5.1 - 05/11/2000 - G.Juyn                                 *}
{*              - changed callback function declarations                    *}
{*                                                                          *}
{*              0.5.3 - 06/16/2000 - G.Juyn                                 *}
{*              - removed processmessages call from refresh callback        *}
{*              0.5.3 - 06/17/2000 - G.Juyn                                 *}
{*              - switched "storechunks" off                                *}
{*              0.5.3 - 06/26/2000 - G.Juyn                                 *}
{*              - changed definition of userdata to mng_ptr                 *}
{*              0.5.3 - 06/28/2000 - G.Juyn                                 *}
{*              - changed the default icon to something more appropriate    *}
{*              - changed definition of memory alloc size to mng_size_t     *}
{*              0.5.3 - 06/29/2000 - G.Juyn                                 *}
{*              - changed order of refresh parameters                       *}
{*                                                                          *}
{*              0.9.0 - 06/30/2000 - G.Juyn                                 *}
{*              - changed refresh parameters to 'x,y,width,height'          *}
{*                                                                          *}
{*              0.9.1 - 07/08/2000 - G.Juyn                                 *}
{*              - fixed to use returncode constants                         *}
{*              - changed to accomodate MNG_NEEDTIMERWAIT returncode        *}
{*              0.9.1 - 07/10/2000 - G.Juyn                                 *}
{*              - changed to use suspension-mode                            *}
{*                                                                          *}
{*              1.0.1 - 05/02/2000 - G.Juyn                                 *}
{*              - removed loading of default sRGB profile (auto in libmng)  *}
{*                                                                          *}
{****************************************************************************/

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

# define _OR_     |
# define _AND_    &
# define _DIV_    /
# define _NOT_    !
# define _NIL_    0
# define _SHR_    >>
# define _SHL_    <<

// Prototypes for static functions - the LibMng Callbacks.
static mng_ptr __stdcall Memalloc( mng_uint32 iLen );
static void __stdcall Memfree( mng_ptr iPtr, mng_size_t iLen );
static mng_bool __stdcall Openstream( mng_handle hHandle );
static mng_bool __stdcall Closestream( mng_handle hHandle );
static mng_bool __stdcall Readdata ( mng_handle hHandle, mng_ptr pBuf,
                              mng_uint32 iBuflen, mng_uint32 *pRead );
static mng_bool __stdcall ProcessHeader ( mng_handle hHandle,
                                mng_uint32 iWidth, mng_uint32 iHeight );
static mng_ptr __stdcall GetCanvasLine ( mng_handle hHandle,
                                                    mng_uint32 iLinenr );
static mng_ptr __stdcall GetAlphaLine( mng_handle hHandle, mng_uint32 iLinenr );
static mng_bool __stdcall ImageRefresh ( mng_handle hHandle,
  mng_uint32 iX, mng_uint32 iY, mng_uint32 iWidth, mng_uint32 iHeight );
static mng_uint32 __stdcall GetTickCount( mng_handle hHandle );
static mng_bool __stdcall SetTimer( mng_handle hHandle, mng_uint32 iMsecs );

//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
  : TForm(Owner)
{
}
//---------------------------------------------------------------------------
static mng_ptr __stdcall Memalloc( mng_uint32 iLen )
{
mng_ptr pResult =

  malloc( iLen );             /* get memory from the heap */

  if( pResult )                 /* Added - condition */
    memset( pResult, 0, iLen );

  return pResult;
}
//---------------------------------------------------------------------------
static void __stdcall Memfree( mng_ptr iPtr, mng_size_t iLen )
{
  free( iPtr );   /* free the memory */
  (void)iLen;     // Kill compiler warning
}
//---------------------------------------------------------------------------
static mng_bool __stdcall Openstream( mng_handle hHandle )
{
TMainForm  *OHForm;

  /* get a fix on our form */
  OHForm = (TMainForm *)mng_get_userdata( hHandle );

  if( OHForm->OFFile != _NIL_ )   /* free previous stream (if any) */
    OHForm->OFFile->Free();

  /* open a new stream */
  OHForm->OFFile = new TFileStream(
            OHForm->SFFileName, fmOpenRead _OR_ fmShareDenyWrite);

  OHForm->ProgressBar1->Position = 0;               /* Added */
  OHForm->ProgressBar1->Min =0;                     /* Added */
  OHForm->ProgressBar1->Max = OHForm->OFFile->Size; /* Added */

  return MNG_TRUE;
}
//---------------------------------------------------------------------------
static mng_bool __stdcall Closestream( mng_handle hHandle )
{
TMainForm  *OHForm;

  /* get a fix on our form */
  OHForm = (TMainForm *)mng_get_userdata( hHandle );

  OHForm->OFFile->Free();             /* cleanup the stream */
  OHForm->OFFile = 0;                 /* don't use it again ! */

  OHForm->ProgressBar1->Position = 0; /* Added */

  return MNG_TRUE;
}
//---------------------------------------------------------------------------
static mng_bool __stdcall Readdata ( mng_handle hHandle, mng_ptr pBuf,
                              mng_uint32 iBuflen, mng_uint32 *pRead )
{
TMainForm     *OHForm;
unsigned int  IHTicks;
unsigned int  IHByte1;
unsigned int  IHByte2;
unsigned int  IHBytesPerSec ;

  /* get a fix on our form */
  OHForm = (TMainForm *)mng_get_userdata( hHandle );

  /* are we at EOF ? */
  if( OHForm->OFFile->Position >= OHForm->OFFile->Size )
  {
    *pRead = 0;                      /* indicate so */
  }
  else
  {
    IHBytesPerSec = OHForm->IFBytesPerSec;  /* fake a slow connection */

    if( IHBytesPerSec > 0 )
    {
      IHTicks       = (unsigned int)GetTickCount();
      IHByte1       = (IHTicks - OHForm->IFTicks) * IHBytesPerSec;
      IHByte2       = (OHForm->IFBytes + iBuflen) * 1000;

      if( IHByte2 > IHByte1 ) /* Added - condition */
        if( ((IHByte2 - IHByte1) _DIV_ IHBytesPerSec) > 10 )
        {
          Sleep( (DWORD)((IHByte2 - IHByte1) _DIV_ IHBytesPerSec) );
        }
    };

    /* read the requested data */
    *pRead   = OHForm->OFFile->Read( pBuf, iBuflen);
    OHForm->IFBytes = OHForm->IFBytes + *pRead;

    OHForm->ProgressBar1->Position = (int)OHForm->IFBytes;  /* Added */
  } // end else;

  return MNG_TRUE;
}
//---------------------------------------------------------------------------
static mng_bool __stdcall ProcessHeader ( mng_handle hHandle,
                                mng_uint32 iWidth, mng_uint32 iHeight )
{
TMainForm  *OHForm;

  /* get a fix on our form */
  OHForm = (TMainForm *)mng_get_userdata( hHandle );

  /* Added */
  OHForm->Caption = ExtractFileName( OHForm->SFFileName ) +
                    " [" +
                    String( iWidth ) +
                    "x" +
                    String( iHeight ) +
                    "]";

  OHForm->OFBitmap->Width  = iWidth;         /* store the new dimensions */
  OHForm->OFBitmap->Height = iHeight;
  OHForm->OFImage->Left    = 0;              /* adjust the visible component */
  OHForm->OFImage->Top     = 0;
  OHForm->OFImage->Width   = iWidth;
  OHForm->OFImage->Height  = iHeight;

  OHForm->FormResize (OHForm);               /* force re-centering the image*/
                                     /* clear the canvas & draw an outline */
  OHForm->OFBitmap->Canvas->Brush->Color = clGray;
  OHForm->OFBitmap->Canvas->Brush->Style = bsSolid;
  OHForm->OFBitmap->Canvas->FillRect( OHForm->OFBitmap->Canvas->ClipRect );
  OHForm->OFBitmap->Canvas->Brush->Color = clRed;
  OHForm->OFBitmap->Canvas->Brush->Style = bsSolid;
  OHForm->OFBitmap->Canvas->Pen->Color   = clRed;
  OHForm->OFBitmap->Canvas->Pen->Style   = psSolid;
  OHForm->OFBitmap->Canvas->FrameRect( OHForm->OFBitmap->Canvas->ClipRect);

  /* make sure it gets out there */
  OHForm->OFImage->Picture->Assign( OHForm->OFBitmap );

  /* tell the library we want funny windows-bgr*/
  if( mng_set_canvasstyle( hHandle, MNG_CANVAS_BGR8 ) )
    OHForm->MNGerror( "libmng reported an error setting the canvas style" );

  return MNG_TRUE;
}
//---------------------------------------------------------------------------
static mng_ptr __stdcall GetCanvasLine ( mng_handle hHandle,
                                                    mng_uint32 iLinenr )
{
TMainForm  *OHForm;

  /* get a fix on our form */
  OHForm = (TMainForm *)mng_get_userdata( hHandle );

  /* easy with these bitmap objects ! */
  return OHForm->OFBitmap->ScanLine[ iLinenr ];
}
//---------------------------------------------------------------------------
static mng_bool __stdcall ImageRefresh ( mng_handle hHandle,
  mng_uint32 iX, mng_uint32 iY, mng_uint32 iWidth, mng_uint32 iHeight )
{
TMainForm  *OHForm;

  /* get a fix on our form */
  OHForm = (TMainForm *)mng_get_userdata( hHandle );

  /* force redraw */
  OHForm->OFImage->Picture->Assign( OHForm->OFBitmap );

  return MNG_TRUE;
}
//---------------------------------------------------------------------------
static mng_uint32 __stdcall GetTickCount( mng_handle hHandle )
{
  return GetTickCount();      /* windows knows that */
}
//---------------------------------------------------------------------------
static mng_bool __stdcall SetTimer( mng_handle hHandle, mng_uint32 iMsecs )
{
TMainForm  *OHForm;

  /* get a fix on our form */
  OHForm = (TMainForm *)mng_get_userdata( hHandle );
  OHForm->OFTimer->Interval = iMsecs;   /* and set the timer */
  OHForm->OFTimer->Enabled  = true;

  return MNG_TRUE;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
String          SHProfileName;
mng_uint16      IHRed, IHGreen, IHBlue; /* word */

  OFBitmap      = new Graphics::TBitmap();     /* initialize */
  IFBytesPerSec = 10000000;
  OFFile        = 0;

  OFOpenDialog->InitialDir = "";
  OFBitmap->HandleType     = bmDIB;    /* make it a 24-bit DIB */
  OFBitmap->PixelFormat    = pf24bit;

  /* now initialize the library */
  IFHandle = mng_initialize( mng_ptr(this), Memalloc, Memfree, _NIL_ );

  if( IFHandle == _NIL_ )
  {
    MNGerror ("libmng initializiation error"\
              "\n"\
              "Program aborted"
    );
    PostMessage( Handle, WM_CLOSE, 0, 0);
    return; // was Exit
  };

  /* no need to store chunk-info ! */
  mng_set_storechunks( IFHandle, MNG_FALSE );

  /* use suspension-buffer */
  mng_set_suspensionmode( IFHandle, MNG_TRUE );

  /* set all the callbacks */
  if(
     (mng_setcb_openstream   (IFHandle, Openstream   ) != MNG_NOERROR) _OR_
     (mng_setcb_closestream  (IFHandle, Closestream  ) != MNG_NOERROR) _OR_
     (mng_setcb_readdata     (IFHandle, Readdata     ) != MNG_NOERROR) _OR_
     (mng_setcb_processheader(IFHandle, ProcessHeader) != MNG_NOERROR) _OR_
     (mng_setcb_getcanvasline(IFHandle, GetCanvasLine) != MNG_NOERROR) _OR_
     (mng_setcb_refresh      (IFHandle, ImageRefresh ) != MNG_NOERROR) _OR_
     (mng_setcb_gettickcount (IFHandle, GetTickCount ) != MNG_NOERROR) _OR_
     (mng_setcb_settimer     (IFHandle, SetTimer     ) != MNG_NOERROR)
    )
  {
    MNGerror ("libmng reported an error setting a callback function!"\
              "\n"\
              "Program aborted"
    );
    PostMessage( Handle, WM_CLOSE, 0, 0 );
    return; // was Exit
  };

  /* supply our own bg-color */
  IHRed   = (mng_uint16)((Color         ) _AND_ 0xFF);
  IHGreen = (mng_uint16)((Color _SHR_  8) _AND_ 0xFF);
  IHBlue  = (mng_uint16)((Color _SHR_ 16) _AND_ 0xFF);

  IHRed   = (mng_uint16)((IHRed   _SHL_ 8) + IHRed);
  IHGreen = (mng_uint16)((IHGreen _SHL_ 8) + IHGreen);
  IHBlue  = (mng_uint16)((IHBlue  _SHL_ 8) + IHBlue);

  if( mng_set_bgcolor (IFHandle, IHRed, IHGreen, IHBlue) != MNG_NOERROR )
    MNGerror( "libmng reported an error setting the background color!");
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormCloseQuery(TObject *Sender,
      bool &CanClose)
{
  BFCancelled = true;

  /* if we're still animating then stop it */
  if( OFTimer->Enabled )
  {
    if( mng_display_freeze (IFHandle) != MNG_NOERROR )
      MNGerror ("libmng reported an error during display_freeze!" );
  }

  OFTimer->Enabled = false;

  mng_cleanup( &IFHandle );

}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
  FormResize( this );
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormResize(TObject *Sender)
{
  /* center the image in the window */

  if( ClientWidth  < OFImage->Width )
    OFImage->Left = 0;
  else
    OFImage->Left = (ClientWidth  - OFImage->Width ) _DIV_ 2;

  if( ClientHeight < OFImage->Height )
    OFImage->Top  = 0;
  else
    OFImage->Top  = (ClientHeight - OFImage->Height) _DIV_ 2;

  ProgressBar1->Width = Panel1->Width - 8;  /* Added */

}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  /* pressing <esc> will freeze an animation */
  if( Key == VK_ESCAPE )
  {
    if( OFTimer->Enabled )
    {
      if( mng_display_freeze( IFHandle) != MNG_NOERROR )
        MNGerror( "libmng reported an error during display_freeze!" );
    }

    OFTimer->Enabled = false;  /* don't let that timer go off then ! */
    BFCancelled     = true;
  }

}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OFTimerTimer(TObject *Sender)
{
mng_retcode IHRslt;

  OFTimer->Enabled = false;            /* only once ! */

  if( _NOT_ BFCancelled )
  {
    /* and inform the library */
    IHRslt = mng_display_resume( IFHandle );

    if( (IHRslt != MNG_NOERROR) _AND_ (IHRslt != MNG_NEEDTIMERWAIT) )
      MNGerror( "libmng reported an error during display_resume!" );

  };

}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OFMenuFileOpenClick(TObject *Sender)
{
mng_retcode   IHRslt;

  OFOpenDialog->InitialDir = "";
OFOpenDialog->InitialDir = GetCurrentDir(); //@@
  OFOpenDialog->FileName   = SFFileName;

  if( OFOpenDialog->Execute() )  /* get the filename */
  {
    if( OFTimer->Enabled )        /* if the lib was active; stop it */
    {
      OFTimer->Enabled = false;

      Application->ProcessMessages(); /* process any timer requests (for safety) */
                                      /* now freeze the animation */
      if( mng_display_freeze( IFHandle ) != MNG_NOERROR )
        MNGerror( "libmng reported an error during display_freeze!" );
    };

    /* save interesting fields */
    SFFileName      = OFOpenDialog->FileName;
    IFTicks         = GetTickCount();
    IFBytes         = 0;
    BFCancelled     = false;

    /* always reset (just in case) */
    if( mng_reset( IFHandle ) != MNG_NOERROR )
    {
      MNGerror( "libmng reported an error during reset!" );
    }
    else
    {
      /* and let the lib do it's job ! */
      IHRslt = mng_readdisplay (IFHandle);

      if( (IHRslt != MNG_NOERROR) _AND_ (IHRslt != MNG_NEEDTIMERWAIT) )
        MNGerror( "libmng reported an error reading the input file!" );

    };
  };
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OFMenuFileProfileClick(TObject *Sender)
{
char    SHProfileDir[ MAX_PATH ];

  GetSystemDirectory( SHProfileDir, MAX_PATH );
  strcat( SHProfileDir, "\\Color" );

  OFOpenDialogProfile->InitialDir = String( SHProfileDir );

  if( OFOpenDialogProfile->Execute() )
  {
    if( mng_set_outputprofile( IFHandle, OFOpenDialogProfile->FileName.c_str()) != 0 )
      MNGerror( "libmng reported an error setting the output-profile!" );
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OFMenuFileExitClick(TObject *Sender)
{
  if( mng_cleanup( &IFHandle ) != MNG_NOERROR )
    MNGerror( "libmng cleanup error" );

  Close();

}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OFMenuOptionsModemSpeedClick(TObject *Sender)
{

  OFMenuOptionsModem28k8->Checked      = false;
  OFMenuOptionsModem33k6->Checked      = false;
  OFMenuOptionsModem56k->Checked       = false;
  OFMenuOptionsModemISDN64->Checked    = false;
  OFMenuOptionsModemISDN128->Checked   = false;
  OFMenuOptionsModemCable512->Checked  = false;
  OFMenuOptionsModemUnlimited->Checked = false;

  if( IFBytesPerSec == (unsigned int)OFMenuOptionsModem28k8->Tag   _DIV_ 10 )
    OFMenuOptionsModem28k8->Checked      = true;
  else
  if( IFBytesPerSec == (unsigned int)OFMenuOptionsModem33k6->Tag   _DIV_ 10 )
    OFMenuOptionsModem33k6->Checked      = true;
  else
  if( IFBytesPerSec == (unsigned int)OFMenuOptionsModem56k->Tag    _DIV_ 10 )
    OFMenuOptionsModem56k->Checked       = true;
  else
  if( IFBytesPerSec == (unsigned int)OFMenuOptionsModemISDN64->Tag _DIV_ 10 )
    OFMenuOptionsModemISDN64->Checked    = true;
  else
  if( IFBytesPerSec == (unsigned int)OFMenuOptionsModemISDN128->Tag _DIV_ 10 )
    OFMenuOptionsModemISDN128->Checked   = true;
  else
  /* Added - changedit was the line below ! */
//  if( IFBytesPerSec == (unsigned int)OFMenuOptionsModemUnlimited->Tag _DIV_ 10 )
  if( IFBytesPerSec == (unsigned int)OFMenuOptionsModemCable512->Tag _DIV_ 10 )
    OFMenuOptionsModemCable512->Checked  = true;
  else
    OFMenuOptionsModemUnlimited->Checked = true;

}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OFMenuOptionsModemXClick(TObject *Sender)
{
  IFBytesPerSec = ((TMenuItem*)Sender)->Tag _DIV_ 10;
}
//---------------------------------------------------------------------------
void TMainForm::MNGerror( String SHMsg )
{
/* get extended info */
mng_uint32  iErrorcode;
mng_uint8   iSeverity;
mng_chunkid iChunkname;
mng_uint32  iChunkseq;
mng_int32   iExtra1;
mng_int32   iExtra2;
mng_pchar   zErrortext;
char        szFormatStr[ 256 ];

  iErrorcode = mng_getlasterror (IFHandle, &iSeverity,
                  &iChunkname, &iChunkseq, &iExtra1, &iExtra2,
                  (mng_pchar*)&zErrortext);

  wsprintf( szFormatStr,
    "Error = %d; Severity = %d; Chunknr = %d; Extra1 = %d",
    (int)iErrorcode, (int)iSeverity, (int)iChunkseq, (int)iExtra1
  );

  MessageDlg( SHMsg +
              "\n\n" +
              String(zErrortext) +
              "\n\n" +
              szFormatStr,  /* see wsprintf above */
              mtError,
              TMsgDlgButtons() << mbOK,
              0
  );

}
//---------------------------------------------------------------------------

