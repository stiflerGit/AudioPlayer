/*
 * position_panel.c
 *
 *  Created on: Apr 24, 2017
 *      Author: stefano
 */

#include "position_panel.h"
#include "../View.h"
#include <stdio.h>
#include <allegro.h>

#define RED (makecol(255,0,0))

#define handle_error(s) \
	do { perror(s); exit(EXIT_FAILURE);} while(0)

position_panel_t position_panel;

static int pos_bar_step;

#define ms_to_sec(ms)	\
	(ms / 1000)

static char* seconds_to_time(char *dst, int raw_seconds) {
	int h, m, s;

	s = raw_seconds;
	m = raw_seconds / 60;
	s -= (m * 60);
	h = raw_seconds / 3600;
	m -= (h * 60);
	sprinttf(dst, "%02d:%02d:%02d", h, m, s);

	return dst;
}

int position_panel_init() {
	int txt_x, txt_y, txt_w, txt_h;
	struct style *s = &position_panel_style;

	rectfill(screen, PPANEL_TBAR_X1, s->y, PPANEL_TBAR_X2, style_gety2(s),
			s->col);
	rectfill(screen, PPANEL_TTXT_X1, s->y, PPANEL_TTXT_X2, style_gety2(s),
			s->bgcol);

	rect(screen, s->x, s->y, s->x + s->w, s->y + s->h, RED);

	bar_ctor(&position_panel.pos_bar, s->x, s->y, PPANEL_PBAR_W, s->h, s->bgcol,
			s->col);

	txt_x = (PPANEL_TTXT_W / 2) + PPANEL_TTXT_X1;
	txt_y = s->y + (s->h / 2);
	text_ctor(&position_panel.time_text, "NO TRACK LOADED", txt_x, txt_y,
			s->col, s->bgcol, centre);

	return 0;
}

void position_panel_draw_load() {
	pos_bar_step = PPANEL_TBAR_W / ms_to_sec(time_dur);
	if (pos_bar_step < 1)
		pos_bar_step = 1;

	text_setstr(&position_panel.time_text, "00:00:00");
}

void position_panel_draw_play() {
	char time[100];
	int sec;

	bar_move(&position_panel.pos_bar,
	bar_getx(&position_panel.pos_bar) + pos_bar_step, -1);

	if (time < time_dur) {
		sec = ms_to_sec(time_dur);
		seconds_to_time(time, sec);
		text_setstr(&position_panel.time_text, time);
	}
}

void position_panel_draw_finish(){
	text_setstr(&position_panel.time_text, "END");
}

void position_panel_draw() {
	bar_draw(&position_panel.pos_bar);
	text_draw(&position_panel.time_text);
}

void position_panel_draw_stop(){
	style *s = &position_panel_style;
	bar_move(&position_panel.pos_bar, s->x, -1 );
	text_setstr(&position_panel.time_text, "00:00:00");
}

void position_panel_draw_unload();
void position_panel_draw_pause();
void position_panel_draw_finish();

void position_panel_clear() {
	struct style *s = &position_panel_style;

	rectfill(screen, PPANEL_TBAR_X1, s->y, PPANEL_TBAR_X2, style_gety2(s),
			s->col);
	rectfill(screen, PPANEL_TTXT_X1, s->y, PPANEL_TTXT_X2, style_gety2(s),
			s->bgcol);
}
