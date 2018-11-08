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
#include <pthread.h>

#include "debug.h"
#include "memory.h"

#define C_ERRO_PTHREAD_CREATE 	1
#define C_ERRO_PTHREAD_JOIN 	2

void *speak(void *arg);

int main(int argc, char *argv[]){
	/* Disable warnings */
	(void)argc; (void)argv;

	pid_t pid;
	pthread_t tids[2];
	int i;

	/* Main code*/

	for (i = 0; i < 3; i++) {
		pid = fork ();
		if (pid==0) {
			/*Processos Filhos*/
			printf("=============================\n");
			printf("[%d] Processo (PID=%d)\n", i, getpid());
			for (i = 0; i < 2; i++) {
				if ((errno = pthread_create(&tids[i], NULL, speak, NULL)) != 0) {
					ERROR(C_ERRO_PTHREAD_CREATE, "pthread_create() failed!");
				}
		  }

			for (i = 0; i < 2; i++) {
				if ((errno = pthread_join(tids[i], NULL)) != 0) {
					ERROR(C_ERRO_PTHREAD_JOIN, "pthread_join() failed!\n");
				}
			}

		}else if (pid>0) {
			/*Processos Pai*/
			wait(NULL);
		}else{
			ERROR(1,"Cannot create Process");
		}
	}

	return 0;
}

void *speak(void *arg)
{
	(void)arg;
	printf("PID pai: %d, Meu PID: %d, TID: %lu\n", getppid(), getpid(), (unsigned long) pthread_self());
	return NULL;
}
