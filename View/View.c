#include <stdio.h>
#include <allegro.h>
#include "View.h"
#include "View_Config.h"
#include "../Model/Player.h"

#define handle_error(e) \
	do{ perror(e); exit(EXIT_FAILURE);} while(1);


/* ********* Spectogram structure **************/
#define		MAXZOOM		5
static const int ZOOM_TO_BAR[6] = { 227, 191, 155, 119, 83, 47};

struct fspect_panel_t{
	Node 		bars[227];
	unsigned char 	zoom;
}fspect_panel;
/***********************************************/

static Player	old_p;	/**< Previous player state. */

static void print_f(float *buf, int size)
{
	printf("const float buf[%d] = {\n", size);
	printf("\t%f", buf[0]);
	for(int i = 1; i < size; i++){
		printf("\t%f", buf[i]);
		if(i % 5)
			printf("\n");
	}
	printf("\n}\n");
}

/*
 * 	SPECTOGRAM PANEL
 */
static int fspect_panel_bars_init()
{
int	bar_x;
int	bar_col;
int	nbar;
int	i;

	nbar = ZOOM_TO_BAR[fspect_panel.zoom];

	bar_x = SPECT_X + 2;
	bar_col = SPECT_COL;

	for (i = 0; i < nbar; i++) {
		fspect_panel.bars[i] = (Node )
			{ BAR, 
			bar_x, SPECT_Y + SPECT_H - 1, ((SPECT_W - 2) / nbar) - 2, 0,
			bar_col, SPECT_BGCOL, 0, NULL};
		bar_x += (SPECT_W - 2) / nbar;
		bar_col = rand() % (0x80000000);
	}

	g_draw(&(nodes[SPECT_PANEL][ZOOMIN_BTN]));
	g_draw(&(nodes[SPECT_PANEL][ZOOMOUT_BTN]));

	return 0;
}

static float avg(float *v, int size)
{
int	i;
float	val;

	val = 0;
	for(i = 0; i < size; i++){
		val += v[i];
	}

	return val / ((float) size);
}

static void fspect_bar_update(int i)
{
int	height;
int	delta;
int	col;
int	nbar;
Node	*n;

	nbar = ZOOM_TO_BAR[fspect_panel.zoom];
	height = avg(&(p.spectogram[i * (PLAYER_WINDOW_SIZE_CPX / nbar)]),
		PLAYER_WINDOW_SIZE_CPX / nbar);
	height = (SPECT_H * height) / 100;
//	printf("bar_stretch(%d, -1, %d)\n",i, height);
	n = &fspect_panel.bars[i];
	delta = n->h - height;
	if(delta == 0)
		return;
	col = (delta < 0) ? n->fg : n->bg;
//	printf("rectfill(screen, %d, %d, %d, %d, %X)\n",n->x, n->y, n->x + n->    w, n->y + delta, col);
	scare_mouse();
	rectfill(screen, n->x, n->y, n->x + n->w, n->y + delta, col);
	if(is_inside(&nodes[SPECT_PANEL][ZOOMIN_BTN], n->x, n->y))
		g_draw(&nodes[SPECT_PANEL][ZOOMIN_BTN]);
	if(is_inside(&nodes[SPECT_PANEL][ZOOMOUT_BTN], n->x, n->y))
		g_draw(&nodes[SPECT_PANEL][ZOOMOUT_BTN]);
	unscare_mouse();
	n->y += delta;
	n->h = height;
}

int fspect_panel_zoomin()
{
	if (fspect_panel.zoom <= MAXZOOM) {
		fspect_panel.zoom++;
		fspect_panel_bars_init();
		//fspect_panel_draw();
		return fspect_panel.zoom;
	}
	return -1;
}

int fspect_panel_zoomout()
{
	if (fspect_panel.zoom > 0) {
		fspect_panel.zoom--;
		fspect_panel_bars_init();
		//fspect_panel_draw();
		return fspect_panel.zoom;
	}
	return -1;
}

/*
 *	TITLE PANEL
 */
static void title_blink()
{
int	col;

	col = nodes[TITLE_PANEL][TITLE_TXT].fg;
	nodes[TITLE_PANEL][TITLE_TXT].fg = nodes[TITLE_PANEL][TITLE_TXT].bg;
	nodes[TITLE_PANEL][TITLE_TXT].bg = col;
	g_draw(&nodes[TITLE_PANEL][TITLE_TXT]);
}

static void title_reset()
{
text	*t;

	t = (text *) nodes[TITLE_PANEL][TITLE_TXT].dp;
	strcpy(t->str, p.trackname);
	strcat(t->str, "       ");
	g_draw(&nodes[TITLE_PANEL][TITLE_TXT]);
}

void title_slide(int speed)
{
Node	*t;
char	slide[100];
int	len;
static	int	step = 0;

	step = (step + 1) % 15;
	if((step % 15) != 0)
		return;
	t = &nodes[TITLE_PANEL][TITLE_TXT];
	len = strlen(((text *)(t->dp))->str);
	strcpy(slide, &((text *)(t->dp))->str[(len - speed) % len]);
	strncat(slide, ((text *)(t->dp))->str, (len - speed) % len);
	slide[len] = '\0';
	strcpy(((text *)(t->dp))->str, slide);
	g_draw(t);
}

/*
 *	CONTROL PANEL
 */
static void highlight(Node *n)
{
	if(n->fg == TSPRNT)
		n->fg = RED;
	else
		n->fg = TSPRNT;
	g_draw(n);
}

static void draw_state()
{
static int	step = 0;

	if(old_p.state != p.state){
		switch (old_p.state) {
		case STOP:
			highlight(&nodes[CTRL_PANEL][STOP_BTN]);
			break;
		case PLAY:
			highlight(&nodes[CTRL_PANEL][PLAY_BTN]);
			break;
		case PAUSE:
			highlight(&nodes[CTRL_PANEL][PAUSE_BTN]);
			break;
		case REWIND:
			highlight(&nodes[CTRL_PANEL][RWND_BTN]);
			break;
		case FORWARD:
			highlight(&nodes[CTRL_PANEL][FRWD_BTN]);
			break;
		default:
			break;
		}
		switch (p.state) {
		case STOP:
			// stop button highlighted
			highlight(&nodes[CTRL_PANEL][STOP_BTN]);
			break;
		case PLAY:
			// play button highlighted
			highlight(&nodes[CTRL_PANEL][PLAY_BTN]);
			break;
		case PAUSE:
			// pause button highlighted
			highlight(&nodes[CTRL_PANEL][PAUSE_BTN]);
			break;
		case REWIND:
			// rewind button highlighted
			highlight(&nodes[CTRL_PANEL][RWND_BTN]);
			break;
		case FORWARD:
			// forward button highlighted
			highlight(&nodes[CTRL_PANEL][FRWD_BTN]);
			break;
		default:
			break;
		}
	}
	step = (step + 1) % 30;
	switch (p.state) {
		case STOP:
			// title is static
			title_reset();
			break;
		case PLAY:
			// title slide to the left
			if((step % 10) == 0)
				title_slide(1);
			break;
		case PAUSE:
			// title stopped blinking
			if(step  == 0)
				title_blink();
			break;
		case REWIND:
			// title slide faster toward right
			if((step % 10) == 0)
				title_slide(-2);
			break;
		case FORWARD:
			// title slide faster towards left
			if((step % 10) == 0)
				title_slide(2);
			break;
		default:
			break;

	}
}

/*
 *	VIEW
 */
int view_init()
{
int	i, j;
Node	*t;

	set_color_depth(COL_D);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, WIN_W, WIN_H, 0, 0);
	clear_to_color(screen, 0);
	
	t = &nodes[TITLE_PANEL][TITLE_TXT];
	strcpy(((text *)t->dp)->str, p.trackname);
	strcat(((text *)t->dp)->str, "       ");

	for(i = 0; i < NPANEL; i++){
		for(j = 0; j < nodes_size[i]; j++){
			g_draw(&(nodes[i][j]));
		}
	}

	fspect_panel.zoom = 5;
	fspect_panel_bars_init();

	memcpy(&old_p, &p, sizeof(Player));
	highlight(&nodes[CTRL_PANEL][STOP_BTN]);

	install_mouse();
	show_mouse(screen);

	return 0;
}


void view_update()
{
int	i, j;
int	nbv;
int	spv;
int	pix;
Node	*n;

	// PLAYER STATE
	draw_state();
	old_p.state = p.state;
	// PLAYER TIME
	if(old_p.time != p.time){
		n = &nodes[POS_PANEL][POS_BAR];
		// position set bar update
		pix = n->w * p.time / p.duration + n->x;
		n = &nodes[POS_PANEL][POS_SETB];
		g_stretch(n, pix, n->y, n->w, n->h);
		// time text update
		n = &nodes[POS_PANEL][POS_TIME];
		sprintf(((text *)(n->dp))->str, "%02d:%02d", ((int)p.time) / 60, ((int)p.time) % 60);
		g_draw(n);
		old_p.time = p.time;
	}
	// PLAYER SPECTOGRAM
	nbv = ZOOM_TO_BAR[fspect_panel.zoom];
	spv = PLAYER_WINDOW_SIZE_CPX / nbv;
	for(i = 0; i < PLAYER_WINDOW_SIZE_CPX; i++){
		if(old_p.spectogram[i] != p.spectogram[i]){
			j = i / spv;
			fspect_bar_update(j);
			if(j < nbv)
				memcpy(&old_p.spectogram[j * spv], &p.spectogram[j * spv], spv);
			else
				memcpy(&old_p.spectogram[j * spv], &p.spectogram[j * spv], PLAYER_WINDOW_SIZE_CPX % nbv);
			i = (j + 1) * spv;
		}
	}
	// PLAYER VOLUME
	if(old_p.volume != p.volume){
		n = &nodes[EQULZ_PANEL][VOL_BAR];
		pix = -((float) (n->h * p.volume) / (float) 100) + n->y + n->h;
		n = &nodes[EQULZ_PANEL][VOL_SBAR];
		g_stretch(n, n->x, pix, n->w, n->h);
		old_p.volume = p.volume;
	}
	// PLAYER EQUALIZ
	for(int i = 0; i < PLAYER_NFILT; i++){
		if(old_p.equaliz[i].gain != p.equaliz[i].gain){
			n = &nodes[EQULZ_PANEL][LFRQ_BAR + i];
			pix = - (n->h *  p.equaliz[i].gain) / 24 + n->y + n->h / 2;
			n = &nodes[EQULZ_PANEL][LFRQ_SBAR + i];
			g_stretch(n, n->x, pix, n->w, n->h);
			old_p.equaliz[i].gain = p.equaliz[i].gain;
		}
	}
}
