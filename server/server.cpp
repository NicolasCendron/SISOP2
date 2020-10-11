#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
//#include <curses.h>
#include <semaphore.h>
//#include "../utils/functions.h"
#include "headers/server_communication_manager.h"

#include <iostream>
#include <fstream>
using namespace std;

#define NUM_THREADS 3

struct thread_data
{
  int thread_id;
  int port;
};

sem_t semaphore_server;

pthread_t listeningThreads[NUM_THREADS];
struct thread_data td[NUM_THREADS];




int main(int argc, char **argv){
    
   // Porta  lista_portas[NUM_THREADS];

    ofstream myfile;
    myfile.open ("../utils/portas.txt");
  
    int i,rc;
    for( i=0; i < NUM_THREADS; i++ )    
    {
        td[i].thread_id = i;

        td[i].port = 9901 + i;
        //lista_portas[i].porta = 9901 + i;
        //lista_portas[i].status = 1;
        myfile << to_string(9901 + i) + ";";
        myfile << "1;\n";
        rc = pthread_create(&listeningThreads[i], NULL, startListening, (void *)&td[i]);

        if (rc){

            printf("Error:unable to create thread: %d",rc);
            fflush(stdout);
            exit(-1);    
        }    
    } 
    myfile.close();
    //exit(1);
    pthread_exit(NULL); 



}



