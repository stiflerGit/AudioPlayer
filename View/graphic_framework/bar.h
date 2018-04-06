/*
 * bar.h
 *
 *  Created on: May 2, 2017
 *      Author: stefano
 */

#ifndef BAR_H_
#define BAR_H_

typedef struct{
	int				x, y;	// top-left corner coordinates
	unsigned int	w, h;	// bar width and height
	int 			col;	// bar color
	int 			bgcol;	// bar background color
}bar;

#define bar_getx(b)	((b)->x)
#define bar_gety(b)	((b)->y)
#define bar_getw(b)	((b)->w)
#define bar_geth(b)	((b)->h)

int bar_ctor(bar *me, int x, int y, int w, int h, int col, int bgcol);
int bar_xtor(bar *me);
int bar_draw(bar *me);
int bar_clear(bar *me);
int bar_move(bar *me, int x, int y);
int bar_stretch(bar *me, int w, int h);

#endif /* BAR_H_ */
