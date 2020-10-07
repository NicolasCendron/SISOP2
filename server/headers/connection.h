
#include <string>
using namespace std;
typedef struct __connection{
int nSocket; // Tipo do pacote (p.ex. DATA | CMD)
string strUserName; // Nome do User
string strGroupName; // Nome do Group
} connection;
