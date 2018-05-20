
/*
 * Controller.c
 *
 *  Created on: Apr 30, 2017
 *      Author: stefano
 */

#include <allegro.h>
#include <stdio.h>
#include "Controller.h"
#include "../Model/Player.h"
#include "../View/View_Config.h"
#include "../View/graphic.h"


#define	x_perc(n, ix)	((ix) / ((n)->x + (n)->w) * 100)
#define	y_perc(n, iy)	((iy) / ((n)->y + (n)->h) * 100)

extern pevent	evt;

static void control(Node* n, int x, int y);

void controller()
{
int pos, x, y;
int	i, j;
   
   	if(mouse_b & 1) {
		pos = mouse_pos;
		x = pos >> 16;
		y = pos & 0x0000ffff;
		//printf("mouse button\tx: %d\tx: %d\n", x, y);
		for(i = 0; i < NPANEL; i++){
			if(is_inside(&(nodes[i][0]), x, y)){
				for(j = 1; j < nodes_size[i]; j++){
					if(is_inside(&nodes[i][j], x, y)){
						control(&nodes[i][j], x, y);
						return;
					}
				}
			}
		}
		mouse_b = 0;
	}
}

static void control(Node* n, int x, int y)
{
int	perc;

	switch(n->evt){
	case JUMP_SIG:
		evt.val = p.duration * ((float) (x - n->x)) / (float) n->w;
		break;
	case FILTLOW_SIG:
	case FILTMED_SIG:
	case FILTMEDHIG_SIG:
	case FILTHIG_SIG:
		evt.val = -24 * ((float)(y - n->y)) / (float) n->h + 12;
		break;
	case VOL_SIG:
		evt.val = 100 * ((float) (n->y + n->h - y)) / (float) n->h;
		break;
	default:
		break;
	}
	evt.sig = n->evt;
	//printf("node founded, event: {.sig = %d, .val = %f}\n", evt.sig, evt.val);
}

