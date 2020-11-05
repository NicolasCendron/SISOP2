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
#include <chrono>
#include <thread>
using namespace std;

struct thread_data
{
  int thread_id;
  int port;
};

int MAIN_REPLICA_PORT = 0;
int MAIN_REPLICA_ALIVE = 1;
int SIMULATE_ONE_DEATH = 1;
int SIMULATE_ALL_DEATHS = 0;
int VOTING_STARTED = 0;
int VOTE_STARTER = 0;
int NEXT_TO_VOTE = 0;
int BEST_CANDIDATE = 0;
int MAX_VOTING_CHANCES = 3;
int VOTING_CHANCES_USED = 0;
sem_t semaphore_server;



pthread_t listeningThreads[NUMBER_OF_REPLICAS];
pthread_t votingThreads[NUMBER_OF_REPLICAS];
struct thread_data td[NUMBER_OF_REPLICAS];
struct thread_data tdVote[NUMBER_OF_REPLICAS];
vector<int> vecReplicas;
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

    ifstream file("db_" + to_string(MAIN_REPLICA_PORT) + "_" + strGroupName.c_str());
    char buffer[PROTOCOL_PACKET_SIZE + 1];
    vector<packet*> arrPacks;
    packet *pack = new packet();

    std::cout << RED << "BLOQUEANDO:: sendEntireFile" << RESET << std::endl;
    //sem_wait(&semaforo_replica_comm);

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
        if(pack == NULL) {
            close(newsockfd);
        }

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

void erasePreviousVotes(){
    sem_wait(&semaforo_replica_comm);
    std::ofstream leadingCandidate;
    leadingCandidate.open("database/leadingCandidate", std::ios_base::out);
    leadingCandidate << to_string(0);
    sem_post(&semaforo_replica_comm);
}

void putKingOnTheThrone(){
        cout << YELLOW <<  "KING ON THE THRONE" << endl;
        std::ofstream masterDBPort;
        masterDBPort.open("database/masterDBPort", std::ios_base::out); 
        masterDBPort << to_string(MAIN_REPLICA_PORT);
        masterDBPort.close();
}

void vote(int myPort){
    //ifstream file("database/leadingCandidate");
    //int nOldCandidatePort;
    // if (!file >> nOldCandidatePort)
    // {
    //      nOldCandidatePort = 0;
    // }

    BEST_CANDIDATE = BEST_CANDIDATE >= myPort ? BEST_CANDIDATE : myPort;
    // std::ofstream leadingCandidate;
    // leadingCandidate.open("database/leadingCandidate", std::ios_base::out);
    // leadingCandidate << to_string(newCandidatePort);

    // file.close();
}

int getWinner(){

    // ifstream file("database/leadingCandidate");
    // int nWinnerPort;
    // if (!(file >> nWinnerPort))
    // {
    //      nWinnerPort = 0;
    // }
    // file.close();
    return BEST_CANDIDATE;
}

int getNextVoter(int port){

    if(port == INITIAL_PORT_REPLICA + NUMBER_OF_REPLICAS - 1){
        return INITIAL_PORT_REPLICA;
    }
    return port + 1;
}

void* checkIfKingAlive(void *threadarg){
    struct thread_data *my_data;   
    my_data = (struct thread_data *) threadarg;
    int myPort = my_data->port;
    sem_wait(&semaforo_replica_comm);
    sem_post(&semaforo_replica_comm);
    while (true)
    {   
        //cout << YELLOW << "MY PORT " << myPort << endl;
        if(myPort == MAIN_REPLICA_PORT && MAIN_REPLICA_ALIVE == 1){   /// ESTE É o REI
            if(SIMULATE_ONE_DEATH || SIMULATE_ALL_DEATHS){  // Força o REI A MORRER
            sem_wait(&semaforo_replica_comm);
                SIMULATE_ONE_DEATH = 0;
                MAIN_REPLICA_ALIVE = 0;
                cout << RED  << "THE KING: " << MAIN_REPLICA_PORT << " IS DEAD !" <<endl;
            sem_post(&semaforo_replica_comm);
            break;
            }
        }
        else{
            if(MAIN_REPLICA_ALIVE == 0 && VOTING_STARTED == 0){ // INICIA A VOTAÇÃO SE O REI ESTIVER MORTO
                //sem_wait(&semaforo_replica_comm);
                VOTING_STARTED = 1;
                VOTE_STARTER = myPort;
                cout << CYAN << "ELECTION STARTED !" << " STARTER = " << VOTE_STARTER <<endl;
                erasePreviousVotes();
                NEXT_TO_VOTE = getNextVoter(myPort);
                cout << WHITE << "IS VOTING: <" << myPort << "> NEXT TO VOTE = " << NEXT_TO_VOTE <<endl;
                vote(myPort);
                //sem_post(&semaforo_replica_comm);
            }
            else if(VOTING_STARTED == 1 && myPort == NEXT_TO_VOTE && myPort == VOTE_STARTER ){ // QUANDO CHEGAR DE VOLTA NO CARA QUE INICIOU A VOTAÇÃO, Decide-se o rei
                sem_wait(&semaforo_replica_comm);
                MAIN_REPLICA_PORT = getWinner();
                cout << GREEN << "THE NEW KING IS = " << MAIN_REPLICA_PORT << " LONG SHALL HE LIVE" << endl;
                putKingOnTheThrone();
                MAIN_REPLICA_ALIVE = 1;
                VOTING_STARTED = 0;
                sem_post(&semaforo_replica_comm);
            }
            else if(VOTING_STARTED == 1 && myPort == NEXT_TO_VOTE){  // O PROXIMO VOTA
                sem_wait(&semaforo_replica_comm);
                cout << WHITE  << "IS VOTING: <" << myPort << "> NEXT TO VOTE = " << NEXT_TO_VOTE <<endl;
                vote(myPort);
                NEXT_TO_VOTE = getNextVoter(myPort);
                sem_post(&semaforo_replica_comm);
            }
            else{ // TRATA o caso do próximo a votar não estar respondendo
                sem_wait(&semaforo_replica_comm);
                VOTING_CHANCES_USED += 1;
                if(VOTING_CHANCES_USED >= MAX_VOTING_CHANCES){
                    NEXT_TO_VOTE = getNextVoter(NEXT_TO_VOTE);
                }
                sem_post(&semaforo_replica_comm);
            }
            
        }
        usleep(2000000);
        
    }

}

void* startListening(void *threadarg) {
    
    //vecReplicas.push_back(((struct thread_data *) threadarg)->port);
    while(true) {
        struct thread_data *my_data;   
        my_data = (struct thread_data *) threadarg;

        int portno = my_data->port;
        int sockfd, newsockfd, clilen;
        struct sockaddr_in serv_addr, cli_addr;

        sockfd = createSocket();
        if(sockfd < 0){
            std::cout << RED << "Error on Binding" << RESET << std::endl;
        }
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


int main(int argc, char **argv){
   // Porta  lista_portas[NUMBER_OF_REPLICAS];
    MAIN_REPLICA_PORT = INITIAL_PORT_REPLICA;
    putKingOnTheThrone();
    sem_init(&semaforo_replica_comm,0,1);
    ofstream myfile;
    myfile.open ("../utils/portas.txt");
  
    int i,rc;
    for( i=0; i < NUMBER_OF_REPLICAS; i++ )    
    {
        td[i].thread_id = i;
        tdVote[i].thread_id = i;
        td[i].port = INITIAL_PORT_REPLICA + i;
        tdVote[i].port = INITIAL_PORT_REPLICA + i;

        rc = pthread_create(&listeningThreads[i], NULL, startListening, (void *)&td[i]);
        rc = pthread_create(&votingThreads[i], NULL, checkIfKingAlive, (void *)&tdVote[i]);

        if (rc){

            printf("Error:unable to create thread: %d",rc);
            fflush(stdout);
            exit(-1);    
        }    
    } 
    //exit(1);
    sem_destroy(&semaforo_replica_comm);
    pthread_exit(NULL); 
}



