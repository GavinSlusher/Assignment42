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
char inputBuffer[MAX_CHARS * MAX_LINES] = {0};
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
char parseBuffer[MAX_CHARS * MAX_LINES] = {0};
int countParse = 0;
// Index where the parse thread will put the next item
int parse_prod_idx = 0;
// Index where the output thread will pick up the next item
int parse_cons_idx = 0;
pthread_mutex_t mutex_parse = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t parse_full = PTHREAD_COND_INITIALIZER;

/*******************
*  Output Buffer  *
*******************/
char outputBuffer[MAX_CHARS * MAX_LINES] = {0};
int countOutput = 0;
int output_prod_idx = 0;
int output_cons_idx = 0;
bool newOutput = false;
pthread_mutex_t mutex_output = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t output_full = PTHREAD_COND_INITIALIZER;

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

/* TODO */
void putParse(char* stringInput){
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_parse);
    /* printf("Inside putstring\n"); */

    // Append the item in the buffer
    strcpy(parseBuffer, stringInput);

    // Track the number of chars
    countParse = strlen(stringInput);

    // Update the production index
    parse_prod_idx = countParse; // TODO Needed?

    // Signal to the consumer that the buffer is no longer empty
    /* printf("newOutput is true\n"); */
    pthread_cond_signal(&parse_full);

    // Unlock the mutex
    pthread_mutex_unlock(&mutex_parse);
}

/* TODO */
void putOutput(char* stringInput){
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_output);
    /* printf("Inside putstring\n"); */

    // Append the item in the buffer
    strcpy(outputBuffer, stringInput);

    // Track the number of chars
    countOutput = strlen(stringInput);

    // Update the production index
    output_prod_idx = countOutput; // TODO Needed?

    // Signal to the consumer that the buffer is no longer empty
    /* printf("newOutput is true\n"); */
    pthread_cond_signal(&output_full);

    // Unlock the mutex
    pthread_mutex_unlock(&mutex_output);
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
void getParsed(char* stringInput){

    pthread_mutex_lock(&mutex_input); 
    
    while (countInput == 0){ 
        pthread_cond_wait(&input_full, &mutex_input);
    }

    strncpy(stringInput, inputBuffer, input_prod_idx); 
  
    pthread_mutex_unlock(&mutex_input);
}

/* TODO */
void getOutput(char* stringInput){

    pthread_mutex_lock(&mutex_output); //TODO change as we add buffers
    
    while (countOutput == 0){ //TODO change as we add buffers
        pthread_cond_wait(&output_full, &mutex_output);//TODO change as we add buffers
    }

    strncpy(stringInput, outputBuffer, output_prod_idx); //TODO change as we add buffers
  
    pthread_mutex_unlock(&mutex_output);
}

/* TODO */
void replaceHelper(char* stringInput, int length){
    for (int i = 0; i < length; ++i) {
        if(stringInput[i] == '\n'){
            stringInput[i] = ' ';
        }
    }
}

/* TODO */
void* replaceSeparator(){
    char stringInput[MAX_CHARS * MAX_LINES] = {0};
    while (1) {
        getParsed(stringInput);

        char* stop = strstr(stringInput, "STOP\n");

        if (stop != NULL){
            replaceHelper(stringInput, strlen(stringInput) - 1);
        } else {
            replaceHelper(stringInput, strlen(stringInput));
        }

        putParse(stringInput);
        
        if (stop){
            return NULL;
        }
    }
    return NULL;
}

/**
    Replace string method replaces any instance of a substring with a
    replacement string.

    pre: The original string must be big enough to old any replacing 
	param: char* -> The old string to replace
	param: const char* -> The substring to search for
	param: const char* -> the replacement string to subsitute into the old string
	post: The old string is replaced

    sources: 
- https://stackoverflow.com/questions/32413667/replace-all-occurrences-of-a-substring-in-a-string-in-c
- https://stackoverflow.com/questions/35595389/efficiently-replace-a-substring-in-a-string
- http://www.cplusplus.com/reference/cstring/strstr/

 */
void replaceString(char* oldString, const char* subString, const char* replacement){
    int replacementLength = strlen(replacement);
    int subStringLength = strlen(subString);

    char buffer[MAX_CHARS] = {0};
    char *insert_point = &buffer[0];
    const char *temp = oldString;

    while (1){
        /* Find the next occurance of the substring in the oldString */
        const char *occurance = strstr(temp, subString);

        if (occurance == NULL){ // no more occurances
            strcpy(insert_point, temp); //copy the rest of the old string
            break;
        }

        /* Copy everything up to the substring into our buffer*/
        memcpy(insert_point, temp, occurance - temp);
        insert_point += occurance - temp;


        /* insert the substring into the buffer */
        memcpy(insert_point, replacement, replacementLength);
        insert_point += replacementLength;

        /* move the pointer up the string */
        temp = occurance + subStringLength;
    }

    /* copy what was in our buffer back into the oldString */
    strcpy(oldString, buffer);
}


/* TODO */
void getPlus(char* stringInput){

    pthread_mutex_lock(&mutex_parse); 
    
    while (countParse == 0){ 
        pthread_cond_wait(&parse_full, &mutex_parse);
    }
    
    strncpy(stringInput, parseBuffer, parse_prod_idx); //TODO change as we add buffers
    
    pthread_mutex_unlock(&mutex_parse);
}

void* replacePlus(){
    char stringInput[MAX_CHARS * MAX_LINES] = {0};

    while (1) {
        getPlus(stringInput);
        
        char* stop = strstr(stringInput, "STOP\n");

        char replacement[2] = "^";
        char subString[] = "++";

        if(strstr(stringInput, subString) != NULL){
            replaceString(stringInput, subString, replacement);
        }

        putOutput(stringInput);

        if (stop){
            return NULL;
        }
    }
}
/**

	param: const char* -> TODO
	post:  the string passed in is replaced with new one, with expanded '$$'
 */
void displayHelper(char* stringInput, int charSkip){
    while (((strlen(stringInput) - charSkip) - parse_cons_idx) >=  80){ // TODO change as we add buffers

        int end = parse_cons_idx + 80; // TODO change as we add buffers

        for (int i = parse_cons_idx; i < end; ++i) {
            printf("%c", stringInput[i]);
            fflush(stdout);
        }
        parse_cons_idx = end;
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
