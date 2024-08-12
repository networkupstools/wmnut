/*
 * wmnut.h  -- Header file for WMNUT
 *
 * Copyright (C)
 *   2002 - 2012  Arnaud Quette <arnaud.quette@free.fr>
 *   2022 - 2024  Jim Klimov <jimklimov+nut@gmail.com>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

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

/* X11 includes */
#include <X11/X.h>
#include <X11/xpm.h>

/* nut and wmnut includes */
#include <upsclient.h>
#include "wmgeneral.h"

/* pixmaps */
#include "wmnut_master.xpm"
#include "wmnut_master_LowColor.xpm"
#include "wmnut_mask.xbm"

#define DELAY 10000L		/* Delay between refreshes (in microseconds) */

#define SMALLBUF	256
#define LARGEBUF	1024

/* Communication status definition */

#define COM_LOST       0
#define COM_OK         1

#define COM_NONE      -1
#define COM_UDP        0
#define COM_TCP        1

/* 0 for UDP, 1 for TCP, -1 for not init'ed */

/* General codes definition */

#define VARNOTSUPP    -2
#define NOK           -1
#define OK             1
#define OFF            0
#define ON             1

/* UPS status definition */

#define UPS_DEFAULT	0
#define UPS_LOWBATT	1
#define UPS_ONBATT	2
#define UPS_ONLINE	3
#define UPS_OVERLOAD	4

/* structure to monitor an UPS unit */

typedef struct ups_info {
	int	hostnumber;
	char	*upsname;
	char	*hostname;
	UPSCONN_t	connexion;
	uint16_t	port;
	int	ups_status;	/* -1 if not init'ed */
	int	comm_status;	/* -1 if not init'ed, -2 if not available */
	int	battery_percentage;	/* -1 if not init'ed, -2 if not available */
	int	battery_load;	/* -1 if not init'ed, -2 if not available */
	int	battery_runtime;	/* -1 if not init'ed, -2 if not available */
} ups_info;

/*
 * struct to monitor multiple hosts
 */

typedef struct nut_info {
	int	hosts_number;		/* total number of hosts */
	int	curhosts_number;	/* number of the currently displayed host */
	ups_info	*Ups_list[9];	/* list of monitored UPSs (from 1 to 9) */
} nut_info;
