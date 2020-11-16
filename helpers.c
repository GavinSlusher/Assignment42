/******************************************************************************
* File:             helpers.c
*
* Author:           Gavin Slusher  
* Created:          09/28/20 
* Description:     
*****************************************************************************/
#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

int numLines = 0;
int linesCompleted = 0;

/******************
*  Input Buffer  *
******************/
char inputBuffer[MAX_CHARS * MAX_LINES];
int countInput = 0;
// Index where the input thread will put the next item
int input_prod_idx = 0;
// Index where the parse thread will pick up the next item
int input_cons_idx = 0;

pthread_mutex_t mutex_input = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t input_full = PTHREAD_COND_INITIALIZER;

/******************
*  Parse Buffer  *
******************/
/* char parseBuffer[MAX_CHARS * MAX_LINES]; */
/* int countParse = 0; */
/* // Index where the parse thread will put the next item */
/* int parse_prod_idx = 0; */
/* // Index where the output thread will pick up the next item */
/* int parse_cons_idx = 0; */
/* pthread_mutex_t mutex_parse = PTHREAD_MUTEX_INITIALIZER; */
/* pthread_cond_t parse_full = PTHREAD_COND_INITIALIZER; */

/*******************
*  Output Buffer  *
*******************/
/* char outputBuffer[MAX_CHARS * MAX_LINES]; */
/* int countOutput = 0; */
/* int output_prod_idx = 0; */
/* int output_cons_idx = 0; */
bool newOutput = false;
/* pthread_mutex_t mutex_output = PTHREAD_MUTEX_INITIALIZER; */
/* pthread_cond_t output_full = PTHREAD_COND_INITIALIZER; */

/* TODO */
void putString(char* stringInput){
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_input);
    /* printf("Inside putstring\n"); */

    // Append the item in the buffer
    strcat(inputBuffer, stringInput);

    // Track the number of chars
    countInput += strlen(stringInput);

    // Update the production index
    input_prod_idx = countInput; // TODO Needed?

    // Signal to the consumer that the buffer is no longer empty
    /* printf("newOutput is true\n"); */
    pthread_cond_signal(&input_full);

    // Unlock the mutex
    pthread_mutex_unlock(&mutex_input);
}

/**
    TODO

	param: char* -> A string buffer
	param: int ->The max size of the string desired.
    post: buffer holds the user input 
 */
void *getString(void* stringInput){

    while(1){
        fgets(stringInput, MAX_CHARS, stdin);
        
        putString(stringInput);

        if (strcmp(stringInput, "STOP\n") == 0){
            /* printf("getString return\n"); */
            fflush(stdout);
            return NULL;
        } 
    }
            
    return NULL;
}

/* TODO */
void getOutput(char* stringInput){

    pthread_mutex_lock(&mutex_input); //TODO change as we add buffers
    
    while (countInput == 0){ //TODO change as we add buffers
        pthread_cond_wait(&input_full, &mutex_input);//TODO change as we add buffers
    }

    strncpy(stringInput, inputBuffer, input_prod_idx);
  
    pthread_mutex_unlock(&mutex_input);
}
/**

	param: const char* -> TODO
	post:  the string passed in is replaced with new one, with expanded '$$'
 */
void displayHelper(char* stringInput, int charSkip){
    while (((strlen(stringInput) - charSkip) - input_cons_idx) >=  80){ // TODO change as we add buffers

        int end = input_cons_idx + 80; // TODO change as we add buffers

        for (int i = input_cons_idx; i < end; ++i) {
            printf("%c", stringInput[i]);
            fflush(stdout);
        }
        input_cons_idx = end;
        printf("\n");
        fflush(stdout);
    }
}

/**

	param: const char* -> TODO
	post:  the string passed in is replaced with new one, with expanded '$$'
 */
void* displayOutput(){
    char stringInput[MAX_CHARS * MAX_LINES] = {0};
    while(1){

        getOutput(stringInput);

        if (strstr(stringInput, "STOP\n") != NULL){
            displayHelper(stringInput, 5);

            return NULL;
        }
        displayHelper(stringInput, 0);
    }
    return NULL;
}
