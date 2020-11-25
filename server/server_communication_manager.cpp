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
#include "functions.h"
#include "headers/connection.h"
#include <arpa/inet.h>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <sstream>
#include <fstream>
#include <semaphore.h>
#include <vector>
#include "colors.h"

int NEXT_SEQ = 0;
char buffer[BUFFER_SIZE] = {0};
vector<connection*> arrConnection;


vector<packet*> handleDataBaseMessages();
int arrReplicasNSock[100000];
void error(char *msg) {
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

//sem_t semaforo_server_comm;
//sem_t semaforo_server_comm;

struct thread_data {
    int thread_id;
    int port;
};

int createSocket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Cria socket para escutar um client
    if (sockfd < 0)
        std::cout << RED << "ERROR opening socket" << RESET << std::endl;
    return sockfd;
}

int getNextSeq()
{
  ++NEXT_SEQ;
  return NEXT_SEQ;
}

struct sockaddr_in prepForListening(int portno) {
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr)); // Prepara a estrutura para ouvir o client
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno); //Converte host byte order para network byte order
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    return serv_addr;
} 

void bindToSocket(int sockfd,struct sockaddr_in  serv_addr) {
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){ // Faz o bind na porta
        std::cout << RED << "ERROR on binding" << RESET << std::endl;
    }
}

void printListeningInfo(int portno) {
    std::cout << CYAN << string("\n Server IP:") + string("127.0.0.1 : ") + to_string(portno) << RESET << std::endl;
    fflush(stdout);
}

/*
 * retorna 0: sucesso
 * retorna -1: erro
 * */
int doNothingWhileListen(int sockfd) {
    return listen(sockfd,5); // Escuta a porta até alguem conectar
}

int acceptConnection(int sockfd,struct sockaddr_in cli_addr) {
    socklen_t clilen = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //Aceita a conexão
    if (newsockfd < 0)
        std::cout << RED << "ERROR on accept" << RESET << std::endl;

    return newsockfd;
}

int getMasterDBSocket(){
    ifstream file("database/masterDBPort");
    int nPort;
    if (file >> nPort)
    {
        return arrReplicasNSock[nPort];
    }
    return 0; 
}

void addReplicaNSock(int port, int nSocket){
    arrReplicasNSock[port] = nSocket;
}

void sendMessageToDataBase(packet *pack) {
    std::cout << RED << "BLOQUEANDO:: sendMessageToDataBase" << RESET << std::endl;
    sem_wait(&semaforo_server_comm);
    int nMasterSocket = getMasterDBSocket();
    string mensagemUsuario = serializePacket(pack);       
    if(!mensagemUsuario.empty()) {
        writeToSocket(nMasterSocket,mensagemUsuario); 
    }
    sem_post(&semaforo_server_comm);
    std::cout << GREEN << "LIBERANDO:: sendMessageToDataBase" << RESET << std::endl;
}

vector<packet*> requestMessageHistoryFromDatabase(string strGroupName) {
    ifstream file(strGroupName.c_str());
    char buffer[PROTOCOL_PACKET_SIZE + 1];
    vector<packet*> arrPacks;
    packet *packRequestHistory = new packet();
    packet *packResponse = new packet();

    std::cout << RED << "BLOQUEANDO:: requestMessageHistoryFromDatabase" << RESET << std::endl;
    sem_wait(&semaforo_server_comm);
    int nMasterSocket = getMasterDBSocket();
    packRequestHistory->nMessageType = READ_FROM_FILE;
    packRequestHistory->strGroupName = strGroupName;
    packRequestHistory->strPayload = strGroupName;

    packRequestHistory->nTimeStamp = getTimeStamp();
    packRequestHistory->strUserName = "SERVER";

    string mensagemUsuario = serializePacket(packRequestHistory);       
    if(!mensagemUsuario.empty()) {
        writeToSocket(nMasterSocket,mensagemUsuario); 
    }
   
    arrPacks = handleDataBaseMessages();

    sem_post(&semaforo_server_comm);
    std::cout << GREEN << "LIBERANDO:: requestMessageHistoryFromDatabase" << RESET << std::endl;

    return arrPacks;
}

void sendConnectionFailedMessage(packet *pack, int newsockfd) {
    pack-> nMessageType = USER_MAX_CONNECTIONS;
    writeToSocket(newsockfd,serializePacket(pack));
}


void sendMessageToGroup(packet *pack)
{
  //ADD SEMA
   std::cout << RED << "BLOQUEANDO:: sendMessageToGroup" << RESET << std::endl;
   sem_wait(&semaforo_connections);
  for(auto oConnection: arrConnection){ 
        if(oConnection->strGroupName.compare(pack->strGroupName) == 0)
        {
            writeToSocket(oConnection->nSocket,serializePacket(pack));
        }
  }
  std::cout << GREEN << "LIBERANDO:: sendMessageToGroup" << RESET << std::endl;
   sem_post(&semaforo_connections);
}


void sendMessageHistoryToClient(string strGroupName, int newsockfd) {
    vector<packet*> arrPacks = requestMessageHistoryFromDatabase(strGroupName);

    for(auto pack: arrPacks){ 
        writeToSocket(newsockfd,serializePacket(pack));
    } 
}

bool handleUserConnection(packet *pack, int newsockfd) {
    string strUserName = pack->strUserName;
    string strGroupName = pack->strGroupName;
    int nSocket = newsockfd;

    int nCountUserConnections = 1;

    for(auto connection: arrConnection) { 
        if(pack->strUserName.compare(connection->strUserName) == 0 ) { /// Verifica Numero de Conexoes
            nCountUserConnections++;
        }
    }

    cout << nCountUserConnections << endl;

    if(nCountUserConnections > 2) {
        return false;
    }

    connection *oConn = new connection;
    oConn->strUserName = strUserName;
    oConn->strGroupName = strGroupName;
    oConn->nSocket = newsockfd;

    arrConnection.push_back(oConn); // Computa essa conexão

    return true;
}

packet* removeClientFromConnections(int nSocketDesconectado){
    packet *pack = new packet();
    vector<connection*> newArrConnection;
    for(auto oConnection: arrConnection){ 
        if(oConnection->nSocket != nSocketDesconectado)
        {
            newArrConnection.push_back(oConnection);
        }
        else{
            pack->nMessageType = USER_DISCONNECTED;
            pack->nTimeStamp = getTimeStamp();
            pack->strPayload = oConnection->strUserName;
            pack->strUserName = "SERVER";
            pack->strGroupName = oConnection->strGroupName;
        }
  }
    arrConnection = newArrConnection;
    return pack;
}

vector<packet*> handleDataBaseMessages() {
    vector<packet*> arrPacks;
    int newsockfd = getMasterDBSocket();
    packet *pack;
    bool bConnectionSuccess = true;
    while(bConnectionSuccess) {
        pack = readFromSocket(newsockfd);
        if(pack == NULL) {
            return arrPacks;;
        }
        switch(pack -> nMessageType) {
            case USER_MESSAGE:
                arrPacks.push_back(pack);
                break;
            case MESSAGE_LIST_END:
                return arrPacks;
                break;
            default:
                cout << "Tipo de mensagem não identificado" << endl;

        }
    }
}

int handleUserMessages(int newsockfd) {
    packet *pack;
    bool bConnectionSuccess = true;
    
    while(bConnectionSuccess) {
        pack = readFromSocket(newsockfd);
        if(pack == NULL) {
            pack = removeClientFromConnections(newsockfd);
        }

        if(pack->strPayload.empty() || pack->strPayload.length() == 0) {
            std::cout << RED << "Err: Nada informado" << RESET << std::endl;
            return -1;
        }

        switch(pack -> nMessageType) {
            case USER_MESSAGE:
                sendMessageToDataBase(pack);
                sendMessageToGroup(pack);
                break;

            case USER_CONNECTED_MESSAGE:
                bConnectionSuccess = handleUserConnection(pack, newsockfd);
                if(bConnectionSuccess) {
                    sendMessageToDataBase(pack);
                    sendMessageHistoryToClient(pack->strGroupName,newsockfd);
                    sendMessageToGroup(pack);
                }
                else {
                    sendConnectionFailedMessage(pack,newsockfd);
                }
                break;

            case USER_DISCONNECTED:
                sendMessageToDataBase(pack);
                sendMessageToGroup(pack);
                bConnectionSuccess = false;
                break;

            default:
                cout << "Tipo de mensagem não identificado" << endl;

        }
    }
}

void* startListening(void *threadarg) {
    //requestMessageHistoryFromDatabase(String("group"));
    //return (void*)1;
    std::cout << CYAN << "INIT:: startListening" << RESET << std::endl;
    sem_init(&semaforo_server_comm,0,1);  // nome do semáforo -- 0 pq compartilha o semáforo entre threads (1 é para o caso de compartilhar entre processos) -- num threads simultaneas
    sem_init(&semaforo_server,0,1);
    sem_init(&semaphore_file_port,0,1);
    sem_init(&semaforo_connections,0,1);
    //int cont =0;

    while(true) {
        struct thread_data *my_data;   
        my_data = (struct thread_data *) threadarg;

        int portno = my_data->port;
        int sockfd, newsockfd, clilen;
        struct sockaddr_in serv_addr, cli_addr;

        sockfd = createSocket();
        serv_addr = prepForListening(portno);
        bindToSocket(sockfd,serv_addr);
        printListeningInfo(portno);

        if(doNothingWhileListen(sockfd) < 0) {
            std::cout << RED << "Err: not listening" << RESET << std::endl;
            exit(1);
        }

        newsockfd = acceptConnection(sockfd, cli_addr);
        int ret = handleUserMessages(newsockfd);

        if(ret == -1) {
            break;
        }
    }


  sem_destroy(&semaforo_server);
  sem_destroy(&semaphore_file_port);
  sem_destroy(&semaforo_server_comm);
  sem_destroy(&semaforo_connections);

}