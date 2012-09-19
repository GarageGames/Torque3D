This is an i586 optimized openal library that is suitable for shipping 
games.  It is glibc 2.2.5+ compatible.  Note that currently this library is 
not used when building the engine source code (it is only used by the
install_build scripts).

Built on RH 7.3 with gcc296.

Configure command:

./configure --target=i586-pc-linux-gnu --enable-optimization --enable-sdl=yes
