all:
	gcc -c fileShareServer.c
	gcc fileShareServer.o -o fileShareServer
	gcc -c client.c
	gcc client.o -o client
