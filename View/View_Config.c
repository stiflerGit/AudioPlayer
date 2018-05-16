#include "View_Config.h"
#include "../Model/Player.h"

#ifndef	NULL
#define NULL	((void *) 0)
#endif	//NULL

/*
 *	SPECTOGRAM PANEL
 */
static img zoomout = { "/home/stefano/workspace/AudioPlayer/View/images/minus.bmp", NULL };
static img zoomin  = { "/home/stefano/workspace/AudioPlayer/View/images/plus.bmp", NULL };

static Node	spect_nodes[SPECT_NNOD] = {
	{ FRAME, SPECT_X, SPECT_Y, SPECT_W, SPECT_H, WHITE, BLACK, 0, NULL},
	{ IMG, SPECT_X + 1, SPECT_Y + 1, SPECT_W * 0.05, SPECT_W * 0.05, WHITE, BLACK, 0, (void *) &zoomout},
	{ IMG, SPECT_X + 2 + SPECT_W * 0.05, SPECT_Y + 1, SPECT_W * 0.05, SPECT_W * 0.05, WHITE, BLACK, 0 , (void *) &zoomin}
};

/*
 *	EQUALIZATOR PANEL
 */

static text	eq_lbl[5] = {
	{centre, "100Hz"},
	{centre, "1 KHz"},
	{centre, "5 KHz"},
	{centre, "12KHz"},
	{centre, "Vol"},
};
static Node	eqlz_nodes[EQLZ_NNOD] = {
	{FRAME, EQLZ_X, EQLZ_Y, EQLZ_W, EQLZ_H, WHITE, BLACK, 0, NULL},
	{LINE, EQLZ_X + 0.82 * EQLZ_W, EQLZ_Y, 0, EQLZ_H, WHITE, BLACK,0, NULL},
	{LINE, EQLZ_X, EQLZ_Y + 0.7 * EQLZ_H, EQLZ_W, 0,WHITE, BLACK, 0, NULL},
	{BAR, EQLZ_X + EQLZ_W * 0.13, EQLZ_Y + 0.1 * EQLZ_H, EQLZ_W * 0.04 , EQLZ_H * 0.5, WHITE, BLACK,  FILTLOW_SIG, NULL},
	{BAR, EQLZ_X + EQLZ_W * 0.30, EQLZ_Y + 0.1 * EQLZ_H, EQLZ_W * 0.04, EQLZ_H * 0.5, WHITE, BLACK,  FILTMED_SIG, NULL},
	{BAR, EQLZ_X + EQLZ_W * 0.47, EQLZ_Y + 0.1 * EQLZ_H, EQLZ_W * 0.04, EQLZ_H * 0.5, WHITE, BLACK,  FILTMEDHIG_SIG, NULL},
	{BAR, EQLZ_X + EQLZ_W * 0.64, EQLZ_Y + 0.1 * EQLZ_H, EQLZ_W * 0.04, EQLZ_H * 0.5, WHITE, BLACK,  FILTHIG_SIG, NULL},
	{BAR, EQLZ_X + EQLZ_W * 0.89, EQLZ_Y + 0.1 * EQLZ_H, EQLZ_W * 0.04, EQLZ_H * 0.5, WHITE, BLACK,  VOL_SIG, NULL},
	{LINE, EQLZ_X + EQLZ_W * 0.13, EQLZ_Y + 0.35 * EQLZ_H, 0.04 * EQLZ_W, 0, RED, WHITE,  0, NULL},
	{LINE, EQLZ_X + EQLZ_W * 0.30,  EQLZ_Y + 0.35 * EQLZ_H, 0.04 * EQLZ_W, 0, RED, WHITE,  0, NULL},
	{LINE, EQLZ_X + EQLZ_W * 0.47, EQLZ_Y + 0.35 * EQLZ_H, 0.04 * EQLZ_W, 0, RED, WHITE,  0, NULL},
	{LINE, EQLZ_X + EQLZ_W * 0.64, EQLZ_Y + 0.35 * EQLZ_H, 0.04 * EQLZ_W, 0, RED, WHITE,  0, NULL},
	{LINE, EQLZ_X + EQLZ_W * 0.89, EQLZ_Y + 0.35 * EQLZ_H, 0.04 * EQLZ_W, 0, RED, WHITE,  0, NULL},
	{TEXT, EQLZ_X + EQLZ_W * 0.15, EQLZ_Y + 0.82 * EQLZ_H, EQLZ_W / 5, EQLZ_H * 0.38, WHITE, BLACK, 0, (void *) &eq_lbl[0]},
	{TEXT, EQLZ_X + EQLZ_W * 0.32, EQLZ_Y + 0.82 * EQLZ_H, EQLZ_W / 5, EQLZ_H * 0.38, WHITE, BLACK, 0, (void *) &eq_lbl[1]},
	{TEXT, EQLZ_X + EQLZ_W * 0.49, EQLZ_Y + 0.82 * EQLZ_H, EQLZ_W / 5, EQLZ_H * 0.38, WHITE, BLACK, 0, (void *) &eq_lbl[2]},
	{TEXT, EQLZ_X + EQLZ_W * 0.66, EQLZ_Y + 0.82 * EQLZ_H, EQLZ_W / 5, EQLZ_H * 0.38, WHITE, BLACK, 0, (void *) &eq_lbl[3]},
	{TEXT, EQLZ_X + EQLZ_W * 0.91 , EQLZ_Y + 0.82 * EQLZ_H, EQLZ_W / 5, EQLZ_H * 0.38, WHITE, BLACK, 0, (void *) &eq_lbl[4]},
};

/*
 *	TITLE PANEL
 */

static text	title_txt = { centre, "" };
static Node	title_nodes[TITLE_NNOD] = {
	{ FRAME, TITLE_X, TITLE_Y, TITLE_W, TITLE_H, WHITE, BLACK, 0, NULL},
	{ TEXT, TITLE_X + TITLE_W * 0.5, TITLE_Y + TITLE_H * 0.40, TITLE_W, TITLE_H, TITLE_COL, TITLE_BGCOL, 0, (void *) &title_txt}
};

/*
 *	CONTROL_PANEL
 */
#define CTRL_BW	(CTRL_W/CTRL_NBTN)

static img	ctrl_btns[5] = {
	{"/home/stefano/workspace/AudioPlayer/View/images/rewind.bmp", NULL},
	{"/home/stefano/workspace/AudioPlayer/View/images/stop.bmp", NULL},
	{"/home/stefano/workspace/AudioPlayer/View/images/pause.bmp", NULL},
	{"/home/stefano/workspace/AudioPlayer/View/images/play.bmp", NULL},
	{"/home/stefano/workspace/AudioPlayer/View/images/forward.bmp", NULL}
};

static Node	ctrl_nodes[CTRL_NNOD] = {
	{ FRAME, CTRL_X, CTRL_Y, CTRL_W, CTRL_H, WHITE, BLACK, 0, NULL},
	{ IMG, CTRL_X + 0.025 * CTRL_W, CTRL_Y + 0.1 * CTRL_H, 0.15 * CTRL_W, 0.8 * CTRL_H, TSPRNT, BLACK, RWND_SIG, (void *) &ctrl_btns[0]},
	{ IMG, CTRL_X + 0.225 * CTRL_W, CTRL_Y + 0.1 * CTRL_H, 0.15 * CTRL_W, 0.8 * CTRL_H, TSPRNT, BLACK, STOP_SIG, (void *) &ctrl_btns[1]},
	{ IMG, CTRL_X + 0.425 * CTRL_W, CTRL_Y + 0.1 * CTRL_H, 0.15 * CTRL_W, 0.8 * CTRL_H, TSPRNT, BLACK, PAUSE_SIG, (void *) &ctrl_btns[2]},
	{ IMG, CTRL_X + 0.625 * CTRL_W, CTRL_Y + 0.1 * CTRL_H, 0.15 * CTRL_W, 0.8 * CTRL_H, TSPRNT, BLACK, PLAY_SIG, (void *) &ctrl_btns[3]},
	{ IMG, CTRL_X + 0.825 * CTRL_W, CTRL_Y + 0.1 * CTRL_H, 0.15 * CTRL_W, 0.8 * CTRL_H, TSPRNT, BLACK, FRWD_SIG, (void *) &ctrl_btns[4]},
};

/*
 *	POSITION PANEL
 */
static text 	pos_txt = {left, "00:00"};
static Node	pos_nodes[POS_NNOD] = {
	{ FRAME, POS_X, POS_Y, POS_W, POS_H, POS_COL, POS_BGCOL, 0, NULL},
	{ BAR, POS_X + 0.05 * POS_W, POS_Y + 0.21 * POS_H, POS_W * 0.75 , 0.5 * POS_H, POS_COL, POS_BGCOL, JUMP_SIG, NULL},
	{ BAR, POS_X + 0.05 * POS_W, POS_Y + 0.21 * POS_H, 4, 0.5 * POS_H, RED, WHITE, 0, NULL},
	{ LINE, POS_X + 0.85 * POS_W, POS_Y,  0, POS_H, WHITE, BLACK, 0, NULL},
	{ TEXT, POS_X + (POS_W * 0.885), POS_Y + (POS_H * 0.45), POS_W, POS_H, POS_COL, POS_BGCOL, 0, (void *) &pos_txt}
};

/*
 *	GLOBAL
 */
Node	*nodes[NPANEL] = {
	spect_nodes, eqlz_nodes, title_nodes, ctrl_nodes, pos_nodes
};
int	nodes_size[NPANEL] = { SPECT_NNOD, EQLZ_NNOD, TITLE_NNOD, CTRL_NNOD, POS_NNOD };
