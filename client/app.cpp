#include <iostream>
#include <string>
using namespace std;
#include "client_communication_manager.h"


int main(int argc, char **argv){

    if(argc < 5){
            fprintf(stderr,"Please follow this template.\n ./app <username> <groupname> <server_ip_address> <port>\n");
            exit(1); //No file name
        }

    string username = string(argv[1]);
    string groupname = string(argv[2]);
    string ip = string(argv[3]);
    string port = string(argv[4]);

    std::cout << "\nuser " + username << std::endl;
    std::cout << "\ngroup " + groupname << std::endl;
    std::cout << "\nip " + ip << std::endl;
    std::cout << "\nport " + port << std::endl;

    connectToServer(stoi(port),ip,username);

    
}