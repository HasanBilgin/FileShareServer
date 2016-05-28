
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
#include <fcntl.h>

char pcClients[256][1000];
int iSocket[256];
pid_t pClientPID[256];
int iSocketServer;
void interruptHandler(int);

int main(int argc, char **argv)
{
	if (argc != 2)	{
		fprintf(stderr, "USAGE\n");
		fprintf(stderr, "./fileShareServer <port>\n");
		exit(1);
	}
	struct sockaddr_in server,client;  
	int iRet, iAddrLen,iRecvLen,iAccept;
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
			iSocket[iNumberOfClient] = iAccept;			
			//printf("Client baglandi %d : %s\n", iNumberOfClient, inet_ntoa(client.sin_addr));
			do{
				iRecvLen = recv(iAccept, ucRecvBuf, 999, 0);
				char ID[1000];
				ucRecvBuf[iRecvLen - 1] = '\0';
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
/*---------------------------------------------------------------------------*/							
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
						strcpy(files,"run");
						send(iAccept,files,999,0);
					}
/*---------------------------------------------------------------------------*/
			else if ( !strcmp (ucRecvBuf,"lsClients\n") ){
				char clients[1000];
				int i = 0;
				for( i = 0 ; i < 256 ;++i) {
					if ( strlen(pcClients[i]) > 0){
					int iSendLen = send(iAccept, pcClients[i], 999, 0);
					if (iSendLen <= 0)	{
						printf("send() hatasi!\n");
						close(iAccept);
						return -1;
						}
					}
				}
				char end[1000];
				strcpy(end,"end");
				send(iAccept,end,999,0);	
				strcpy(end,"run");
				send(iAccept,end,999,0);	
			}
			else if ( !strcmp (ucRecvBuf,"listLocal\n") || !strcmp(ucRecvBuf,"help\n") ){
				char run[1000];
				strcpy(run,"run");
				send(iAccept,run,999,0);	
			}
			else if ( ucRecvBuf[0] == 's' && ucRecvBuf[1] == 'e' && ucRecvBuf[2] == 'n' &&ucRecvBuf[3] == 'd' &&
				ucRecvBuf[4] == 'F' && ucRecvBuf[5] == 'i' && ucRecvBuf[6] == 'l' && ucRecvBuf[7] == 'e')	{
				char filename[1000];
				char client[1000];	
				char sendCommand[1000];
				sscanf(ucRecvBuf,"%s %s %s\n",sendCommand,client,filename);
				char run[1000];
				strcpy(run,"wait");
				send(iAccept,run,999,0);
				
				recv(iAccept, ucRecvBuf, BUFSIZ, 0);
			    send(iAccept, "wait", 999, 0);
    			int fileCapacity = atoi(ucRecvBuf);
				int fd = open (filename,O_CREAT | O_WRONLY | O_TRUNC, 0666);
    			int unCopy = fileCapacity;
    			int length;
    			while (((length = recv(iAccept, ucRecvBuf, BUFSIZ, 0)) > 0) && (unCopy > 0)) {
       				send(iAccept, "wait", 999, 0);
        			write(fd, ucRecvBuf, BUFSIZ);
        			unCopy -= length;
    			}    			
				strcpy(run,"run");
				send(iAccept,run,999,0);
				close(fd);						
			}
			else	{
				close(iAccept);
			}
			}/* end of while(1)	*/
		}/*end of child	*/
		}/* end of accept*/
	}/* end of while(1)	*/
	close(iSocketServer);
	return 0;
}
/*  Ctrl + C sinyali olusunca server i ve butun client lari kapatir */
void interruptHandler (int signo)   {
	char interrupt[1000];
	strcpy(interrupt,"dead");
	int i = 0;
	for ( i = 0 ; i < 256 ; ++i)
		send(iSocket[i],interrupt, 999, 0);

	close(iSocketServer);	
	printf("Ctrl + C sinyali geldi server kapandi\n");
    exit(EXIT_FAILURE);
}