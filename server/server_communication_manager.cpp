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
#include <algorithm> 
#include <cctype>
#include <locale>
#include <sstream>
#include <fstream>
#include <vector>
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
#define USER_MESSAGE 1000
#define USER_CONNECTED_MESSAGE 1001
#define USER_DISCONNECTED 1002
#define USER_EXIT_GROUP 1003
#define PROTOCOL_INT_SIZE 10
#define PROTOCOL_STRING_SIZE 140
#define PROTOCOL_PACKET_SIZE 3*PROTOCOL_STRING_SIZE + 2*PROTOCOL_INT_SIZE
struct thread_data
{
  int thread_id;
  int port;
};
char buffer[BUFFER_SIZE] = {0}; 
void printPacket(packet * pack){
  std::cout << "\nPacket do Cliente" << std::endl;
  std::cout << "Tipo de Mensagem: " + to_string(pack->nMessageType)  << std::endl;
  std::cout << "Texto da mensagem: " + pack->strPayload  << std::endl;
  std::cout << "Nome Usuário: " + pack->strUserName  << std::endl;
  std::cout << "Nome Grupo: " + pack->strGroupName  << std::endl;
  //std::cout << "Tamanho da mensagem: " + to_string(pack->nLength)  << std::endl;
  //std::cout << "Timestamp: " + to_string(pack->timestamp)  << std::endl;

}

int createSocket(){
  int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Cria socket para escutar um client
  if (sockfd < 0)
    std::cout << "ERROR opening socket" << std::endl;
  return sockfd;
}

vector<packet> vecPackets;

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
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

string padLeft(string strOld,char cPad,int nSize){
    return std::string(nSize - strOld.length(), cPad) + strOld;
}

string serializePacket(packet * pack)
{
    string serialized;
    serialized = serialized + padLeft(to_string(pack->nMessageType),'0',PROTOCOL_INT_SIZE);
    serialized = serialized + padLeft(to_string(pack->nLength),'0',PROTOCOL_INT_SIZE);
    serialized = serialized + padLeft(pack->strPayload,' ',PROTOCOL_STRING_SIZE);
    serialized = serialized + padLeft(pack->strUserName,' ',PROTOCOL_STRING_SIZE);
    serialized = serialized + padLeft(pack->strGroupName,' ',PROTOCOL_STRING_SIZE);
    //std::cout << "**" +  serialized + "&&" << std::endl;
    return serialized;
}

packet* deserializePacket(string strPack)
{   int nPointer = 0;
    packet *pack = new packet;
    string strBuff;

    std::istringstream( strPack.substr(nPointer,PROTOCOL_INT_SIZE) ) >> pack->nMessageType; nPointer+=PROTOCOL_INT_SIZE;
    std::istringstream( strPack.substr(nPointer,PROTOCOL_INT_SIZE) ) >> pack->nLength; nPointer+=PROTOCOL_INT_SIZE;
    pack->strPayload = strPack.substr(nPointer,PROTOCOL_STRING_SIZE);ltrim( pack->strPayload ) ;nPointer+=PROTOCOL_STRING_SIZE;
    pack->strUserName = strPack.substr(nPointer,PROTOCOL_STRING_SIZE); ltrim(pack->strUserName) ; nPointer+=PROTOCOL_STRING_SIZE;
    pack->strGroupName = strPack.substr(nPointer,PROTOCOL_STRING_SIZE); ltrim(pack->strGroupName) ; nPointer+=PROTOCOL_STRING_SIZE;  
    return pack;
}

packet* readFromSocket(int newsockfd){
    char * buffer = (char*)malloc(PROTOCOL_PACKET_SIZE);
    int n = read(newsockfd,buffer,PROTOCOL_PACKET_SIZE);
    if (n < 0) 
      std::cout << "ERROR reading from socket" << std::endl;
  
    fflush(stdout);
    
    packet * pack = deserializePacket(string(buffer));
    //printPacket(pack);
    return pack;
}


void writeMessageToFile(packet *pack){
    std::ofstream outfile;
    outfile.open(pack->strGroupName, std::ios_base::app); // append instead of overwrite
    outfile << serializePacket(pack);
}

int handleMessages(int newsockfd)
{

  packet *pack;
  while(true)
  {
    pack = readFromSocket(newsockfd);
  
    switch(pack -> nMessageType){
      case USER_MESSAGE:
        writeMessageToFile(pack);
        break;
    }


  }
 
}


void readEntireFile(){
  ifstream file("group");
  char buffer[PROTOCOL_PACKET_SIZE + 1];
  
  while (file.read(&buffer[0], PROTOCOL_PACKET_SIZE))
  {
    printPacket(deserializePacket(string(buffer)));
  }
                // terminated for use with cout.
}


void* startListening(void *threadarg)
{
  //readEntireFile();
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
    doNothingWhileListen(sockfd);
    newsockfd = acceptConnection(sockfd, cli_addr);
    handleMessages(newsockfd);
  }
  pthread_exit(NULL); 
}