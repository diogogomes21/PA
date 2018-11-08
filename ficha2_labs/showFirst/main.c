/**
* @file main.c
* @brief Description
* @date 2018-1-1
* @author name of author
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "debug.h"
#include "memory.h"

#include <signal.h>
#include "args.h"

char *G_filename = NULL;

void trata_sinal(int signal);

// zona das funções
void trata_sinal(int signal)
{
	int aux;
	aux = errno;   // Copia da variável global errno

	// código
	printf("Recebi o sinal (%d)\n", signal);
	printf("Ficheiro  processar: %s\n", G_filename);

	errno = aux;   // Restaura valor da variável global errno
}

int main(int argc, char *argv[]){

	struct gengetopt_args_info args_info;

    cmdline_parser (argc, argv, &args_info);
    G_filename = args_info.file_arg;


//======================================================
	/*Configure process to handle SIGUSR1*/
	struct sigaction act;

	act.sa_handler = trata_sinal; 	// Definir a rotina de resposta a sinais
	sigemptyset(&act.sa_mask);  	// mascara sem sinais -- nao bloqueia os sinais
	act.sa_flags = 0;
	act.sa_flags |= SA_RESTART; 	// recupera chamadas bloqueantes

	// Captura do sinal ???
	if(sigaction(SIGUSR1, &act, NULL) < 0)
		ERROR(2, "sigaction (sa_handler) - ???");
//======================================================

	pid_t pid=fork();

	if (pid==-1) {
		ERROR(1,"Cannot fork");
	}
	if (pid==0) {
		printf("Son process %d\n", getpid());
		while (1) {
			kill(getppid(),SIGUSR1);
			sleep(5);
		}
	}
	if (pid>0) {
		wait(NULL);
	}
	cmdline_parser_free(&args_info);
	return 0;
}
