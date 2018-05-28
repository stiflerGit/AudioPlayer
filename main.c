#include <stdlib.h>
#include <stdio.h>
#include <allegro.h>
#include <pthread.h>
#include "ptask.c"
#include "Model/Player.h"
#include "View/View.h"
#include "Controller/Controller.h"

void init()
{
	allegro_init();
	//install_keyboard();
	install_mouse();
	//enable_hardware_cursor();
	install_sound(DIGI_AUTODETECT, 0, 0);
}

/*
int main(int argc, char *argv[])
{
pevent	evt;

	if(argc != 2){
		printf("usage: main <track>\n");
		exit(EXIT_FAILURE);
	}

	init();
	pinit(argv[1]);
	view_init();
	pprint();

	evt.sig = EMPTY_SIG;
	while(1){
		controller();
		if(evt.sig != 0)
			printf("event: %d\n", evt.sig);
		pdispatch(evt);
		view_update();
		sleep(0.5);
	}
}
*/

struct task_par 	tp[3];
pthread_attr_t 		att[3];
pthread_t		tid[3];

void *model(void *arg)
{
struct task_par	*tp;

	tp = (struct task_par *)arg;

	set_period(tp);

	while(1){
		pdispatch();

		if(deadline_miss(tp))
			printf("MODEL MISS\n");
		wait_for_period(tp);
	}
}

void *view(void *arg)
{
struct task_par	*tp;

	tp = (struct task_par *)arg;

	set_period(tp);

	while(1){
		view_update();

		if(deadline_miss(tp))
			printf("VIEW MISS\n");
		wait_for_period(tp);
	}
}

void *ctrl(void *arg)
{
struct task_par	*tp;

	tp = (struct task_par *)arg;

	set_period(tp);

	while(1){
		controller();

		if(deadline_miss(tp))
			printf("CONTROLLER MISS\n");
		wait_for_period(tp);
	}
}



int main(int argc, char *argv[])
{
int			i;
struct sched_param	mypar;

	if(argc != 2){
		printf("usage: main <track>\n");
		exit(EXIT_FAILURE);
	}

	init();
	pinit(argv[1]);
	view_init();
	//pprint();
	evt.sig = EMPTY_SIG;

	tp[0].arg = 0;
	tp[0].period = 50;
	tp[0].deadline = 50;
	tp[0].priority = 20;
	tp[0].dmiss = 0;
	pthread_attr_init(&att[0]);
	pthread_attr_setinheritsched(&att[0] , PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setschedpolicy(&att[0] , SCHED_FIFO );
	mypar.sched_priority = tp[0].priority;
	pthread_attr_setschedparam(&att[0], &mypar);
	pthread_create(&tid[0], &att[0], model, &tp[0]);

	tp[1].arg = 0;
	tp[1].period = 80;
	tp[1].deadline = 80;
	tp[1].priority = 20;
	tp[1].dmiss = 0;
	pthread_attr_init(&att[1]);
	pthread_attr_setinheritsched(&att[1] , PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setschedpolicy(&att[1] , SCHED_FIFO );
	mypar.sched_priority = tp[1].priority;
	pthread_attr_setschedparam(&att[1], &mypar);
	pthread_create(&tid[1], &att[1], view, &tp[1]);

	tp[2].arg = 0;
	tp[2].period = 20;
	tp[2].deadline = 20;
	tp[2].priority = 99;
	tp[2].dmiss = 0;
	pthread_attr_init(&att[2]);
	pthread_attr_setinheritsched(&att[2] , PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setschedpolicy(&att[2] , SCHED_FIFO );
	mypar.sched_priority = tp[2].priority;
	pthread_attr_setschedparam(&att[2], &mypar);
	pthread_create(&tid[2], &att[2], ctrl, &tp[2]);

	for (i=0; i<3; i++) {
		pthread_join(tid[i], NULL);
	}

	//while(1){}
}


