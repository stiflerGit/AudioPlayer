#include <stdio.h>
#include <allegro.h>
#include <math.h>
#include "../View/View.h"
#include "../View/View_Config.h"

#define handle_error(e) \
	do{ perror(e); exit(EXIT_FAILURE);} while(1);

//static pstate old_state;	/**< Old Player State. */

/* ********* Spectogram structure **************/
#define		MAXZOOM		6
static const int ZOOM_TO_BAR[7] = { 227, 191, 155, 119, 83, 47, 11 };

struct fspect_panel_t{
	bar bars[227];
	unsigned char zoom;
}fspect_panel;
/* ************************************* */

Player p;

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

static int fspect_panel_bars_init()
{
int				bar_x;
int				bar_col;
unsigned int	nbar;
int				i;

	nbar = ZOOM_TO_BAR[fspect_panel.zoom];

	bar_x = SPECT_X + 1;
	bar_col = SPECT_COL;

	for (i = 0; i < nbar; i++) {
		bar_ctor(&fspect_panel.bars[i], bar_x, SPECT_Y + 1,
				((SPECT_W-1) / nbar) - 1, SPECT_H - 2, bar_col,
				SPECT_BGCOL);
		bar_x += (SPECT_W / nbar) + 1;
		bar_col = rand() % (0x80000000);
	}

	button_draw(&(buttons[SPECT_PANEL][ZOOMIN_BTN]));
	button_draw(&(buttons[SPECT_PANEL][ZOOMOUT_BTN]));

	return 0;
}

void fspect_panel_clear()
{
bar	*bar;
int	nbar;
int	i;

	nbar = ZOOM_TO_BAR[fspect_panel.zoom];

	for (i = 0; i < nbar; i++) {
		bar = &fspect_panel.bars[i];
		bar_clear(bar);
	}

	button_clear(&(buttons[SPECT_PANEL][ZOOMIN_BTN]));
	button_clear(&(buttons[SPECT_PANEL][ZOOMOUT_BTN]));
}

static float avg(float *v, int size)
{
int		i;
float	val;

	val = 0;
	for(i = 0; i < size; i++){
		val += v[i];
	}

	return val / ((float) size);
}

static void fspect_panel_draw()
{
int		i;
int		height;
int		nbar;

	nbar = ZOOM_TO_BAR[fspect_panel.zoom];

	for (i = 0; i < nbar; i++) {
		height = avg(&(p.spectogram[i * (PLAYER_WINDOW_SIZE_CPX / nbar)]),
			PLAYER_WINDOW_SIZE_CPX / nbar);
		height = (SPECT_H * height) / 100;
//		printf("bar_stretch(%d, -1, %d)\n",i, height);
		bar_stretch(&fspect_panel.bars[i], -1, height);
	}

	button_draw(&(buttons[SPECT_PANEL][ZOOMIN_BTN]));
	button_draw(&(buttons[SPECT_PANEL][ZOOMOUT_BTN]));
}

int fspect_panel_zoomin()
{
	if (fspect_panel.zoom <= MAXZOOM) {
		fspect_panel_clear();
		fspect_panel.zoom++;
		fspect_panel_bars_init();
		fspect_panel_draw();
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
		fspect_panel_draw();
		return fspect_panel.zoom;
	}
	return -1;
}

static void title_blink()
{
int	col;

	col = texts[TITLE_PANEL][TITLE_TXT].col;
	texts[TITLE_PANEL][TITLE_TXT].col = texts[TITLE_PANEL][TITLE_TXT].bgcol;
	texts[TITLE_PANEL][TITLE_TXT].bgcol = col;
	text_draw(&texts[TITLE_PANEL][TITLE_TXT]);
}

static void title_reset()
{
	text_ctor(&texts[TITLE_PANEL][TITLE_TXT], "trackname", TITLE_X, TITLE_Y, TITLE_COL, TITLE_BGCOL, centre);
	text_draw(&texts[TITLE_PANEL][TITLE_TXT]);
}

static void title_slide(int speed)
{
int	i;
	for(i=0; i < speed; i++){
		if (speed < 0)
			text_slidel(&texts[TITLE_PANEL][TITLE_TXT]);
		else
			text_slider(&texts[TITLE_PANEL][TITLE_TXT]);
	}
}

int view_init()
{
int	i, j;

	set_color_depth(COL_D);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, WIN_W, WIN_H, 0, 0);
	clear_to_color(screen, makecol(255, 255, 255));

	for(i = 0; i < NPANEL; i++){
		for(j = 0; j < buttons_size[i]; j++){
			button_ctor(&(buttons[i][j]), buttons[i][j].path, buttons[i][j].x,
					buttons[i][j].y, buttons[i][j].w, buttons[i][j].h,
					buttons[i][j].bgcol, 0x00);
			button_draw(&(buttons[i][j]));
		}
		for(j = 0; j < texts_size[i]; j++){
			text_draw(&texts[i][j]);
		}
		for(j = 0; j < bars_size[i]; j++){
			bar_draw(&bars[i][j]);
		}
	}

	for(i = 0; i < 4096; i++){
		p.spectogram[i] = i % 101;
	}

	fspect_panel.zoom = 2;
	fspect_panel_bars_init();

	text_setstr(&texts[TITLE_PANEL][TITLE_TXT], "trackname");
	text_setstr(&texts[POS_PANEL][TIME_TXT], "00:00");

//	old_state = p.state;

	show_mouse(screen);

	return 0;
}

void view_update(pstate s)
{

	switch (s) {
	case STOP:
		// title is static
		title_reset();
		// stop button highlighted
		button_highlight(&buttons[CTRL_PANEL][STOP_BTN]);
		break;
	case PLAY:
		// title slide to the left
		title_slide(1);
		// update spectogram
		fspect_panel_draw();
// ONLY HERE ??
		// play button highlighted
		button_highlight(&buttons[CTRL_PANEL][PLAY_BTN]);
		break;
	case PAUSE:
		// title stopped blinking
		title_blink();
		// pause button highlighted
		button_highlight(&buttons[CTRL_PANEL][PAUSE_BTN]);
		break;
	case REWIND:
		// title slide faster toward right
		title_slide(2);
		// rewind button highlighted
		button_highlight(&buttons[CTRL_PANEL][RWND_BTN]);
		break;
	case FORWARD:
		// title slide faster towards left
		title_slide(-2);
		// forward button highlighted
		button_highlight(&buttons[CTRL_PANEL][FRWD_BTN]);
		break;
	default:
			break;
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
