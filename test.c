#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h> 

pthread_t t1;
pthread_t t2;
pthread_mutex_t mutex;

static int counter;

void t1_main(void);
void t2_main(void);
void printing(int thread_num);


void sig_handler(int signum) {

    printf("Received SIGINT. Exit\n");

    pthread_cancel(t1);
    pthread_cancel(t2);

    exit(0);
}

void t1_main(void) {
    printf("Thread 1");
    while(1) {
        usleep(1000000);
        printing(1);
    }
}


void t2_main(void) {
    printf("Thread 2");
    while(1) {
        usleep(1000000);
        printing(2);
    }
}

void printing(int thread_num) {
    int i;
    pthread_mutex_lock(&mutex);
    printf("\n");
    counter += 1;
    printf("Thread %d, counter =  %d",thread_num, counter);
    usleep(1);
    pthread_mutex_unlock(&mutex);
}

int main(void) {
    pthread_attr_t attr;
    int status;
 
    signal(SIGINT, sig_handler);

    counter = 0;

    pthread_attr_init(&attr);
   
    printf("Creating t1\n");
    status = pthread_create(&t1, &attr, (void*)&t1_main, NULL);   

    printf("Creating t2\n");
    status = pthread_create(&t2, &attr, (void*)&t2_main, NULL); 

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    sig_handler(SIGINT);
    
    return 0;
}