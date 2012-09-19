//---------------------------------------------------------------------------
#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <stdio.h>
//---------------------------------------------------------------------------
// These MUST be defined before we include "Libmng.h
//# define MNG_SUPPORT_READ
//# define MNG_ACCESS_CHUNKS
//# define MNG_STORE_CHUNKS
//# define MNG_NO_CMS
# define MNG_USE_DLL
# define MNG_SKIP_ZLIB
# define MNG_SKIP_LCMS
# define MNG_SKIP_IJG6B

#include "../../../libmng.h"
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
  TMainMenu *MainMenu1;
  TMenuItem *mnuFile;
  TMenuItem *mnuOpen;
  TMenuItem *mnuReload;
  TMenuItem *mnuSave;
  TMenuItem *N1;
  TMenuItem *mnuPrint;
  TMenuItem *mnuPrintSetup;
  TMenuItem *N2;
  TMenuItem *mnuExit;
  TMenuItem *mnuEdit;
  TMenuItem *mnuUndo;
  TMenuItem *N3;
  TMenuItem *mnuCut;
  TMenuItem *mnuCopy;
  TMenuItem *mnuPaste;
  TMenuItem *N4;
  TMenuItem *mnuSelectAll;
  TMenuItem *N5;
  TMenuItem *mnuSetFont;
  TMenuItem *mnuWordWrap;
  TMenuItem *mnuSearch;
  TMenuItem *mnuFind;
  TMenuItem *mnuFindNext;
  TPanel *Panel1;
  TSpeedButton *sbtnOpen;
  TSpeedButton *sbtnReload;
  TSpeedButton *sbtnSave;
  TSpeedButton *sbtnPrint;
  TSpeedButton *sbtnPrintSetup;
  TSpeedButton *sbtnUndo;
  TSpeedButton *sbtnCut;
  TSpeedButton *sbtnCopy;
  TSpeedButton *sbtnPaste;
  TOpenDialog *OpenDialog1;
  TSaveDialog *SaveDialog1;
  TPrintDialog *PrintDialog;
  TPrinterSetupDialog *PrinterSetupDialog;
  TFontDialog *FontDialog1;
  TFindDialog *FindDialog;
  TPageControl *PageControl1;
  TTabSheet *tsChunks;
  TTabSheet *tsReport;
  TPanel *pnlChunks;
  TListBox *ListBoxChunks;
  TSpeedButton *sbtnCopyChunk;
  TRichEdit *RichEditChunks;
  TRichEdit *RichEditReport;
  TPanel *PanelStatusBar;
  TProgressBar *ProgressBar1;
  TTabSheet *tsOptions;
  TTabSheet *tsAbout;
  TLabel *LabelAbout;
  TTabSheet *tsHelp;
  TLabel *LabelHelp;
  TCheckBox *cbBool;
  TCheckBox *cbMacroIdentifier;
  TCheckBox *cbRawData;
  TCheckBox *cbRGBOrder;
  TCheckBox *cbPaletteEntries;
  TCheckBox *cbComments;
  TRadioGroup *RadioGroupTabSize;
  TStaticText *StaticTextStatus;
  TStaticText *StaticText1;
  void __fastcall mnuFileOpenClick(TObject *Sender);
  void __fastcall FormCreate(TObject *Sender);
  void __fastcall ListBoxChunksClick(TObject *Sender);
  void __fastcall mnuReloadClick(TObject *Sender);
  void __fastcall mnuExitClick(TObject *Sender);
  void __fastcall FormDestroy(TObject *Sender);
  void __fastcall mnuUndoClick(TObject *Sender);
  void __fastcall mnuCutClick(TObject *Sender);
  void __fastcall mnuCopyClick(TObject *Sender);
  void __fastcall mnuPasteClick(TObject *Sender);
  void __fastcall mnuSelectAllClick(TObject *Sender);
  void __fastcall mnuWordWrapClick(TObject *Sender);
  void __fastcall mnuSetFontClick(TObject *Sender);
  void __fastcall FontDialog1Apply(TObject *Sender, HWND Wnd);
  void __fastcall mnuFindClick(TObject *Sender);
  void __fastcall FindDialogFind(TObject *Sender);
  void __fastcall mnuFindNextClick(TObject *Sender);
  void __fastcall sbtnCopyChunkClick(TObject *Sender);
  void __fastcall mnuPrintClick(TObject *Sender);
  void __fastcall mnuPrintSetupClick(TObject *Sender);
  void __fastcall mnuSaveClick(TObject *Sender);
  void __fastcall EventShowPage(TObject *Sender);
  void __fastcall RadioGroupTabSizeClick(TObject *Sender);
protected :
	void virtual __fastcall OnMinMax(TMessage& Msg);
	BEGIN_MESSAGE_MAP // to limit minimum form size for about/help panes
	  MESSAGE_HANDLER( WM_GETMINMAXINFO, TMessage, OnMinMax )
	END_MESSAGE_MAP(TForm)
private:	// User declarations
  FILE    *fd;
  int     iChunkCount;  // To link stringList to listbox
  String  asTab;        // Number of spaces to use as tab stop
public:		// User declarations
  // Constructor
  __fastcall TMainForm(TComponent* Owner);
  // Destructor
  __fastcall ~TMainForm( void );

  // ------------------------------------------------------------------
  // Callbacks ... as static member functions
  // ------------------------------------------------------------------
  static mng_ptr __stdcall Alloc( mng_size_t iSize );
  static void __stdcall Free( mng_ptr pPtr, mng_size_t iSize );
  static mng_bool __stdcall FileReadData( mng_handle hMNG,
                                          mng_ptr pBuf,
                                          mng_uint32 iSize,
                                          mng_uint32 *iRead );
  static mng_bool __stdcall ProcessHeader( mng_handle hHandle,
                                            mng_uint32 iWidth,
                                            mng_uint32 iHeight );
  static mng_bool __stdcall OpenStream( mng_handle hMNG );
  static mng_bool __stdcall CloseStream( mng_handle hMNG );
  static mng_bool __stdcall IterateChunks( mng_handle  hMNG,
                                          mng_handle hChunk,
                                          mng_chunkid iChunktype,
                                          mng_uint32 iChunkseq );

  // ------------------------------------------------------------------
  //public data members
  // ------------------------------------------------------------------
  // Associates a string, strList[n], with a chunk in the listbox[N]
  TStringList   *strList;
  String  asAppName; // pinch Application->Title at startup

  // ------------------------------------------------------------------
  //  MessageBox functions 
  // ------------------------------------------------------------------
  int __fastcall MessageBox( String &as, UINT flags );
  void __fastcall MsgBoxOk( String as );
  int __fastcall MsgBoxYN( String as );
  int __fastcall MsgBoxYNC( String as );
  void __fastcall MsgBoxStop( String as );
  void __fastcall MsgBoxInfo( String as );
  void __fastcall MsgBoxWarn( String as );

  // ------------------------------------------------------------------
  // Just to isolate teh "FILE *fd" variable from possible change.
  inline FILE* __fastcall GetFd( void )
    { return fd; };
    
  // ------------------------------------------------------------------
  //								Options
  // ------------------------------------------------------------------
  inline bool _fastcall WantsComments( void )
    { return cbComments->Checked; };

  inline bool _fastcall WantsPaletteEntries( void )
    { return cbPaletteEntries->Checked; };

  inline bool _fastcall WantsRgbOrder( void )
    { return cbRGBOrder->Checked; };

  inline bool _fastcall WantsRawData( void )
    { return cbRawData->Checked; };

  inline bool _fastcall WantsMacroIds( void )
    { return cbMacroIdentifier->Checked; };

  inline bool _fastcall WantsBool( void )
    { return cbBool->Checked; };

  // ------------------------------------------------------------------
  // Chunk count stuff
  // ------------------------------------------------------------------
  inline int __fastcall IncChunkCount( void )
    { return iChunkCount += 1; };
  inline int __fastcall SetChunkCount( int anInt )
    { iChunkCount = anInt; };
  inline int __fastcall GetChunkCount( void )
    { return iChunkCount; };

  // ------------------------------------------------------------------
  // Open a file, initialise things, and then calls "Dumptree(...)"
  void __fastcall LoadFile( void );

  // All LibMng calls involving the library handle are made from here :
  //  Initialize the library
  //  Setup callbacks
  //  Read the file into memory
  //  Run through the chunk list (mng_iterate_chunks)
  //  Cleanup the library
  // Show Report page, all done. (then explode into the starry heavens!)
  bool __fastcall DumpTree( void );

  // Handle library errors gracefully.
  void __fastcall MNGError( mng_handle hMNG, String SHMsg );

  // Uesd with Palette entries - to right align int's to N chars width
  String __fastcall PadInt( int iInt, int iWidth = 3 ); // default = 3

  // A long switch that calls Info_XXXX's to assemble the chunks string
  mng_bool __fastcall ShowChunk(  mng_handle hMNG,
                                  mng_handle hChunk,
                                  mng_chunkid iChunktype );

  // ------------------------------------------------------------------
  // The following functions are to assemble a string for each chunk.
  // The function name "Info_????" denotes which chunk it handles
  // Definitions can be found in "Chunks.cpp"
  // ------------------------------------------------------------------
  bool __fastcall Info_BACK( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_BASI( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_CLIP( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_CLON( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_DBYK( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_DEFI( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_DHDR( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_DISC( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_DROP( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_ENDL( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_FRAM( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_IDAT( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_IEND( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_IHDR( mng_handle hMNG, mng_handle hChunk, String &as );
  // MNG_UINT_IJNG - Function missing @ap@
  // MNG_UINT_IPNG - Function missing @ap@
  bool __fastcall Info_JDAT( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_JHDR( mng_handle hMNG, mng_handle hChunk, String &as );
  // MNG_UINT_JSEP - Function missing @ap@
  bool __fastcall Info_LOOP( mng_handle hMNG, mng_handle hChunk, String &as );
  // MAGN  ? MNG_UINT_MAGN @ap@
  bool __fastcall Info_MEND( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_MHDR( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_MOVE( mng_handle hMNG, mng_handle hChunk, String &as );
  // MaGN ? MNG_UINT_MaGN @ap@
  bool __fastcall Info_ORDR( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_PAST( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_PLTE( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_PPLT( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_PROM( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_SAVE( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_SEEK( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_SHOW( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_TERM( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_bKGD( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_cHRM( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_eXPI( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_fPRI( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_gAMA( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_hIST( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_iCCP( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_iTXt( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_nEED( mng_handle hMNG, mng_handle hChunk, String &as );
  // MNG_UINT_oFFs - Function missing @ap@
  // MNG_UINT_pCAL - Function missing @ap@
  bool __fastcall Info_pHYg( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_pHYs( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_sBIT( mng_handle hMNG, mng_handle hChunk, String &as );
  // MNG_UINT_sCAL - Function missing @ap@
  bool __fastcall Info_sPLT( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_sRGB( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_tEXt( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_tIME( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_tRNS( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_zTXt( mng_handle hMNG, mng_handle hChunk, String &as );
  bool __fastcall Info_Unknown( mng_handle hMNG, mng_handle hChunk, String &as );

};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
