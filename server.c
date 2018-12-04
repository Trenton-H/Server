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

struct lists
{
	int fd;
	time_t active;
};

void join(int , int [], clock_t[], char [], FILE*);
void leaveConnection(int, int[], char[], clock_t [], FILE*);
int list(int, int [], char [], clock_t[], FILE*);
int logger(int, int[], FILE*);
void sendLog(int, FILE*);

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
	 int activeAgents[5] = { 0,0,0,0,0 };
	 char reply[16];
	 FILE *file_pointer; //pointer to the log file
	 file_pointer = fopen("log.txt", "w"); //open file log
	 int check1, check2, intSwitchValue = -1;
	 clock_t time[5];
	 char listReturn[500];

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

	 //puts server in loop to catch all messages sent to it through socket
	 while ((newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen)) >= 0)
	 {
		if (newsockfd < 0) {
			printf("%d \n\n", newsockfd);
			error("ERROR on accept");
		}

		 bzero(buffer, 256);
		 intSwitchValue = -1;
		 strcpy(reply, "");
		 //reads message from the client
		 n = read(newsockfd, buffer, 100);
		 if (n < 0)
			 error("ERROR reading from socket");

		 if ((strcmp(buffer, "JOIN")) == 0){
			 intSwitchValue = 1;
		 }
		 else if ((strcmp(buffer, "LEAVE")) == 0){
			 intSwitchValue = 2;
		 }
		 else if ((strcmp(buffer, "LIST")) == 0) {
			 intSwitchValue = 3;
		 }
		 else if ((strcmp(buffer, "LOG")) == 0) {
			 intSwitchValue = 4;
		 }

		 printf("%i \n\n", intSwitchValue);

		 switch (intSwitchValue)
		 {
		 case 1:
			 join(newsockfd, activeAgents, time, reply, file_pointer);
			 //strcpy(reply, join(newsockfd, activeAgents, time));
			 n = write(newsockfd, reply, 15);
			 break;
		 case 2:
			 leaveConnection(newsockfd, activeAgents, reply, time, file_pointer);
			 n = write(newsockfd, reply, 15);
			 break;
		 case 3:
			 check1 = list(newsockfd, activeAgents, listReturn, time, file_pointer);
			 if (check1 != -1)
				 n = write(newsockfd, listReturn, 500); //adjust the size as needed
			 break;
		 case 4:
			 check2 = logger(newsockfd, activeAgents, file_pointer);
			 if (check2 != -1)
				 sendLog(newsockfd, file_pointer);
			 break;
		 default:
			 break;
		 }
		 //printf("Here is the message: %s\n",buffer);
		 //n = write(newsockfd,"I got your message",18);
		 if (n < 0) 
			 error("ERROR writing to socket");
		 close(newsockfd);
	 }
	//close(sockfd);
     return 0; 
}

void join(int sd, int AA[], clock_t time[], char reply[], FILE * filePointer)
{
	//char reply[16];
	int check = -1;
	time_t now;
	struct tm *mytime;// = localtime(&now);
	time(&now);
	mytime = localtime(&now);

	fprintf(filePointer, "%s: Received a JOIN action from agent %i\n", asctime(mytime), sd);

	for(int i =0; i < 5; i++)
	{
		if(AA[i] == sd)
			check = 1;
	}
	if (check == 1)
	{
		strcpy(reply, "$ALREADY MEMBER");
		fprintf(filePointer, "%s: Responded to agent %i with $ALREADY MEMBER\n", asctime(mytime), sd);
	}
	else
	{
		for(int i=0; i < 5; i++)
		{
			if(AA[i] == 0)
			{
				AA[i] = sd;
				time[i] = clock();
				fprintf(filePointer, "%s: Added agent %i to active list\n", asctime(mytime), sd);
				break;
			}
		}
		strcpy(reply, "$OK");
		fprintf(filePointer, "%s: Respond to active agent %i with $OK\n", asctime(mytime), sd);
	}
}

void leaveConnection(int sd, int AA[], char reply[], clock_t timer[], FILE * filePointer)
{
	int check = -1;
	time_t now;
	struct tm *mytime;// = localtime(&now);
	time(&now);
	mytime = localtime(&now);
	fprintf(filePointer, "%s: Received a LEAVE action from agent %i\n", asctime(mytime), sd);
	for (int i = 0; i < 5; i++)
	{
		if (AA[i] == sd)
		{
			AA[i] = 0;
			timer[i] = time(NULL);
			strcpy(reply, "$OK");
			check = 1;
			fprintf(filePointer, "%s: Responeded to agent %i with $OK\n", asctime(mytime), sd);
			break;
		}
	}
	if (check == -1)
	{
		strcpy(reply, "$NOT MEMBER");
		fprintf(filePointer, "%s: Responded to inactive agent %i with $NOT MEMBER\n", asctime(mytime), sd);
	}
}

int list(int sd, int AA[], char list[], clock_t time[], FILE * filePointer)
{
	list = "";
	int check = -1, temp;
	char ipAddress[5];
	char timeStart[5];
	time_t now;
	struct tm *mytime;// = localtime(&now);
	time(&now);
	mytime = localtime(&now);

	fprintf(filePointer, "%s: Received a LIST action from agent %i\n", asctime(mytime), sd);

	for (int i = 0; i < 5; i++)
	{
		if (AA[i] == sd)
		{
			check = 1;
			fprintf(filePointer, "%s: Sent list to agent %i\n", asctime(mytime), sd);
			break;
		}
	}
	if (check == 1)
	{
		for (int i = 0; i < 5; i++)
		{
			if (AA[i] != 0)
			{
				clock_t t = clock();
				t = (double)(t - time[i]) / CLOCKS_PER_SEC;
				//list = stradd(lists, "<" + AA[i] + ", " + t + ">\n");
				//list = stradd("<%s, %s>", AA[i], t);
				temp = AA[i];
				//itoa(temp, ipAddress, 10);
				snprintf(ipAddress, 10, "%d", temp);
				//itoa(t, timeStart, 60);
				snprintf(timeStart, 60, "%ld", t);
				strcat(list, "<");
				strcat(list, ipAddress);
				strcat(list, ", ");
				strcat(list, timeStart);
				strcat(list, ">\n");
			}
		}
	}
	else
	{
		fprintf(filePointer, "%t: No response is supplied to agent %i (agent not active)\n", asctime(mytime), sd);
	}
	return check;
}

int logger(int sd, int AA[], FILE * file_pointer)
{
	int check = -1;
	time_t now;
	struct tm *mytime;// = localtime(&now);
	time(&now);
	mytime = localtime(&now);

	for (int i = 0; i < 5; i++)
	{
		if (AA[i] == sd)
		{
			check = 1;
			fprintf(filePointer, "%t: Sent Log to agent %i\n", asctime(mytime), AA[i]);
			break;
		}
	}
	if(check == -1)
		fprintf(filePointer, "%t: No response is supplied to agent %i (agent not active)\n", asctime(mytime), sd);
	return check;
}

void sendLog(int sd, FILE * filePointer)
{
	int size = 1000;
	char buff[size];
	int actuallyRead;

	time_t now;
	struct tm *mytime;// = localtime(&now);
	time(&now);
	mytime = localtime(&now);

	fprintf(filePointer, "%t: Sending Log to agent %i\n", asctime(mytime), sd);

	char *fsName = "./log.txt"; //opening file for read
	FILE *fs = fopen(fsName, "r");
	
	

	if (fs == NULL)
	{
		printf("ERROR: File not found.\n");
	}



	/*while ((actuallyRead = read("./log.txt", buff, sizeof(buff)) > 0))
		sendto(sd, buff, actuallyRead, 0);*/
	while ((actuallyRead = fread(buff, sizeof(char), size, filePointer) > 0))
		send(sd, buff, actuallyRead, 0);

	//send(sockfd, sdbuf, fs_block_sz, 0)

	bzero(buff, sizeof(buff));
}