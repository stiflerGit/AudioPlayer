/**
 * @file	View.c
 * @author	Stefano Fiori
 * @date	26 May 2018
 * @brief	Implementation of the View strictly linked to the Audio Player
 * *
 */
#include "View.h"
#include <stdio.h>
#include <allegro.h>
#include <assert.h>
#include "View_Config.h"
#include "../Model/Player.h"

#define handle_error(e) \
	do{ perror(e); exit(EXIT_FAILURE);} while(1);

#define		MAXZOOM		5
static const int ZOOM_TO_BAR[6] = { 210, 170, 140, 100, 70, 30};
	/**< Zoom to bar table. */

/**
 * @brief	Structure for frequency spectogram panel
 */
struct fspect_panel_t{
	Node 		bars[227];	/**< Bars of the spectogram. */
	unsigned char 	zoom;		/**< Actual zoom of the panel.
					It changes the no. bar shown. */
}fspect_panel;

static Player	old_p;	/**< Previous player state. */

/**
 * @brief	Initialize variables of each frequency spectrum bar
 *		that depends on zoom
 */
static int fspect_panel_bars_init()
{
int	bar_x;		/**< X bars coordinate. */
int	bar_col;	/**< Color of the bar. */
int	nbar;		/**< No. bars. */
int	i;		/**< Bars Array index. */

	nbar = ZOOM_TO_BAR[fspect_panel.zoom];

	bar_x = SPECT_X + 2;
	bar_col = RED;

	for (i = 0; i < nbar; i++) {
		fspect_panel.bars[i] = (Node )
			{ BAR, 
			bar_x, SPECT_Y + SPECT_H - 1, ((SPECT_W - 2) / nbar) - 2, 0,
			bar_col, SPECT_BGCOL, 0, NULL};
		bar_x += (SPECT_W - 2) / nbar;
		bar_col = RED;
	}

	g_draw(&(nodes[SPECT_PANEL][ZOOMIN_BTN]));
	g_draw(&(nodes[SPECT_PANEL][ZOOMOUT_BTN]));

	return 0;
}

/**
 * @brief	Compute the average value of a buffer.
 * @param[in]	v	buffer of float falues.
 * @param[in]	size	size of the buffer.
 * @ret			average float value.
 */
static float avg(float *v, unsigned int size)
{
int	i;
float	val;

	if(v == NULL)
		handle_error("NULL pointer");
	val = 0;
	for(i = 0; i < size; i++){
		val += v[i];
	}

	return val / ((float) size);
}

/**
 * @brief	Draw a spectogram view bar with respect to the player spectogram
 *
 * Draw a bar of the spectogram depending on the player spectogram values corres
 * -ponding to that bar. Indeed, each spectogram view bar represent more than a
 * player spectogram value, since view bars are lesser.
 *
 * @param[in]	i	No bar to draw.
 */
 
static void fspect_bar_update(unsigned int i)
{
int	height;	/**< New bar update. */
int	delta;	/**< Difference between old height and new height of the bar. */
int	col;	/**< Color with which to draw. */
int	nbar;	/**< No. bar actually showed in the spect.. Depends on zoom. */
Node	*n;	/**< Pointer to the right Spect. Bar. */
	
	nbar = ZOOM_TO_BAR[fspect_panel.zoom];
	assert(i<nbar);
	// the last bar can overflow the player spectogram.
	if (i < nbar){
		height = 
		avg(&(p.spectogram[i * (PLAYER_WINDOW_SIZE_CPX / nbar)]),
		PLAYER_WINDOW_SIZE_CPX / nbar);
	} else {
		height = 
		avg(&(p.spectogram[i * (PLAYER_WINDOW_SIZE_CPX / nbar)]),
		PLAYER_WINDOW_SIZE_CPX % nbar);
	}
	// since height is in the [0-100] range we can obtain easily the new
	// height by multiplying for Panel Height
	height = SPECT_H * height / 100;
	n = &fspect_panel.bars[i];
	// delta could be negative or positive.
	delta = n->h - height;
	// depending on delta sign, we have to clear or draw a rect.
	col = (delta < 0) ? n->fg : n->bg;
	scare_mouse();
	rectfill(screen, n->x, n->y, n->x + n->w, n->y + delta, col);
	unscare_mouse();
	// if the bar draw on the buttons re-draws them
	if(is_inside(&nodes[SPECT_PANEL][ZOOMIN_BTN], n->x, n->y))
		g_draw(&nodes[SPECT_PANEL][ZOOMIN_BTN]);
	if(is_inside(&nodes[SPECT_PANEL][ZOOMOUT_BTN], n->x, n->y))
		g_draw(&nodes[SPECT_PANEL][ZOOMOUT_BTN]);
	// update bar state
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

/*******************************************************************************
 *			TITLE PANEL
 ******************************************************************************/
/**
 * @brief	Draw the title in the title panel depending on player state.
 *
 * The title can be static or slide to the left or the right with different
 * speed. It can also blink. It behaviour depends on the player state. This
 * function update the title and draw it.
 */
static void title_draw()
{
char	slide[100];	/**< Buffer for title slide. */
int 	len;		/**< title length. */
int	speed;		/**< No. character to slide. */
Node	*n;		/**< Pointer to a graphic Obj. */
static int	blink_count = 0;
			/**< Counter for title blinking. */

	n = &nodes[TITLE_PANEL][TITLE_TXT];

	if(p.state != PAUSE)
		n->fg = WHITE;

	speed = 0;
	switch (p.state) {
	case STOP:
		// reset title
		strcpy(((text *)(n->dp))->str, p.trackname);
		strcat(((text *)(n->dp))->str, "       ");
		break;
	case PLAY:
		// title slide to the left
		speed = 1;
		break;
	case PAUSE:
		// title static blinking
		blink_count = (blink_count + 1) % 5;
		if(blink_count == 0)
			n->fg = (n->fg == BLACK) ? WHITE : BLACK;
		break;
	case REWIND:
		// title slide faster toward right
		speed = -2;
		break;
	case FORWARD:
		// title slide faster towards left
		speed = 2;
		break;
	default:
		break;
	}
	if (speed != 0) {
		len = strlen(((text *)(n->dp))->str);
		strcpy(slide, &((text *)(n->dp))->str[(len - speed) % len]);
		strncat(slide, ((text *)(n->dp))->str, (len - speed) % len);
		slide[len] = '\0';
		strcpy(((text *)(n->dp))->str, slide);
	}
	g_draw(n);
}

/*******************************************************************************
 *			CONTROL PANEL
 ******************************************************************************/
/**
 * @brief	Control Panel update
 *
 * Switch off the button corresponding to the older player state and than 
 * hilight the button corresponding to the new palyer state
 */
static void control_draw()
{
Node	*n;

	if(old_p.state != p.state){

		switch (old_p.state) {
		case STOP:
			n = (&nodes[CTRL_PANEL][STOP_BTN]);
			break;
		case PLAY:
			n = (&nodes[CTRL_PANEL][PLAY_BTN]);
			break;
		case PAUSE:
			n = (&nodes[CTRL_PANEL][PAUSE_BTN]);
			break;
		case REWIND:
			n = (&nodes[CTRL_PANEL][RWND_BTN]);
			break;
		case FORWARD:
			n = (&nodes[CTRL_PANEL][FRWD_BTN]);
			break;
		default:
			break;
		}
		n->fg = TSPRNT;
		g_draw(n);
		switch (p.state) {
		case STOP:
			n = (&nodes[CTRL_PANEL][STOP_BTN]);
			break;
		case PLAY:
			n = (&nodes[CTRL_PANEL][PLAY_BTN]);
			break;
		case PAUSE:
			n = (&nodes[CTRL_PANEL][PAUSE_BTN]);
			break;
		case REWIND:
			n = (&nodes[CTRL_PANEL][RWND_BTN]);
			break;
		case FORWARD:
			n = (&nodes[CTRL_PANEL][FRWD_BTN]);
			break;
		default:
			break;
		}
		n->fg = RED;
		g_draw(n);
	}

}

/*******************************************************************************
 *				VIEW
 ******************************************************************************/
/**
 * @brief Initialize the GUI
 *
 * It call first the allegro functions that initialiaze a Windows on the screen
 * Than draw all graphic objects defined statically in the configuration file.
 * The only things that need to be initialized dinamically are the spectogram
 * bars, but only for pratically reasons.
 * Take the first copy of Player p.
 */
int view_init()
{
int	i, j;	/**< Array indexes. */
Node	*n;	/**< Pointer to a graphic Obj. */

	set_color_depth(COL_D);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, WIN_W, WIN_H, 0, 0);
	clear_to_color(screen, 0);
	
	n = &nodes[TITLE_PANEL][TITLE_TXT];
	strcpy(((text *)n->dp)->str, p.trackname);
	strcat(((text *)n->dp)->str, "       ");

	for(i = 0; i < NPANEL; i++){
		for(j = 0; j < nodes_size[i]; j++){
			g_draw(&(nodes[i][j]));
		}
	}

	fspect_panel.zoom = 4;
	fspect_panel_bars_init();

	memcpy(&old_p, &p, sizeof(Player));

	// What if the player already started to play??
	nodes[CTRL_PANEL][STOP_BTN].fg = RED;
	g_draw(&nodes[CTRL_PANEL][STOP_BTN]);

	install_mouse();
	show_mouse(screen);

	return 0;
}

/**
 * @brief Looks at the actual Player outputs and represent its on the screen
 *
 * Compare the actual Player output and the last copy of player the View has,
 * updated the last time view_update has been called.
 * If something is changed it represents such a change and update its player 
 * copy.
 */
void view_update()
{
int	i, j;	/**< Array indexes for spectogram. */
int	nbv;	/**< No. bars of the View (Spectogram). */
int	spv;	/**< player Spectogram bar Per View bar. */
char	next;	/**< A boolean variable for jump to next spect bar update. */
int	pix;	/**< Pixel variable. */
Node	*n;	/**< Pointer to a graphic object. */

	// PLAYER STATE
	if(old_p.state != p.state){
		control_draw();
		old_p.state = p.state;
	}
	// Title is always moving
	title_draw();
	// PLAYER TIME
	if(old_p.time != p.time){
		n = &nodes[POS_PANEL][POS_BAR];
		// position set bar update
		pix = n->w * p.time / p.duration + n->x;
		n = &nodes[POS_PANEL][POS_SETB];
		g_stretch(n, pix, n->y, n->w, n->h);
		// time text update
		n = &nodes[POS_PANEL][POS_TIME];
		sprintf(((text *)(n->dp))->str, "%02d:%02d",
			((int)p.time) / 60, ((int)p.time) % 60);
		g_draw(n);
		old_p.time = p.time;
	}
	// PLAYER SPECTOGRAM
	/* DA RIVEDERE */
	nbv = ZOOM_TO_BAR[fspect_panel.zoom];
	spv = PLAYER_WINDOW_SIZE_CPX / nbv;
	for (i=0; i<nbv; i++){
		next = 0;
		for (j= i*spv; (j < (i+1)*spv) && (next == 0) ; j++){
			if(old_p.spectogram[j] != p.spectogram[j]){
				fspect_bar_update(i);
				memcpy(&old_p.spectogram[j], &p.spectogram[j], 
				sizeof(float)*(spv - (j % spv)));
				next = 1;
			}
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
