// gcc -Wall -Wextra -o example-3.1 example-3.1.c -lpthread

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int val = 0;  // Shared condition data

void *waiter(void *arg)
{
    (void)arg;

    pthread_mutex_lock(&mutex);

    while (val < 5) {
        printf("Waiter: sleeping because %d < 5\n", val);
        pthread_cond_wait(&cond, &mutex);
        printf("Waiter: waking up to see if %d >= 5\n", val);
    }

    printf("Waiter: it is! Let's do our work right now!\n");

    // Make my own local copy--we can't use val outside the mutex since
    // it's shared, but we can use our own local copy of it.
    int my_val = val;

    pthread_mutex_unlock(&mutex);

    printf("Waiter: I can do work with my own copy of val: %d\n", my_val);

    return NULL;
}

int main(void)
{
    pthread_t t;

    pthread_create(&t, NULL, waiter, NULL);

    for (int i = 0; i <= 5; i++) {
        pthread_mutex_lock(&mutex);

        // We have the lock, so we can modify shared val
        val = i;

        pthread_cond_signal(&cond);  // Or broadcast if you should

        pthread_mutex_unlock(&mutex);

        //sleep(1);  // Sleep for one second
    }

    pthread_join(t, NULL);
}