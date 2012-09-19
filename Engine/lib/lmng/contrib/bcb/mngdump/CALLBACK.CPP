//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
# include <stdlib.h>

#include "Main.h"

//---------------------------------------------------------------------------
mng_ptr __stdcall TMainForm::Alloc( mng_size_t iSize )
{
  return (mng_ptr)calloc( 1, (size_t)iSize );
}
//---------------------------------------------------------------------------
void __stdcall TMainForm::Free( mng_ptr pPtr, mng_size_t iSize )
{
  free( pPtr );
  return;
}
//---------------------------------------------------------------------------
mng_bool __stdcall TMainForm::FileReadData( mng_handle hMNG,
                                        mng_ptr    pBuf,
                                        mng_uint32 iSize,
                                        mng_uint32 *iRead )
{
TMainForm *MainForm = (TMainForm *)mng_get_userdata( hMNG );

  *iRead = fread( pBuf, 1, iSize, MainForm->GetFd() );

  // iRead will indicate EOF

  MainForm->ProgressBar1->Position += (int)iRead;

  return MNG_TRUE;
}
//---------------------------------------------------------------------------
mng_bool __stdcall TMainForm::ProcessHeader(  mng_handle hHandle,
                                          mng_uint32 iWidth,
                                          mng_uint32 iHeight )
{
TMainForm  *MainForm = (TMainForm *)mng_get_userdata( hHandle );

  MainForm->Caption = ExtractFileName( MainForm->OpenDialog1->FileName ) +
                    " [" +
                    String( iWidth ) +
                    "x" +
                    String( iHeight ) +
                    "]";

  Application->Title = MainForm->asAppName + " " + MainForm->Caption;

  MainForm->ProgressBar1->Max = iWidth * iHeight;

  return MNG_TRUE;
}
//---------------------------------------------------------------------------
mng_bool __stdcall TMainForm::OpenStream( mng_handle hMNG )
{
  // nothing to do !
  return MNG_TRUE;
}
//---------------------------------------------------------------------------
mng_bool __stdcall TMainForm::CloseStream( mng_handle hMNG )
{
  MainForm->ProgressBar1->Position = 0;

  return MNG_TRUE;
}
//---------------------------------------------------------------------------
mng_bool __stdcall TMainForm::IterateChunks(  mng_handle  hMNG,
                                          mng_handle  hChunk,
                                          mng_chunkid iChunktype,
                                          mng_uint32  iChunkseq )
{
TMainForm         *MainForm = (TMainForm *)mng_get_userdata( hMNG );
char              aCh[5];

  // decode the chunkname
  aCh[0] = (char)((iChunktype >> 24) & 0xFF);
  aCh[1] = (char)((iChunktype >> 16) & 0xFF);
  aCh[2] = (char)((iChunktype >>  8) & 0xFF);
  aCh[3] = (char)((iChunktype      ) & 0xFF);
  aCh[4] = (char)0; // zero terminate - used as a "C" string below

  MainForm->RichEditReport->Lines->Add( "" );

  MainForm->RichEditReport->Lines->Add(
    "Chunk " + String( iChunkseq + 1 )  + " : " + String( aCh ) );

  // Add Chunk text to listbox
  MainForm->ListBoxChunks->Items->Add( aCh );

  // keep'm coming ... unless we encounter an error
  return MainForm->ShowChunk( hMNG, hChunk, iChunktype );
}
//---------------------------------------------------------------------------

