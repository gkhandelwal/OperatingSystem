/* submitted by Gaurav Khandelwal (khand052), Gopal Sarda (sarda014), Ashritha Nagavaram (nagav001) */


/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <time.h>

struct disk *disk;
char *pagefaulthandler;
int *frame_mapping;

int disk_read_cnt = 0;
int disk_write_cnt = 0;
int page_fault_cnt = 0;

struct Node
{
	int frame;
	struct Node *next;
};
typedef struct Node node;
node *head=NULL,*tail=NULL;

// mapping structure used for mapping in custom algorithm
struct mapper
{
	int frame;
	int page;
};
typedef struct mapper mapping;
mapping *elements;


//Add new frame into the list
void add(int frame)
{
	if(head==NULL)
	{
		head= (node *)malloc(sizeof(node));
		head->next=NULL;
		head->frame = frame;
		tail = head;
	}
	else
	{
		node * temp;
		temp= (node *) malloc(sizeof(node));
		temp->next = NULL;
		temp ->frame = frame;
		tail->next = temp;
		tail=tail->next;
	}
}


//delete based on head movement
void delete()
{
	node *temp;
	temp = head;
	if(head->next==NULL)
	{
		head=tail=NULL;
	}
	else
		head=head->next;
	free(temp);
	temp=NULL;		
}

// generic delete based on value
void deleteFrame(int frame)
{
	node *c = head;
	if(c==NULL)
		return;
	else
	{
		if(c->frame == frame)
		{
			if(c->next==NULL)
			{
				head = tail = NULL;
				free(c);
			}
			else
			{
				head = head->next;
				free(c);
			}
			return;
		}
		else
		{
			node *temp=NULL;
			while(c!=NULL)
			{
				if(c->frame==frame)
				{
					temp = c;
					break;
				}
				c=c->next;	
			}
			c=head;
			while(c->next!=temp)
				c=c->next;
			if(temp->next==NULL)
				tail=c;
			c->next=temp->next;
			free(temp);
		}
	}	
}


//cleanup memory once program finishes
void cleanup(struct page_table *pt)
{
	int page_frame, page_bits;
	node *c = NULL;
	while(head!=NULL)
	{
		
		page_table_get_entry(pt, frame_mapping[head->frame], &page_frame, &page_bits);			
		if((page_bits & PROT_WRITE) != 0)
		{
			char *physmem = page_table_get_physmem(pt);
	   		//If the page is modified write contenets to the disk
	   		disk_write( disk, frame_mapping[head->frame], &physmem[page_frame*PAGE_SIZE] );
		}
		c = head;
		head=head->next;
		free(c);
	}
// free elements and other stuff
	if(frame_mapping)
		free(frame_mapping);
	if(elements)
		free(elements);
	
}



//Returns the first empty frame encountered
int get_empty_frame(int nframes)
{	
	int i=0;
	for(i=0;i<nframes;i++) {
		if(frame_mapping[i] == -1)
			break;		
	}
	if(i == nframes) i = -1;
	return i;
}



//based on locality.. explained in document
int customAlgorithm()
{
	node *c = head;
	if(c==NULL)
		return -1;
	mapping temp;
	int i=0,j=0,k=0;
	while(c!=NULL)
	{
		elements[i].frame=c->frame;
		elements[i].page = frame_mapping[c->frame];
		i++;
		c=c->next;
	}
	// sorting based on page number
	for(j=0;j<i;j++)
	{
		for(k=j+1;k<i;k++)
		{
			if(elements[j].page > elements[k].page)
			{
				temp.page = elements[j].page;
				temp.frame = elements[j].frame;

				elements[j].page=elements[k].page;
				elements[j].frame=elements[k].frame;

				elements[k].page = temp.page;
				elements[k].frame = temp.frame;
				
			}
		}
	}
	for(j=0;j<i;j++)
	{
		
		if(tail==NULL)
		{
			exit(0);
		}
		if(elements[j].frame == tail->frame)
			break;
	}
	// comparing distance between index
	if((j-0)>=(i-1-j))
	{
		deleteFrame(elements[0].frame);
		return elements[0].frame;
	}
	else
	{
		deleteFrame(elements[i-1].frame);
		return elements[i-1].frame;
	}
	
}

//Returns the frame that should be replaced according to different algorithms
int get_replacement_frame( struct page_table *pt , int nframes ) 
{
	if(!strcmp(pagefaulthandler,"rand"))
	{
		int frame = rand() % nframes;
		return frame;
	}
	if(!strcmp(pagefaulthandler,"fifo")) {
        	int frame = head->frame;
        	delete();
        	return frame;
        }
        if(!strcmp(pagefaulthandler,"custom")) {
		int frame = customAlgorithm();
        	return frame;
        }
	else {
		fprintf(stderr,"unknown replacement page algorithm: ");
		page_table_delete(pt);
		disk_close(disk);
		exit(1);
	}
	
}
void page_fault_handler( struct page_table *pt, int page)
{
        //Increment page fault counter
	page_fault_cnt++;	
	int page_frame; int page_bits;
	//check if there is a page_table entrty for the page
	page_table_get_entry(pt, page, &page_frame, &page_bits);
	//If entry is present change the permission bits
	if(page_bits != 0)
	{
		page_table_set_entry(pt,page,page_frame,PROT_READ|PROT_WRITE);
	} 
	else 
	{
	        char *physmem = page_table_get_physmem(pt);	
		int nframes = page_table_get_nframes(pt);
		//Get free fram if available
		int frame = get_empty_frame(nframes);
		if(frame == -1) {
		        //If no free frame is available get the frame to be replaced
			frame = get_replacement_frame(pt,nframes);
			//Get page entry for the frame entry to be replaced			
			int old_page = frame_mapping[frame];
			
			page_table_get_entry(pt, old_page, &page_frame, &page_bits);			
			if((page_bits & PROT_WRITE) != 0){
			        //If the page is modified write contenets to the disk
				disk_write( disk, old_page, &physmem[page_frame*PAGE_SIZE] );
				//Increment sick write count			
				disk_write_cnt++;
			}
			page_table_set_entry(pt,old_page,0,0);
		}
		//Add the frame to the list incase of fifo algorithm
		if(!strcmp(pagefaulthandler,"fifo") || !strcmp(pagefaulthandler,"custom")) 
		        add(frame);
		//Update frame_mapping list
		frame_mapping[frame] = page;
		
		page_table_set_entry(pt,page,frame,PROT_READ);
		//Read data from the disk and place in ohysical memory
		disk_read(disk,page,&physmem[frame*PAGE_SIZE]);
		//update disk read count	
		disk_read_cnt++;	
	}
	
}

int main( int argc, char *argv[] )
{
	srand ( time(NULL) );
	if(argc!=5) {
		printf("use: virtmem <npages> <nframes> <rand|fifo|custom> <sort|scan|focus>\n");
		return 1;
	}

	int npages = atoi(argv[1]);
	int nframes = atoi(argv[2]);
	const char *program = argv[4];
        pagefaulthandler = argv[3];
        
	disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}


	struct page_table *pt = page_table_create( npages, nframes, page_fault_handler );
	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

	char *virtmem = page_table_get_virtmem(pt);
	int i;	
	frame_mapping = (int *)malloc(sizeof(int)*nframes);
	elements = (mapping*)malloc(sizeof(mapping)*nframes);
	for(i=0;i<nframes;i++) 
		frame_mapping[i] = -1;


	if(!(!strcmp(pagefaulthandler,"rand")|| !strcmp(pagefaulthandler,"fifo") || !strcmp(pagefaulthandler,"custom"))) {
		fprintf(stderr,"unknown replacement algorithm: %s\n",argv[3]);
		exit(1);

	}



	if(!strcmp(program,"sort")) {
		sort_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"scan")) {
		scan_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"focus")) {
		focus_program(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[4]);

	}

	printf("disk read : %d\n",disk_read_cnt);
	printf("disk write : %d\n",disk_write_cnt);
	printf("page fault: %d\n",page_fault_cnt);

	//cleanup
	cleanup(pt);

	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
