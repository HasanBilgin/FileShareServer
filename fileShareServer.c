
#include <sys/types.h>         
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <dirent.h>


char pcClients[256][1000];
pid_t pClientPID[256];
int iSocketServer;
void interruptHandler(int);

int main(int argc, char **argv)
{
	if (argc != 2)	{
		fprintf(stderr, "USAGE\n");
		fprintf(stderr, "./fileShareServer port\n");
		exit(1);
	}
	int iAccept;      
	struct sockaddr_in server,client;  
	int iRet;
	int iAddrLen;
	int iRecvLen;
	char ucRecvBuf[1000];

	int iNumberOfClient = -1;        

	signal(SIGCHLD, SIG_IGN);
	signal(SIGINT,interruptHandler);
	fprintf(stderr, "Program basladi...\n" );
	iSocketServer = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == iSocketServer)
	{
		printf("socket error!\n");
		return -1;
	}
	server.sin_family = AF_INET;
	/* (htons: host to net, short)*/
	server.sin_port   = htons(atoi(argv[1]));    

	server.sin_addr.s_addr = INADDR_ANY;      

	memset(server.sin_zero, 0, 8);
	
	iRet = bind(iSocketServer, (const struct sockaddr *)&server, sizeof(struct sockaddr));
	if (-1 == iRet)
	{
		printf("bind error!\n");
		return -1;
	}

	iRet = listen(iSocketServer, 20);
	if (-1 == iRet)
	{
		printf("listen error!\n");
		return -1;
	}

	while (1)
	{
		iAddrLen = sizeof(struct sockaddr);
		iAccept = accept(iSocketServer, (struct sockaddr *)&client, &iAddrLen);
		if (iAccept != -1)
		{
			iNumberOfClient++;
			
			//printf("Client baglandi %d : %s\n", iNumberOfClient, inet_ntoa(client.sin_addr));
			do{
				iRecvLen = recv(iAccept, ucRecvBuf, 999, 0);
				strcpy(pcClients[iNumberOfClient],ucRecvBuf);
			}while (iRecvLen == 0);
	
			pid_t process;
			process  = fork();
			if (process == -1 )	{
				perror ("Fork failed !!!");
				exit(1);
			}
			pClientPID[iNumberOfClient] = getpid();
			if (process == 0 )   
			{
				while (1)
				{

					iRecvLen = recv(iAccept, ucRecvBuf, 999, 0);
					if (iRecvLen <= 0)
					{
						close(iAccept);
						return -1;
					}
					else
					{
						ucRecvBuf[iRecvLen] = '\0';
					}
					if ( !strcmp(ucRecvBuf,"listServer\n" ) )	{
						DIR  *dir;
						struct dirent* dirent;
						char dirName[256];
						getcwd(dirName,256);
						dir = opendir(dirName);
						char files[1000];
						while ( (dirent = readdir(dir)) != NULL){
							if (dirent->d_type == DT_REG){
								strcpy(files,dirent->d_name);
								int iSendLen = send(iAccept, files, 999, 0);
							if (iSendLen <= 0)	{
								printf("send() hatasi!\n");
								close(iAccept);
								return -1;
							}	
							
							}
						}
						strcpy(files,"end");
						send(iAccept,files,999,0);
					}
/*---------------------------------------------------------------------------*/							
				}
			}
			else	{
				close(iAccept);
			}
		}
	}

	close(iSocketServer);
	return 0;
}
/*  Ctrl + C sinyali olusunca server i ve butun client lari kapatir */
void interruptHandler (int signo)   {
	close(iSocketServer);
	printf("Ctrl + C sinyali geldi program kapatildi\n");
    exit(EXIT_FAILURE);
}