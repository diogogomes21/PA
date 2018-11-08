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
#include <assert.h>
#include <arpa/inet.h>

#include "debug.h"
#include "memory.h"

void mostra_bytes(void *ptr, size_t len);

int main(int argc, char *argv[]){
	/* Disable warnings */
	(void)argc; (void)argv;

	struct sockaddr_in my_addr_IPv4;

  /* Mostrar tamanho (em bytes) do tipo struct sockaddr_in */

	printf("sizeof(sockaddr_in)=%zu B\n", sizeof(struct sockaddr_in));

  /* Colocar a "zero" uma variável (my_addr_IPv4) do tipo struct sockaddr_in */

	memset(&my_addr_IPv4,0,sizeof(my_addr_IPv4));

  /* Mostrar bytes da memória da variável my_addr_IPv4 (criar e chamar função)*/

	mostra_bytes(&my_addr_IPv4,sizeof(my_addr_IPv4));

  /* Colocar a "4" os primeiros 6 bytes da variável my_addr_IPv4 */

	memset(&my_addr_IPv4,4,6);

  /* Mostrar bytes da memória da variável my_addr_IPv4 (chamar função)*/

	printf("\n");
	mostra_bytes(&my_addr_IPv4,sizeof(my_addr_IPv4));

	return 0;
}

void mostra_bytes(void *ptr, size_t len){
	unsigned char *show_ptr = ptr;
	for (size_t i = 0; i < len; i++) {
		printf("[%02ld]:%x\n", i, show_ptr[i]);
	}
}
