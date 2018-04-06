/*
 * control_panel.c
 *
 *  Created on: Apr 19, 2017
 *      Author: stefano
 */

#include "control_panel.h"
#include "../View.h"
#include "../../Controller.h"
#include <allegro.h>

#define handle_error(s) \
		do{ perror(s); exit(EXIT_FAILURE); } while(0)

#define RED			(makecol(255, 0, 0))

extern style control_panel_style;
extern char CTRLPANEL_BTN_PATHS[][64];
control_panel_t control_panel;

int control_panel_init() {

	unsigned int btn_x, btn_y, btn_w, btn_h;
	style *s = &control_panel_style;
	rect(screen, s->x, s->y, s->x + s->w, s->y + s->h, RED);

	btn_y = s->y;
	btn_w = (s->w) / 6;
	btn_h = s->h;
	for (int i = 0; i < CTRLPANEL_NBTN; i++) {
		btn_x = s->x + i * btn_w;
		button_ctor(&control_panel.btns[i], CTRLPANEL_BTN_PATHS[i], btn_x, btn_y, btn_w,
				btn_h, CTRLPANEL_EVTS[i]);
	}
	return 0;
}

void control_panel_deinit(){
	for (int i = 0; i < CTRLPANEL_NBTN; i++) {
			button_xtor(&control_panel.btns[i]);
	}
}

void control_panel_draw(){
	for (int i = 0; i < CTRLPANEL_NBTN; i++) {
		button_draw(&control_panel.btns[i]);
	}
}
