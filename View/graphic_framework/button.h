/*
 * button.h
 *
 *  Created on: May 2, 2017
 *      Author: stefano
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include <allegro.h>

typedef struct{
	char		path[256];
	int			x, y;
	int			w, h;
	int			bgcol;
	char		evt;
	BITMAP*		_img;
}button;

int button_ctor(button *me, const char* path, int x, int y, int w, int h,int bgcol, char evt);
void button_xtor(button *me);
void button_highlight(button *me);
void button_draw(button *me);
void button_clear(button *me);
void button_print(button *me);

#endif /* BUTTON_H_ */
