#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 6023 

void procitaj()
{
	int fd;
	fd=open("/home/vlada/Desktop/projekatSPR/Downloadedpodaci.txt", O_RDONLY);
	char buff[1000];
	read(fd,buff,sizeof(buff));
	
}


void downloadFile(int sock)
{
	int fd;	

	fd = open("/home/vlada/Desktop/projekatSPR/Downloadedpodaci.txt", O_CREAT | O_WRONLY , S_IRWXU);
	
	char buff[10000];
	
	read(sock,buff,sizeof(buff));
	
	write(fd,buff,sizeof(buff));
	
	close(fd);
}



void enterUserName(int sock)
{
	 // asking client to enter his username
      
	 while(1)
{	 
	char u[20];
	
	
	printf("Enter your username : ");
	scanf("%s",u);
	
	// if client entered nothing
	if(u==NULL)
	{
		puts("You entered nothing! \n");
		continue;
	}
	if((strchr(u,' '))==NULL)
	{
		write(sock,u,20);
		break;
	}
	else 
	{
		puts("Username cannot contain spaces! ");
		continue;
	}
}
	
	
	
}
   
   
void enterTorrentSite(int sock)
{   
	//asking client to enter torrent site
	 while(1)
{	 
	char t[40];
	
	printf("Enter torrent site : ");
	scanf("%s",t);
	
	// if client entered nothing
	if(t==NULL)
	{
		puts("You entered nothing! \n");
		continue;
	}
	if((strchr(t,' '))==NULL)
	{
		write(sock,t,40);
		break;
	}
	else 
	{
		puts("Torrent site cannot contain spaces! \n");
		continue;
	}
}	
}
 
 
int main(int argc, char const *argv[]) 
{ 	int port=PORT;
	in_addr_t ipaddress = inet_addr("10.0.2.15");
	if(argc>2)
	{
		
		ipaddress = inet_addr(argv[1]);
		port = atoi(argv[2]);
	}
	
	int sock;
	
	struct sockaddr_in serv_addr;
	
	char buff[1024];
	
	if ((sock=socket(AF_INET , SOCK_STREAM , 0)) < 0)
	{ 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = ipaddress;
    
    if(connect(sock,(struct sockaddr *)&serv_addr , sizeof(serv_addr))<0)
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    
   
   enterUserName(sock);
   enterTorrentSite(sock);
    

    read(sock,buff,sizeof(buff));
    
    if((strcmp(buff,"File coming along!"))!=0)
    {
		printf("%s\n",buff);
		exit(1);
	}
	
	else
	{
		downloadFile(sock);
		puts("File successfully downloaded ! \n");
	}
    
    
    close(sock);
    return 0;
    
}
	
