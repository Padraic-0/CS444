#include <pthread.h>
#include <stdio.h>

void * run(void * arg){
  char * thread_num = arg;

  for (int i = 0; i <= 4; i++){
    printf("Thread %s: %d\n", thread_num, i);
  }
  
  return NULL;
}

int main(void){
  pthread_t t1;
  pthread_t t2;

  pthread_create(&t1, NULL, run, "1");
  pthread_create(&t2, NULL, run, "2");

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  return 0;
}