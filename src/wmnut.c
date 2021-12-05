/*
 * wmnut.c  -- core implementation of WMNUT
 *
 * Copyright (C)
 *   2002 - 2012  Arnaud Quette <arnaud.quette@free.fr>
 *
 * based on wmapm originally written by
 * Chris D. Faulhaber <jedgar@speck.ml.org>. Version 3.0
 * and extensively modified version of version 2.0 
 * by Michael G. Henderson <mghenderson@lanl.gov>.
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

#include "wmnut.h"

/* defines */
#define LARGEBUF      1024

void 	ParseCMDLine(int argc, char *argv[]);
void 	InitHosts();
int  	AddHost(char *hostname);
void 	GetFirstHost();
int 	GetNextHost();
int 	GetPrevHost();
void 	InitCom();

/* base parameters */
/* Controls whether alert is sent to all users via wall: Off by default */
int Alert = 0;
char *upshost = NULL;
/* 1 for verbose mode : displays NUT available features and base values */
int Verbose = 0;
int	CriticalLevel = 10;
int	LowLevel = 40;
float BlinkRate = 3.0;		/* blinks per second */
float UpdateRate = 0.8; 	/* Number of updates per second */
/* Controls beeping when you get to CriticalLevel: Off by default */
int	Beep = 0;
int	Volume = 50;	 		/* ring bell at 50% volume */
/* Use a lower number of colors for the poor saps on 8-bit displays */
int	UseLowColorPixmap = 0;
float LAlertRate = 300.0;	/* send alert every 5 minutes when Low */
float CAlertRate = 120.0;	/* send alert every 2 minutes when Critical */
int WithDrawn = 1;			/* start in withdrawn shape (for WindowMaker) */

#define RETRY_COUNT	10
int TryCount = 0;

/* UPS currently monitored */
ups_info *CurHost; 

/* List of all UPSs monitored */
nut_info Hosts;

rckeys	wmnut_keys[13];

/* Debug macros */
#define DEBUGOUT(...) 	{ if (Verbose) fprintf(stdout, __VA_ARGS__); }
#define DEBUGERR(...) 	{ if (Verbose) fprintf(stderr, __VA_ARGS__); }

/* 
 * Get a variable from the UPS
 ********************************************************* */
int get_ups_var (char *variable, char *value)
{
	int retcode;
	unsigned int	numq, numa;
	const	char	*query[4];
	char	**answer;

	DEBUGOUT("Trying to get variable: %s (on %s)\n", variable, CurHost->upsname);

	query[0] = "VAR";
	query[1] = CurHost->upsname;
	query[2] = variable;

	numq = 3;

	if ((retcode = upscli_get(&CurHost->connexion, numq, query, &numa, &answer)) < 0) {

		/*  if ((retcode = upscli_getvar(&CurHost->connexion, CurHost->upsname,
			variable, value, SMALLBUF)) < 0) { */

		DEBUGERR("Error: %s (%s)\n", upscli_strerror(&CurHost->connexion),
			variable);

		/* should cover driver and upsd disconnection */
		if (CurHost->connexion.upserror == UPSCLI_ERR_DATASTALE)
			CurHost->comm_status = COM_LOST;

		if ((CurHost->connexion.upserror == UPSCLI_ERR_READ) ||
			(CurHost->connexion.upserror == UPSCLI_ERR_INVALIDARG)) {

			CurHost->comm_status = COM_LOST;
			InitCom();
		}

		if (CurHost->connexion.upserror == UPSCLI_ERR_VARNOTSUPP)
			retcode = VARNOTSUPP;

		/* 	  return retcode; */
	}
	else {
		retcode = OK;
		strcpy(value, answer[3]);
		DEBUGOUT("Var: %s = %s\n", variable, value);
		CurHost->comm_status = COM_OK;
	}

	return retcode;
}

void get_ups_info(void) 
{
	char value[SMALLBUF];
	int	retVal;

	/* Get UPS status */
	if (get_ups_var ("ups.status", value) > NOK) {

		if (!strncmp(value, "OL", 2))
			CurHost->ups_status = UPS_ONLINE;
		else if (!strncmp(value, "OB", 2))
			CurHost->ups_status = UPS_ONBATT;
		else if (!strstr(value, "LB"))
			CurHost->ups_status = UPS_LOWBATT;
		else if (!strncmp(value, "OVER", 4))
			CurHost->ups_status = UPS_OVERLOAD;
		else
		CurHost->ups_status = UPS_DEFAULT;
	}
	else
		CurHost->ups_status = UPS_DEFAULT;

	/* Get battery charge level */
	if ((CurHost->battery_runtime != VARNOTSUPP)
		|| (TryCount >= RETRY_COUNT)) {

		retVal = get_ups_var ("battery.charge", value);

		if (retVal == OK)
			CurHost->battery_percentage = atoi(value);
		else
			CurHost->battery_percentage = retVal;
	}

	/* Get runtime to empty */
	if ((CurHost->battery_runtime != VARNOTSUPP)
		|| (TryCount >= RETRY_COUNT)) {

		retVal = get_ups_var ("battery.runtime", value);

		if (retVal == OK)
			CurHost->battery_runtime = atoi(value);
		else
			CurHost->battery_runtime = retVal;
	}

	/* Get Battery load level */
	if ((CurHost->battery_load != VARNOTSUPP)
		|| (TryCount >= RETRY_COUNT)) {

		retVal = get_ups_var ("ups.load", value);

		if (retVal == OK)
			CurHost->battery_load = atoi(value);
		else 
			CurHost->battery_load = retVal;
	}

	if (TryCount >= RETRY_COUNT)
		TryCount = 0;
	else
		TryCount++;

	return;
}

int main(int argc, char *argv[]) {

	int		time_left, hour_left, min_left;
	int		i, m, mMax, n, nMax, k, retVal, Toggle = OFF;
	long int	r, rMax, s, sMax;
	char		*v, *ptr;
 
	/* ignore upsd stop and don't crash (thanks to Russell Kroll) */ 
	signal(SIGPIPE, SIG_IGN);

	/* Set default values */ 
	BlinkRate = 3.0;
	UpdateRate = 1.0 / 1.25;

	/* Multiple hosts setup */
	Hosts.curhosts_number = Hosts.hosts_number = 0;

	/* Create default parameters table */
	AddRcKey(&wmnut_keys[0], "UPS", TYPE_STRING, upshost);
	AddRcKey(&wmnut_keys[1], "LAlertRate", TYPE_FLOAT, &LAlertRate);
	AddRcKey(&wmnut_keys[2], "CAlertRate", TYPE_FLOAT, &CAlertRate);
	AddRcKey(&wmnut_keys[3], "Alert", TYPE_BOOL, &Alert);
	AddRcKey(&wmnut_keys[4], "BlinkRate", TYPE_FLOAT, &BlinkRate); 
	AddRcKey(&wmnut_keys[5], "Beep", TYPE_BOOL, &Beep);
	AddRcKey(&wmnut_keys[6], "Volume", TYPE_INT, &Volume);
	AddRcKey(&wmnut_keys[7], "LowLevel", TYPE_INT, &LowLevel);
	AddRcKey(&wmnut_keys[8], "CriticalLevel", TYPE_INT, &CriticalLevel);
	AddRcKey(&wmnut_keys[9], "UseLowColorPixmap", TYPE_BOOL, &UseLowColorPixmap);
	AddRcKey(&wmnut_keys[10], "Verbose", TYPE_BOOL, &Verbose);
	AddRcKey(&wmnut_keys[11], "WithDrawn", TYPE_BOOL, &WithDrawn);
	AddRcKey(&wmnut_keys[12], NULL, TYPE_NULL, NULL);

	/* Initialise host structure */
	InitHosts();

	/* Parse rcfile command arguments.
	 * First, try with /etc/wmnutrc else 
	 * (if not exists), try ~/.wmnutrc 
	 * Note that the 2nd override the 1st */
	LoadRCFile(wmnut_keys);

	/* Parse any command line arguments.
	 * Note that it overrides RCFiles params */
	ParseCMDLine(argc, argv);

	for (i = 0; i < 12; i++ ) {
		switch(wmnut_keys[i].type) {
			case TYPE_STRING :
				DEBUGOUT("%s = %s\n", wmnut_keys[i].label, wmnut_keys[i].var.str);
				break;
			case TYPE_BOOL: 
			case TYPE_INT:
				DEBUGOUT("%s = %i\n", wmnut_keys[i].label, (int) *wmnut_keys[i].var.integer);
				break;
			case TYPE_FLOAT:
				DEBUGOUT("%s = %f\n", wmnut_keys[i].label, (float) *wmnut_keys[i].var.floater);
				break;
			case TYPE_NULL:
				break;
		}
	}

	/* original basic setup */
	BlinkRate = (BlinkRate >= 0.0) ? BlinkRate : -1.0*BlinkRate;
	UpdateRate = (UpdateRate >= 0.0) ? UpdateRate : -1.0*UpdateRate;

	nMax = (int)( 1.0e6/(2.0*UpdateRate*DELAY)  );
	mMax = (BlinkRate > 0.0) ? (int)( 1.0e6/(2.0*BlinkRate*DELAY)  ) : nMax;
	rMax = (int)( LAlertRate*1.0e6/(2.0*DELAY)  );
	sMax = (int)( CAlertRate*1.0e6/(2.0*DELAY)  );

	/* if no UPS after rcfiles and cmd line, try with localhost */
	if(Hosts.hosts_number == 0)
		AddHost("localhost");

	if(Hosts.hosts_number == 0) {
		fputs("No UPS available.\n", stderr);
		exit(EXIT_FAILURE);
	}

	/*  Check NUT daemon availability on all host(s) */
	InitCom();

	/* Monitor 1rst host by default */
	GetFirstHost();

	/* Open display */
	if (UseLowColorPixmap)
		openXwindow(argc, argv, wmnut_master_LowColor, wmnut_mask_bits, 
					wmnut_mask_width, wmnut_mask_height, WithDrawn);
	else
		openXwindow(argc, argv, wmnut_master, wmnut_mask_bits, 
					wmnut_mask_width, wmnut_mask_height, WithDrawn);

	/* Loop until we die... */
	n = m = 32000;
	r = rMax+1;
	s = sMax+1;

	while(1) {
		/* Only process nut info only every nMax cycles of this
		*  loop. We run it faster to catch the xevents like button 
		*  presses and expose events, etc...
		*  
		*  DELAY is set at 0.00625 seconds, so process nut info
		*  every 1.25 seconds...
		*/
		if (n>nMax){
      
			n = 0;

			/* invert toggle */
			Toggle = (Toggle == OFF) ? ON : OFF;

			get_ups_info();

			/* Check communication status */
			if ((int)(CurHost->comm_status) == COM_LOST) {

				DEBUGERR("Communication lost with UPS %s\n", CurHost->hostname);

				/*
				*  Communication Status: COM_LOST.
				*  Blink red "C" on/off ...
				*/
				if (Toggle||(BlinkRate == 0.0)) {
				/*	if (Beep)
						XBell(display, Volume);
				*/
					copyXPMArea(110,  6, 5, 7,  6,  7);
				} else
					copyXPMArea(98,  6, 5, 7,  6,  7);

				/* ... and "stale" status */
				CurHost->ups_status = UPS_DEFAULT;
			}
			else {
				/* Communication Status: COM_OK */
				copyXPMArea(104,  6, 5, 7,  6,  7);
			}
      
			/* Check UPS status */
			switch (CurHost->ups_status) {

			/* case UPS_LOWBATT: */
				case UPS_ONBATT:
				{
					if(CurHost->battery_percentage <= CriticalLevel)
					{
						/* Battery Status: Critical.
						*  Blink red battery [TODO : and digital %age] on/off...
						*/
						if (Toggle||(BlinkRate == 0.0)) {
							/* if (Beep)
								XBell(display, Volume);
							*/
							/* Toggle = OFF; */
							copyXPMArea(99, 20, 12, 7, 30, 50);
						}
						else{
							/* Toggle = ON; */
							copyXPMArea(83, 20, 12, 7, 30, 50);
						}
					}
					else if(CurHost->battery_percentage <= LowLevel) {
						/*
						*  Battery Status: Low.
						*  Blink the yellow battery [TODO : and digital %age] on/off...
						*/
						if (Toggle||(BlinkRate == 0.0)) {

							/* if (Beep) 
								XBell(display, Volume);
							*/
							/* Toggle = OFF; */
							copyXPMArea(99, 20, 12, 7, 30, 50);
						}
						else{
							/* Toggle = ON; */
							copyXPMArea(69, 20, 12, 7, 30, 50);
						}
					}
					else{
						/*
						*  Battery Status: High but charging.
						*  Blink the green battery [TODO : and digital %age] on/off...
						*/
	      				if (Toggle||(BlinkRate == 0.0)) {
							
							/* if (Beep)
								XBell(display, Volume);
							*/
							copyXPMArea(99, 20, 12, 7, 30, 50); 
						}
						else
							copyXPMArea(83, 6, 12, 7, 30, 50);
					}
				}
				break;
	
				case UPS_OVERLOAD:  
				{
					/*
					*  UPS is overloaded.
					*  [TODO : Blink the battery load %age on/off...]
					*/
					/* Hide digits and '%' */
					copyXPMArea(37, 34, 19, 7,37, 34);
				}
				break;
	
				case UPS_ONLINE:  
				{
					/*
					*   UPS on-line. I.e. we are "plugged-in".
					*/
					copyXPMArea(68, 6, 12, 7, 30, 50);
				}
				break;
	
				case UPS_DEFAULT:  
				{
					/*
					*   used to "erase" UPS status when COM_LOST.
					*/
					copyXPMArea(99, 20, 12, 7, 30, 50);
				}
				break;
	
				default: 
					break;
			}

			/* 
			*    Paste up the default !?! communication !?! status and runtime
			*/
			copyXPMArea(83, 93, 41, 9, 15, 7);

			/*
			*   Repaint buttons.
			*/
			copyXPMArea(42, 106, 13, 11, 5, 48);
			copyXPMArea(57, 106, 13, 11, 46, 48);

			/* 
			*   Repaint host number.
			*/
			copyXPMArea((CurHost->hostnumber) * 7 + 5, 93, 7, 9, 22, 49); 

			/* 
			*  Paste up the "Time Left". This time means (format HH:MM) :
			*  
			*         Time left before battery drains to 0%
			*         If not supported (RUNTIME feature) --:--
			*/
			if(CurHost->battery_runtime >= 0) {
				/* convert in minutes */
				time_left = CurHost->battery_runtime / 60;
				
				hour_left = time_left / 60;
				min_left  = time_left % 60;
				
				/* Show 10's (hour) */
				copyXPMArea( (hour_left / 10) * 7 + 5, 93, 7, 9, 21, 7);
				/* Show 1's (hour) */
				copyXPMArea((hour_left % 10) * 7 + 5, 93, 7, 9, 29, 7);
				/* colon */
				copyXPMArea(76, 93, 2, 9, 38, 7);
				/* Show 10's (min) */
				copyXPMArea((min_left / 10) * 7 + 5, 93, 7, 9, 42, 7);
				/* Show 1's (min) */
				copyXPMArea((min_left % 10) * 7 + 5, 93, 7, 9, 50, 7);
			}
			else /* Show --:-- */
				copyXPMArea(83, 106, 41, 9, 15, 7);

			/* 
			*   Do Battery Load.
			*/
			if (CurHost->battery_load >= 0) {      

				if ((CurHost->ups_status != UPS_OVERLOAD)){ /* needed ?? */
					if (CurHost->battery_load >= 10)
						copyXPMArea((CurHost->battery_load / 10) * 6 + 4,
										81, 6, 7, 37, 34); /* Show 10's */
					else
						copyXPMArea(76, 81, 6, 7, 37, 34); /* Erase 10's */
					copyXPMArea((CurHost->battery_load % 10) * 6 + 4,
									81, 6, 7, 43, 34);    	/* Show 1's */
					copyXPMArea(64, 81, 7, 7, 50, 34);		/* Show '%' */
				}
			}
			else { /* erase zone */
				copyXPMArea(76, 81, 19, 7, 37, 34);
			}

			/*
			 *   Do Battery Percentage.
			 */
			
			/* erase zone */
			/*        copyXPMArea(76, 81, 19, 7, 7, 34);   */
			copyXPMArea(76, 81, 20, 7, 7, 34);	/* Show Default % */
			copyXPMArea(66, 31, 49, 9, 7, 21);	/* Show Default Meter */
      
			if (CurHost->battery_percentage > 0) {
				/* displays battery percent bis */
				if (CurHost->battery_percentage == 100){
					copyXPMArea(15, 81, 1, 7,  7, 34);		/* If 100%, show 100% */
					copyXPMArea( 5, 81, 6, 7,  9, 34);
					copyXPMArea( 5, 81, 6, 7, 15, 34);
					copyXPMArea(64, 81, 6, 7, 21, 34);		/* Show '%' */
					copyXPMArea(66, 42, 49, 9, 7, 21);		/* Show Meter */
				}
				else {	  
					if (CurHost->battery_percentage >= 10)
						copyXPMArea((CurHost->battery_percentage / 10) * 6 + 4,
										81, 6, 7,  9, 34);	/* Show 10's */
					copyXPMArea((CurHost->battery_percentage % 10) * 6 + 4,
									81, 6, 7, 15, 34);	/* Show 1's */
					copyXPMArea(64, 81, 7, 7, 21, 34);	/* Show '%' */
				}
			}

			/*
			 *  Show Meter
			 */
			k = CurHost->battery_percentage * 49 / 100;
			copyXPMArea(66, 42, k, 9, 7, 21);
			if (k%2) 
				copyXPMArea(66+k-1, 52, 1, 9, 7+k-1, 21);
			else
				copyXPMArea(66+k, 52, 1, 9, 7+k, 21);
		}
		else {      
			/* Update the counter. When it hits nMax, we will
			*  process nut information again */
			++n;
		}

		/* Process any pending X events */
		CheckX11Events();

		/* Redraw and wait for next update */
		RedrawWindow();
		usleep(DELAY);
	}
}

void InitCom()
{
	int i, ret;
	char vars[LARGEBUF];
	unsigned int	numq, numa;
	const	char	*query[4];
	char	**answer;

	/*
	 *  Check NUT daemon availability on host(s) 
	 */
	GetFirstHost();

	for (i = 0; i <= (Hosts.hosts_number - 1) ; i++)
	{
		/* Close existing com
		if( &CurHost->connexion)
			upscli_disconnect ( &Hosts.Ups_list[i -1]->connexion );
		*/

		if (upscli_connect(&CurHost->connexion, CurHost->hostname,
						 CurHost->port, UPSCLI_CONN_TRYSSL) < 0) {
			fprintf(stderr, "Error: %s\n", 
					upscli_strerror(&CurHost->connexion));
		}
		else {
			DEBUGERR("Communication established with UPS %s\n", CurHost->hostname);
			CurHost->comm_status = COM_OK;
		}

		query[0] = "VAR";
		query[1] = CurHost->upsname;
		numq = 2;

		if (ret = upscli_list_start(&CurHost->connexion, numq, query) < 0)
		/* if (upscli_getlist(&CurHost->connexion, CurHost->upsname,
			 UPSCLI_LIST_VARS, vars, sizeof(vars)) < 0) */
		{
			DEBUGERR("Unable to get variable list for %s - %s\n", 
					CurHost->upsname, upscli_strerror(&CurHost->connexion));
		}
		else {
			DEBUGERR("Got variables list for %s@%s\n", 
					CurHost->upsname, CurHost->hostname);
					CurHost->comm_status = COM_OK;

			/* FIXME: LIST VAR seems to be necessary here (otherwise,
			 * we got an "Error: Protocol error" => check why */
			ret = upscli_list_next(&CurHost->connexion, numq, query, &numa, &answer);

			while (ret == 1) {
				/* VAR <upsname> <varname> <val> */
				if (numa < 4) {
					DEBUGERR("Error: insufficient data "
							"(got %d args, need at least 4)\n", numa);
					/* return EXIT_FAILURE; */
				}
				DEBUGERR("%s: %s\n", answer[2], answer[3]);
				ret = upscli_list_next(&CurHost->connexion, numq, query, &numa, &answer);
			}
		}

		if (strlen(vars) == 0) {
			DEBUGERR("%s", "No data available check your configuration (ups.conf)\n");
		}
		GetNextHost();
	}
}

/* init monitored UPS internal data */
void InitHosts() 
{
	int i;
  
	for ( i = 0 ; i >= 9 ; i++ ) {
		Hosts.Ups_list[i -1]->ups_status = -1;
		Hosts.Ups_list[i -1] = NULL;
	}
}

/* Clean all monitored UPS internal data */
void CleanHosts()
{
	int i;

	for ( i = 1 ; i <= Hosts.hosts_number ; i++ ) {
		upscli_disconnect ( &Hosts.Ups_list[i -1]->connexion );

		if ( Hosts.Ups_list[i -1] != NULL )
			free ( Hosts.Ups_list[i -1] );
	}
}

/* Add an UPS to be monitored
 * return 1 on success, 0 on failure (hostname already exist, ...) */
int AddHost(char *hostname)
{
	int nbHosts, ret;
	const	char	*query[4];
	unsigned int	numq, numa;
	char	**answer;
	char  newhostname[32];
	UPSCONN_t ups;

	DEBUGOUT("AddHost(%s)\n", hostname);

	if(Hosts.hosts_number < 9) {
		/* CurHost = Hosts.Ups_list[nbHosts -1]; */

		/* UPS auto discovery mode : */
		if (strchr(hostname, '@') == NULL) {
			/* Connect to host... */
			if (upscli_connect(&ups, hostname, 3493, UPSCLI_CONN_TRYSSL) < 0) 
				return 0;

			/* ... and retrieve UPS list */
			query[0] = "UPS";
			numq = 1;

			ret = upscli_list_start(&ups, numq, query);

			if (ret < 0) {
				return 0;
				/* FIXME: check for an old upsd */
			}

			ret = upscli_list_next(&ups, numq, query, &numa, &answer);

			while (ret == 1) {
				/* UPS <upsname> "<description>" */
				if (numa < 3) {
					fprintf(stderr, "Error: insufficient data "
							"(got %d args, need at least 4)\n", numa);

					return 0;
				}

				DEBUGOUT("%s: %s\n", hostname, answer[1]);

				sprintf(newhostname, "%s@%s", answer[1], hostname);
				if (AddHost(newhostname)) {
					/* break; */
					newhostname[0] = '\0';
				}
				ret = upscli_list_next(&ups, numq, query, &numa, &answer);
			}
			/* if(&ups) */
			/* 	    { */
			upscli_disconnect ( &ups );
			/* 	    } */
			return 1;
		}

		Hosts.hosts_number++;
		nbHosts = Hosts.hosts_number;
		/*		Hosts.Ups_list[nbHosts -1] = (ups_info *)xmalloc(sizeof(ups_info)); */
		Hosts.Ups_list[nbHosts -1] = (ups_info *)malloc(sizeof(ups_info));

		if(Hosts.Ups_list[nbHosts -1] == NULL)
			return 0;

		upscli_splitname(hostname, &Hosts.Ups_list[nbHosts -1]->upsname, 
						&Hosts.Ups_list[nbHosts -1]->hostname, 
						&Hosts.Ups_list[nbHosts -1]->port);

		Hosts.Ups_list[nbHosts -1]->hostnumber = nbHosts;
		Hosts.Ups_list[nbHosts -1]->ups_status = -1;
		Hosts.Ups_list[nbHosts -1]->comm_status = COM_LOST;
		Hosts.Ups_list[nbHosts -1]->battery_percentage = -1;
		Hosts.Ups_list[nbHosts -1]->battery_load = -1;
		Hosts.Ups_list[nbHosts -1]->battery_runtime = -1;

		return 1;
	}
	else
		fprintf(stderr, "Error: Maximum number (9) of monitored hosts reached");

	return 0;
}

void GetFirstHost()
{
	Hosts.curhosts_number = 1;

	/* align to tab */
	CurHost = Hosts.Ups_list[Hosts.curhosts_number - 1];

	DEBUGOUT("(First) Monitored host : %d (%s)\n",
		Hosts.curhosts_number, CurHost->upsname);
}

int GetNextHost()
{
	if(Hosts.curhosts_number < Hosts.hosts_number)
		Hosts.curhosts_number++;
	else /* loop from last to first */
		Hosts.curhosts_number = 1;

	/* align to tab */
	CurHost = Hosts.Ups_list[Hosts.curhosts_number - 1];

	DEBUGOUT("(Next) Monitored host : %d (%s)\n",
		Hosts.curhosts_number, CurHost->upsname);

	return Hosts.curhosts_number;
}

int GetPrevHost()
{
	if(Hosts.curhosts_number > 1)
		Hosts.curhosts_number--;
	else /* loop from first to last */
		Hosts.curhosts_number = Hosts.hosts_number;

	/* align to tab */
	CurHost = Hosts.Ups_list[Hosts.curhosts_number - 1];

	DEBUGOUT("(Prev) Monitored host : %d (%s)\n",
			Hosts.curhosts_number, CurHost->hostname);

	return Hosts.curhosts_number;
}
