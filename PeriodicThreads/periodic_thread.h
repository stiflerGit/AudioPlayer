#include <time.h>

struct task_par {
	int		arg;		// task argument
	long int	wcet;		// worst case execution time in ms
	int 		period;		// period in ms
	int 		deadline;	// relative deadline in ms
	int		priority;	// task priority in [0; 99]
	int		dmiss;		// no. of deadline misses
	struct timespec	at;		// next activation time
	struct timespec dl;		// absolute deadline
};

void set_period(struct task_par *tp);
int deadline_miss(struct task_par *tp);
void wait_for_period(struct task_par *tp);

