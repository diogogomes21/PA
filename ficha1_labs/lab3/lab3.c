#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "debug.h"

int main (int argc, char *argv[])
{
	int i;
	pid_t pid;
	
	/* Silencia os warnings */
	(void)argc;
	(void)argv;

	printf("PID=%d (%d)\n", getpid(), getppid());
	for (i = 0; i < 3; i++) {
		pid = fork ();
		if (pid == -1)
			ERROR (1, "Erro na execucao do fork");
		else
			printf ("Processo %d (%d)\n", getpid (), getppid ());
	}

	return 0;
}
