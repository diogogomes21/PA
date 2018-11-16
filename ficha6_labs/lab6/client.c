/**
 * @file: client.c
 * @date: 2016-11-17
 * @author: autor
 */
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "memory.h"
#include "debug.h"
#include "common.h"
#include "client_opt.h"
#include "time.h"

#define C_ERR_WRONG_SERV_ADDR (4)

int main(int argc, char *argv[]){
    /* Estrutura gerada pelo utilitario gengetopt */
    struct gengetopt_args_info args_info;

    /* Processa os parametros da linha de comando */
    if (cmdline_parser (argc, argv, &args_info) != 0){
	    exit(ERR_ARGS);
    }

    int serv_port = args_info.port_arg;
    if (serv_port<=0 || serv_port > MAX_PORT) {
      fprintf(stderr, "Invalid port given: %d (Wanted:[1,%d])\n", serv_port, MAX_PORT);
      exit(C_ERR_INVALID_PORT);
    }

    //create socket
    int udp_server_socket;
    if ((udp_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
      ERROR(C_ERR_CANT_CREATE_SOCKET, "Can't create udp_server_socket (IPv4)");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;  	//AF_INET = PF_INET
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(serv_port);  	// Server port

    char *serv_IP = args_info.ip_arg;
    int ret_inet_pton = inet_pton(AF_INET, serv_IP, &(serv_addr.sin_addr.s_addr));
    if (ret_inet_pton < 1) {
      fprintf(stderr, "Wrong server address '%s'\n", serv_IP);
      exit(C_ERR_WRONG_SERV_ADDR);
    }

    //uint16_t random number: 1...100
    srand(time(NULL));
    uint16_t my_value = (uint16_t)((rand()%100)+1);
    printf("CLIENT: my_value=%d\n", my_value);
    my_value = htons(my_value);
    socklen_t serv_addr_len = sizeof(serv_addr);

    //sendto
    ssize_t ret_sendto = sendto(udp_server_socket, &my_value, sizeof(my_value), 0, (struct sockaddr *)&serv_addr, serv_addr_len);
    if (ret_sendto == -1) {
      fprintf(stderr, "ERROR: sendto (buff = %zu bytes): %s\n", sizeof(my_value), strerror(errno));
      return 1;
    }

    //recvfrom
    serv_addr_len = sizeof(serv_addr);
    char S[128];
    ssize_t ret_recvfrom = recvfrom(udp_server_socket, S, sizeof(S), 0, (struct sockaddr *)&serv_addr, &serv_addr_len);
    if (ret_recvfrom == -1) {
      fprintf(stderr, "ERROR: recvfrom (buff = %zu bytes): %s\n", sizeof(serv_addr), strerror(errno));
      return 1;
    }

    S[ret_recvfrom]='\0';
    printf("CLIENT: received from server '%s'\n", S);

    cmdline_parser_free(&args_info);

    return 0;
}
