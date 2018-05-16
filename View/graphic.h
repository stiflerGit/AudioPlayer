#ifndef GRAPHIC_H_
#define GRAPHIC_H_

#include <allegro.h>

#ifndef	COLORS
#define TSPRNT	(0xff00ff)
#define BLACK	(0)
#define WHITE	(0xffffff)
#define RED	(0xff0000)
#define GREEN	(0x00ff00)
#define BLUE	(0x0000ff)
#endif

typedef enum {
	LINE, FRAME, BAR, TEXT, IMG
} NType;

typedef struct {
	NType	type;
	int	x, y, w, h;
	int	fg, bg;
	char	evt;
	void	*dp;
} Node;

void g_draw(Node *n);
void g_clear(Node *n);
void g_stretch(Node *n, int x, int y, int width, int height);
void g_move(Node *n, int x, int y);

// TEXT
typedef enum  {
	left,
	centre,
	right
}alignment;
typedef struct {
	alignment align;
	char str[1024];
} text;

// IMG
typedef struct{
	char		path[1024];
	BITMAP*		_img;
} img;

#endif
