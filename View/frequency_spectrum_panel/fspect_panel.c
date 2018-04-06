/*
 * freq_spect_panel.c
 *
 *  Created on: Apr 6, 2017
 *      Author: stefano
 */

#include "fspect_panel.h"
#include "../View.h"
#include "../../Controller.h"
#include <allegro.h>
#include <stdio.h>

#define 	RED 	(makecol(255,0,0))

extern style fspect_panel_style;
extern char FSPECTPANEL_BTN_PATHS[][64];

fspect_panel_t fspect_panel;

static const unsigned char ZOOM_TO_BAR[FSPANEL_MAX_ZOOM + 1] = { 227, 191, 155, 119,
		83, 47, 11 };

unsigned int compute_bar_height(unsigned int magnitude[], unsigned int size,
		unsigned int reference, unsigned int max_h);

static int fspect_panel_zoom_init(unsigned char zoom);
static int fspect_panel_bars_init();
static void fspect_panel_btns_init();
//void fspect_panel_draw();
//void fspect_panel_clear();

/*
 * @brief 				Initialize the frequency spectrum panel;
 *
 * @param panel 		The panel to initialize;
 *
 * @param feat			Position where the panel have to be initialized;
 *
 * @detail				Initialize the graphical feature of each bar within the panel depending on the
 * 						graphical feature of the panel given by the second parameter
 * 						The number of bars displayed at the beginning are standard and are defined by
 * 						FSPANEL_STD_NBARS;
 *
 * @return				0 on success, -1 if panel doesn't exist;
 */

int fspect_panel_init() {
	style *s = &fspect_panel_style;
	// Background
	rectfill(screen, s->x, s->y, s->x + s->w, s->y + s->h, s->bgcol);
	// Border Window
	rect(screen, s->x, s->y, s->x + s->w, s->y + s->h, s->col);

	fspect_panel.zoom = FSPANEL_MAX_ZOOM + 1;
	fspect_panel_zoom_init(FSPANEL_STD_ZOOM);
	fspect_panel_btns_init();

	return 0;
}

static int fspect_panel_zoom_init(unsigned char zoom) {
	if (zoom < FSPANEL_MAX_ZOOM + 1 && fspect_panel.zoom != zoom) {
		fspect_panel.zoom = zoom;
		return fspect_panel_bars_init();
	}
	return -1;
}

static int fspect_panel_bars_init() {
	style *s = &fspect_panel_style;
	int bar_x, bar_y;
	int bar_w, bar_h;
	int bar_col, bar_bgcol;

	unsigned int nbar, bar_width;

	nbar = ZOOM_TO_BAR[fspect_panel.zoom];

	bar_x = s->x + 1;
	bar_y = s->y + 1;
	bar_w = ((s->w - 2) / nbar);
	bar_h = s->h - 2;
	bar_bgcol = s->bgcol;
	bar_col = FSPANEL_BAR_COLOR;

	for (int i = 0; i < nbar; i++) {
		bar_ctor(&fspect_panel.bars[i], bar_x, bar_y, bar_w, bar_h, bar_col,
				bar_bgcol);
		bar_x += bar_w;
		bar_col = FSPANEL_BAR_COLOR * (i * fspect_panel.zoom + 1);
	}
	return 0;
}

static void fspect_panel_btns_init() {
	style *s = &fspect_panel_style;
	int btn_x, btn_y, btn_w, btn_h;
	btn_h = ((s->h) * FSPANEL_ZBTNS_H);
	btn_w = ((s->w) * FSPANEL_ZBTNS_W);
	btn_x = s->x + s->w * (1 - (2 * FSPANEL_ZBTNS_W));
	printf("btn_x: %d\nbtn_y: %d\nbtn_w: %d\nbtn_w: %d\n", btn_x, btn_y, btn_w,
			btn_h);
	btn_y = s->y + s->h * (1 - FSPANEL_ZBTNS_H);
	button_ctor(&(fspect_panel.zoomin), FSPECTPANEL_BTN_PATHS[0], btn_x, btn_y,
			btn_w, btn_h, ZOOMIN_BTN);
	btn_x += btn_w;
	printf("btn_x: %d\nbtn_y: %d\nbtn_w: %d\nbtn_w: %d\n", btn_x, btn_y, btn_w,
			btn_h);
	button_ctor(&(fspect_panel.zoomout), FSPECTPANEL_BTN_PATHS[1], btn_x, btn_y,
			btn_w, btn_h, ZOOMIN_BTN);

}

void fspect_panel_deinit() {

}

void fspect_panel_clear() {
	bar *bar;
	int nbar;
	nbar = ZOOM_TO_BAR[fspect_panel.zoom];
	for (int i = 0; i < nbar; i++) {
		bar = &fspect_panel.bars[i];
		bar_clear(bar);
	}
}

void fspect_panel_draw() {
	int nbar;
	nbar = ZOOM_TO_BAR[fspect_panel.zoom];
	for (int i = 0; i < nbar; i++) {
		bar_draw(&(fspect_panel.bars[i]));
	}
	button_draw(&(fspect_panel.zoomin));
	button_draw(&(fspect_panel.zoomout));
}

/* @brief	Update the spectrum of the panel
 *
 * @param[in]	panel		subject panel;
 *
 * @param[in]	magnitude[]	the input data stream representing frequency magnitudes of each frequency;
 *
 * @param[in]	size		size of the input data stream;
 *
 * @detail 					Given a stream of data that represent the magnitude of the frequency update
 * 							each bar in the frequency spectrum;
 *
 * @return					0 on success, -1 in case either panel or magnitude don't exist;
 */

int fspect_panel_update(unsigned int magnitude[], unsigned int size) {
	int max_value;
	int band_size;
	int step_size;
	int height;
	int nbar;
	style *s = &fspect_panel_style;
	unsigned int *magn;
	if (magnitude != NULL) {
		max_value = magnitude[size - 1];
		nbar = ZOOM_TO_BAR[fspect_panel.zoom];
		// how to group frequencies in order to represent them????
		band_size = size / nbar;
		magn = &magnitude[1];
		size--;
		for (int i = 0; i < nbar; i++) {
			step_size = ((i + 1) * band_size > size) ? size : band_size;
			height = compute_bar_height(&magn[i * band_size], step_size,
					max_value, s->h);
			bar_stretch(&fspect_panel.bars[i], -1, height);
		}
		button_draw(&(fspect_panel.zoomin));
		button_draw(&(fspect_panel.zoomout));
	}
	return -1;
}

/*
 * @brief
 * @param[in]	panel
 * @return
 * @detail
 */

int fspect_panel_zoomin() {
	if (fspect_panel.zoom < FSPANEL_MAX_ZOOM) {
		fspect_panel_clear();
		fspect_panel_zoom_init(fspect_panel.zoom + 1);
		fspect_panel_draw();
		return fspect_panel.zoom;
	}
	return -1;
}

/*
 * @brief
 * @param[in]	panel
 * @return
 * @detail
 */
int fspect_panel_zoomout() {
	if (fspect_panel.zoom > 0) {
		fspect_panel_clear();
		fspect_panel_zoom_init(fspect_panel.zoom - 1);
		fspect_panel_draw();
		return fspect_panel.zoom;
	}
	return -1;
}

/*
 * @brief	compute the height of a bar depending on a set of values and a reference value
 * @param	magnitude[]		the set of value on which the height is calculated
 * @param	size			size of the magnitude set
 * @param	reference		the reference value that represent the bar at maximum height
 * @return	the computed height in pixel
 */
unsigned int compute_bar_height(unsigned int magnitude[], unsigned int size,
		unsigned int reference, unsigned int max_h) {
	unsigned int average;
	if (magnitude != NULL && size > 0) {
		average = 0;
		for (int i = 0; i < size; i++) {
			average += magnitude[i];
		}
		average /= size;
		if (average > reference) {
			average = reference;
		}
		return max_h * average / reference;
	}
	return -1;
}

/**********************************************************************************************
 * 											TEST
 *********************************************************************************************/
void test1() {
	style *s = &fspect_panel_style;
	bar* b;
	for (int j = 0; j < ZOOM_TO_BAR[fspect_panel.zoom]; j++) {
		for (int i = 0; i < s->h; i++) {
			b = &(fspect_panel.bars[j]);
			bar_stretch(b, -1, bar_geth(b) - 1);
			usleep(1500);
		}
	}
	for (int j = 0; j < ZOOM_TO_BAR[fspect_panel.zoom]; j++) {
		for (int i = 0; i < s->h; i++) {
			b = &(fspect_panel.bars[j]);
			bar_stretch(b, -1, bar_geth(b) + 1);
			usleep(1500);
		}
	}
}

void test2() {
	unsigned int magnitude[1024];
	magnitude[0] = 512 * 1025;
	for (int i = 1; i < 1024; i++) {
		magnitude[i] = i;
	}
	fspect_panel_update(magnitude, 1024);
}
