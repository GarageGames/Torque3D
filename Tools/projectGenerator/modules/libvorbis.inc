<?php
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

// NOTE:  This all the source in the libvorbis library which
// according to the README includes libvorbis, libvorbisfile,
// and libvorbisenc.
//
// We really only need libvorbis and libvorbisfile for loading
// ogg vorbis files, but the source is distrubuted all in one
// folder and the project generator does not allow excluding 
// files.
// 
// In an effort to make upgrading this library simple the
// the inclusion of the encoder is acceptable.  It should
// be filtered out of the build by the linker.

addLibSrcDir( 'libvorbis/lib' );
addLibSrcDir( 'libvorbis/lib/books/coupled' );
addLibSrcDir( 'libvorbis/lib/books/floor' );
addLibSrcDir( 'libvorbis/lib/books/uncoupled' );
addLibSrcDir( 'libvorbis/lib/modes' );
addLibSrcDir( 'libvorbis/include/vorbis' );

addLibIncludePath( 'libvorbis/lib' );
addLibIncludePath( 'libvorbis/include' );
addLibIncludePath( 'libogg/include' );

?>