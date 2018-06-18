/**
 * @file	graphic.h
 * @author	Stefano Fiori
 * @date	26 May 2018
 * @brief	Provide a simple graphic framework for GUI
 */
#ifndef GRAPHIC_H_
#define GRAPHIC_H_

#include <allegro.h>

// COLORS definition
#ifndef	COLORS
#define TSPRNT	(0xff00ff)	/**< TRANSPARENT. */
#define BLACK	(0)
#define WHITE	(0xffffff)
#define RED	(0xff0000)
#define GREEN	(0x00ff00)
#define BLUE	(0x0000ff)
#endif

/**
 * @brief	Node Type, types of graphic objects that can be managed.
 */
typedef enum {
	LINE,	/**< A one pixel line. */
	FRAME,	/**< A one pixel rectangle. */
	BAR,	/**< A rectangle filled with a color. */
	TEXT,	/**< Text. */
	IMG	/**< Image. */
} NType;

/**
 * @brief	Graphic Node structure.
 *
 * This represent the different objects drawn on the screen. 
 * Thanks to type, position and colors variable is possible to give
 * a first draw directives. Additional object-specific data are
 * needed for some types of object: text and images.
 * All object can become sensible to mouse click and hence raise 
 * an event thanks to the event variable.
 */
typedef struct {
	NType	type;		/**< Type of the Graphic Node. */
	int	x, y, w, h;	/**< X Y coordinates, Width, Height. */
	int	fg, bg;		/**< Foreground and background colors. */
	char	evt;		/**< Envent associated to the mouse click. */
	void	*dp;		/**< Pointer to more object-specific data. */
} Node;

/**
 * @brief	Draw a graphic object on screen.
 * @param[in]	n	address of the Object to draw
 */
void g_draw(Node *n);

/**
 * @brief	Clear the screen from a graphic object
 * @param[in]	n	address of the Object to clear
 */
void g_clear(Node *n);

/**
 * @brief	Change a position or size of an Gr. Obj. and draw its.
 * @param[in]   n       address of the Object to stretch.                       
 * @param[in]   x       new x coordinate.                                       
 * @param[in]   y       new y coordinate.                                       
 * @param[in]   w       new width.                                              
 * @param[in]   h       new height. 
 */
void g_stretch(Node *n, unsigned int x, unsigned int y, 
	unsigned int width, unsigned int height);

/**
 * @brief	Control if a 2D coordinate is inside an object area
 * @param[in]	n	adress of the Obj.
 * @param[in]	x	x coordinate
 * @param[in]	y	y coordinate
 * @return	1 if the 2D coordinate is inside the Obj. Area.
 *		0 Otherwise.
 */
char is_inside(Node *n, unsigned int x, unsigned int y);

/*******************************************************************************
 *				TEXT
 ******************************************************************************/
/**
 * @brief	enum for the alignment of text obj. 
 */
typedef enum  {
	left,	/**< Text start from the x,y coordinates. */
	centre,	/**< x, y coordinates are the center of the text. */
	right	/**< Text end in the x, y coordinates. */
}alignment;

/**
 * @brief	Text-specific data.
 *
 * When g_draw is called on a text-object ad dp is equal to NULL
 */
typedef struct {
	alignment 	align;		/**< Text alignment with respect to
					the x,y coordinates. */
	char 		str[1024];	/**< String of the text. */
} text;

/*******************************************************************************
 *				IMAGE
 ******************************************************************************/
 /**
  * @brief	Image-specific data
  * When g_draw is called on a image-object ad dp is equal to NULL
  */
typedef struct{
	char		path[1024];	/**< Path of the image. */
	BITMAP*		_img;		/**< Pointer to a 
					memory-loaded bitmap. */
} img;

#endif
