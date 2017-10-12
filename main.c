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

const char *global_alg;
int *frame_table;
struct disk *disk;
int randm = 0;

void page_fault_handler( struct page_table *pt, int page )
{
	printf("page fault on page #%d\n",page);
	printf("%s\n", global_alg );

  if (strcmp(global_alg,"rand") == 0){
		printf("%s\n", "Algoritmo Random" );

		char *physcal_mem = page_table_get_physmem(pt);
		//int number_frames = page_table_get_npages(pt);

    disk_read(disk,page,&physcal_mem[randm]);
		page_table_set_entry(pt,page,randm,PROT_READ);

    for (size_t i = 0; i < 10; i++) {

			printf("%d\n", &physcal_mem[i] );

    }
		printf("%d\n", randm );




	}
	if (strcmp(global_alg,"fifo") == 0){

	}
	if (strcmp(global_alg,"custom") == 0){


	}

	exit(1);
	free(frame_table);
}

int main( int argc, char *argv[] )
{
	if(argc!=5) {
		printf("use: virtmem <npages> <nframes> <rand|fifo|custom> <sort|scan|focus>\n");
		return 1;
	}

	int npages = atoi(argv[1]);
	int nframes = atoi(argv[2]);
	global_alg = argv[3];
	frame_table = malloc(nframes*sizeof(int)); // Se llena de 0
	srand48(time(NULL));
  randm = lrand48()%nframes;



	const char *program = argv[4];

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

	char *physmem = page_table_get_physmem(pt);

	if(!strcmp(program,"sort")) {
		sort_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"scan")) {
		scan_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"focus")) {
		focus_program(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[4]);

	}

	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
