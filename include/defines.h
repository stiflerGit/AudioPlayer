/**
 * @file defines.h
 * @author Stefano Fiori (fioristefano.90@gmail.com)
 * @brief commons marcos 
 * @version 0.1
 * @date 2019-03-17
 * 
 * 
 */
#ifndef DEFINES_H
#define DEFINES_H

#include <stdlib.h>

#undef NULL
#define NULL ((void *)0) /**< NULL definition. */
#undef TRUE
#define TRUE -1
#undef FALSE
#define FALSE 0

/**
 * @brief print an error message and the errno, than exit
 * 
 */
#define handle_error(s)                \
    do                                 \
    {                                  \
        perror(s), exit(EXIT_FAILURE); \
    } while (0)

/**
 * @brief concatenate statical strings
 * 
 */
#define STRCAT(str1, str2) (str1 str2)

#endif