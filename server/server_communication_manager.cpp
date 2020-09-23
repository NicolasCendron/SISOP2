#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include "headers/server_communication_manager.h"
#include "packet.h"
#include <arpa/inet.h>
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

#define BUFFER_SIZE 2560
#define USER_MESSAGE_TYPE 1000
#define PROTOCOL_LENGTH_SIZE 10
struct thread_data
{
  int thread_id;
  int port;
};
char buffer[BUFFER_SIZE] = {0}; 
void printPacket(packet * pack){
  std::cout << "Packet do Cliente" << std::endl;
  std::cout << "Tipo de Mensagem: " + to_string(pack->nMessageType)  << std::endl;
  std::cout << "Texto da mensagem: " + pack->strPayload  << std::endl;
  std::cout << "Tamanho da mensagem: " + to_string(pack->nLength)  << std::endl;
  std::cout << "Timestamp: " + to_string(pack->timestamp)  << std::endl;

}

int createSocket(){
  int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Cria socket para escutar um client
  if (sockfd < 0)
    std::cout << "ERROR opening socket" << std::endl;
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
    std::cout << "ERROR on binding" << std::endl;
}

void printListeningInfo(int portno){
    std::cout << string("\n Server IP:") + string("127.0.0.1 : ") + to_string(portno) << std::endl;
    fflush(stdout);
}

void doNothingWhileListen(int sockfd){
  listen(sockfd,5); // Escuta a porta até alguem conectar
}

int acceptConnection(int sockfd,struct sockaddr_in cli_addr){
  socklen_t clilen = sizeof(cli_addr);
  int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //Aceita a conexão
  if (newsockfd < 0)
    std::cout << "ERROR on accept" << std::endl;
  return newsockfd;
}


string readFromSocket(int newsockfd){
    int n = 0;
    n = read(newsockfd,buffer,PROTOCOL_LENGTH_SIZE);
    int sizeOfMessage = atoi(buffer);
    bzero(buffer,BUFFER_SIZE);
    n = read(newsockfd,buffer,sizeOfMessage);
    if (n < 0) 
      std::cout << "ERROR reading from socket" << std::endl;
    fflush(stdout);
    string x = string(buffer);
    return x;
}

int handleMessages(int newsockfd)
{

  packet *pack;
  while(true)
  {
    string message = readFromSocket(newsockfd);

    std::cout << string("*") + message + string("*") << std::endl;
    fflush(stdout);
    if(message.empty())
        return 0;
      
  }
 
}

void* startListening(void *threadarg)
{
  while(true)
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
  }
  pthread_exit(NULL); 
}