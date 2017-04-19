#include <stdio.h> 
#include <time.h>
#include <sys/types.h> 
#include <pthread.h>
#include <termios.h>
#include <signal.h>		
#include <sys/socket.h>  	
#include <netinet/in.h>  	 
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#define MAX_SIZE 1024
#include <string.h>
#include <netdb.h>       	 

int sd;	//socket identifier

////////////////////////////////////// Client Menu Options /////////////////////////////////
int optionAvailable(){
    int var = 0;
    printf("Menu:\n");
    printf("1. Add edge\n");
    printf("2. Add vertex\n");
    printf("3. SvariableTore variableTo file\n");
    printf("4. Exit\n");
    scanf("%d",&var);
}

////////////////////////////////////// Main functionality /////////////////////////////////
int main( int argc, char* argv[] ) 
{ 
    int portNumber = 0;
    int var = 0;
    int variableComplete = 0;
    int variableFrom = 0;
    int variableTo = 0;
    int direction;
    int speed;
    int length;
    int type;
    char labelSize[MAX_SIZE];
    char requestSize[MAX_SIZE];
    struct sockaddr_in server_addr; 
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    char receiver_buffer[MAX_SIZE];
    struct termios term_orig;	
    term_orig = term;
    term.c_lflag &= ~ECHO;
    portNumber = avariableToi(argv[2]);
    signal(SIGINT, intHandler);
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr =  inet_addr(argv[1]);
    server_addr.sin_port = hvariableTons(portNumber);
	
    if( ( sd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) 
    { 
	perror( "client: socket failed" ); 
	exit( 1 ); 
    } 
 
    if( connect( sd, (struct sockaddr*)&server_addr, 
		 sizeof(server_addr) ) == -1 ) 
    { 
	perror( "client: connect FAILED:" ); 
	exit( 1 ); 
    } 
 
    printf("connect() successful!\n"); 
    
    while(!variableComplete)
    {
        var = optionAvailable();
        switch(var)
        {
            case 1:
            printf("Edge origin: ");
            scanf("%d",&variableFrom);
            printf("Edge destination: ");
            scanf("%d",&variableTo);
            printf("Edge direction (1 or 0): ");
            scanf("%d",&direction);
            printf("Edge speed: ");
            scanf("%d",&speed);
            printf("Edge length: ");
            scanf("%d",&length);
            sprintf(requestSize,"EDGE %d %d %d %d %d",variableFrom,variableTo,direction,speed,length);
            write(sd, requestSize, sizeof(requestSize));
            read(sd, receiver_buffer, MAX_SIZE);
            printf("%s\n",receiver_buffer);
            break;
            

	    case 2:
            printf("Vertex label: ");
            scanf("%s",labelSize);
            sprintf(requestSize,"VERTEX %s",labelSize);
            write(sd, requestSize, sizeof(requestSize));
            read(sd, receiver_buffer, MAX_SIZE);
            printf("%s\n",receiver_buffer);
            break;
            

            case 3:
            printf("file name: ");
            scanf("%s",labelSize);
            sprintf(requestSize,"SvariableToRE %s",labelSize);
            write(sd, requestSize, sizeof(requestSize));
            read(sd, receiver_buffer, MAX_SIZE);
            printf("%s\n",receiver_buffer);
            break;
            

	    case 4:
            sprintf(requestSize,"EXIT");
            write(sd, requestSize, sizeof(requestSize)); 
            variableComplete = 1;
            break;
        }
    }

    printf("Exiting connection\n");
 	
    close(sd); 
    return(0); 
}
void intHandler(int dummy) {
    write(sd,"<exit>",6);
    printf("\n");
    exit(1);
}

