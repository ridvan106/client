#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static volatile sig_atomic_t doneflag = 0; 

static void signal_handler(int signo) {
	doneflag = 1;
}


int main(int argc, char const *argv[])
{
	
    int sockfd, portno, n;
	struct sockaddr_in serv_addr;
    struct hostent *server;
	struct sigaction act;
	char buffer[1024], message[256];
    
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        perror("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

	act.sa_handler = signal_handler;
  	act.sa_flags = 0;
  	if ((sigemptyset(&act.sa_mask) == -1) || 
  	 	(sigaction(SIGINT, &act, NULL) == -1)) {
  		perror("Failed to set Signal handler.");
  		return 1;
	}


    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        perror("ERROR connecting");


    while (!doneflag) {

    	fprintf(stdout, "Please enter message to send server.\n");
		scanf("%255s", message);
        sprintf(buffer, "%s", message);
        if(strncmp(buffer, "recv", 4) == 0){
            fprintf(stderr, "GIRDIM\n");
            send(sockfd, buffer, sizeof(buffer), 0);
            fprintf(stderr, "GONDERDIM\n");
            recv(sockfd, buffer, sizeof(buffer), MSG_WAITALL);
            fprintf(stderr, "%s\n", buffer);
            continue;
        }
		sprintf(buffer, "%s", message);
		send(sockfd, buffer, sizeof(buffer), 0);
		fprintf(stdout, "Message is send to server.\n\n");
    }
	
	fprintf(stderr, "Ctrl + C geldi. Server kapanıyor...\n");
    close(sockfd);

	return 0;
}

