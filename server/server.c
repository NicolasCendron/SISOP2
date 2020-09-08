#include <stdio.h>
#include <stdlib.h>

#include "server_communication_manager.h"

int main(int argc, char **argv){

    if(argc < 2){
            fprintf(stderr,"Please follow this template.\n ./server <port>\n");
            exit(1); //No file name
        }
    char *port = argv[1];



    startListening(atoi(port));

}