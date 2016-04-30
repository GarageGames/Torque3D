# Native File Dialog #

A tiny, neat C library that portably invokes native file open and save dialogs.  Write dialog code once and have it pop up native dialogs on all supported platforms.  Avoid linking large dependencies like wxWidgets and qt.

Features:

 - Lean C API, static library -- no ObjC, no C++, no STL.
 - Zlib licensed.
 - Consistent UTF-8 support on all platforms.
 - Simple universal file filter syntax.
 - Paid support available.
 - Multiple file selection support.
 - 64-bit and 32-bit friendly.
 - GCC, Clang and Visual Studio supported.
 - No third party dependencies.
 - Support for Vista's modern `IFileDialog` on Windows.
 - Support for non-deprecated Cocoa APIs on OS X.
 - GTK+3 dialog on Linux.
 - Tested, works alongside [http://www.libsdl.org](SDL2) on all platforms, for the game developers out there.

# Example Usage #

```C
#include <nfd.h>
#include <stdio.h>
#include <stdlib.h>

int main( void )
{
    nfdchar_t *outPath = NULL;
    nfdresult_t result = NFD_OpenDialog( NULL, NULL, &outPath );
        
    if ( result == NFD_OKAY ) {
        puts("Success!");
        puts(outPath);
        free(outPath);
    }
    else if ( result == NFD_CANCEL ) {
        puts("User pressed cancel.");
    }
    else {
        printf("Error: %s\n", NFD_GetError() );
    }

    return 0;
}
```

See [NFD.h](src/include/nfd.h) for more options.

# Screenshots #

![Windows 8 rendering an IFileOpenDialog](screens/open_win8.png?raw=true)
![GTK3 on Linux](screens/open_gtk3.png?raw=true)
![Cocoa on Yosemite](screens/open_cocoa.png?raw=true)


## Building ##

NFD uses [SCons](http://www.scons.org) for cross-platform builds.  After installing SCons, build it with:

    cd src
    scons debug=[0,1]

Alternatively, you can avoid Scons by just including NFD files to your existing project:

 1. Add all header files in `src/` and `src/include` to your project.
 2. Add `src/include` to your include search path or copy it into your existing search path.
 3. Add `src/nfd_common.c` to your project.
 4. Add `src/nfd_<platform>` to your project, where `<platform>` is the NFD backend for the platform you are fixing to build.
 5. On Visual Studio, define `_CRT_SECURE_NO_WARNINGS` to avoid warnings.

### Compiling Your Programs ###

 1. Add `src/include` to your include search path.
 2. Add `nfd.lib` to the list of list of static libraries to link against.
 3. Add `src/` to the library search path.

On Linux, you must compile and link against GTK+.  Recommend use of `pkg-config --cflags --libs gtk+-3.0`.

On Mac OS X, add `AppKit` to the list of frameworks.

On Windows, ensure you are building against `comctl32.lib`.

## Usage ##

See `NFD.h` for API calls.  See `tests/*.c` for example code.

See `tests/SConstruct` for a working build script that compiles on all platforms.

## File Filter Syntax ##

There is a form of file filtering in every file dialog, but no consistent means of supporting it.  NFD provides support for filtering files by groups of extensions, providing its own descriptions (where applicable) for the extensions.

A wildcard filter is always added to every dialog.

### Separators ###

 - `;` Begin a new filter.
 - `,` Add a separate type to the filter.

#### Examples ####

`txt` The default filter is for text files.  There is a wildcard option in a dropdown.

`png,jpg;psd` The default filter is for png and jpg files.  A second filter is available for psd files.  There is a wildcard option in a dropdown.

`NULL` Wildcard only.

## Iterating Over PathSets ##

See [test_opendialogmultiple.c](test/test_opendialogmultiple.c).

# Known Limitations #

I accept quality code patches, or will resolve these and other matters through support.

 - No support for Windows XP's legacy dialogs such as `GetOpenFileName`.
 - No support for file filter names -- ex: "Image Files" (*.png, *.jpg).  Nameless filters are supported, though.
 - No support for selecting folders instead of files.
 - On Linux, GTK+ cannot be uninitialized to save memory.  Launching a file dialog costs memory.  I am open to accepting an alternative `nfd_zenity.c` implementation which uses Zenity and pipes.

# Copyright and Credit #

Copyright &copy; 2014 [Frogtoss Games](http://www.frogtoss.com), Inc.
File [LICENSE](LICENSE) covers all files in this repo.

Native File Dialog by Michael Labbe
<mike@frogtoss.com>

Tomasz Konojacki for [microutf8](http://puszcza.gnu.org.ua/software/microutf8/)

## Support ##

Directed support for this work is available from the original author under a paid agreement.

[Contact Frogtoss Games](http://www.frogtoss.com/pages/contact.html).
