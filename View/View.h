#ifndef VIEW_VIEW_H_
#define VIEW_VIEW_H_
/**
 * @brief Initialize the GUI
 *
 * It initialize all the Graphic objects in the GUI that either needs it or is
 * more pratical to initialize its dinamically
 */
int view_init();

/**
 * @brief Looks at the actual Player outputs and represent its on the screen
 */
void view_update();

/**
 * @brief Destroy all objects that needs to be destroyed
 */
void view_exit();

#endif /* VIEW_VIEW_H_ */
