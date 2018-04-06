/*
 * car.c
 *
 *  Created on: May 2, 2017
 *      Author: stefano
 */

#include <stdio.h>
#include <allegro.h>
#include "../../View/graphic_framework/bar.h"

int bar_ctor(bar *me, int x, int y, int w, int h, int col, int bgcol) {
	me->x = x;
	me->y = y;
	me->w = w;
	me->h = h;
	me->col = col;
	me->bgcol = bgcol;
	bar_draw(me);
	return 0;
}

int bar_xtor(bar *me) {
	//DO NOTHING
	return 0;
}

int bar_draw(bar *me) {
	rectfill(screen, me->x, me->y, me->x + me->w, me->y + me->h, me->col);
	return 0;
}

int bar_clear(bar *me) {
	rectfill(screen, me->x, me->y, me->x + me->w, me->y + me->h, me->bgcol);
	me->y += me->h;
	me->h = 0;
	return 0;
}

int bar_move(bar *me, int x, int y) {
	scare_mouse();
	rectfill(screen, me->x, me->y, me->x + me->w, me->y + me->h, me->bgcol);
	me->x = (x > 0) ? x : me->x;
	me->y = (y > 0) ? y : me->y;
	rectfill(screen, me->x, me->y, me->x + me->w, me->y + me->h, me->col);
	unscare_mouse();
	return 0;
}

int bar_stretch(bar *me, int w, int h)
{
int	delta;
int	col;

	scare_mouse();
	if (me->h != h && h > 0) {
		delta = me->h - h;
		col = (delta < 0) ? me->col : me->bgcol;
//		printf("rectfill(screen, %d, %d, %d, %d, %X)\n",me->x, me->y, me->x + me->w, me->y + delta, col);
		rectfill(screen, me->x, me->y, me->x + me->w, me->y + delta, col);
		me->y += delta;
		me->h = h;
	}
	if (me->w != w && w > 0) {
		delta = me->h - w;
		col = (delta < 0) ? me->col : me->bgcol;
		rectfill(screen, me->x, me->y, me->x + delta, me->y + me->h, col);
		me->x += delta;
		me->w = w;
	}
	unscare_mouse();
	return 0;
}
