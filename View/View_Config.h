/*
 * View_Config.h
 *
 *  Created on: Mar 27, 2017
 *      Author: stefano
 */

#ifndef VIEW_CONFIG_H_
#define VIEW_CONFIG_H_

#include "graphic.h"

#define WIN_W	641	/**< Window Width. */
#define WIN_H	481	/**< Window Height */
#define	COL_D	32	/**< Color Depth. */

#define NPANEL		5

#define SPECT_PANEL	0
#define EQULZ_PANEL	1
#define TITLE_PANEL	2
#define CTRL_PANEL	3
#define POS_PANEL	4

extern Node	*nodes[NPANEL];
extern int	nodes_size[NPANEL];

/*
 * SPECTOGRAM PANEL
 */
#define SPECT_X		(0)
#define SPECT_Y		(0)
#define SPECT_W		(WIN_W)
#define SPECT_H		(WIN_H * 0.29)
#define SPECT_COL	WHITE
#define SPECT_BGCOL	BLACK

#define SPECT_NNOD	3

#define SPECT_FRAME	0
#define ZOOMOUT_BTN	1
#define ZOOMIN_BTN	2

/*
 *	EQUALIZATOR PANEL
 */
#define	EQLZ_X		(0)
#define	EQLZ_Y		(WIN_H * 0.29)
#define	EQLZ_W		(WIN_W)
#define	EQLZ_H		(WIN_H * 0.33)
#define EQLZ_COL	WHITE
#define EQLZ_BGCOL	BLACK

#define	EQLZ_NNOD	18

#define EQLZ_FRAME	0
#define EQLZ_SEP1	1
#define EQLZ_SEP2	2
#define	LFRQ_BAR	3
#define	MFRQ_BAR	4
#define	MHFRQ_BAR	5
#define	HFRQ_BAR	6
#define	VOL_BAR		7
#define	LFRQ_SBAR	8
#define	MFRQ_SBAR	9
#define	MHFRQ_SBAR	10
#define HFRQ_SBAR	11
#define VOL_SBAR	12
#define	LFRQ_LBL	13
#define	MFRQ_LBL	14
#define	MHFRQ_LBL	15
#define	HFRQ_LBL	16
#define	VOL_LBL		17


/*
 * TITLE PANEL
 */
#define TITLE_X		(0)
#define TITLE_Y		(WIN_H * 0.62)
#define TITLE_W		(WIN_W)
#define TITLE_H		(WIN_H * 0.13)
#define TITLE_COL	WHITE
#define TITLE_BGCOL	BLACK

#define TITLE_NNOD	2

#define TITLE_FRAME	0
#define TITLE_TXT	1

/*
 * CONTROL PANEL
 */
#define CTRL_X		(0)
#define CTRL_Y		(WIN_H * 0.75)
#define CTRL_W		(WIN_W)
#define CTRL_H		(WIN_H * 0.13)
#define CTRL_COL	WHITE
#define CTRL_BGCOL	BLACK

#define CTRL_NNOD	6

#define CTRL_FRAME	0
#define RWND_BTN	1
#define STOP_BTN	2
#define PAUSE_BTN	3
#define PLAY_BTN	4
#define FRWD_BTN	5

/*
 * POSITION PANEL
 */
#define POS_X		(0)
#define POS_Y		(WIN_H * 0.88)
#define POS_W		(WIN_W)
#define POS_H		(WIN_H * 0.13)
#define POS_COL		WHITE
#define POS_BGCOL	BLACK

#define POS_NNOD	5

#define POS_FRAME	0
#define POS_BAR		1
#define POS_SETB	2
#define POS_SEP		3
#define POS_TIME	4

#endif
