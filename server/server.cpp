#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
//#include <curses.h>
#include <semaphore.h>
//#include "../utils/functions.h"
#include "packet.h"
//#include "functions.h"
#include "headers/server_communication_manager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <strings.h>
using namespace std;

#define NUMBER_OF_REPLICAS 3
#define INITIAL_PORT_REPLICA 50020
#define INITIAL_SERVER_REPLICA 30020
#define READ_FROM_FILE 2000
#define WRITE_TO_FILE 2001

#define BUFFER_SIZE 2560
#define NUM_THREADS 4

struct thread_data
{
  int thread_id;
  int port;
};

sem_t semaphore_server;

pthread_t listeningThreads[NUM_THREADS];
struct thread_data td[NUM_THREADS];

struct sockaddr_in prepServerConnection(struct hostent* server, int portno) {

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));  //Inicializa estrutura de comunicação com o server
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    return serv_addr;
} 

struct hostent * getServerInfo(string host) {
    struct hostent *server;
    server = gethostbyname(host.c_str()); // Pega o endereço do Server
    if (server == NULL) {
        std::cout << RED << "ERROR, no such host" << RESET << std::endl;
        fflush(stdout);
        exit(0);
    }
}

int extern createSocket();

int connectToReplicas() {
    bool bTerminate = false;
    int sockfd, n,port;
    struct sockaddr_in serv_addr; 

    struct hostent *server;
    string strMessageContent;
    char buffer[BUFFER_SIZE];
    for(int cont = 0; cont < NUMBER_OF_REPLICAS; cont ++){ 
        sockfd = createSocket();
        cout << YELLOW << "Conectado ao DB na porta : " << INITIAL_PORT_REPLICA + cont <<endl;
        server = getServerInfo("127.0.0.1"); //127.0.0.1 
        serv_addr = prepServerConnection(server,INITIAL_PORT_REPLICA + cont);
        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) //Conecta
        {
            std::cout << RED << "\n ERROR connecting" << RESET << std::endl;
            fflush(stdout);
        }
        port = INITIAL_PORT_REPLICA + cont;
        addReplicaNSock(port,sockfd);
    }

    return 0;
}


int main(int argc, char **argv){
    
   // Porta  lista_portas[NUM_THREADS];

    ofstream myfile;
    myfile.open ("../utils/portas.txt");
    connectToReplicas();
    int i,rc;
    for( i=0; i < NUM_THREADS; i++ )    
    {
        td[i].thread_id = i;

        td[i].port = INITIAL_SERVER_REPLICA + i;
        //lista_portas[i].porta = 9901 + i;
        //lista_portas[i].status = 1;
        myfile << to_string(INITIAL_SERVER_REPLICA + i) + ";";
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



