/*
 * position_panel.h
 *
 *  Created on: Apr 6, 2017
 *      Author: stefano
 */

#ifndef GUI_POSITION_PANEL_H_
#define GUI_POSITION_PANEL_H_

#include "../graphic_framework/bar.h"
#include "../graphic_framework/text.h"

#define PPANEL_TBAR_W 		(0.75 * position_panel_style.w)
#define PPANEL_TBAR_X1		(position_panel_style.x)
#define PPANEL_TBAR_X2		(position_panel_style.x + PPANEL_TBAR_W)

#define PPANEL_PBAR_W		(0.01 * position_panel_style.w)

#define PPANEL_TTXT_W		(0.25 * position_panel_style.w)
#define PPANEL_TTXT_X1		(PPANEL_TBAR_X2)
#define PPANEL_TTXT_X2		(style_getx2(&position_panel_style))

//Position-Panel
typedef struct {
	bar pos_bar;
	text time_text;
} position_panel_t;

extern position_panel_t position_panel;

int position_panel_init();
void position_panel_deinit();
void position_panel_draw_play();
void position_panel_draw_stop();
void position_panel_draw_unload();
void position_panel_draw_pause();
void position_panel_draw_finish();
void position_panel_clear();

#endif /* GUI_POSITION_PANEL_H_ */
