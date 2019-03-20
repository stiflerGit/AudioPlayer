/**
 * @file controller.c
 * @author Stefano Fiori (fioristefano.90@gmail.com)
 * @brief manages user input clicks 
 * @version 0.1
 * @date 2019-03-17
 * 
 * Implementation of the Controller thread. The main function, i.e.
 * the controller routine executed periodically is the controller_run.
 * 
 */
#include "controller.h"

#include <stdio.h>

#include <allegro.h>

#include "defines.h"

#include "player/player.h"
#include "view/graphic.h"
#include "view/view.h"
#include "view/view_config.h"

/**
 * @brief compute percentage of the click x coordinate w.r.t the width
 */
#define x_perc(n, ix) ((ix) / ((n)->x + (n)->w) * 100)
/**
 * @brief compute percentage of the click y coordinate w.r.t the height
 */
#define y_perc(n, iy) ((iy) / ((n)->y + (n)->h) * 100)

static task_par_t tp = {
	arg : 0,
	period : 50,
	deadline : 50,
	priority : 99,
	dmiss : 0
}; /**< default controller task parameters. */

static pthread_t tid; /**< thread identifier of the controller. */

static char controller_run_exit = 0; /**< variable to notice the thread that has to exit. */

static void *controller_run(void *arg);

/**
 * @brief build the event correctly w.r.t the coordinates and dispatch it 
 * to the player
 * 
 * @param n the node which has been clicked
 * @param x x coordinate of the click
 * @param y y coordinate of the click
 */
static void control(Node *n, int x, int y);

/**
 * @brief manage the event raised by a click
 * 
 * @param n pointer to the node on which the user has clicked
 * @param x coordinate of the click
 * @param y coordinate of the click
 */
static void control(Node *n, int x, int y)
{
	player_event_t evt;
	switch (n->evt)
	{
	case JUMP_SIG:
		evt.val = p.duration * ((float)(x - n->x)) / (float)n->w;
		break;
	case FILTLOW_SIG:
	case FILTMED_SIG:
	case FILTMEDHIG_SIG:
	case FILTHIG_SIG:
		evt.val = -(MAX_GAIN * 2) * ((float)(y - n->y)) / (float)n->h + MAX_GAIN;
		break;
	case VOL_SIG:
		evt.val = 100 * ((float)(n->y + n->h - y)) / (float)n->h;
		break;
	default:
		break;
	}
	evt.sig = n->evt;
	player_dispatch(evt);
}

/**
 * @brief function passed to allegro for manage the click
 * on the close window button
 * 
 */
static void controller_close_button_handler()
{
	controller_exit();
	view_exit();
	player_exit();
}

/**
 * @brief initialize the controller
 * 
 */
void controller_init()
{
	set_close_button_callback(controller_close_button_handler);
}

/**
 * @brief start the controller thread
 * 
 * @param task_par thread parameter with which start the controller thread
 * @return pthread_t* pointer to the controller thread identificator
 */
pthread_t *controller_start(task_par_t *task_par)
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
	pthread_create(&tid, &attr, controller_run, &tp);

	return &tid;
}

/**
 * @brief controller thread routine
 * 
 * Periodically the controller check for user clicks, and, when it
 * happens, find the graphic object clicked and the event that it should
 * raise. Finally the correct event is dispatched to the player 
 * 
 * @param arg pointer to the argument passed to the routine(actually task 
 * paramenters)
 * @return void* pointer to the returned variable of the routine
 */
static void *controller_run(void *arg)
{
	int pos, x, y; /**< coordinates of user click. */
	int i, j;	  /**<array indexes. */
	char found;
	/**< bool value, true when the clicked graphic object has been found. */

	set_period(&tp);

	while (1)
	{
		if (mouse_needs_poll())
			poll_mouse();
		//check for user clicks
		if (mouse_b & 1)
		{
			pos = mouse_pos;
			x = pos >> 16;
			y = pos & 0x0000ffff;
			found = FALSE;
			for (i = 0; i < NPANEL && !found; i++)
			{
				if (is_inside(&(nodes[i][0]), x, y))
				{
					for (j = 1; j < nodes_size[i] && !found; j++)
					{
						if (is_inside(&nodes[i][j], x, y))
						{
							control(&nodes[i][j], x, y);
							found = TRUE;
						}
					}
				}
			}
			mouse_b = 0;
		}

		if (controller_run_exit)
			pthread_exit(NULL);

		if (deadline_miss(&tp))
		{
			printf("VIEW MISS");
		}
		wait_for_period(&tp);
	}
	return NULL;
}

/**
 * @brief notice the controller thread to exit
 * 
 */
void controller_exit()
{
	controller_run_exit = 1;
}