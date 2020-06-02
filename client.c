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
#define LOCATIONOFDOWNLOAD 100
#define NAMEOFDOWNLOADSIZE 20
#define USERSIZE 20
#define TORRENTSIZE 40
#define IPADDRESS "10.0.2.15"
#define DOWNLOADFILEINTO "/home/vlada/Desktop/projekatSPR/Downloads/"
#define LOGOUTMESSAGE "Client logging out !\n\n"
#define CONFIRMATIONMESSAGE "File coming along!"

typedef struct data
{
	char user[USERSIZE];
	char torrent[TORRENTSIZE];
}info;


void downloadFile(int sock , info userinput)
{
	int fd ,n;
	
	FILE *fp;
	
	char charsWritten[10];
	
	char name[NAMEOFDOWNLOADSIZE];
	
	puts("Enter the name how you want the downloaded file to be saved as :");
	scanf("%s",name);
	
	char location[LOCATIONOFDOWNLOAD]=DOWNLOADFILEINTO;
	
	strcat(location,name);
	
	char buff[BUFFERSIZE];	

	fd = open(location , O_CREAT | O_WRONLY , S_IRWXU);
	
	read(sock,buff,sizeof(buff));
	
	n=write(fd,buff,sizeof(buff));
	
	close(fd);
	
	sprintf(charsWritten,"%d",n);
	
	strcat(location,"INFO");
	
		

	fp = fopen( location , "a");
	
	strcat(buff,"Name Of the file downloaded: ");
	strcat(buff,name);
	strcat(buff,"\nFile has been downloaded from: ");
	strcat(buff,userinput.torrent);
	strcat(buff,"\nSize of the file is: ");
	strcat(buff,charsWritten);

	fprintf(fp,"%s",buff);
	
	fclose(fp);
	
}



info enterUserNameAndTorrentSite(int sock)
{
	info userinput;
	
	char user[USERSIZE];
	char torrent[TORRENTSIZE];
	
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
	strcpy(userinput.user,user);
	strcpy(userinput.torrent,torrent);
	
	return userinput;

}
   
   
int main(int argc, char const *argv[]) 
{ 	
	char buff[MESSAGESIZE];
	
	int sock , port;
	
	info userinput;
	
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
    
   
	userinput = enterUserNameAndTorrentSite(sock);
    
	
    read(sock,buff,sizeof(buff));
    
    if((strcmp(buff,CONFIRMATIONMESSAGE))!=0)
    {
		printf("%s\n",buff);
	}
	
	else
	{
		downloadFile(sock , userinput);
		puts("File successfully downloaded !");
	}
    
    write(sock,LOGOUTMESSAGE,sizeof(LOGOUTMESSAGE));
    printf("%s",LOGOUTMESSAGE);
    close(sock);
    return 0;
    
}
	
