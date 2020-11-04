#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
//#include <curses.h>
#include <semaphore.h>
#include "../headers/server_communication_manager.h"
#include "../packet.h"
#include "../functions.h"
#include <iostream>
#include <fstream>
#include <string>
#include <netinet/in.h>
#include <netdb.h>
#include <inttypes.h>
#include <algorithm> 
#include <arpa/inet.h>
#include <locale>
#include <termios.h>
#include <cctype>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
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
#include <termios.h>
#include <semaphore.h>
#include <vector>
#include <mutex>
#include <condition_variable>
using namespace std;

struct thread_data
{
  int thread_id;
  int port;
};

int MAIN_REPLICA = 0;

sem_t semaphore_server;

pthread_t listeningThreads[NUMBER_OF_REPLICAS + 1];
struct thread_data td[NUMBER_OF_REPLICAS + 1];
vector<int> vecReplicas;

 // *************** Comunicação Com o Front End ***************

int createSocket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Cria socket para escutar um client
    if (sockfd < 0)
        std::cout << RED << "ERROR opening socket" << RESET << std::endl;
    return sockfd;
}

void bindToSocket(int sockfd,struct sockaddr_in  serv_addr) {
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){ // Faz o bind na porta
        std::cout << RED << "ERROR on binding" << RESET << std::endl;
    }
}

struct sockaddr_in prepForListening(int portno) {
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr)); // Prepara a estrutura para ouvir o client
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno); //Converte host byte order para network byte order
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    return serv_addr;
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

void writeMessageToFile(packet *pack) {
    std::cout << RED << "BLOQUEANDO:: writeMessageToFile" << RESET << std::endl;
    //sem_wait(&semaforo_replica_comm);
    std::ofstream outfile;
    for (int cont = 0; cont < NUMBER_OF_REPLICAS; cont ++){
        string strPath = "db_" + to_string(INITIAL_PORT_REPLICA + cont) + "_" + pack->strGroupName;
        cout << "path:" << strPath <<endl;
        outfile.open(strPath, std::ios_base::app); // append instead of overwrite
        outfile << serializePacket(pack);
    }
    //sem_post(&semaforo_replica_comm);
    std::cout << GREEN << "LIBERANDO:: writeMessageToFile" << RESET << std::endl;
}

vector<packet*> sendEntireFile(string strGroupName, int nSocket) {

    ifstream file("db_" + to_string(MAIN_REPLICA) + "_" + strGroupName.c_str());
    char buffer[PROTOCOL_PACKET_SIZE + 1];
    vector<packet*> arrPacks;
    packet *pack = new packet();

    std::cout << RED << "BLOQUEANDO:: sendEntireFile" << RESET << std::endl;
    sem_wait(&semaforo_replica_comm);

    while (file.read(&buffer[0], PROTOCOL_PACKET_SIZE)) {
        pack = deserializePacket(string(buffer));
        arrPacks.push_back(pack);
    }

    string strMessage;
    for(auto pack : arrPacks){
        strMessage = serializePacket(pack);
        writeToSocket(nSocket,strMessage); 
    }
    pack->nMessageType = MESSAGE_LIST_END;
    strMessage = serializePacket(pack);
    writeToSocket(nSocket,strMessage); 

    sem_post(&semaforo_replica_comm);
    std::cout << GREEN << "LIBERANDO:: sendEntireFile" << RESET << std::endl;

    return arrPacks;
}

int handleRequisitions(int newsockfd) {
    packet *pack;
    bool bConnectionSuccess = true;
    while(bConnectionSuccess) {
        pack = readFromSocket(newsockfd);

        if(pack->strPayload.empty() || pack->strPayload.length() == 0) {
            std::cout << RED << "Err: Nada informado" << RESET << std::endl;
            return -1;
        }

        switch(pack -> nMessageType) {
            case READ_FROM_FILE:
                cout << "Recuperando mensagens do grupo" << pack->strGroupName <<endl;
                sendEntireFile(pack->strGroupName,newsockfd);
                break;
            case USER_MESSAGE:
            case WRITE_TO_FILE:
            case USER_CONNECTED_MESSAGE:
            case USER_DISCONNECTED:
                cout << "Salvando mensagem do usuário " << pack->strUserName <<endl;
                writeMessageToFile(pack);
                break;

            default:
                cout << "Tipo de mensagem não identificado" << endl;

        }
    }
}

void* startListeningFront(void *threadarg) {
    MAIN_REPLICA = INITIAL_PORT_REPLICA;
    //vecReplicas.push_back(((struct thread_data *) threadarg)->port);
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
        cout << "Aguardando Conexão" <<endl;
        if(doNothingWhileListen(sockfd) < 0) {
            std::cout << RED << "Err: not listening" << RESET << std::endl;
            exit(1);
        }

        newsockfd = acceptConnection(sockfd, cli_addr);
        int ret = handleRequisitions(newsockfd);
        if(ret == -1) {
            break;
        }
    }

}

// *********** MAIN() *******************************************************************

int connectToSibling(int portno, string host);
struct sockaddr_in prepSiblingConnection(struct hostent* server, int portno);
void* startListeningSiblings(void *threadarg);
struct hostent * getServerInfo(string host);

int main(int argc, char **argv){
    sem_init(&semaforo_replica_comm,0,1);

    int myPort = atoi(argv[1]);
    
    td[0].thread_id = 0;
    td[0].port = myPort;
    pthread_create(&listeningThreads[0], NULL, startListeningFront, (void *)&td[0]);
    int last_port,next_port,i,rc;
    
    if(myPort == INITIAL_PORT_REPLICA){
        last_port = INITIAL_PORT_REPLICA + NUMBER_OF_REPLICAS - 1;
        next_port = myPort + 1;
    }
    else if(myPort == INITIAL_PORT_REPLICA + NUMBER_OF_REPLICAS - 1){
        last_port = myPort - 1;
        next_port = INITIAL_PORT_REPLICA;
    }
    else{
        last_port = myPort - 1;
        next_port = myPort + 1;
    }

    td[1].thread_id = 1;
    td[1].port = next_port;
    rc = pthread_create(&listeningThreads[i], NULL, startListeningSiblings, (void *)&td[1]);
    if (rc){

        printf("Error:unable to create thread: %d",rc);
        fflush(stdout);
        exit(-1);    
    }    
    connectToSibling(last_port,"127.0.0.1");
    
    sem_destroy(&semaforo_replica_comm);
    pthread_exit(NULL); 
}

/// *********** COMUNICAÇÃO COM OS IRMÃOS

void* startListeningSiblings(void *threadarg) {
    MAIN_REPLICA = INITIAL_PORT_REPLICA;
    vecReplicas.push_back(((struct thread_data *) threadarg)->port);
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
        cout << "Aguardando Conexão" <<endl;
        if(doNothingWhileListen(sockfd) < 0) {
            std::cout << RED << "Err: not listening" << RESET << std::endl;
            exit(1);
        }

        newsockfd = acceptConnection(sockfd, cli_addr);
        int ret = handleRequisitions(newsockfd);
        if(ret == -1) {
            break;
        }
    }
}

int connectToSibling(int portno, string host) {
    bool bTerminate = false;
    int sockfd, n;
    struct sockaddr_in serv_addr; // specifies a transport address and port for the AF_INET address family

    struct hostent *server; //used by functions to store information about a given host, such as host name, IPv4 address
    string strMessageContent;
    char buffer[BUFFER_SIZE];

    sockfd = createSocket();
  
    server = getServerInfo(host); //127.0.0.1 
    serv_addr = prepSiblingConnection(server,portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) //Conecta
        {
        std::cout << RED << "\n ERROR connecting" << RESET << std::endl;
        fflush(stdout);
        }

    /**
     *  The sem_destroy() function destroys the unnamed semaphore specified by the sem argument. You can only use this 
     * function to destroy a semaphore created with the sem_init() function. Additionally, subsequent use of the specified 
     * semaphore by another semaphore function results in that function failing with errno set to EINVAL. Reinitializing the
     * semaphore with the sem_init() function allows it to be used again.  
     **/
       
    //editPort(portno, 0);

    //writeToSocket(sockfd,createUserConnectedMessage(strUserName,strGroupName));

    while(bTerminate == false) {   
        if(strcmp(buffer,"exit") == 0) {
            printf("terminate");
            bTerminate = true;
        }
        else {   
            string mensagemUsuario ;//createUserMessage(strUserName,strGroupName);
            
            if(!mensagemUsuario.empty()) {
                writeToSocket(sockfd,mensagemUsuario); 
            }
            else {
                std::cout << RED << "Err: Nada informado" << RESET << std::endl;
            }  
        }
    }
    close(sockfd);
    return 0;
}

struct hostent * getServerInfo(string host) {
    struct hostent *server;
    server = gethostbyname(host.c_str()); // Pega o endereço do Server
    if (server == NULL) {
        std::cout << RED << "ERROR, no such host" << RESET << std::endl;
        fflush(stdout);
        exit(0);
    }
}

struct sockaddr_in prepSiblingConnection(struct hostent* server, int portno) {
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));  //Inicializa estrutura de comunicação com o server
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    return serv_addr;
} 

