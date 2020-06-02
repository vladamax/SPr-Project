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
#define BUFFERSIZE 10000
#define MESSAGESIZE 100
#define IPADDRESS "10.0.2.15"
#define DOWNLOADFILEINTO "/home/vlada/Desktop/projekatSPR/DownloadedFile"
#define LOGOUTMESSAGE "Client logging out !\n\n"
#define CONFIRMATIONMESSAGE "File coming along!"


void downloadFile(int sock)
{
	char buff[BUFFERSIZE];
	
	int fd;	

	fd = open(DOWNLOADFILEINTO, O_CREAT | O_WRONLY , S_IRWXU);
	
	read(sock,buff,sizeof(buff));
	
	write(fd,buff,sizeof(buff));
	
	close(fd);
}



void enterUserName(int sock)
{
	char user[20];
	
	 // asking client to enter his username
      
	 while(1)
{	 
	
	printf("\nEnter your username : ");
	scanf("%s",user);
	
	// if client entered nothing
	
	if(user==NULL)
	{
		puts("You entered nothing! \n");
		continue;
	}

	// if there are no blank spaces in the username entered by the user , send it to the server
	
	if((strchr(user,' '))==NULL)
	{
		write(sock,user,sizeof(user));
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
	char torrent[40];
	
	//asking client to enter torrent site
	 while(1)
{	 

	
	printf("Enter torrent site : ");
	scanf("%s",torrent);
	
	// if client entered nothing
	if(torrent==NULL)
	{
		puts("You entered nothing! \n");
		continue;
	}
	if((strchr(torrent,' '))==NULL)
	{
		write(sock,torrent,sizeof(torrent));
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
{ 	
	char buff[MESSAGESIZE];
	
	int sock , port;
	
	struct sockaddr_in serv_addr;
	
	
	
	port=PORT;
	in_addr_t ipaddress = inet_addr(IPADDRESS);
	if(argc>2)
	{
		ipaddress = inet_addr(argv[1]);
		port = atoi(argv[2]);
	}
	
	// creation of the socket
	
	if ((sock=socket(AF_INET , SOCK_STREAM , 0)) < 0)
	{ 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
    
    // preparation of the socket address
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = ipaddress;
    
    // connecting to the server
    
    if(connect(sock,(struct sockaddr *)&serv_addr , sizeof(serv_addr))<0)
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    
   
   enterUserName(sock);
   enterTorrentSite(sock);
    
	
    read(sock,buff,sizeof(buff));
    
    if((strcmp(buff,CONFIRMATIONMESSAGE))!=0)
    {
		printf("%s\n",buff);
	}
	
	else
	{
		downloadFile(sock);
		puts("File successfully downloaded !");
	}
    
    write(sock,LOGOUTMESSAGE,sizeof(LOGOUTMESSAGE));
    printf("%s",LOGOUTMESSAGE);
    close(sock);
    return 0;
    
}
	
