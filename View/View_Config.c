#include "../View/View_Config.h"

#ifndef	NULL
#define NULL	((void *) 0)
#endif	//NULL

static button	spect_btns[SPECT_NBTN] = {
	{"./images/plus.bmp", SPECT_X, SPECT_Y, 20, 20, WHITE},
	{"./images/minus.bmp", SPECT_X + 20, SPECT_Y, 20, 20, WHITE}
};

static text	title_txts[TITLE_NTXT] = {
		{ TITLE_X, TITLE_Y, TITLE_COL, TITLE_BGCOL, centre, ""}
};

#define CTRL_BW	(CTRL_W/CTRL_NBTN)
static button	ctrl_btns[CTRL_NBTN] = {
	{"./images/rewind.bmp", CTRL_X, CTRL_Y, CTRL_BW, CTRL_H, CTRL_BGCOL},
	{"./images/stop.bmp", CTRL_X + CTRL_BW, CTRL_Y, CTRL_BW, CTRL_H, CTRL_BGCOL},
	{"./images/play.bmp", CTRL_X + 2*(CTRL_BW), CTRL_Y, CTRL_BW, CTRL_H, CTRL_BGCOL},
	{"./images/pause.bmp", CTRL_X + 3 * (CTRL_BW), CTRL_Y, CTRL_BW, CTRL_H, CTRL_BGCOL},
	{"./images/forward.bmp", CTRL_X + 4 * (CTRL_BW), CTRL_Y, CTRL_BW, CTRL_H, CTRL_BGCOL},
//	{"./images/volumeup.bmp", CTRL_X + 500, CTRL_Y, 100, 100, CTRL_BGCOL},
//	{"./images/volumedown.bmp", CTRL_X + 600, CTRL_Y, 100, 100, WHITE}
};

static text	pos_txts[POS_NTXT] = {
	{ POS_X + ((9 * POS_W) / 10), POS_Y + (POS_H / 2), POS_COL, POS_BGCOL, centre, "00:00"}
};

static bar	pos_bars[POS_NBAR] = {
	{POS_X, POS_Y, ((4 * POS_W) / 5), POS_H, POS_COL, POS_BGCOL}
};


button *buttons[NPANEL] = { spect_btns, NULL, ctrl_btns, NULL };

int buttons_size[NPANEL] = { SPECT_NBTN, TITLE_NBTN, CTRL_NBTN, POS_NBTN };

text *texts[NPANEL] = { NULL, title_txts, NULL, pos_txts };

int texts_size[NPANEL] = { SPECT_NTXT, TITLE_NTXT, CTRL_NTXT, POS_NTXT };

bar *bars[NPANEL] = { NULL, NULL, NULL, pos_bars };

int bars_size[NPANEL] = { SPECT_NBAR, TITLE_NBAR, CTRL_NBAR, POS_NBAR };
