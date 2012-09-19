//
//  Chunks.cpp
//
// The following functions are to assemble a string for each chunk
// The function name "Info_????" denote which chunk it handles
//
// Comments marked @todo@ may, or maynot, be filled in at a later date !
//
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "Main.h"

# define xUSE_UNKNOWN    // remove "x" to report of "UNKNOWN" - else ""

# define FFACTOR        ((float)100000.0)

// NB We must use Octal for RichEdit's newlines - MicroSoft love Octal !
# define nl             "\015\012"
# define TAB            asTab

// Ps : If anyone reading this knows how to do this using token
// pasting please let me know ! (AP)

// ConCat newline to string
# define NL             as = as + nl

// Integer ARGuement
# define IARG( _str_, _int_ )\
  as = as + TAB + (_str_) + " " + String( _int_ )

// Bool ARGuement
# define BARG( _str_, _bool_ )\
  if( WantsBool() )\
    as = as + TAB + (_str_) + " " + ((_bool_) ? "true" : "false");\
  else\
    as = as + TAB + (_str_) + " " + IntToStr( _bool_ );

// Float ARGuement
# define FARG( _str_, _float_ )\
  as = as + TAB + (_str_) + " " + String( _float_ )

// szText ARGuement
# define ZARG( _str_, _sz_ )\
  as = as + TAB + (_str_) + " \"" + String( _sz_ ) + "\""

// Concat general strings
# define STR( _str_ )         as = as + _str_

// Concat Hex string
# define HEXSTR( _str_ )      as = as + IntToHex(_str_,2)

// Concat Macro Identifier to ARG
#define MI( _str_ )           as = as + TAB + _str_;

// Concat "(" Comment ")" to ARG
#define PCOM( _str_ )\
  if( WantsComments() )       as = as + TAB + "(" + _str_ + ")";

// Concat "float(" Comment ")" to ARG
#define FPCOM( _str_ )\
  if( WantsComments() )       as = as + TAB + "float(" + _str_ + ")";

//---------------------------------------------------------------------------
// Macro Identifiers - these, below, are used several times
// NB those used once have local #define's and #undef's
//---------------------------------------------------------------------------

# ifdef USE_UNKNOWN
#   define UNKNOWN              "UNKNOWN"
# else
#   define UNKNOWN              ""
# endif
// Does this need two to be more visible ?
# define ILLEGAL_VALUE        ">> ILLEGAL VALUE << "

// IHDR, BASI, JHDR, PROM, sPLT
# define MI_BITDEPTH( _i_ )\
  if( WantsMacroIds() )\
  switch( (_i_) ) {\
  case MNG_BITDEPTH_1   : MI( "MNG_BITDEPTH_1" ); break;\
  case MNG_BITDEPTH_2   : MI( "MNG_BITDEPTH_2" ); break;\
  case MNG_BITDEPTH_4   : MI( "MNG_BITDEPTH_4" ); break;\
  case MNG_BITDEPTH_8   : MI( "MNG_BITDEPTH_8" ); break;\
  case MNG_BITDEPTH_16  : MI( "MNG_BITDEPTH_16" ); break;\
  default : MI(ILLEGAL_VALUE);\
  }

// IHDR, BASI, PROM
# define MI_COLORTYPE( _i_ )\
  if( WantsMacroIds() )\
  switch( (_i_) ) {\
  case MNG_COLORTYPE_GRAY     : MI( "MNG_COLORTYPE_GRAY" );    break;\
  case MNG_COLORTYPE_RGB      : MI( "MNG_COLORTYPE_RGB" );     break;\
  case MNG_COLORTYPE_INDEXED  : MI( "MNG_COLORTYPE_INDEXED" ); break;\
  case MNG_COLORTYPE_GRAYA    : MI( "MNG_COLORTYPE_GRAYA" );   break;\
  case MNG_COLORTYPE_RGBA     : MI( "MNG_COLORTYPE_RGBA" );    break;\
  default : MI(ILLEGAL_VALUE);\
  }

// IHDR, zTXt, iTXt, iCCP, BASI, JHDR 
# define MI_COMPRESSION_DEFLATE( _i_ )\
  if( WantsMacroIds() ) {\
    if( (_i_) == MNG_COMPRESSION_DEFLATE ) {\
      MI( "MNG_COMPRESSION_DEFLATE" );\
    } else {\
      MI( ILLEGAL_VALUE );\
    }\
  }

// IHDR, BASI, JHDR 
# define MI_FILTER( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_FILTER_ADAPTIVE      : MI( "MNG_FILTER_ADAPTIVE" );    break;\
  case MNG_FILTER_NO_ADAPTIVE   : MI( "MNG_FILTER_NO_ADAPTIVE" ); break;\
  case MNG_FILTER_DIFFERING     : MI( "MNG_FILTER_DIFFERING" ); break;\
  case MNG_FILTER_MASK          : MI( "MNG_FILTER_MASK" ); break;\
  default : MI(UNKNOWN);\
  }
  //NB MNG_FILTER_NO_DIFFERING  == MNG_FILTER_ADAPTIVE

// IHDR, BASI, JHDR 
#define MI_INTERLACE( _i_ )\
  if( WantsMacroIds() ) {\
    if( (_i_) == MNG_INTERLACE_NONE ) {\
      MI( "MNG_INTERLACE_NONE" )\
    } else if( (_i_) == MNG_INTERLACE_ADAM7 ) {\
      MI( "MNG_INTERLACE_ADAM7" );\
    } else { MI(ILLEGAL_VALUE);} \
  }

// pHYs, pHYg
#define MI_UNITS( _i_ )\
  if( WantsMacroIds() ) {\
    if( (_i_) == MNG_UNIT_UNKNOWN ) {\
      MI( "MNG_UNIT_UNKNOWN" )\
    } else if( (_i_) == MNG_UNIT_METER ) {\
      MI( "MNG_UNIT_METER" );\
    } else { MI(UNKNOWN);} \
  }

// CLON & MOVE
#define MI_LOCATION( _i_ )\
  if( WantsMacroIds() ) {\
    if( (_i_) == MNG_LOCATION_ABSOLUTE ) {\
      MI( "MNG_LOCATION_ABSOLUTE" )\
    } else if( (_i_) == MNG_LOCATION_RELATIVE ) {\
      MI( "MNG_LOCATION_RELATIVE" );\
    } else { MI(ILLEGAL_VALUE);} \
  }

//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_BACK( mng_handle hMNG, mng_handle hChunk, String &as )
{
// NB for BACKGROUND other values are only advisory
#define MI_BACKGROUND( _i_ )\
  if( WantsMacroIds() ) {\
    if( (_i_) == MNG_BACKGROUNDCOLOR_MANDATORY ) {\
      MI( "MNG_BACKGROUNDCOLOR_MANDATORY" )\
    } else if( (_i_) == MNG_BACKGROUNDIMAGE_MANDATORY ) {\
      MI( "MNG_BACKGROUNDIMAGE_MANDATORY" );\
    } \
  }

# define MI_BACKGROUND_TILE( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_BACKGROUNDIMAGE_NOTILE : MI( "MNG_BACKGROUNDIMAGE_NOTILE" );    break;\
  case MNG_BACKGROUNDIMAGE_TILE   : MI( "MNG_BACKGROUNDIMAGE_TILE" ); break;\
  default : MI(UNKNOWN);\
  }
mng_uint16       iRed;
mng_uint16       iGreen;
mng_uint16       iBlue;
mng_uint8        iMandatory;
mng_uint16       iImageid;
mng_uint8        iTile;

  if( mng_getchunk_back( hMNG, hChunk,
          &iRed, &iGreen, &iBlue,
          &iMandatory, &iImageid, &iTile ) != 0 )
    return false;

      IARG( "iRed", iRed );
  NL; IARG( "iGreen",iGreen );
  NL; IARG( "iBlue", iBlue );
  NL; IARG( "iMandatory", iMandatory ); MI_BACKGROUND( iMandatory );  
  NL; IARG( "iImageid", iImageid );
  NL; IARG( "iTile", iTile );           MI_BACKGROUND_TILE( iTile );

  return true;
# undef MI_BACKGROUND
# undef MI_BACKGROUND_TILE
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_BASI( mng_handle hMNG, mng_handle hChunk, String &as )
{
#define MI_VIEWABLE( _i_ )\
  if( WantsMacroIds() ) {\
    if( (_i_) == MNG_NOTVIEWABLE ) {\
      MI( "MNG_NOTVIEWABLE" )\
    } else if( (_i_) == MNG_VIEWABLE ) {\
      MI( "MNG_VIEWABLE" );\
    } else { MI(UNKNOWN);} \
  }
mng_uint32       iWidth;
mng_uint32       iHeight;
mng_uint8        iBitdepth;
mng_uint8        iColortype;
mng_uint8        iCompression;
mng_uint8        iFilter;
mng_uint8        iInterlace;
mng_uint16       iRed;
mng_uint16       iGreen;
mng_uint16       iBlue;
mng_uint16       iAlpha;
mng_uint8        iViewable;

  if( mng_getchunk_basi( hMNG, hChunk,
          &iWidth, &iHeight, &iBitdepth, &iColortype,
          &iCompression, &iFilter, &iInterlace,
          &iRed, &iGreen, &iBlue, &iAlpha,
          &iViewable ) != 0 )
    return false;

      IARG( "iWidth", iWidth );
  NL; IARG( "iHeight", iHeight );
  NL; IARG( "iBitdepth", iBitdepth );       MI_BITDEPTH( iBitdepth );
  NL; IARG( "iColortype", iColortype );     MI_COLORTYPE( iColortype );
  NL; IARG( "iCompression", iCompression ); MI_COMPRESSION_DEFLATE( iCompression );
  NL; IARG( "iFilter", iFilter );           MI_FILTER( iFilter );
  NL; IARG( "iInterlace", iInterlace );     MI_INTERLACE( iInterlace );
  NL; IARG( "iRed", iRed );
  NL; IARG( "iGreen", iGreen );
  NL; IARG( "iBlue", iBlue );
  NL; IARG( "iAlpha", iAlpha );
  NL; IARG( "iViewable", iViewable );       MI_VIEWABLE( iViewable );

  return true;
# undef MI_VIEWABLE
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_CLIP( mng_handle hMNG, mng_handle hChunk, String &as )
{
# define MI_CLIPPING( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_CLIPPING_ABSOLUTE  : MI( "MNG_CLIPPING_ABSOLUTE" );  break;\
  case MNG_CLIPPING_RELATIVE  : MI( "MNG_CLIPPING_RELATIVE" );  break;\
  default : MI(UNKNOWN);\
  }

mng_uint16       iFirstid;
mng_uint16       iLastid;
mng_uint8        iCliptype;
mng_int32        iClipl;
mng_int32        iClipr;
mng_int32        iClipt;
mng_int32        iClipb;

  if( mng_getchunk_clip( hMNG, hChunk,
          &iFirstid, &iLastid,
          &iCliptype, &iClipl, &iClipr, &iClipt, &iClipb ) != 0 )
    return false;

      IARG( "iFirstid", iFirstid );
  NL; IARG( "iLastid", iLastid );
  NL; IARG( "iCliptype", iCliptype ); MI_CLIPPING( iCliptype );
  NL; IARG( "iClipl", iClipl );
  NL; IARG( "iClipr", iClipr );
  NL; IARG( "iClipt", iClipt );
  NL; IARG( "iClipb", iClipb );

  return true;
# undef MI_CLIPPING
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_CLON( mng_handle hMNG, mng_handle hChunk, String &as )
{
# define MI_CLONTYPE( _i_ )\
  if( WantsMacroIds() )\
  switch( (_i_) ) {\
  case MNG_FULL_CLONE     : MI( "MNG_FULL_CLONE" );    break;\
  case MNG_PARTIAL_CLONE  : MI( "MNG_PARTIAL_CLONE" );     break;\
  case MNG_RENUMBER       : MI( "MNG_RENUMBER" ); break;\
  default : MI(UNKNOWN);\
  }

#define MI_CLON_CONCRETE( _i_ )\
  if( WantsMacroIds() ) {\
    if( (_i_) == MNG_CONCRETE_ASPARENT ) {\
      MI( "MNG_CONCRETE_ASPARENT" )\
    } else if( (_i_) == MNG_CONCRETE_MAKEABSTRACT ) {\
      MI( "MNG_CONCRETE_MAKEABSTRACT" );\
    } else { MI(UNKNOWN);} \
  }

mng_uint16       iSourceid;
mng_uint16       iCloneid;
mng_uint8        iClonetype;
mng_uint8        iDonotshow;
mng_uint8        iConcrete;
mng_bool         bHasloca;
mng_uint8        iLocationtype;
mng_int32        iLocationx;
mng_int32        iLocationy;

  if( mng_getchunk_clon( hMNG, hChunk,
          &iSourceid, &iCloneid, &iClonetype, &iDonotshow,
          &iConcrete, &bHasloca,
          &iLocationtype, &iLocationx, &iLocationy ) != 0 )
    return false;

      IARG( "iSourceid", iSourceid );
  NL; IARG( "iCloneid", iCloneid );
  NL; IARG( "iClonetype", iClonetype ); MI_CLONTYPE( iClonetype );
  NL; IARG( "iDonotshow", iDonotshow );
  NL; IARG( "iConcrete", iConcrete );   MI_CLON_CONCRETE( iConcrete );
  NL; BARG( "bHasloca", bHasloca );
  NL; IARG( "iLocationtype", iLocationtype ); MI_LOCATION( iLocationtype );
  NL; IARG( "iLocationx", iLocationx );
  NL; IARG( "iLocationy", iLocationy );

  return true;
# undef MI_CLONTYPE
# undef MI_CLON_CONCRETE
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_DBYK( mng_handle hMNG, mng_handle hChunk, String &as )
{
# define MI_POLARITY( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_POLARITY_ONLY    : MI( "MNG_POLARITY_ONLY" );    break;\
  case MNG_POLARITY_ALLBUT  : MI( "MNG_POLARITY_ALLBUT" );    break;\
  default : MI(UNKNOWN);\
  }
mng_chunkid      iChunkname;
mng_uint8        iPolarity;
mng_uint32       iKeywordssize;
mng_pchar        zKeywords;

  if( mng_getchunk_dbyk( hMNG, hChunk,
          &iChunkname, &iPolarity, &iKeywordssize, &zKeywords ) != 0 )
    return false;

      IARG( "iChunkname", iChunkname );     // show chunk name ? @todo@
  NL; IARG( "iPolarity", iPolarity );         MI_POLARITY( iPolarity );
  NL; IARG( "iKeywordssize", iKeywordssize );
  NL; ZARG( "zKeywords", zKeywords );

  return true;
# undef MI_POLARITY  
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_DEFI( mng_handle hMNG, mng_handle hChunk, String &as )
{
#define MI_DONOTSHOW( _i_ )\
  if( WantsMacroIds() ) {\
    if( (_i_) == MNG_DONOTSHOW_VISIBLE ) {\
      MI( "MNG_DONOTSHOW_VISIBLE" )\
    } else if( (_i_) == MNG_DONOTSHOW_NOTVISIBLE ) {\
      MI( "MNG_DONOTSHOW_NOTVISIBLE" );\
    }\
  }

#define MI_DEFI_CONCRETE( _i_ )\
  if( WantsMacroIds() ) {\
    if( (_i_) == MNG_ABSTRACT ) {\
      MI( "MNG_ABSTRACT" )\
    } else if( (_i_) == MNG_CONCRETE ) {\
      MI( "MNG_CONCRETE" );\
    }\
  }
mng_uint16       iObjectid;
mng_uint8        iDonotshow;
mng_uint8        iConcrete;
mng_bool         bHasloca;
mng_int32        iXlocation;
mng_int32        iYlocation;
mng_bool         bHasclip;
mng_int32        iLeftcb;
mng_int32        iRightcb;
mng_int32        iTopcb;
mng_int32        iBottomcb;

  if( mng_getchunk_defi( hMNG, hChunk,
          &iObjectid, &iDonotshow, &iConcrete,
          &bHasloca, &iXlocation, &iYlocation,
          &bHasclip, &iLeftcb, &iRightcb, &iTopcb, &iBottomcb ) != 0 )
    return false;

      IARG( "iObjectid", iObjectid );
  NL; IARG( "iDonotshow", iDonotshow ); MI_DONOTSHOW( iDonotshow );
  NL; IARG( "iConcrete", iConcrete );   MI_DEFI_CONCRETE( iConcrete ); 
  NL; BARG( "bHasloca", bHasloca );
  NL; IARG( "iXlocation", iXlocation );
  NL; IARG( "iYlocation", iYlocation );
  NL; BARG( "bHasclip", bHasclip );
  NL; IARG( "iLeftcb", iLeftcb );
  NL; IARG( "iRightcb", iRightcb );
  NL; IARG( "iTopcb", iTopcb );
  NL; IARG( "iBottomcb", iBottomcb );

  return true;
# undef MI_DONOTSHOW
# undef MI_DEFI_CONCRETE
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_DHDR( mng_handle hMNG, mng_handle hChunk, String &as )
{
# define MI_IMAGETYPE( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_IMAGETYPE_UNKNOWN  : MI( "MNG_IMAGETYPE_UNKNOWN" );    break;\
  case MNG_IMAGETYPE_PNG      : MI( "MNG_IMAGETYPE_PNG" );    break;\
  case MNG_IMAGETYPE_JNG      : MI( "MNG_IMAGETYPE_JNG" );    break;\
  default : MI(UNKNOWN);\
  }

# define MI_DELTATYPE( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_DELTATYPE_REPLACE            : MI( "MNG_DELTATYPE_REPLACE" );    break;\
  case MNG_DELTATYPE_BLOCKPIXELADD      : MI( "MNG_DELTATYPE_BLOCKPIXELADD" );    break;\
  case MNG_DELTATYPE_BLOCKALPHAADD      : MI( "MNG_DELTATYPE_BLOCKALPHAADD" );    break;\
  case MNG_DELTATYPE_BLOCKCOLORADD      : MI( "MNG_DELTATYPE_BLOCKCOLORADD" );    break;\
  case MNG_DELTATYPE_BLOCKPIXELREPLACE  : MI( "MNG_DELTATYPE_BLOCKPIXELREPLACE" );    break;\
  case MNG_DELTATYPE_BLOCKALPHAREPLACE  : MI( "MNG_DELTATYPE_BLOCKALPHAREPLACE" );    break;\
  case MNG_DELTATYPE_BLOCKCOLORREPLACE  : MI( "MNG_DELTATYPE_BLOCKCOLORREPLACE" );    break;\
  case MNG_DELTATYPE_NOCHANGE           : MI( "MNG_DELTATYPE_NOCHANGE" );    break;\
  default : MI(UNKNOWN);\
  }

mng_uint16       iObjectid;
mng_uint8        iImagetype;
mng_uint8        iDeltatype;
mng_uint32       iBlockwidth;
mng_uint32       iBlockheight;
mng_uint32       iBlockx;
mng_uint32       iBlocky;

  if( mng_getchunk_dhdr( hMNG, hChunk,
          &iObjectid, &iImagetype, &iDeltatype,
          &iBlockwidth, &iBlockheight, &iBlockx, &iBlocky ) != 0 )
    return false;

      IARG( "iObjectid", iObjectid );
  NL; IARG( "iImagetype", iImagetype );     MI_IMAGETYPE( iImagetype );
  NL; IARG( "iDeltatype", iDeltatype );     MI_DELTATYPE( iDeltatype );
  NL; IARG( "iBlockwidth", iBlockwidth );
  NL; IARG( "iBlockheight", iBlockheight );
  NL; IARG( "iBlockx", iBlockx );
  NL; IARG( "iBlocky", iBlocky );

  return true;
# undef MI_IMAGETYPE
# undef MI_DELTATYPE
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_DISC( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint32       iCount;
mng_uint16p      pObjectids;

  if( mng_getchunk_disc( hMNG, hChunk, &iCount, &pObjectids ) != 0 )
    return false;

      IARG( "iCount", iCount );
  //pObjectids pObjectids

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_DROP( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint32       iCount;
mng_chunkidp     pChunknames;

  if( mng_getchunk_drop( hMNG, hChunk, &iCount, &pChunknames ) != 0 )
    return false;

    IARG( "iCount", iCount );
  // pChunknamesp Chunknames // Iterate chunk names ? @todo@

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_ENDL( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint8        iLevel;

  if( mng_getchunk_endl( hMNG, hChunk, &iLevel ) != 0 )
    return false;

  IARG( "iLevel", iLevel );

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_FRAM( mng_handle hMNG, mng_handle hChunk, String &as )
{
# define MI_BOUNDARY( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_BOUNDARY_ABSOLUTE  : MI( "MNG_BOUNDARY_ABSOLUTE" );    break;\
  case MNG_BOUNDARY_RELATIVE  : MI( "MNG_BOUNDARY_RELATIVE" ); break;\
  default : MI(UNKNOWN);\
  }

# define MI_FRAMINGMODE( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_FRAMINGMODE_NOCHANGE : MI( "MNG_FRAMINGMODE_NOCHANGE" );    break;\
  case MNG_FRAMINGMODE_1        : MI( "MNG_FRAMINGMODE_1" );    break;\
  case MNG_FRAMINGMODE_2        : MI( "MNG_FRAMINGMODE_2" );    break;\
  case MNG_FRAMINGMODE_3        : MI( "MNG_FRAMINGMODE_3" );    break;\
  case MNG_FRAMINGMODE_4        : MI( "MNG_FRAMINGMODE_4" );    break;\
  default : MI(UNKNOWN);\
  }

# define MI_CHANGEDELAY( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_CHANGEDELAY_NO           : MI( "MNG_CHANGEDELAY_NO" );    break;\
  case MNG_CHANGEDELAY_NEXTSUBFRAME : MI( "MNG_CHANGEDELAY_NEXTSUBFRAME" );    break;\
  case MNG_CHANGEDELAY_DEFAULT      : MI( "MNG_CHANGEDELAY_DEFAULT" );    break;\
  default : MI(UNKNOWN);\
  }

# define MI_CHANGETIMOUT( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_CHANGETIMOUT_NO              : MI( "MNG_CHANGETIMOUT_NO" );    break;\
  case MNG_CHANGETIMOUT_DETERMINISTIC_1 : MI( "MNG_CHANGETIMOUT_DETERMINISTIC_1" );    break;\
  case MNG_CHANGETIMOUT_DETERMINISTIC_2 : MI( "MNG_CHANGETIMOUT_DETERMINISTIC_2" );    break;\
  case MNG_CHANGETIMOUT_DECODER_1       : MI( "MNG_CHANGETIMOUT_DECODER_1" );    break;\
  case MNG_CHANGETIMOUT_DECODER_2       : MI( "MNG_CHANGETIMOUT_DECODER_2" );    break;\
  case MNG_CHANGETIMOUT_USER_1          : MI( "MNG_CHANGETIMOUT_USER_1" );    break;\
  case MNG_CHANGETIMOUT_USER_2          : MI( "MNG_CHANGETIMOUT_USER_2" );    break;\
  case MNG_CHANGETIMOUT_EXTERNAL_1      : MI( "MNG_CHANGETIMOUT_EXTERNAL_1" );    break;\
  case MNG_CHANGETIMOUT_EXTERNAL_2      : MI( " MNG_CHANGETIMOUT_EXTERNAL_2" );    break;\
  default : MI(UNKNOWN);\
  }

# define MI_CHANGECLIPPING( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_CHANGECLIPPING_NO            : MI( "MNG_CHANGECLIPPING_NO" );    break;\
  case MNG_CHANGECLIPPING_NEXTSUBFRAME  : MI( "MNG_CHANGECLIPPING_NEXTSUBFRAME" );    break;\
  case MNG_CHANGECLIPPING_DEFAULT       : MI( "MNG_CHANGECLIPPING_DEFAULT" );    break;\
  default : MI(UNKNOWN);\
  }

# define MI_CHANGESYNCID( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_CHANGESYNCID_NO            : MI( "MNG_CHANGESYNCID_NO" );    break;\
  case MNG_CHANGESYNCID_NEXTSUBFRAME  : MI( "MNG_CHANGESYNCID_NEXTSUBFRAME" );    break;\
  case MNG_CHANGESYNCID_DEFAULT       : MI( "MNG_CHANGESYNCID_DEFAULT" );    break;\
  default : MI(UNKNOWN);\
  }

mng_bool         bEmpty;
mng_uint8        iMode;
mng_uint32       iNamesize;
mng_pchar        zName;
mng_uint8        iChangedelay;
mng_uint8        iChangetimeout;
mng_uint8        iChangeclipping;
mng_uint8        iChangesyncid;
mng_uint32       iDelay;
mng_uint32       iTimeout;
mng_uint8        iBoundarytype;
mng_int32        iBoundaryl;
mng_int32        iBoundaryr;
mng_int32        iBoundaryt;
mng_int32        iBoundaryb;
mng_uint32       iCount;
mng_uint32p      pSyncids;

  if( mng_getchunk_fram( hMNG, hChunk,
    &bEmpty, &iMode, &iNamesize, &zName,
    &iChangedelay, &iChangetimeout, &iChangeclipping, &iChangesyncid,
    &iDelay, &iTimeout,
    &iBoundarytype, &iBoundaryl, &iBoundaryr,&iBoundaryt, &iBoundaryb,
    &iCount, &pSyncids ) != 0 )
    return false;

      BARG( "bEmpty", bEmpty );
  NL; IARG( "iMode", iMode );           MI_FRAMINGMODE( iMode );
  NL; IARG( "iNamesize", iNamesize );
  NL; ZARG( "zName", zName );
  NL; IARG( "iChangedelay", iChangedelay );       MI_CHANGEDELAY( iChangedelay );
  NL; IARG( "iChangetimeout", iChangetimeout );   MI_CHANGETIMOUT( iChangetimeout );
  NL; IARG( "iChangeclipping", iChangeclipping ); MI_CHANGECLIPPING( iChangeclipping );
  NL; IARG( "iChangesyncid", iChangesyncid );     MI_CHANGESYNCID( iChangesyncid );      
  NL; IARG( "iDelay", iDelay );
  NL; IARG( "iTimeout", iTimeout );
  NL; IARG( "iBoundarytype", iBoundarytype );     MI_BOUNDARY( iBoundarytype );
  NL; IARG( "iBoundaryl", iBoundaryl );
  NL; IARG( "iBoundaryr", iBoundaryr );
  NL; IARG( "iBoundaryt", iBoundaryt );
  NL; IARG( "iBoundaryb", iBoundaryb );
  NL; IARG( "iCount", iCount );
  //pSyncids pSyncids @todo@

  return true;
# undef MI_BOUNDARY
# undef MI_FRAMINGMODE
# undef MI_CHANGEDELAY
# undef MI_CHANGETIMOUT
# undef MI_CHANGECLIPPING
# undef MI_CHANGESYNCID
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_IDAT( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint32      iRawlen;
mng_ptr         pRawdata;

  if( mng_getchunk_idat( hMNG, hChunk, &iRawlen, &pRawdata ) != 0 )
    return false;

  IARG( "iRawlen", iRawlen );

  if( WantsRawData() ) {
  Byte  *bp = (Byte*)pRawdata;

    NL; STR( TAB + "Rawdata : " );
    // show the first 16 bytes, as hex
    for( int n = 0; n < 16; n +=1 )
      HEXSTR( (int)bp[n] ) + " ";
  }

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_IEND( mng_handle hMNG, mng_handle hChunk, String &as )
{
  NL + TAB; STR( "End of Image." );
  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_IHDR(
                      mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint32        iWidth;
mng_uint32        iHeight;
mng_uint8         iBitdepth;
mng_uint8         iColortype;
mng_uint8         iCompression;
mng_uint8         iFilter;
mng_uint8         iInterlace;

  if( mng_getchunk_ihdr( hMNG, hChunk, &iWidth, &iHeight, &iBitdepth,
          &iColortype, &iCompression, &iFilter, &iInterlace ) != 0 )
    return false;

      IARG( "iWidth", iWidth );
  NL; IARG( "iHeight", iHeight );
  NL; IARG( "iBitdepth", iBitdepth );       MI_BITDEPTH( iBitdepth );
  NL; IARG( "iColortype", iColortype );     MI_COLORTYPE( iColortype );
  NL; IARG( "iCompression", iCompression ); MI_COMPRESSION_DEFLATE( iCompression );
  NL; IARG( "iFilter", iFilter );           MI_FILTER( iFilter );
  NL; IARG( "iInterlace", iInterlace );     MI_INTERLACE( iInterlace );

  if( WantsComments() )
  {
    NL; NL + TAB;
    switch( iColortype ) {
    case MNG_COLORTYPE_GRAY :
      switch( iBitdepth ) {
      case 1 : case 2 : case 4 : case 8 : case 16 :
        STR( "Each pixel value is a greyscale level -" );
      } // inner switch
      break;
    case MNG_COLORTYPE_RGB :
      switch( iBitdepth ) {
      case 8 : case 16 :
        STR( "Each pixel value is an R,G,B series -" );
      } // inner switch
      break;
    case MNG_COLORTYPE_INDEXED :
      switch( iBitdepth ) {
      case 1 : case 2 : case 4 : case 8 :
        STR( "Each pixel value is a palette index -" );
      } // inner switch
      break;
    case MNG_COLORTYPE_GRAYA :
      switch( iBitdepth ) {
      case 8 : case 16 :
        STR( "Each pixel value is a greyscale level, "\
              "followed by an Alpha channel level -" );
      } // inner switch
      break;
    case MNG_COLORTYPE_RGBA :
      switch( iBitdepth ){
      case 8 : case 16:
        STR( "Each pixel value is an R,G,B "\
                  "series, followed by an Alpha channel level -" );
        } // inner switch
        break;
    }
    STR(  " " + String(iBitdepth) + " bits per pixel." );
  }

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_JDAT( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint32      iRawlen;
mng_ptr         pRawdata = NULL;

  if( mng_getchunk_jdat( hMNG, hChunk, &iRawlen, &pRawdata ) != 0 )
    return false;

  IARG( "iRawlen", iRawlen );

  if( WantsRawData() ) {
  Byte  *bp = (Byte*)pRawdata;

    NL; STR( TAB + "Rawdata : " );
    // show the first 16 bytes, as hex
    for( int n = 0; n < 16; n +=1 )
      HEXSTR( (int)bp[n] ) + " ";
  }

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_JHDR( mng_handle hMNG, mng_handle hChunk, String &as )
{
# define MI_JPEG_COLORTYPE( _i_ )\
  if( WantsMacroIds() ) {\
    switch( _i_ ){\
    case MNG_COLORTYPE_JPEGGRAY   : MI( "MNG_COLORTYPE_JPEGGRAY");break;\
    case MNG_COLORTYPE_JPEGCOLOR  : MI( "MNG_COLORTYPE_JPEGCOLOR");break;\
    case MNG_COLORTYPE_JPEGGRAYA  : MI( "MNG_COLORTYPE_JPEGGRAYA");break;\
    case MNG_COLORTYPE_JPEGCOLORA : MI( "MNG_COLORTYPE_JPEGCOLORA");break;\
    default : MI(UNKNOWN);\
    }\
}

# define MI_JPEG_BITDEPTH( _i_ )\
  if( WantsMacroIds() ) {\
    switch( _i_ ){\
    case MNG_BITDEPTH_JPEG8       : MI("MNG_BITDEPTH_JPEG8");break;\
    case MNG_BITDEPTH_JPEG12      : MI("MNG_BITDEPTH_JPEG12");break;\
    case MNG_BITDEPTH_JPEG8AND12  : MI("MNG_BITDEPTH_JPEG8AND12");break;\
    default : MI(UNKNOWN);\
    }\
  }

# define MI_JPEGCOMPRESSION( _i_ )\
  if( WantsMacroIds() ) {\
    if( iImagecompression == MNG_COMPRESSION_BASELINEJPEG ) {\
      MI( "MNG_COMPRESSION_BASELINEJPEG" );\
    } else { MI(ILLEGAL_VALUE); };\
  }

# define MI_JPEGINTERLACE( _i_ )\
  if( WantsMacroIds() ) {\
    if( iImageinterlace == MNG_INTERLACE_SEQUENTIAL ) {\
      MI( "MNG_INTERLACE_SEQUENTIAL");\
    } else {\
      MI( "MNG_INTERLACE_PROGRESSIVE" );\
    }\
  }

// NB alpha bitdepth is not png bitdepth because it can be 0 (zero)
# define MI_ALPHABITDEPTH( _i_ )\
  if( WantsMacroIds() )\
  switch( (_i_) ) {\
  case MNG_BITDEPTH_1   : MI( "MNG_BITDEPTH_1" ); break;\
  case MNG_BITDEPTH_2   : MI( "MNG_BITDEPTH_2" ); break;\
  case MNG_BITDEPTH_4   : MI( "MNG_BITDEPTH_4" ); break;\
  case MNG_BITDEPTH_8   : MI( "MNG_BITDEPTH_8" ); break;\
  case MNG_BITDEPTH_16  : MI( "MNG_BITDEPTH_16" ); break;\
  case 0 : break;\
  default : MI(ILLEGAL_VALUE);\
  }

mng_uint32       iWidth;
mng_uint32       iHeight;
mng_uint8        iColortype;
mng_uint8        iImagesampledepth;
mng_uint8        iImagecompression;
mng_uint8        iImageinterlace;
mng_uint8        iAlphasampledepth;
mng_uint8        iAlphacompression;
mng_uint8        iAlphafilter;
mng_uint8        iAlphainterlace;

  if( mng_getchunk_jhdr( hMNG, hChunk,
          &iWidth, &iHeight, &iColortype, &iImagesampledepth,
          &iImagecompression, &iImageinterlace,
          &iAlphasampledepth, &iAlphacompression,
          &iAlphafilter, &iAlphainterlace ) != 0 )
    return false;

      IARG( "iWidth", iWidth );
  NL; IARG( "iHeight", iHeight );
  NL; IARG( "iColortype", iColortype );               MI_JPEG_COLORTYPE( iColortype );
  NL; IARG( "iImagesampledepth", iImagesampledepth ); MI_JPEG_BITDEPTH( iImagesampledepth );
  NL; IARG( "iImagecompression", iImagecompression ); MI_JPEGCOMPRESSION( iImagecompression );
  NL; IARG( "iImageinterlace", iImageinterlace );     MI_JPEGINTERLACE( iImageinterlace );
  NL; IARG( "iAlphasampledepth", iAlphasampledepth ); MI_ALPHABITDEPTH( iAlphasampledepth );
  NL; IARG( "iAlphacompression", iAlphacompression ); MI_COMPRESSION_DEFLATE( iAlphacompression );
  NL; IARG( "iAlphafilter", iAlphafilter );           MI_FILTER( iAlphafilter );
  NL; IARG( "iAlphainterlace", iAlphainterlace );     MI_INTERLACE( iAlphainterlace );

  return true;
# undef MI_JPEG_COLORTYPE
# undef MI_JPEG_BITDEPTH
# undef MI_JPEGCOMPRESSION
# undef MI_JPEGINTERLACE
# undef MI_ALPHABITDEPTH
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_LOOP( mng_handle hMNG, mng_handle hChunk, String &as )
{
# define MI_TERMINATION( _i_ )\
  if( WantsMacroIds() )\
  switch( (_i_) ) {\
  case MNG_TERMINATION_DECODER_NC       :\
        MI( "MNG_TERMINATION_DECODER_NC" ); break;\
  case MNG_TERMINATION_USER_NC          :\
        MI( "MNG_TERMINATION_USER_NC" ); break;\
  case MNG_TERMINATION_EXTERNAL_NC      :\
        MI( "MNG_TERMINATION_EXTERNAL_NC" ); break;\
  case MNG_TERMINATION_DETERMINISTIC_NC :\
        MI( "MNG_TERMINATION_DETERMINISTIC_NC" ); break;\
  case MNG_TERMINATION_DECODER_C        :\
        MI( "MNG_TERMINATION_DECODER_C" ); break;\
  case MNG_TERMINATION_USER_C           :\
        MI( "MNG_TERMINATION_USER_C" ); break;\
  case MNG_TERMINATION_EXTERNAL_C       :\
        MI( "MNG_TERMINATION_EXTERNAL_C" ); break;\
  case MNG_TERMINATION_DETERMINISTIC_C  :\
        MI( "MNG_TERMINATION_DETERMINISTIC_C" ); break;\
  default : MI(UNKNOWN);\
  }
mng_uint8        iLevel;
mng_uint32       iRepeat;
mng_uint8        iTermination;
mng_uint32       iItermin;
mng_uint32       iItermax;
mng_uint32       iCount;
mng_uint32p      pSignals;

  if( mng_getchunk_loop( hMNG, hChunk,
          &iLevel, &iRepeat, &iTermination,
          &iItermin, &iItermax, &iCount, &pSignals ) != 0 )
    return false;

      IARG( "iLevel", iLevel );
  NL; IARG( "iRepeat", iRepeat );
  NL; IARG( "iTermination", iTermination ); MI_TERMINATION( iTermination ); 
  NL; IARG( "iItermin", iItermin );
  NL; IARG( "iItermax", iItermax );
  NL; IARG( "iCount", iCount );
  //pSignals pSignals //@todo@

  return true;
# undef MI_TERMINATION
}
//---------------------------------------------------------------------------
/*
bool __fastcall TMainForm::Info_M?GN( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint16       iFirstid;
mng_uint16       iLastid;
mng_uint16       iMethodX;
mng_uint16       iMX;
mng_uint16       iMY;
mng_uint16       iML;
mng_uint16       iMR;
mng_uint16       iMT;
mng_uint16       iMB;
mng_uint16       iMethodY;

  if( mng_getchunk_magn( hMNG, hChunk,
          &iFirstid, &iLastid,
          &iMethodX, &iMX, &iMY, &iML, &iMR, &iMT, &iMB,
          &iMethodY ) != 0 )
    return false;

      IARG( "iFirstid", iFirstid );
  NL; IARG( "iLastid", iLastid );
  NL; IARG( "iMethodX", iMethodX );
  NL; IARG( "iMX", iMX );
  NL; IARG( "iMY", iMY );
  NL; IARG( "iML", iML );
  NL; IARG( "iMR", iMR );
  NL; IARG( "iMT", iMT );
  NL; IARG( "iMB", iMB );
  NL; IARG( "iMethodY", iMethodY );

  return true;
}
*/
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_MEND( mng_handle hMNG, mng_handle hChunk, String &as )
{
  NL + TAB; STR( "End of Multiply Network Graphic." );
  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_MHDR( mng_handle hMNG, mng_handle hChunk, String &as )
{
// NB "iSimplicity" is a bit field
# define MI_SIMPLICITY( _i_ )\
  if( WantsMacroIds() ) {\
    if( (_i_) & MNG_SIMPLICITY_VALID )\
      MI( "MNG_SIMPLICITY_VALID" );\
    if( (_i_) & MNG_SIMPLICITY_SIMPLEFEATURES )\
      MI( "MNG_SIMPLICITY_SIMPLEFEATURES" );\
    if( (_i_) & MNG_SIMPLICITY_COMPLEXFEATURES )\
      MI( "MNG_SIMPLICITY_COMPLEXFEATURES" );\
    if( (_i_) & MNG_SIMPLICITY_TRANSPARENCY )\
      MI( "MNG_SIMPLICITY_TRANSPARENCY" );\
    if( (_i_) & MNG_SIMPLICITY_JNG )\
      MI( "MNG_SIMPLICITY_JNG" );\
    if( (_i_) & MNG_SIMPLICITY_DELTAPNG )\
      MI( "MNG_SIMPLICITY_DELTAPNG" );\
  }

mng_uint32       iWidth;
mng_uint32       iHeight;
mng_uint32       iTicks;
mng_uint32       iLayercount;
mng_uint32       iFramecount;
mng_uint32       iPlaytime;
mng_uint32       iSimplicity;

  if( mng_getchunk_mhdr( hMNG, hChunk,
          &iWidth, &iHeight, &iTicks,
          &iLayercount, &iFramecount, &iPlaytime,
          &iSimplicity ) != 0 )
    return false;

      IARG( "iWidth", iWidth );
  NL; IARG( "iHeight", iHeight );
  NL; IARG( "iTicks", iTicks );
  NL; IARG( "iLayercount", iLayercount );
  NL; IARG( "iFramecount", iFramecount );
  NL; IARG( "iPlaytime", iPlaytime );
  NL; IARG( "iSimplicity", iSimplicity ); MI_SIMPLICITY( iSimplicity );

  return true;
# undef MI_SIMPLICITY  
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_MOVE( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint16       iFirstid;
mng_uint16       iLastid;
mng_uint8        iMovetype;
mng_int32        iMovex;
mng_int32        iMovey;

  if( mng_getchunk_move( hMNG, hChunk,
          &iFirstid, &iLastid, &iMovetype, &iMovex, &iMovey ) != 0 )
    return false;

      IARG( "iFirstid", iFirstid );
  NL; IARG( "iLastid", iLastid );
  NL; IARG( "iMovetype", iMovetype ); MI_LOCATION( iMovetype );
  NL; IARG( "iMovex", iMovex );
  NL; IARG( "iMovey", iMovey );

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_ORDR( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint32       iCount;

  if( mng_getchunk_ordr( hMNG, hChunk, &iCount ) != 0 )
    return false;

    IARG( "iCount", iCount );

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_PAST( mng_handle hMNG, mng_handle hChunk, String &as )
{
// PAST
# define MI_TARGET( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_TARGET_ABSOLUTE          : MI( "MNG_TARGET_ABSOLUTE" );    break;\
  case MNG_TARGET_RELATIVE_SAMEPAST : MI( "MNG_TARGET_RELATIVE_SAMEPAST" ); break;\
  case MNG_TARGET_RELATIVE_PREVPAST : MI( "MNG_TARGET_RELATIVE_PREVPAST" ); break;\
  default : MI(UNKNOWN);\
  }

// COMPOSITE, ORIENTATION, OFFSET & BOUNDARY depend upon "iCount"
mng_uint16       iDestid;
mng_uint8        iTargettype;
mng_int32        iTargetx;
mng_int32        iTargety;
mng_uint32       iCount;

  if( mng_getchunk_past( hMNG, hChunk,
          &iDestid, &iTargettype, &iTargetx, &iTargety, &iCount ) != 0 )
    return false;

      IARG( "iDestid", iDestid );
  NL; IARG( "iTargettype", iTargettype ); MI_TARGET( iTargettype );
  NL; IARG( "iTargetx", iTargetx );
  NL; IARG( "iTargety", iTargety );
  NL; IARG( "iCount", iCount );

  return true;
# undef MI_TARGET  
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_PLTE( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint32      iCount;
mng_palette8    aPalette;
mng_uint32      iPalEntry;

  if( mng_getchunk_plte( hMNG, hChunk, &iCount, &aPalette ) != 0 )
    return false;

  IARG( "iCount", iCount );

  if( WantsPaletteEntries() )
  {
    iPalEntry = 0;
    do{
      if( WantsRgbOrder() )
      {
        as = as + nl +
        TAB + "Palette entry [" + PadInt( iPalEntry ) + "]" +
        TAB + "R(" + PadInt( aPalette[ iPalEntry ].iRed   ) + ") " +
        TAB + "G(" + PadInt( aPalette[ iPalEntry ].iGreen ) + ") " +
        TAB + "B(" + PadInt( aPalette[ iPalEntry ].iBlue  ) + ")";
      }
      else
      {
        as = as + nl +
        TAB + "Palette entry [" + PadInt( iPalEntry ) + "]" +
        TAB + "B(" + PadInt( aPalette[ iPalEntry ].iBlue  ) + ") " +
        TAB + "G(" + PadInt( aPalette[ iPalEntry ].iGreen ) + ") " +
        TAB + "R(" + PadInt( aPalette[ iPalEntry ].iRed   ) + ")";

      };
      iPalEntry += 1;
    } while( iPalEntry < iCount );
  }

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_PPLT( mng_handle hMNG, mng_handle hChunk, String &as )
{
/*
# define MI_DELTATYPE( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_DELTATYPE_REPLACERGB   : MI( "MNG_DELTATYPE_REPLACERGB" );    break;\
  case MNG_DELTATYPE_DELTARGB     : MI( "MNG_DELTATYPE_DELTARGB" );    break;\
  case MNG_DELTATYPE_REPLACEALPHA : MI( "MNG_DELTATYPE_REPLACEALPHA" );    break;\
  case MNG_DELTATYPE_DELTAALPHA   : MI( "MNG_DELTATYPE_DELTAALPHA" );    break;\
  case MNG_DELTATYPE_REPLACERGBA  : MI( "MNG_DELTATYPE_REPLACERGBA" );    break;\
  case MNG_DELTATYPE_DELTARGBA    : MI( "MNG_DELTATYPE_DELTARGBA" );    break;\
  default : MI(UNKNOWN);\
  }
*/
mng_uint32       iCount;

  if( mng_getchunk_pplt( hMNG, hChunk, &iCount ) != 0 )
    return false;

      IARG( "iCount", iCount );

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_PROM( mng_handle hMNG, mng_handle hChunk, String &as )
{
# define MI_FILLMETHOD( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_FILLMETHOD_LEFTBITREPLICATE  : MI( "MNG_FILLMETHOD_LEFTBITREPLICATE" );    break;\
  case MNG_FILLMETHOD_ZEROFILL          : MI( "MNG_FILLMETHOD_ZEROFILL" );    break;\
  default : MI(UNKNOWN);\
  }
mng_uint8        iColortype;
mng_uint8        iSampledepth;
mng_uint8        iFilltype;

  if( mng_getchunk_prom( hMNG, hChunk,
          &iColortype, &iSampledepth, &iFilltype ) != 0 )
    return false;

      IARG( "iColortype", iColortype );     MI_COLORTYPE( iColortype );
  NL; IARG( "iSampledepth", iSampledepth ); MI_BITDEPTH( iSampledepth );
  NL; IARG( "iFilltype", iFilltype );       MI_FILLMETHOD( iFilltype );

  return true;
# undef MI_FILLMETHOD
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_SAVE( mng_handle hMNG, mng_handle hChunk, String &as )
{
# define MI_SAVEOFFSET( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_SAVEOFFSET_4BYTE : MI( "MNG_SAVEOFFSET_4BYTE" );    break;\
  case MNG_SAVEOFFSET_8BYTE : MI( "MNG_SAVEOFFSET_8BYTE" );    break;\
  default : MI(UNKNOWN);\
  }

/*
# define MI_SAVEENTRY( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_SAVEENTRY_SEGMENTFULL    : MI( "MNG_SAVEENTRY_SEGMENTFULL" );    break;\
  case MNG_SAVEENTRY_SEGMENT        : MI( "MNG_SAVEENTRY_SEGMENT" );    break;\
  case MNG_SAVEENTRY_SUBFRAME       : MI( "MNG_SAVEENTRY_SUBFRAME" );    break;\
  case MNG_SAVEENTRY_EXPORTEDIMAGE  : MI( "MNG_SAVEENTRY_EXPORTEDIMAGE" );    break;\
  default : MI(UNKNOWN);\
  }
*/
mng_bool         bEmpty;
mng_uint8        iOffsettype;
mng_uint32       iCount;

  if( mng_getchunk_save( hMNG, hChunk,
          &bEmpty, &iOffsettype, &iCount ) != 0 )
    return false;

      BARG( "bEmpty", bEmpty );
  NL; IARG( "iOffsettype", iOffsettype ); MI_SAVEOFFSET( iOffsettype );
  NL; IARG( "iCount", iCount );

  return true;
# undef MI_SAVEOFFSET
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_SEEK( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint32       iNamesize;
mng_pchar        zName;

  if( mng_getchunk_seek( hMNG, hChunk, &iNamesize, &zName ) != 0 )
    return false;

      IARG( "iNamesize", iNamesize );
  NL; ZARG( "zName", zName );

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_SHOW( mng_handle hMNG, mng_handle hChunk, String &as )
{
# define MI_SHOWMODE( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_SHOWMODE_0  : MI( "MNG_SHOWMODE_0" );    break;\
  case MNG_SHOWMODE_1  : MI( "MNG_SHOWMODE_1" );    break;\
  case MNG_SHOWMODE_2  : MI( "MNG_SHOWMODE_2" );    break;\
  case MNG_SHOWMODE_3  : MI( "MNG_SHOWMODE_3" );    break;\
  case MNG_SHOWMODE_4  : MI( "MNG_SHOWMODE_4" );    break;\
  case MNG_SHOWMODE_5  : MI( "MNG_SHOWMODE_5" );    break;\
  case MNG_SHOWMODE_6  : MI( "MNG_SHOWMODE_6" );    break;\
  case MNG_SHOWMODE_7  : MI( "MNG_SHOWMODE_7" );    break;\
  default : MI(UNKNOWN);\
  }
mng_bool        bEmpty;
mng_uint16      iFirstid;
mng_uint16      iLastid;
mng_uint8       iMode;

  if( mng_getchunk_show( hMNG, hChunk,
          &bEmpty, &iFirstid, &iLastid, &iMode ) != 0 )
    return false;

      BARG( "bEmpty", bEmpty );
  NL; IARG( "iFirstid", iFirstid );
  NL; IARG( "iLastid", iLastid );
  NL; IARG( "iMode", iMode );       MI_SHOWMODE( iMode );

  return true;
# undef MI_SHOWMODE
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_TERM( mng_handle hMNG, mng_handle hChunk, String &as )
{
# define MI_TERMACTION( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_TERMACTION_LASTFRAME   : MI( "MNG_TERMACTION_LASTFRAME" ); break;\
  case MNG_TERMACTION_CLEAR       : MI( "MNG_TERMACTION_CLEAR" ); break;\
  case MNG_TERMACTION_FIRSTFRAME  : MI( "MNG_TERMACTION_FIRSTFRAME" ); break;\
  case MNG_TERMACTION_REPEAT      : MI( "MNG_TERMACTION_REPEAT" ); break;\
  default : MI(UNKNOWN);\
  }

# define MI_ITERACTION( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_ITERACTION_LASTFRAME   : MI( "MNG_ITERACTION_LASTFRAME" ); break;\
  case MNG_ITERACTION_CLEAR       : MI( "MNG_ITERACTION_CLEAR" ); break;\
  case MNG_ITERACTION_FIRSTFRAME  : MI( "MNG_ITERACTION_FIRSTFRAME" ); break;\
  default : MI(UNKNOWN);\
  }

mng_uint8        iTermaction;
mng_uint8        iIteraction;
mng_uint32       iDelay;
mng_uint32       iItermax;

  if( mng_getchunk_term( hMNG, hChunk,
        &iTermaction, &iIteraction, &iDelay, &iItermax ) != 0 )
    return false;

      IARG( "iTermaction", iTermaction ); MI_TERMACTION( iTermaction );
  NL; IARG( "iIteraction", iIteraction ); MI_ITERACTION( iIteraction );
  NL; IARG( "iDelay", iDelay );
  NL; IARG( "iItermax", iItermax );

  return true;
# undef MI_TERMACTION
# undef MI_ITERACTION
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_bKGD( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_bool         bEmpty;
mng_uint8        iType;
mng_uint8        iIndex;
mng_uint16       iGray;
mng_uint16       iRed;
mng_uint16       iGreen;
mng_uint16       iBlue;

  if( mng_getchunk_bkgd( hMNG, hChunk,
          &bEmpty, &iType, &iIndex, &iGray,
          &iRed, &iGreen, &iBlue ) != 0 )
    return false;

      BARG( "bEmpty", bEmpty );
  NL; IARG( "iType", iType );
  NL; IARG( "iIndex", iIndex );
  NL; IARG( "iGray", iGray );
  NL; IARG( "iRed", iRed );
  NL; IARG( "iGreen", iGreen );
  NL; IARG( "iBlue", iBlue );

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_cHRM( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_bool         bEmpty;
mng_uint32       iWhitepointx;
mng_uint32       iWhitepointy;
mng_uint32       iRedx;
mng_uint32       iRedy;
mng_uint32       iGreenx;
mng_uint32       iGreeny;
mng_uint32       iBluex;
mng_uint32       iBluey;

  if( mng_getchunk_chrm( hMNG, hChunk,
          &bEmpty, &iWhitepointx, &iWhitepointy,
          &iRedx, &iRedy,
          &iGreenx,&iGreeny,
          &iBluex, &iBluey ) != 0 )
    return false;

      BARG( "bEmpty", bEmpty );
  NL; FARG( "iWhitepointx", iWhitepointx );
      FPCOM( String( (float)(iWhitepointx /FFACTOR) ) );
  NL; FARG( "iWhitepointy", iWhitepointy );
      FPCOM( String( (float)(iWhitepointy /FFACTOR) ) );

  NL; FARG( "iRedx", iRedx );
      FPCOM( String( (float)(iRedx /FFACTOR) ) );
  NL; FARG( "iRedy", iRedy );
      FPCOM( String( (float)(iRedy /FFACTOR) ) );

  NL; FARG( "iGreenx", iGreenx );
      FPCOM( String( (float)(iGreenx /FFACTOR) ) );
  NL; FARG( "iGreeny", iGreeny );
      FPCOM( String( (float)(iGreeny /FFACTOR) ) );

  NL; FARG( "iBluex", iBluex );
      FPCOM( String( (float)(iBluex /FFACTOR) ) );
  NL; FARG( "iBluey", iBluey );
      FPCOM( String( (float)(iBluey /FFACTOR) ) );

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_eXPI( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint16       iSnapshotid;
mng_uint32       iNamesize;
mng_pchar        zName;

  if( mng_getchunk_expi( hMNG, hChunk,
          &iSnapshotid, &iNamesize, &zName ) != 0 )
    return false;

      IARG( "iSnapshotid", iSnapshotid );
  NL; IARG( "iNamesize", iNamesize );
  NL; ZARG( "zName", zName );

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_fPRI( mng_handle hMNG, mng_handle hChunk, String &as )
{
# define MI_PRIORITY( _i_ )\
  if( WantsMacroIds() )\
  switch( _i_ ) {\
  case MNG_PRIORITY_ABSOLUTE  : MI( "MNG_PRIORITY_ABSOLUTE" );    break;\
  case MNG_PRIORITY_RELATIVE  : MI( "MNG_PRIORITY_RELATIVE" );    break;\
  default : MI(UNKNOWN);\
  }
mng_uint8        iDeltatype;
mng_uint8        iPriority;

  if( mng_getchunk_fpri( hMNG, hChunk, &iDeltatype, &iPriority ) != 0 )
    return false;

      IARG( "iDeltatype", iDeltatype ); MI_PRIORITY( iDeltatype ); 
  NL; IARG( "iPriority", iPriority );

  return true;
# undef MI_PRIORITY  
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_gAMA( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_bool        bEmpty; 
mng_uint32      iGamma;

  if( mng_getchunk_gama( hMNG, hChunk, &bEmpty, &iGamma ) != 0 )
    return false;

      BARG( "bEmpty", bEmpty );
  NL; FARG( "iGamma", iGamma );
      FPCOM( String( (float)(iGamma /FFACTOR) ) );

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_hIST( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint32       iEntrycount;
mng_uint16arr    aEntries;

  if( mng_getchunk_hist( hMNG, hChunk, &iEntrycount, &aEntries ) != 0 )
    return false;


  IARG( "iEntrycount", iEntrycount );
  //aEntries aEntries   @todo@

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_iCCP( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_bool         bEmpty;
mng_uint32       iNamesize;
mng_pchar        zName;
mng_uint8        iCompression;
mng_uint32       iProfilesize;
mng_ptr          pProfile;

  if( mng_getchunk_iccp( hMNG, hChunk,
          &bEmpty, &iNamesize, &zName, &iCompression,
          &iProfilesize,&pProfile ) != 0 )
    return false;

      BARG( "bEmpty", bEmpty );
  NL; IARG( "iNamesize", iNamesize );
  NL; ZARG( "Name", zName );
  NL; IARG( "iCompression", iCompression ); MI_COMPRESSION_DEFLATE( iCompression );
  NL; IARG( "iProfilesize", iProfilesize );
//      "pProfile " + String( pProfile );

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_iTXt( mng_handle hMNG, mng_handle hChunk, String &as )
{
#define MI_ITXT_FLAG( _i_ )\
  if( WantsMacroIds() ) {\
    if( (_i_) == MNG_FLAG_UNCOMPRESSED ) {\
      MI( "MNG_FLAG_UNCOMPRESSED" )\
    } else if( (_i_) == MNG_FLAG_COMPRESSED ) {\
      MI( "MNG_FLAG_COMPRESSED" );\
    }\
  }
mng_uint32       iKeywordsize;
mng_pchar        zKeyword;
mng_uint8        iCompressionflag;
mng_uint8        iCompressionmethod;
mng_uint32       iLanguagesize;
mng_pchar        zLanguage;
mng_uint32       iTranslationsize;
mng_pchar        zTranslation;
mng_uint32       iTextsize;
mng_pchar        zText;

  if( mng_getchunk_itxt( hMNG, hChunk,
          &iKeywordsize, &zKeyword,
          &iCompressionflag, &iCompressionmethod,
          &iLanguagesize, &zLanguage,
          &iTranslationsize, &zTranslation,
          &iTextsize, &zText ) != 0 )
    return false;

      IARG( "iKeywordsize", iKeywordsize );
  NL; ZARG( "zKeyword", zKeyword );
  NL; IARG( "iCompressionflag", iCompressionflag );     MI_ITXT_FLAG( iCompressionflag );
  NL; IARG( "iCompressionmethod", iCompressionmethod ); MI_COMPRESSION_DEFLATE( iCompressionflag );
  NL; IARG( "iLanguagesize", iLanguagesize );
  NL; ZARG( "zLanguage", zLanguage );
  NL; IARG( "iTranslationsize", iTranslationsize );
  NL; ZARG( "zTranslation", zTranslation );
  NL; IARG( "iTextsize", iTextsize );
  NL; ZARG( "zText", zText );

  return true;
# undef MI_ITXT_FLAG
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_nEED( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint32       iKeywordssize;
mng_pchar        zKeywords;

  if( mng_getchunk_need( hMNG, hChunk, &iKeywordssize, &zKeywords ) != 0)
    return false;

      IARG( "iKeywordssize", iKeywordssize );
  NL; IARG( "zKeywords", zKeywords );

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_pHYg( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_bool         bEmpty;
mng_uint32       iSizex;
mng_uint32       iSizey;
mng_uint8        iUnit;

  if( mng_getchunk_phyg( hMNG, hChunk,
          &bEmpty, &iSizex, &iSizey, &iUnit ) != 0 )
    return false;

      BARG( "bEmpty", bEmpty );
  NL; IARG( "iSizex", iSizex );
  NL; IARG( "iSizey", iSizey );
  NL; IARG( "iUnit", iUnit ); MI_UNITS( iUnit );

  if( iUnit ) {
      PCOM("X/Y pixels per unit" );
  } else {
    if( iSizex == iSizey ) {
      PCOM( "Square pixels" );
    }
  }

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_pHYs( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_bool         bEmpty;
mng_uint32       iSizex;
mng_uint32       iSizey;
mng_uint8        iUnit;

  if( mng_getchunk_phys( hMNG, hChunk,
          &bEmpty, &iSizex, &iSizey, &iUnit ) != 0 )
    return false;

      BARG( "bEmpty", bEmpty );
  NL; IARG( "iSizex", iSizex );
  NL; IARG( "iSizey", iSizey );
  NL; IARG( "iUnit", iUnit ); MI_UNITS( iUnit );

  if( iUnit ) {
      PCOM("X/Y pixels per unit" );
  } else {
    if( iSizex == iSizey ) {
      PCOM( "Square pixels" );
    }
  }
  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_sBIT( mng_handle hMNG, mng_handle hChunk, String &as )
{ // tested with cs3* cs5* cs8*
mng_bool         bEmpty;
mng_uint8        iType;
mng_uint8arr4    aBits;

  if( mng_getchunk_sbit( hMNG, hChunk, &bEmpty, &iType, &aBits ) != 0 )
    return false;

      BARG( "bEmpty", bEmpty );
  NL; IARG( "iType", iType );
  //aBits aBits @todo@

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_sPLT( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_bool         bEmpty;
mng_uint32       iNamesize;
mng_pchar        zName;
mng_uint8        iSampledepth;
mng_uint32       iEntrycount;
mng_ptr          pEntries;

  if( mng_getchunk_splt( hMNG, hChunk,
          &bEmpty, &iNamesize, &zName,
          &iSampledepth, &iEntrycount, &pEntries ) != 0 )
    return false;

      BARG( "bEmpty", bEmpty );
  NL; IARG( "iNamesize", iNamesize );
  NL; ZARG( "zName", zName );
  NL; IARG( "iSampledepth", iSampledepth ); MI_BITDEPTH( iSampledepth );
  NL; IARG( "iEntrycount", iEntrycount );
  //pEntries pEntries  @todo@ 

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_sRGB( mng_handle hMNG, mng_handle hChunk, String &as )
{
# define MI_RENDERINGINTENT( _i_ )\
  if( WantsMacroIds() ) {\
    switch( (_i_) ) {\
    case MNG_INTENT_PERCEPTUAL            :\
      MI( "MNG_INTENT_PERCEPTUAL" );          break;\
    case MNG_INTENT_RELATIVECOLORIMETRIC  :\
      MI( "MNG_INTENT_RELATIVECOLORIMETRIC" );break;\
    case MNG_INTENT_SATURATION            :\
      MI( "MNG_INTENT_SATURATION" );          break;\
    case MNG_INTENT_ABSOLUTECOLORIMETRIC  :\
      MI( "MNG_INTENT_ABSOLUTECOLORIMETRIC" );break;\
    }\
  }
mng_bool         bEmpty;
mng_uint8        iRenderingintent;

  if( mng_getchunk_srgb( hMNG, hChunk,&bEmpty, &iRenderingintent ) != 0)
    return false;

      BARG( "bEmpty", bEmpty );
  NL; IARG( "iRenderingintent", iRenderingintent );
                                MI_RENDERINGINTENT( iRenderingintent );

  return true;
# undef MI_RENDERINGINTENT
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_tEXt( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint32       iKeywordsize;
mng_pchar        zKeyword;
mng_uint32       iTextsize;
mng_pchar        zText;

  if( mng_getchunk_text( hMNG, hChunk,
          &iKeywordsize, &zKeyword, &iTextsize, &zText ) != 0 )
    return false;

      IARG( "iKeywordsize", iKeywordsize );
  NL; ZARG( "zKeyword", zKeyword );
  NL; IARG( "iTextsize", iTextsize );
  NL; ZARG( "zText", zText );

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_tIME( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint16       iYear;
mng_uint8        iMonth;
mng_uint8        iDay;
mng_uint8        iHour;
mng_uint8        iMinute;
mng_uint8        iSecond;

  if( mng_getchunk_time( hMNG, hChunk,
          &iYear, &iMonth, &iDay, &iHour, &iMinute, &iSecond ) != 0 )
    return false;

      IARG( "iYear", iYear );
  NL; IARG( "iMonth", iMonth );
  NL; IARG( "iDay", iDay );
  NL; IARG( "iHour", iHour );
  NL; IARG( "iMinute", iMinute );
  NL; IARG( "iSecond", iSecond );
  // Do not do help line here - may confuse international readers !

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_tRNS( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_bool         bEmpty;
mng_bool         bGlobal;
mng_uint8        iType;
mng_uint32       iCount;
mng_uint8arr     aAlphas;
mng_uint16       iGray;
mng_uint16       iRed;
mng_uint16       iGreen;
mng_uint16       iBlue;
mng_uint32       iRawlen;
mng_uint8arr     aRawdata;

  if( mng_getchunk_trns( hMNG, hChunk,
        &bEmpty, &bGlobal, &iType, &iCount,
        &aAlphas,
        &iGray, &iRed, &iGreen, &iBlue,
        &iRawlen,
        &aRawdata ) != 0 )
    return false;

      BARG( "bEmpty", bEmpty );
  NL; BARG( "bGlobal", bGlobal );
  NL; IARG( "iType", iType );
  NL; IARG( "iCount", iCount );
//aAlphas aAlphas   @todo@
  NL; IARG( "iGray", iGray );
  NL; IARG( "iRed", iRed );
  NL; IARG( "iGreen", iGreen );
  NL; IARG( "iBlue", iBlue );
  NL; IARG( "iRawlen", iRawlen );
//aRawdata aRawdata  @todo@

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_zTXt( mng_handle hMNG, mng_handle hChunk, String &as )
{
mng_uint32       iKeywordsize;
mng_pchar        zKeyword;
mng_uint8        iCompression;
mng_uint32       iTextsize;
mng_pchar        zText;

  if( mng_getchunk_ztxt( hMNG, hChunk,
    &iKeywordsize, &zKeyword, &iCompression, &iTextsize, &zText ) != 0 )
    return false;

      IARG( "iKeywordsize", iKeywordsize );
  NL; ZARG( "zKeyword", zKeyword );
  NL; IARG( "iCompression", iCompression ); MI_COMPRESSION_DEFLATE( iCompression );
  NL; IARG( "iTextsize", iTextsize );
  NL; ZARG( "zText", zText );

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TMainForm::Info_Unknown( mng_handle hMNG, mng_handle hChunk, String &as )
{
  NL + TAB; STR( "Unknown Chunk" );
  NL + TAB; STR( nl + TAB + "(See help tab for a list of unsupported chunks)!" );

  return true;
}
//---------------------------------------------------------------------------



