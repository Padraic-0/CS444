#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "eventbuf.h"

int main(int argc, char *argv[]){
    if (argc != 5){
        fprintf(stderr,"Usage: # of producers , # of consumers, # of events, max outstanding\n");
        exit(1);
    }
    int producers = atoi(argv[1]);
    int consumers = atoi(argv[2]);
    int max_events = atoi(argv[3]);
    int max_outstanding = atoi(argv[4]);
    return 0;
}