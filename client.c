
#include <sys/types.h>         
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>


void interruptHandler (int signo);
void closeClient(int signo);

int main(int argc, char **argv)
{
	if (argc != 3)	{
		fprintf(stderr, "USAGE :\n");
		fprintf(stderr, "./client <port> <ipaddress>\n");
		exit(1);
	}
	int iSocketClient;
	struct sockaddr_in server;   
	
	int iRet;
	char ucSendBuf[1000];
	int iSendLen;
	
	signal(SIGINT,interruptHandler);
	iSocketClient = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;
	server.sin_port   = htons(atoi(argv[1]));

	iRet = inet_aton(argv[2], &server.sin_addr);
	if (iRet == 0)
	{
		printf("IP adresi hatali");
		return -1;
	}
	memset(server.sin_zero, 0, 8);

	iRet = connect(iSocketClient, (const struct sockaddr *)&server, sizeof(struct sockaddr));
	if (iRet == -1)
	{
		printf("connect() hatasi!\n");
		return -1;
	}
	fprintf(stderr, " Client ID sini girin : ");
	if (fgets(ucSendBuf,999,stdin))	{
		iSendLen = send(iSocketClient, ucSendBuf, strlen(ucSendBuf), 0);
		if (iSendLen <= 0)	{
			printf("send() hatasi!\n");
			close(iSocketClient);
		return -1;
		}
	}	
	int interrupt=0;
	while (1)
	{
		if ( !strcmp(ucSendBuf,"dead"))
				kill(getpid(),SIGUSR1);
		fprintf(stderr, " Yapilacak islemi girin : ");
		if (fgets(ucSendBuf, 999, stdin))
		{
			iSendLen = send(iSocketClient, ucSendBuf, strlen(ucSendBuf), 0);
			if (iSendLen <= 0)
			{
				printf("send() hatasi!\n");
				close(iSocketClient);
				return -1;
			}
/*---------------------------------------------------------------------------*/							
			if ( !strcmp (ucSendBuf,"listServer\n") ){
				while (recv(iSocketClient, ucSendBuf, 999, 0) > 0){
					if ( !strcmp(ucSendBuf,"end"))
						break;
					if ( !strcmp(ucSendBuf,"dead"))
						interrupt = 1;
					
					fprintf(stderr, "%s\n", ucSendBuf);
				}
			}
/*---------------------------------------------------------------------------*/			

			else if ( !strcmp (ucSendBuf,"listLocal\n") ){
				DIR  *dir;
				struct dirent* dirent;
				char dirName[256];
				getcwd(dirName,256);
				dir = opendir(dirName);
				char files[1000];
				while ( (dirent = readdir(dir)) != NULL){
					if (dirent->d_type == DT_REG){
						puts(dirent->d_name);		
					}
				}
			}
/*---------------------------------------------------------------------------*/						
			else if ( !strcmp(ucSendBuf,"help\n"))
			{
				puts("listLocal : Client in oldugu klasordeki dosyalari listeler\n");
				puts( "listServer : Serverin oldugu klasordeki dosyalari listeler\n");
				puts( "lsClients : Servera bagli clientlari listeler\n");
				puts( "sendFile <clientID> <filename> : ID si verilen client a dosya adi verilen dosyayi yollar\n" );
			}
/*---------------------------------------------------------------------------*/
			else if ( !strcmp (ucSendBuf,"lsClients\n") ){
				char clients[1000];
				int i = 0;
				while (recv(iSocketClient, ucSendBuf, 999, 0) > 0){
					if ( !strcmp(ucSendBuf,"end"))
						break;
					if ( !strcmp(ucSendBuf,"dead"))
						interrupt = 1;
					fprintf(stderr, "%s\n", ucSendBuf);			
				}
			}
			else if ( ucSendBuf[0] == 's' && ucSendBuf[1] == 'e' && ucSendBuf[2] == 'n' &&ucSendBuf[3] == 'd' &&
					ucSendBuf[4] == 'F' && ucSendBuf[5] == 'i' && ucSendBuf[6] == 'l' && ucSendBuf[7] == 'e')	{
				char filename[1000];
				char client[1000];	
				char sendCommand[1000];
				sscanf(ucSendBuf,"%s %s %s\n",sendCommand,client,filename);
				
				recv(iSocketClient, ucSendBuf, 999, 0);				
				int fd = open (filename,O_RDONLY);
				struct stat status;
				fstat(fd, &status);
				sprintf(ucSendBuf, "%d",(int)status.st_size);
			    send(iSocketClient, ucSendBuf, 999, 0);
				recv(iSocketClient, ucSendBuf, 999, 0);	
				int fileCapacity = (int)status.st_size;
				size_t bytes_read;
				char buffer[BUFSIZ];
				while (((bytes_read = (size_t)read(fd, buffer, BUFSIZ))) && (fileCapacity > 0)) {
				    int bytes_send = (int)send(iSocketClient, buffer, bytes_read, 0);
				    recv(iSocketClient, ucSendBuf, 999, 0);	
				    fileCapacity -= bytes_send;
				}
				send(iSocketClient, "endCopy", 999, 0);
				close(fd);
			}
/*---------------------------------------------------------------------------*/						
		}
		signal(SIGUSR1,closeClient);
		if (interrupt)
			kill(getpid(),SIGUSR1);
		recv(iSocketClient, ucSendBuf, 999, 0);

	} /* end of while (1)	*/	
	return 0;
}
void interruptHandler (int signo)   {
	if (signo == SIGINT){
		printf("Ctrl + C sinyali geldi program kapatildi\n");
    	exit(EXIT_FAILURE);
	}
}

void closeClient(int signo)	{
	printf("Server kapandi. Client kapatiliyor!!!\n");
	exit(EXIT_FAILURE);
}