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

int main(int argc, char *argv[]){
	/* Disable warnings */
	(void)argc; (void)argv;

    /* a) Criar um socket UDP / IPv4 */
		int sock_udp_ipv4;

		sock_udp_ipv4 = socket(AF_INET,SOCK_DGRAM,0);
		if (sock_udp_ipv4==-1) {
			fprintf(stderr, "Cannot create socket IPv4 / UDP: %s\n", strerror(errno));
			exit(1);
		}

    /* b) Criar um socket UDP / IPv6 */
		int sock_udp_ipv6;

		sock_udp_ipv6 = socket(AF_INET,SOCK_DGRAM,0);
		if (sock_udp_ipv4==-1) {
			fprintf(stderr, "Cannot create socket IPv4 / UDP: %s\n", strerror(errno));
			exit(1);
		}

    /* c) Mostrar descritores dos sockets */

		printf("sock_udp_ipv4=%d\n", sock_udp_ipv4);
		printf("sock_udp_ipv6=%d\n", sock_udp_ipv6);

    /* d) Fechar os descritores dos sockets */

		close(sock_udp_ipv4);
		close(sock_udp_ipv6);

	return 0;
}
