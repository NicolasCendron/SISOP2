#include <stdio.h>
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
void error(char *msg)
{
  perror(msg);
  fflush(stdout);
  exit(-1);
}

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
typedef int bool;
#define true 1
#define false 0
#define BUFFER_SIZE 2560
#define USER_MESSAGE 1000
#define USER_CONNECTED_MESSAGE 1001

char* getMessageNameByType(int msgType){

        switch (msgType)
        {
        case USER_MESSAGE:
            return "Mensagem de Texto";
        case USER_CONNECTED_MESSAGE:
            return "Usuário Conectado";
        default:
            return "";
        }
    
}

void printPacket(packet * pack){
    printf("\n Packet do Cliente");
    printf("\n Tipo de Mensagem: %s",getMessageNameByType(pack->messageType));
    printf("\n Usuário: %s",pack->_userName);
    printf("\n Texto da mensagem: %s",pack->_payload);
    printf("\n Tamanho da mensagem: %d",pack->length);
    printf("\n Timestamp: %f",pack->timestamp);
}

double getTimeStamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double time_in_mill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
    printf("%f",time_in_mill);
    return time_in_mill;
}

packet* createUserMessage(char* buffer, char* username){
    printf("\nPlease enter the message: ");
    fflush(stdout);
    bzero(buffer,BUFFER_SIZE);
    fgets(buffer,BUFFER_SIZE-1,stdin); // Pega a mensagem
       
    packet *pack = (packet*)malloc(sizeof(packet));
    pack->messageType = USER_MESSAGE;
    pack->timestamp = getTimeStamp();
    pack->length = strlen(buffer);
    pack->_payload = buffer;
    pack->_userName = username;
    return pack;
}

packet* createUserConnectedMessage(char* username){
    packet *pack = (packet*)malloc(sizeof(packet));
    pack->messageType = USER_CONNECTED_MESSAGE;
    pack->timestamp = getTimeStamp();
    pack->length = strlen(username);
    pack->_payload = username;
    pack->_userName = username;
    return pack;
}

int createSocket(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);// Cria Socket
    if (sockfd < 0) 
        {
            error("ERROR opening socket");
            fflush(stdout);
        }
        return sockfd;
}

struct hostent * getServerInfo(char* host){
    struct hostent *server;
    server = gethostbyname(host); // Pega o endereço do Server
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
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

int connectToServer(int portno, char* host, char* username)
{
  bool bTerminate = false;
  int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFFER_SIZE];

    sockfd = createSocket();
      
    server = getServerInfo(host);
      
    serv_addr = prepServerConnection(server,portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) //Conecta
        {
        printf("ERROR connecting");
        fflush(stdout);
        }
    
    packet *pack = createUserConnectedMessage(username);
    printPacket(pack); 
    n = write(sockfd,pack,sizeof(pack)); // Envia para o server
    if (n < 0){
        printf("ERROR writing connected message");
        fflush(stdout);
    }
    while(bTerminate == false)
    {   
        pack = createUserMessage(buffer,username);
        printPacket(pack);  
        if(strcmp(buffer,"exit") == 0)
        {
            printf("terminate");
            bTerminate = true;
        }
        else
        {
            n = write(sockfd,pack,sizeof(pack)); // Envia para o server
            if (n < 0) 
                error("ERROR writing to socket");
            bzero(buffer,BUFFER_SIZE);
            n = read(sockfd,buffer,BUFFER_SIZE); // Pega a resposta do server
            if (n < 0) 
                error("ERROR reading from socket");
        }
    }
    close(sockfd);
    return 0;

}