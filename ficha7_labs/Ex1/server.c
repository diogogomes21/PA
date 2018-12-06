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
#include <errno.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>

#include "debug.h"
#include "common.h"
#include "server_opt.h"

#define C_MAX_PORT ((1<<16)-1)
#define C_FIM "fim"

int check_port(int port){
  if (port <= 0 || port > C_MAX_PORT) {
    fprintf(stderr, "[ERROR] Invalid port '%d' [1,%d]\n", port,C_MAX_PORT);
    exit(EXIT_FAILURE);
  }
  return port;
}

int processa_cliente(int clnt_sock);

int main(int argc, char *argv[]){
  /* Estrutura gerada pelo utilitario gengetopt */
  struct gengetopt_args_info args;

  /* Processa os parametros da linha de comando */
  if (cmdline_parser (argc, argv, &args) != 0){
    exit(ERR_ARGS);
  }

  // Check Port
  int my_port = check_port(args.port_arg);

  // TCP IPv4: cria socket
  int tcp_server_socket;
  if ((tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    ERROR(EXIT_FAILURE, "Can't create tcp_server_socket (IPv4): %s\n", strerror(errno));
  }

  // TCP IPv4: bind a IPv4/porto
  struct sockaddr_in tcp_server_endpoint;
  memset(&tcp_server_endpoint, 0, sizeof(struct sockaddr_in));
  tcp_server_endpoint.sin_family = AF_INET;
  tcp_server_endpoint.sin_addr.s_addr = htonl(INADDR_ANY); 		// Todas as interfaces de rede
  tcp_server_endpoint.sin_port = htons(my_port);		// Server port
  int bind_ret = bind(tcp_server_socket, (struct sockaddr *) &tcp_server_endpoint, sizeof(struct sockaddr_in));
  if ( bind_ret == -1){
    ERROR(EXIT_FAILURE, "Can't bind @tcp_server_endpoint: %s\n", strerror(errno));
  }

  // TCP IPv4: "listen" por clientes
  int tcp_max_simultaneous_clients = 1;
  int listen_ret = listen(tcp_server_socket, tcp_max_simultaneous_clients);
  if ( listen_ret == -1){
    ERROR(EXIT_FAILURE, "Can't listen for %d clients: %s\n", tcp_max_simultaneous_clients, strerror(errno));
  }

  // TCP IPv4: "accept" 1 cliente (bloqueante)
  char tcp_client_string_ip[20];
  struct sockaddr_in tcp_client_endpoint;
  while(1){
    socklen_t tcp_client_endpoint_length = sizeof(struct sockaddr_in);
    printf("[SERVER] about to call accept (NEXT CLIENT)\n");
    int tcp_client_socket = accept(tcp_server_socket, (struct sockaddr *) &tcp_client_endpoint, &tcp_client_endpoint_length);
    if (tcp_client_socket == -1){
      if (errno == EINTR) {
        fprintf(stderr, "[SERVER] EINTR detected, Continuing ...\n");
        continue;
      }else{
        ERROR(EXIT_FAILURE, "[SERVER] Can't accept client\n");
      }
    }
    printf("ok. \n");
    inet_ntop(AF_INET, &tcp_client_endpoint.sin_addr, tcp_client_string_ip, sizeof(tcp_client_string_ip));
    printf("[SERVER] cliente: %s@%d\n", tcp_client_string_ip, htons(tcp_client_endpoint.sin_port));

    //Call processa_cliente
    processa_cliente(tcp_client_socket);
    close(tcp_client_socket);

  } //While(1)

  close(tcp_server_socket);
  cmdline_parser_free(&args);

  return 0;
}

int processa_cliente(int clnt_sock){
  ssize_t tcp_read_bytes, tcp_sent_bytes;
  char echo_buf[1024];
  printf("[SERVER] Waiting for client data\n");
  do {
    // TCP IPv4: "recv" do cliente (bloqueante)
    tcp_read_bytes = recv(clnt_sock, echo_buf, sizeof(echo_buf)-1, 0);
    if ( tcp_read_bytes == -1){
      fprintf(stderr, "[SERVER] Can't recieve from client: %s\n", strerror(errno));
      return -1;
    }
    if (tcp_read_bytes == 0) {
      fprintf(stderr, "[SERVER] connection closed by peer\n");
      return -1;
    }
    printf("[SERVER] OK (%zd bytes received)\n", tcp_read_bytes);

    //Add '\0' to end of string
    echo_buf[tcp_read_bytes-1]='\0';

    // TCP IPv4: "send" para o cliente
    printf("[SERVER] Sending data to client \n");
    tcp_sent_bytes = send(clnt_sock, echo_buf, strlen(echo_buf), 0);
    if ( tcp_sent_bytes == -1){
      fprintf(stderr, "[SERVER] Cannot send: %s\n", strerror(errno));
      return -1;
    }
    printf("[DEBUG SERVER] echo_buf '%s'\n", echo_buf);
    printf("[DEBUG SERVER] C_FIM '%s'\n", C_FIM);
  } while(strcasecmp(echo_buf, C_FIM)!=0);

  printf("[SERVER] OK (%zd bytes enviados)\n", tcp_sent_bytes);
  return 0;
}
