#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <inttypes.h>
#include <math.h>
#include "packet.h"

// struct  hostent
// {
//   char    *h_name;        /* official name of host */
//   char    **h_aliases;    /* alias list */
//   int     h_addrtype;     /* host address type */
//   int     h_length;       /* length of address */
//   char    **h_addr_list;  /* list of addresses from name server */
//   #define h_addr  h_addr_list[0]  /* address, for backward compatiblity */
// };

// h_name       Official name of the host.
// h_aliases    A zero  terminated  array  of  alternate
//              names for the host.
// h_addrtype   The  type  of  address  being  returned;
//              currently always AF_INET.
// h_length     The length, in bytes, of the address.
// h_addr_list  A pointer to a list of network addresses
//              for the named host.  Host addresses are
//              returned in network byte order.

#define BUFFER_SIZE 2560
#define USER_MESSAGE 1000
#define USER_CONNECTED_MESSAGE 1001
#define USER_DISCONNECTED 1002
#define USER_EXIT_GROUP 1003
#define PROTOCOL_INT_SIZE 10
#define PROTOCOL_STRING_SIZE 140
#define PROTOCOL_PACKET_SIZE 2*PROTOCOL_STRING_SIZE + 2*PROTOCOL_INT_SIZE


string padLeft(string strOld,char cPad,int nSize){
    return std::string(nSize - strOld.length(), cPad) + strOld;
}

string getMessageNameByType(int msgType){

        switch (msgType)
        {
        case USER_MESSAGE:
            return string("Mensagem de Texto");
        case USER_CONNECTED_MESSAGE:
            return string("Usuário Conectado");
        case USER_DISCONNECTED:
            return string("Usuário Desconectou");
        case USER_EXIT_GROUP:
            return string("Usuário saiu do grupo");
        default:
            return string("");
        }
    
}



void printPacket(packet * pack){
    
    std::cout << "Packet do Cliente" << std::endl;
    std::cout << "Tipo de Mensagem:" + to_string(pack->nMessageType) << std::endl;
    std::cout << "Usuário:" + pack->strUserName << std::endl;
    std::cout << "Texto da mensagem:" + pack->strPayload << std::endl;
    //std::cout << "Tamanho da mensagem:" + to_string(pack->nLength) << std::endl;
    //std::cout << "Timestamp:" + to_string(pack->timestamp) << std::endl;
}

string serializePacket(packet * pack)
{
    string serialized;
    serialized = serialized + padLeft(to_string(pack->nMessageType),'0',PROTOCOL_INT_SIZE);
    serialized = serialized + padLeft(to_string(pack->nLength),'0',PROTOCOL_INT_SIZE);
    serialized = serialized + padLeft(pack->strPayload,' ',PROTOCOL_STRING_SIZE);
    serialized = serialized + padLeft(pack->strUserName,' ',PROTOCOL_STRING_SIZE);
    std::cout << "**" +  serialized + "&&" << std::endl;
    return serialized;
}

double getTimeStamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double time_in_mill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
    return time_in_mill;
}

string createUserMessage(string strUserName){
    char* buffer = (char*)malloc(PROTOCOL_STRING_SIZE);
    printf("\nPlease enter the message: ");
    fflush(stdout);
    string strUserMessage;
    getline(cin,strUserMessage); // Pega a mensagem

    packet *pack = new packet;

    pack->nMessageType = USER_MESSAGE;
    pack->nLength = strUserMessage.length();
    pack->strPayload =  strUserMessage;
    pack->strUserName = strUserName;

    return serializePacket(pack);
}

string createUserConnectedMessage(string strUserName){

    packet *pack = new packet;
    pack->nMessageType = USER_CONNECTED_MESSAGE;
    //pack->timestamp = getTimeStamp();
    pack->nLength = strUserName.length();
    pack->strUserName = strUserName;
    pack->strPayload = strUserName;//strUserName.substr(0,strUserName.length());

    return  serializePacket(pack);
}

int createSocket(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);// Cria Socket
    if (sockfd < 0) 
        {
            std::cout << "ERROR opening socket" << std::endl;
            fflush(stdout);
        }
        return sockfd;
}

struct hostent * getServerInfo(string host){
    struct hostent *server;
    server = gethostbyname(host.c_str()); // Pega o endereço do Server
    if (server == NULL) {
        std::cout << "ERROR, no such host" << std::endl;
        fflush(stdout);
        exit(0);
    }
}

struct sockaddr_in prepServerConnection(struct hostent* server, int portno){

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));  //Inicializa estrutura de comunicação com o server
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    return serv_addr;
}

int writeToSocket(int sockfd, string message){
     fflush(stdin);
    int nMessageLength = message.length();
    int n;
    n = write(sockfd,message.c_str(),nMessageLength); // Envia para o server
    if (n < 0){
        std::cout << "\nERROR writing connected message" << std::endl;
        fflush(stdout);
    }
}


int connectToServer(int portno, string host, string strUserName)
{
  
  bool bTerminate = false;
  int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    string strMessageContent;
    char buffer[BUFFER_SIZE];

    sockfd = createSocket();
    server = getServerInfo(host);
    serv_addr = prepServerConnection(server,portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) //Conecta
        {
        std::cout << "\n ERROR connecting" << std::endl;
        fflush(stdout);
        }
        
    writeToSocket(sockfd,createUserConnectedMessage(strUserName));
 
    while(bTerminate == false)
    {   
        if(strcmp(buffer,"exit") == 0)
        {
            printf("terminate");
            bTerminate = true;
        }
        else
        {  

            writeToSocket(sockfd,createUserMessage(strUserName));
           
        }
    }
    close(sockfd);
    return 0;

}