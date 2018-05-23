#ifndef PTASK_H_
#define PTASK_H_

#include <time.h>

struct task_par {
	int		arg;		/**< Task argument.			*/
	long int	wcet;		/**< Worst case execution time in us.	*/
	int 		period;		/**< Period in ms.			*/
	int 		deadline;	/**< Relative deadline in ms.		*/
	int		priority;	/**< Task priority in [0; 99].		*/
	int		dmiss;		/**< No. of deadline misses.		*/
	struct timespec	at;		/**< next activation time.		*/
	struct timespec dl;		/**< absolute deadline.			*/
};

void set_period(struct task_par *tp);
int deadline_miss(struct task_par *tp);
void wait_for_period(struct task_par *tp);

#endif
