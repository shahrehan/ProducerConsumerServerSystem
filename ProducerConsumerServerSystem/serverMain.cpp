#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>  
#define LISTENER_NUMBER_ALLOWED 20
#include <netdb.h>       
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include "Graph.h"
#define MAXIMUM_SIZE_ALLOWED 1024
#define MAXIMUM_BUFFER 1024
#include <arpa/inet.h>
int consumer_sleep_time_io = 0;
float probability_consumer = 25.0;

typedef struct command{
	int sockObject;
	char cmd[MAXIMUM_SIZE_ALLOWED];
} command;

/////////////////////////////////// struct def //////////////////////////////////////
typedef struct {
	command buffer[MAXIMUM_SIZE_ALLOWED];
	long queueHead, queuetail;
	int time;
	int size;
	int queueitem;
	int full, empty;
	int simulationTime;	
	Graph *mygraph;
	pthread_mutex_t *mut;
	pthread_cond_t *notFull, *notEmpty;
} queue;

queue *fifo;
void queueAdd (queue *q, command c);
void queueDel (queue *q, command *out);
int consumer_sleep_time = 0;
int producer_sleep_time = 0;
void *producer(void* sockObject)
{
	command c;
	int isDone = 0;
	char receiver_buffer[MAXIMUM_BUFFER];	
	char trans_buffer[MAXIMUM_BUFFER];	

	int socket_identifier = *(int*)sockObject;
	
	bzero(receiver_buffer,MAXIMUM_BUFFER);
	bzero(trans_buffer,MAXIMUM_BUFFER);

	printf("Sock id = %d Created\n" , socket_identifier);
	while(!isDone)
	{
	
		read(socket_identifier, receiver_buffer, MAXIMUM_BUFFER);
		if(strcmp(receiver_buffer,"EXIT") == 0)
		{
			isDone = 1;
		}
		else
		{
			pthread_mutex_lock (fifo->mut); 
			while (fifo->full) {
				printf ("producer: queue FULL.\n");
				pthread_cond_wait (fifo->notFull, fifo->mut);
			}
			c.sockObject = socket_identifier;
			strcpy(c.cmd,receiver_buffer);
			queueAdd (fifo, c);
			pthread_mutex_unlock (fifo->mut);
			pthread_cond_signal (fifo->notEmpty);
			sleep (producer_sleep_time);
		}
	}
	close(socket_identifier);
	printf("Socket %d closed\n\n",socket_identifier);
	pthread_exit(NULL);
}

//////////////////////////////// process ////////////////////////////////////////
void process(command c)
{
	int origin,destination;
	char buf_maximum[MAXIMUM_SIZE_ALLOWED];
	char label[MAXIMUM_SIZE_ALLOWED];
	ostringstream out;
	int direction,speed;
	int length;
	sscanf(c.cmd,"%s",buf_maximum);
	if(strcmp(buf_maximum,"EDGE") == 0)
	{
		sscanf(c.cmd,"%s %d %d %d %d %d",buf_maximum,&origin,&destination,&direction,&speed,&length);
		if(fifo->mygraph->add_edge(origin,destination,direction,speed,length))
		{
			strcpy(buf_maximum,"Edge added successfully");
			write(c.sockObject, buf_maximum, sizeof(buf_maximum)); 
		}
		else
		{
			strcpy(buf_maximum,"Error: invalid edge specified");
			write(c.sockObject, buf_maximum, sizeof(buf_maximum)); 
		}
	}
	else if(strcmp(buf_maximum,"VERTEX") == 0)
	{
		sscanf(c.cmd,"%s %s",buf_maximum,label);
		fifo->mygraph->addVertex(INTERSECTION,label);
		strcpy(buf_maximum,"Vertex added successfully");
		write(c.sockObject, buf_maximum, sizeof(buf_maximum)); 
	}
	else if(strcmp(buf_maximum,"STORE") == 0)
	{
		sscanf(c.cmd,"%s %s",buf_maximum,label);
		fifo->mygraph->store(label);
		strcpy(buf_maximum,"Graph stored successfully");
		write(c.sockObject, buf_maximum, sizeof(buf_maximum)); 
	}
	else if(strcmp(buf_maximum,"TABLE") == 0)
	{
		sscanf(c.cmd,"%s %d %d",buf_maximum,&origin,&destination);
		out << endl;
		out << "\n------------------------------------------------------- (start of table)" << endl;
		out << "Routing Table for Node " << origin << ":" << endl;
		out << "Destination	Next hop	time(length/speed)	Visited" << endl;
		
		if (origin != destination)
		{
			fifo->mygraph->routingTable(origin, destination,out);
		}
		
		out << "\n------------------------------------------------------- (end of table)" << endl;

		//ss <<  out.rdbuf();
		string output = out.str();
		strcpy(buf_maximum,output.c_str());
		write(c.sockObject, buf_maximum, sizeof(buf_maximum)); 
	}
	else if(strcmp(buf_maximum,"TRIP") == 0)
	{
		sscanf(c.cmd,"%s %d %d",buf_maximum,&origin,&destination);
		out << "Summary of Shortest Path from given source and destination:" << endl;
		fifo->mygraph->trip(origin,destination,out);
		string output = out.str();
		strcpy(buf_maximum,output.c_str());
		write(c.sockObject, buf_maximum, sizeof(buf_maximum)); 
	}
	else
	{
		strcpy(buf_maximum,"Error: invalid command");
		write(c.sockObject,buf_maximum,sizeof(buf_maximum));
	}
}

////////////////////////// consumer ////////////////////////////////////////////////
void *consumer (void *q)
{
	int i;
	command d;
	float probability = 0;
	
	while (1) {
		pthread_mutex_lock (fifo->mut);
		if (fifo->empty) {
			printf ("consumer: queue EMPTY.\n");
			pthread_cond_wait (fifo->notEmpty, fifo->mut);
			pthread_mutex_unlock (fifo->mut);
		}
		else
		{
			queueDel (fifo, &d);
			process(d);
			pthread_mutex_unlock (fifo->mut);
			pthread_cond_signal (fifo->notFull);
			probability = ((rand() % 100) / 100.0);
			if(probability <= probability_consumer)
				sleep (consumer_sleep_time);
			else
			{
				sleep (consumer_sleep_time_io);
			}
			printf ("Consumed: %s \n",d.cmd);
			i++;
		}
	}
	printf("All done consuming.\n");
	return (NULL);
}

//////////////////////////////// server socket //////////////////////////////////
int serverSocket(int port_no, struct sockaddr_in *servaddr, char *serverName)
{
	int sd;
	int status; 
	int errno = 1;
	struct hostent *hostptr;

	 int optval = 1; 
	
	sd = socket(AF_INET, SOCK_STREAM, 0);
 	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

	if (sd < 0)
	{
		perror("cannot create socket");
		return -1;
	}
	printf("socket descriptor: %d , server-ip: %s, port: %d\n", sd,serverName,port_no);
	hostptr = gethostbyname(serverName);
	memset((void *)servaddr, 0, (size_t)sizeof(struct sockaddr_in));
	servaddr->sin_family = AF_INET;
	servaddr->sin_addr.s_addr = inet_addr(serverName);
	servaddr->sin_port = htons((unsigned short)port_no);

	status = bind(sd, (struct sockaddr *) servaddr,(socklen_t)sizeof(*servaddr));
	if(status != 0)
	{
		perror("socket--bind");
		exit(errno);
	}

	status = listen(sd, LISTENER_NUMBER_ALLOWED);
	if(status != 0)
	{
		perror("socket--listen");
		exit(errno);
	}

	return sd;
}

/////////////////////////// queue ///////////////////////////////////////////
queue *queueInit(int queueSize, char fileName[MAXIMUM_SIZE_ALLOWED])
{
	queue *q;

	q = (queue *)malloc (sizeof (queue));
	if (q == NULL) 
		return (NULL);
	
	q->mygraph = new Graph(fileName);
	q->empty = 1;
	q->full = 0;
	q->queueHead = 0;
	q->queuetail = 0;
	q->time = 0;
	q->size = queueSize;
	q->queueitem = 0;
	q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
	pthread_mutex_init (q->mut, NULL);
	q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->notFull, NULL);
	q->notEmpty = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->notEmpty, NULL);
	
	return (q);
}

////////////////////////////////////// Queue delete /////////////////////////////////
void queueDelete (queue *q)
{
	delete q->mygraph;
	pthread_mutex_destroy (q->mut);
	free (q->mut);	
	pthread_cond_destroy (q->notFull);
	free (q->notFull);
	pthread_cond_destroy (q->notEmpty);
	free (q->notEmpty);
	free (q);
}

////////////////////////////////////// Queue Add /////////////////////////////////
void queueAdd (queue *q, command c)
{
	q->buffer[q->queuetail] = c;
	q->queuetail++;
	if (q->queuetail == q->size)
		q->queuetail = 0;
	if (q->queuetail == q->queueHead)
		q->full = 1;
	q->empty = 0;
	q->queueitem++;
	return;
}

////////////////////////////////////// Queue delete /////////////////////////////////
void queueDel (queue *q, command *out)
{
	*out = q->buffer[q->queueHead];

	q->queueHead++;
	if (q->queueHead == q->size)
		q->queueHead = 0;
	if (q->queueHead == q->queuetail)
		q->empty = 1;
	q->full = 0;

	return;
}

//////////////////////////// Initializing server requirements //////////////////////
int main(int argc, char *argv[])
{
	int sd; 
	int con = 0;
	int a_sd[LISTENER_NUMBER_ALLOWED];  
	int i = 0;
	int portnum; 
	struct sockaddr_in servaddr;
	struct sockaddr_in cli_addr; 
	socklen_t sockadrr_in_size = sizeof(struct sockaddr_in);		
	pthread_t mythread;
	int val[LISTENER_NUMBER_ALLOWED];	
	int index = 0;
	char fileName[MAXIMUM_SIZE_ALLOWED];

	if ( argc != 4)
   	{
      		fprintf(stderr, "Usage: <ip> <portnum> <filename>\n");
      		exit(1);
   	}
	portnum = atoi (argv[2]);
	sd = serverSocket(portnum,&servaddr,argv[1]);

	for(i = 0; i < LISTENER_NUMBER_ALLOWED; i++)
		a_sd[i] = 0;

	strcpy(fileName,argv[3]); 
	fifo = queueInit (10,fileName);

	con = pthread_create(&mythread, NULL, consumer, NULL);
	
	while (1)
	{
		
		if (a_sd[index] == 0)	
		{
			a_sd[index] = accept(sd, (struct sockaddr *)(&cli_addr), &sockadrr_in_size);
			
			val[index] = pthread_create(&mythread, NULL, producer, &a_sd[index]);
			
		}
		index = index + 1 % LISTENER_NUMBER_ALLOWED;	
	}


	
	close(sd);
	return 0;

}

