
#include <string>
using namespace std;
typedef struct __packet{
int nType; // Tipo do pacote (p.ex. DATA | CMD)
int nMessageType; // Tipo da mensagem
int nSeq; // Número de sequência
int nLength; // Comprimento do payload
long long nTimeStamp; // Timestamp do dado
string strPayload; // Dados da mensagem
string strUserName; // Nome do User
string strGroupName; //Nome do Group
} packet;




typedef struct __conn{
    int nSocket; //número do socket
    string strUsername; // nome do usuário
    string strGroupname; // nome do grupo
}conn;