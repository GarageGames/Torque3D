# autogen.sh
#
# invoke the auto* tools to create the configureation system

# move out configure.in
if ! test -f configure.in; then
  echo "copying configure.in"
  ln -s makefiles/configure.in .
fi

# move out the macros and run aclocal
if test ! -f acinclude.m4 -a -r makefiles/acinclude.m4; then
  echo "copying configure macros"
  ln -s makefiles/acinclude.m4 .
fi

# copy up our Makefile template
if ! test -f Makefile.am; then
  echo "copying automake template"
  ln -s makefiles/Makefile.am .
fi

echo "running aclocal"
aclocal

# libtool is named glibtool on MacOS X
for LIBTOOLIZE in libtoolize glibtoolize nope; do
  ($LIBTOOLIZE --version) < /dev/null > /dev/null 2>&1 && break
done
if test x$LIBTOOLIZE = xnope; then
  echo "error: Could not find libtoolize in the path!"
  echo "  You'll need to install a copy of libtool before continuing"
  echo "  with the generation of the build system."
  echo
  exit 1
fi

echo "running $LIBTOOLIZE"
$LIBTOOLIZE --automake

echo "running automake"
automake --foreign --add-missing

echo "building configure script"
autoconf

# and finally invoke our new configure
./configure $*

# end
