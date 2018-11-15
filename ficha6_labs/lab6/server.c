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
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>

#include "debug.h"
#include "common.h"
#include "server_opt.h"

#define C_CANT_BIND               (4)

int handle_client(int serv_sock);

int main(int argc, char *argv[]){
  // Estrutura gerada pelo utilitario gengetopt
  struct gengetopt_args_info args_info;

  // Processa os parametros da linha de comando
  if (cmdline_parser (argc, argv, &args_info) != 0){
    exit(ERR_ARGS);
  }

  int my_port = args_info.port_arg;
  if (my_port<=0 || my_port > MAX_PORT) {
    fprintf(stderr, "Invalid port given: %d (Wanted:[1,%d])\n", my_port, MAX_PORT);
    exit(C_ERR_INVALID_PORT);
  }

  // 1: create socket
  int udp_server_socket;
  if ((udp_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
    ERROR(C_ERR_CANT_CREATE_SOCKET, "Can't create udp_server_socket (IPv4)");
  }

  // 2: bind socket
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr.sin_family = AF_INET;  	//AF_INET = PF_INET
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  	// Todas as interfaces de rede
  serv_addr.sin_port = htons(my_port);  	// Server port
  printf("UDP Server / Port:%d\n", my_port);
  int ret_bind = bind(udp_server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (ret_bind == -1){
    fprintf(stderr, "ERROR: cannot bind at port: %d: %s\n", my_port, strerror(errno));
    exit(C_CANT_BIND);
  }

  // 3: loop:recvfrom/sendto
  while (1) {
    int ret = handle_client(udp_server_socket);
    if (ret == -1) {
      fprintf(stderr,"Problem with a client: continue\n");
    }
  }

  // 4: close socket
  if (close(udp_server_socket) == -1){
    ERROR(C_ERR_CANT_CLOSE_SOCKET, "Can't close udp_server_socket (IPv4)");
  }

  //Free args_info
  cmdline_parser_free(&args_info);

  return 0;
}

int handle_client(int serv_sock){
  uint16_t clnt_value;
  struct sockaddr_in clnt_addr;
  socklen_t clnt_addr_len;

  //recvfrom
  clnt_addr_len = sizeof(clnt_addr);
  printf("SERVER: waiting for client\n");
  ssize_t ret_recvfrom = recvfrom(serv_sock, &clnt_value, sizeof(clnt_value), 0, (struct sockaddr *)&clnt_addr, &clnt_addr_len);
  if (ret_recvfrom == -1) {
    fprintf(stderr, "ERROR: recvfrom (buff = %zu bytes): %s\n", sizeof(clnt_addr), strerror(errno));
    return -1;
  }

  //Calcular a sqrt
  uint16_t host_clnt_value = ntohs(clnt_value);
  double sqrt_clnt = sqrt(host_clnt_value);
  char S[128];
  snprintf(S, sizeof(S), "%lf", sqrt_clnt);
  printf("sqrt_clnt='%s'\n", S);

  //sendto
  ssize_t ret_sendto = sendto(serv_sock, S, strlen(S), 0, (struct sockaddr *)&clnt_addr, clnt_addr_len);
  if (ret_sendto == -1) {
    fprintf(stderr, "ERROR: sendto (buff = %zu bytes): %s\n", strlen(S), strerror(errno));
    return -1;
  }

  return 0;
}
