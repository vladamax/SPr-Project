#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <strings.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> 
#include <pthread.h>
#include <string.h>
#include <signal.h>

#define SERV_PORT 6023 
#define MAXCLIENTS 100
#define BUFFERSIZE 10000
#define IPADDRESS "10.0.2.15"
#define LOGOUTMESSAGE "Client logging out !\n\n"
#define CONFIRMATIONMESSAGE "File coming along!"
#define DOWNLOADHISTORYLOCATION "/home/vlada/Desktop/projekatSPR/DownloadingHistory"
#define DECLINEMESSAGE "You already tried to download this torrent - it is a bad torrent !"
#define USERSIZE 20
#define TORRENTSIZE 40

char user[1000][USERSIZE];
char torrent[1000][TORRENTSIZE];
int iterator;

typedef struct data
{
	char tempUser[USERSIZE];
	char tempTor[TORRENTSIZE];
}ClientInfo;


pthread_mutex_t lock;

void *addDownloadToHistory(void * structure)
{
	ClientInfo *data;
	data=(ClientInfo*)structure;
	
	FILE *fp;	

	fp = fopen( DOWNLOADHISTORYLOCATION , "a");
	
	if(fp==NULL)
	{
	puts("Problem opening file");
	exit(1);
	}
	
		fprintf(fp , "%s %s " , data->tempUser , data->tempTor );
		
	fclose(fp);
	
	++iterator;
	
	strcpy(user[iterator],data->tempUser);
	strcpy(torrent[iterator],data->tempTor);
	
	
	pthread_exit(NULL);
}


int checkDownloadHistory(ClientInfo data)
{

	for(int z=0; z<=(iterator+1) ; z++)
	{
		
		if(strcmp(user[z],data.tempUser)==0)
		{
			if(strcmp(torrent[z],data.tempTor)==0)
			{
				return 1;
				break;
			}
			
		}
		
	}
	
	return 0;
}

void *sendTheFile(void *sock)
{
	char buff[BUFFERSIZE];
	
	int connfd;
	connfd=(int64_t)sock;
		
	write(connfd , CONFIRMATIONMESSAGE , 25);
		 
	int fd;	

	fd = open( DOWNLOADHISTORYLOCATION , O_RDONLY);
	
	if(fd<0)
	{
	puts("Problem opening file");
	exit(1);
	}
	
	read(fd,buff,sizeof(buff));
	
	close(fd);
	
	write(connfd,buff,sizeof(buff));
	
	pthread_exit(NULL);
	
	
}



void *loadDownloadHistory()
{	
	char buff[BUFFERSIZE];
	char *token;
	
	FILE *fp;	

	fp = fopen( DOWNLOADHISTORYLOCATION , "r");
	
	if(fp==NULL)
	{
	puts("Problem opening file");
	exit(1);
	}
	
	fgets( buff , sizeof(buff), fp);
	
	token = strtok(buff," ");
	iterator=0;

	while(token!=NULL)
	{
	strcpy(user[iterator],token);
	token = strtok(NULL," ");
	strcpy(torrent[iterator],token);
	token = strtok(NULL," ");
	
	++iterator;

	}
	fclose(fp);
	if(iterator!=0)
	iterator--;
	
	pthread_exit(NULL);
}

 ClientInfo receiveDataFromClient(int connfd)
{
	ClientInfo data;
	int n;
	
	// receiving username from the client
	
	n=read(connfd,data.tempUser,20);
	if (n < 0) 
	 {
		perror("Read error");
		exit(1);
	 }

	// receiving torrent site from the client
	n=read(connfd,data.tempTor,40);
	if (n < 0) 
	 {
		perror("Read error");
		exit(1);
	 }
	 
	 return data;
	 
	
}
	

 void *serveTheClient(void *sock)
 {
	 ClientInfo ClientData;
	 
	 char buff[BUFFERSIZE];
	 
	 int connfd = *(int *)sock;
	 
	 int rc;
	 
	 pthread_t t_addDownloadToHistory , t_sendTheFile ;	 
	 
	   pthread_mutex_lock(&lock);
		
	// Receiving the username and the torrent that he wants to download
	
		ClientData = receiveDataFromClient((int)connfd);

	// Creating thread for checking if the client had already made such download 

		int result = checkDownloadHistory(ClientData);
		
		// If the response from checkDownloadHistory is positive ( the user had already made such download ) ,
		// send an explanaiton to the client
		
		// Else if the response is negative ( the user hadn't already made such download ) , 
		// add the download case to the download history and send the file to the client 
		
		if(result==1)
		{
			write(connfd,DECLINEMESSAGE,sizeof(DECLINEMESSAGE));
			puts("Client tried to download the file that he has already downloaded.");
			
		}
		else
		{
			rc = pthread_create(&t_addDownloadToHistory,NULL,addDownloadToHistory,(void*)&ClientData);
			if(rc!=0)
			printf("The problem occured while making a new thread - %s \n " , strerror(errno));
			rc = pthread_create(&t_sendTheFile,NULL,sendTheFile,(void*)(intptr_t)connfd);
			if(rc!=0)
			printf("The problem occured while making a new thread - %s \n " , strerror(errno));
			
			// waiting for both to finish
			
			pthread_join(t_addDownloadToHistory,NULL);
			pthread_join(t_sendTheFile,NULL);
			puts("File successfully sent to the client");
		}
		
		while(1)
		{
			read(connfd,buff,sizeof(buff));
			if(strcmp( buff , LOGOUTMESSAGE )==0)
			{
				puts("Client logged out");
				break;
			}
			
		}
		
		pthread_mutex_unlock(&lock);
		close(connfd);
		puts("Service finished");
		
		
 
	 pthread_exit(NULL);
 }



int main (int argc, char **argv)
{
 int listenfd , connfd , rc;
 
 socklen_t clilen;
 
 struct sockaddr_in cliaddr, servaddr;
 
 pthread_t t_loadDownloadHistory;
 
 pthread_t tred[MAXCLIENTS];
 
 
 // creation of the socket
 
 listenfd = socket (AF_INET, SOCK_STREAM, 0);

 // preparation of the socket address
 
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = inet_addr(IPADDRESS);
 servaddr.sin_port = htons(SERV_PORT);
 
 // Creating thread for loading history of downloading from the file on the server
 
		rc = pthread_create(&t_loadDownloadHistory,NULL,loadDownloadHistory,NULL);
		if(rc!=0)
		printf("The problem occured while making a new thread - %s \n " , strerror(errno));
		
// Waiting for the server to load downloading history
	
		pthread_join(t_loadDownloadHistory,NULL);
		

 bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

 listen (listenfd, MAXCLIENTS);

 printf("%s\n","Server running...waiting for connections.");
 
 clilen = sizeof(cliaddr);
 
 for ( int i=0; i<MAXCLIENTS ;i++ ) {
	 
	 
  clilen = sizeof(cliaddr);
  
  // if there is client waiting , connect him
  
  if(listenfd >0);
  connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
  
	printf("%s\n","\nClient connected...");
	
		// creating thread for serving the client
		rc = pthread_create(&tred[i],NULL,serveTheClient,&connfd);
		if(rc!=0)
		printf("The problem is:  %s \n " , strerror(errno));
		
		
}

for ( int i=0; i<MAXCLIENTS ;i++ ) 
{
	pthread_join(tred[i],NULL);
}

 return EXIT_SUCCESS;
}
 
 
 
 
 
