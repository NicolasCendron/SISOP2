#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include "headers/server_communication_manager.h"
#include "headers/packet.h"
#include "headers/connection.h"
#include <arpa/inet.h>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <sstream>
#include <fstream>
#include <vector>
#include "colors.h"
#include "../utils/functions.h"


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


struct thread_data
{
  int thread_id;
  int port;
};
char buffer[BUFFER_SIZE] = {0}; 


int createSocket(){
  int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Cria socket para escutar um client
  if (sockfd < 0)
    std::cout << "ERROR opening socket" << std::endl;
  return sockfd;
}

vector<connection*> arrConnection;
int NEXT_SEQ = 0;

int getNextSeq()
{
  ++NEXT_SEQ;
  return NEXT_SEQ;
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
    std::cout << CYAN << string("\n Server IP:") + string("127.0.0.1 : ") + to_string(portno) << RESET << std::endl;
    fflush(stdout);
}


/*
 * retorna 0: sucesso
 * retorna -1: erro
 * */
int doNothingWhileListen(int sockfd){
  return listen(sockfd,5); // Escuta a porta até alguem conectar
}

int acceptConnection(int sockfd,struct sockaddr_in cli_addr){
  socklen_t clilen = sizeof(cli_addr);
  int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //Aceita a conexão
  if (newsockfd < 0)
    std::cout << "ERROR on accept" << std::endl;
  return newsockfd;
}




void writeMessageToFile(packet *pack){
    std::ofstream outfile;
    outfile.open(pack->strGroupName, std::ios_base::app); // append instead of overwrite
    outfile << serializePacket(pack);
}

vector<packet*> readEntireFile(string strGroupName){
  ifstream file(strGroupName.c_str());
  char buffer[PROTOCOL_PACKET_SIZE + 1];
  vector<packet*> arrPacks;
  packet *pack = new packet();
  while (file.read(&buffer[0], PROTOCOL_PACKET_SIZE))
  {
    pack = deserializePacket(string(buffer));
    printPacket(pack);
    arrPacks.push_back(pack);
  }
  return arrPacks;
}

void sendSequenceNumber(packet *pack, int newsockfd)
{
  pack-> nSeq = getNextSeq();
  writeToSocket(newsockfd,serializePacket(pack));
}

void sendConnectionFailedMessage(packet *pack, int newsockfd)
{
  pack-> nMessageType = USER_MAX_CONNECTIONS;
  writeToSocket(newsockfd,serializePacket(pack));
}

void sendMessageHistoryToClient(string strGroupName, int newsockfd){
  vector<packet*> arrPacks = readEntireFile(strGroupName);
  for(auto pack: arrPacks){ 
	  writeToSocket(newsockfd,serializePacket(pack));
  } 

}

bool handleUserConnection(packet *pack, int newsockfd)
{
  string strUserName = pack->strUserName;
  string strGroupName = pack->strGroupName;
  int nSocket = newsockfd;
 
  int nCountUserConnections = 1;
  for(auto connection: arrConnection){ 
	  if(pack->strUserName.compare(connection->strUserName) == 0 ){ /// Verifica Numero de Conexoes
      nCountUserConnections++;
    }
  }  
  cout << nCountUserConnections <<endl;
  if(nCountUserConnections > 2){
    return false;
  }

  connection *oConn = new connection;
  oConn->strUserName = strUserName;
  oConn->strGroupName = strGroupName;
  oConn->nSocket = newsockfd;

  arrConnection.push_back(oConn); // Computa essa conexão

  return true;
}

int handleMessages(int newsockfd)
{

  packet *pack;
  bool bConnectionSuccess = true;
  while(true)
  {
    pack = readFromSocket(newsockfd);
  
    switch(pack -> nMessageType){
      case ASK_SEQ:
        sendSequenceNumber(pack,newsockfd);
        break;
      case USER_MESSAGE:
        writeMessageToFile(pack);
        break;
      case USER_CONNECTED_MESSAGE:
        bConnectionSuccess = handleUserConnection(pack, newsockfd);
        if(bConnectionSuccess)
        {
          writeMessageToFile(pack);
          sendMessageHistoryToClient(pack->strGroupName,newsockfd);
        }
        else{
          sendConnectionFailedMessage(pack,newsockfd);
        }
        break;
    }
    printPacket(pack);
  }
 
}

void* startListening(void *threadarg)
{
  //readEntireFile(String("group"));
  //return (void*)1;

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
    if(doNothingWhileListen(sockfd) < 0){
      std::cout << RED << "Err: not listening" << RESET << std::endl;
      exit(1);
    }

  //init_sem(&semaforo_server,0,1);  // nome do semáforo -- 0 pq compartilha o semáforo entre threads (1 é para o caso de compartilhar entre processos) -- num threads simultaneas

    newsockfd = acceptConnection(sockfd, cli_addr);
    handleMessages(newsockfd);
  }
  pthread_exit(NULL); 
}