#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include "headers/server_communication_manager.h"

void error(char *msg)
{
  perror(msg);
  fflush(stdout);
  exit(1);
}

// struct sockaddr_in
// {
//   short   sin_family; /* must be AF_INET */
//   u_short sin_port;
//   struct  in_addr sin_addr;
//   char    sin_zero[8]; /* Not used, must be zero */
// };

typedef int bool;
#define true 1
#define false 0

struct thread_data
{
  int thread_id;
  int port;
};


void* startListening(void *threadarg)
{ 
  struct thread_data *my_data;   
  my_data = (struct thread_data *) threadarg;

  int portno = my_data->port;
  int sockfd, newsockfd, clilen, n;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  sockfd = socket(AF_INET, SOCK_STREAM, 0); // Cria socket para escutar um client
  if (sockfd < 0)
    error("ERROR opening socket");

  bzero((char *) &serv_addr, sizeof(serv_addr)); // Prepara a estrutura para ouvir o client
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portno); //Converte host byte order para network byte order
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");;

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) // Faz o bind na porta
    error("ERROR on binding");

    printf("\nServer IP: %s : %d","127.0.0.1",portno);
    fflush(stdout);
  
  listen(sockfd,5); // Escuta a porta até alguem conectar

  clilen = sizeof(cli_addr);
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //Aceita a conexão
  if (newsockfd < 0)
    error("ERROR on accept");  
  
  while(true)
  {
    bzero(buffer,256);
    n = read(newsockfd,buffer,255);
    if (n < 0) error("ERROR reading from socket");
    printf("\n Received message: %s",buffer);
    n = write(newsockfd,"message_here",18);
    if (n < 0) error("ERROR writing to socket");
  }
 
  pthread_exit(NULL); 
}