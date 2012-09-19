
Please note : 

 If your project includes the header file "Rpcndr.h", then
you will then have to define "HAVE_BOOLEAN" before "libmng.h"
is included. Why ? ...

"jmorecfg.h" has
#ifndef HAVE_BOOLEAN
typedef int boolean;
#endif

 and "Rpcndr.h" has
typedef unsigned char boolean

 This >>MAY<< affect other libraries used in the same project
that depend on the jpeg source - especially as "boolean" is
used in structures (possible alignment problems). For example
"Sam leffler's" LibTiff can be built with the Jpeg codec.

 Just a little something to be aware of !. AP - 15/9/2000.

