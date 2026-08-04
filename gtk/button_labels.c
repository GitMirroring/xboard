/*
 * button_labels.c
 *
 * Copyright 2026 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
/* <> is used to support out-of-source autoconf builds. */
# include <config.h>
#endif

#include "gtk/button_labels.h"

#include <libintl.h>
#include <string.h>


char * cancel_button_c_str(void) {
#if API_USED_FOR_DRAWING_GUI == 3
# if ENABLE_NLS
    return strdup(gettext("_Cancel"));
# else
    return strdup("_Cancel");
# endif
#elif API_USED_FOR_DRAWING_GUI == 2
    return strdup("gtk-cancel");
#else
    #error Not implemented.
#endif
}

char * ok_button_c_str(void) {
#if API_USED_FOR_DRAWING_GUI == 3
# if ENABLE_NLS
    return strdup(gettext("_OK"));
# else
    return strdup("_OK");
# endif
#elif API_USED_FOR_DRAWING_GUI == 2
    return strdup("gtk-ok");
#else
    #error Not implemented.
#endif
}

char * open_button_c_str(void) {
#if API_USED_FOR_DRAWING_GUI == 3
# if ENABLE_NLS
    return strdup(gettext("_Open"));
# else
    return strdup("_Open");
# endif
#elif API_USED_FOR_DRAWING_GUI == 2
    return strdup("gtk-open");
#else
    #error Not implemented.
#endif
}

char * save_button_c_str(void) {
#if API_USED_FOR_DRAWING_GUI == 3
# if ENABLE_NLS
    return strdup(gettext("_Save"));
# else
    return strdup("_Save");
# endif
#elif API_USED_FOR_DRAWING_GUI == 2
    return strdup("gtk-save");
#else
    #error Not implemented.
#endif
}
