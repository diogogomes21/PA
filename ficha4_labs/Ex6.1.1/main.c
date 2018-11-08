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

#define MAX_THREAD 200

void *task(void *arg);

typedef struct
{
	int contador;
	pthread_mutex_t *ptr_mutex;
}thread_params_t;

int main(int argc, char *argv[]){
	/* Disable warnings*/
	(void)argc; (void)argv;

	pthread_t tids[MAX_THREAD];
  thread_params_t thread_params;

	pthread_mutex_t mutex;
	thread_params.ptr_mutex = &mutex;

	thread_params.contador=0;

	for (int i = 0; i < MAX_THREAD; i++){
		if ((errno = pthread_mutex_init(&mutex, NULL)) != 0)
			ERROR(12, "pthread_mutex_init() failed");

		if ((errno = pthread_create(&tids[i], NULL, task, &thread_params)) != 0)
			ERROR(10, "Erro no pthread_create()!");
	}

	for (int i = 0; i < MAX_THREAD; i++){
		if ((errno = pthread_join(tids[i], NULL)) != 0)
			ERROR(11, "Erro no pthread_join()!\n");
	}

	if ((errno = pthread_mutex_destroy(&mutex)) != 0)
		ERROR(13, "pthread_mutex_destroy() failed");

	printf("Contador: %d\n", thread_params.contador);

	return 0;
}

void *task(void *arg)
{
	// cast para o tipo de dados enviado pela 'main thread'
	thread_params_t *params = (thread_params_t *) arg;

	// Mutex: entra na secção crítica
	if ((errno = pthread_mutex_lock(params->ptr_mutex)) != 0){
		WARNING("pthread_mutex_lock() failed");
		return NULL;
	}

	for (size_t i = 0; i < 1000; i++) {
		params->contador++;
	}

	// Mutex: sai da secção crítica
	if ((errno = pthread_mutex_unlock(params->ptr_mutex)) != 0){
		WARNING("pthread_mutex_unlock() failed");
		return NULL;
	}

	return NULL;
}
