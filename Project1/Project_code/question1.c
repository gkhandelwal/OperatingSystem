/* 
Server program which serves the read request from the client assigning one thread per client
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/time.h>

#define MILLION 1000000
#define BUF_SIZE 1024
#define NTHREADS 10000
#define MAXCONNECTIONS 10000

static pthread_t threads[NTHREADS];
void read_data(int);

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int n,i = 0;

	if (argc < 2) 
	{
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	// socket creation 
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		exit(1);
	 
	memset((char *) &serv_addr,0, sizeof(serv_addr));

	portno = atoi(argv[1]);
  	// sets the address in intenet address form
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	//binding the server address
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		exit(1);
	
	//sockets starts listening for the incoming conenctions
	listen(sockfd,MAXCONNECTIONS);
	clilen = sizeof(cli_addr);
	//Run the server continuously	  
	while (1)
	{
		// accepting new connection
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if(newsockfd > 0)
		{
			//create new thread for every client connection using pthread_create and arguments are the funtion address and the socketid
			if(pthread_create(&(threads[i]),NULL, &read_data , newsockfd) < 0) 
		  	{
				printf("Error\n");
		  	}
		  	//detach the threads
		  	pthread_detach(threads[i]);
		  	i++;
 
	   }
		
	}
		 
	close(sockfd);
	return 0; 
}

//Function to read the data from the socket id
void read_data(int socket_desc)
{
	int read_size;
	char client_message[BUF_SIZE];
	
	long time_read;   	
   	struct timeval start, end;
	//start the timer	
	gettimeofday(&start, NULL);
    //read the data in chunks of 1024KB
	while( (read_size = read(socket_desc , client_message , BUF_SIZE)) > 0 )
		memset(client_message, 0, BUF_SIZE);
		
	//When read is completed
	if(read_size == 0)
	{   
		//close the socket
		close(socket_desc);
		//end the timer when the read is completed
		gettimeofday(&end, NULL);
		//calculate time taken per thread
		time_read = ((end.tv_sec * MILLION + end.tv_usec)- (start.tv_sec * MILLION + start.tv_usec));
       	printf("Time taken for thread %d is %ld \n",socket_desc,time_read);
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		printf("recv failed");
	}
		 
	return;
} 
