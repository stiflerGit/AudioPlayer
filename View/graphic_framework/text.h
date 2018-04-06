/*
 * text.h
 *
 *  Created on: May 4, 2017
 *      Author: stefano
 */

#ifndef VIEW_GRAPHIC_FRAMEWORK_TEXT_H_
#define VIEW_GRAPHIC_FRAMEWORK_TEXT_H_

typedef enum  {
	left,
	centre,
	right
}alignment;

typedef struct {
	int x, y;
	int col, bgcol;
	alignment align;
	char str[1024];
} text;

int text_ctor(text *me, const char *str, int x, int y, int col, int bgcol, alignment align);
void text_dtor(text *me);
void text_draw(text *me);
void text_clear(text *me);
void text_setstr(text *me, const char *str);
void text_slidel(text *me);
void text_slider(text *me);

#endif /* VIEW_GRAPHIC_FRAMEWORK_TEXT_H_ */
