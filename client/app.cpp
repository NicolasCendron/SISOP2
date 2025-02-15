
#include <string>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include "colors.h"
#include <ctime>


using namespace std;
#include "client_communication_manager.h"

int validateStr(string str);
bool is_number(const std::string& s);
int selectPort();


struct thread_data
{
  int thread_id;
  int port;
};


pthread_t listenNewMessagesThread;

int main(int argc, char **argv){

    if(argc < 5){
            fprintf(stderr,"Please follow this template.\n ./app <username> <groupname> <server_ip_address> <port>\n");
            exit(1); //No file name
        }

   /*
    string nomeUsu;
    printf("Informe o seu usuário: \n");
    getline(cin,nomeUsu);
    std::cout << "O usuário escolhido foi: " << nomeUsu << std::endl;

    string nomeGrupo;
    printf("Informe o nome do grupo: \n");
    getline(cin,nomeGrupo);
    std::cout << "O grupo escolhido foi: " << nomeGrupo << std::endl;

*/


    //int numberPort = listAvaiablePorts();
    //std::cout << "Porta escolhida: " << numberPort << std::endl;
    
   // exit(1);

    string username = string(argv[1]);
    string strGroupName = string(argv[2]);
    string ip = string(argv[3]);
    string port = string(argv[4]);


    
    if(validateStr(username) == 0 || validateStr(strGroupName) == 0){
        exit(1);
    }


    //std::cout << " Escolha uma porta:"  << std::endl;
    int numPorta = selectPort();
    //std::cout << " Porta :::" + numPorta  << std::endl;
    //exit(1);


    std::cout << "user: " + username << std::endl;
    std::cout << "group: " + strGroupName << std::endl;
    std::cout << "ip: " + ip << std::endl;
    std::cout << "port: " + to_string(numPorta) << std::endl;



    int rc = pthread_create(&listenNewMessagesThread, NULL, listenForNewMessages, NULL);
    connectToServer(numPorta,ip,username,strGroupName);
    //connectToServer(port,ip,username,strGroupName);

    
}


int listAvaiablePorts(){
    int portNumber;




    return portNumber;
}


/*
 * Função que valida o tamanho da string do nome dos grupos e dos usuários
 * - 1 = retorno correto
 * - 0 = retorno errado
 */
int validateStr(string str){
        
    int errors = 0;
    if(str.length() < 4 ){
        errors++;
        std::cout << RED << "Err: Invalid length (must be greater than 4 characters)" << RESET << std::endl;
        //return 0;
    }

    if(str.length() > 20){
        errors++;
        std::cout << RED << "Err: Invalid length (must be less than 20 characters)" << RESET << std::endl;
        //return 0;
    }

   // std::cout << str << std::endl;
   
    for(int i=0; i< str.length(); i++){
        if( isdigit(str[i]) && i ==0 ){
            errors++;
            std::cout << RED << "Err: First caracter must be a letter  " << RESET << std::endl;
            //return 0;
        }
        if(i > 0 && !isdigit(str[i]) && !isalpha(str[i]) && str[i] != '.'){
            std::cout << RED << "Err: Invalid name (must be without " << str[i] << ") " << RESET << std::endl;
            errors++;
            //return 0;
        }
    }

    if (errors > 0)
    {
        return 0; //teve algum erro no caminho
    }
    

    return 1; //retornou tudo certo
}

