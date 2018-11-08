#include<sys/types.h>
#include<unistd.h>
#include<stdio.h>

int main (int argc, char *argv[])
{
	/* Silencia os warnings */
	(void)argc;
	(void)argv;

	printf ("O meu PID e':%d \n\n", getpid ());
	printf ("O PID do meu pai e':%d\n\n", getppid ());
	return 0;
}
