/**
 * @file view.h
 * @author Stefano Fiori (fiori.stefano@gmail.com)
 * @brief view thread interface
 * @version 0.1
 * @date 2019-03-17
 * 
 * //TODO: 
 */
#ifndef VIEW_H_
#define VIEW_H_

#include <pthread.h>

#include "ptask.h"

/**
 * @brief Initialize the GUI
 *
 * It initialize all the Graphic objects in the GUI that either needs to
 * be initialized or is more pratical to initialize its dinamically
 */
int view_init();

/**
 * @brief start the view thread
 * 
 * @param task_par    pointer to a struct definig task parameters(not mandatory)
 * @return pthread_t* pointer to the thread identificator
 */
pthread_t *view_start(task_par_t *tp);

/**
 * @brief notice the view thread to exit
 */
void view_exit();

#endif /* VIEW_H_ */
