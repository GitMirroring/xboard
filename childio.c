/*
 * childio.c -- set up communication with child processes
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2016, 2026 Free Software Foundation, Inc.
 *
 * The following terms apply to Digital Equipment Corporation's copyright
 * interest in XBoard:
 * ------------------------------------------------------------------------
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * ------------------------------------------------------------------------
 *
 * The following terms apply to the enhanced version of XBoard
 * distributed by the Free Software Foundation:
 * ------------------------------------------------------------------------
 *
 * GNU XBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU XBoard is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.  *
 *
 *------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

/* This file splits into two entirely different pieces of code
   depending on whether USE_PTYS is 1.  The whole reason for all
   the pty nonsense is that select() does not work on pipes in System-V
   derivatives (at least some of them).  This is a problem because
   XtAppAddInput works by adding its argument to a select that is done
   deep inside Xlib.
*/

#include "config.h"

#if USE_PTYS
# define _XOPEN_SOURCE 500
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "common.h"
#include "frontend.h"
#include "backend.h" /* for safeStrCpy */

#if !USE_PTYS
/* This code is for systems where pipes work properly */

void SetUpChildIO(int to_prog[2], int from_prog[2]) {
    signal(SIGPIPE, SIG_IGN);
    pipe(to_prog);
    pipe(from_prog);
}

#else /* USE_PTYS == 1 */
/* This code is for all systems where we must use ptys */

# include <errno.h>
# include <sys/stat.h>
# include <sys/ioctl.h>
# if HAVE_STROPTS_H
#  include <stropts.h>
# endif /* HAVE_STROPTS_H */
# if HAVE_SYS_FCNTL_H
#  include <sys/fcntl.h>
# else /* do not HAVE_SYS_FCNTL_H */
#  if HAVE_FCNTL_H
#   include <fcntl.h>
#  endif /* HAVE_FCNTL_H */
# endif /* do not HAVE_SYS_FCNTL_H */

int PseudoTTY(char pty_name[]);

int SetUpChildIO(int to_prog[2], int from_prog[2]) {
    char pty_name[MSG_SIZ];

    if ((to_prog[1] = PseudoTTY(pty_name)) == -1) {
        DisplayFatalError("Can't open pseudo-tty", errno, 1);
        ExitEvent(1);
    }
    from_prog[0] = to_prog[1];
    to_prog[0] = from_prog[1] = open(pty_name, O_RDWR, 0);

# if HAVE_STROPTS_H /* do we really need this??  pipe-like behavior is fine */
    if (ioctl(to_prog[0], I_PUSH, "ptem") == -1 || ioctl(to_prog[0], I_PUSH, "ldterm") == -1 ||
     ioctl(to_prog[0], I_PUSH, "ttcompat") == -1) {
#  ifdef NOTDEF /* seems some systems don't have or need ptem and ttcompat */
        DisplayFatalError("Can't ioctl pseudo-tty", errno, 1);
        ExitEvent(1);
#  endif /* NOTDEF */
    }
# endif /* HAVE_STROPTS_H */
}

# if HAVE_GRANTPT
/* This code is for SVR4 */

int PseudoTTY(char pty_name[]) {
    char * ptss;
    int fd;

    fd = open("/dev/ptmx", O_RDWR);
    if (fd < 0) {
        return fd;
    }
    if (grantpt(fd) == -1) {
        return -1;
    }
    if (unlockpt(fd) == -1) {
        return -1;
    }
    if (!(ptss = ptsname(fd))) {
        return -1;
    }
    safeStrCpy(pty_name, ptss, sizeof(pty_name) / sizeof(pty_name[0]));
    return fd;
}

# elif HAVE__GETPTY
/* This code is for IRIX */

int PseudoTTY(char pty_name[]) {
    int fd;
    char * ptyn;

    ptyn = _getpty(&fd, O_RDWR, 0600, 0);
    if (ptyn == NULL) {
        return -1;
    }
    safeStrCpy(pty_name, ptyn, sizeof(pty_name) / sizeof(pty_name[0]));
    return fd;
}

# elif HAVE_LIBSEQ
/* This code is for Sequent DYNIX/ptx.  Untested. --tpm */

int PseudoTTY(char pty_name[]) {
    int fd;
    char *slave, *master;

    fd = getpseudotty(&slave, &master);
    if (fd < 0) {
        return fd;
    }
    safeStrCpy(pty_name, slave, sizeof(pty_name));
    return fd;
}

# else
/* This code is for all other systems */
/* The code is adapted from GNU Emacs 19.24 */

#  ifndef FIRST_PTY_LETTER
#   define FIRST_PTY_LETTER 'p'
#  endif
#  ifndef LAST_PTY_LETTER
#   define LAST_PTY_LETTER 'z'
#  endif

#  define DEFAULT_PTY_ITERATION \
      for (ch = FIRST_PTY_LETTER; ch <= LAST_PTY_LETTER; ++ch) \
          for (i = 0; i < 16; ++i)
#  define DEFAULT_PTY_NAME_C_STR sprintf(pty_name, "/dev/pty%c%x", ch, i)
#  define DEFAULT_PTY_TTY_NAME_C_STR sprintf(pty_name, "/dev/tty%c%x", ch, i)

#  define HPUX_PTY_NAME_C_STR sprintf(pty_name, "/dev/ptym/pty%c%x", ch, i)
#  define HPUX_PTY_TTY_NAME_C_STR sprintf(pty_name, "/dev/pty/tty%c%x", ch, i)

#  define AIX3_BOSX_PTY_ITERATION for (ch = 0; !ch; ++ch)
#  define AIX3_BOSX_PTY_NAME_C_STR strcpy(pty_name, "/dev/ptc")
#  define AIX3_BOSX_PTY_TTY_NAME_C_STR strcpy(pty_name, ttyname(fd))

#  define RTU_PTY_NAME_C_STR sprintf(pty_name, "/dev/pty%x", i)
#  define RTU_PTY_TTY_NAME_C_STR sprintf(pty_name, "/dev/ttyp%x", i)

#  define IRIS_IRIX3_PTY_ITERATION for (ch = 0; !ch; ++ch)
#  define IRIS_IRIX3_PTY_NAME_C_STR strcpy(pty_name, "/dev/ptc")
#  define IRIS_IRIX3_PTY_TTY_NAME_C_STR sprintf(pty_name, "/dev/ttyq%d", minor(stb.st_rdev))

#  define SCO_PTY_ITERATION for (i = 0;; ++i)
#  define SCO_PTY_NAME_C_STR sprintf(pty_name, "/dev/ptyp%d", i)
#  define SCO_PTY_TTY_NAME_C_STR sprintf(pty_name, "/dev/ttyp%d", i)

/* The build system will augment the compilation flags such that PTY_ITERATION
   is #defined to the appropriate *_PTY_ITERATION from the options above. */
#  ifndef PTY_ITERATION
#   error "PTY_ITERATION ought to have been defined."
"PTY_ITERATION ought to have been defined."
#  endif

/* The build system will augment the compilation flags such that PTY_NAME_C_STR
   is #defined to the appropriate *_PTY_NAME_C_STR from the options above. */
#  ifndef PTY_NAME_C_STR
#   error "PTY_NAME_C_STR ought to have been defined."
 "PTY_NAME_C_STR ought to have been defined."
#  endif

/* The build system will augment the compilation flags such that
   PTY_TTY_NAME_C_STR is #defined to the appropriate *_PTY_TTY_NAME_C_STR from
   the options above. */
#  ifndef PTY_TTY_NAME_C_STR
#   error "PTY_TTY_NAME_C_STR ought to have been defined."
 "PTY_TTY_NAME_C_STR ought to have been defined."
#  endif

 int PseudoTTY(char pty_name[]) {
    struct stat stb;
    char ch;
    int fd;
    int i;

    /* Some systems name their pseudoterminals so that there are gaps
       in the usual sequence.  For example, on HP9000/S700 systems,
       there are no pseudoterminals with names ending in 'f'.  So, we
       wait for three failures in a row before deciding that we've
       reached the end of the ptys. */
    int consecutive_failures_count = 0;

    PTY_ITERATION {
        PTY_NAME_C_STR;
        if (stat(pty_name, &stb) < 0) {
            ++consecutive_failures_count;
            if (consecutive_failures_count == 3) {
                return -1;
            }
        } else {
            consecutive_failures_count = 0;
        }
        fd = open(pty_name, O_RDWR, 0);

        if (fd >= 0) {
            /* check to make certain that both sides are available
               this avoids a nasty yet stupid bug in rlogins */
            PTY_TTY_NAME_C_STR;
#  ifndef UNIPLUS
            if (access(pty_name, 6) != 0) {
                close(fd);
                continue;
            }
#  endif /* ifndef UNIPLUS */
#  ifdef IBM_RT_PC_AIX
            /* On these systems, the parent gets SIGHUP when a
               pty-attached child dies.  So, we ignore SIGHUP once
               we've started a child on a pty.  Note that this may
               cause XBoard not to die when it should, i.e., when its
               own controlling tty goes away. */
            signal(SIGHUP, SIG_IGN);
#  endif /* IBM_RT_PC_AIX */
            return fd;
        }
    }
    return -1;
}

# endif /* HAVE_GRANTPT #elif HAVE__GETPTY #elif HAVE_LIBSEQ #else */
#endif /* USE_PTYS */
