/**
 * @file controller.h
 * @author Stefano Fiori (fioristefano.90@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-03-17
 * 
 * This file defines the functions for initialize, start, and exit
 * the controller thread, that is supposed to control and manages
 * the user inputs. Possible user input types are only clicks in this
 * case. It need player and view threads to be already started. 
 * 
 */
#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <pthread.h>
#include "ptask.h"

/**
 * @brief initialize the controller
 * 
 */
void controller_init();

/**
 * @brief start the controller thread
 * 
 * @param task_par thread parameter with which start the controller thread
 * @return pthread_t* pointer to the controller thread identificator
 */
pthread_t *controller_start(task_par_t *task_par);

/**
 * @brief notice the controller thread to exit
 * 
 */
void controller_exit();

#endif /* CONTROLLER_H_ */
