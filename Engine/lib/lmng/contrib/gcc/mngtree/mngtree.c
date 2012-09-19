/* ************************************************************************** */
/* *                                                                        * */
/* * COPYRIGHT NOTICE:                                                      * */
/* *                                                                        * */
/* * Copyright (c) 2000 Gerard Juyn (gerard :at: libmng.com)                * */
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
/* * project   : mngtree                                                    * */
/* * file      : mngtree.c                 copyright (c) 2000 G.Juyn        * */
/* * version   : 1.0.1                                                      * */
/* *                                                                        * */
/* * purpose   : main project file                                          * */
/* *                                                                        * */
/* * author    : G.Juyn                                                     * */
/* * web       : http://www.3-t.com                                         * */
/* * email     : mailto:info@3-t.com                                        * */
/* *                                                                        * */
/* * comment   : mngtree simply dumps the chunk-structure of the supplied   * */
/* *             first parameter to stdout (should be a xNG-file)           * */
/* *                                                                        * */
/* * changes   : 0.5.2 - 06/03/2000 - G.Juyn                                * */
/* *             - fixed for compilation under Linux                        * */
/* *             0.5.3 - 06/26/2000 - G.Juyn                                * */
/* *             - changed definition of userdata to mng_ptr                * */
/* *             0.5.3 - 06/28/2000 - G.Juyn                                * */
/* *             - changed memory allocation size parameters to mng_size_t  * */
/* *                                                                        * */
/* *             1.0.1 - 12/07/2003 - G.Juyn                                * */
/* *             - fixed inclusion of libmng.h (Thanks Raphael)             * */
/* *                                                                        * */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "../../../libmng.h"

/* ************************************************************************** */

typedef struct user_struct {

          FILE *hFile;                 /* file handle */
          int  iIndent;                /* for nice indented formatting */

        } userdata;

typedef userdata * userdatap;

/* ************************************************************************** */

mng_ptr myalloc (mng_size_t iSize)
{                                      /* duh! */
  return (mng_ptr)calloc (1, (size_t)iSize);
}

/* ************************************************************************** */

void myfree (mng_ptr pPtr, mng_size_t iSize)
{
  free (pPtr);                         /* duh! */
  return;
}

/* ************************************************************************** */

mng_bool myopenstream (mng_handle hMNG)
{
  return MNG_TRUE;                     /* already opened in main function */
}

/* ************************************************************************** */

mng_bool myclosestream (mng_handle hMNG)
{
  return MNG_TRUE;                     /* gets closed in main function */
}

/* ************************************************************************** */

mng_bool myreaddata (mng_handle hMNG,
                     mng_ptr    pBuf,
                     mng_uint32 iSize,
                     mng_uint32 *iRead)
{                                      /* get to my file handle */
  userdatap pMydata = (userdatap)mng_get_userdata (hMNG);
                                       /* read it */
  *iRead = fread (pBuf, 1, iSize, pMydata->hFile);
                                       /* iRead will indicate EOF */
  return MNG_TRUE;
}

/* ************************************************************************** */

mng_bool myiterchunk (mng_handle  hMNG,
                      mng_handle  hChunk,
                      mng_chunkid iChunktype,
                      mng_uint32  iChunkseq)
{                                      /* get to my file handle */
  userdatap pMydata = (userdatap)mng_get_userdata (hMNG);
  char aCh[4];
  char zIndent[80];
  int iX;
                                       /* decode the chunkname */
  aCh[0] = (char)((iChunktype >> 24) & 0xFF);
  aCh[1] = (char)((iChunktype >> 16) & 0xFF);
  aCh[2] = (char)((iChunktype >>  8) & 0xFF);
  aCh[3] = (char)((iChunktype      ) & 0xFF);
                                       /* indent less ? */
  if ( (iChunktype == MNG_UINT_MEND) || (iChunktype == MNG_UINT_IEND) ||
       (iChunktype == MNG_UINT_ENDL) )
    pMydata->iIndent -= 2;
                                       /* this looks ugly; but I haven't
                                          figured out how to do it prettier */
  for (iX = 0; iX < pMydata->iIndent; iX++)
    zIndent[iX] = ' ';
  zIndent[pMydata->iIndent] = '\0';
                                       /* print a nicely indented line */
  printf ("%s%c%c%c%c\n", zIndent, aCh[0], aCh[1], aCh[2], aCh[3]);
                                       /* indent more ? */
  if ( (iChunktype == MNG_UINT_MHDR) || (iChunktype == MNG_UINT_IHDR) ||
       (iChunktype == MNG_UINT_JHDR) || (iChunktype == MNG_UINT_DHDR) ||
       (iChunktype == MNG_UINT_BASI) || (iChunktype == MNG_UINT_LOOP)    )
    pMydata->iIndent += 2;

  return MNG_TRUE;                     /* keep'm coming... */
}

/* ************************************************************************** */

int dumptree (char * zFilename)
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
  if ((pMydata->hFile = fopen (zFilename, "rb")) == NULL)
  {                                    /* error out if we can't */
    fprintf (stderr, "Cannot open input file %s.\n", zFilename);
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
         ((iRC = mng_setcb_readdata    (hMNG, myreaddata   )) != 0)    )
      fprintf (stderr, "Cannot set callbacks for libmng.\n");
    else
    {                                  /* read the file into memory */
      if ((iRC = mng_read (hMNG)) != 0)
        fprintf (stderr, "Cannot read the file.\n");
      else
      {
        pMydata->iIndent = 2;          /* start of the indenting at a nice level */

        printf ("Starting dump of %s.\n\n", zFilename);
                                       /* run through the chunk list */
        if ((iRC = mng_iterate_chunks (hMNG, 0, myiterchunk)) != 0)
          fprintf (stderr, "Cannot iterate the chunks.\n");

        printf ("\nDone.\n");
      }
    }

    mng_cleanup (&hMNG);               /* cleanup the library */
  }

  fclose (pMydata->hFile);             /* cleanup */
  free (pMydata);

  return iRC;
}

/* ************************************************************************** */

int main(int argc, char *argv[])
{
        if (argc > 1)                  /* need that first parameter ! */
          return dumptree (argv[1]);
        else
          printf ("\nUsage: mngtree <file.mng>\n\n");  

        return 0;
}

/* ************************************************************************** */
