#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <sstream>
#include <fstream>
#include <vector>
#include <ctime>
#include<semaphore.h>


//#include <termios.h>
#include "../client/colors.h"



#define BUFFER_SIZE 2560
#define USER_MESSAGE 1000
#define USER_CONNECTED_MESSAGE 1001
#define USER_DISCONNECTED 1002
#define USER_EXIT_GROUP 1003
#define USER_MAX_CONNECTIONS 1004
#define ASK_SEQ 1005
#define PROTOCOL_INT_SIZE 10
#define PROTOCOL_STRING_SIZE 140
#define PROTOCOL_LONG_SIZE 20
#define PROTOCOL_PACKET_SIZE 3*PROTOCOL_STRING_SIZE + PROTOCOL_INT_SIZE + PROTOCOL_LONG_SIZE

sem_t semaphore_file_port;


int writeToSocket(int sockfd, string message){
    
    

    fflush(stdin);
    int nMessageLength = message.length();
    int n;
    n = write(sockfd,message.c_str(),nMessageLength); // Envia para o server
    if (n < 0){
        std::cout << "\nERROR writing connected message" << std::endl;
        fflush(stdout);
    }
       
    

     return 0;
}

time_t getTimeStamp(){
    //struct timeval tv;
    //gettimeofday(&tv, NULL);
    //long long time_in_mill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;

    time_t now = time(0);
    tm *time_now = localtime(&now);

    return now;
}

string timestamp_to_date(time_t timestamp_packet){
    tm *time_now = localtime(&timestamp_packet);
    char buf[80];

    /*
    //std::cout << "timestamp_na funcao:::: " << timestamp_packet << std::endl;
    
    //time(&timestamp_packet);
    //timeinfo = localtime(&timestamp_packet);


    //time_t now;
    //time(&timestamp_packet);
    //ts = *localtime(&timestamp_packet);
    
    //strftime(buf, sizeof(buf),"%H:%M:%S",&ts);

    //sprintf(buf,"")

    //std::time_t test = 1439467747492;
    //std::cout << std::ctime(&test);
    //std::cout << "ctime:::: " << ctime(&timestamp_packet) << std::endl;
    
    //return ctime(&timestamp_packet);
    */
    string horas = to_string(time_now->tm_hour);
    string horasCompletas="0";
    if(horas.length() == 1){
        horasCompletas.append(horas);
    }else{
        horasCompletas = horas;
    }


    string minutos = to_string(time_now->tm_min);
    string minutosCompletos="0";
    if(minutos.length() == 1){
        minutosCompletos.append(minutos);
    }else{
        minutosCompletos = minutos;
    }

    string segundos = to_string(time_now->tm_sec);
    string segundosCompletos="0";
    if(segundos.length() == 1){
       segundosCompletos.append(segundos);
    }else{
        segundosCompletos = segundos;
    }
    

    
    return horasCompletas+":"+minutosCompletos+":"+segundosCompletos;
}



/*  PACKETS  */
void printPacket(packet * pack){
  std::cout << "\nPacket do Cliente" << std::endl;
  std::cout << "Tipo de Mensagem: " + to_string(pack->nMessageType)  << std::endl;
  std::cout << "Numero de Sequencia: " + to_string(pack->nSeq)  << std::endl;
  std::cout << "Texto da mensagem: " + pack->strPayload  << std::endl;
  std::cout << "Nome Usuário: " + pack->strUserName  << std::endl;
  std::cout << "Nome Grupo: " + pack->strGroupName  << std::endl;
  //std::cout << "Timestamp: " + to_string(pack->nTimeStamp)  << std::endl;
  std::cout << timestamp_to_date(pack->nTimeStamp)  << std::endl;
 
}





void delete_line(const char *file_name, int n) 
{ 
    // open file in read mode or in mode 
    ifstream is(file_name); 
  
    // open file in write mode or out mode 
    ofstream ofs; 
    ofs.open("temp.txt", ofstream::out); 
  
    // loop getting single characters 
    char c; 
    int line_no = 0; 
    while (is.get(c)) 
    { 
        // if a newline character 
        if (c == '\n') 
        line_no++; 
  
        // file content not to be deleted 
        if (line_no != n) 
            ofs << c; 
    } 

     
    // closing output file 
    ofs.close(); 
  
    // closing input file 
    is.close(); 
  
    // remove the original file 
    remove(file_name); 
  
    // rename the file 
    rename("temp.txt", file_name); 
} 

void blank_line(const char *file_name)
{   
  ifstream fin(file_name);    
  
  ofstream fout;                
  fout.open("temp.txt", ios::out);
  
  string str;
  while(getline(fin,str))
  { 
    while (str.length()==0 ) 
       getline(fin,str);   
  
    fout<<str<<endl;
  }
  fout.close();  
  fin.close();  
  remove(file_name);        
  rename("temp.txt", file_name);
}

int showPortsFile(){
    string line;
    ifstream myfile;

    vector<int> portas;
    myfile.open( "../utils/portas.txt");

    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            //cout << line << '\n';  
            std::size_t pos = line.find(";");      // position of "live" in str
            std::string porta = line.substr (0,pos);

            std::string status = line.substr ((pos+1), line.length());

            //cout << porta << '\n';
            //cout << status << '\n';
            
            if (status.compare(0,1,"0") == 0){
                std::cout << "Porta: " << porta + " > " << RED << "OCUPADA" << RESET << std::endl;
            }else{
                portas.push_back(stoi(porta));
                std::cout << "Porta: " << porta + " > " << GREEN << "LIVRE" << RESET << std::endl;
            }

        }
        
        myfile.close();
    }
    else{
        //cout << "Unable to open file";
        std::cout << RED << "Err: Unable to open file" << RESET << std::endl;
        exit(1);
    }


    if(portas.size() > 0){
        int numPorta;
        bool encontrou = false;
        while(!encontrou){
            printf("Escolha uma porta livre: \n");
            scanf("%d",&numPorta);
            printf("A porta escolhida foi grupo escolhido foi: %d \n", numPorta);

            for (auto i = portas.begin(); i != portas.end(); ++i) {
                if(numPorta == *i){   
                    encontrou = true;
                    //cout << *i << " "; 
                }
            }

            if(!encontrou){
                std::cout << YELLOW << "Warn: Informe uma porta liberada" << RESET << std::endl;
            }
        }

        if(encontrou){
            return numPorta;
        }
    }else{
        std::cout << RED << "Err: Não existem mais portas livres" << RESET << std::endl;
    }

    return -1;
 

}

/**
 * status = 1  --> então  a porta ta liberada
 * status = 0  --> então  a porta ta ocupada
 */
void editPort(int numPorta, int status){
    int linha=0;
    int escolhida = -1;
    string line;
    ifstream myfile;
    ofstream editFile;
    myfile.open( "../utils/portas.txt");
    editFile.open( "../utils/portas.txt", fstream::in | fstream::out | fstream::ate | ios::app);
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            std::size_t pos = line.find(";");  
            if(pos > 0){
                std::string porta = line.substr (0,pos);
                if( stoi(porta) == numPorta){
                    if(line.find(porta)!=string::npos){
                        editFile << porta+";" + to_string(status) +";\n";
                        escolhida = linha;            
                    }
                }
                linha++;
            }         
        }
        
        myfile.close();
    }
    else{
        //cout << "Unable to open file";
        std::cout << RED << "Err: Unable to open file" << RESET << std::endl;
        exit(1);
    }
    editFile.close();
    //exit(1);
    if(escolhida != -1){
        //semaphore_file_port.wait()
        delete_line("../utils/portas.txt",escolhida);
        blank_line("../utils/portas.txt");
    }

   
}

string padLeft(string strOld,char cPad,int nSize){
    return std::string(nSize - strOld.length(), cPad) + strOld;
}

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

packet* deserializePacket(string strPack)
{   int nPointer = 0;
    packet *pack = new packet;
    string strBuff;
    try{
        std::istringstream( strPack.substr(nPointer,PROTOCOL_INT_SIZE) ) >> pack->nMessageType; nPointer+=PROTOCOL_INT_SIZE;
        std::istringstream( strPack.substr(nPointer,PROTOCOL_LONG_SIZE) ) >> pack->nTimeStamp; nPointer+=PROTOCOL_LONG_SIZE;
        pack->strPayload = strPack.substr(nPointer,PROTOCOL_STRING_SIZE);ltrim( pack->strPayload ) ;nPointer+=PROTOCOL_STRING_SIZE;
        pack->strUserName = strPack.substr(nPointer,PROTOCOL_STRING_SIZE); ltrim(pack->strUserName) ; nPointer+=PROTOCOL_STRING_SIZE;
        pack->strGroupName = strPack.substr(nPointer,PROTOCOL_STRING_SIZE); ltrim(pack->strGroupName) ; nPointer+=PROTOCOL_STRING_SIZE;
    }
    catch(...){
        cout << "\n\n\n----------- Entrou no Cacth (Ajustar) ------------ " << endl;
        pack->nMessageType = USER_DISCONNECTED;
        pack->nTimeStamp = getTimeStamp();
        pack->strPayload = "Alguém saiu";
        pack->strUserName = "Não sei quem";
        pack->strGroupName = "group";
    }
    return pack;
}

string serializePacket(packet * pack)
{
    string serialized;
    serialized = serialized + padLeft(to_string(pack->nMessageType),'0',PROTOCOL_INT_SIZE);
    serialized = serialized + padLeft(to_string(pack->nTimeStamp),'0',PROTOCOL_LONG_SIZE);
    serialized = serialized + padLeft(pack->strPayload,' ',PROTOCOL_STRING_SIZE);
    serialized = serialized + padLeft(pack->strUserName,' ',PROTOCOL_STRING_SIZE);
    serialized = serialized + padLeft(pack->strGroupName,' ',PROTOCOL_STRING_SIZE);
    //std::cout << "**" +  serialized + "&&" << std::endl;
    return serialized;
}


packet* readFromSocket(int newsockfd){
    char * buffer = (char*)malloc(PROTOCOL_PACKET_SIZE);

    int n = read(newsockfd,buffer,PROTOCOL_PACKET_SIZE);
    if (n < 0) 
      std::cout << RED << "ERROR reading from socket" << RESET << std::endl;
  
    fflush(stdout);
    
    packet * pack = deserializePacket(string(buffer));
    return pack;
}