MNGPLG
A simple browser plug-in for the MNG image/animation file format.

By Jason Summers  <jason1@pobox.com>
Version 1.0.1  2 Oct 2002
Web site: <http://pobox.com/~jason1/mngplg/>


COPYRIGHT NOTICE

Copyright (c) 2000-2002 by Jason Summers <jason1@pobox.com>

THIS SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT 
ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR 
FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND 
PERFORMANCE OF THE LIBRARY IS WITH YOU.  SHOULD THE LIBRARY PROVE DEFECTIVE, 
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING WILL 
ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR REDISTRIBUTE 
THIS SOFTWARE AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING 
ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE 
USE OR INABILITY TO USE THIS SOFTWARE (INCLUDING BUT NOT LIMITED TO LOSS OF 
DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD 
PARTIES OR A FAILURE OF THIS SOFTWARE TO OPERATE WITH ANY OTHER PROGRAMS), 
EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGES.

Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it 
freely, subject to the following restrictions:

 1. The origin of this source code must not be misrepresented;
    you must not claim that you wrote the original software.
 2. Altered source versions must be plainly marked as such, and must not
    be misrepresented as being the original source.
 3. Altered binary versions must not be misrepresented as being the
    original.
 4. This Copyright notice may not be removed or altered from any source
    or altered source distribution, although you may add a Copyright
    notice for yourself for any code that you have written.


This software uses several third-party libraries (listed below), some of 
which are optional. If you redistribute MNGPLG, it is your responsibility to 
comply with the licenses of any libraries used.

This package includes a compiled executable plug-in file, npmngplg.dll. This 
file includes code from lcms, which is distributed under the LGPL. To the 
best of my understanding, that basically means that anyone distributing that 
file must (1) make it possible for the recipient to modify the plug-in to 
use a new or modified version of lcms, and (2) make available the lcms 
source code. Requirement (1) is satisfied by the inclusion of the source 
code. For requirement (2), you can find out how to get the lcms source code
at the web site listed at the beginning of this document, if necessary.


---------

Based on libmng.
   Copyright (c) 2000,2002 Gerard Juyn (gerard@libmng.com)
   <http://www.libmng.com/>

Uses the zlib compression library.
   (C) 1995-2002 Jean-loup Gailly and Mark Adler

This software is based in part on the work of the Independent JPEG Group.
   Copyright (C) 1991-1998, Thomas G. Lane

Uses the lcms color management library by Martí Maria Saguer.
   (distributed under the GNU LESSER GENERAL PUBLIC LICENSE)

---------

SECURITY NOTICE

Although I've tried to write it carefully, MNGPLG has not had any sort of 
security audit. Due to the nature of plug-ins, it is possible for certain 
types of bugs to exist which may allow remote web sites to take control of 
your computer or do harm to it by sending a carefully constructed data file 
to the plug-in. If you are paranoid about security, you may not wish to 
leave MNGPLG enabled in your browser for an extended period of time.

---------

INTRODUCTION

MNGPLG is a Netscape-style browser plug-in which displays the MNG 
image/animation format. It is configured to claim the following MIME types:

 video/x-mng
 video/mng
 image/x-jng
 image/jng

It claims the file extensions ".mng" and ".jng", but file extensions should 
only apply when no MIME type is available (e.g. on an FTP site, or on your 
local hard disk).

It can also display PNG image files, but it would cause too many problems 
for it to try to claim the PNG data type.

If you are configuring a web server to support MNG and JNG, the correct
MIME types to use are "video/x-mng" and "image/x-jng", since the MIME types
have not, as of this writing, been officially registered.


REQUIREMENTS

MNG requires a 32-bit Windows operating system, and a 32-bit web browser 
that supports Netscape-style plug-ins. For example, it works in Netscape 3 
and higher, Opera 3.51 and higher, and Microsoft Internet Explorer from 
about version 3 to 5.0. (It does not readily work in IE 5.5sp2 and 
higher.) Netscape 6 and higher (and related browsers) include native
support for MNG, so it should not be necessary to use this plug-in.


INSTALLATION

There's no install program. To install it, copy the included "npmngplg.dll" 
file to your browser's "Plugins" folder, then restart your browser.

For Netscape 4.x, the Plugins folder is typically located somewhere like:
C:\Program Files\Netscape\Communicator\Program\Plugins

Note: Windows Explorer, by default, is configured to hide files that end in 
".dll". You should probably change that setting. I'd tell you how, but it's 
different in almost every version of Windows.

In Netscape 4.x, you can verify that the plug-in is installed by choosing 
Help|About Plug-ins from the main menu (with JavaScript enabled).

To uninstall, delete the npmngplg.dll file. It does not create any other 
files. It currently does not write anything to the Windows registry.


HOW TO USE (FOR END USERS)

Right-click on an MNG image as it is being displayed to get a menu with some 
of the usual features.

Right-click and choose "Properties" to display some internal information
about the image. Some images have embedded text information that will be
shown in the "Image comments" area. For technical reasons, some or all of
the comments may not be available until the animation completes a full loop.


HOW TO USE (FOR WEB DEVELOPERS)

First, if at all possible, configure your web server (not browser) to 
assign the MIME type "video/x-mng" to files that end in ".mng", and
assign type "image/x-jng" to files that end in ".jng".

The most reliable way to embed MNG files in a web page is (unfortunately)
to use the  nonstandard <embed> tag. For example:

<embed src="foo.mng" width="100" height="100" type="video/x-mng">

The src, width, and height attributes are required. Width and height should 
match the actual width and height of the image.

Transparency is not supported, and probably never will be. However, you can 
supply a background color to use in transparent areas by using the BGCOLOR 
attribute in the EMBED tag, i.e.:

<embed src="foo.mng" bgcolor="#ff0000"  ...>

You cannot use color names like "red"; you must use the hexadecimal format 
as in the example.

An image can be made into a "hotlink" by including an HREF and optionally a 
TARGET attribute in the EMBED tag. For example:

<embed src="foo.mng" href="http://www.libpng.org/pub/mng/" target="_blank" 
...>


SOURCE CODE

The C source code is included. I've only tested it with libmng 1.0.5,
but it's probably also compatible with other versions, maybe with
minor changes.

To compile it, you'll need:

- libmng MNG library <http://www.libmng.com/>.

libmng in turn uses some other libraries:

    - zlib compression library

    - IJG JPEG library

    - [optional] lcms "Little Color Management System" library. 

If you include lcms, turn on the MNG_FULL_CMS option in libmng_conf.h 
before compiling. Note that lcms is distributed under the LGPL -- be sure 
you understand the implications of that before distributing any resulting 
executable files.

If you don't include lcms, comment out the "#define MNGPLG_CMS" line in 
npmngplg.c.

I also recommend turning on the MNG_ERROR_TELLTALE and
MNG_SUPPORT_DYNAMICMNG options in libmng_conf.h.

The files from the Netscape plug-in SDK are no longer needed as of MNGPNG
0.9.4.

Make sure to include the npmngplg.def file in your project, or declare the 
necessary DLL entry points in some other way.
