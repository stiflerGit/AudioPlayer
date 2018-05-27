#include "graphic.h"
#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include <assert.h>

#define handle_error(s) \
	do{ perror(s); exit(EXIT_FAILURE);}while(0);

static void g_draw_text(Node *n);
static void g_draw_img(Node *n);
static void g_clear_text(Node *n);

/**
 * @brief       Control if a 2D coordinate is inside an object area
 * @param[in]   n       address of the Obj.
 * @param[in]   x       x coordinate
 * @param[in]   y       y coordinate
 * @return      return 1 if the 2D coordinate is inside the Obj. Area.
 *              0 Otherwise.
 */
char is_inside(Node *n, int x, int y)
{
	assert(n != NULL);
	if(	x > n->x && x < n->x + n->w &&
		y > n->y && y < n->y + n->h)
		return 1;
	return 0;
}

/** 
 * @brief       Draw a graphic object on screen. 
 *
 * Switch among Node type to call the correct allegro function, more complex
 * object(the ones that needs addition object-specific data) are managed in 
 * dedicated functions.
 *
 * @param[in]   n       address of the Object to draw 
 */
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

/** 
 * @brief       Clear the screen from a graphic object.
 *
 * Switch among Node type to call the correct allegro function, more complex
 * object(the ones that needs addition object-specific data) are managed in 
 * dedicated functions.
 *
 * @param[in]   n       address of the Object to clear.
 */
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

/**
 * @brief	Change a position and/or size of a gr. obj. and draws it.
 *
 * It simply clear the old obj. than change its parameters and draws the obj
 * again.
 *
 * @param[in]	n	address of the Object to stretch.
 * @param[in]	x	new x coordinate.
 * @param[in]	y	new y coordinate.
 * @param[in]	w	new width.
 * @param[in]	h	new height.
 */
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

/*******************************************************************************
 *				TEXT
 ******************************************************************************/
/**
 * @brief	Dedicate Text draw function.
 *
 * Switch among alignment to call the right allegro function.
 *
 * @param[in]	n	address of the text Object.
 */
static void g_draw_text(Node *n)
{
text	*me;

	assert(n!=NULL);
	assert(n->type == TEXT && n->dp != NULL);
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

/**
 * @brief	Dedicated Text clear function.
 *
 * Switch among alignment and draw a text with the background color.
 *
 * @param[in]	n	address of the text Object.
 */
static void g_clear_text(Node *n)
{
text	*me;

	assert(n!=NULL);
	assert(n->type == TEXT && n->dp != NULL);
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

/*******************************************************************************
 *				IMAGE
 ******************************************************************************/
/**
 * @brief	Dedicate Image draw function.
 *
 * If the img_ pointer is equal to NULL, it is initialized with a call
 * to allegro load_bitmap function.
 * Image are managed with three layer, first a rect of background color,
 * second a rect of foreground color and than the image is drawn.
 *
 * @param[in]	n	address of the text Object.
 */
static void g_draw_img(Node *n)
{
img	*me;
	assert(n!=NULL);
	assert(n->type == IMG && n->dp != NULL);
	me = n->dp;
	if (me->_img == NULL) {
		me->_img = load_bitmap(me->path, NULL);
		if (me->_img == NULL){
			printf("load_bitmap: %s\n", me->path);
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
