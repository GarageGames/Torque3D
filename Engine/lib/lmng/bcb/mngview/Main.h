//---------------------------------------------------------------------------
#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
// These MUST be defined before we include "Libmng.h
# define MNG_SUPPORT_READ
# define MNG_ACCESS_CHUNKS
# define MNG_STORE_CHUNKS
# define MNG_NO_CMS
# define MNG_USE_DLL
# define MNG_SUPPORT_DISPLAY
# define MNG_SKIP_ZLIB          // we don't need the zlib definitions here
# define MNG_SKIP_IJG6B         // we don't need the IJG definitions here
#include "libmng.h"
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
  TMainMenu *OFMainMenu;
  TMenuItem *OFMenuFile;
  TMenuItem *OFMenuFileOpen;
  TMenuItem *OFMenuFileProfile;
  TMenuItem *OFMenuFileN1;
  TMenuItem *OFMenuFileExit;
  TMenuItem *OFMenuOptions;
  TMenuItem *OFMenuOptionsModemSpeed;
  TMenuItem *OFMenuOptionsModem28k8;
  TMenuItem *OFMenuOptionsModem33k6;
  TMenuItem *OFMenuOptionsModem56k;
  TMenuItem *OFMenuOptionsModemISDN64;
  TMenuItem *OFMenuOptionsModemISDN128;
  TMenuItem *OFMenuOptionsModemCable512;
  TMenuItem *OFMenuOptionsModemUnlimited;
  TOpenDialog *OFOpenDialog;
  TTimer *OFTimer;
  TOpenDialog *OFOpenDialogProfile;
  TImage *OFImage;
  TPanel *Panel1;
  TProgressBar *ProgressBar1;
  void __fastcall FormCreate(TObject *Sender);
  void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall FormResize(TObject *Sender);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
  void __fastcall OFTimerTimer(TObject *Sender);
  void __fastcall OFMenuFileOpenClick(TObject *Sender);
  void __fastcall OFMenuFileProfileClick(TObject *Sender);
  void __fastcall OFMenuFileExitClick(TObject *Sender);
  void __fastcall OFMenuOptionsModemSpeedClick(TObject *Sender);
  void __fastcall OFMenuOptionsModemXClick(TObject *Sender);
private:	// User declarations
public :
    // Data - was private in the pascal version
    String            SFFileName;     /* filename of the input stream */
    TFileStream       *OFFile;        /* input stream */
    mng_handle        IFHandle;       /* the libray handle */
    Graphics::TBitmap *OFBitmap;      /* drawing canvas */
# ifdef TEST_RGB8_A8
    void *OFAlpha;
# endif
    bool              BFCancelled;    /* <esc> or app-exit */
    unsigned int      IFTicks;        /* used to fake slow connections */
    unsigned int      IFBytes;
    unsigned int      IFBytesPerSec;
    // Methods
    void MNGerror( String SHMsg );
public:		// User declarations
  __fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif

