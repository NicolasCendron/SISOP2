#include <iostream>
#include <string>

//the following are UBUNTU/LINUX, and MacOS ONLY terminal color codes.
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

using namespace std;
#include "client_communication_manager.h"

int validateStr(string str);
bool is_number(const std::string& s);

int main(int argc, char **argv){

    if(argc < 5){
            fprintf(stderr,"Please follow this template.\n ./app <username> <groupname> <server_ip_address> <port>\n");
            exit(1); //No file name
        }

    string username = string(argv[1]);
    string groupname = string(argv[2]);
    string ip = string(argv[3]);
    string port = string(argv[4]);
    
    //int validou = validateStr(username);
    // int validouGroupName = validateStr(groupname);

    
    //exit(1);

    /*if(validateStr(username) && !validateStr(groupname)){

    }*/


    std::cout << "user: " + username << std::endl;
    std::cout << "group: " + groupname << std::endl;
    std::cout << "ip: " + ip << std::endl;
    std::cout << "port: " + port << std::endl;

    connectToServer(stoi(port),ip,username);

    
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
        return 0;
    }
    

    return 1;
}

