//Reference: the following codes are adapted from the sample lab's code

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <time.h>


#define BUFFER_SIZE 1024
#define MAX_PARAM 10

int sock;
struct sockaddr_in sa;
time_t dateTime;
struct tm *getTime;
char *month[12] = {"Jan","Feb","Mar","Apr","May","Jun",
						"Jul","Aug","Sep","Nov","Dec"};
const char *responseWithNewLine[3] = {"HTTP/1.0 200 OK\n\n", "HTTP/1.0 400 Bad Request\n\n", 
						"HTTP/1.0 404 Not Found\n\n"};
char *response[3] = {"HTTP/1.0 200 OK", "HTTP/1.0 400 Bad Request", 
						"HTTP/1.0 404 Not Found"};						

void serverLog(char *requestLine, int responseLine, char *root, char *fileName){
	dateTime = time(NULL);
	getTime = localtime(&dateTime);
	printf("%s %d %d:%d:%d ", month[getTime->tm_mon], getTime->tm_mday, 
		getTime->tm_hour, getTime->tm_min, getTime->tm_sec);
	fflush(stdout);
	printf("%s:%d ", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));  
	fflush(stdout);
	printf("%s; %s; %s%s\n", requestLine, response[responseLine], root, fileName);
}												

//send response to client
void sendToClient(const char *message){
	if(sendto(sock,message,strlen(message),0,(struct sockaddr *)&sa,sizeof sa)==-1){
		fprintf(stderr,"%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
} 

//send data to client
void dataToClient(char *fileName, char *request, char *root){
	FILE *f;
	size_t bytesRead;
	long int fileSize;
	f = fopen(fileName+1, "r");	
	if(f == NULL){
		//can't open or find file
		sendToClient(responseWithNewLine[2]);
		serverLog(request, 2, root, fileName);
	}else{
		fseek(f, 0, SEEK_END);
		fileSize = ftell(f);
		fseek(f, 0, SEEK_SET);
		char buf[sizeof(char)*fileSize];
		memset(&buf, 0, sizeof(buf));
		bytesRead = fread(buf, sizeof(char), fileSize, f);
		fclose(f);
		if(bytesRead == 0){
			//bad file
			sendToClient(responseWithNewLine[2]);
			serverLog(request, 2, root, fileName);
		}
		else{
			sendToClient(responseWithNewLine[0]);
			if(bytesRead <= BUFFER_SIZE){
				if(sendto(sock, &buf, bytesRead,0,(struct sockaddr *)&sa,sizeof sa)==-1){
					fprintf(stderr,"%s\n", strerror(errno));
					exit(EXIT_FAILURE);
				}
			}
			else{
				int i = 0;
				int size = BUFFER_SIZE;
				while(i < bytesRead){
					if(sendto(sock, &buf[i], size,0,(struct sockaddr *)&sa,sizeof sa)==-1){
						fprintf(stderr,"%s\n", strerror(errno));
						exit(EXIT_FAILURE);
					}							
					i = BUFFER_SIZE + i;
					if((bytesRead - i) < BUFFER_SIZE){
						size = bytesRead - i;
					}
				}
			}
			serverLog(request, 0, root, fileName);
		}
	}
}

int main(int argc, char *argv[]){ 
	//check if given enough arguments
	if(argc != 3){
		printf("Usage: \"./sws <port> <directory>\"\n");
		return -1;
	}
	
	//change current directory to the given directory
	if(chdir(argv[2])){
		fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}
	
	//open socket
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(sock == -1){
		perror("failed to open a socket");
		return -1;
	}
	
	//reuse the same port
	int opt = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) == -1){
		perror("setsockopt failed");
		close(sock);
		return -1;
	}
	
	//bind addr
	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(atoi(argv[1]));
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sock, (struct sockaddr *) &sa, sizeof sa) == -1){
		perror("failed to bind");
		close(sock);
		return -1;
	}
	
	printf("sws is running on UDP port %s and serving %s\n", argv[1], argv[2]);
	printf("press 'q' to quit ...\n");
	
	fd_set readfds;
	char buffer[BUFFER_SIZE];
	char clientReq[BUFFER_SIZE];
	ssize_t recvSize;
	socklen_t fromlen = sizeof(sa);	
	
	for(;;){
		//add stdin and socket to fd_set
		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);
		FD_SET(sock, &readfds);
		if(select(sock+1, &readfds, NULL, NULL, NULL) == -1){
			perror("select() failed");
			close(sock);
			return -1;
		}
		
		//check whether the request is from keyboard or client
		//if from keyboard, quit if q
		if(FD_ISSET(STDIN_FILENO, &readfds)){
			fgets(buffer, BUFFER_SIZE, stdin);
			strtok(buffer, "\n");
			if(strcmp(buffer, "q") == 0){
				close(sock);
				return 0;
			}
		}

		//get client request
		memset(&clientReq, 0, sizeof(clientReq));
		if(FD_ISSET(sock, &readfds)){
			if(recvfrom(sock,clientReq,sizeof(clientReq),0,(struct sockaddr *)&sa,&fromlen)==-1){
				fprintf(stderr, "%s\n", strerror(errno));
				return -1;
			}
			
			char cReq[BUFFER_SIZE];
			strcpy(cReq, clientReq);
			char *request = strtok(clientReq, " ");
			char *list[MAX_PARAM] = {NULL};
			int i = 0;
			while(request != NULL){
				list[i] = request;
				i++;
				request = strtok(NULL, " ");
			}
			
			request = strtok(cReq, "\r");
			
			//check the http method, http version, and end of http header
			if(strcasecmp(list[0], "GET") || strcasecmp(list[2], "HTTP/1.0\r\n\r\n")){
				sendToClient(responseWithNewLine[1]);
				serverLog(request, 1, argv[2], list[1]);
			}
			else if(strstr(list[1], "..") != NULL){
				//not allowing client to access root dir
				sendToClient(responseWithNewLine[2]);
				serverLog(request, 2, argv[2], list[1]);
			}
			else if(strcmp(list[1], "/") == 0){
				//return index.html if no file provided
				dataToClient("/index.html", request, argv[2]);	
			}
			else{
				//other file
				if(list[1][0] != '/'){
					sendToClient(responseWithNewLine[1]);
					serverLog(request, 1, argv[2], list[1]);
					continue;
				}
				dataToClient(list[1], request, argv[2]);								
			}
		}
	}
}
