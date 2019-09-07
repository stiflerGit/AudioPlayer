/**
 * @file	View.c
 * @author	Stefano Fiori
 * @date	26 May 2018
 * @brief	Implementation of the View strictly linked to the Audio Player
 * *
 */
#include "view/view.h"

#include <stdio.h>

#include <assert.h>

#include <allegro.h>

#include "defines.h"
#include "player/player.h"
#include "view/view_config.h"

#define MAXZOOM 5

static pthread_t tid;

static task_par_t tp = {
	arg : 0,
	period : 80,
	deadline : 80,
	priority : 20,
	dmiss : 0,
};
static char _view_exit = 0;
static pthread_mutex_t _view_exit_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *view_run(void *arg);

static const int ZOOM_TO_BAR[6] = {210, 170, 140, 100, 70, 30};
/**< Zoom to bar table. */

/**
 * @brief	Structure for frequency spectogram panel
 */
struct fspect_panel_t
{
	Node bars[227];		/**< Bars of the spectogram. */
	unsigned char zoom; /**< Actual zoom of the panel.
							 It changes the no. bar shown. */
	unsigned int id;
};

struct fspect_panel_t filt_spect_panel; /**< filtered frequency spectrum 
											 panel */
struct fspect_panel_t orig_spect_panel; /**< original(not equalized) frequency
	 										 spectrum panel */

static Player_t old_p, actual_p; /**< Previous player state. */

/*******************************************************************************
 *				TIME DATA PANEL
 ******************************************************************************/

/**
 * @brief refresh the time data panel
 * 
 */
static void timedata_panel_update()
{
	Node *n;
	BITMAP *buf;
	int x, y;
	static int old_y = 0;

	n = &nodes[TIME_PANEL][0];
	buf = create_bitmap(n->w / 2 - n->w / 30, n->h - 2);

	x = n->x + n->w / 2;
	// depend on actual_p.time data
	y = (n->y + n->h / 2) -
		actual_p.time_data * (n->h / 2) / ((1 << (actual_p.bits - 1)) - 1);

	scare_mouse();
	blit(screen, buf, n->x + n->w / 30, n->y + 1, 0, 0, buf->w, buf->h);
	blit(buf, screen, 0, 0, n->x + 1, n->y + 1, buf->w, buf->h);
	rectfill(screen, x + 1 - n->w / 30, n->y + 1, x, n->y + n->h - 1, BLACK);
	line(screen, x + 1 - n->w / 30, old_y, x, y, RED);
	unscare_mouse();
	old_y = y;
}

/*******************************************************************************
 *			FREQUENCY SPECTRUM PANEL
 ******************************************************************************/
/**
 * @brief	Initialize variables of each frequency spectrum bar
 *		that depends on zoom
 */
static int fspect_panel_init(struct fspect_panel_t *panel)
{
	int bar_x;   /**< X bars coordinate. */
	int bar_col; /**< Color of the bar. */
	int nbar;	/**< No. bars. */
	int i;		 /**< Bars Array index. */
	Node *frame;

	nbar = ZOOM_TO_BAR[panel->zoom];
	// first node in a panel is the frame.
	frame = &nodes[panel->id][0];
	bar_x = frame->x + 1;
	bar_col = RED;

	for (i = 0; i < nbar; i++)
	{
		panel->bars[i] = (Node){BAR,
								.x = bar_x,
								.y = frame->y + frame->h - 1,
								.w = ((frame->w - 2) / nbar) - 2,
								.h = 0,
								.fg = bar_col, .bg = BLACK,
								.evt = 0, .dp = NULL};
		bar_x += (frame->w - 2) / nbar;
		bar_col = RED;
	}

	g_draw(&(nodes[panel->id][ZOOMIN_BTN]));
	g_draw(&(nodes[panel->id][ZOOMOUT_BTN]));

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
	int i;
	float val;

	val = 0;
	for (i = 0; i < size; i++)
	{
		val += v[i];
	}

	return val / ((float)size);
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

static void fspect_bar_update(struct fspect_panel_t *panel, unsigned int i,
							  float spect[])
{
	int height; /**< New bar update. */
	int delta;  /**< Difference between old height and new height of the bar. */
	int col;	/**< Color with which to draw. */
	int nbar;   /**< No. bar actually showed in the spect.. Depends on zoom. */
	Node *n;	/**< Pointer to the right Spect. Bar. */

	nbar = ZOOM_TO_BAR[panel->zoom];
	assert(i < nbar);
	// the last bar can overflow the player spectogram.
	if (i < nbar)
	{
		height =
			avg(&(spect[i * (PLAYER_WINDOW_SIZE_CPX / nbar)]),
				PLAYER_WINDOW_SIZE_CPX / nbar);
	}
	else
	{
		height =
			avg(&(spect[i * (PLAYER_WINDOW_SIZE_CPX / nbar)]),
				PLAYER_WINDOW_SIZE_CPX % nbar);
	}
	// since height is in the [0-100] range we can obtain easily the new
	// height by multiplying for Panel Height
	n = &nodes[panel->id][0];
	height = n->h * height / 100;
	n = &panel->bars[i];
	// delta could be negative or positive.
	delta = n->h - height;
	// depending on delta sign, we have to clear or draw a rect.
	col = (delta < 0) ? n->fg : n->bg;
	scare_mouse();
	rectfill(screen, n->x, n->y, n->x + n->w, n->y + delta, col);
	unscare_mouse();
	// if the bar draw on the buttons re-draws them
	if (is_inside(&nodes[panel->id][ZOOMIN_BTN], n->x, n->y))
		g_draw(&nodes[panel->id][ZOOMIN_BTN]);
	if (is_inside(&nodes[panel->id][ZOOMOUT_BTN], n->x, n->y))
		g_draw(&nodes[panel->id][ZOOMOUT_BTN]);
	// update bar state
	n->y += delta;
	n->h = height;
}

int fspect_panel_zoomin(struct fspect_panel_t *panel)
{
	if (panel->zoom <= MAXZOOM)
	{
		panel->zoom++;
		fspect_panel_init(panel);
		//fspect_panel_draw();
		return panel->zoom;
	}
	return -1;
}

int fspect_panel_zoomout(struct fspect_panel_t *panel)
{
	if (panel->zoom > 0)
	{
		panel->zoom--;
		fspect_panel_init(panel);
		//fspect_panel_draw();
		return panel->zoom;
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
	char slide[100];			/**< Buffer for title slide. */
	int len;					/**< title length. */
	int speed;					/**< No. character to slide. */
	Node *n;					/**< Pointer to a graphic Obj. */
	static int blink_count = 0; /**< Counter for title blinking. */

	n = &nodes[TITLE_PANEL][TITLEP_TXT];

	if (actual_p.state != PAUSE)
		n->fg = WHITE;

	speed = 0;
	switch (actual_p.state)
	{
	case STOP:
		// reset title
		strcpy(((text *)(n->dp))->str, actual_p.trackname);
		strcat(((text *)(n->dp))->str, "       ");
		break;
	case PLAY:
		// title slide to the left
		speed = 1;
		break;
	case PAUSE:
		// title static blinking
		blink_count = (blink_count + 1) % 5;
		if (blink_count == 0)
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
	if (speed != 0)
	{
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
	Node *n;

	if (old_p.state != actual_p.state)
	{

		switch (old_p.state)
		{
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
		switch (actual_p.state)
		{
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
 * @brief Initialize the GUI.
 * 
 * This needs to be called before the start. Anyway, a view needs the player to
 * render, so it is highly reccomended to init the player before init the view
 *
 */
void view_init()
{
	int i, j; /**< Array indexes. */
	Node *n;  /**< Pointer to a graphic Obj. */
	// draw the window and clear with the background color
	set_color_depth(COL_D);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, WIN_W, WIN_H, 0, 0);
	clear_to_color(screen, 0);
	// draw the title
	n = &nodes[TITLE_PANEL][TITLEP_TXT];
	strcpy(((text *)n->dp)->str, actual_p.trackname);
	strcat(((text *)n->dp)->str, "       ");
	// draw each node
	for (i = 0; i < NPANEL; i++)
	{
		for (j = 0; j < nodes_size[i]; j++)
		{
			g_draw(&(nodes[i][j]));
		}
	}
	// init the fspect panel
	filt_spect_panel.zoom = 4;
	filt_spect_panel.id = FILT_SP_PANEL;
	fspect_panel_init(&filt_spect_panel);
	orig_spect_panel.zoom = 4;
	orig_spect_panel.id = ORIG_SP_PANEL;
	fspect_panel_init(&orig_spect_panel);

	player_get_player(&old_p);

	nodes[CTRL_PANEL][STOP_BTN].fg = RED;
	g_draw(&nodes[CTRL_PANEL][STOP_BTN]);

	install_mouse();
	show_mouse(screen);
}

/**
 * @brief Looks at the actual Player outputs and represent its on the screen
 * 
 */
static void view_run_body()
{
	int i, j;  /**< Array indexes for spectogram. */
	int nbv;   /**< No. bars of the View (Spectogram). */
	int spv;   /**< player Spectogram bar Per View bar. */
	char next; /**< A boolean variable for jump to next spect bar update. */
	int pix;   /**< Pixel variable. */
	Node *n;   /**< Pointer to a graphic object. */

	// PLAYER STATE
	if (old_p.state != actual_p.state)
	{
		control_draw();
		old_p.state = actual_p.state;
	}
	// Title is always moving
	title_draw();
	// PLAYER TIME
	if (old_p.time != actual_p.time)
	{
		n = &nodes[POS_PANEL][POSP_BAR];
		// position set bar update
		pix = n->w * actual_p.time / actual_p.duration + n->x;
		n = &nodes[POS_PANEL][POSP_SETB];
		// HERE
		g_stretch(n, pix, n->y, n->w, n->h);
		// time text update
		n = &nodes[POS_PANEL][POSP_TIME];
		sprintf(((text *)(n->dp))->str, "%02d:%02d",
				((int)actual_p.time) / 60, ((int)actual_p.time) % 60);
		g_draw(n);
		old_p.time = actual_p.time;
	}
	// PLAYER TIMEDATA
	if (actual_p.state != STOP && actual_p.state != PAUSE)
	{
		timedata_panel_update();
	}
	// PLAYER SPECTOGRAM
	nbv = ZOOM_TO_BAR[filt_spect_panel.zoom];
	spv = PLAYER_WINDOW_SIZE_CPX / nbv;
	for (i = 0; i < nbv; i++)
	{
		next = 0;
		for (j = i * spv; (j < (i + 1) * spv) && (next == 0); j++)
		{
			if (old_p.filt_spect[j] != actual_p.filt_spect[j])
			{
				fspect_bar_update(&filt_spect_panel, i,
								  actual_p.filt_spect);
				memcpy(&old_p.filt_spect[j], &actual_p.filt_spect[j],
					   sizeof(float) * (spv - (j % spv)));
				next = 1;
			}
		}
	}
	nbv = ZOOM_TO_BAR[orig_spect_panel.zoom];
	spv = PLAYER_WINDOW_SIZE_CPX / nbv;
	for (i = 0; i < nbv; i++)
	{
		next = 0;
		for (j = i * spv; (j < (i + 1) * spv) && (next == 0); j++)
		{
			if (old_p.orig_spect[j] != actual_p.orig_spect[j])
			{
				fspect_bar_update(&orig_spect_panel, i,
								  actual_p.orig_spect);
				memcpy(&old_p.orig_spect[j], &actual_p.orig_spect[j],
					   sizeof(float) * (spv - (j % spv)));
				next = 1;
			}
		}
	}
	// PLAYER VOLUME
	if (old_p.volume != actual_p.volume)
	{
		// VOLUME SET BAR
		n = &nodes[EQULZ_PANEL][VOL_BAR];
		pix = -((float)(n->h * actual_p.volume) / (float)100) + n->y + n->h;
		n = &nodes[EQULZ_PANEL][VOL_SBAR];
		g_stretch(n, n->x, pix, n->w, n->h);
		// VOLUME VAL LABEL
		n = &nodes[EQULZ_PANEL][VOL_VAL_LBL];
		sprintf(((text *)(n->dp))->str, "%3d", actual_p.volume);
		g_draw(n);

		old_p.volume = actual_p.volume;
	}
	// PLAYER EQUALIZ
	for (int i = 0; i < PLAYER_NFILT; i++)
	{
		if (old_p.eq_gain[i] != actual_p.eq_gain[i])
		{
			// EQ SET BAR
			n = &nodes[EQULZ_PANEL][LFRQ_BAR + i];
			pix = -(n->h * actual_p.eq_gain[i]) / (MAX_GAIN * 2) + n->y + n->h / 2;
			n = &nodes[EQULZ_PANEL][LFRQ_SBAR + i];
			g_stretch(n, n->x, pix, n->w, n->h);
			//EQ GAIN LABEL
			n = &nodes[EQULZ_PANEL][LFRQ_GAIN_LBL + i];
			sprintf(((text *)(n->dp))->str, "%3d dB",
					(int)actual_p.eq_gain[i]);
			g_draw(n);

			old_p.eq_gain[i] = actual_p.eq_gain[i];
		}
	}
}

/**
 * @brief destroy all additional allocated memory from the view
 * 
 */
static void view_xtor()
{
	int i, j; /**< Array indexes. */
	for (i = 0; i < NPANEL; i++)
	{
		for (j = 0; j < nodes_size[i]; j++)
		{
			g_destroy(&(nodes[i][j]));
		}
	}

	pthread_mutex_destroy(&_view_exit_mutex);
}

/**
 * @brief start the view thread
 * 
 * @param[in] task_par thread parameter with which start the view thread
 * @return pthread_t* pointer to the view thread identificator
 */
pthread_t *view_start(task_par_t *task_par)
{
	struct sched_param mypar;
	pthread_attr_t attr;

	if (task_par != NULL)
		tp = *task_par;

	pthread_attr_init(&attr);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	mypar.sched_priority = tp.priority;
	pthread_attr_setschedparam(&attr, &mypar);
	pthread_create(&tid, &attr, view_run, &tp);

	return &tid;
}

/**
 * @brief view thread routine
 * 
 * @param[in] arg  argument passed to the routine(actually is not used)
 * @return void* pointer to the variable returned by the thread(actually nothing)
 */
static void *view_run(void *arg)
{
	char local_view_exit;
	set_period(&tp);

	while (1)
	{
		// manage exit
		pthread_mutex_lock(&_view_exit_mutex);
		local_view_exit = _view_exit;
		pthread_mutex_unlock(&_view_exit_mutex);
		if (local_view_exit == 1)
		{
			view_xtor();
			pthread_exit(NULL);
		}

		player_get_player(&actual_p);
		view_run_body();

		if (deadline_miss(&tp))
		{
			printf("VIEW MISS");
		}
		wait_for_period(&tp);
	}
	return NULL;
}

/**
 * @brief notice the view thread that has to exit
 * 
 */
void view_exit()
{
	pthread_mutex_lock(&_view_exit_mutex);
	_view_exit = 1;
	pthread_mutex_unlock(&_view_exit_mutex);
}