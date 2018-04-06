/*
 * control_panel.h
 *
 *  Created on: Apr 6, 2017
 *      Author: stefano
 */

#ifndef GUI_CONTROL_PANEL_H_
#define GUI_CONTROL_PANEL_H_

#include "../graphic_framework/button.h"

#define CTRLPANEL_NBTN 6

//Control-Panel
typedef struct {
	button btns[CTRLPANEL_NBTN];
} control_panel_t;

extern control_panel_t control_panel;

int control_panel_init();
void control_panel_deinit();

void control_panel_draw();

#endif /* GUI_CONTROL_PANEL_H_ */
