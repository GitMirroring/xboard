/*
 * common.c -- Common definitions for X and Windows NT versions of XBoard
 *
 * Copyright 2026 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
/* <> is used to support out-of-source autoconf builds. */
# include <config.h>
#endif

#include <stdlib.h>

#include "common.h"

#if !defined(HAVE_STRDUP)
char * strdup(char const * const c_str) {
    size_t length;
    char * result;
    if (NULL == c_str) {
        result = NULL;
    } else {
        length = strlen(c_str) + 1;
        result = malloc(length);
        if (NULL != result) {
            memcpy(result, c_str, length);
        }
    }
    return result;
}
#endif

char * free_then_strdup(char * c_str, char const * const c_str_to_dup) {
    if (c_str) {
        free(c_str);
    }
    return strdup(c_str_to_dup);
}

int default_line_gap(int const square_size) {
    if (square_size < 37) return 1;
    if (square_size < 59) return 2;
    if (square_size < 116) return 3;
    return 4;
}

int desired_board_dimension_in_pixels(int const board_row_or_column_count, int const square_size, int const line_gap) {
    return line_gap + board_row_or_column_count * (square_size + line_gap);
}
