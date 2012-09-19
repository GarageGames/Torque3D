dnl autoconf macros for detecting libmng
dnl add this to your aclocal or acinclude to make use of it
dnl
dnl (c) 2000 Ralph Giles <giles@ashlu.bc.ca>
dnl

dnl A basic check: looks for libmng and its dependencies
dnl and adds the required bits to CFLAGS and LIBS

# check for libmng
AC_DEFUN(LIBMNG_CHECK, [
  dnl prerequisites first
  AC_CHECK_LIB(jpeg, jpeg_set_defaults)
  AC_CHECK_LIB(z, inflate)
  dnl now the library
  AC_CHECK_LIB(mng, mng_readdisplay, [_libmng_present=1])
  AC_CHECK_HEADER(libmng.h)
  dnl see if we need the optional link dependency
  AC_CHECK_LIB(lcms, cmsCreateRGBProfile, [
	AC_CHECK_HEADER(lcms.h)
	AC_CHECK_LIB(mng, mnglcms_initlibrary, [
		LIBS="$LIBS -llcms"
		AC_DEFINE(HAVE_LIBLCMS)
		_libmng_present=1
	])
  ])
  if test $_libmng_present -eq 1; then
	LIBS="-lmng $LIBS"
	AC_DEFINE(HAVE_LIBMNG)
  fi
  _libmng_present=
])

dnl end LIBMNG macros
