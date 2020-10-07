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
#include <sstream>
#include <ctime>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <unistd.h>
#include <termios.h>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "../utils/functions.h"


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


// ESC-H, ESC-J (I remember using this sequence on VTs)
#define clear() printf("\033[H\033[J")

//ESC-BRACK-column;row (same here, used on terminals on an early intranet)
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

int SOCKET_ID = 0;
string USER_NAME = "";
vector<packet*> arrMessages;



long long getTimeStamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long time_in_mill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
    return time_in_mill;
}

string createUserMessage(string strUserName,string strGroupName){
    char* buffer = (char*)malloc(PROTOCOL_STRING_SIZE);
    printf(" >>>> ");
    fflush(stdout);
    string strUserMessage;
    getline(cin,strUserMessage); // Pega a mensagem

    packet *pack = new packet;

    pack->nMessageType = USER_MESSAGE;
    pack->nTimeStamp = getTimeStamp();
    pack->strPayload =  strUserMessage;
    pack->strUserName = strUserName;
    pack->strGroupName = strGroupName;
    return serializePacket(pack);
}


string createUserConnectedMessage(string strUserName, string strGroupName){

    packet *pack = new packet;
    pack->nMessageType = USER_CONNECTED_MESSAGE;
    //pack->timestamp = getTimeStamp();
    pack->nTimeStamp = getTimeStamp();
    pack->strUserName = strUserName;
    pack->strPayload = strUserName;//strUserName.substr(0,strUserName.length());
    pack->strGroupName = strGroupName;

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

bool compareBySeq(const packet* a, const packet* b)
{
    return a->nTimeStamp < b->nTimeStamp;
}


void printAllMessages()
{   clear();
    std::sort(arrMessages.begin(), arrMessages.end(), compareBySeq);
    for(auto pack: arrMessages){ 
        if(pack->strUserName.compare(USER_NAME) == 0)
        {
            pack->strUserName = "Você";
        }
         
	  if(pack->strUserName.compare(pack->strPayload) == 0 ){
            cout << "[" + pack->strUserName  + "] >> <ENTROU NO GRUPO>" << endl;
            continue;
        }

        cout << "[" + pack->strUserName  + "] >> " + pack->strPayload << endl;

        fflush(stdout);
  } 
}

void handleMessages(packet *pack)
{ 

    
    if (pack->nMessageType == USER_MAX_CONNECTIONS){
        clear();
        cout << "informamos que o número máximo de conexões simultâneas para um mesmo usuário foi atingido" << endl;
        return;
    }
    arrMessages.push_back(pack);
    printAllMessages();
    
}



void* listenForNewMessages(void *threadarg){
    int i = 1;
    clear();
    gotoxy(1,1);
    while(true){   
     if(SOCKET_ID != 0){
        packet *pack  = readFromSocket(SOCKET_ID);
        handleMessages(pack);
        
     }
    }
}

int connectToServer(int portno, string host, string strUserName, string strGroupName)

{
  bool bTerminate = false;
  int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    string strMessageContent;
    char buffer[BUFFER_SIZE];

    sockfd = createSocket();

   /*conn *connection = new conn;
   connection->nSocket = sockfd;
   connection->strGroupname = strGroupName;
   connection->strUsername = strUserName;*/
    


    server = getServerInfo(host);
    serv_addr = prepServerConnection(server,portno);
    SOCKET_ID = sockfd;
    USER_NAME = strUserName;
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) //Conecta
        {
        std::cout << "\n ERROR connecting" << std::endl;
        fflush(stdout);
        }

    writeToSocket(sockfd,createUserConnectedMessage(strUserName,strGroupName));

    while(bTerminate == false)
    {   
        if(strcmp(buffer,"exit") == 0)
        {
            printf("terminate");
            bTerminate = true;
        }
        else
        {   
            writeToSocket(sockfd,createUserMessage(strUserName,strGroupName)); 
            
        }
    }
    close(sockfd);
    return 0;

}