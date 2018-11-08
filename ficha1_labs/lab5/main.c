#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "debug.h"

int main(int argc, char *argv[]){
	pid_t pid;
	pid_t pid_retorno;

	pid = fork();
	if (pid == 0)
	{	/* Processo filho */
		printf("Filho: %d\n", getpid());
	}
	else if (pid > 0)
	{	/* Processo pai */
		pid_retorno = wait(NULL);
		printf("Pai: Terminou o processo %d\n",pid_retorno);
	}
	else  /* < 0 -- erro */
	 ERROR(1, "Erro na execução do fork()");

	return 0;
}
