/**
 * @file controller.h
 * @author Stefano Fiori (fioristefano.90@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-03-17
 * 
 * //TODO: more complete doc on file
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
