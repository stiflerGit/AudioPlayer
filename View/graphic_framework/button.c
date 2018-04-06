/*
 * button.c
 *
 *  Created on: May 2, 2017
 *      Author: stefano
 */

#include "../../View/graphic_framework/button.h"

#include <stdio.h>

#define handle_error(s) \
	do{ perror(s); exit(EXIT_FAILURE);}while(0);

int button_ctor(button *me, const char* path, int x, int y, int w, int h, int bgcol, char evt)
{
	me->_img = load_bitmap(path, NULL);
	if (me->_img == NULL)
		handle_error("load_bitmap");
	strcpy(me->path, path);
	me->x = x;
	me->y = y;
	me->w = w;
	me->h = h;
	me->bgcol = bgcol;
	me->evt = evt;
	return 0;
}

void button_draw(button *me){
	rectfill(screen, me->x, me->y, me->x + me->w, me->y + me->h, me->bgcol);
	stretch_sprite(screen, me->_img, me->x, me->y, me->w, me->h);
}

void button_clear(button *me)
{
	rectfill(screen, me->x, me->y, me->x + me->w, me->y + me->h, me->bgcol);
}

void button_highlight(button *me)
{
	rectfill(screen, me->x, me->y, me->x + me->w, me->y + me->h, !(me->bgcol));
	stretch_sprite(screen, me->_img, me->x, me->y, me->w, me->h);
}

void button_xtor(button *me)
{
	destroy_bitmap(me->_img);
}

void button_print(button *me)
{
	printf("path: %s\n", me->path);
	printf("x: %d\n", me->x);
	printf("y: %d\n", me->y);
	printf("w: %d\n", me->w);
	printf("h: %d\n", me->h);
	printf("bgcol: %x\n", me->bgcol);

}
