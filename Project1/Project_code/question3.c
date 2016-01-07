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
// for preventing socket from monopolizing
#define stopLimit 1048576
int nServed = 0;
long double avgSum = 0;

// main function
void main(int argc, char *argv[])
{
	// used for computing time of each individual thread
	long time_read;
	struct timeval start[10000];
	struct timeval end[10000];

	
     	int sockfd; // used for main socket
	int newsockfd; // used for handover socket during accept
	int portno; // port no. on which server is listening
	int clilen; // client length
	int i; 
	fd_set master;    // master file descriptor list
    	fd_set read_fds;  // temp file descriptor list for select()
    	int fdmax;        // maximum file descriptor number
	// structure for socket
     	struct sockaddr_in serv_addr, cli_addr;

	// zeroing memory using memset 
	memset(start,0,10000*sizeof(struct timeval));
	memset(end,0,10000*sizeof(struct timeval));
     	
	// if we did not pass port number
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
	// zeroing memory
     	memset(&serv_addr,0,sizeof(serv_addr));
	// getting port no from command line vector
     	portno = atoi(argv[1]);
     	// sets the address in intenet address form
     	serv_addr.sin_family = AF_INET;
     	serv_addr.sin_addr.s_addr = INADDR_ANY;
     	serv_addr.sin_port = htons(portno);
     
     	//here we are binding the server address etc
     	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	{ 
              	printf("ERROR on binding");
		exit(1);
	}

	// listening 
     	listen(sockfd,MAX_THREADS);
	// here we are making main socket non-blocking
        if (fcntl(sockfd, F_SETFL, O_NONBLOCK | FASYNC) < 0)
    		exit(1);
	
     	clilen = sizeof(cli_addr);
	// here we are clearing list of Select
	FD_ZERO(&master);  
    	FD_ZERO(&read_fds);
    	// here we are adding main socket to master set
    	FD_SET(sockfd, &master);
	fdmax = sockfd;// it is storing maximum socket no.
	// non ending while loop, which is listening client
	while(1)
	{
		// copying everything to temporary from master set.
		// reason is during select some of the values changes
		// and we don't want to make our master set dirty or inconsistent
		read_fds = master; 
		// adding our reader list to select and if it fails we
		// exit from program
        	if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) 
		{
            		perror("select");
            		exit(4);
        	}
		// traversing our master set
		for(i = 0; i <= fdmax; i++) 
		{	
			// check whether any read is there or not
            		if (FD_ISSET(i, &read_fds)) 
	     		{ 
				// if that non-blocking is our main socket, we will do accept
                		if (i == sockfd) 
				{
				    // handle new connections
				    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
				    // if it fails, we will exit
				    if (newsockfd < 0)
				    {
		  			// ignore
				    }
		    		    else 
		    		    {
					// we are starting to record the time for this connection
					gettimeofday(&start[newsockfd], NULL);
					// we are making this socket as non-blocking
					if (fcntl(newsockfd, F_SETFL, O_NONBLOCK | FASYNC) < 0)
    						exit(1);
					// we are adding this to our master list
                        		FD_SET(newsockfd, &master); // add to master set
					// trying to manipulate maximum socket number
                        		if (newsockfd > fdmax) 
					{
                            			fdmax = newsockfd;
                        		}
                       			//printf("\nselect(): New Server Request from socket %d", newsockfd);
                           	     }
                		} 
				else 
				{
					char *buffer; // buffer 
					int nBytes = 0;// no. of bytes read
					int flag = 0; // flag to indicate
					int nStop = 0; // for stoplimit
  				        // reading data in chunks of BUFFER_SIZE
					do
					{
						nBytes = 0;
						flag = 0;
						// allocating memory to buffer
						buffer = (char *)malloc(sizeof(char)*(BUFFER_SIZE));
   						memset(buffer,0,sizeof(char)*(BUFFER_SIZE));
						// number of bytes read
						nBytes = read(i,buffer,BUFFER_SIZE*sizeof(char));
						if(nBytes >0)
							nStop = nStop + nBytes;
						// if it is less either it is non-blocked or it read system call is failing
						if(nBytes<0)
						{
							if(errno== EAGAIN)
							{
								flag = 1;
								free(buffer);
								buffer = NULL;
								break;
							}
							else
							{
								printf("\nError in Reading");
								fflush(stdout);
								break;
							}
							
						}
						// freeing the buffer
						free(buffer);
						buffer = NULL;
					}while((nBytes!=0)&&(nStop<stopLimit));
					if(nBytes==0)
					{
						printf("\nReceived Data from socket %d", i);
						fflush(stdout);
					}
					if((nBytes==0)&&(flag==0))
					{			
						nServed ++;
						// clearing socket from master set		
		                		FD_CLR(i, &master);
						// time computation of this thread
						gettimeofday(&end[i], NULL);
						time_read = ((end[i].tv_sec * MILLION + end[i].tv_usec)- (start[i].tv_sec * MILLION + start[i].tv_usec));
       						printf(" and Total time taken for this thread is  %ld \n",time_read);
						avgSum = (avgSum*((long double)(nServed-1)/nServed))+(long double)(time_read/nServed);
						printf(" avg time taken for all threads till this time is  %Lf \n",avgSum);
						// closing socket
						close(i); 
					}
                    			
                		} 
            		} 
        	}
	}
}

