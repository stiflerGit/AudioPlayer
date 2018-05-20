#include "graphic.h"
#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include <assert.h>

#define handle_error(s) \
	do{ perror(s); exit(EXIT_FAILURE);}while(0);

static void g_draw_text(Node *n);
static void g_draw_img(Node *n);

char is_inside(Node *n, int x, int y)
{
	assert(n != NULL);
	if(	x > n->x && x < n->x + n->w &&
		y > n->y && y < n->y + n->h)
		return 1;
	return 0;
}

void g_draw(Node *n)
{
	assert(n != NULL);
	scare_mouse();
	switch(n->type){
	case LINE:
		line(screen, n->x, n->y, n->x + n->w, n->y + n->h, n->fg);
		break;
	case FRAME:
		rect(screen, n->x, n->y, n->x + n->w, n->y + n->h, n->fg);
		break;
	case BAR:
		rectfill(screen, n->x, n->y, n->x + n->w, n->y + n->h, n->fg);
		break;
	case TEXT:
		g_draw_text(n);
		break;
	case IMG:
		g_draw_img(n);
		break;
	default:
		break;
	}
	unscare_mouse();
}

static void g_draw_text(Node *n)
{
text	*me;

	assert(n!=NULL);
	assert(n->type == TEXT);
	assert(n->dp != NULL);
	me = n->dp;
	switch (me->align) {
	case left:
		textout_ex(screen, font, me->str, n->x, n->y, n->fg, n->bg);
		break;
	case centre:
		textout_centre_ex(screen, font, me->str, n->x, n->y, n->fg,
				n->bg);
		break;
	case right:
		textout_right_ex(screen, font, me->str, n->x, n->y, n->fg,
				n->bg);
		break;
	default:
		break;
	}
}

static void g_draw_img(Node *n)
{
img	*me;
	assert(n!=NULL);
	assert(n->type == IMG);
	assert(n->dp != NULL);
	me = n->dp;
	if (me->_img == NULL) {
		me->_img = load_bitmap(me->path, NULL);
		if (me->_img == NULL){
			printf("load_birmap: %s\n", me->path);
			handle_error("load_bitmap");
		}
	}
	if(n->bg != TSPRNT){
		rectfill(screen, n->x, n->y, n->x + n->w, n->y + n->h, n->bg);
	}
	if(n->fg != TSPRNT){
		rectfill(screen, n->x, n->y, n->x + n->w, n->y + n->h, n->fg);
	}
	stretch_sprite(screen, me->_img, n->x, n->y, n->w, n->h);
}

void g_stretch(Node *n, int x, int y, int w, int h)
{

	assert(n != NULL);
	g_clear(n);
	n->x = x;
	n->y = y;
	n->w = w;
	n->h = h;
	g_draw(n);
}

static void g_clear_text(Node *n);
void g_clear(Node * n)
{
	assert(n != NULL);
	scare_mouse();
	switch(n->type){
	case LINE:
		line(screen, n->x, n->y, n->x + n->w, n->y + n->h, n->bg);
		break;
	case FRAME:
		rect(screen, n->x, n->y, n->x + n->w, n->y + n->h, n->bg);
		break;
	case BAR:
	case IMG:
		rectfill(screen, n->x, n->y, n->x + n->w, n->y + n->h, n->bg);
		break;
	case TEXT:
		g_clear_text(n);
		break;
	default:
		break;
	}
	unscare_mouse();
}

static void g_clear_text(Node *n)
{
text	*me;

	assert(n!=NULL);
	assert(n->type == TEXT);
	assert(n->dp != NULL);
	me = n->dp;
	switch (me->align) {
	case left:
		textout_ex(screen, font, me->str, n->x, n->y, n->bg, n->bg);
		break;
	case centre:
		textout_centre_ex(screen, font, me->str, n->x, n->y, n->bg,
				n->bg);
		break;
	case right:
		textout_right_ex(screen, font, me->str, n->x, n->y, n->bg,
				n->bg);
		break;
	default:
		break;
	}
}

/*
int bar_clear(bar *me) {
	rectfill(screen, me->x, me->y, me->x + me->w, me->y + me->h, me->bg);
	me->y += me->h;
	me->h = 0;
	return 0;
}

int bar_move(bar *me, int x, int y) {
	scare_mouse();
	rectfill(screen, me->x, me->y, me->x + me->w, me->y + me->h, me->bg);
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
		col = (delta < 0) ? me->col : me->bg;
//		printf("rectfill(screen, %d, %d, %d, %d, %X)\n",me->x, me->y, me->x + me->w, me->y + delta, col);
		rectfill(screen, me->x, me->y, me->x + me->w, me->y + delta, col);
		me->y += delta;
		me->h = h;
	}
	if (me->w != w && w > 0) {
		delta = me->h - w;
		col = (delta < 0) ? me->col : me->bg;
		rectfill(screen, me->x, me->y, me->x + delta, me->y + me->h, col);
		me->x += delta;
		me->w = w;
	}
	unscare_mouse();
	return 0;
}


void button_clear(button *me)
{
	rectfill(screen, me->x, me->y, me->x + me->w, me->y + me->h, me->bg);
}

void button_highlight(button *me)
{
	rectfill(screen, me->x, me->y, me->x + me->w, me->y + me->h, !(me->bg));
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
	printf("bg: %x\n", me->bg);

}

void text_setstr(text *me, const char *str){
	strcpy(me->str, str);
	text_draw(me);
}

void text_clear(text *me) {
	switch (me->align) {
	case left:
		textout_ex(screen, font, me->str, me->x, me->y, me->bg, me->bg);
		break;
	case centre:
		textout_centre_ex(screen, font, me->str, me->x, me->y, me->bg,
				me->bg);
		break;
	case right:
		textout_right_ex(screen, font, me->str, me->x, me->y, me->bg,
				me->bg);
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
*/
