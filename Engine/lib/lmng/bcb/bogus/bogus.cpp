/* ************************************************************************** */
/* *                                                                        * */
/* * COPYRIGHT NOTICE:                                                      * */
/* *                                                                        * */
/* * Copyright (c) 2000,2002 Gerard Juyn (gerard@libmng.com)                * */
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
/* * project   : bogus                                                      * */
/* * file      : bogus.cpp                 copyright (c) 2000,2002 G.Juyn   * */
/* * version   : 1.0.1                                                      * */
/* *                                                                        * */
/* * purpose   : main project file                                          * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : bogus is (quite literally) a bogus sample which creates and* */
/* *             writes a totally valid, be it somewhat trivial, MNG-file   * */
/* *                                                                        * */
/* * changes   : 0.5.3 - 06/26/2000 - G.Juyn                                * */
/* *             - changed userdata variable to mng_ptr                     * */
/* *             0.5.3 - 06/28/2000 - G.Juyn                                * */
/* *             - changed memory allocation size parameters to mng_size_t  * */
/* *                                                                        * */
/* *             1.0.1 - 10/07/2002 - G.Juyn                                * */
/* *             - fixed copyright notice                                   * */
/* *             - updated MHDR simplicity flag                             * */
/* *                                                                        * */
/* ************************************************************************** */

#pragma hdrstop
#include <condefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>

#include "libmng.h"

/* ************************************************************************** */

USERES("bogus.res");
USELIB("..\win32dll\libmng.lib");
//---------------------------------------------------------------------------
typedef struct user_struct {

          FILE *hFile;                 /* file handle */

        } userdata;

typedef userdata * userdatap;

/* ************************************************************************** */

#define MY_DECL __stdcall              /* get the right callback convention */

/* ************************************************************************** */

mng_ptr MY_DECL myalloc (mng_size_t iSize)
{
  return (mng_ptr)calloc (1, iSize);   /* duh! */
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

mng_bool MY_DECL mywritedata (mng_handle hMNG,
                              mng_ptr    pBuf,
                              mng_uint32 iSize,
                              mng_uint32 *iWritten)
{                                      /* get to my file handle */
  userdatap pMydata = (userdatap)mng_get_userdata (hMNG);
                                       /* write it */
  *iWritten = fwrite (pBuf, 1, iSize, pMydata->hFile);
                                       /* iWritten will indicate errors */
  return MNG_TRUE;
}

/* ************************************************************************** */

int makeimage (char * zFilename)
{
  userdatap pMydata;
  mng_handle hMNG;
  mng_retcode iRC;
                                       /* get a data buffer */
  pMydata = (userdatap)calloc (1, sizeof (userdata));

  if (pMydata == NULL)                 /* oke ? */
  {
    fprintf (stderr, "Cannot allocate a data buffer.\n");
    return 1;
  }
                                       /* can we open the file ? */
  if ((pMydata->hFile = fopen (zFilename, "wb")) == NULL)
  {                                    /* error out if we can't */
    fprintf (stderr, "Cannot open output file %s.\n", zFilename);
    return 1;
  }
                                       /* let's initialize the library */
  hMNG = mng_initialize ((mng_ptr)pMydata, myalloc, myfree, MNG_NULL);

  if (!hMNG)                           /* did that work out ? */
  {
    fprintf (stderr, "Cannot initialize libmng.\n");
    iRC = 1;
  }
  else
  {                                    /* setup callbacks */
    if ( ((iRC = mng_setcb_openstream  (hMNG, myopenstream )) != 0) ||
         ((iRC = mng_setcb_closestream (hMNG, myclosestream)) != 0) ||
         ((iRC = mng_setcb_writedata   (hMNG, mywritedata  )) != 0)    )
      fprintf (stderr, "Cannot set callbacks for libmng.\n");
    else
    {                                  /* create the file in memory */
      if ( ((iRC = mng_create        (hMNG)                                                    ) != 0) ||
           ((iRC = mng_putchunk_mhdr (hMNG, 640, 480, 1000, 3, 1, 3, 0x0047)                   ) != 0) ||
           ((iRC = mng_putchunk_basi (hMNG, 640, 160, 8, 2, 0, 0, 0, 0xFF, 0x00, 0x00, 0xFF, 1)) != 0) ||
           ((iRC = mng_putchunk_iend (hMNG)                                                    ) != 0) ||
           ((iRC = mng_putchunk_defi (hMNG, 0, 0, 0, MNG_TRUE, 0, 160, MNG_FALSE, 0, 0, 0, 0  )) != 0) ||
           ((iRC = mng_putchunk_basi (hMNG, 640, 160, 8, 2, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 1)) != 0) ||
           ((iRC = mng_putchunk_iend (hMNG)                                                    ) != 0) ||
           ((iRC = mng_putchunk_defi (hMNG, 0, 0, 0, MNG_TRUE, 0, 320, MNG_FALSE, 0, 0, 0, 0  )) != 0) ||
           ((iRC = mng_putchunk_basi (hMNG, 640, 160, 8, 2, 0, 0, 0, 0x00, 0x00, 0xFF, 0xFF, 1)) != 0) ||
           ((iRC = mng_putchunk_iend (hMNG)                                                    ) != 0) ||
           ((iRC = mng_putchunk_mend (hMNG)                                                    ) != 0)    )
        fprintf (stderr, "Cannot create the chunks for the image.\n");
      else
      {
        if ((iRC = mng_write (hMNG)) != 0)
          fprintf (stderr, "Cannot write the image.\n");

      }
    }

    mng_cleanup (&hMNG);               /* cleanup the library */
  }

  fclose (pMydata->hFile);             /* cleanup */
  free (pMydata);

  return iRC;
}

/* ************************************************************************** */

#pragma argsused
int main (int argc, char *argv[])
{
  return makeimage ("dutch.mng");
}

/* ************************************************************************** */
