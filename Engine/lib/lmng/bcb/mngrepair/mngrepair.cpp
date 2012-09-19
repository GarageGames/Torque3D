/* ************************************************************************** */
/* *                                                                        * */
/* * COPYRIGHT NOTICE:                                                      * */
/* *                                                                        * */
/* * Copyright (c) 2000-2002 Gerard Juyn (gerard@libmng.com)                * */
/* * [You may insert additional notices after this sentence if you modify   * */
/* *  this source]                                                          * */
/* *                                                                        * */
/* * For the purposes of this copyright and license, "Contributing Authors" * */
/* * is defined as the following set of individuals:                        * */
/* *                                                                        * */
/* *    Gerard Juyn                                                         * */
/* *    (hopefully some more to come...)                                    * */
/* *                                                                        * */
/* * The MNG Library is supplied "AS IS".  The Contributing Authors         * */
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
/* *                                                                        * */
/* * project   : mngrepair                                                  * */
/* * file      : mngrepair.cpp             copyright (c) 2002 G.Juyn        * */
/* * version   : 1.0.0                                                      * */
/* *                                                                        * */
/* * purpose   : main project file                                          * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : mngrepair iterates tries to fix a couple of 'common'       * */
/* *             MNG encoding errors (such as in JASC Animation Shop)       * */
/* *                                                                        * */
/* * changes   :                                                            * */
/* *                                                                        * */
/* ************************************************************************** */

#define HAVE_BOOLEAN

#include <windows.h>
#pragma hdrstop
#include <condefs.h>

#include "libmng.h"

/* ************************************************************************** */

USERES("mngrepair.res");
USEUNIT("..\..\libmng_hlapi.c");
USEUNIT("..\..\libmng_callback_xs.c");
USEUNIT("..\..\libmng_prop_xs.c");
USEUNIT("..\..\libmng_chunk_xs.c");
USEUNIT("..\..\libmng_object_prc.c");
USEUNIT("..\..\libmng_chunk_prc.c");
USEUNIT("..\..\libmng_chunk_io.c");
USEUNIT("..\..\libmng_read.c");
USEUNIT("..\..\libmng_write.c");
USEUNIT("..\..\libmng_display.c");
USEUNIT("..\..\libmng_dither.c");
USEUNIT("..\..\libmng_pixels.c");
USEUNIT("..\..\libmng_filter.c");
USEUNIT("..\..\libmng_error.c");
USEUNIT("..\..\libmng_trace.c");
USEUNIT("..\..\libmng_cms.c");
USEUNIT("..\..\libmng_zlib.c");
USEUNIT("..\..\libmng_jpeg.c");
USEUNIT("..\..\..\zlib\adler32.c");
USEUNIT("..\..\..\zlib\compress.c");
USEUNIT("..\..\..\zlib\crc32.c");
USEUNIT("..\..\..\zlib\deflate.c");
USEUNIT("..\..\..\zlib\infblock.c");
USEUNIT("..\..\..\zlib\infcodes.c");
USEUNIT("..\..\..\zlib\inffast.c");
USEUNIT("..\..\..\zlib\inflate.c");
USEUNIT("..\..\..\zlib\inftrees.c");
USEUNIT("..\..\..\zlib\infutil.c");
USEUNIT("..\..\..\zlib\trees.c");
USEUNIT("..\..\..\zlib\uncompr.c");
USEUNIT("..\..\..\zlib\zutil.c");
//---------------------------------------------------------------------------
typedef struct user_struct {

          FILE        *hFileI;         /* input file handle */
          FILE        *hFileO;         /* output file handle */
          mng_handle  hHandleI;        /* input mng handle */
          mng_handle  hHandleO;        /* output mng handle */
          mng_bool    bHasSAVE;        /* indicates a SAVE in the input */
          mng_bool    bHasTERM;        /* indicates we saved the TERM */
          mng_bool    bIsJASC;         /* indicates if this is an AS file */
          mng_chunkid iLastchunk;      /* last processed chunk */
          mng_retcode iError;          /* errorcode from function in callback */
          mng_uint8   iTermaction;     /* saved TERM parameters */
          mng_uint8   iIteraction;
          mng_uint32  iDelay;
          mng_uint32  iItermax;

        } userdata;

typedef userdata * userdatap;

/* ************************************************************************** */

#define MY_DECL                        /* get the right callback convention */

/* ************************************************************************** */

mng_ptr MY_DECL myalloc (mng_size_t iSize)
{                                      /* duh! */
  return (mng_ptr)calloc (1, (size_t)iSize);
}

/* ************************************************************************** */

#pragma argsused
void MY_DECL myfree (mng_ptr pPtr, mng_size_t iSize)
{
  free (pPtr);                         /* duh! */
  return;
}

/* ************************************************************************** */

#pragma argsused
mng_bool MY_DECL myopenstream (mng_handle hMNG)
{
  return MNG_TRUE;                     /* already opened in main function */
}

/* ************************************************************************** */

#pragma argsused
mng_bool MY_DECL myclosestream (mng_handle hMNG)
{
  return MNG_TRUE;                     /* gets closed in main function */
}

/* ************************************************************************** */

mng_bool MY_DECL myreaddata (mng_handle hMNG,
                             mng_ptr    pBuf,
                             mng_uint32 iSize,
                             mng_uint32 *iRead)
{                                      /* get to my file handle */
  userdatap pMydata = (userdatap)mng_get_userdata (hMNG);
                                       /* read it */
  *iRead = fread (pBuf, 1, iSize, pMydata->hFileI);
                                       /* iRead will indicate EOF */
  return MNG_TRUE;
}

/* ************************************************************************** */

mng_bool MY_DECL mywritedata (mng_handle hMNG,
                              mng_ptr    pBuf,
                              mng_uint32 iSize,
                              mng_uint32 *iWritten)
{                                      /* get to my file handle */
  userdatap pMydata = (userdatap)mng_get_userdata (hMNG);
                                       /* write it */
  *iWritten = fwrite (pBuf, 1, iSize, pMydata->hFileO);
                                       /* iWritten will indicate errors */
  return MNG_TRUE;
}

/* ************************************************************************** */

mng_bool MY_DECL myprocesserror (mng_handle  hMNG,
                                 mng_int32   iErrorcode,
                                 mng_int8    iSeverity,
                                 mng_chunkid iChunkname,
                                 mng_uint32  iChunkseq,
                                 mng_int32   iExtra1,
                                 mng_int32   iExtra2,
                                 mng_pchar   zErrortext)
{                                      /* sequence error for TERM we ignore ! */
  if ((iErrorcode == MNG_SEQUENCEERROR) && (iChunkname == MNG_UINT_TERM))
    return MNG_TRUE;

  return MNG_FALSE;                    /* all others get dumped ! */
}

/* ************************************************************************** */

#pragma argsused
mng_bool MY_DECL myiterchunk (mng_handle  hMNG,
                              mng_handle  hChunk,
                              mng_chunkid iChunktype,
                              mng_uint32  iChunkseq)
{
  mng_uint32  iWidth;                  /* temps for IHDR processing */
  mng_uint32  iHeight;
  mng_uint8   iBitdepth;
  mng_uint8   iColortype;
  mng_uint8   iCompression;
  mng_uint8   iFilter;
  mng_uint8   iInterlace;
  
  mng_bool    bEmpty;                  /* temps for FRAM processing */
  mng_uint8   iMode;
  mng_uint32  iNamesize;
  mng_pchar   zName;
  mng_uint8   iChangedelay;
  mng_uint8   iChangetimeout;
  mng_uint8   iChangeclipping;
  mng_uint8   iChangesyncid;
  mng_uint32  iDelay;
  mng_uint32  iTimeout;
  mng_uint8   iBoundarytype;
  mng_int32   iBoundaryl;
  mng_int32   iBoundaryr;
  mng_int32   iBoundaryt;
  mng_int32   iBoundaryb;
  mng_uint32  iCount;
  mng_uint32p pSyncids;
                                       /* get to my userdata */
  userdatap  pMydata = (userdatap)mng_get_userdata (hMNG);

  if (pMydata->hFileO)                 /* are we writing this time ? */
  {                                    /* do we need to 'forget' the TERM ? */
    if ((iChunktype == MNG_UINT_TERM) && (pMydata->bHasTERM))
      ;
    else
    {                                  /* fix JASC AS frame_type ? */
      if ((iChunktype == MNG_UINT_FRAM) && (pMydata->bIsJASC))
      {
        if ((pMydata->iError = mng_getchunk_fram (hMNG, hChunk,
                                                  &bEmpty, &iMode, &iNamesize, &zName,
                                                  &iChangedelay, &iChangetimeout,
                                                  &iChangeclipping, &iChangesyncid,
                                                  &iDelay, &iTimeout, &iBoundarytype,
                                                  &iBoundaryl, &iBoundaryr,
                                                  &iBoundaryt, &iBoundaryb,
                                                  &iCount, &pSyncids)) != 0)
        {
          fprintf (stderr, "Cannot get FRAM fields.\n");
          return MNG_FALSE;            /* stop the process ! */
        }

        if (iMode == 1)                /* really ? */
          iMode = 3;

        if ((pMydata->iError = mng_putchunk_fram (pMydata->hHandleO,
                                                  bEmpty, iMode, iNamesize, zName,
                                                  iChangedelay, iChangetimeout,
                                                  iChangeclipping, iChangesyncid,
                                                  iDelay, iTimeout, iBoundarytype,
                                                  iBoundaryl, iBoundaryr,
                                                  iBoundaryt, iBoundaryb,
                                                  iCount, pSyncids)) != 0)
        {
          fprintf (stderr, "Cannot write FRAM chunk.\n");
          return MNG_FALSE;            /* stop the process ! */
        }
      }
      else
      {
        if ((pMydata->iError = mng_copy_chunk (hMNG, hChunk, pMydata->hHandleO)) != 0)
        {
          fprintf (stderr, "Cannot copy the chunk.\n");
          return MNG_FALSE;            /* stop the process ! */
        }
      }
    }
                                       /* need to insert TERM in the proper place ? */
    if ((iChunktype == MNG_UINT_MHDR) && (pMydata->bHasTERM))
    {
      if ((pMydata->iError = mng_putchunk_term (pMydata->hHandleO,
                                                pMydata->iTermaction,
                                                pMydata->iIteraction,
                                                pMydata->iDelay,
                                                pMydata->iItermax)) != 0)
      {
        fprintf (stderr, "Cannot write TERM chunk.\n");
        return MNG_FALSE;              /* stop the process ! */
      }
    }
  }
  else                                 /* first iteration; just checking stuff */
  {
    if (iChunktype == MNG_UINT_SAVE)  /* SAVE ? */
    {
      pMydata->bHasSAVE = MNG_TRUE;    /* we got a SAVE ! */
      pMydata->bIsJASC  = MNG_FALSE;   /* so it's definitely not an invalid AS file */
    }
    else
    {                                 /* TERM ? */
      if (iChunktype == MNG_UINT_TERM)
      {                               /* is it in the wrong place ? */
        if ((pMydata->iLastchunk != MNG_UINT_MHDR) ||
            (pMydata->iLastchunk != MNG_UINT_SAVE)    )
        {
          pMydata->bHasTERM = MNG_TRUE;

          if ((pMydata->iError = mng_getchunk_term (hMNG, hChunk,
                                                    &pMydata->iTermaction,
                                                    &pMydata->iIteraction,
                                                    &pMydata->iDelay,
                                                    &pMydata->iItermax)) != 0)
          {
            fprintf (stderr, "Cannot get TERM fields.\n");
            return MNG_FALSE;          /* stop the process ! */
          }
        }
      }
      else
      {                                /* IHDR ? */
        if (iChunktype == MNG_UINT_IHDR)
        {
          if ((pMydata->iError = mng_getchunk_ihdr (hMNG, hChunk,
                                                    &iWidth, &iHeight, &iBitdepth,
                                                    &iColortype, &iCompression,
                                                    &iFilter, &iInterlace)) != 0)
          {
            fprintf (stderr, "Cannot get IHDR fields.\n");
            return MNG_FALSE;          /* stop the process ! */
          }
                                       /* is it not a typical JASC AS file */
          if ((iBitdepth != 8) || (iColortype != 6))
            pMydata->bIsJASC = MNG_FALSE;
        }
      }
    }

    pMydata->iLastchunk = iChunktype;
  }

  return MNG_TRUE;                     /* keep'm coming... */
}

/* ************************************************************************** */

int fixit (char * zFilenameI,
           char * zFilenameO)
{
  userdatap pMydata;
  mng_retcode iRC;
                                       /* get a data buffer */
  pMydata = (userdatap)calloc (1, sizeof (userdata));

  if (pMydata == NULL)                 /* oke ? */
  {
    fprintf (stderr, "Cannot allocate a data buffer.\n");
    return 1;
  }

  pMydata->hFileO      = 0;            /* initialize some stuff! */
  pMydata->hHandleI    = MNG_NULL;
  pMydata->hHandleO    = MNG_NULL;
  pMydata->bHasSAVE    = MNG_FALSE;
  pMydata->bHasTERM    = MNG_FALSE;
  pMydata->bIsJASC     = MNG_TRUE;
  pMydata->iLastchunk  = MNG_UINT_HUH;
  pMydata->iTermaction = 0;
  pMydata->iIteraction = 0;
  pMydata->iDelay      = 0;
  pMydata->iItermax    = 0;
                                       /* can we open the input file ? */
  if ((pMydata->hFileI = fopen (zFilenameI, "rb")) == NULL)
  {                                    /* error out if we can't */
    fprintf (stderr, "Cannot open input file %s.\n", zFilenameI);
    return 1;
  }
                                       /* let's initialize the library */
  pMydata->hHandleI = mng_initialize ((mng_ptr)pMydata, myalloc, myfree, MNG_NULL);

  if (!pMydata->hHandleI)              /* did that work out ? */
  {
    fprintf (stderr, "Cannot initialize libmng.\n");
    iRC = 1;
  }
  else
  {                                    /* some informatory messages */
    fprintf (stderr, "Compiled with libmng %s.\n", MNG_VERSION_TEXT);
    fprintf (stderr, "Running with libmng %s.\n", mng_version_text());
                                       /* setup callbacks */
    if ( ((iRC = mng_setcb_openstream  (pMydata->hHandleI, myopenstream  )) != 0) ||
         ((iRC = mng_setcb_closestream (pMydata->hHandleI, myclosestream )) != 0) ||
         ((iRC = mng_setcb_readdata    (pMydata->hHandleI, myreaddata    )) != 0) ||
         ((iRC = mng_setcb_errorproc   (pMydata->hHandleI, myprocesserror)) != 0)    )
      fprintf (stderr, "Cannot set callbacks for libmng.\n");
    else
    {                                  /* reaad the file into memory */
      if ((iRC = mng_read (pMydata->hHandleI)) != 0)
        fprintf (stderr, "Cannot read the input file.\n");
      else
      {                                /* run through the chunk list to get TERM */
        if ((iRC = mng_iterate_chunks (pMydata->hHandleI, 0, myiterchunk)) != 0)
          fprintf (stderr, "Cannot iterate the chunks.\n");
        else
        {
          if (pMydata->iError)         /* did the iteration fail somehow ? */
            iRC = pMydata->iError;
          else
          {                            /* can we open the output file ? */
            if ((pMydata->hFileO = fopen (zFilenameO, "wb")) == NULL)
            {                            /* error out if we can't */
              fprintf (stderr, "Cannot open output file %s.\n", zFilenameO);
              iRC = 1;
            }
            else
            {                          /* let's initialize the library */
              pMydata->hHandleO = mng_initialize ((mng_ptr)pMydata, myalloc, myfree, MNG_NULL);

              if (!pMydata->hHandleO)  /* did that work out ? */
              {
                fprintf (stderr, "Cannot initialize libmng.\n");
                iRC = 1;
              }
              else
              {                        /* setup callbacks */
                if ( ((iRC = mng_setcb_openstream  (pMydata->hHandleO, myopenstream )) != 0) ||
                     ((iRC = mng_setcb_closestream (pMydata->hHandleO, myclosestream)) != 0) ||
                     ((iRC = mng_setcb_writedata   (pMydata->hHandleO, mywritedata  )) != 0)    )
                  fprintf (stderr, "Cannot set callbacks for libmng.\n");
                else
                {
                  if ((iRC = mng_create (pMydata->hHandleO)) != 0)
                    fprintf (stderr, "Cannot create a new MNG.\n");
                  else
                  {                    /* run through the chunk again and create the new file */
                    if ((iRC = mng_iterate_chunks (pMydata->hHandleI, 0, myiterchunk)) != 0)
                      fprintf (stderr, "Cannot iterate the chunks.\n");
                    else
                    {                  /* did the iteration fail somehow ? */
                      if (pMydata->iError)
                        iRC = pMydata->iError;
                      else
                      {                /* now write the created new file !! */
                        if ((iRC = mng_write (pMydata->hHandleO)) != 0)
                          fprintf (stderr, "Cannot write the output file.\n");
                      }
                    }
                  }
                }
                                       /* cleanup the library */
                mng_cleanup (&pMydata->hHandleO);
              }
                                       /* cleanup output file */
              fclose (pMydata->hFileO);
            }
          }
        }
      }
    }

    mng_cleanup (&pMydata->hHandleI);  /* cleanup the library */
  }

  fclose (pMydata->hFileI);            /* cleanup input file and userdata */
  free (pMydata);

  return iRC;
}

/* ************************************************************************** */

int main(int argc, char *argv[])
{
  if (argc > 2)                  /* need two (2) parameters ! */
    return fixit (argv[1], argv[2]);
  else
    fprintf (stdout, "\nUsage: mngrepair <input.mng> <output.mng>\n\n");

  return 0;
}

/* ************************************************************************** */
