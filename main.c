/******************************************************************************
* File:             main.c
*
* Author:           Gavin Slusher  
* Created:          2020-10-20
* Description:      
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "helpers.h"

int main(int argc, const char * argv[]) {
    pthread_t input_t, parse_t, replacePlus_t, output_t;
    char line[MAX_CHARS * MAX_LINES] = {0};

    // Get a line of input
    pthread_create(&input_t, NULL, getString, line);

    // Replace every line separator with a space
    pthread_create(&parse_t, NULL, replaceSeparator, NULL);

    // Replace every pair of plus signs
    pthread_create(&replacePlus_t, NULL, replacePlus, NULL);

    // Write the output as lines of exactly 80 chars
    pthread_create(&output_t, NULL, displayOutput, NULL);

    // Wait for the threads to terminate
    pthread_join(input_t, NULL);
    pthread_join(parse_t, NULL);
    pthread_join(replacePlus_t, NULL);
    pthread_join(output_t, NULL);

    return 1;
}
