#include <stdio.h>
#include <stdlib.h>
#include "client_communication_manager.h"


int main(int argc, char **argv){

    if(argc < 5){
            fprintf(stderr,"Please follow this template.\n ./app <username> <groupname> <server_ip_address> <port>\n");
            exit(1); //No file name
        }

    char *username = argv[1];
    char *groupname = argv[2];
    char *ip = argv[3];
    char *port = argv[4];

    printf("\nuser %s",username);
    printf("\ngroup %s",groupname);
    printf("\nip %s",ip);
    printf("\nport %s",port);

    sendMessage(atoi(port),ip);


}