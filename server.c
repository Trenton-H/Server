/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

struct connections
{
	int sockfd;
	time_t start_t;
}

void join(int , int [], char[]);
void leaveConnection(int, int[], char[]);
int list(int, int [], char []);
void sendLog(int);

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     int activeAgents[5] = {0,0,0,0,0};
     char reply [15];
	 FILE *file_pointer;
	 file_pointer = fopen("log.txt", "w");
	 int check 1, check2;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");
     bzero(buffer,256);
     //reads message from the client
     n = read(newsockfd,buffer,15);
     if (n < 0) 
	     error("ERROR reading from socket");
     switch(buffer)
     {
	     case "#JOIN":
		     join(newsockfd, activeAgents, reply);
			 n = write(newsockfd, reply, 15);
		     break;
	     case "#LEAVE":
			 leaveConnection(newsockfd, activeAgents, reply);
			 n = write(newsockfd, reply, 15);
		     break;
	     case "#List":
			 check1 = list(newsockfd, activeAgents, list);
			 if(check1 != -1)
				 n = write(newsockfd, list, 15); //adjust the size as needed
		     break;
	     case "#LOG":
			 check2 = log(newsockfd, activeAgents, log);
			 if (check2 != -1)
				 send(newsockfd);
		     break;
	     default:
		     break;
     }
     //printf("Here is the message: %s\n",buffer);
     n = write(newsockfd,"I got your message",18);
     if (n < 0) error("ERROR writing to socket");
     close(newsockfd);
     close(sockfd);
     return 0; 
}

void join(int sd, int AA[], char reply[])
{
	int check = -1;
	for(int i =0; i < 5; i++)
	{
		if(AA[i] == sd)
			check = 1;
	}
	if(check == 1)
		reply = "$ALREADY MEMBER";
	else
	{
		for(int i=0; i < 5; i++)
		{
			if(AA[i] == 0)
			{
				AA[i] = sd;
				break;
			}
		}
		reply = "$OK";
	}
}

void leaveConnection(int sd, int AA[], char reply[])
{
	int check = -1;
	for (int i = 0; i < 5; i++)
	{
		if (AA[i] == sd)
		{
			AA[i] = 0;
			reply = "$OK";
			check = 1;
			break;
		}
	}
	if (check == -1)
	{
		reply = "$NOT MEMBER";
	}
}

int list(int sd, int AA[], char list[])
{
	int check = -1;
	for (int i = 0; i < 5; i++)
	{
		if (AA[i] == sd)
			check = 1;
	}
	if (check = 1)
	{

	}
}

int log(int sd, int AA[], char log[])
{

}

void sendLog(int sd)
{
	int size = 1000;
	int buff[size];
	int actuallyRead;
	char *fsName = "./log.txt";
	FILE *fs = fopen(fsName, "r");
	if (fs == NULL)
	{
		
	}

	while ((actuallyRead = read("log.txt", buff, sizeof(buff)) > 0))
		sendto(sd, buff, actuallyRead, 0);
}