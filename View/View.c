#include <stdio.h>
#include <allegro.h>
#include "View.h"
#include "View_Config.h"
#include "../Model/Player.h"

#define handle_error(e) \
	do{ perror(e); exit(EXIT_FAILURE);} while(1);

//static pstate old_state;	/**< Old Player State. */

/* ********* Spectogram structure **************/
#define		MAXZOOM		5
static const int ZOOM_TO_BAR[6] = { 227, 191, 155, 119, 83, 47};

struct fspect_panel_t{
	Node 		bars[227];
	unsigned char 	zoom;
}fspect_panel;
/***********************************************/

static Player	old_p;

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

void fspect_panel_clear()
{
int	nbar;
int	i;

	nbar = ZOOM_TO_BAR[fspect_panel.zoom];

	for (i = 0; i < nbar; i++) 
		g_clear(&fspect_panel.bars[i]);

	g_clear(&(nodes[SPECT_PANEL][ZOOMIN_BTN]));
	g_clear(&(nodes[SPECT_PANEL][ZOOMOUT_BTN]));
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
	rectfill(screen, n->x, n->y, n->x + n->w, n->y + delta, col);
	n->y += delta;
	n->h = height;
}

int fspect_panel_zoomin()
{
	if (fspect_panel.zoom <= MAXZOOM) {
		fspect_panel_clear();
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
		fspect_panel_clear();
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
int	x;

	x = n->fg;
	n->fg = n->bg;
	n->bg = x;
	g_draw(n);
}

static void state()
{
	switch (p.state) {
	case STOP:
		// title is static
		title_reset();
		// stop button highlighted
		highlight(&nodes[CTRL_PANEL][STOP_BTN]);
		break;
	case PLAY:
		// title slide to the left
		title_slide(1);
		// play button highlighted
		highlight(&nodes[CTRL_PANEL][PLAY_BTN]);
		break;
	case PAUSE:
		// title stopped blinking
		title_blink();
		// pause button highlighted
		highlight(&nodes[CTRL_PANEL][PAUSE_BTN]);
		break;
	case REWIND:
		// title slide faster toward right
		 title_slide(2);
		// rewind button highlighted
		highlight(&nodes[CTRL_PANEL][RWND_BTN]);
		break;
	case FORWARD:
		// title slide faster towards left
		title_slide(-2);
		// forward button highlighted
		highlight(&nodes[CTRL_PANEL][FRWD_BTN]);
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

	for(i = 0; i < NPANEL; i++){
		for(j = 0; j < nodes_size[i]; j++){
			g_draw(&(nodes[i][j]));
		}
	}

	fspect_panel.zoom = 5;
	fspect_panel_bars_init();

	memcpy(&old_p, &p, sizeof(Player));

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
char	upd;
Node	*n;

	// PLAYER STATE
	state();
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
	upd = 0;
	for(i = 0; i < PLAYER_WINDOW_SIZE_CPX; i++){
		if(old_p.spectogram[i] != p.spectogram[i]){
			j = i / spv;
			fspect_bar_update(j);
			if(j < nbv)
				memcpy(&old_p.spectogram[j * spv], &p.spectogram[j * spv], spv);
			else
				memcpy(&old_p.spectogram[j * spv], &p.spectogram[j * spv], PLAYER_WINDOW_SIZE_CPX % nbv);
			i = (j + 1) * spv;
			upd = 1;
		}
	}
	if(upd != 0){
		g_draw(&(nodes[SPECT_PANEL][ZOOMIN_BTN]));
		g_draw(&(nodes[SPECT_PANEL][ZOOMOUT_BTN]));
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

void view_exit() {
	remove_sound();
	allegro_exit();
}

/*
int main() {
	view_init();
	//	test1();
	printf("zoom:%d\n", 0);
	for (int i = 1; i < FSPANEL_MAX_ZOOM + 1; i++) {
		fspect_panel_zoomin(&fspect_panel);
		printf("zoom:%d\n", i);
		test2();
		readkey();
	}
	for (int i = 6; i >= 0; i--) {
		fspect_panel_zoomout(&fspect_panel);
		printf("zoom:%d\n", i);
		test2();
		readkey();
	}
	readkey();
	view_destroy();
}
*/
/*
int main()
{
	allegro_init();
	view_init();
	while(1){}
}
*/
