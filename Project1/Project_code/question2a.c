#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <aio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <fcntl.h>

static const int BUFFER_SIZE = 1024;

struct SocketNode {
	/* aiocb structure */
	struct aiocb* data;
	struct SocketNode* next;
	/* start time */
	struct timeval start_time;
	/* end time */
	struct timeval end_time;
};

typedef struct SocketNode node;

node *head=NULL, *curr = NULL;

/* function to add socket to the linked list */
void addToList(struct aiocb* fd) {
	if(head==NULL) {		
		head=(node *)malloc (sizeof(node));
		if(head==NULL) {
			printf("\n%d malloc failure",errno);
			fflush(stdout);
			exit(1);
		}		
		curr = head;
		head->data = fd;
		head->next=NULL;
		gettimeofday(&head->start_time, NULL);
	} else {		
		node *temp = (node *)malloc (sizeof(node));
		temp->data = fd;
		temp->next=NULL; 
		gettimeofday(&temp->start_time, NULL);
		curr->next = temp;
		curr=temp;
	}		
}

/* function to delete a node from the linked list */
void deleteNode(node *a) {
	node* temp;
	if(head==a) {
		if(head->next==NULL) {
			head = curr = NULL;
			free((void*)a->data->aio_buf);
			free(a->data);
			free(a);
			a=NULL;
		} else {
			temp = head;
			head = head->next;
			free((void*)a->data->aio_buf);
			free(a->data);
			free(a);
			temp=NULL;
		}
	} else {		
		temp=head;
		while(temp->next!=a)
			temp=temp->next;
		temp->next=a->next;
		if(a->next==NULL)
			curr=temp;
		free((void*)a->data->aio_buf);
		free(a->data);
		free(a);
		a=NULL;
	}
}

/* main driver program */
int main(int argc, char *argv[]){
	int sockfd, newsockfd, portno;
	socklen_t clilen; struct sockaddr_in serv_addr, cli_addr;

	struct aiocb* cb; char* data;
	struct aiocb* cb_new; char* data_new;
	int bytes_read=-1; node* start = head;
	
	int no_of_requests = 0;
	unsigned long request_time = 0;
	long double avg_time = 0;

	if(argc < 2) {
		perror("Please provide a port\n");
		exit(1);
	}

	// create listening socket 
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	portno = atoi(argv[1]);
	// set the address in intenet address form
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	// make the socket non-blocking
	if(fcntl(sockfd, F_SETFL, O_NONBLOCK | FASYNC) < 0)
		exit(1);
	// bind the server address
	if(bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		exit(1);
	}
	listen(sockfd,10000);
	clilen = sizeof(cli_addr);

	// infinite loop to let the server running continuously
	while(1){
		// accept a new connection and add the socket to linked list
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if(newsockfd > 0){
			data_new = calloc(1,BUFFER_SIZE);
			cb_new = calloc(1,sizeof(struct aiocb));
			cb_new->aio_offset = 0;
			cb_new->aio_nbytes = BUFFER_SIZE;
			cb_new->aio_fildes = newsockfd;
			cb_new->aio_buf = data_new;
			aio_read(cb_new);
			addToList(cb_new);
		}

		// iterate over the linked list and read data from sockets which are ready
		start = head;
		while(start != NULL){ 
			cb = start->data;
			if(aio_error(cb) != EINPROGRESS){
				bytes_read = aio_return(cb);
				if(bytes_read != 0){
					// read the data from the ready socket, update offset and issue aio_read again
					//printf("Bytes Read : %d from socket : %d\n",bytes_read,cb->aio_fildes);
					cb->aio_offset += bytes_read;
					data = calloc(1,BUFFER_SIZE);
					cb->aio_buf = data;
					aio_read(cb);
					start = start->next;
				}else{
					// entire data has been read from a socket
					node* temp = start;
					start = start->next;
					gettimeofday(&temp->end_time, NULL);
					// print time taken to serve each connection
					request_time = (temp->end_time.tv_sec * 1000000 + temp->end_time.tv_usec) - (temp->start_time.tv_sec * 1000000 + temp->start_time.tv_usec);
					no_of_requests++;
					if(no_of_requests == 1)
						avg_time = request_time;
					else
						avg_time = (avg_time*((long double)(no_of_requests-1)/no_of_requests))+(long double)(request_time/no_of_requests);
					printf("Time taken for this request : %lu microseconds\n", request_time);
					printf("Average Time for the server : %Lf microseconds\n", avg_time);
					// close the socket
					close(cb->aio_fildes);
					// delete node from the linked list
					deleteNode(temp);
				}
			}
			else{
				start = start->next;
			}
		}
	}
	close(sockfd);
	return 0;
}
