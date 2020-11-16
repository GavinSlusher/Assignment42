/******************************************************************************
* File:             helpers.c
*
* Author:           Gavin Slusher  
* Created:          09/28/20 
* Description:     
*****************************************************************************/
#include <pthread.h>
#ifndef HELPERS_H
#define HELPERS_H 

#ifndef MAX_CHARS
#define MAX_CHARS 1000
#define MAX_LINES 49
#endif /* ifndef MAX_CHARS and MAX_LINES */


/********************
*  Helper Methods  *
********************/
void* getString(void* stringInput);
void* replaceSeparator();
void* replacePlus();
void* displayOutput();

#endif /* ifndef HELPERS_H */
