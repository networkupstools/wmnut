/*
 * wmnut-common.h  -- Header file for WMNUT with common includes and definitions
 *
 * Copyright (C)
 *   2002 - 2012  Arnaud Quette <arnaud.quette@free.fr>
 *   2022 - 2025  Jim Klimov <jimklimov+nut@gmail.com>
 *          2024  desertwitch <dezertwitsh@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 */

#ifndef WMNUT_COMMON_H_INCLUDED
#define WMNUT_COMMON_H_INCLUDED

#ifdef HAVE_CONFIG_H
/* May be generated without include header guards */
# ifndef CONFIG_H_INCLUDED
#  include "config.h"
#  ifndef CONFIG_H_INCLUDED
#   define CONFIG_H_INCLUDED
#  endif	/* CONFIG_H_INCLUDED */
# endif	/* CONFIG_H_INCLUDED */
#endif	/* HAVE_CONFIG_H */

/* standard system includes */
#ifdef FreeBSD
# include <err.h>
# include <sys/file.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#if HAVE_LIMITS_H
# include <limits.h>
#endif

/* nut and wmnut includes */
#include <upsclient.h>
#if defined HAVE_UPSCLI_STR_CONTAINS_TOKEN && HAVE_UPSCLI_STR_CONTAINS_TOKEN
# define	STR_CONTAINS_TOKEN(haystack, needle)	upscli_str_contains_token(haystack, needle)
#else
# define	STR_CONTAINS_TOKEN(haystack, needle)	strstr(haystack, needle)
#endif

#define SMALLBUF	256
#define LARGEBUF	1024

/* Portable max path length, may be or not be defined in NUT headers too */
#ifndef NUT_PATH_MAX
# define NUT_PATH_MAX	SMALLBUF
# if (defined(PATH_MAX)) && PATH_MAX > NUT_PATH_MAX
#  undef NUT_PATH_MAX
#  define NUT_PATH_MAX	PATH_MAX
# endif
# if (defined(MAX_PATH)) && MAX_PATH > NUT_PATH_MAX
/* PATH_MAX is the POSIX equivalent for Microsoft's MAX_PATH */
#  undef NUT_PATH_MAX
#  define NUT_PATH_MAX	MAX_PATH
# endif
# if (defined(UNIX_PATH_MAX)) && UNIX_PATH_MAX > NUT_PATH_MAX
#  undef NUT_PATH_MAX
#  define NUT_PATH_MAX	UNIX_PATH_MAX
# endif
# if (defined(MAXPATHLEN)) && MAXPATHLEN > NUT_PATH_MAX
#  undef NUT_PATH_MAX
#  define NUT_PATH_MAX	MAXPATHLEN
# endif
#endif	/* !NUT_PATH_MAX */

#endif	/* WMNUT_H_COMMON_INCLUDED */
