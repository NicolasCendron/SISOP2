
#include <string>
using namespace std;
typedef struct __packet{
int nType; // Tipo do pacote (p.ex. DATA | CMD)
int nMessageType; // Tipo da mensagem
int nSeq; // Número de sequência
int nLength; // Comprimento do payload
float timestamp; // Timestamp do dado
string strPayload; // Dados da mensagem
string strUserName; // Nome do User
} packet;



