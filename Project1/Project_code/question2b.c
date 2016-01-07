#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/file.h>

#define MILLION 1000000
#define BUFFER_SIZE 4096
#define MAX_THREADS 10000

struct SocketList
{
	int socket;
	char *buf;
	struct SocketList *next;
};
typedef struct SocketList node;
node *head=NULL, *curr = NULL;
int nServed = 0;
long double avgSum = 0;
// for adding socket to our data structure
void addToList(int fd)
{
	// memory allocation of buffer
	char *buffer = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	//condition of addition of first node to a list
	if(head==NULL)
	{
		
		head=(node *)malloc (sizeof(node));
		// malloc failure.. We are exiting if malloc fails...
		if(head==NULL)
		{
			printf("\n%d malloc failure",errno);
			fflush(stdout);
			exit(1);
		}		
		curr = head;
		head->socket = fd;
		head->buf = buffer;
		head->next=NULL;
	}
	else
	{
		
		node *temp = (node *)malloc (sizeof(node));
		temp->socket = fd;
		temp->buf = buffer;
		temp->next=NULL; 
		curr->next = temp;
		curr=temp;
	}
		
}

// delete socket from list of pending items
void deleteNode(node *a)
{
	node *temp;
	// when we have to delete first node of list
	if(head==a)
	{
		// when there is only one node in list
		if(head->next==NULL)
		{
			head = curr = NULL;
			free(a->buf);
			free(a);
			a=NULL;
		}
		else
		{
			temp = head;
			head = head->next;
			free(temp->buf);
			free(temp);
			temp=NULL;
		}
	}
	else
	{
		
			temp=head;
			while(temp->next!=a)
				temp=temp->next;
			temp->next=temp->next->next;
			if(a->next==NULL)
				curr = temp;
			free(a->buf);
			free(a);
			a=NULL;
	}
}
	

// main program
void main(int argc, char *argv[])
{
	node *temp=NULL; // used for temporary exchanges with node
	
     	int sockfd; // socket id of main socket
	int newsockfd; // socket id after accept
	int portno; // port no, on which server is listening
	int clilen; // client length
	unsigned long time_read; // used for calculating time difference of each thread.
	int nBytes=0; // used for storing number of bytes read while read

	// for measuring time
	struct timeval start[MAX_THREADS];
	struct timeval end[MAX_THREADS];
	
	//socket structure
     	struct sockaddr_in serv_addr, cli_addr;
	
	// setting memory with zeroes
	memset(start,0,MAX_THREADS*sizeof(struct timeval));
	memset(end,0,MAX_THREADS*sizeof(struct timeval));

	// if we don't provide port number
     	if (argc < 2) {
         	fprintf(stderr,"ERROR, no port provided\n");
         	exit(1);
     	}

     	// socket creation
     	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// if socket creation failed
     	if (sockfd < 0)
	{
        	printf("ERROR opening socket");
		exit(1);
	}
     	memset(&serv_addr,0,sizeof(serv_addr));
	// getting port number from command line
     	portno = atoi(argv[1]);

     	// sets the address in intenet address form
     	serv_addr.sin_family = AF_INET;
     	serv_addr.sin_addr.s_addr = INADDR_ANY;
     	serv_addr.sin_port = htons(portno);
     
	// fcntl flag used for making socket non-blocking
	if (fcntl(sockfd, F_SETFL, O_NONBLOCK | FASYNC) < 0)
    		exit(1);

     	//here we are binding the server address etc
     	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	{
              	printf("ERROR on binding");
		exit(1);
	}

     	listen(sockfd,MAX_THREADS); // listening
        
     	clilen = sizeof(cli_addr);
	
	// non ending loop for server to be on every time
	while(1)
	{

		// handles new connections
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		// if creation of new handover socket failed
		if (newsockfd < 0)
		{ 
			if(errno != EAGAIN)
			{
				
			}
			
		}
		else 
		{
			// amking socket non-blocking
			if (fcntl(newsockfd, F_SETFL, O_NONBLOCK | FASYNC) < 0)
    				exit(1);
			// starting time recording of socket 
			gettimeofday(&start[newsockfd], NULL);
			// adding socket to list
			addToList(newsockfd);
		}
		// starting to traverse list
		temp=head;
		while(temp!=NULL)
		{
			// reading data in chunks of BUFFER_SIZE
			do
			{
				nBytes = 0;
				nBytes = read(temp->socket,temp->buf,BUFFER_SIZE*sizeof(char));
				if(nBytes<0)
				{
					if(errno== EAGAIN)
					{
						break;
					}
					else
					{
						printf("\nError in Reading");
						fflush(stdout);
						break;
					}			
				}		
			}while(nBytes!=0);
			// if read is completed
			if(nBytes==0)
			{
				node *c = temp;
				temp=temp->next;
				printf("\nReceived Data from socket %d", c->socket);
				nServed ++;
				// stopping time recording for this thread
				gettimeofday(&end[c->socket], NULL);
				time_read = ((end[c->socket].tv_sec * MILLION + end[c->socket].tv_usec)- 
					    (start[c->socket].tv_sec * MILLION + start[c->socket].tv_usec));
       				printf(" and total time taken for this thread is  %ld \n",time_read);
				avgSum = (avgSum*((long double)(nServed-1)/nServed))+(long double)(time_read/nServed);
				printf(" avg time taken for all threads till this time is  %Lf \n",avgSum);
				
				close(c->socket);
				deleteNode(c);
				fflush(stdout);
			}
			else
				temp=temp->next;		
		}
        }              
}

