
void error(char *msg);

int connectToServer(int portno, string host, string userName,string groupName);
void* listenForNewMessages(void *threadarg);
void* sendKeepAlive(void *socket);

