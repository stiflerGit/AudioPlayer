/*
 * equalizator_panel.h
 *
 *  Created on: Apr 6, 2017
 *      Author: stefano
 */

#ifndef GUI_EQUALIZATOR_PANEL_H_
#define GUI_EQUALIZATOR_PANEL_H_

#include "../graphic_framework/graphic_framework.h"

//Equalizator Panel
typedef struct {
	gfeat_t pos;
	unsigned int n_bars;
	tbar_t* bars;
} eqlz_panel_t;

#endif /* GUI_EQUALIZATOR_PANEL_H_ */
