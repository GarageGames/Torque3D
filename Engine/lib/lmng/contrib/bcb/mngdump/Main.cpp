//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#include <dir.h>
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
#include "Main.h"
TMainForm *MainForm;
//---------------------------------------------------------------------------
#include "About.h"
#include "Help.h"

# define PAGE_CHUNKS    1
# define PAGE_REPORT    2
# define PAGE_OPTIONS   3
# define PAGE_ABOUT     4
# define PAGE_HELP      5

/* ************************************************************************** */
/* *                                                                        * */
/* *          MngDump is based on Gerard Juyn's MngTree                     * */
/* *                                                                        * */
/* * COPYRIGHT NOTICE:                                                      * */
/* *                                                                        * */
/* * Copyright (c) 2000 Andy Protano                                        * */
/* *                    Gerard Juyn (gerard@libmng.com)                     * */
/* * [You may insert additional notices after this sentence if you modify   * */
/* *  this source]                                                          * */
/* *                                                                        * */
/* * For the purposes of this copyright and license, "Contributing Authors" * */
/* * is defined as the following set of individuals:                        * */
/* *                                                                        * */
/* *    Gerard Juyn (gerard@libmng.com)                                     * */
/* *    Andy Protano (a.a.protano@care4free.net)                            * */
/* *                                                                        * */
/* * This program is supplied "AS IS".  The Contributing Authors            * */
/* * disclaim all warranties, expressed or implied, including, without      * */
/* * limitation, the warranties of merchantability and of fitness for any   * */
/* * purpose.  The Contributing Authors assume no liability for direct,     * */
/* * indirect, incidental, special, exemplary, or consequential damages,    * */
/* * which may result from the use of the MNG Library, even if advised of   * */
/* * the possibility of such damage.                                        * */
/* *                                                                        * */
/* * Permission is hereby granted to use, copy, modify, and distribute this * */
/* * source code, or portions hereof, for any purpose, without fee, subject * */
/* * to the following restrictions:                                         * */
/* *                                                                        * */
/* * 1. The origin of this source code must not be misrepresented;          * */
/* *    you must not claim that you wrote the original software.            * */
/* *                                                                        * */
/* * 2. Altered versions must be plainly marked as such and must not be     * */
/* *    misrepresented as being the original source.                        * */
/* *                                                                        * */
/* * 3. This Copyright notice may not be removed or altered from any source * */
/* *    or altered source distribution.                                     * */
/* *                                                                        * */
/* * The Contributing Authors specifically permit, without fee, and         * */
/* * encourage the use of this source code as a component to supporting     * */
/* * the MNG and JNG file format in commercial products.  If you use this   * */
/* * source code in a product, acknowledgment would be highly appreciated.  * */
/* *                                                                        * */
/* ************************************************************************** */

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
  : TForm(Owner)
{
}
//---------------------------------------------------------------------------
__fastcall TMainForm::~TMainForm( void )
{
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OnMinMax(TMessage& Msg)
{
  ((POINT far *)Msg.LParam)[3].x = 500;
  ((POINT far *)Msg.LParam)[3].y = 450;

  TForm::Dispatch(&Msg);
}
//---------------------------------------------------------------------------
//								MessageBox Methods
// ----------------------------------------------------------------------
int __fastcall TMainForm::MessageBox( String &as, UINT flags )
{
  return ::MessageBox( Handle, as.c_str(), Application->Title.c_str(), flags );
}
// ----------------------------------------------------------------------
void __fastcall  TMainForm::MsgBoxOk( String as )
{
  (void)MessageBox( as, MB_ICONINFORMATION );
}
//-------------------------------------------------------------------------
int __fastcall  TMainForm::MsgBoxYN( String as )
{
  return MessageBox( as, MB_YESNO + MB_ICONQUESTION + MB_DEFBUTTON1 );
}
//-------------------------------------------------------------------------
int __fastcall  TMainForm::MsgBoxYNC( String as )
{
  return MessageBox( as, MB_YESNOCANCEL | MB_ICONQUESTION );
}
//-------------------------------------------------------------------------
void __fastcall  TMainForm::MsgBoxStop( String as )
{
  (void)MessageBox( as, MB_ICONSTOP );
}
//-------------------------------------------------------------------------
void __fastcall  TMainForm::MsgBoxInfo( String as )
{
  (void)MessageBox( as, MB_OK | MB_ICONINFORMATION );
}
//-------------------------------------------------------------------------
void __fastcall  TMainForm::MsgBoxWarn( String as )
{
  (void)MessageBox( as, MB_OK | MB_ICONWARNING );
}
//-------------------------------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
  // For when the application name changes - coz i'll forget !
  asAppName = Application->Title;
  // And while we've got it set the caption
  Caption = Application->Title;

  // Load About Screen
  LabelAbout->Caption = _szAbout;
  // Load Help Screen
  LabelHelp->Caption  = _szHelp;

  fd = NULL;
  strList = new TStringList();

  asTab = "  ";// default tab size 2
  RadioGroupTabSize->ItemIndex = 1; // range is zero based

  // Start with the help pane
  PageControl1->ActivePage = tsHelp;

  mnuWordWrap->Checked = false;
  // This call will TOGGLE and set word wrap for all the applicables
  // editors and enable/disable the Horz scroll bar.
  mnuWordWrapClick( this ); // So start with false to get to true;

  // Clear things from design time settings
  StaticTextStatus->Caption = "";
  //strList->Clear(); constructor dodid that
  pnlChunks->Caption = "";
  ListBoxChunks->Clear();
  RichEditChunks->Clear();
  RichEditReport->Clear();
  ProgressBar1->Min = 0;
  ProgressBar1->Position = 0;
  SetChunkCount( 0 );

  // Set default states for switches
  cbPaletteEntries->Checked   = false;
  cbRGBOrder->Checked         = true;
  cbRawData->Checked          = true;
  cbComments->Checked         = true;
  cbMacroIdentifier->Checked  = true;
  cbBool->Checked             = true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormDestroy(TObject *Sender)
{
  delete strList;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuExitClick(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuFileOpenClick(TObject *Sender)
{
  if( !OpenDialog1->Execute() )
    return;

  Application->ProcessMessages(); // Refresh the screen
  LoadFile();

}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuReloadClick(TObject *Sender)
{
  // Only reload - if we have something to reload
  if( OpenDialog1->FileName.Length() )
    LoadFile();
  else
    MsgBoxStop( "Nothing to reload yet !" );
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuUndoClick(TObject *Sender)
{
  if( RichEditReport->Lines->Count )  // Do we have something in there ?
    RichEditReport->Undo();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuCutClick(TObject *Sender)
{
  if( RichEditReport->Lines->Count )  // Do we have something in there ?
    RichEditReport->CutToClipboard();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuCopyClick(TObject *Sender)
{
  if( RichEditReport->Lines->Count )  // Do we have something in there ?
    RichEditReport->CopyToClipboard();
}
//-------------------------------------------------------------------------
void __fastcall TMainForm::sbtnCopyChunkClick(TObject *Sender)
{
  if( RichEditChunks->Lines->Count )  // Do we have something in there ?
  {
    // Copy chunk data from Chunks pane to clipboard
    RichEditChunks->SelectAll();
    RichEditChunks->CopyToClipboard();
    RichEditChunks->SelLength = 0;
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuPasteClick(TObject *Sender)
{
  if( RichEditReport->Lines->Count )  // Do we have something in there ?
    RichEditReport->PasteFromClipboard();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuSelectAllClick(TObject *Sender)
{
  if( RichEditReport->Lines->Count )  // Do we have something in there ?
    RichEditReport->SelectAll();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuWordWrapClick(TObject *Sender)
{
bool  bWrap;

  mnuWordWrap->Checked = !mnuWordWrap->Checked;
  bWrap = mnuWordWrap->Checked;

  RichEditChunks->WordWrap = bWrap;
  RichEditReport->WordWrap = bWrap;

  if( bWrap ) { // if Word wrap on then no need for horizontal scrollbar
    RichEditChunks->ScrollBars = ssVertical;
    RichEditReport->ScrollBars = ssVertical;
  } else { // Horizontal scrollbar required (auto)
    RichEditChunks->ScrollBars = ssBoth;
    RichEditReport->ScrollBars = ssBoth;
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuSetFontClick(TObject *Sender)
{
  if( FontDialog1->Execute() )
  {
    RichEditChunks->Font = FontDialog1->Font;
    RichEditReport->Font = FontDialog1->Font;
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FontDialog1Apply(TObject *Sender, HWND Wnd)
{
  RichEditChunks->Font = FontDialog1->Font;
  RichEditReport->Font = FontDialog1->Font;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuFindClick(TObject *Sender)
{
  if( RichEditReport->Lines->Count == 0 )
  {
    MsgBoxStop( "Find what ?.");
    return;
  }

  FindDialog->Execute();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FindDialogFind(TObject *Sender)
{
TSearchTypes st;
TFindDialog *fd;
int newpos;

  if( RichEditReport->Lines->Count == 0 )
  {
    MsgBoxStop( "Find what ?.");
    return;
  }

  fd = dynamic_cast<TFindDialog *>( Sender );

  if( fd->Options.Contains( frMatchCase ) )
      st << stMatchCase;
  if( fd->Options.Contains( frWholeWord ) )
      st << stWholeWord;

  if( RichEditReport->SelLength )
      RichEditReport->SelStart += 1;

  newpos = RichEditReport->FindText( fd->FindText,
        RichEditReport->SelStart, RichEditReport->Text.Length(), st );

  if( newpos != -1 ) {
    RichEditReport->SelStart = newpos;
    RichEditReport->SelLength = fd->FindText.Length();
  } else {
    MsgBoxInfo( "End of document reached." );
    RichEditReport->SelStart = 0;
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuFindNextClick(TObject *Sender)
{
  if( RichEditReport->Lines->Count == 0 )
  {
    MsgBoxStop( "Find what ?.");
    return;
  }

  FindDialogFind( FindDialog );
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuPrintClick(TObject *Sender)
{
  if( RichEditReport->Lines->Count == 0 )
  {
    MsgBoxStop( "I can't find anything to print !.");
    return;
  }

  RichEditReport->Print( ExtractFileName( OpenDialog1->FileName ) );

}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuPrintSetupClick(TObject *Sender)
{
  PrinterSetupDialog->Execute();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuSaveClick(TObject *Sender)
{
  if( RichEditReport->Lines->Count == 0 )
  {
    MsgBoxStop( "I can't find anything to save !.");
    return;
  }

  // Grab the graphic's filename and change to "txt" extension
  {
  char    szFile[ MAXFILE ];
  char    szExt[ MAXEXT ];
  char    szDest[ MAXPATH ];
  String  as;

    fnsplit( OpenDialog1->FileName.c_str(), NULL, NULL, szFile, szExt );

    memset( szDest, 0, MAXPATH );
    strcat( szDest, szFile );
    strcat( szDest, "\." );
    strcat( szDest, "txt" );
    as = szDest;

    // Initially create a text file of the same name as the graphic
    // but with "txt" extension
    SaveDialog1->FileName = as;
  }

  if( !SaveDialog1->Execute() )
    return; // Given up hey ?

  RichEditReport->Lines->SaveToFile( SaveDialog1->FileName );
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TMainForm::LoadFile( void )
{
int     bStatus;

  // Can we open the file ? 
  if( (fd = fopen( OpenDialog1->FileName.c_str(), "rb") ) == NULL )
  {
    MsgBoxStop( "Cannot open input file\n\n" + OpenDialog1->FileName );
    return;
  }

  pnlChunks->Caption = "";
  strList->Clear();
  ListBoxChunks->Clear();
  RichEditChunks->Clear();
  RichEditReport->Clear();

  ProgressBar1->Min = 0;
  ProgressBar1->Position = 0;
  SetChunkCount( 0 );

  RichEditReport->Lines->Add( "" );
  RichEditReport->Lines->Add( "Chunk contents of file :" );
  RichEditReport->Lines->Add( OpenDialog1->FileName );
  RichEditReport->Lines->Add( "" );

  // Make report pane visible - prove we are working and not locked up
  PageControl1->ActivePage = tsReport;

try {
    StaticTextStatus->Caption = "Working ... please wait";
    StaticTextStatus->Update();
    bStatus = DumpTree(); // true indicates success
}
__finally {
    ProgressBar1->Position = 0;
    StaticTextStatus->Caption = "Ok";
    fclose( fd );
    fd = NULL;
}

  if( !bStatus ) // We have an error
  {
    pnlChunks->Caption = "";
    strList->Clear();
    ListBoxChunks->Clear();
    RichEditChunks->Clear();

    RichEditReport->Lines->Add("");
    RichEditReport->Lines->Add("An error occurred while reading the file.");

  } else {  // Read file and chunks without any problems

    // Set Report Panels caption
    pnlChunks->Caption = "Chunk 1 of " + String( ListBoxChunks->Items->Count );

    // Set Listbox to first item
    ListBoxChunks->ItemIndex = 0;

    // Put gleeful message in report pane
    RichEditChunks->Lines->Add( "" );
    RichEditChunks->Lines->Add(
      "No error's found when reading file ... " );
    RichEditChunks->Lines->Add(
      "Click the list box to display the data for each chunk.");

    // Place cursor at top of editor
    RichEditChunks->SelStart = 0;
    RichEditChunks->SelLength = 0;

    // Place cursor at top of editor
    RichEditReport->SelStart = 0;
    RichEditReport->SelLength = 0;

    // Lock richedits - so an immediate undo does not make data vanish !
    RichEditReport->Modified = false;
    // @ap@ Borland should fix this ! (it don't work)
  }

}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::DumpTree( void )
{
mng_handle  hMNG;

  // let's initialize the library
  hMNG = mng_initialize( (mng_ptr)this, Alloc, Free, NULL );

  if( !hMNG )                           // did that work out ?
  {
    MNGError( hMNG, "Cannot initialize libmng." );
    mng_cleanup( &hMNG ); // Always cleanup the library
    MsgBoxStop( "Bye!" );
    Application->Terminate(); // Exit now
  }

  // setup callbacks
  if( (mng_setcb_openstream   ( hMNG, OpenStream    ) != 0) ||
      (mng_setcb_closestream  ( hMNG, CloseStream   ) != 0) ||
      (mng_setcb_processheader( hMNG,ProcessHeader  ) != 0) ||
      (mng_setcb_readdata     ( hMNG, FileReadData  ) != 0)
    )
  {
    MNGError( hMNG, "Cannot set callbacks for libmng.");
    mng_cleanup( &hMNG ); // Always cleanup the library
    MsgBoxStop( "Bye!" );
    Application->Terminate(); // Exit now
  }
  else
  {
    // read the file into memory
    if( mng_read( hMNG ) != 0 )
    {
      // Because we read the whole file in first,
      // here is where bad input files first choke !
      MNGError( hMNG, "Cannot read the file." );
      mng_cleanup( &hMNG ); // Always cleanup the library
      return false;
    }
    else
    {
      // run through the chunk list
      if( mng_iterate_chunks( hMNG, 0, IterateChunks ) != 0 )
      {
        MNGError( hMNG, "Error Getting Chunk info!" );
        mng_cleanup( &hMNG ); // Always cleanup the library
        return false;     // Errors may occur with bad chunk data
      }
    }
  }

  mng_cleanup( &hMNG ); // Always cleanup the library

  return true;
}
//---------------------------------------------------------------------------
void  __fastcall TMainForm::MNGError( mng_handle hMNG, String SHMsg )
{
// get extended info 
mng_uint32  iErrorcode;
mng_uint8   iSeverity;
mng_chunkid iChunkname;
mng_uint32  iChunkseq;
mng_int32   iExtra1;
mng_int32   iExtra2;
mng_pchar   zErrortext;
char        szFormatStr[ 256 ];

  iErrorcode = mng_getlasterror( hMNG, &iSeverity, &iChunkname,
                          &iChunkseq, &iExtra1, &iExtra2,
                          &zErrortext);

  wsprintf( szFormatStr,
    "Error = %d; Severity = %d; Chunknr = %d; Extra1 = %d",
    (int)iErrorcode, (int)iSeverity, (int)iChunkseq, (int)iExtra1
  );

  MsgBoxStop( SHMsg + "\n\n" +  String( zErrortext ) + "\n\n" + String( szFormatStr ) );
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ListBoxChunksClick(TObject *Sender)
{
// When the Chunks listbox is clicked ...
// display strings from strlist in the RichEditChunks window.
int iIndex = ListBoxChunks->ItemIndex;

  if( !ListBoxChunks->Items->Count )
    return;

  // Update panel to reflect the selection : "Chunk N of NChunks"
  pnlChunks->Caption  = "Chunk " + String( iIndex + 1) +
    " of " + String( ListBoxChunks->Items->Count );

  // Reset Chunks windows ...
  RichEditChunks->Clear();
  RichEditChunks->Lines->Add( "" );
  // ... Chunks header string : "Chunk N : XXXX" (XXXX == Chunk name)
  RichEditChunks->Lines->Add( "Chunk " + String( iIndex + 1 ) + " : " +
                            ListBoxChunks->Items->Strings[ iIndex ] );
  // ... Seperator line
  //RichEditChunks->Lines->Add("");

  // ... add the data from the stringlist
  {
  String as = strList->Strings[ iIndex ] + 1;
  String z;
    int n =1;
    do{
      if( as[n] == '\n' )
      {
        RichEditChunks->Lines->Add( z );
        z = "";
      }
      else
        z = z + as[n];
      n+=1;
    } while( n < as.Length() );
    RichEditChunks->Lines->Add( z );
  }

  // ... Seperator line
  RichEditChunks->Lines->Add("");

  // Place cursor at top of editor
  RichEditChunks->SelStart = 0;
  RichEditChunks->SelLength = 0;

}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EventShowPage(TObject *Sender)
{

  switch( ((TTabSheet *)Sender)->Tag ) {
  case PAGE_REPORT  :
    mnuPrint->Enabled = true;
    mnuEdit->Enabled = true;
    mnuUndo->Enabled = true;
    mnuSave->Enabled = true;
    sbtnSave->Enabled = true;
    sbtnUndo->Enabled = true;
    mnuCut->Enabled = true;
    sbtnCut->Enabled = true;
    mnuCopy->Enabled = true;
    sbtnCopy->Enabled = true;
    mnuPaste->Enabled = true;
    sbtnPaste->Enabled = true;
    mnuSearch->Enabled = true;
    sbtnPrint->Enabled = true;
    break;
  //case PAGE_CHUNKS  :
  //case PAGE_OPTIONS :
  ///case PAGE_ABOUT   :
  //case PAGE_HELP    :
  default :
    mnuPrint->Enabled = false;
    mnuEdit->Enabled = false;
    // no need to enable/disable menu commands as none will be available
    mnuSave->Enabled = false;
    sbtnSave->Enabled = false;
    sbtnUndo->Enabled = false;
    sbtnCut->Enabled = false;
    sbtnCopy->Enabled = false;
    sbtnPaste->Enabled = false;
    mnuSearch->Enabled = false;
    sbtnPrint->Enabled = false;
  }
  
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::RadioGroupTabSizeClick(TObject *Sender)
{
  switch( RadioGroupTabSize->ItemIndex ) {
  case 0  : asTab = " ";      break;
  case 1  : asTab = "  ";     break;
  case 2  : asTab = "   ";    break;
  case 3  : asTab = "    ";   break;
  case 4  : asTab = "     ";  break;
  case 5  : asTab = "      "; break;
  }
}
//---------------------------------------------------------------------------
String __fastcall TMainForm::PadInt( int iInt, int iWidth )
{

char  szFmtStr[ 24 ];
char  szInt[ 24 ];    // Should be wide enough !
String as;
  wsprintf( szFmtStr,"%%%d\d\0",iWidth ); //"%[iWidth]d"
  as = szFmtStr;
  wsprintf( szInt, szFmtStr, iInt );

  return String( szInt );
}
//---------------------------------------------------------------------------
mng_bool __fastcall TMainForm::ShowChunk(
  mng_handle hMNG, mng_handle hChunk, mng_chunkid iChunktype )
{
String  asDataText;

  // Fill asDataText with string data including newline's ...
  // this is added to string list for each chunk
  // and also added to the tail end of "Report"

  // NOTE :
  // Return True to continue processing.
  // If mng_getchunk_xxxx fails just return false to the
  // caller "(bool) myiterchunk" which inturn will then return false
  // to "(mng_retcode) mng_iterate_chunks(...)" which will then
  // give us the correct error code.
  // In other words DON'T check for errors in "(bool) myiterchunk" !

  switch( iChunktype ) {
  case  MNG_UINT_BACK :
    if( !Info_BACK( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_BASI :
    if( !Info_BASI( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_CLIP :
    if( !Info_CLIP( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_CLON :
    if( !Info_CLON( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_DBYK : // untested @ap@
    if( !Info_DBYK( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_DEFI :
    if( !Info_DEFI( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_DHDR :
    if( !Info_DHDR( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_DISC : // untested @ap@
    if( !Info_DISC( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_DROP : // untested @ap@
    if( !Info_DROP( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_ENDL :
    if( !Info_ENDL( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_FRAM :
    if( !Info_FRAM( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_IDAT :
    if( !Info_IDAT( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_IEND :
    if( !Info_IEND( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_IHDR :
    if( !Info_IHDR( hMNG, hChunk, asDataText ) )  return false; break;
#define MNG_UINT_IJNG 0x494a4e47L // Function AWOL @ap@
#define MNG_UINT_IPNG 0x49504e47L // Function AWOL @ap@
  case  MNG_UINT_JDAT :
    if( !Info_JDAT( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_JHDR :
    if( !Info_JHDR( hMNG, hChunk, asDataText ) )  return false; break;
#define MNG_UINT_JSEP 0x4a534550L // Function AWOL @ap@ 
  case  MNG_UINT_LOOP :
    if( !Info_LOOP( hMNG, hChunk, asDataText ) )  return false; break;
#define MNG_UINT_MaGN 0x4d61474eL // which one "mng_getchunk_magn" ? 
  case  MNG_UINT_MEND :
    if( !Info_MEND( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_MHDR :
    if( !Info_MHDR( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_MOVE :
    if( !Info_MOVE( hMNG, hChunk, asDataText ) )  return false; break;
#define MNG_UINT_MaGN 0x4d61474eL // which one "mng_getchunk_magn" ?
  case  MNG_UINT_ORDR :
    if( !Info_ORDR( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_PAST :
    if( !Info_PAST( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_PLTE :
    if( !Info_PLTE( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_PPLT :
    if( !Info_PPLT( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_PROM :
    if( !Info_PROM( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_SAVE :
    if( !Info_SAVE( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_SEEK :
    if( !Info_SEEK( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_SHOW :
    if( !Info_SHOW( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_TERM :
    if( !Info_TERM( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_bKGD :
    if( !Info_bKGD( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_cHRM :
    if( !Info_cHRM( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_eXPI :
    if( !Info_eXPI( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_fPRI :
    if( !Info_fPRI( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_gAMA :
    if( !Info_gAMA( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_hIST :
    if( !Info_hIST( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_iCCP :
    if( !Info_iCCP( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_iTXt : // untested @ap@
    if( !Info_iTXt( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_nEED :
    if( !Info_nEED( hMNG, hChunk, asDataText ) )  return false; break;
#define MNG_UINT_oFFs 0x6f464673L // Function AWOL @ap@
#define MNG_UINT_pCAL 0x7043414cL // Function AWOL @ap@
  case  MNG_UINT_pHYg : // untested @ap@
    if( !Info_pHYg( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_pHYs :
    if( !Info_pHYs( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_sBIT :
    if( !Info_sBIT( hMNG, hChunk, asDataText ) )  return false; break;
#define MNG_UINT_sCAL 0x7343414cL // Function AWOL @ap@
  case  MNG_UINT_sPLT : // untested @ap@
    if( !Info_sPLT( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_sRGB :
    if( !Info_sRGB( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_tEXt :
    if( !Info_tEXt( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_tIME :
    if( !Info_tIME( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_tRNS :
    if( !Info_tRNS( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_zTXt :
    if( !Info_zTXt( hMNG, hChunk, asDataText ) )  return false; break;
  case  MNG_UINT_HUH :
  default :           // this will catch unknown chunks - Huh !
    if( !Info_Unknown( hMNG, hChunk, asDataText ) )  return false; break;
  } // end of switch
  //-------------------------------------------------

  // Add this chunk's string to our string list
  strList->Insert( GetChunkCount(), asDataText );

  // Now's the time to bump chunk count
  IncChunkCount();

  // Add this chunk's string to the Report
  MainForm->RichEditReport->Lines->Add( asDataText );

  return true;
}
//---------------------------------------------------------------------------

