/**
 * @file	View_Config.h
 * @author	Stefano Fiori
 * @date	
 * @brief	
 */

#ifndef VIEW_CONFIG_H_
#define VIEW_CONFIG_H_

#include "graphic.h"

#define WIN_W	642	/**< Window Width. */
#define WIN_H	642	/**< Window Height */
#define	COL_D	32	/**< Color Depth. */

#define NPANEL		7

#define TIME_PANEL	0
#define ORIG_SP_PANEL	1
#define FILT_SP_PANEL	2
#define EQULZ_PANEL	3
#define TITLE_PANEL	4
#define CTRL_PANEL	5
#define POS_PANEL	6

extern Node	*nodes[NPANEL];
extern int	nodes_size[NPANEL];

/*******************************************************************************
 * 			TIME DATA PANEL
 ******************************************************************************/
#define TIMEP_X		(0)
#define TIMEP_Y		(0)
#define TIMEP_W		(WIN_W)
#define TIMEP_H		(WIN_H * 0.184)
#define TIMEP_FG	WHITE
#define TIMEP_BG	BLACK

#define TIMEP_NNOD	1

#define TIMEP_FRAME	0

/*******************************************************************************
 * 			ORIGINAL SPECTOGRAM PANEL
 ******************************************************************************/
#define ORIG_SP_P_X	(0)
#define ORIG_SP_P_Y	(WIN_H * 0.184)
#define ORIG_SP_P_W	(WIN_W)
#define ORIG_SP_P_H	(WIN_H * 0.184)
#define ORIG_SP_P_FG	WHITE
#define ORIG_SP_P_BG	BLACK

#define ORIG_SP_P_NNOD	3

#define ORIG_SP_P_FRAME	0
#define ZOOMOUT_BTN	1
#define ZOOMIN_BTN	2

/*******************************************************************************
 * 			FILTERED SPECTOGRAM PANEL
 ******************************************************************************/
#define FILT_SP_P_X	(0)
#define FILT_SP_P_Y	(WIN_H * 0.368)
#define FILT_SP_P_W	(WIN_W)
#define FILT_SP_P_H	(WIN_H * 0.184)
#define FILT_SP_P_COL	WHITE
#define FILT_SP_P_BGCOL	BLACK

#define FILT_SP_P_NNOD	3

#define FILT_SP_P_FRAME	0
#define ZOOMOUT_BTN	1
#define ZOOMIN_BTN	2

/*******************************************************************************
 *			EQUALIZATOR PANEL
 ******************************************************************************/
#define	EQLZP_X		(0)
#define	EQLZP_Y		(WIN_H * 0.552)
#define	EQLZP_W		(WIN_W)
#define	EQLZP_H		(WIN_H * 0.211)
#define EQLZP_COL	WHITE
#define EQLZP_BGCOL	BLACK

#define	EQLZP_NNOD	23

#define EQLZP_FRAME	0
#define EQLZP_SEP1	1
#define EQLZP_SEP2	2
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
#define LFRQ_GAIN_LBL	18
#define MFRQ_GAIN_LBL	19
#define MHFRQ_GAIN_LBL	20
#define HFRQ_GAIN_LBL	21
#define VOL_VAL_LBL	22

/*******************************************************************************
 *			TITLE PANEL
 ******************************************************************************/
#define TITLEP_X	(0)
#define TITLEP_Y	(WIN_H * 0.763)
#define TITLEP_W	(WIN_W)
#define TITLEP_H	(WIN_H * 0.079)
#define TITLEP_COL	WHITE
#define TITLEP_BGCOL	BLACK

#define TITLEP_NNOD	2

#define TITLEP_FRAME	0
#define TITLEP_TXT	1

/*******************************************************************************
 *			CONTROL PANEL
 ******************************************************************************/
#define CTRLP_X		(0)
#define CTRLP_Y		(WIN_H * 0.842)
#define CTRLP_W		(WIN_W)
#define CTRLP_H		(WIN_H * 0.079)
#define CTRLP_COL	WHITE
#define CTRLP_BGCOL	BLACK

#define CTRLP_NNOD	6

#define CTRLP_FRAME	0
#define RWND_BTN	1
#define STOP_BTN	2
#define PAUSE_BTN	3
#define PLAY_BTN	4
#define FRWD_BTN	5

/*******************************************************************************
 *			POSITION PANEL
 ******************************************************************************/
#define POSP_X		(0)
#define POSP_Y		(WIN_H * 0.921)
#define POSP_W		(WIN_W)
#define POSP_H		(WIN_H * 0.079)
#define POSP_COL	WHITE
#define POSP_BGCOL	BLACK

#define POSP_NNOD	5

#define POSP_FRAME	0
#define POSP_BAR	1
#define POSP_SETB	2
#define POSP_SEP	3
#define POSP_TIME	4

#endif
