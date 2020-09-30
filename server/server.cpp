#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "headers/server_communication_manager.h"
#define NUM_THREADS 4

struct thread_data
{
  int thread_id;
  int port;
};


pthread_t listeningThreads[NUM_THREADS];
struct thread_data td[NUM_THREADS];

int main(int argc, char **argv){

    int i,rc;
    for( i=0; i < NUM_THREADS; i++ )    
    {
        td[i].thread_id = i;

        td[i].port = 7001 + i;

        rc = pthread_create(&listeningThreads[i], NULL, startListening, (void *)&td[i]);

        if (rc){

            printf("Error:unable to create thread: %d",rc);
            fflush(stdout);
            exit(-1);    
        }    
    } 
    pthread_exit(NULL); 

}