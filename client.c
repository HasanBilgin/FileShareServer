
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

void interruptHandler (int signo);

int main(int argc, char **argv)
{
	if (argc != 3)	{
		fprintf(stderr, "USAGE :\n");
		fprintf(stderr, "./client port ipaddress\n");
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

	while (1)
	{
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
					fprintf(stderr, "%s\n", ucSendBuf);
				}
			}
/*---------------------------------------------------------------------------*/			

			if ( !strcmp (ucSendBuf,"listLocal\n") ){
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
				fprintf(stderr, "listLocal : Client in oldugu klasordeki dosyalari listeler\n");
				fprintf(stderr, "listServer : Serverin oldugu klasordeki dosyalari listeler\n");
				fprintf(stderr, "lsClients : Servera bagli clientlari listeler\n");
				fprintf(stderr, "sendFile <clientID> <filename> : ID si verilen client a dosya adi verilen dosyayi yollar\n" );
			}
/*---------------------------------------------------------------------------*/						
		}
	}	
	return 0;
}
void interruptHandler (int signo)   {
	printf("Ctrl + C sinyali geldi program kapatildi\n");
    exit(EXIT_FAILURE);
}