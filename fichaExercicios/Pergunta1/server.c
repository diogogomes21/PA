/**
 * @file: server.c
 * @date: 2016-11-17
 * @author: autor
 */
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include "debug.h"
#include "common.h"
#include "server_opt.h"

#define C_MAX_PORT ((1<<16)-1)
#define C_MAX_STATUS ((1<<16)-1)

int check_uint16(int int16, char*msg){
  if (int16 <= 0 || int16 > C_MAX_PORT) {
    fprintf(stderr, "[ERROR] Invalid %s '%d' [1,%d]\n", msg, int16, C_MAX_PORT);
    exit(EXIT_FAILURE);
  }
  return int16;
}

int show_status(uint16_t status);

int main(int argc, char *argv[]){
    /* Estrutura gerada pelo utilitario gengetopt */
    struct gengetopt_args_info args;

    /* Processa os parametros da linha de comando */
    if (cmdline_parser (argc, argv, &args) != 0){
        exit(ERR_ARGS);
    }

    // UDP IPv4: cria socket
  	int udp_server_socket = socket(AF_INET, SOCK_DGRAM, 0);
  	if ( udp_server_socket == -1){
      ERROR(EXIT_FAILURE, "Can't create udp_server_socket (IPv4)\n");
    }

    //Check port
    int my_port = check_uint16(args.port_arg, "port");
    //Chech status
    uint16_t status = (uint16_t) check_uint16(args.status_arg, "status");

    //DEBUG
    show_status(status);

  	// UDP IPv4: bind a IPv4/porto
  	struct sockaddr_in udp_server_endpoint;
  	memset(&udp_server_endpoint, 0, sizeof(struct sockaddr_in));
  	udp_server_endpoint.sin_family = AF_INET;
  	udp_server_endpoint.sin_addr.s_addr = htonl(INADDR_ANY);  	// Todas as interfaces de rede
  	udp_server_endpoint.sin_port = htons(my_port);	// Server port
    int ret_bind = bind(udp_server_socket, (struct sockaddr *) &udp_server_endpoint, sizeof(struct sockaddr_in));
  	if ( ret_bind == -1){
      ERROR(EXIT_FAILURE, "Can't bind @udp_server_endpoint info");
    }



    // UDP IPv4: fecha socket (server)
    if (close(udp_server_socket) == -1){
      ERROR(EXIT_FAILURE, "Can't close udp_client_socket (IPv4)");
    }
    cmdline_parser_free(&args);
    return 0;
}

int show_status(uint16_t status){
  char status_S[16][128];
  strcpy(status_S[0], "1 Portão da garagem (aberto / fechado)");
  strcpy(status_S[1], "2 Iluminação do hall de entrada");
  strcpy(status_S[2], "3 Iluminação sala");
  strcpy(status_S[3], "4 Iluminação jardim");
  strcpy(status_S[4], "5 Persiana 1");
  strcpy(status_S[5], "6 Persiana 2");
  strcpy(status_S[6], "7 Persiana 3");
  strcpy(status_S[7], "8 Piso radiante (ligado / desligado)");
  for (int i = 8; i < 16; i++) {
    strcpy(status_S[i],"");
  }

  int num_bits = sizeof(status)*8;
  for (int i = 0; i < num_bits; i++) {
    int mask = 1<<i;
    int bit = status & mask;
    if (strlen(status_S[i])>0) {
      printf("%s: %s\n", status_S[i],bit?"ON":"OFF");
    }
  }
  return 1;
}
