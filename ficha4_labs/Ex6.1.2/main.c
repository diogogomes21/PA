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

void *ping(void *arg);
void *pong(void *arg);

typedef struct
{
	int id;
	int parouimpar;
	int numerodeocurrencias;
	pthread_cond_t *ptr_cond;
	pthread_mutex_t *ptr_mutex;
}thread_params_t;

int main(int argc, char *argv[]){
	/* Disable warnings */
	(void)argc; (void)argv;

	pthread_t tid;
	pthread_t tid2;
	thread_params_t thread_params;
	pthread_mutex_t mutex;
	thread_params.ptr_mutex = &mutex;
	thread_params.parouimpar=0;
	thread_params.numerodeocurrencias=0;

	pthread_cond_t cond;
	thread_params.ptr_cond = &cond;

	// Mutex: inicializa o mutex antes de criar a(s) thread(s)
	if ((errno = pthread_mutex_init(&mutex, NULL)) != 0)
		ERROR(12, "pthread_mutex_init() failed");

	// Var.Condição: inicializa variável de condição
	if ((errno = pthread_cond_init(&cond, NULL)) != 0)
		ERROR(14, "pthread_cond_init() failed!");

	// cria thread 1 + passagem de parâmetro
	if ((errno = pthread_create(&tid, NULL, ping, &thread_params) != 0))
		ERROR(10, "Erro no pthread_create()!");

	// cria uma thread 2 + passagem de parâmetro
	if ((errno = pthread_create(&tid2, NULL, pong, &thread_params) != 0))
		ERROR(10, "Erro no pthread_create()!");

	// espera a thread 1 termine
	if ((errno = pthread_join(tid, NULL)) != 0)
		ERROR(11, "Erro no pthread_join()!\n");

	// espera a thread 2 termine
	if ((errno = pthread_join(tid2, NULL)) != 0)
		ERROR(11, "Erro no pthread_join()!\n");

	// Var.Condição: destroi a variável de condição
	if ((errno = pthread_cond_destroy(&cond)) != 0)
		ERROR(15,"pthread_cond_destroy failed!");

	// Mutex: liberta recurso
	if ((errno = pthread_mutex_destroy(&mutex)) != 0)
		ERROR(13, "pthread_mutex_destroy() failed");

	printf("Numero de ocurrencias: %d\n", thread_params.numerodeocurrencias);
	return 0;
}

void *ping(void *arg)
{
	// cast para o tipo de dados enviado pela 'main thread'
	thread_params_t *params = (thread_params_t *) arg;

	while (params->numerodeocurrencias<10){
		// Var.Condição: bloqueia a thread usando a variável de condição (em conjunto com o mutex)
		while ((params->parouimpar%2)!=0){
			if ((errno = pthread_cond_wait(params->ptr_cond, params->ptr_mutex)!=0)){
				WARNING("pthread_cond_wait() failed");
				return NULL;
			}
		}

		printf("ping...");
		params->parouimpar++;

		if ((errno = pthread_cond_signal(params->ptr_cond)) != 0){
			WARNING("pthread_cond_signal() failed");
			return NULL;
		}
		sleep(0.5);
	}
	return NULL;
}

void *pong(void *arg)
{
	// cast para o tipo de dados enviado pela 'main thread'
	thread_params_t *params = (thread_params_t *) arg;

	while (params->numerodeocurrencias<10){
		// Var.Condição: bloqueia a thread usando a variável de condição (em conjunto com o mutex)
		while ((params->parouimpar%2)==0){
			if ((errno = pthread_cond_wait(params->ptr_cond, params->ptr_mutex)!=0)){
				WARNING("pthread_cond_wait() failed");
				return NULL;
			}
		}

		printf("pong!\n");
		params->parouimpar++;
		params->numerodeocurrencias++;

		if ((errno = pthread_cond_signal(params->ptr_cond)) != 0){
			WARNING("pthread_cond_signal() failed");
			return NULL;
		}/*
		sleep(0.5);*/
	}
	return NULL;
}
