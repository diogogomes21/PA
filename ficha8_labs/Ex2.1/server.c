/**
 * @file: server.c
 * @date: 2019-01-03
 * @author: Diogo Marques
 */
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
/* According to POSIX.1-2001, POSIX.1-2008 */
#include <sys/select.h>
/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "common.h"
#include "server_opt.h"

#define C_PAR (0)
#define C_IMPAR (1)

#define MAX(x,y) (x)>(y)?(x):(y)

#define C_MAX_PORT ((1<<16)-1)

int check_port(int port){
  if (port <= 0 || port > C_MAX_PORT) {
    fprintf(stderr, "[ERROR] Invalid port '%d' [1,%d]\n", port,C_MAX_PORT);
    exit(EXIT_FAILURE);
  }
  return port;
}

int my_select(int tcp_sock, int udp_sock);
int process_udp(int udp_sock);
int process_tcp(int tcp_sock);
int process_string(char *buf_with_num,int num_type);

int main(int argc, char *argv[]){
  /* Estrutura gerada pelo utilitario gengetopt */
  struct gengetopt_args_info args;

  /* Processa os parametros da linha de comando */
  if (cmdline_parser (argc, argv, &args) != 0){
    exit(ERR_ARGS);
  }

//======================================================== SERVER CODE

  int port = check_port(args.port_arg); // Check Port

  //======================================================== TCP
  // TCP IPv4: cria socket
  int tcp_server_socket;
  if ((tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    ERROR(51, "Can't create tcp_server_socket (IPv4)");
  }

  // TCP IPv4: bind a IPv4/porto
  struct sockaddr_in tcp_server_endpoint;
  memset(&tcp_server_endpoint, 0, sizeof(struct sockaddr_in));
  tcp_server_endpoint.sin_family = AF_INET;
  tcp_server_endpoint.sin_addr.s_addr = htonl(INADDR_ANY); 		// Todas as interfaces de rede
  tcp_server_endpoint.sin_port = htons(port);
  if (bind(tcp_server_socket, (struct sockaddr *) &tcp_server_endpoint, sizeof(struct sockaddr_in)) == -1){
    ERROR(52, "Can't bind @tcp_server_endpoint");
  }

  // TCP IPv4: "listen" por clientes
	int tcp_max_simultaneous_clients = 1;
  if (listen(tcp_server_socket, tcp_max_simultaneous_clients)  == -1){
		ERROR(53, "Can't listen for %d clients", tcp_max_simultaneous_clients);
  }
  //======================================================== UDP
  // UDP IPv4: cria socket
  int udp_server_socket;
  if ((udp_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
    ERROR(31, "Can't create udp_server_socket (IPv4)");
  }

  // UDP IPv4: bind a IPv4/porto
  struct sockaddr_in udp_server_endpoint;
  memset(&udp_server_endpoint, 0, sizeof(struct sockaddr_in));
  udp_server_endpoint.sin_family = AF_INET;
  udp_server_endpoint.sin_addr.s_addr = htonl(INADDR_ANY);  	// Todas as interfaces de rede
  udp_server_endpoint.sin_port = htons(port);
  if (bind(udp_server_socket, (struct sockaddr *) &udp_server_endpoint, sizeof(struct sockaddr_in)) == -1){
    ERROR(32, "Can't bind @udp_server_endpoint info");
  }
  //========================================================

  my_select(tcp_server_socket, udp_server_socket);

//======================================================== SERVER CODE

  // TCP IPv4: fecha socket (server)
  if (close(tcp_server_socket) == -1){
    ERROR(56, "Can't close tcp_server_socket (IPv4)");
  }
  // liberta recurso: socket UDP IPv4
	if (close(udp_server_socket) == -1){
		ERROR(33, "Can't close udp_server_socket (IPv4)");
  }
  cmdline_parser_free(&args);
  return 0;
}

int my_select(int tcp_sock, int udp_sock){
  assert(tcp_sock > 0); // Abort if false
  assert(udp_sock > 0); // Abort if false
  fd_set read_set;
  int max_descriptor = MAX(tcp_sock, udp_sock);
  struct timeval timeout;
  while(1){
    //Config select
    FD_ZERO(&read_set);
    FD_SET(tcp_sock, &read_set);
    FD_SET(udp_sock, &read_set);
    //Call select
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    int ret_select = select(max_descriptor+1, &read_set, NULL, NULL, &timeout);
    if(ret_select == -1 ){
      ERROR(EXIT_FAILURE,"Cannot select");
    }
    if (ret_select == 0) {
      printf("Timeout detected. Continue\n");
      continue;
    }
    if (ret_select > 0) {
      if (FD_ISSET(udp_sock, &read_set)) {
        process_udp(udp_sock);
      }
      if (FD_ISSET(tcp_sock, &read_set)) {
        process_tcp(tcp_sock);
      }
      continue;
    }
  } // While(1)
}

int process_udp(int udp_sock){
  // UDP IPv4: variáveis auxiliares para sendto() / recvfrom()
	socklen_t udp_sock_len = sizeof(struct sockaddr_in);
	struct sockaddr_in udp_sock_endpoint;
	ssize_t udp_read_bytes, udp_sent_bytes;
  char buffer[256];

  // recvfrom
  udp_read_bytes = recvfrom(udp_sock, buffer, sizeof(buffer)-1, 0, (struct sockaddr *) &udp_sock_endpoint, &udp_sock_len);
	if (udp_read_bytes == -1){
		fprintf(stderr, "Can't recvfrom client:%s\n",strerror(errno));
    return -1;
  }
  buffer[udp_read_bytes] = '\0';

  int num_to_ret = process_string(buffer,C_IMPAR);

  snprintf(buffer, sizeof(buffer), "UDP:%d\n", num_to_ret);

	// sendto
  udp_sent_bytes = sendto(udp_sock, buffer, strlen(buffer), 0, (struct sockaddr *) &udp_sock_endpoint, udp_sock_len);
	if (udp_sent_bytes == -1){
    fprintf(stderr, "Can't sendto client:%s\n",strerror(errno));
    return -1;
  }
	printf("ok.  (%zd bytes sent)\n", udp_sent_bytes);
  return 0;
}

int process_tcp(int tcp_sock){
	ssize_t tcp_read_bytes, tcp_sent_bytes;
	char buffer[256];

  // accept
	char tcp_client_string_ip[20];
	struct sockaddr_in tcp_client_endpoint;
	socklen_t tcp_client_endpoint_length = sizeof(struct sockaddr_in);
	int tcp_client_socket;
  tcp_client_socket = accept(tcp_sock, (struct sockaddr *) &tcp_client_endpoint, &tcp_client_endpoint_length);
	if (tcp_client_socket == -1){
		ERROR(54, "Can't accept client");
  }
	printf("cliente Tcp: %s@%d\n", inet_ntop(AF_INET, &tcp_client_endpoint.sin_addr, tcp_client_string_ip, sizeof(tcp_client_string_ip)), htons(tcp_client_endpoint.sin_port));

	// recv
  tcp_read_bytes = recv(tcp_client_socket, buffer, sizeof(buffer), 0);
	if (tcp_read_bytes == -1){
    fprintf(stderr, "Can't recv client:%s\n",strerror(errno));
    return -1;
  }
  buffer[tcp_read_bytes] = '\0';

  int num_to_ret = process_string(buffer,C_PAR);

  snprintf(buffer, sizeof(buffer), "UDP:%d\n", num_to_ret);

	// send
  tcp_sent_bytes = send(tcp_client_socket, buffer, strlen(buffer), 0);
	if (tcp_sent_bytes == -1){
    fprintf(stderr, "Can't send client:%s\n",strerror(errno));
    return -1;
  }
  printf("ok.  (%zd bytes sent)\n", tcp_sent_bytes);

  // TCP IPv4: fecha socket (server)
  if (close(tcp_client_socket) == -1){
    ERROR(56, "Can't close tcp_client_socket (IPv4)");
  }
  return 0;
}

int process_string(char *buf_with_num, int num_type){
  assert(C_PAR == 0); // Abort if false
  assert(C_IMPAR == 1); // Abort if false

  int num = atoi(buf_with_num);
  if (num <= 0) {
    fprintf(stderr, "Invalid value from client: '%s'\n", buf_with_num);
    return -1;
  }

  int num_to_ret = rand() % num;

  if ((num_to_ret % 2) != num_type) {
    // Not the correct parity
    num_to_ret++;
  }
  return num_to_ret;
}
