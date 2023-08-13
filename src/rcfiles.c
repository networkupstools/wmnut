/*
 * rcfiles.c: useful functions to deal with parameters from rc files
 * or command line
 *
 * Copyright (C)
 *   2002 - 2012  Arnaud Quette <arnaud.quette@free.fr>
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "wmgeneral.h"
#include "config.h"

#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#endif


/*******************************************************************************\
|* AddRcKey                                                                     |
\*******************************************************************************/
void AddRcKey(rckeys *key, const char *label, int type, void *var) {

  if(label != NULL) {
/*    key->label = (char *)xmalloc(strlen(label) + 1); */
    key->label = (char *)malloc(strlen(label) + 1);
    strcpy(key->label, label);
  }
  else
    key->label = NULL;

  key->type = type;

  switch(type) {
  case TYPE_STRING :
    key->var.str = (char *)var;
    break;
  case TYPE_BOOL: 
  case TYPE_INT:
    key->var.integer = (int *)var;
    break;
  case TYPE_FLOAT:
    key->var.floater = (float *)var;
    break;
  case TYPE_NULL:
    break;
  }
}

/* TODO : void FreeRcKeys(rckeys *key)*/

/*******************************************************************************\
|* ParseRCFile		  						        |
\*******************************************************************************/
void ParseRCFile(const char *filename, rckeys *keys)
{
	char	*p, *tmp;
	char	temp[128];
	char	*tokens = " =\t\n#";
	FILE	*fp;
	int	key;
	
	fp = fopen(filename, "r");
	if (fp) {
#ifdef DEBUG
		printf("Opening rc file %s\n", filename);
#endif
		while (fgets(temp, 128, fp)) {
			if (temp[0] != '#') {
				key = 0;
				while (key >= 0 && keys[key].label) {
					if ((p = strstr(&temp[0], keys[key].label))) {
						/* jump to "=" */
						p += strlen(keys[key].label);
						/* suppress tokens before value */
						p += strspn(p, tokens);
						/* suppress tokens after value */
						tmp = strpbrk(p, tokens);
						*tmp = '\0';
#ifdef DEBUG
						printf("Parameter -> %s%s\tValue -> %s\n", keys[key].label, 
							((strlen(keys[key].label) <12)?"\t\t":"\t"), p);
#endif
						switch(keys[key].type)
						{
							case TYPE_STRING:
								/*			if (keys[key].var.str !=NULL)
								free(keys[key].var.str);
								keys[key].var.str = (char *)xmalloc(strlen(p) + 1);				
								strncpy(keys[key].var.str, p, strlen(p)); 
								*/
								if(!strcmp(keys[key].label, "UPS"))
									AddHost(p);
								break;
							case TYPE_BOOL:    
								if (!strncmp(p, "on", 2))
									*keys[key].var.bool = 1;
								else
									*keys[key].var.bool = 0;
								break;
							case TYPE_INT:
								*keys[key].var.integer = atoi(p);
								break;
							case TYPE_FLOAT:
								*keys[key].var.floater = atof(p);
								break;
						}
						key = -1;
					} else
						key++;
				}
			}
		}
		fclose(fp);
	}
}

/*******************************************************************************\
|* LoadRCFile		  						        |
\*******************************************************************************/
void LoadRCFile(rckeys *keys)
{
	char	home_file[128];
	char	*p;

#ifdef DEBUG
    printf("Loading rc files\n");
#endif

	ParseRCFile(MAINRC_FILE, keys);
	
	p = getenv("HOME");
	sprintf(home_file, "%s/%s", p, RC_FILE);
	ParseRCFile(home_file, keys);
}

/*******************************************************************************\
|* ParseCMDLine()  
\*******************************************************************************/
void ParseCMDLine(int argc, char *argv[])
{
	char *cmdline, *p;
	int  i, c;

#ifdef HAVE_GETOPT_LONG
		static struct option long_options[] =
		{
			{"alarmint",required_argument,NULL,'A'},
			{"blinkrate",required_argument,NULL,'b'},
			{"beepvol",required_argument,NULL,'B'},
			{"criticalevel",required_argument,NULL,'C'},
			{"display",required_argument,NULL,'d'},
			{"help",no_argument,NULL,'h'},
			{"lowcolor",no_argument,NULL,'l'},
			{"lowlevel",required_argument,NULL,'L'},
			{"upsname",required_argument,NULL,'U'},
			{"version",no_argument,NULL,'v'},
			{"verbose",no_argument,NULL,'V'},
			{"windowed",no_argument,NULL,'w'},
			{0, 0, 0, 0}
		};
		
		#define GETOPTFUNC(x,y,z) getopt_long(x,y,"-" z, long_options, NULL)
		#define GETOPTENDCHAR -1
#else   /* ! HAVE_GETOPT_LONG */
		#define GETOPTFUNC(x,y,z) getopt(x,y,z)
		#define GETOPTENDCHAR EOF
#endif

	while(1)
	{
	  if ((c=GETOPTFUNC (argc, argv, "A:b:B:C:d:hlL:U:vVw")) == GETOPTENDCHAR)
			break;

		switch (c)
		{
			case 'A': 
				Alert = 1;
				printf ("option A : valeur %s\n", optarg);
				p = strchr(optarg, ',');
				if (p != NULL)
					*p = '\0';
				LAlertRate = atof(optarg);
				CAlertRate = atof(p+1);
				printf ("option A : valeur LAlertRate = %f(%s), CAlertRate = %f(%s)\n",
					LAlertRate, optarg, CAlertRate, (p+1));
				break;
			case 'b': 
				BlinkRate = atof(optarg);
				break;
			case 'C': 
				CriticalLevel = atoi(optarg);
				break;
			case 'L': 
				LowLevel = atoi(optarg);
				break;
			case 'l': 
				UseLowColorPixmap = 1;
				break;
			case 'V': 
				Verbose = 1;
				break;
			case 'v':   
				printf("\nThis is wmnut version: %s\n", VERSION);
				printf("\nCopyright 2001-2016 Arnaud Quette <%s>\n", PACKAGE_BUGREPORT);
				printf("\nComplete documentation for WMNUT should be found on this system using\n");
				printf("`man wmnut' or `wmnut -h'.  If you have access to the Internet, point your\n");
				printf("browser at https://github.com/networkupstools/wmnut - the WMNUT Repository.\n\n");
				/* Previously http://wmnut.mgeops.org, now defunct; last cached at
				 * http://web.archive.org/web/20111003175836/http://wmnut.mgeops.org/
				 */
				exit(1);
			case 'w': 
				WithDrawn = 0; /* not in default withdrawn mode, so in windowed mode */
				break;
			case 'B': 
				Beep = 1;
				Volume = atoi(optarg);
				break;
			case 'U': 
				AddHost(optarg);
				break;
			case 'h':
			default:  
				printf("\n%s version: %s\n", PACKAGE, VERSION);
				printf("Usage: %s [arguments]\n\n", PACKAGE_NAME);
				printf("-A <T1,T2>\tSend messages to users terminals when Low and critical.\n");
				printf("             \tT1 is seconds between messages when Low.\n"); 
				printf("             \tT2 is seconds between messages when Critical.\n");
				printf("-b <BlinkRate>\tBlink rate for red LED. (0 for no blinking.)\n");
				printf("-B <Volume>\tBeep at Critical Level with Volume (between -100%% to 100%%).\n");
				printf("-C <CriticaLvl>\tDefine level at which red LED turns on (CriticalLevel).\n");
				printf("-d <display>\tUse alternate display.\n");
				printf("-h\t\tDisplay this help screen.\n");
				printf("-l\t\tUse a low-color pixmap to conserve colors on 8-bit displays.\n");
				printf("-L <LowLevel>\tDefine level at which yellow LED turns on.\n");
				printf("             \tCriticalLevel takes precedence if LowLevel < CriticalLevel.\n");
				printf("-U <upsname>\tDefine upsname ([upsname@]hostname, default is localhost)\n");
				printf("-v \t\tPrint version (includes important WMNUT info).\n");
				printf("-V \t\tVerbose mode : display NUT available features and base value.\n");
				printf("-w \t\tWindowed mode (opposite to native Window Maker withdrawn mode).\n\n");
				exit (1);
		}
	}
}
