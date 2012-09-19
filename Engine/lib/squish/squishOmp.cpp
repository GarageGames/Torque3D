/* -----------------------------------------------------------------------------

Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the 
"Software"), to	deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to 
permit persons to whom the Software is furnished to do so, subject to 
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

-------------------------------------------------------------------------- */
#include <squish.h>

//#define ENABLE_OPEN_MP

#ifndef ENABLE_OPEN_MP
void squish::CompressImageOMP(u8 const* rgba, int width, int height, void* blocks, int flags)
{
   squish::CompressImage( rgba, width, height, blocks, flags );
}

#else

// OMP implementation
#include <omp.h>

// OpenMP implementation of squish::CompressImage
//
// If you have any fixes, improvements, suggestions of: "L2OMP n00b" 
// please send them to:
// Pat Wilson
// patw@garagegames.com
namespace squish {

struct _blk_row
{
   unsigned int pxl[4];
};

void CompressImageOMP( u8 const* rgba, int width, int height, void* blocks, int flags )
{
   // fix any bad flags
   flags = FixFlags( flags );

   // Should really assert here or something
   if( width % 4 || height % 4 )
      return;

   // initialize the block output
   u8 *const targetBlock = reinterpret_cast<u8 *>( blocks );
   const int bytesPerBlock = ( ( flags & kDxt1 ) != 0 ) ? 8 : 16;
   const int blockHeight = height >> 2;
   const int blockWidth = width >> 2;

#pragma omp parallel 
{  // begin omp block

   // loop over blocks
#pragma omp for
   for( int by = 0; by < blockHeight; by++ )
   {
      const int y = by * 4;

      for( int bx = 0; bx < blockWidth; bx++ )
      {
         const int x = bx * 4;

         // build the 4x4 block of pixels
         u8 sourceRgba[16 * 4];

#define _load_row(r) (reinterpret_cast<_blk_row *>(sourceRgba))[r] = (*reinterpret_cast<const _blk_row *>(rgba + 4 * ( width * (y + r) + x )))
         _load_row(0);
         _load_row(1);
         _load_row(2);
         _load_row(3);
#undef _load_row

         // compress it into the output
         const int blockIdx = by * blockWidth + bx;
         Compress( sourceRgba, &targetBlock[blockIdx * bytesPerBlock], flags );
      }
   }

} // end omp block

}

} // namespace
#endif
