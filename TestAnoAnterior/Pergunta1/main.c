/**
* @file main.c
* @brief Description
* @date 2018-1-1
* @author name of author
*/

/*PARA ENVIAR UM SINAL PARA UM PROCESSO UAR O KILL => man 3 kill*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "debug.h"
#include "memory.h"
#include "args.h"

struct gengetopt_args_info args;

void trata_sinal(int signal);

void trata_sinal(int signal)
{
	/* Cópia da variável global errno */
	int aux = errno;

	if(signal==SIGUSR1){
		printf("[INFO] SIGUSR1 received. Executing command ‘%s’\n", args.execute_arg);
		int success=system(args.execute_arg);
		if (success!=0) {
			fprintf(stderr, "[ERROR] Failed execution: exiting!\n");
			exit(2);
		}
		printf("[WAITING for signal]\n");
	}

	if(signal==SIGINT){
		printf("[INFO] SIGINT received. Exiting!\n");
		exit(1);
	}

	/* Restaura valor da variável global errno */
	errno = aux;
}

int main(int argc, char *argv[]){
	/* Disable warnings */
	(void)argc; (void)argv;

	/*struct gengetopt_args_info args;*/
	if(cmdline_parser(argc, argv, &args))
		ERROR(1, "Erro: execução de cmdline_parser\n");

	struct sigaction act;

	act.sa_handler = trata_sinal;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if(sigaction(SIGUSR1, &act, NULL) < 0)
		ERROR(4, "sigaction (sa_sigaction) - SIGUSR1");
	if(sigaction(SIGINT, &act, NULL) < 0)
		ERROR(4, "sigaction (sa_sigaction) - SIGINT");

	printf("[PID:%d]: command to execute is '%s'\n", getpid(),args.execute_arg);
	printf("[INFO] kill -s SIGUSR1 %d (to execute the command)\n", getpid());
	printf("[INFO] kill -s SIGINT %d (to terminate)\n", getpid());

	while(1){
		pause();
	} /*PAUSA PROGRAMA*/

	cmdline_parser_free(&args);
	return 0;
}
