/**
 * @file	ptask.h
 * @author	Stefano Fiori
 * @date	28 May 2018
 * @brief	Provide a simple framework for Periodic Thread definitions
 *
 * The structure and the functions defined here serve as a framework for a peri-
 * odic thread definition.
 * Scheme of periodic thread:
 *
 * void *task(void *arg)
 * {
 * <local state variables>
 * struct task_par	*tp;
 *	// retrieves the pointer to task_par
 *	tp = (struct task_par *)arg;
 *	// retrieves the argument
 *	i = tp->arg;
 * 	set_period(tp);
 *	while (1) {
 *		<thread body>
 *		if (deadline_miss(tp)) <do action>;
 *		wait_for_period(tp);
 *	}
 * }
 */
#ifndef PTASK_H_
#define PTASK_H_

#include <time.h>

/**
 * @brief 	It provides all variable need for periodic tasks
 *
 * Such a structure must be initialized before calling the thread_create and 
 * passed as a thread argument. After initialized can be passed to the functions
 * defined in this library to obtain a periodic task behaviour.
 * NOTE: A structure for each thread is required.
 */
struct task_par {
	int		arg;		/**< Task argument. */
	long int	wcet;		/**< Worst case execution time in us. */
	int 		period;		/**< Period in ms. */
	int 		deadline;	/**< Relative deadline in ms. */
	int		priority;	/**< Task priority in [0; 99]. */
	int		dmiss;		/**< No. of deadline misses. */
	struct timespec	at;		/**< next activation time. */
	struct timespec dl;		/**< absolute deadline. */
};

/**
 * @brief	set the period of a thread.
 * @param[in]	tp	pointer to the task_par structure of the calling thread.
 */
void set_period(struct task_par *tp);

/**
 * @brief	check for a deadline miss.
 * @param[in]	tp	pointer to the task_par structure of the calling thread.
 * @ret		returns 1 in case of deadline miss, otherwise returns 0.
 */
int deadline_miss(struct task_par *tp);

/**
 * @brief	suspend the calling thread until the next activation.
 * @param[in]	tp	pointer to the task_par structure of the calling thread.
 */
void wait_for_period(struct task_par *tp);

#endif
