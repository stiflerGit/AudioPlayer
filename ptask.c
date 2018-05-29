/**
 * @file	ptask.c
 * @author	Stefano Fiori
 * @date	28 May 2018
 * @brief	
 */
#include "ptask.h"

/**
 * @brief	Copies a source time variable ts in a destination variable.
 * @param[out]	td	pointer to the destination time variable
 * @param[in]	ts	source time variable to copy
 */
static void time_copy(struct timespec *td, struct timespec ts)
{
	td->tv_sec = ts.tv_sec;
	td->tv_nsec = ts.tv_nsec;
}

/**
 * @brief	Adds a milliseconds value to a time variable.
 * @param[out]	t	pointer to the time variable.
 * @param[in]	ms	milliseconds value
 */
static void time_add_ms(struct timespec *t, int ms)
{
	t->tv_sec += ms/1000;
	t->tv_nsec += (ms%1000)*1000000;

	if (t->tv_nsec > 1000000000) {
		t->tv_nsec -= 1000000000;
		t->tv_sec += 1;
	}
}

/**
 * @brief	This function compares two time variables 
 * @param[in]	t1	a time variable.
 * @param[in]	t2	another time variable.
 * @return	0 if they are equal, 1 if t1 > t2, ‐1 if t1 < t2.
 */
static int time_cmp(struct timespec t1, struct timespec t2)
{
	if (t1.tv_sec > t2.tv_sec) return 1;
	if (t1.tv_sec < t2.tv_sec) return -1;
	if (t1.tv_nsec > t2.tv_nsec) return 1;
	if (t1.tv_nsec < t2.tv_nsec) return -1;
	return 0;
}

/**
 * @brief	Set the period for a periodic task.
 *
 * Reads the current time and computes the next activation time and the absolute 
 * deadline of the task. NOTE: the timer is not set to interrupt.
 */
void set_period(struct task_par *tp)
{
struct timespec t;
	
	clock_gettime(CLOCK_MONOTONIC, &t);
	time_copy(&(tp->at), t);
	time_copy(&(tp->dl), t);
	time_add_ms(&(tp->at), tp->period);
	time_add_ms(&(tp->dl), tp->deadline);
}

/**
 * @brief	When called it controls if the task have a miss
 *
 * If the thread is still in execution when re‐activated, it increments the 
 * value of dmiss.
 *
 * @param[in]	tp	task_par structure of the calling thread.
 * @return 	If the thread is still in execution when re‐activated, it 
 *		returns 1, otherwise returns 0.
 */
int deadline_miss(struct task_par *tp)
{
struct timespec now;
	
	clock_gettime(CLOCK_MONOTONIC, &now);
	if (time_cmp(now, tp->dl) > 0) {
		tp->dmiss++;
		return 1;
	}
	return 0;
}

/**
 * @brief 	Suspends the calling thread until the next activation 
 *
 * Suspends the calling thread until the next activation and, when awaken, 
 * updates activation time and deadline. 
 * NOTE: Even though the thread calls time_add_ms() after the wake‐up time, the 
 * computation is correct.
 *
 * @param[in]	tp	pointer to the task_param stucture of the calling thread
 */
void wait_for_period(struct task_par *tp)
{
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(tp->at), NULL);
	time_add_ms(&(tp->at), tp->period);
	time_add_ms(&(tp->dl), tp->period);
}
