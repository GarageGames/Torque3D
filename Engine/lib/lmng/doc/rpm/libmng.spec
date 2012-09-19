Summary: A library of functions for manipulating MNG format files.
Name: libmng
Version: 1.0.10
Release: 2.1
Copyright: AS IS
Group: System Environment/Libraries
Source0: libmng-%{PACKAGE_VERSION}.tar.gz
Patch: libmng-%{PACKAGE_VERSION}-rhconf.patch
URL: http://www.libmng.com/
BuildRoot: /var/tmp/libmng-root
BuildPrereq: libjpeg-devel, zlib-devel, lcms-devel

%description
libmng - library for reading, writing, displaying and examing
Multiple-Image Network Graphics. MNG is the animation extension to the
popular PNG image-format.

%package devel
Summary: Development tools for programs to manipulate MNG format files.
Group: Development/Libraries
Requires: libmng = %{PACKAGE_VERSION}
%description devel
The libmng-devel package contains the header files and static
libraries necessary for developing programs using the MNG
(Multiple-Image Network Graphics) library.

If you want to develop programs which will manipulate MNG image format
files, you should install libmng-devel.  You'll also need to install
the libmng package.

%changelog
* Fri Jul 13 2007 Glenn Randers-Pehrson <glennrp at users.sf.net>
- updated to 1.0.10

* Thu Aug  5 2004 Gerard Juyn <gerard at libmng.com>
* Sun Jan 30 2005 Gerard Juyn <gerard at libmng.com>
- updated to 1.0.9

* Thu Aug  5 2004 Gerard Juyn <gerard at libmng.com>
- updated to 1.0.8

* Sun Mar 21 2004 Gerard Juyn <gerard at libmng.com>
- updated to 1.0.7

* Sun Oct 19 2003 Gerard Juyn <gerard at libmng.com>
- updated to 1.0.6

* Tue Sep 24 2002 Gerard Juyn <gerard at libmng.com>
- updated to 1.0.5

* Sun Jun 23 2002 Gerard Juyn <gerard at libmng.com>
- updated to 1.0.4

* Mon Sep 18 2001 Gerard Juyn <gerard at libmng.com>
- updated to 1.0.3

* Sat Jul 7 2001 Gerard Juyn <gerard at libmng.com>
- updated to 1.0.2

* Wed May 2 2001 Gerard Juyn <gerard at libmng.com>
- updated to 1.0.1

* Mon Feb 5 2001 Gerard Juyn <gerard at libmng.com>
- updated to 1.0.0

* Fri Jan 19 2001 Gerard Juyn <gerard at libmng.com>
- updated to 0.9.4

* Sat Oct 28 2000 Gerard Juyn <gerard at libmng.com>
- updated to 0.9.3

* Tue Aug 15 2000 MATSUURA Takanori <t-matsuu at protein.osaka-u.ac.jp>
- based on libmng-0.9.2/doc/rpm/libmng.spec
- use %%configure and %%makeinstall

* Sat Aug  5 2000 Gerard Juyn <gerard at libmng.com>
- updated to 0.9.2

* Wed Jul 26 2000 Gerard Juyn <gerard at libmng.com>
- updated to 0.9.1

* Sat Jul  1 2000 MATSUURA Takanori <t-matsuu at protein.osaka-u.ac.jp>
- updated to 0.9.0

* Sat Jun 24 2000 MATSUURA Takanori <t-matsuu at protein.osaka-u.ac.jp>
- 1st release for RPM

%prep
%setup
%configure

%build
make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc CHANGES LICENSE README doc
/usr/lib/libmng.so.*

%files devel
%defattr(-,root,root)
/usr/include/*
/usr/lib/libmng.a
/usr/lib/libmng.so

