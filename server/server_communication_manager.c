#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include "headers/server_communication_manager.h"
#include "packet.h"
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
#define BUFFER_SIZE 2560
#define USER_MESSAGE 1000

struct thread_data
{
  int thread_id;
  int port;
};

void printPacket(packet * pack){
    printf("\n Packet do Cliente");
    printf("\n Tipo de Mensagem: %d",pack->messageType);
    printf("\n Texto da mensagem: %s",pack->_payload);
    printf("\n Tamanho da mensagem: %d",pack->length);
    printf("\n Timestamp: %f",pack->timestamp);
}

int createSocket(){
  int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Cria socket para escutar um client
  if (sockfd < 0)
    error("ERROR opening socket");
  return sockfd;
}

struct sockaddr_in prepForListening(int portno){
  struct sockaddr_in serv_addr;
  bzero((char *) &serv_addr, sizeof(serv_addr)); // Prepara a estrutura para ouvir o client
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portno); //Converte host byte order para network byte order
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");;
  return serv_addr;
} 

void bindToSocket(int sockfd,struct sockaddr_in  serv_addr){
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) // Faz o bind na porta
    error("ERROR on binding");
}

void printListeningInfo(portno){
    printf("\nServer IP: %s : %d","127.0.0.1",portno);
    fflush(stdout);
}

void doNothingWhileListen(int sockfd){
  listen(sockfd,5); // Escuta a porta até alguem conectar
}

int acceptConnection(int sockfd,struct sockaddr_in cli_addr){
  int clilen = sizeof(cli_addr);
  int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //Aceita a conexão
  if (newsockfd < 0)
    error("ERROR on accept");  
  
}

void handleMessages(int newsockfd)
{
  int n = 0;
  packet *pack;
  while(true)
  {
    pack = (packet*)malloc(sizeof(packet));
    //bzero(buffer,BUFFER_SIZE);
    n = read(newsockfd,&pack,sizeof(pack));
    if (n < 0) error("ERROR reading from socket");
    printPacket(pack);
    //n = write(newsockfd,"message_here",18);
    //if (n < 0) error("ERROR writing to socket");
    //free(&pack);
  }
 
}

void* startListening(void *threadarg)
{ 
  struct thread_data *my_data;   
  my_data = (struct thread_data *) threadarg;

  int portno = my_data->port;
  int sockfd, newsockfd, clilen;
  struct sockaddr_in serv_addr, cli_addr;
  
  sockfd = createSocket();
  serv_addr = prepForListening(portno);
  bindToSocket(sockfd,serv_addr);
  printListeningInfo(portno);
  doNothingWhileListen(sockfd);
  newsockfd = acceptConnection(sockfd, cli_addr);
  handleMessages(newsockfd);
 
  pthread_exit(NULL); 
}