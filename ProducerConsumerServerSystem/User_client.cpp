#include <stdio.h> 
#include <time.h>
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  	
#include <netinet/in.h>  	 
#include <netdb.h>       	 
#include <pthread.h>
#include <termios.h>
#include <signal.h>		
#include <arpa/inet.h>

#define MAX_SIZE 1024

int sd;	//socket

////////////////////////////////////// initializing Menu /////////////////////////////////
int optionAvailable(){
    int var = 0;
    printf("MENU:\n");
    printf("1. Get Routing Table\n");
    printf("2. Find Shortest Distance\n");
    printf("3. Exit\n");
    scanf("%d",&var);
    if(var >= 1 && var <= 3)
        return var;
    else
    {
        printf("Error: invalid var entered\n");
        return 0;
    }
}
void intHandler(int a) {
    printf("\n");	
    write(sd,"<exit>",6);
    printf("\n\n");
    exit(1);
}

/////////////////////////////////// Server conection via socket ///////////////////////////////
int main( int argc, char* argv[] ) 
{ 
    int var = 0;
    int isDone = 0;
    int portNumber = 0;
    int variableFrom = 0;
    int variableTo = 0;
    int direction;
    char label[MAX_SIZE];
    char request[MAX_SIZE];
    char rx_buff[MAX_SIZE];
    int speed;
    int length;
    int type;
    struct sockaddr_in server_address; 

    struct termios term, term_orig;
    tcgetattr(STDIN_FILENO, &term);
    term_orig = term;
    term.c_lflag &= ~ECHO;

    portNumber = atoi(argv[2]);
    signal(SIGINT, intHandler);
    bzero((char *)&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr =  inet_addr(argv[1]);
    server_address.sin_port = htons(portNumber);
    if( ( sd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) 
    { 
	perror( "client: socket failed" ); 
	exit( 1 ); 
    } 
 
    if( connect( sd, (struct sockaddr*)&server_address, 
		 sizeof(server_address) ) == -1 ) 
    { 
	perror( "client: connect FAILED:" ); 
	exit( 1 ); 
    } 
 
    printf("connect() successful!\n"); 
    
    while(!isDone)
    {
        var = optionAvailable();
        switch(var)
        {
            case 1:
            printf("Vertex origin: ");
            scanf("%d",&variableFrom);
            printf("Vertex destination: ");
            scanf("%d",&variableTo);
            sprintf(request,"TABLE %d %d",variableFrom,variableTo);
            write(sd, request, sizeof(request));
            read(sd, rx_buff, MAX_SIZE);
            printf("%s\n",rx_buff);
            break;

            case 2:
            printf("Vertex origin: ");
            scanf("%d",&variableFrom);
            printf("Vertex destination: ");
            scanf("%d",&variableTo);
            sprintf(request,"TRIP %d %d",variableFrom,variableTo);
            write(sd, request, sizeof(request));
            read(sd, rx_buff, MAX_SIZE);
            printf("%s\n",rx_buff);
            break;

            case 3:
            sprintf(request,"EXIT");
            //write request variableTo server
            write(sd, request, sizeof(request)); 
            isDone = 1;
            break;
        }
    }

    printf("Client Connection terminated!\n");
    close(sd); 
    return(0); 
}

