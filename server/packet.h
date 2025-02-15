#include <iostream>
#include <string>
#include <list>


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
typedef struct __packet{
int nType; // Tipo do pacote (p.ex. DATA | CMD)
int nMessageType; // Tipo da mensagem
int nSeq; // Número de sequência
int nLength; // Comprimento do payload
time_t nTimeStamp; // Timestamp do dado
string strPayload; // Dados da mensagem
string strUserName; // Nome do User
string strGroupName; // Nome do Group
} packet;


/*
typedef struct __conn{
    int nSocket; //número do socket
    string strUsername; // nome do usuário
    string strGroupname; // nome do grupo
}conn;

typedef struct __group{
    string strGroupname; // nome do grupo
    list<string> last_msgs;
} group;
*/
