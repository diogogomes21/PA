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
#include <sys/socket.h>
#include <arpa/inet.h>

#include "debug.h"
#include "memory.h"
#include "args.h"

#define C_ERR_SOCKET        (1)
#define C_ERR_BIND          (2)
#define C_ERR_INVALID_PORT  (3)

int main(int argc, char *argv[]){
    /* Receber porto a registar */
    struct gengetopt_args_info args;
    cmdline_parser(argc, argv, &args);

    if (args.port_arg<=0 || args.port_arg>=(1<<16)) {
      fprintf(stderr, "invalid port: %d (valid range:[1 - 65535])\n", args.port_arg);
      exit(C_ERR_INVALID_PORT);
    }

    /* Criar e registar socket UDP IPv4 */

    int sock_udp_ipv4;

		sock_udp_ipv4 = socket(AF_INET,SOCK_DGRAM,0);
		if (sock_udp_ipv4==-1) {
			fprintf(stderr, "Cannot create socket IPv4 / UDP: %s\n", strerror(errno));
			exit(C_ERR_SOCKET);
		}

    /* Registo local - bind */

    struct sockaddr_in addr_ipv4;
    memset(&addr_ipv4,0,sizeof(addr_ipv4));
    addr_ipv4.sin_family = AF_INET;
    addr_ipv4.sin_port = htons(args.port_arg);
    addr_ipv4.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret_bind = bind(sock_udp_ipv4, (struct sockaddr*)&addr_ipv4, sizeof(addr_ipv4));
    if (ret_bind == -1){
      fprintf(stderr, "Cannot bind: '%s'\n", strerror(errno));
      exit(C_ERR_BIND);
    }

    /* Liberta memória */
    cmdline_parser_free(&args);
    close(sock_udp_ipv4);

    return 0;
}

/*
 * Perguntas:
 *  a)  O que sucede quando é indicado um porto entre 1 e 1023 (inclusive)?
 *  b)  O que sucede quando se tenta registar um porto que já está registado?
 *      Nota: é possível obter uma lista dos portos UDP registados no sistema através do utilitário netstat, executado da seguinte forma: netstat -u -l
*/
