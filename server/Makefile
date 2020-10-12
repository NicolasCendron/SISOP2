server: server.cpp server_communication_manager.cpp
	g++ -pthread server.cpp server_communication_manager.cpp -std=gnu++11 -o server
server_communication_manager.cpp:
	g++ -o app server_communication_manager.cpp -std=gnu++11
clean:
	rm server
