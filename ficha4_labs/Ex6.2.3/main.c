/**
* @file main.c
* @brief Description
* @date 2018-1-1
* @author name of author
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "args.h"

#include "debug.h"
#include "memory.h"

struct gengetopt_args_info args;

#define ERRO_CMDLINE_PARSER 1
#define CANT_STAT 2
#define NUM_LETTERS 26

typedef struct
{
	int id;
	char *filename;
	int filesize;
	int blocksize;
	int nthreads;
	size_t my_histo[NUM_LETTERS];
}thread_params_t;

void *lerficheiro(void *arg);
int get_file_size(char *filename);

int main(int argc, char *argv[]){
	/* Disable warnings */
	(void)argc; (void)argv;

	// gengetopt parser
	if(cmdline_parser(argc, argv, &args)){
		fprintf(stderr,"Erro: execução de cmdline_parser\n");
		exit(ERRO_CMDLINE_PARSER);
	}

	printf("\nFile: %s\n", args.file_arg);
	printf("Numero threads: %d\n", args.num_threads_arg);
	printf("Block Size: %d\n\n", args.size_arg);

	size_t tid_size = args.num_threads_arg*sizeof(pthread_t);
	pthread_t *tids_ptr=malloc(tid_size);
	size_t num_bytes = args.num_threads_arg*sizeof(thread_params_t);
	thread_params_t *thread_params_ptr = malloc(num_bytes);

	// Inicialização das estruturas - para cada thread
	for (int i = 0; i < args.num_threads_arg; i++){
		thread_params_ptr[i].id = i + 1;
		thread_params_ptr[i].filename = args.file_arg;
		thread_params_ptr[i].blocksize = args.size_arg;
		thread_params_ptr[i].filesize = get_file_size(args.file_arg);
		thread_params_ptr[i].nthreads = args.num_threads_arg;
		/*for (size_t j = 0; j < NUM_LETTERS; j++) {
			thread_params_ptr[i].my_histo[j]=0;
		} OU */
		memset(thread_params_ptr[i].my_histo, 0, NUM_LETTERS*sizeof(size_t));
	}

	// Criação das threads + passagem de parâmetro
	for (int i = 0; i < args.num_threads_arg; i++){
		if ((errno = pthread_create(&tids_ptr[i], NULL, lerficheiro, &thread_params_ptr[i])) != 0)
			ERROR(10, "Erro no pthread_create()!");
	}

	// Espera que todas as threads terminem
	for (int i = 0; i < args.num_threads_arg; i++){
		if ((errno = pthread_join(tids_ptr[i], NULL)) != 0)
			ERROR(11, "Erro no pthread_join()!\n");
	}

	free(thread_params_ptr);
	free(tids_ptr);

	// gengetopt: libertar recurso (assim que possível)
	cmdline_parser_free(&args);

	return 0;
}

void *lerficheiro(void *arg)
{
	// cast para o tipo de dados enviado pela 'main thread'
	thread_params_t *params = (thread_params_t *) arg;

	// para debug :: (apagar se não for necessário)
	printf("Teste: id = %d | TID = %lu\n", params->id, pthread_self());

	return NULL;
}

int get_file_size(char *filename){
	struct stat statbuf;
	int ret_stat = stat(filename,&statbuf);
	if (ret_stat!=0) {
		ERROR(CANT_STAT,"Cannot stat file '%s'",filename);
		return -1;
	}
	return statbuf.st_size;
}
