#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
void error(char *msg)
{
  perror(msg);
  fflush(stdout);
  exit(0);
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


int connectToServer(int portno, char* host)
{
  bool bTerminate = false;
  int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);// Cria Socket
    if (sockfd < 0) 
        {
            error("ERROR opening socket");
            fflush(stdout);
        }
      
    server = gethostbyname(host); // Pega o endereço do Server
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        fflush(stdout);
        exit(0);
    }
      
    bzero((char *) &serv_addr, sizeof(serv_addr));  //Inicializa estrutura de comunicação com o server
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
   
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) //Conecta
        {
        error("ERROR connecting");
        fflush(stdout);
        }
    while(bTerminate == false)
    {       
        printf("\nPlease enter the message: ");
        fflush(stdout);
        bzero(buffer,256);
        fgets(buffer,255,stdin); // Pega a mensagem
        if(strcmp(buffer,"exit") == 0)
        {
            bTerminate = true;
        }
        else
        {
            n = write(sockfd,buffer,strlen(buffer)); // Envia para o server
            if (n < 0) 
                error("ERROR writing to socket");
            bzero(buffer,256);
            n = read(sockfd,buffer,255); // Pega a resposta do server
            if (n < 0) 
                error("ERROR reading from socket");
        }
    }
    close(sockfd);
    return 0;

}