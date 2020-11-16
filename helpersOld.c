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
char parseBuffer[MAX_CHARS * MAX_LINES];
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
char outputBuffer[MAX_CHARS * MAX_LINES];
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

    // Put the item in the buffer
    strcpy(inputBuffer, stringInput);

    // Track the number of chars
    countInput = strlen(stringInput);

    // Signal to the consumer that the buffer is no longer empty
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
    bool STOP = false;
    do {

        fgets(stringInput, MAX_CHARS, stdin);

        if (strcmp(stringInput, "STOP\n") == 0){
            STOP = true;
            putString(stringInput);
        } else{
            numLines++;
            putString(stringInput);
        }
    
    } while (STOP ==  false);
            
    return NULL;
}

/* TODO */
void putParsed(char* stringInput){
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_parse);
    /* printf("Inside putParsed\n"); */

    // Put the item in the buffer
    strcpy(parseBuffer, stringInput);

    // Track the number of chars
    countParse = strlen(stringInput);

    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&parse_full);

    // Unlock the mutex
    pthread_mutex_unlock(&mutex_parse);
}

/* TODO */
void* replaceSeparator(){
    char stringInput[MAX_CHARS] = {0};
    
    /* while(strcmp(stringInput, "STOP\n") != 0){ */
    while(1){
        // Lock the mutex before checking if the buffer has data
        pthread_mutex_lock(&mutex_input);
        
        while (countInput == 0){
            // Buffer is empty. Wait for the producer to signal that the buffer has data
            pthread_cond_wait(&input_full, &mutex_input);
        }

        strcpy(stringInput, inputBuffer);
        /* printf("Working in replaceSeparator\n"); */
        
        if (strcmp(stringInput, "STOP\n") == 0){
            strcpy(inputBuffer, "");
            countInput = 0;
            
            putParsed(stringInput);
            pthread_mutex_unlock(&mutex_input);
            break;
        }

        if (strcmp(stringInput, "STOP\n") != 0){
            for (int i = 0; i < countInput; ++i) {
                if(stringInput[i] == '\n'){
                    stringInput[i] = ' ';
                }
            }
        }

        strcpy(inputBuffer, "");
        countInput = 0;

        putParsed(stringInput);
        pthread_mutex_unlock(&mutex_input);
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
void putOutput(char* stringInput){
  // Lock the mutex before putting the item in the buffer
  pthread_mutex_lock(&mutex_output);

  // Put the item in the buffer
  strcat(outputBuffer, stringInput);

  // Track the number of chars
  countOutput += strlen(stringInput);
  newOutput = true;
  
  // Signal to the consumer that the buffer is no longer empty
  pthread_cond_signal(&output_full);
  
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_output);
}

/**

	param: const char* -> TODO
	post:  the string passed in is replaced with new one, with expanded '$$'
 */
void* replacePlus(){
    char stringInput[MAX_CHARS] = {0};

    /* while(strcmp(stringInput, "STOP\n") != 0){ */
    while(1){
        // Lock the mutex before checking if the buffer has data
        pthread_mutex_lock(&mutex_parse);
      
        while (countParse == 0){
            // Buffer is empty. Wait for the producer to signal that the buffer has data
            pthread_cond_wait(&parse_full, &mutex_parse);
        }
        
    /* printf("replacePlus doing work\n"); */
        strcpy(stringInput, parseBuffer);

        if (strcmp(stringInput, "STOP\n") == 0){
            strcpy(parseBuffer, "");
            countParse = 0;

            putOutput(stringInput);
            pthread_mutex_unlock(&mutex_parse);
            break;
        }

        char replacement[2] = "^";
        char subString[] = "++";

        if(strstr(stringInput, subString) != NULL){
            replaceString(stringInput, subString, replacement);
        }

        strcpy(parseBuffer, "");
        countParse = 0;

        putOutput(stringInput);
        pthread_mutex_unlock(&mutex_parse);
    }

    return NULL;
}

/**

	param: const char* -> TODO
	post:  the string passed in is replaced with new one, with expanded '$$'
 */
void* displayOutput(){
    while(1){
        // Lock the mutex before checking if the buffer has data
        /* pthread_mutex_lock(&mutex_output); */
        
        /* printf("Display Output being called\n"); */
        
        while (newOutput == false){
            // Buffer is empty. Wait for the producer to signal that the buffer has data
            pthread_mutex_lock(&mutex_output);
            pthread_cond_wait(&output_full, &mutex_output);
            pthread_mutex_unlock(&mutex_output);
        }
        
        if (strstr(outputBuffer, "STOP\n") != NULL){
            newOutput = false;
            break;
        }

        while ((countOutput - output_cons_idx) >= 80){

            int end = output_cons_idx + 80;
        
            for (int i = output_cons_idx; i < end; ++i) {
                printf("%c", outputBuffer[i]);
                output_cons_idx = i;
            }
            // TODO is this right?
            output_cons_idx++;
            printf("\n");
        }

        newOutput = false;
    }
    return NULL;
}
