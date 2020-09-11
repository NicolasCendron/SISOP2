server: server.c server_communication_manager.c
	gcc -pthread server.c server_communication_manager.c -o server
server_communication_manager.c:
	gcc -o app server_communication_manager.c
clean:
	rm server
