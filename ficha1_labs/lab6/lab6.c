#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "debug.h"

int main (int argc, char *argv[])
{
	pid_t pid1, pid2;

	/* Silencia os warnings */
	(void)argc;
	(void)argv;

	pid1 = fork ();
	if (pid1 == 0) {	/* Processo filho */
		printf ("Filho 1\n");
		sleep (6);
	} else if (pid1 > 0) {	/* Processo pai */
		printf ("Pai criou o filho 1\n");
		pid2 = fork ();
		if (pid2 == 0) {	/* Processo filho */
			printf ("Filho 2\n");
			sleep (2);
		} else if (pid2 > 0) {	/* Processo pai */
			printf ("Pai criou o filho 2\n");
			printf ("Pai 'a espera do filho 2\n");
			waitpid (pid2, NULL, 0);
			printf ("Filho 2 terminou\nPai 'a espera do filho 1\n");
			waitpid (pid1, NULL, 0);
			printf ("Filho 1 terminou\nPai acabou!!!\n");
		} else		/* < 0 - erro */
			ERROR (2, "Erro na execucao do fork()");
	} else			/* < 0 - erro */
		ERROR (1, "Erro na execucao do fork()");

	return 0;
}
