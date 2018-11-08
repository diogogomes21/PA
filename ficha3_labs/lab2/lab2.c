#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include "debug.h"

int main (int argc, char *argv[])
{
	(void)argc;

	char command_S[256];

	printf("Nome do programa '%s'\n", argv[0]);
	printf("PID do processo atual: %d\n", getpid());

	snprintf(command_S,sizeof(command_S),"ps -eLf | grep -i %d | grep -v grep | wc -l", getpid());
	printf("Numero de threads do processo atual: ");
	fflush(stdout);
	system(command_S);

	exit(0);
}
