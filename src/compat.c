/* compat.c - xstrdup compatibility for wmnut

   Copyright (C)
    2001         Russell Kroll <rkroll@exploits.org>
    2022 - 2024  Jim Klimov <jimklimov+nut@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char *oom_msg = "Out of memory";

char *xstrdup(const char *string)
{
	char *p = strdup(string);

	if (p == NULL) {
		fprintf(stderr, "%s", oom_msg);
		exit(1);
	}

	return p;
}

