dnl| configure.ac (formerly configure.in)
dnl|
dnl| Copyright 1992-2016, 2026 Free Software Foundation, Inc.
dnl|
dnl| GNU XBoard is free software: you can redistribute it and/or modify
dnl| it under the terms of the GNU General Public License as published by
dnl| the Free Software Foundation, either version 3 of the License, or (at
dnl| your option) any later version.
dnl|
dnl| GNU XBoard is distributed in the hope that it will be useful, but
dnl| WITHOUT ANY WARRANTY; without even the implied warranty of
dnl| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
dnl| General Public License for more details.
dnl|
dnl| You should have received a copy of the GNU General Public License
dnl| along with this program. If not, see http://www.gnu.org/licenses/.
dnl|
dnl| --------------------------------------------------------------------
dnl|
dnl| You can process this file with autoconf to produce a configure script.
dnl| However, normally the supplied configure script will work fine.
dnl|
dnl| If you do need to change the configure script, instead of editing
dnl| it directly, try to edit configure.ac (in a way that will keep
dnl| it portable to sites and systems other than your own), and run autoconf
dnl| to regenerate configure.  Then submit your changes to be folded into
dnl| the standard version of xboard.

AC_PREREQ([2.67])

dnl| define second argument as MAJOR.MINOR.PATCHLEVEL, e.g., 4.4.0
dnl| MAJOR means large changes
dnl| MINOR can includes new features and bug fixes
dnl| PATCHLEVEL is used for bugfixes only
dnl| pre-release version could have a -YYYYMMDD attached, e.g., 4.9.0-20160313
AC_INIT([GNU XBoard], [4.9.2-20260221], [bug-xboard@gnu.org])

AC_CONFIG_SRCDIR([zippy.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE([
    gnu
    check-news
    dist-xz
    filename-length-max=99
    info-in-builddir
    std-options
    subdir-objects
    tar-v7
    --warnings=all
])

AM_MAINTAINER_MODE([enable])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_MISSING_PROG(MAKEINFO, makeinfo)

AX_WITH_PROG([PERL], [perl], [/usr/bin/perl])

AH_TOP([#ifndef GNU_XBOARD_CONFIG_H
#define GNU_XBOARD_CONFIG_H])
AH_BOTTOM([#endif /* GNU_XBOARD_CONFIG_H */])

dnl| AH_TEMPLATEs are declared for the use of AC_DEFINE with autoheader.
dnl| It is not necessary to declare an AH_TEMPLATE for a symbol that
dnl| will be set by AC_CHECK_HEADERS, AC_CHECK_FUNCS, etc.
AH_TEMPLATE(
    [ATTENTION],
    [Attempt to send SIGINT to an engine to get it to stop thinking when desirable.]
)
AH_TEMPLATE(
    [REMOTE_SHELL],
    [The command to invoke to create a remote shell.]
)
AH_TEMPLATE(
    [USE_XAW3D],
    [Use a 3D-styled version of the X Athena Widgets.]
)

dnl| PTY-related autoheader templates.
AH_TEMPLATE(
    [FIRST_PTY_LETTER],
    [Letter to use in finding device name of first PTY, if PTYs are supported.]
)
AH_TEMPLATE(
    [IBMRTAIX],
    [Indicates that some special PTY-related code for AIX systems should be used.  Specifically, indicates whether or not a parent gets SIGHUP when a PTY-attached child dies.]
)
AH_TEMPLATE(
    [LAST_PTY_LETTER],
    [Letter to use in finding device name of last PTY, if PTYs are supported.]
)
AH_TEMPLATE(
    [PTY_ITERATION],
    [How to iterate over PTYs.]
)
AH_TEMPLATE(
    [PTY_NAME_SPRINTF],
    [How to get the device name of the control end of a PTY, if non-standard.]
)
AH_TEMPLATE(
    [PTY_OPEN],
    [How to open a PTY, if non-standard.]
)
AH_TEMPLATE(
    [PTY_TTY_NAME_SPRINTF],
    [How to get device name of the tty end of a PTY, if non-standard.]
)
AH_TEMPLATE(
    [UNIPLUS],
    [Indicates that some special PTY-related code for Uniplus systems should be used.]
)
AH_TEMPLATE(
    [USE_PTYS],
    [Whether or not to use PTYs.]
)

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_AWK

AC_TYPE_SIZE_T
AC_C_INLINE
AC_FUNC_FORK

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_REQUIRE_VERSION(0.20.2)

AC_ARG_VAR(RSH)
AC_CHECK_PROGS(RSH, rsh remsh, rsh)
AC_DEFINE_UNQUOTED(REMOTE_SHELL, "$RSH")

AC_HEADER_DIRENT
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS_ONCE([stropts.h sys/file.h sys/ioctl.h sys/systeminfo.h sys/time.h unistd.h])
AC_CHECK_HEADERS([fcntl.h sys/fcntl.h], break)
AC_CHECK_HEADERS([sys/socket.h lan/socket.h], break)

AC_CHECK_FUNCS_ONCE([_getpty grantpt ptsname setitimer unlockpt usleep])
AC_CHECK_FUNCS([gettimeofday ftime], break)
AC_CHECK_FUNCS([random rand48], break)
AC_CHECK_FUNCS([gethostname sysinfo], break)

AC_CHECK_LIB(seq, getpseudotty)

dnl| Check for pkg-config.
PKG_PROG_PKG_CONFIG

dnl| Check for cairo, pangocairo, and librsvg.
PKG_CHECK_MODULES([CAIRO], [cairo >= 1.2.0 pangocairo librsvg-2.0 >= 2.14.0])
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LIBS)

dnl| Ascertain which front end to use.
dnl| First, check for GTK (currently, only version 2 is supported).
dnl| Then, check for Xaw3d.
dnl| Finally, check for Xaw.
AC_ARG_VAR(FRONTEND_CFLAGS)
AC_ARG_VAR(FRONTEND_LIBS)
FRONTEND_CFLAGS=""
FRONTEND_LIBS=""

AC_ARG_WITH(
    [gtk],
    [AS_HELP_STRING(
        [--with-gtk],
        [use GTK 2 front-end (default, and preferred)]
    )],
    [with_GTK=$withval],
    [with_GTK="yes"]
)

AC_ARG_WITH(
    [Xaw3d],
    [AS_HELP_STRING(
        [--with-Xaw3d],
        [use Xaw3d front-end]
    )],
    [with_Xaw3d="$withval"],
    [with_Xaw3d="no"]
)
dnl| If Xaw3d is selected, then deselect GTK.
AS_IF([test "x$with_Xaw3d" = xyes], [with_GTK="no"])

AC_ARG_WITH(
    [Xaw],
    [AS_HELP_STRING(
        [--with-Xaw],
        [use Xaw front-end]
    )],
    [with_Xaw="$withval"],
    [with_Xaw="no"]
)
dnl| If Xaw is selected, then deselect GTK.
AS_IF([test "x$with_Xaw" = xyes], [with_GTK="no"])

AC_ARG_WITH(
    [iconsdir],
    [AS_HELP_STRING(
        [--with-iconsdir=DIR],
        [path where icons get installed (default: $datadir/icons/hicolor/48x48/apps)]
    )],
    [ICONSDIR="$withval"],
    [ICONSDIR='$(datadir)/icons/hicolor/48x48/apps']
)

AC_ARG_WITH(
    [svgiconsdir],
    [AS_HELP_STRING(
        [--with-svgiconsdir=DIR],
        [path where svg icons get installed (default: $datadir/icons/hicolor/scalable/apps)]
    )],
    [SVGICONSDIR="$withval"],
    [SVGICONSDIR='$(datadir)/icons/hicolor/scalable/apps']
)

AC_ARG_WITH(
    [desktopdir],
    [AS_HELP_STRING(
        [--with-desktopdir=DIR],
        [path where desktop files get installed (default: $datadir/applications)]
    )],
    [DESKTOPDIR="$withval"],
    [DESKTOPDIR='$(datadir)/applications']
)

AC_ARG_WITH(
    [mimedir],
    [AS_HELP_STRING(
        [--with-mimedir=DIR],
        [path where mime files get installed (default: $datadir/mime/packages)]
    )],
    [MIMEDIR="$withval"],
    [MIMEDIR='$(datadir)/mime/packages']
)

AC_ARG_WITH(
    [gamedatadir],
    [AS_HELP_STRING(
        [--with-gamedatadir=DIR],
        [path where game data files get installed (default: $datadir/games/xboard)]
    )],
    [GAMEDATADIR="$withval"],
    [GAMEDATADIR='$(datadir)/games/xboard']
)

AC_ARG_ENABLE(
    [update-mimedb],
    [AS_HELP_STRING(
        [--disable-update-mimedb],
        [disable the update-mime-database after install [default=no]]
    )],
    [enable_mimedb="no"],
    [enable_mimedb="yes"]
)

dnl| TODO: Reconsider how this is being done.  Most of the special processing
dnl| for different kinds of hosts is done after AC_CANONICAL_HOST.  So, why is
dnl| this up here?
AC_ARG_ENABLE(
    [osxapp],
    [AS_HELP_STRING(
        [--enable-osxapp],
        [building for macOS (a.k.a. Mac OS X) [default=no]]
    )],
    [enable_OSXAPP=$enableval],
    [enable_OSXAPP="no"]
)

AS_IF(
    [test "x$enable_OSXAPP" = xyes],
    [
        prefix=`pwd`/XBoard.app/Contents
        AC_DEFINE([OSXAPP], [1], [Force many settings when building for macOS])
        with_GTK="yes"
        enable_mimedb="no"
        datarootdir='$(prefix)/Resources'
        datadir='$(datarootdir)/share'
        sysconfdir='$(datarootdir)/etc'
        infodir='$(datadir)/info'
        mandir='$(datadir)/man'
        bindir='$(prefix)/MacOS'
        localedir='$(datadir)/locale'
        GAMEDATADIR='$(datadir)/xboard'
    ]
)

AM_CONDITIONAL([OSX_APP], [test "x$enable_OSXAPP" = xyes])

AM_CONDITIONAL([NLS_ENABLED], [test "x$USE_NLS" = xyes])

AM_CONDITIONAL([ENABLE_UPDATE_MIMEDB], [test "x$enable_mimedb" = xyes])
AS_IF([test "x$enable_mimedb" = xyes], [
    AC_PATH_PROG([XDG_MIME], [xdg-mime])
    AC_SUBST(XDG_MIME)
    AC_PATH_PROG([XDG_DESKTOP_MENU], [xdg-desktop-menu])
    AC_SUBST(XDG_DESKTOP_MENU)
    AC_PATH_PROG([XDG_ICON_RESOURCE], [xdg-icon-resource])
    AC_SUBST(XDG_ICON_RESOURCE)
])

AC_SUBST(ICONSDIR)
AC_SUBST(SVGICONSDIR)
AC_SUBST(DESKTOPDIR)
AC_SUBST(MIMEDIR)
AC_SUBST(GAMEDATADIR)

dnl| Check for some required libraries when GTK is used.
AS_IF([test "x$with_GTK" = xyes], [
    PKG_CHECK_MODULES([GTK], [gtk+-2.0 >= 2.16.0 gmodule-2.0])
    FRONTEND_CFLAGS=$GTK_CFLAGS
    FRONTEND_LIBS=$GTK_LIBS
    with_Xaw="no"
])

AS_IF([test "x$with_GTK" = xno], [
dnl| general check for Xaw and Xaw3d
    AC_PATH_XTRA
    AS_IF(
        [test "x$no_x" = xyes],
        [AC_MSG_ERROR(
            ["    $PACKAGE requires the X Window System header files and libraries!
They were not found on your system.  See FAQ topic C.2.
configure failed"]
        )]
    )
    save_cflags="$CFLAGS"
    save_cppflags="$CPPFLAGS"
    CFLAGS="$CFLAGS $X_CFLAGS"
    CPPFLAGS="$CPPFLAGS $X_CFLAGS"
    AC_CHECK_HEADER([X11/Intrinsic.h], [xt="yes"], [xt="no"])
    AS_IF(
        [test "x$xt" = xno],
        [AC_MSG_ERROR([Xt headers not found. Please install the X11 packages.])]
    )
    CFLAGS="$save_cflags"
    CPPFLAGS="$save_cppflags"

dnl| check availability of Xaw3d
    FRONTEND_LIBS=
    AS_IF(
        [test "x$with_Xaw3d" != xno],
        [AC_CHECK_LIB(
            [Xaw3d],
            [XawTextReplace],
            [
                FRONTEND_LIBS="-lXaw3d"
                AC_DEFINE([USE_XAW3D], [1], [Define if you want to use Xaw3d])
            ],
            [AC_MSG_ERROR(
                [--with-Xaw3d was given, but test for Xaw3d failed. Make sure that you have Xaw3d installed, else you might want to try the configure option --without-Xaw3d]
            )],
            [-lXaw]
        )]
    )

dnl| no Xaw3d, check for Xaw; make it the default
    AS_IF(
        [test "x$with_Xaw3d" = xno],
        [
            with_Xaw=yes
            save_cflags="$CFLAGS"
            save_cppflags="$CPPFLAGS"
            CFLAGS="$CFLAGS $X_CFLAGS"
            CPPFLAGS="$CPPFLAGS $X_CFLAGS"
            AC_CHECK_HEADER([X11/Xaw/Dialog.h], [xaw_headers="yes"], [xaw_headers="no"])
            AS_IF(
                [test "x$xaw_headers" = xno],
                [AC_MSG_ERROR([Xaw headers not found. Please install the Xaw package and headers.])]
            )
            CFLAGS="$save_cflags"
            CPPFLAGS="$save_cppflags"
            FRONTEND_LIBS="-lXaw"
        ]
    )
])

dnl| make results available in Makefile.am
AM_CONDITIONAL([withGTK], [test "x$with_GTK" = xyes])
AM_CONDITIONAL([withXaw], [test "x$with_Xaw3d" = xyes || test "x$with_Xaw" = xyes])

dnl| end Front-end check

AC_CANONICAL_HOST

dnl| The following info was mostly gathered from GNU Emacs 19.24.  Basically,
dnl| we are trying to find out whether this is a UNIX System V derivative in
dnl| which pipes don't work with select(), and if so, whether there is anything
dnl| strange about the way to open a pseudo-tty (pty).  Some of the work was
dnl| done above by looking for _getpty, grantpt, and getpseudotty.  A few other
dnl| strange properties of particular systems are also handled here.

dnl| 1997-04-06: Tim wasn't sure that there really were any systems where pipes
dnl| didn't work with select().  Meanwhile, the use of ptys caused problems on
dnl| many systems.  So, he changed the default to disable ptys in all cases.  He
dnl| wrote that he would change it back if it received bug reports that were
dnl| fixed by doing "configure --enable-ptys".  Apparently, this didn't happen.

dnl| TODO: Consider removing all of the pseudo-tty (pty) code from GNU XBoard.

USE_PTYS=0
case "$host" in
  *-*-hpux* )
    AC_DEFINE(PTY_TTY_NAME_SPRINTF,
        [sprintf (pty_name, "/dev/pty/tty%c%x", c, i);])
    AC_DEFINE(PTY_NAME_SPRINTF,
        [sprintf (pty_name, "/dev/ptym/pty%c%x", c, i);])
dnl| USE_PTYS=1
    if test "$GCC" = yes; then
        CONF_CFLAGS="-fwritable-strings"
    else
dnl| Note: You might be able to build older versions of GNU XBoard even if your
dnl| compiler does not support ANSI C89/ISO C90 (-Aa), because GNU XBoard itself
dnl| used to not require ANSI/ISO C.  Tim didn't know whether the X header files
dnl| on HP-UX required it.
        CONF_CFLAGS="-Aa -D_HPUX_SOURCE"
    fi

dnl| HP didn't supply a full set of X header files and libraries. People often
dnl| had some things installed in one place and some in another.  AC_PATH_XTRA
dnl| will find only one place, so he tried to add all the likely ones that might
dnl| have been missing here.  It might be better to change AC_PATH_XTRA to try
dnl| to extract this information from imake (and/or xmkmf?), because folks who
dnl| install the missing bits often configured their imake to find them, but Tim
dnl| didn't want to delve into autoconf and hack on its internals.
    if test -d /opt/hppd/include/X11; then
      X_CFLAGS="$X_CFLAGS -I/opt/hppd/include"
      X_LIBS="$X_LIBS -L/opt/hppd/lib"
    elif test -d /usr/contrib/X11R5/include; then
      X_CFLAGS="$X_CFLAGS -I/usr/contrib/X11R5/include"
      X_LIBS="$X_LIBS -L/usr/contrib/X11R5/lib"
    elif test -d /usr/contrib/mitX11R5/include; then
      X_CFLAGS="$X_CFLAGS -I/usr/contrib/mitX11R5/include"
      X_LIBS="$X_LIBS -L/usr/contrib/mitX11R5/lib"
    elif test -d /MIT/X11R5/include; then
      X_CFLAGS="$X_CFLAGS -I/MIT/X11R5/include"
      X_LIBS="$X_LIBS -L/MIT/X11R5/lib"
    elif test -d /usr/local/include/X11R5; then
      X_CFLAGS="$X_CFLAGS -I/usr/local/include/X11R5"
      X_LIBS="$X_LIBS -L/usr/local/lib/X11R5"
    fi
    if test -d /usr/include/X11R5; then
      X_CFLAGS="$X_CFLAGS -I/usr/include/X11R5"
      X_LIBS="$X_LIBS -L/usr/lib/X11R5 -L/usr/lib/X11R4"
    elif test -d /usr/include/X11R4; then
      X_CFLAGS="$X_CFLAGS -I/usr/include/X11R4"
      X_LIBS="$X_LIBS -L/usr/lib/X11R4"
    fi
  ;;

  romp-ibm-aix* )
    AC_DEFINE(IBMRTAIX, 1)
dnl| USE_PTYS=1
  ;;

  i386-ibm-aix )
dnl| USE_PTYS=1
    if test "$GCC" = yes; then
      CONF_CFLAGS="-fwritable-strings"
    fi
  ;;

  *-*-aix3* | *-*-bosx* )
    AC_DEFINE(PTY_ITERATION, [for (c = 0; !c; c++)])
    AC_DEFINE(PTY_NAME_SPRINTF, [strcpy (pty_name, "/dev/ptc");])
    AC_DEFINE(PTY_TTY_NAME_SPRINTF, [strcpy (pty_name, ttyname (fd));])
dnl| USE_PTYS=1
  ;;

  *-*-cxux* )
    AC_DEFINE(FIRST_PTY_LETTER, 'A')
    AC_DEFINE(LAST_PTY_LETTER, 'P')
dnl| USE_PTYS=1
  ;;

  *-*-uniplus* )
    AC_DEFINE(UNIPLUS, 1)
dnl| USE_PTYS=1
  ;;

  *-*-rtu* )
    AC_DEFINE(FIRST_PTY_LETTER, 'z')
    AC_DEFINE(PTY_TTY_NAME_SPRINTF,
        [sprintf (pty_name, "/dev/ttyp%x", i);])
    AC_DEFINE(PTY_NAME_SPRINTF,
        [sprintf (pty_name, "/dev/pty%x", i);])
dnl| USE_PTYS=1
  ;;

  *-*-iris* | *-*-irix3* )
    AC_DEFINE(PTY_ITERATION, [for (c = 0; !c; c++)])
    AC_DEFINE(PTY_NAME_SPRINTF, [strcpy (pty_name, "/dev/ptc");])
    AC_DEFINE(PTY_TTY_NAME_SPRINTF,
        [sprintf (pty_name, "/dev/ttyq%d", minor(stb.st_rdev));])
dnl| USE_PTYS=1
  ;;

  *-*-irix* )
dnl| USE_PTYS=1
  ;;

  *-*-sunos4* | *-*-solaris1* )

dnl| Work around a bug in the SunOS 4.x linker.  Not needed if you have patches
dnl| 100512-02 and 100573-03 from Sun.  The X FAQ says that the following is
dnl| "overkill," but doesn't explain what should be done instead.

    if test "$GCC" = yes; then
      PRE_XMULIB="-static"
      POST_XMULIB="-dynamic"
    else
      PRE_XMULIB="-Bstatic"
      POST_XMULIB="-Bdynamic"
    fi
  ;;

  *-*-sunos5* | *-*-solaris2* )
dnl| USE_PTYS=1

dnl| I'm not sure -lelf is needed, but it was in the old Imakefile.
dnl| The other libraries should all be found by AC_PATH_XTRA or other
dnl| code above.

    X_LIBS="$X_LIBS -lelf"
  ;;

  *-*-sco* )
    AC_DEFINE(PTY_ITERATION, [for (i = 0; ; i++)])
    AC_DEFINE(PTY_NAME_SPRINTF, [sprintf (pty_name, "/dev/ptyp%d", i);])
    AC_DEFINE(PTY_TTY_NAME_SPRINTF, [sprintf (pty_name, "/dev/ttyp%d", i);])
dnl| USE_PTYS=1
  ;;

  *-*-dynix* | *-*-ptx* )
dnl| USE_PTYS=1
  ;;

  *-*-esix* )
dnl| USE_PTYS=1
  ;;

  *-*-usg5-4* | *-*-sysvr4* )
dnl| USE_PTYS=1
  ;;

  *-*-usg* | *-*-sysv* | *-*-aix* )
dnl| USE_PTYS=1
  ;;

  vax-*-ultrix )
    if test "$GCC" = yes; then
      CONF_CFLAGS="-fwritable-strings"
    fi
  ;;

dnl| Add some libraries for macOS.
  *-apple-* )
    AC_MSG_WARN([Apple macOS support is experimental. Please report any problems to bug-xboard@gnu.org.])
    AC_MSG_WARN([Use --enable-osxapp to enable the build target and automatically adjust target directories.])
    AC_MSG_WARN([Building for macOS currently relies on some programs such as dylib that we do not test for.])
    AC_MSG_WARN([Patches from macOS users would be most welcome! ;)])
    AC_PATH_PROG(SW_VERS, sw_vers)
    if test "x$SW_VERS" != x; then
        AC_MSG_CHECKING(Mac OS X version)
        MACOSX_VERSION=`$SW_VERS -productVersion`
        AC_MSG_RESULT([$MACOSX_VERSION])
    fi
    FRONTEND_LIBS="$FRONTEND_LIBS -headerpad_max_install_names "
    if test x"$enable_OSXAPP" = x"yes" ; then
        FRONTEND_LIBS="$FRONTEND_LIBS -lgtkmacintegration-gtk2"
    fi
    FRONTEND_CFLAGS="$FRONTEND_CFLAGS -I/opt/local/include -L/opt/local/lib"
  ;;
esac


AC_SUBST(FRONTEND_CFLAGS)
AC_SUBST(FRONTEND_LIBS)
AC_SUBST(PRE_XMULIB)
AC_SUBST(POST_XMULIB)
AC_SUBST(CONF_CFLAGS)
AC_SUBST(CONF_LDFLAGS)

AC_MSG_CHECKING(whether ptys or pipes should be used)
AC_ARG_ENABLE(
    [ptys],
    [AS_HELP_STRING(
        [--enable-ptys],
        [Use pseudo-ttys to communicate with child processes.]
    )],
    [if test "$enableval" = yes; then
        USE_PTYS=1
        enable_ptys="ptys"
        AC_MSG_RESULT([ptys (user override)])
    fi
    if test "$enableval" = no; then
        USE_PTYS=0
        enable_ptys="pipes"
        AC_MSG_RESULT([pipes (user override)])
    fi],
    [if test "$USE_PTYS" = 1; then
        AC_MSG_RESULT(ptys)
        enable_ptys="ptys"
    else
        AC_MSG_RESULT(pipes)
        enable_ptys="pipes"
    fi]
)
AC_DEFINE_UNQUOTED(USE_PTYS, $USE_PTYS)


dnl | define not to build zippy as a default, so that autoheader is happy
AC_ARG_ENABLE(
    [zippy],
    [AS_HELP_STRING(
        [--enable-zippy],
        [support interfacing a chess program to ICS (default)]
    )],
    [],
    [enable_zippy="no"]
)

if test x"$enable_zippy" != x"no"; then
    AC_DEFINE(ZIPPY, 1, [1 if Zippy should be enabled, otherwise 0.])
else
    AC_DEFINE(ZIPPY, 0, [1 if Zippy should be enabled, otherwise 0.])
fi
AM_CONDITIONAL([ZIPPY], [test "x$enable_zippy" != xno])

AC_ARG_ENABLE(
    [sigint],
    [AS_HELP_STRING(
        [--enable-sigint],
        [sending SIGINT (^C) wakes up GNU Chess (default)]
    )],
    [
        if test "$enableval" = yes; then
            AC_DEFINE(ATTENTION, 1)
        fi
        enable_sigint=$enableval
    ],
    [
        enable_sigint="yes"
        AC_DEFINE(ATTENTION, 1)
    ]
)

if test x"$with_Xaw3d" = x"yes" -o  x"$with_Xaw" = x"yes" ; then
dnl | save all information for X in X_LIBS, also add the libraries themself
dnl | since AC_PATH_XTRA only adds the paths
X_LIBS="$X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS -lXmu -lX11 -lXt "
AC_SUBST(X_CFLAGS)
AC_SUBST(X_LIBS)
fi

dnl| Create a string that holds the configure options, so that this information
dnl| may be displayed from within GNU XBoard while it is running.  (It can be
dnl| useful for bug reporting purposes.)
CONFIGURE_OPTIONS="\
prefix=${prefix}; \
datarootdir=${datarootdir}; \
datadir=${datadir}; \
gamedatadir=${GAMEDATADIR}; \
desktopdir=${DESKTOPDIR}; \
mimedir=${MIMEDIR}; \
iconsdir=${ICONSDIR}; \
svgiconsdir=${SVGICONSDIR}; \
infodir=${infodir}; \
sysconfigdir=${sysconfigdir}; \
update_mimedb=${enable_mimedb}; \
NLS=${USE_NLS}; \
GTK=${with_GTK}; \
Xaw3d=${with_Xaw3d}; \
Xaw=${with_Xaw}; \
ptys=${enable_ptys}; \
zippy=${enable_zippy}; \
sigint=${enable_sigint}"

AC_SUBST(CONFIGURE_OPTIONS)

AC_CONFIG_FILES([Makefile cmail po/Makefile.in osxapp/Info.plist])
AC_CONFIG_COMMANDS(
    [test-stamp-h],
    [test -z "$CONFIG_HEADERS" || date > stamp-h]
)
AC_CONFIG_COMMANDS([chmod-cmail], [chmod 755 cmail])
AC_OUTPUT

dnl|
dnl| Output a summary of the configuration.
dnl|
AS_ECHO(["
Configuration summary:

    prefix:          ${prefix}
    datarootdir:     ${datarootdir}
    datadir:         ${datadir}
    gamedatadir:     ${GAMEDATADIR}
    desktopdir:      ${DESKTOPDIR}
    mimedir:         ${MIMEDIR}
    iconsdir:        ${ICONSDIR}
    svgiconsdir:     ${SVGICONSDIR}
    infodir:         ${infodir}   (info files will go here)
    sysconfdir:      ${sysconfdir}   (xboard.conf will go here)

    update mimedb:   ${enable_mimedb}

    NLS support:     ${USE_NLS}

    GTK:             ${with_GTK}
    Xaw3d:           ${with_Xaw3d}
    Xaw:             ${with_Xaw}

    ptys:            ${enable_ptys}
    zippy:           ${enable_zippy}
    sigint:          ${enable_sigint}
"])
