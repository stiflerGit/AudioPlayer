#include "view/view_config.h"

#include "defines.h"
#include "player/player.h"

/*******************************************************************************
 *			TIME DATA PANEL
 ******************************************************************************/
// TIME DATA PANEL IMPLEMENTATION
static Node time_nodes[TIMEP_NNOD] = {
	{FRAME, TIMEP_X, TIMEP_Y, TIMEP_W, TIMEP_H, TIMEP_FG, TIMEP_BG, 0, NULL}};

/*******************************************************************************
 *			ORIGNAL SPECTOGRAM PANEL
 ******************************************************************************/
static img zoomout =
	{STRCAT(IMAGES_PATH, "minus.bmp"), NULL};
static img zoomin =
	{STRCAT(IMAGES_PATH, "plus.bmp"), NULL};

static Node orig_sp_nodes[ORIG_SP_P_NNOD] = {
	{FRAME, ORIG_SP_P_X, ORIG_SP_P_Y, ORIG_SP_P_W, ORIG_SP_P_H,
	 WHITE, BLACK, 0, NULL},
	{IMG, ORIG_SP_P_X + 2, ORIG_SP_P_Y + 1, ORIG_SP_P_W * 0.05,
	 ORIG_SP_P_W * 0.05, BLACK, BLACK, 0, (void *)&zoomout},
	{IMG, ORIG_SP_P_X + 2 + ORIG_SP_P_W * 0.05, ORIG_SP_P_Y + 1,
	 ORIG_SP_P_W * 0.05, ORIG_SP_P_W * 0.05,
	 BLACK, BLACK, 0, (void *)&zoomin}};

/*******************************************************************************
 *			FILTERED SPECTOGRAM PANEL
 ******************************************************************************/
static Node filt_sp_nodes[FILT_SP_P_NNOD] = {
	{FRAME, FILT_SP_P_X, FILT_SP_P_Y, FILT_SP_P_W, FILT_SP_P_H,
	 WHITE, BLACK, 0, NULL},
	{IMG, FILT_SP_P_X + 2, FILT_SP_P_Y + 1, FILT_SP_P_W * 0.05,
	 FILT_SP_P_W * 0.05, BLACK, BLACK, 0, (void *)&zoomout},
	{IMG, FILT_SP_P_X + 2 + FILT_SP_P_W * 0.05, FILT_SP_P_Y + 1,
	 FILT_SP_P_W * 0.05, FILT_SP_P_W * 0.05,
	 BLACK, BLACK, 0, (void *)&zoomin}};

/*******************************************************************************
 *			EQUALIZATOR PANEL
 ******************************************************************************/
static text eq_lbl[5] = {
	{centre, "100Hz"},
	{centre, "1 KHz"},
	{centre, "5 KHz"},
	{centre, "12KHz"},
	{centre, "Vol"},
};

static text eq_gain_lbl[5] = {
	{right, "0 dB"},
	{right, "0 dB"},
	{right, "0 dB"},
	{right, "0 dB"},
	{right, "100"},
};

static Node eqlz_nodes[EQLZP_NNOD] = {
	// FRAME AND SEPARATORS
	{FRAME, EQLZP_X, EQLZP_Y, EQLZP_W, EQLZP_H, WHITE, BLACK, 0, NULL},
	{LINE, EQLZP_X + 0.82 * EQLZP_W, EQLZP_Y, 0, EQLZP_H, WHITE, BLACK, 0,
	 NULL},
	{LINE, EQLZP_X, EQLZP_Y + 0.7 * EQLZP_H, EQLZP_W, 0, WHITE, BLACK, 0, NULL},
	// BARS
	{BAR, EQLZP_X + EQLZP_W * 0.13, EQLZP_Y + 0.1 * EQLZP_H, EQLZP_W * 0.04,
	 EQLZP_H * 0.5, WHITE, BLACK, FILTLOW_SIG, NULL},
	{BAR, EQLZP_X + EQLZP_W * 0.30, EQLZP_Y + 0.1 * EQLZP_H, EQLZP_W * 0.04,
	 EQLZP_H * 0.5, WHITE, BLACK, FILTMED_SIG, NULL},
	{BAR, EQLZP_X + EQLZP_W * 0.47, EQLZP_Y + 0.1 * EQLZP_H, EQLZP_W * 0.04,
	 EQLZP_H * 0.5, WHITE, BLACK, FILTMEDHIG_SIG, NULL},
	{BAR, EQLZP_X + EQLZP_W * 0.64, EQLZP_Y + 0.1 * EQLZP_H, EQLZP_W * 0.04,
	 EQLZP_H * 0.5, WHITE, BLACK, FILTHIG_SIG, NULL},
	{BAR, EQLZP_X + EQLZP_W * 0.89, EQLZP_Y + 0.1 * EQLZP_H, EQLZP_W * 0.04,
	 EQLZP_H * 0.5, WHITE, BLACK, VOL_SIG, NULL},
	// SET BARS
	{BAR, EQLZP_X + EQLZP_W * 0.13, EQLZP_Y + 0.35 * EQLZP_H, 0.04 * EQLZP_W, 5,
	 RED, WHITE, 0, NULL},
	{BAR, EQLZP_X + EQLZP_W * 0.30, EQLZP_Y + 0.35 * EQLZP_H, 0.04 * EQLZP_W, 5,
	 RED, WHITE, 0, NULL},
	{BAR, EQLZP_X + EQLZP_W * 0.47, EQLZP_Y + 0.35 * EQLZP_H, 0.04 * EQLZP_W, 5,
	 RED, WHITE, 0, NULL},
	{BAR, EQLZP_X + EQLZP_W * 0.64, EQLZP_Y + 0.35 * EQLZP_H, 0.04 * EQLZP_W, 5,
	 RED, WHITE, 0, NULL},
	{BAR, EQLZP_X + EQLZP_W * 0.89, EQLZP_Y + 0.1 * EQLZP_H, 0.04 * EQLZP_W, 5,
	 RED, WHITE, 0, NULL},
	// LABELS
	{TEXT, EQLZP_X + EQLZP_W * 0.15, EQLZP_Y + 0.82 * EQLZP_H, EQLZP_W / 5,
	 EQLZP_H * 0.38, WHITE, BLACK, 0, (void *)&eq_lbl[0]},
	{TEXT, EQLZP_X + EQLZP_W * 0.32, EQLZP_Y + 0.82 * EQLZP_H, EQLZP_W / 5,
	 EQLZP_H * 0.38, WHITE, BLACK, 0, (void *)&eq_lbl[1]},
	{TEXT, EQLZP_X + EQLZP_W * 0.49, EQLZP_Y + 0.82 * EQLZP_H, EQLZP_W / 5,
	 EQLZP_H * 0.38, WHITE, BLACK, 0, (void *)&eq_lbl[2]},
	{TEXT, EQLZP_X + EQLZP_W * 0.66, EQLZP_Y + 0.82 * EQLZP_H, EQLZP_W / 5,
	 EQLZP_H * 0.38, WHITE, BLACK, 0, (void *)&eq_lbl[3]},
	{TEXT, EQLZP_X + EQLZP_W * 0.91, EQLZP_Y + 0.82 * EQLZP_H, EQLZP_W / 5,
	 EQLZP_H * 0.38, WHITE, BLACK, 0, (void *)&eq_lbl[4]},
	// GAIN LABELS
	{TEXT, EQLZP_X + EQLZP_W * 0.12, EQLZP_Y + EQLZP_H * 0.35, EQLZP_W * 0.20,
	 EQLZP_H * 0.38, WHITE, BLACK, 0, (void *)&eq_gain_lbl[0]},
	{TEXT, EQLZP_X + EQLZP_W * 0.29, EQLZP_Y + EQLZP_H * 0.35, EQLZP_W * 0.20,
	 EQLZP_H * 0.38, WHITE, BLACK, 0, (void *)&eq_gain_lbl[1]},
	{TEXT, EQLZP_X + EQLZP_W * 0.46, EQLZP_Y + EQLZP_H * 0.35, EQLZP_W * 0.20,
	 EQLZP_H * 0.38, WHITE, BLACK, 0, (void *)&eq_gain_lbl[2]},
	{TEXT, EQLZP_X + EQLZP_W * 0.63, EQLZP_Y + EQLZP_H * 0.35, EQLZP_W * 0.20,
	 EQLZP_H * 0.38, WHITE, BLACK, 0, (void *)&eq_gain_lbl[3]},
	{TEXT, EQLZP_X + EQLZP_W * 0.88, EQLZP_Y + EQLZP_H * 0.35, EQLZP_W * 0.20,
	 EQLZP_H * 0.38, WHITE, BLACK, 0, (void *)&eq_gain_lbl[4]},
};

/*******************************************************************************
 *			TITLE PANEL
 ******************************************************************************/
static text title_txt = {centre, ""};
static Node title_nodes[TITLEP_NNOD] = {
	{FRAME, TITLEP_X, TITLEP_Y, TITLEP_W, TITLEP_H, WHITE, BLACK, 0, NULL},
	{TEXT, TITLEP_X + TITLEP_W * 0.5, TITLEP_Y + TITLEP_H * 0.40, TITLEP_W,
	 TITLEP_H, TITLEP_COL, TITLEP_BGCOL, 0, (void *)&title_txt}};

/*******************************************************************************
 *			CONTROL PANEL
 ******************************************************************************/
#define CTRL_BW (CTRL_W / CTRL_NBTN)

static img ctrl_btns[5] = {
	{STRCAT(IMAGES_PATH, "rewind.bmp"), NULL},
	{STRCAT(IMAGES_PATH, "stop.bmp"), NULL},
	{STRCAT(IMAGES_PATH, "pause.bmp"), NULL},
	{STRCAT(IMAGES_PATH, "play.bmp"), NULL},
	{STRCAT(IMAGES_PATH, "forward.bmp"), NULL}};

static Node ctrl_nodes[CTRLP_NNOD] = {
	{FRAME, CTRLP_X, CTRLP_Y, CTRLP_W, CTRLP_H, WHITE, BLACK, 0, NULL},
	{IMG, CTRLP_X + 0.025 * CTRLP_W, CTRLP_Y + 0.1 * CTRLP_H, 0.15 * CTRLP_W,
	 0.8 * CTRLP_H, TSPRNT, BLACK, RWND_SIG, (void *)&ctrl_btns[0]},
	{IMG, CTRLP_X + 0.225 * CTRLP_W, CTRLP_Y + 0.1 * CTRLP_H, 0.15 * CTRLP_W,
	 0.8 * CTRLP_H, TSPRNT, BLACK, STOP_SIG, (void *)&ctrl_btns[1]},
	{IMG, CTRLP_X + 0.425 * CTRLP_W, CTRLP_Y + 0.1 * CTRLP_H, 0.15 * CTRLP_W,
	 0.8 * CTRLP_H, TSPRNT, BLACK, PAUSE_SIG, (void *)&ctrl_btns[2]},
	{IMG, CTRLP_X + 0.625 * CTRLP_W, CTRLP_Y + 0.1 * CTRLP_H, 0.15 * CTRLP_W,
	 0.8 * CTRLP_H, TSPRNT, BLACK, PLAY_SIG, (void *)&ctrl_btns[3]},
	{IMG, CTRLP_X + 0.825 * CTRLP_W, CTRLP_Y + 0.1 * CTRLP_H, 0.15 * CTRLP_W,
	 0.8 * CTRLP_H, TSPRNT, BLACK, FRWD_SIG, (void *)&ctrl_btns[4]},
};

/*******************************************************************************
 *			POSITION PANEL
 ******************************************************************************/
static text pos_txt = {left, "00:00"};
static Node pos_nodes[POSP_NNOD] = {
	{FRAME, POSP_X, POSP_Y, POSP_W, POSP_H, POSP_COL, POSP_BGCOL, 0, NULL},
	{BAR, POSP_X + 0.05 * POSP_W, POSP_Y + 0.21 * POSP_H, POSP_W * 0.75,
	 0.5 * POSP_H, POSP_COL, POSP_BGCOL, JUMP_SIG, NULL},
	{BAR, POSP_X + 0.05 * POSP_W, POSP_Y + 0.21 * POSP_H, 4, 0.5 * POSP_H, RED,
	 WHITE, 0, NULL},
	{LINE, POSP_X + 0.85 * POSP_W, POSP_Y, 0, POSP_H, WHITE, BLACK, 0, NULL},
	{TEXT, POSP_X + (POSP_W * 0.885), POSP_Y + (POSP_H * 0.45), POSP_W, POSP_H,
	 POSP_COL, POSP_BGCOL, 0, (void *)&pos_txt}};

/*******************************************************************************
 *			GLOBAL VARIABLES
 ******************************************************************************/
Node *nodes[NPANEL] =
	{time_nodes, orig_sp_nodes, filt_sp_nodes, eqlz_nodes, title_nodes,
	 ctrl_nodes, pos_nodes};

int nodes_size[NPANEL] =
	{TIMEP_NNOD, ORIG_SP_P_NNOD, FILT_SP_P_NNOD, EQLZP_NNOD, TITLEP_NNOD,
	 CTRLP_NNOD, POSP_NNOD};
