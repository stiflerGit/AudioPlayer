/*
 * text.c
 *
 *  Created on: May 4, 2017
 *      Author: stefano
 */

#include "../../View/graphic_framework/text.h"

#include <allegro.h>
#include <string.h>
#include <stdio.h>

int text_ctor(text *me, const char *str, int x, int y, int col, int bgcol, alignment align) {
	if (str != NULL) {
		memset(me->str, '\0', sizeof(me->str));
		strcpy(me->str, str);
		me->x = x;
		me->y = y;
		me->col = col;
		me->bgcol = bgcol;
		me->align = align;
		text_draw(me);
		return 0;
	}
	return -1;
}

void text_dtor(text *me) {
	//DO NOTHING
}

void text_draw(text *me) {
	switch (me->align) {
	case left:
		textout_ex(screen, font, me->str, me->x, me->y, me->col, me->bgcol);
		break;
	case centre:
		textout_centre_ex(screen, font, me->str, me->x, me->y, me->col,
				me->bgcol);
		break;
	case right:
		textout_right_ex(screen, font, me->str, me->x, me->y, me->col,
				me->bgcol);
		break;
	default:
		break;
	}
}

void text_setstr(text *me, const char *str){
	strcpy(me->str, str);
	text_draw(me);
}

void text_clear(text *me) {
	switch (me->align) {
	case left:
		textout_ex(screen, font, me->str, me->x, me->y, me->bgcol, me->bgcol);
		break;
	case centre:
		textout_centre_ex(screen, font, me->str, me->x, me->y, me->bgcol,
				me->bgcol);
		break;
	case right:
		textout_right_ex(screen, font, me->str, me->x, me->y, me->bgcol,
				me->bgcol);
		break;
	default:
		break;
	}
}

void text_slidel(text *me)
{
char	slide[100];
char	first[2];

	first[0] = me->str[0];
	first[1] = '\0';
	strcpy(slide, &me->str[1]);
	strcat(slide, first);
	text_setstr(me, slide);
}

void text_slider(text *me)
{
char	slide[100];
char	*last;

	strcpy(&slide[1], me->str);
	last = index(slide, '\0');
	last--;
	slide[0] = *last;
	*last = '\0';
	strcpy(me->str, slide);
}
