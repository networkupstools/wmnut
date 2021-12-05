#ifndef WMGENERAL_H_INCLUDED
#define WMGENERAL_H_INCLUDED

/* X11 includes */
#include <X11/X.h>
#include <X11/xpm.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

  /***********/
 /* Defines */
/***********/

/* Configuration files */
#define MAINRC_FILE "/etc/wmnutrc"
#define RC_FILE     ".wmnutrc"


#define MAX_MOUSE_REGION (8)

#define TYPE_NULL	0
#define TYPE_INT	1
#define TYPE_FLOAT	2
#define TYPE_STRING	3
#define TYPE_BOOL	4

  /************/
 /* Typedefs */
/************/

union var {
  int   *integer;
  float *floater;
  char  *str;
  int   *bool;
};

typedef struct {
  /*const*/ char *label;
  int        type;
  union var  var;
}rckeys;

typedef struct {
  Pixmap	pixmap;
  Pixmap	mask;
  XpmAttributes	attributes;
} XpmIcon;

  /*******************/
 /* Global variable */
/*******************/

extern Display	*display;

  /***********************/
 /* Function Prototypes */
/***********************/


extern void CleanHosts();
extern int AddHost(char *hostname);
extern void GetFirstHost();
extern int GetNextHost();
extern int GetPrevHost();

void AddMouseRegion(int index, int left, int top, int right, int bottom);
int  CheckMouseRegion(int x, int y);

void SetWindowName(char *name);
void openXwindow(int argc, char *argv[], char **, char *, int, int, int withdrawn);
void RedrawWindow(void);
void RedrawWindowXY(int x, int y);
void CheckX11Events();
void pressEvent(XButtonEvent *xev);

void copyXPMArea(int, int, int, int, int, int);
void copyXBMArea(int, int, int, int, int, int);
void setMaskXY(int, int);

void AddRcKey(rckeys *key, const char *label, int type, void *var);
void ParseRCFile(const char *filename, rckeys *keys);
void ParseCMDLine(int argc, char *argv[]);
void LoadRCFile(rckeys *keys);
void ReloadRCFile();

  /************/
 /* Datadefs */
/************/
extern int     Alert;  	 /* Controls whether alert is sent to
					    all users via wall: Off by default  */
/* base parameters */
extern char    *upshost;
extern int     Verbose;	/* 1 for verbose mode : displays NUT 
					   available features and base values */
extern int     CriticalLevel;
extern int     LowLevel;
extern float   BlinkRate;	 /* blinks per second */
extern float   UpdateRate;   /* Number of updates per second */
extern int     Beep;	 /* Controls beeping when you get to 
					    CriticalLevel: Off by default */
extern int     Volume;	 /* ring bell at 50% volume */
extern int     UseLowColorPixmap; 	 /* Use a lower number of colors for the
					    poor saps on 8-bit displays */
extern float   LAlertRate; /* send alert every 5 minutes when Low */
extern float   CAlertRate; /* send alert every 2 minutes when Critical */
extern int     WithDrawn;	 /* start in withdrawn shape (for WindowMaker) */

#endif
