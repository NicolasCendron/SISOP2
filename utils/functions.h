#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <sstream>
#include <fstream>
#include <vector>
#include <ctime>
#include "../client/colors.h"



#define BUFFER_SIZE 2560
#define USER_MESSAGE 1000
#define USER_CONNECTED_MESSAGE 1001
#define USER_DISCONNECTED 1002
#define USER_EXIT_GROUP 1003
#define USER_MAX_CONNECTIONS 1004
#define ASK_SEQ 1005
#define PROTOCOL_INT_SIZE 10
#define PROTOCOL_STRING_SIZE 140
#define PROTOCOL_LONG_SIZE 20
#define PROTOCOL_PACKET_SIZE 3*PROTOCOL_STRING_SIZE + PROTOCOL_INT_SIZE + PROTOCOL_LONG_SIZE

/*  SOCKETS  */

int writeToSocket(int sockfd, string message){
     fflush(stdin);
    int nMessageLength = message.length();
    int n;
    n = write(sockfd,message.c_str(),nMessageLength); // Envia para o server
    if (n < 0){
        std::cout << "\nERROR writing connected message" << std::endl;
        fflush(stdout);
    }
    return 0;
}


string timestamp_to_date(time_t timestamp_packet){
    struct tm ts;
    char buf[80];

    //time_t now;
    time(&timestamp_packet);
    ts = *localtime(&timestamp_packet);
    strftime(buf, sizeof(buf),"%H:%M:%S",&ts);

    //std::time_t test = 1439467747492;
    //std::cout << std::ctime(&test);
    //std::cout <<  buf << std::endl;
    
    return buf;
}



/*  PACKETS  */
void printPacket(packet * pack){
  std::cout << "\nPacket do Cliente" << std::endl;
  std::cout << "Tipo de Mensagem: " + to_string(pack->nMessageType)  << std::endl;
  std::cout << "Numero de Sequencia: " + to_string(pack->nSeq)  << std::endl;
  std::cout << "Texto da mensagem: " + pack->strPayload  << std::endl;
  std::cout << "Nome Usuário: " + pack->strUserName  << std::endl;
  std::cout << "Nome Grupo: " + pack->strGroupName  << std::endl;
  //std::cout << "Timestamp: " + to_string(pack->nTimeStamp)  << std::endl;
  std::cout << timestamp_to_date(pack->nTimeStamp)  << std::endl;
 
}




string padLeft(string strOld,char cPad,int nSize){
    return std::string(nSize - strOld.length(), cPad) + strOld;
}

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

packet* deserializePacket(string strPack)
{   int nPointer = 0;
    packet *pack = new packet;
    string strBuff;

    std::istringstream( strPack.substr(nPointer,PROTOCOL_INT_SIZE) ) >> pack->nMessageType; nPointer+=PROTOCOL_INT_SIZE;
    std::istringstream( strPack.substr(nPointer,PROTOCOL_LONG_SIZE) ) >> pack->nTimeStamp; nPointer+=PROTOCOL_LONG_SIZE;
    pack->strPayload = strPack.substr(nPointer,PROTOCOL_STRING_SIZE);ltrim( pack->strPayload ) ;nPointer+=PROTOCOL_STRING_SIZE;
    pack->strUserName = strPack.substr(nPointer,PROTOCOL_STRING_SIZE); ltrim(pack->strUserName) ; nPointer+=PROTOCOL_STRING_SIZE;
    pack->strGroupName = strPack.substr(nPointer,PROTOCOL_STRING_SIZE); ltrim(pack->strGroupName) ; nPointer+=PROTOCOL_STRING_SIZE;  
    return pack;
}

string serializePacket(packet * pack)
{
    string serialized;
    serialized = serialized + padLeft(to_string(pack->nMessageType),'0',PROTOCOL_INT_SIZE);
    serialized = serialized + padLeft(to_string(pack->nTimeStamp),'0',PROTOCOL_LONG_SIZE);
    serialized = serialized + padLeft(pack->strPayload,' ',PROTOCOL_STRING_SIZE);
    serialized = serialized + padLeft(pack->strUserName,' ',PROTOCOL_STRING_SIZE);
    serialized = serialized + padLeft(pack->strGroupName,' ',PROTOCOL_STRING_SIZE);
    //std::cout << "**" +  serialized + "&&" << std::endl;
    return serialized;
}


packet* readFromSocket(int newsockfd){
    char * buffer = (char*)malloc(PROTOCOL_PACKET_SIZE);

    int n = read(newsockfd,buffer,PROTOCOL_PACKET_SIZE);
    if (n < 0) 
      std::cout << RED << "ERROR reading from socket" << RESET << std::endl;
  
    fflush(stdout);
    
    packet * pack = deserializePacket(string(buffer));
    return pack;
}