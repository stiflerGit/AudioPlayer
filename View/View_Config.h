/*
 * View_Config.h
 *
 *  Created on: Mar 27, 2017
 *      Author: stefano
 */

#ifndef VIEW_CONFIG_H_
#define VIEW_CONFIG_H_

#include "../View/graphic_framework/bar.h"
#include "../View/graphic_framework/button.h"
#include "../View/graphic_framework/text.h"
#include "../View/View.h"

#define		BLACK	(0)
#define 	WHITE	(0xffffff)
#define		RED		(0xff0000)
#define		GREEN	(0x00ff00)
#define		BLUE	(0x0000ff)

#define WIN_W	480	/**< Window Width. */
#define WIN_H	480		/**< Window Height */
#define	COL_D	32		/**< Color Depth. */

#define NPANEL		4

#define SPECT_PANEL	0
#define TITLE_PANEL	1
#define CTRL_PANEL	2
#define POS_PANEL	3

extern button		*buttons[NPANEL];
extern int			buttons_size[NPANEL];

extern text			*texts[NPANEL];
extern int			texts_size[NPANEL];

extern bar			*bars[NPANEL];
extern int			bars_size[NPANEL];

/*
 * SPECTOGRAM PANEL
 */
#define SPECT_X		(0)
#define SPECT_Y		(0)
#define SPECT_W		(WIN_W)
#define SPECT_H		((40 * WIN_H) / 100)
#define SPECT_COL	RED
#define SPECT_BGCOL	WHITE

#define SPECT_NBTN	2

#define ZOOMIN_BTN	0
#define ZOOMOUT_BTN	1

#define SPECT_NTXT	0

#define SPECT_NBAR	0

/*
 * TITLE PANEL
 */
#define TITLE_X		(WIN_W / 2)
#define TITLE_Y		((45 * WIN_H) / 100)
#define TITLE_W		(WIN_W)
#define TITLE_H		((10 * WIN_H) / 100)
#define TITLE_COL	BLACK
#define TITLE_BGCOL	WHITE

#define TITLE_NBTN	0

#define TITLE_NTXT	1

#define TITLE_TXT	0

#define TITLE_NBAR	0

/*
 * CONTROL PANEL
 */
#define CTRL_X		(0)
#define CTRL_Y		((60 * WIN_H) / 100)
#define CTRL_W		(WIN_W)
#define CTRL_H		((20 * WIN_H) / 100)
#define CTRL_COL	BLACK
#define CTRL_BGCOL	WHITE

#define CTRL_NBTN	5

#define RWND_BTN	0
#define STOP_BTN	1
#define PLAY_BTN	2
#define PAUSE_BTN	3
#define FRWD_BTN	4
//#define VOLDWN_BTN	5
//#define VOLUP_BTN	6

#define CTRL_NTXT	0

#define CTRL_NBAR	0

/*
 * POSITION PANEL
 */
#define POS_X		(0)
#define POS_Y		((85 * WIN_H) / 100)
#define POS_W		(WIN_W)
#define POS_H		((10 * WIN_H) / 100)
#define POS_COL		BLACK
#define POS_BGCOL	WHITE

// BUTTONS
#define POS_NBTN	0
// TEXTS
#define POS_NTXT	1

#define TIME_TXT	0
// BARS
#define POS_NBAR	1

#define POS_BAR		0

#endif
