#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "eventbuf.h"

struct eventbuf *eb;
int max_events;
sem_t *writing;
sem_t *in_queue_mutex;
sem_t *exiting_producers_mutex;
int exiting_producers;
int in_queue = 0;
int max_outstanding;

sem_t *sem_open_temp(const char *name, int value)
{
    sem_t *sem;

    // Create the semaphore
    if ((sem = sem_open(name, O_CREAT, 0600, value)) == SEM_FAILED)
        return SEM_FAILED;

    // Unlink it so it will go away after this process exits
    if (sem_unlink(name) == -1) {
        sem_close(sem);
        return SEM_FAILED;
    }

    return sem;
}

void *producer(void *arg){
    int *producer_number = arg;
    for (int i = 0; i < max_events; i++){
        int event = *producer_number * 100 + i;
        sem_wait(writing);
        sem_wait(in_queue_mutex);
        if(in_queue < max_outstanding){
            printf("P%d: adding event %d\n", *producer_number, event);
            in_queue += 1;
            eventbuf_add(eb,event);
        }else{
            i -= 1;
        }
        sem_post(in_queue_mutex);
        sem_post(writing);
        
        
    }
    sem_wait(exiting_producers_mutex);
    exiting_producers -= 1;
    sem_post(exiting_producers_mutex);
    printf("P%d: exiting\n", *producer_number);

    return 0;
}

void *consumer(void* arg){
    int *consumer_number = arg;
    while(1){
        sem_wait(writing);
        if(eventbuf_empty(eb)){
            sem_post(writing);
            sem_wait(exiting_producers_mutex);
            if(exiting_producers == 0){
                printf("C%d: exiting\n", *consumer_number);
                sem_post(exiting_producers_mutex);
                return 0;
            }
            sem_post(exiting_producers_mutex);
        }else{
            printf("C%d: got event %d\n", *consumer_number, eventbuf_get(eb));

            sem_wait(in_queue_mutex);
            in_queue -= 1;
            sem_post(in_queue_mutex);

            sem_post(writing);
        }
    }

    return 0;
}

int main(int argc, char *argv[]){
    if (argc != 5){
        fprintf(stderr,"Usage: # of producers , # of consumers, # of events, max outstanding\n");
        exit(1);
    }
    int producers = atoi(argv[1]);
    int consumers = atoi(argv[2]);
    max_events = atoi(argv[3]);
    max_outstanding = atoi(argv[4]);

    exiting_producers = producers;

    // Allocate thread handle array for all producers
    pthread_t *producer_thread = calloc(producers, sizeof *producer_thread);

    // Allocate thread ID array for all producers
    int *producer_thread_id = calloc(producers, sizeof *producer_thread_id);

    // Allocate thread handle array for all consumers
    pthread_t *consumer_thread = calloc(consumers, sizeof *consumer_thread);

    // Allocate thread ID array for all consumers
    int *consumer_thread_id = calloc(consumers, sizeof *consumer_thread_id);

    eb = eventbuf_create();
    writing = sem_open_temp("writing", 1);
    in_queue_mutex = sem_open_temp("in_queue_mutex", 1);
    exiting_producers_mutex = sem_open_temp("exiting_producers_mutex",1);

    for (int i = 0; i < producers; i++){
        producer_thread_id[i] = i;
        pthread_create(producer_thread + i, NULL, producer, producer_thread_id + i);
    }

    for (int i = 0; i < consumers; i++){
        consumer_thread_id[i] = i;
        pthread_create(consumer_thread + i, NULL, consumer, consumer_thread_id + i);
    }

    for (int i = 0; i < producers; i++)
        pthread_join(producer_thread[i], NULL);

    for (int i = 0; i < consumers; i++)
        pthread_join(consumer_thread[i], NULL);

    eventbuf_free(eb);
    free(producer_thread);
    free(producer_thread_id);
    free(consumer_thread);
    free(consumer_thread_id);
    
    return 0;
}