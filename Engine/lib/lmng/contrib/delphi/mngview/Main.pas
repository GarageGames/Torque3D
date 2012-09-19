unit Main;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  Menus, StdCtrls, ExtCtrls, Buttons,
  libmng;

{****************************************************************************}
{*              For conditions of distribution and use,                     *}
{*                 see copyright notice in libmng.pas                       *}
{****************************************************************************}
{*                                                                          *}
{*  project   : libmng                                                      *}
{*  file      : main.pas                  copyright (c) 2000-2002 G.Juyn    *}
{*  version   : 1.0.5                                                       *}
{*                                                                          *}
{*  purpose   : Main form for mngview application                           *}
{*                                                                          *}
{*  author    : G.Juyn                                                      *}
{*  web       : http://www.3-t.com                                          *}
{*  email     : mailto:info@3-t.com                                         *}
{*                                                                          *}
{*  comment   : this is the heart of the mngview applciation                *}
{*                                                                          *}
{*  changes   : 0.5.1 - 05/02/2000 - G.Juyn                                 *}
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
{*              0.9.3 - 09/11/2000 - G.Juyn                                 *}
{*              - removed some tesst-stuff                                  *}
{*                                                                          *}
{*              1.0.1 - 05/02/2000 - G.Juyn                                 *}
{*              - removed loading default sRGB profile (auto in libmng)     *}
{*                                                                          *}
{*              1.0.5 - 09/16/2002 - G.Juyn                                 *}
{*              - added dynamic MNG features                                *}
{*              1.0.5 - 11/27/2002 - G.Juyn                                 *}
{*              - fixed freeze during read-cycle                            *}
{*                                                                          *}
{****************************************************************************}

type
  TMainForm = class(TForm)

    OFMainMenu: TMainMenu;
    OFMenuFile: TMenuItem;
    OFMenuFileOpen: TMenuItem;
    OFMenuFileProfile: TMenuItem;
    OFMenuFileN1: TMenuItem;
    OFMenuFileExit: TMenuItem;

    OFMenuOptions: TMenuItem;
    OFMenuOptionsModemSpeed: TMenuItem;
    OFMenuOptionsModem28k8: TMenuItem;
    OFMenuOptionsModem33k6: TMenuItem;
    OFMenuOptionsModem56k: TMenuItem;
    OFMenuOptionsModemISDN64: TMenuItem;
    OFMenuOptionsModemISDN128: TMenuItem;
    OFMenuOptionsModemCable512: TMenuItem;
    OFMenuOptionsModemUnlimited: TMenuItem;

    OFTimer: TTimer;

    OFOpenDialog: TOpenDialog;
    OFOpenDialogProfile: TOpenDialog;

    OFImage: TImage;

    procedure FormCreate(Sender: TObject);
    procedure FormCloseQuery(Sender: TObject; var CanClose: Boolean);
    procedure FormShow(Sender: TObject);
    procedure FormResize(Sender: TObject);
    procedure FormKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure FormMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);

    procedure OFImageMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure OFImageMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure OFImageMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);

    procedure OFTimerTimer(Sender: TObject);

    procedure OFMenuFileOpenClick(Sender: TObject);
    procedure OFMenuFileProfileClick(Sender: TObject);
    procedure OFMenuFileExitClick(Sender: TObject);

    procedure OFMenuOptionsModemSpeedClick(Sender: TObject);
    procedure OFMenuOptionsModemXClick(Sender: TObject);

  private
    { Private declarations }

    SFFileName    : string;            { filename of the input stream }
    OFFile        : TFileStream;       { input stream }
    IFHandle      : mng_handle;        { the libray handle }
    OFBitmap      : TBitmap;           { drawing canvas }
    BFCancelled   : boolean;           { <esc> or app-exit }
    BFHasMouse    : boolean;           { mouse is/was over image }

    IFTicks       : cardinal;          { used to fake slow connections }
    IFBytes       : cardinal;
    IFBytesPerSec : integer;

    procedure MNGerror (SHMsg : string);

  public
    { Public declarations }

  end;

var
  MainForm: TMainForm;

{****************************************************************************}

implementation

{$R *.DFM}

{****************************************************************************}

{$F+}
function  Memalloc (iLen : mng_uint32) : mng_ptr; stdcall;
{$F-}
begin
  getmem   (Result, iLen);             { get memory from the heap }
  fillchar (Result^, iLen, 0);         { and initialize it }
end;

{****************************************************************************}

{$F+}
procedure Memfree (iPtr : mng_ptr;
                   iLen : mng_size_t); stdcall;
{$F-}
begin
  freemem (iPtr, iLen);                { free the memory }
end;

{****************************************************************************}

{$F+}
function Openstream (hHandle : mng_handle) : mng_bool; stdcall;
{$F-}

var OHForm : TMainForm;

begin                                  { get a fix on our form }
  OHForm := TMainForm (mng_get_userdata (hHandle));

  with OHFORM do
  begin
    if OFFile <> nil then              { free previous stream (if any) }
      OFFile.Free;
                                       { open a new stream }
    OFFile := TFileStream.Create (SFFileName, fmOpenRead or fmShareDenyWrite);
  end;

  Result := MNG_TRUE;  
end;

{****************************************************************************}

{$F+}
function Closestream (hHandle : mng_handle) : mng_bool; stdcall;
{$F-}

var OHForm : TMainForm;

begin                                  { get a fix on our form }
  OHForm := TMainForm (mng_get_userdata (hHandle));

  with OHFORM do
  begin
    OFFile.Free;                       { cleanup the stream }
    OFFile := nil;                     { don't use it again ! }
  end;

  Result := MNG_TRUE;
end;

{****************************************************************************}

{$F+}
function Readdata (    hHandle : mng_handle;
                       pBuf    : mng_ptr;
                       iBuflen : mng_uint32;
                   var pRead   : mng_uint32) : mng_bool; stdcall;
{$F-}

var OHForm        : TMainForm;
    IHTicks       : cardinal;
    IHByte1       : cardinal;
    IHByte2       : cardinal;
    IHBytesPerSec : cardinal;

begin
                                       { get a fix on our form }
  OHForm := TMainForm (mng_get_userdata (hHandle));

  with OHForm do
  begin                                { are we at EOF ? }
    if OFFile.Position >= OFFile.Size then
    begin
      pRead := 0;                      { indicate so }
    end
    else
    begin
      IHBytesPerSec := IFBytesPerSec;  { fake a slow connection }

      if IHBytesPerSec > 0 then
      begin
        IHTicks       := Windows.GetTickCount;
        IHByte1       := round (((IHTicks - IFTicks) / 1000) * IHBytesPerSec);
        IHByte2       := (IFBytes + iBuflen);

        if ((IHByte2 - IHByte1) div IHBytesPerSec) > 10 then
          Windows.Sleep ((IHByte2 - IHByte1) div IHBytesPerSec);

      end;
                                       { read the requested data }
      pRead   := OFFile.Read (pBuf^, iBuflen);
      IFBytes := IFBytes + pRead;
    end;
  end;

  Result := MNG_TRUE;  
end;

{****************************************************************************}

{$F+}
function ProcessHeader (hHandle : mng_handle;
                        iWidth  : mng_uint32;
                        iHeight : mng_uint32) : mng_bool; stdcall;
{$F-}

var OHForm : TMainForm;

begin                                  { get a fix on our form }
  OHForm := TMainForm (mng_get_userdata (hHandle));

  with OHForm do
  begin
    OFBitmap.Width  := iWidth;         { store the new dimensions }
    OFBitmap.Height := iHeight;
    OFImage.Left    := 0;              { adjust the visible component }
    OFImage.Top     := 0;
    OFImage.Width   := iWidth;
    OFImage.Height  := iHeight;

    FormResize (OHForm);               { force re-centering the image}
                                       { clear the canvas & draw an outline }
    OFBitmap.Canvas.Brush.Color := clGray;
    OFBitmap.Canvas.Brush.Style := bsSolid;
    OFBitmap.Canvas.FillRect  (OFBitmap.Canvas.ClipRect);
    OFBitmap.Canvas.Brush.Color := clRed;
    OFBitmap.Canvas.Brush.Style := bsSolid;
    OFBitmap.Canvas.Pen.Color   := clRed;
    OFBitmap.Canvas.Pen.Style   := psSolid;
    OFBitmap.Canvas.FrameRect (OFBitmap.Canvas.ClipRect);

    OFImage.Picture.Assign (OFBitmap); { make sure it gets out there }
                                       { tell the library we want funny windows-bgr}
    if mng_set_canvasstyle (hHandle, MNG_CANVAS_BGRX8) <> 0 then
      MNGerror ('libmng reported an error setting the canvas style');

  end;

  Result := MNG_TRUE;
end;

{****************************************************************************}

{$F+}
function GetCanvasLine (hHandle : mng_handle;
                        iLinenr : mng_uint32) : mng_ptr; stdcall;
{$F-}

var OHForm : TMainForm;

begin                                  { get a fix on our form }
  OHForm := TMainForm (mng_get_userdata (hHandle));
                                       { easy with these bitmap objects ! }
  Result := OHForm.OFBitmap.ScanLine [iLinenr];
end;

{****************************************************************************}

{$F+}
function ImageRefresh (hHandle : mng_handle;
                       iX      : mng_uint32;
                       iY      : mng_uint32;
                       iWidth  : mng_uint32;
                       iHeight : mng_uint32) : mng_bool; stdcall;
{$F-}

var OHForm : TMainForm;

begin                                  { get a fix on our form }
  OHForm := TMainForm (mng_get_userdata (hHandle));
                                       { force redraw }
  OHForm.OFImage.Picture.Assign (OHForm.OFBitmap);

  Result := MNG_TRUE;
end;


{****************************************************************************}

{$F+}
function GetTickCount (hHandle : mng_handle) : mng_uint32; stdcall;
{$F-}
begin
  Result := Windows.GetTickCount;      { windows knows that }
end;

{****************************************************************************}

{$F+}
function SetTimer (hHandle : mng_handle;
                   iMsecs  : mng_uint32) : mng_bool; stdcall;
{$F-}

var OHForm : TMainForm;

begin                                  { get a fix on our form }
  OHForm := TMainForm (mng_get_userdata (hHandle));
  OHForm.OFTimer.Interval := iMsecs;   { and set the timer }
  OHForm.OFTimer.Enabled  := true;

  Result := MNG_TRUE;
end;

{****************************************************************************}

procedure TMainForm.FormCreate(Sender: TObject);

var IHRed, IHGreen, IHBlue : word;

begin                                  { initialize }
  OFBitmap                := TBitmap.Create;
  IFBytesPerSec           := 10000000;
  BFHasMouse              := false;
  OFFile                  := nil;

  OFOpenDialog.Initialdir := '';
  OFBitmap.HandleType     := bmDIB;    { make it a 24-bit DIB }
  OFBitmap.PixelFormat    := pf32bit;
                                       { now initialize the library }
  IFHandle := mng_initialize (mng_ptr(self), Memalloc, Memfree, nil);

  if IFHandle = NIL then
  begin
    MNGerror ('libmng initialization error' + #13#10 +
              'Program aborted');
    Windows.Postmessage (handle, WM_Close, 0, 0);
    Exit;
  end;
                                       { no need to store chunk-info ! }
  mng_set_storechunks    (IFHandle, MNG_FALSE);
                                       { do not use suspension-buffer }
  mng_set_suspensionmode (IFHandle, MNG_FALSE);
                                       { set all the callbacks }
  if (mng_setcb_openstream    (IFHandle, Openstream   ) <> MNG_NOERROR) or
     (mng_setcb_closestream   (IFHandle, Closestream  ) <> MNG_NOERROR) or
     (mng_setcb_readdata      (IFHandle, Readdata     ) <> MNG_NOERROR) or
     (mng_setcb_processheader (IFHandle, ProcessHeader) <> MNG_NOERROR) or
     (mng_setcb_getcanvasline (IFHandle, GetCanvasLine) <> MNG_NOERROR) or
     (mng_setcb_refresh       (IFHandle, ImageRefresh ) <> MNG_NOERROR) or
     (mng_setcb_gettickcount  (IFHandle, GetTickCount ) <> MNG_NOERROR) or
     (mng_setcb_settimer      (IFHandle, SetTimer     ) <> MNG_NOERROR) then
  begin
    MNGerror ('libmng reported an error setting a callback function!' + #13#10 +
              'Program aborted');
    Windows.Postmessage (handle, WM_Close, 0, 0);
    Exit;
  end;

  IHRed   := (Color       ) and $FF;   { supply our own bg-color }
  IHGreen := (Color shr  8) and $FF;
  IHBlue  := (Color shr 16) and $FF;

  IHRed   := (IHRed   shl 8) + IHRed;
  IHGreen := (IHGreen shl 8) + IHGreen;
  IHBlue  := (IHBlue  shl 8) + IHBlue;

  if mng_set_bgcolor (IFHandle, IHRed, IHGreen, IHBlue) <> MNG_NOERROR then
    MNGerror ('libmng reported an error setting the background color!');

end;

{****************************************************************************}

procedure TMainForm.FormCloseQuery(Sender: TObject; var CanClose: Boolean);
begin
  OFTimer.Enabled := false;
  BFCancelled     := true;
                                       { if we're still animating then stop it }
  if mng_status_running (IFHandle) and not mng_status_reading (IFHandle) then
    if mng_display_freeze (IFHandle) <> MNG_NOERROR then
      MNGerror ('libmng reported an error during display_freeze!');

  mng_cleanup (IFHandle);
end;

{****************************************************************************}

procedure TMainForm.FormShow(Sender: TObject);
begin
  FormResize (self);
end;

{****************************************************************************}

procedure TMainForm.FormResize(Sender: TObject);
begin                                  { center the image in the window }
  if ClientWidth  < OFImage.Width  then
    OFImage.Left := 0
  else
    OFImage.Left := (ClientWidth  - OFImage.Width ) div 2;

  if ClientHeight < OFImage.Height then
    OFImage.Top  := 0
  else
    OFImage.Top  := (ClientHeight - OFImage.Height) div 2;

end;

{****************************************************************************}

procedure TMainForm.FormKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if Key = vk_Escape then              { pressing <esc> will freeze an animation }
  begin
    OFTimer.Enabled := false;          { don't let that timer go off then ! }
    BFCancelled     := true;

    if mng_status_running (IFHandle) and not mng_status_reading (IFHandle) then
      if mng_display_freeze (IFHandle) <> MNG_NOERROR then
        MNGerror ('libmng reported an error during display_freeze!');
  end;
end;

{****************************************************************************}

procedure TMainForm.FormMouseMove(Sender: TObject; Shift: TShiftState; X,
  Y: Integer);
begin
  if mng_status_dynamic (IFHandle) then
  begin
    if BFHasMouse then                 { if we had the mouse, it's left ! }
    begin
      if mng_trapevent (IFHandle, 3, 0, 0) <> MNG_NOERROR then
        MNGerror ('libmng reported an error during trapevent!');

      BFHasMouse := false;
    end;
  end;
end;

{****************************************************************************}

procedure TMainForm.OFImageMouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
begin
  if mng_status_dynamic (IFHandle) then
  begin
    if BFHasMouse then                 { did we have the mouse already ? }
    begin
      if mng_trapevent (IFHandle, 2, X, Y) <> MNG_NOERROR then
        MNGerror ('libmng reported an error during trapevent!');
    end
    else
    begin                              { if not, it has entered ! }
      if mng_trapevent (IFHandle, 1, X, Y) <> MNG_NOERROR then
        MNGerror ('libmng reported an error during trapevent!');

      BFHasMouse := true;
    end;
  end;
end;

{****************************************************************************}

procedure TMainForm.OFImageMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  if mng_status_dynamic (IFHandle) then
    if mng_trapevent (IFHandle, 4, X, Y) <> MNG_NOERROR then
      MNGerror ('libmng reported an error during trapevent!');
end;

{****************************************************************************}

procedure TMainForm.OFImageMouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  if mng_status_dynamic (IFHandle) then
    if mng_trapevent (IFHandle, 5, X, Y) <> MNG_NOERROR then
      MNGerror ('libmng reported an error during trapevent!');
end;

{****************************************************************************}

procedure TMainForm.OFTimerTimer(Sender: TObject);

var IHRslt : mng_retcode;

begin
  OFTimer.Enabled := false;            { only once ! }

  if not BFCancelled then
  begin                                { and inform the library }
    IHRslt := mng_display_resume (IFHandle);

    if (IHRslt <> MNG_NOERROR) and (IHRslt <> MNG_NEEDTIMERWAIT) then
      MNGerror ('libmng reported an error during display_resume!');

  end;
end;

{****************************************************************************}

procedure TMainForm.OFMenuFileOpenClick(Sender: TObject);

var IHRslt : mng_retcode;

begin
  OFOpenDialog.InitialDir := '';
  OFOpenDialog.FileName   := SFFileName;

  if OFOpenDialog.Execute then         { get the filename }
  begin
    if OFTimer.Enabled then            { if the lib was active; stop it }
    begin
      OFTimer.Enabled := false;

      Application.ProcessMessages;     { process any timer requests (for safety) }
                                       { now freeze the animation }
      if mng_display_freeze (IFHandle) <> MNG_NOERROR then
        MNGerror ('libmng reported an error during display_freeze!');
    end;
                                       { save interesting fields }
    SFFileName      := OFOpenDialog.FileName;
    IFTicks         := Windows.GetTickCount;
    IFBytes         := 0;
    BFCancelled     := false;
                                       { always reset (just in case) }
    if mng_reset (IFHandle) <> MNG_NOERROR then
      MNGerror ('libmng reported an error during reset!')
    else
    begin                              { and let the lib do it's job ! }
      IHRslt := mng_readdisplay (IFHandle);

      if (IHRslt <> MNG_NOERROR) and (IHRSLT <> MNG_NEEDTIMERWAIT) then
        MNGerror ('libmng reported an error reading the input file!');

    end;
  end;
end;

{****************************************************************************}

procedure TMainForm.OFMenuFileProfileClick(Sender: TObject);

var SHProfileDir : array [0 .. MAX_PATH + 20] of char;

begin
  GetSystemDirectory (@SHProfileDir, MAX_PATH);
  strcat (@SHProfileDir, '\Color');

  OFOpenDialogProfile.InitialDir := strpas (@SHProfileDir);

  if OFOpenDialogProfile.Execute then
    if mng_set_outputprofile (IFHandle, pchar (OFOpenDialogProfile.FileName)) <> 0 then
      MNGerror ('libmng reported an error setting the output-profile!');

end;

{****************************************************************************}

procedure TMainForm.OFMenuFileExitClick(Sender: TObject);
begin
  if mng_cleanup (IFHandle) <> MNG_NOERROR then
    MNGerror ('libmng cleanup error');

  Close;
end;

{****************************************************************************}

procedure TMainForm.OFMenuOptionsModemSpeedClick(Sender: TObject);
begin
  OFMenuOptionsModem28k8.Checked      := false;
  OFMenuOptionsModem33k6.Checked      := false;
  OFMenuOptionsModem56k.Checked       := false;
  OFMenuOptionsModemISDN64.Checked    := false;
  OFMenuOptionsModemISDN128.Checked   := false;
  OFMenuOptionsModemCable512.Checked  := false;
  OFMenuOptionsModemUnlimited.Checked := false;

  if IFBytesPerSec = OFMenuOptionsModem28k8.Tag      div 10 then
    OFMenuOptionsModem28k8.Checked      := true
  else
  if IFBytesPerSec = OFMenuOptionsModem33k6.Tag      div 10 then
    OFMenuOptionsModem33k6.Checked      := true
  else
  if IFBytesPerSec = OFMenuOptionsModem56k.Tag       div 10 then
    OFMenuOptionsModem56k.Checked       := true
  else
  if IFBytesPerSec = OFMenuOptionsModemISDN64.Tag    div 10 then
    OFMenuOptionsModemISDN64.Checked    := true
  else
  if IFBytesPerSec = OFMenuOptionsModemISDN128.Tag   div 10 then
    OFMenuOptionsModemISDN128.Checked   := true
  else
  if IFBytesPerSec = OFMenuOptionsModemUnlimited.Tag div 10 then
    OFMenuOptionsModemCable512.Checked  := true
  else
    OFMenuOptionsModemUnlimited.Checked := true;

end;

{****************************************************************************}

procedure TMainForm.OFMenuOptionsModemXClick(Sender: TObject);
begin
  IFBytesPerSec := TMenuItem (Sender).Tag div 10;
end;

{****************************************************************************}

procedure TMainForm.MNGerror;

var iErrorcode : mng_uint32;
    iSeverity  : mng_uint8;
    iChunkname : mng_chunkid;
    iChunkseq  : mng_uint32;
    iExtra1    : mng_int32;
    iExtra2    : mng_int32;
    zErrortext : mng_pchar;

begin                                  { get extended info }
  iErrorcode := mng_getlasterror (IFHandle, iSeverity, iChunkname, iChunkseq,
                                            iExtra1, iExtra2, zErrortext);

  MessageDlg (SHMsg + #13#10#13#10 + strpas (zErrortext) + #13#10#13#10 +
              Format ('Error = %d; Severity = %d; Chunknr = %d; Extra1 = %d',
                      [iErrorcode, iSeverity, iChunkseq, iExtra1]),
              mtError, [mbOK], 0);
end;

{****************************************************************************}

end.
