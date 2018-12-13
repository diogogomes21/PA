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

#include "memory.h"
#include "debug.h"
#include "common.h"
#include "client_opt.h"

int main(int argc, char *argv[]){
  /* Estrutura gerada pelo utilitario gengetopt */
  struct gengetopt_args_info args;

  /* Processa os parametros da linha de comando */
  if (cmdline_parser (argc, argv, &args) != 0){
    exit(ERR_ARGS);
  }

  // UDP IPv4: cria socket
  int udp_client_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_client_socket == -1){
    ERROR(EXIT_FAILURE, "Can't create udp_client_socket (IPv4)\n");
  }

  struct timeval timeout;
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;
  int ret = setsockopt(udp_client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  if (ret == -1) {
    ERROR(EXIT_FAILURE,"Cannot setsockopt");
  }

  // UDP IPv4: informação do servidor UDP
  socklen_t udp_server_endpoint_length = sizeof(struct sockaddr_in);
  struct sockaddr_in udp_server_endpoint;
  memset(&udp_server_endpoint, 0, sizeof(struct sockaddr_in));
  udp_server_endpoint.sin_family = AF_INET;
  int ret_inet = inet_pton(AF_INET, args.ip_arg, &udp_server_endpoint.sin_addr.s_addr);
  if (ret_inet == -1) {
    ERROR(EXIT_FAILURE,"Invalid family '%s'\n");
  }else if(ret_inet == 0){
    fprintf(stderr, "[ERROR] Invalid IPv4 '%s'\n", args.ip_arg);
    exit(EXIT_FAILURE);
  }
  udp_server_endpoint.sin_port = htons(args.port_arg);

  uint16_t request;
  request=0x00;

  ssize_t udp_sent_bytes = sendto(udp_client_socket, &request, sizeof(request), 0, (struct sockaddr *) &udp_server_endpoint, udp_server_endpoint_length);
	if (udp_sent_bytes == -1){
		ERROR(EXIT_FAILURE, "Can't sendto server\n");
  }
	printf("ok.  (%d bytes enviados)\n", (int)udp_sent_bytes);

  uint16_t response;
  ssize_t udp_read_bytes = recvfrom(udp_client_socket, &response, sizeof(response), 0, (struct sockaddr *) &udp_server_endpoint, &udp_server_endpoint_length);
	if ( udp_read_bytes == -1){
		ERROR(EXIT_FAILURE, "Can't recvfrom server\n");
  }
	printf("ok.  (%d bytes recebidos)\n", (int)udp_read_bytes);

  // UDP IPv4: fecha socket (client)
  if (close(udp_client_socket) == -1){
    ERROR(EXIT_FAILURE, "Can't close udp_client_socket (IPv4)\n");
  }
  cmdline_parser_free(&args);
  return 0;
}
