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

#define C_MAX_PORT ((1<<16)-1)

int check_port(int port){
  if (port <= 0 || port > C_MAX_PORT) {
    fprintf(stderr, "[ERROR] Invalid port '%d' [1,%d]\n", port, C_MAX_PORT);
    exit(EXIT_FAILURE);
  }
  return port;
}

int tiny_http(int sock);

int main(int argc, char *argv[]){
  /* Estrutura gerada pelo utilitario gengetopt */
  struct gengetopt_args_info args;

  /* Processa os parametros da linha de comando */
  if (cmdline_parser (argc, argv, &args) != 0){
    exit(ERR_ARGS);
  }

  /* Verifica porto */
  int remote_port = check_port(args.port_arg);

  // TCP IPv4: cria socket
  int tcp_client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (tcp_client_socket == -1){
    ERROR(EXIT_FAILURE, "Can't create tcp_client_socket (IPv4)\n");
  }

  // TCP IPv4: connect ao IP/porto do servidor
  struct sockaddr_in tcp_server_endpoint;
  memset(&tcp_server_endpoint, 0, sizeof(struct sockaddr_in));
  tcp_server_endpoint.sin_family = AF_INET;
  int inet_ret = inet_pton(AF_INET, args.ip_arg, &tcp_server_endpoint.sin_addr);  // Server IP
  if (inet_ret == -1) {
    ERROR(EXIT_FAILURE,"Invalid family '%s'\n");
  }else if(inet_ret == 0){
    fprintf(stderr, "[ERROR] Invalid IPv4 '%s'\n", args.ip_arg);
    exit(EXIT_FAILURE);
  }
  tcp_server_endpoint.sin_port = htons(remote_port);  // Server port

  printf("[CLIENT] Attempting to connect to server\n");
  int connect_ret = connect(tcp_client_socket, (struct sockaddr *) &tcp_server_endpoint, sizeof(struct sockaddr_in));
  if ( connect_ret == -1){
    ERROR(EXIT_FAILURE, "Can't connect @tcp_server_endpoint\n");
  }

  tiny_http(tcp_client_socket);

  /* Liberta tcp_client_socket */
  close(tcp_client_socket);
  /* Liberta args */
  cmdline_parser_free(&args);
  return 0;
}

int tiny_http(int sock){
  char buff[256];
  ssize_t tcp_read_bytes, tcp_sent_bytes;

  snprintf(buff, sizeof(buff), "GET / HTTP/1.1\r\nHost:www.google.pt\r\nUser-agent:ProgA\r\nConnection:close\r\n\r\n");

  tcp_sent_bytes = send(sock, buff, strlen(buff), 0);
  if (tcp_sent_bytes == -1){
    ERROR(EXIT_FAILURE, "Can't send to server: '%s'\n", buff);
  }
  printf("[CLIENT]Ok. (%zu bytes enviados)\n", tcp_sent_bytes);
  printf("[CLIENT] Waiting for server response\n");

  while (1) {
    tcp_read_bytes = recv(sock, buff, sizeof(buff)-1, 0);
  	if ( tcp_read_bytes == -1){
      ERROR(EXIT_FAILURE, "Cannot recv");
    }else if(tcp_read_bytes == 0){
      fprintf(stderr, "[CLIENT] Server has closed connection\n");
      return 1;
    }else{
    	printf("[CLIENT]Ok. (%zu bytes recebidos)\n", tcp_read_bytes);
      buff[tcp_read_bytes]='\0';
      printf("[CLIENT] '%s'\n", buff);
    }
  }
  return 1;
}
