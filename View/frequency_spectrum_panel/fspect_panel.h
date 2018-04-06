/*
 * freq_spect_panel.h
 *
 *  Created on: Apr 6, 2017
 *      Author: stefano
 */

#ifndef GUI_FREQ_SPECT_PANEL_H_
#define GUI_FREQ_SPECT_PANEL_H_

#include "../graphic_framework/bar.h"
#include "../graphic_framework/button.h"

#define FSPANEL_MAX_BARS	255
#define FSPANEL_MIN_ZOOM	0
#define FSPANEL_MAX_ZOOM	6
#define FSPANEL_STD_ZOOM	6
#define FSPANEL_MAX_BAR_H	255

#define FSPANEL_ZBTNS_W			(0.100)
#define FSPANEL_ZBTNS_H			(0.2)

#define FSPANEL_BAR_COLOR	makecol(0,255,0)

typedef struct fspect_panel_t fspect_panel_t;
struct fspect_panel_t{
	bar bars[FSPANEL_MAX_BARS];
	button zoomin;
	button zoomout;
	unsigned char zoom;
};

extern fspect_panel_t fspect_panel;

int fspect_panel_init();
void fspect_panel_deinit();
void fspect_panel_draw();
void fspect_panel_clear();
int fspect_panel_update(unsigned int magnitude[],
		unsigned int size);
int fspect_panel_zoomin();
int fspect_panel_zoomout();

void test1();
void test2();

#endif /* GUI_FREQ_SPECT_PANEL_H_ */
