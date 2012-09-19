//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include <stdlib.h>
#include <string.h>
#include <mm_malloc.h>

//--------------------------------------
void* dRealMalloc(dsize_t in_size)
{
   return malloc(in_size);
}


//--------------------------------------
void dRealFree(void* in_pFree)
{
   free(in_pFree);
}

void *dMalloc_aligned(dsize_t in_size, int alignment)
{
   return _mm_malloc(in_size, alignment);
}

void dFree_aligned(void* p)
{
   return _mm_free(p);
}

void* dMemcpy(void *dst, const void *src, dsize_t size)
{
   return memcpy(dst,src,size);
}   


//--------------------------------------
void* dMemmove(void *dst, const void *src, dsize_t size)
{
   return memmove(dst,src,size);
}  
 
//--------------------------------------
void* dMemset(void *dst, int c, dsize_t size)
{
   return memset(dst,c,size);   
}   

//--------------------------------------
int dMemcmp(const void *ptr1, const void *ptr2, dsize_t len)
{
   return(memcmp(ptr1, ptr2, len));
}
