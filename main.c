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
int nframes;
// Variables para contar faltas de pag, acceso a disco, escritura en disco
int count_page_fault = 0;
int count_disk_read = 0;
int count_disk_write = 0;

//Variables para FIFO
int indice = 0;
int fifo_array = 0;


void page_fault_handler( struct page_table *pt, int page )
{

	char *physcal_mem = page_table_get_physmem(pt);

  if (strcmp(global_alg,"rand") == 0){
		printf("%s\n", "Algoritmo Random" );
		printf("Random: %d\n", randm );
	  //int number_frames = page_table_get_nframes(pt);
		page_table_set_entry(pt,page,randm,PROT_READ);
    disk_read(disk,page,&physcal_mem[randm]);
		frame_table[page] = randm;
		count_page_fault++;
		count_disk_read++;
		//printf("%d\n", page_table_get_bits(pt,page) );
		page_table_print(pt);

    // Caso donde se intenta escribir en una pagina con bit de lectura.
		if (page_table_get_bits(pt,page) == 1){ // 1 = Read
					page_table_set_entry(pt,page,randm,PROT_READ|PROT_WRITE);
					count_page_fault++;
		}

		for (size_t i = 0; i < nframes; i++) {  // Imprime tabla de marcos
			printf("%d\n",frame_table[i] );
    }

		page_table_print(pt);


			//page = 1;
			// Si la pagina elegida accede a un marco ya ocupado , hay que escribir en disco y hacer el swap
			 if (frame_table[randm] != -1) {
				 		disk_write(disk,frame_table[randm],&physcal_mem[randm]);
						disk_read(disk,page,&physcal_mem[randm]);
						page_table_set_entry(pt,page,frame_table[randm],PROT_READ);
						page_table_set_entry(pt,randm,0,0);
						count_page_fault++;
						count_disk_read++;
						count_disk_write++;
						printf("%s\n", "Ultimo caso");
						frame_table[randm] = -1;
						frame_table[1] = page;
				    page_table_print(pt);
			 }

    for (size_t i = 0; i < nframes; i++) { // Imprime tabla de marcos
			printf("%d\n",frame_table[i] );
    }
	}

	if (strcmp(global_alg,"fifo") == 0){
		printf("%s\n", "Algoritmo FIFO" );
		page_table_set_entry(pt,page,indice,PROT_READ);
    disk_read(disk,page,&physcal_mem[indice]);
		frame_table[page] = indice;
		page++;
		indice++;

		page_table_set_entry(pt,page,indice,PROT_READ);
    disk_read(disk,page,&physcal_mem[indice]);
		frame_table[page] = indice;
		page_table_print(pt);
		printf("NFrames: %d\n\n", nframes);
		indice++;
		for (size_t i = 0; i < 10; i++) {
			printf("%d\n",frame_table[i] );

		}
		if (page_table_get_bits(pt,page) == 1){ // 1 = Read
					printf("%s\n","Hola" );
					page_table_set_entry(pt,page,indice,PROT_READ|PROT_WRITE);
		}

		if (nframes == indice) {
			printf("%s\n", "hola");
			indice = 0;
		}

	}
	if (strcmp(global_alg,"custom") == 0){
		printf("%s\n", "Algoritmo Custom" );
    // Que primero revise los pares y despues los impares?
		// Esta el LRU , pero hay que revisar que frames son los menos usados , peluo
		// Que sea un numero randon al comienzo y de ahi avance y/o disminuya en 1.


	}
  free(frame_table);
	exit(1);

}

int main( int argc, char *argv[] )
{
	if(argc!=5) {
		printf("use: virtmem <npages> <nframes> <rand|fifo|custom> <sort|scan|focus>\n");
		return 1;
	}

	int npages = atoi(argv[1]);
	nframes = atoi(argv[2]);
	global_alg = argv[3];
	frame_table = malloc(nframes*sizeof(int)); // Se llena de 0 , lo cual no nos sirve por que hay marcos 0
	srand48(time(NULL));
  randm = lrand48()%nframes;

 // LLenamos la tabla de marcos con -1 para evitar confusion si un marco esta libre o esta siendo usado por el marco 0
	for (size_t i = 0; i < nframes; i++) {
		 frame_table[i] = -1;
	}



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
	printf("Faltas de pagina : %d\n", count_page_fault);
	printf("Lecturas a disco : %d\n", count_disk_read );
	printf("Escrituras en disco : %d\n", count_disk_write);

	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
