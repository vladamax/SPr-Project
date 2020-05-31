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

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 6023 /*port*/
#define LISTENQ 100 /*maximum number of client connections */
#define MAXCLIENTS 100

//TODO after i find out from where the file should be sent ;; what kind of address are we expecting , change the function sendTheFile
//TODO put mutexes - if they arent mutally inclusive ( talking about next TODO and this TODO :) )
//TODO make them atomic

char user[1000][20];
char tor[1000][40];
char tempUser[20];
char tempTor[40];

// Defining a structure in order to pass multiple values to the thread

// 
int newCompare()
{
	
	int fd;	

	fd = open("/home/vlada/Desktop/projekatSPR/podaci.txt" , O_CREAT | O_RDONLY , S_IRWXU);
	
	if(fd<0)
	{
	puts("Problem opening file");
	exit(1);
	}
	
	char buff[10000];
	
	read(fd,buff,sizeof(buff));
	
	close(fd);
	
	/*FILE *f;
	f=fopen("/home/vlada/Desktop/projekatSPR/podaci.txt","w+");
	
	char buff[1000];
	fgets(buff,1000,f);
	*/
	
	
	char *token;
	
	
	
	token = strtok(buff," ");
	
	while (token!=NULL)
	{
		if((strcmp(tempUser,token))==0)
		{
			token = strtok(NULL," ");
			if((strcmp(tempTor,token))==0)
			{
				return 0;
				break;
			}
			else continue;
			
		}
		
		
		token = strtok(NULL," ");
	}
	return 1;
}


void *addDownloadToHistory()
{
	int fd;	

	fd = open("/home/vlada/Desktop/projekatSPR/podaci.txt", O_CREAT | O_WRONLY | O_APPEND , S_IRWXU);
	
	if(fd<0)
	{
	puts("Problem opening file");
	exit(1);
	}
	
	strcat(tempUser," ");
	strcat(tempTor," ");
	
	char total[60];
	
	strcat(total,tempUser);
	strcat(total,tempTor);
	
	// adding new case of downloading to the file
	write(fd,total,60);
	close(fd);
	
	pthread_exit(NULL);
	/*FILE *f;
	f=fopen("/home/vlada/Desktop/projekatSPR/podaci.txt","a+");
	fputs(total,f);
	
	char x[1000];
	fgets(x,1000,f);
	printf("Ovde je string iz fajla %s \n",x);
	fclose(f);
		*/
}


void *checkDownloadHistory()
{
	
	int i=0;
	while(user[i][0]!='\0')
	{
		if((strcmp(user[i],tempUser))==0)
		{
			if((strcmp(tor[i],tempTor))==0)
			{
			pthread_exit((void*)1);
			
		}
		}
		i++;
	}
	pthread_exit((void*)0);
	
}


void *sendTheFile(void *sock)
{
	int connfd;
	connfd=(int64_t)sock;
		
	write(connfd,"File coming along!",25);
		
	int fd;	

	fd = open("/home/vlada/Desktop/projekatSPR/podaci.txt" , O_RDONLY);
	
	if(fd<0)
	{
	puts("Problem opening file");
	exit(1);
	}

	char buff[10000];
	
	read(fd,buff,sizeof(buff));
	
	close(fd);
	
	write(connfd,buff,sizeof(buff));
	
	pthread_exit(NULL);
	
	
}



void *loadDownloadHistory()
{
	int fd;	

	fd = open("/home/vlada/Desktop/projekatSPR/podaci.txt" , O_CREAT | O_RDONLY , S_IRWXU);
	
	if(fd<0)
	{
	puts("Problem opening file");
	exit(1);
	}
	
	char buff[10000];
	
	read(fd,buff,sizeof(buff));
	
	close(fd);
	
	char *token;
	
	
	
	token = strtok(buff," ");
	int i=0;
	while (token!=NULL)
	{
		strncpy(user[i],token,50);
		
		
		token = strtok(NULL," ");
		
		strncpy(tor[i],token,50);
		
		token = strtok(NULL," ");
		i++;
	}
	pthread_exit(NULL);

}

void receiveDataFromClient(int connfd)
{
	int n;
	
	// receiving username from the client
	
	n=read(connfd,tempUser,20);
	if (n < 0) 
	 {
		perror("Read error");
		exit(1);
	 }

	// receiving torrent site from the client
	n=read(connfd,tempTor,40);
	if (n < 0) 
	 {
		perror("Read error");
		exit(1);
	 }
	
}
	

 void *serveTheClient(void *sock)
 {
	 
	 int connfd = *(int *)sock;
	 
	 int rc;
	 
	 pthread_t t_loadDownloadHistory , t_checkDownloadHistory , t_addDownloadToHistory , t_sendTheFile ;	 
	 

	// Creating thread for loading history of downloading from the file on the server
		rc = pthread_create(&t_loadDownloadHistory,NULL,loadDownloadHistory,NULL);
		if(rc!=0)
		printf("The problem occured while making a new thread - %s \n " , strerror(errno));
		
	// Receiving the username and the torrent that he wants to download
	
		receiveDataFromClient((int)connfd);
		
	// Waiting for the server to load downloading history
	
		pthread_join(t_loadDownloadHistory,NULL);
		
		
	// Creating thread for checking if the client had already made such download 

		rc = pthread_create(&t_checkDownloadHistory , NULL , checkDownloadHistory , NULL);
		if(rc!=0)
		printf("The problem occured while making a new thread - %s \n " , strerror(errno));
		
	 // Waiting for server to check if that particular user already downloaded that torrent
	 
		void *retValue;
		pthread_join(t_checkDownloadHistory , &retValue);
		
		// Converting the value received from the check if the user had already made such download and storing it into flag
		
		int16_t flag = (int64_t)(retValue);
		
		// If the response from checkDownloadHistory is positive ( the user had already made such download ) , send an explanaiton to the client
		
		// Else if the response is negative ( the user hadn't already made such download ) , 
		// add the download case to the download history and send the file to the client 
		
		if(flag==1)
		{
			char temp[70]="You already tried to download this torrent - it is a bad torrent !";
			write(connfd,temp,sizeof(temp));
		}
		else
		{
			//puts("Ovde sam pre adddownloadhistory");
			rc = pthread_create(&t_addDownloadToHistory,NULL,addDownloadToHistory,NULL);
			if(rc!=0)
			printf("The problem occured while making a new thread - %s \n " , strerror(errno));
			//puts("Ovde sam posle adddownloadhistory");
			rc = pthread_create(&t_sendTheFile,NULL,sendTheFile,(void*)(intptr_t)connfd);
			if(rc!=0)
			printf("The problem occured while making a new thread - %s \n " , strerror(errno));
			//puts("Posle treda");
			// waiting for both to finish
			
			pthread_join(t_addDownloadToHistory,NULL);
			//puts("Ovde sam posle adddownloadhistory joina");
			sleep(3);
			pthread_join(t_sendTheFile,NULL);
			// Add puts successfully if not existing 									//////////////
			//puts("Ovde sam posle sendTheFile joina");
		}
		
		close(connfd);
		puts("Service finished");
		
		
 
	 pthread_exit(NULL);
 }



int main (int argc, char **argv)
{
 int listenfd, connfd;
 socklen_t clilen;
 //char buf[MAXLINE];
 struct sockaddr_in cliaddr, servaddr;
 
 pthread_t tred[MAXCLIENTS];
 
 int rc;
 
 //creation of the socket
 listenfd = socket (AF_INET, SOCK_STREAM, 0);

 //preparation of the socket address
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = inet_addr("10.0.2.15");	//192.168.46.2
 servaddr.sin_port = htons(SERV_PORT);

 bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

 listen (listenfd, LISTENQ);

 printf("%s\n","Server running...waiting for connections.");
 
 clilen = sizeof(cliaddr);
 
 for ( int i=0; i<MAXCLIENTS ;i++ ) {
	 
	 
  clilen = sizeof(cliaddr);
  if(listenfd >0);
  connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
  
  
  // Adding data to the structure in order to pass multiple arguments to the thread
  // Doing this to become able to close listenfd from inside of thread ...
  // not sure if i should do this or it can be done outside of thread as well but inside for ...
  // I just read that if i close socket in parent thread it will be also closed in child thread
  // so there is no need for this structure but i'll keep it here just 
  // in case it turns out to be vice-versa
  
  
	printf("%s\n","Received request...");
	
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
 
 
 
 
 
