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
#include <errno.h>
#include <stdint.h>

#include "memory.h"
#include "debug.h"
#include "common.h"
#include "client_opt.h"

#define C_MAX_PORT ((1<<16)-1)

int check_port(int port){
  if (port <= 0 || port > C_MAX_PORT) {
    fprintf(stderr, "[ERROR] Invalid port '%d' [1,%d]\n", port,C_MAX_PORT);
    exit(EXIT_FAILURE);
  }
  return port;
}

int main(int argc, char *argv[]){
    char ip[256];
    int key;
    char mensagem[1024];

    /* Estrutura gerada pelo utilitario gengetopt */
    struct gengetopt_args_info args;

    /* Processa os parametros da linha de comando */
    if (cmdline_parser (argc, argv, &args) != 0){
	    exit(ERR_ARGS);
    }

    if (args.ip_given) {
      strcpy(ip, args.ip_arg);
    }else{
      strcpy(ip, "127.0.0.1");
    }

    int port = check_port(args.port_arg);

    if (args.key_given) {
      key = args.key_arg;
    }else{
      key = UINT8_MAX;
    }

    // TCP IPv4: cria socket
  	int tcp_client_socket;
  	if ((tcp_client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  		ERROR(41, "Can't create tcp_client_socket (IPv4)");

  	// TCP IPv4: connect ao IP/porto do servidor
  	struct sockaddr_in tcp_server_endpoint;
  	memset(&tcp_server_endpoint, 0, sizeof(struct sockaddr_in));
  	tcp_server_endpoint.sin_family = AF_INET;
  	switch (inet_pton(AF_INET, ip, &tcp_server_endpoint.sin_addr)) {
  		case 0:
  			fprintf(stderr, "[%s@%d] ERROR - Cannot convert IP address (IPv4): Invalid Network Address\n",
  				__FILE__, __LINE__);
  			exit(22);
  		case -1:
  			ERROR(22, "Cannot convert IP address (IPv4)");
  	}
  	tcp_server_endpoint.sin_port = htons(port);						// Server port

  	printf("a ligar ao servidor... "); fflush(stdout);
  	if (connect(tcp_client_socket, (struct sockaddr *) &tcp_server_endpoint, sizeof(struct sockaddr_in)) == -1)
  		ERROR(43, "Can't connect @tcp_server_endpoint");
  	printf("ok. \n");

  if (key==UINT8_MAX){ //cifrar
    // send cifrar request
  	int ret_send = send(tcp_client_socket, &key, sizeof(key), 0);
  	if (ret_send == -1) {
  		fprintf(stderr, "Cannot send to server: %s\n", strerror(errno));
  		exit(4);
  	}

    //Enviar mensagem
    ret_send = send(tcp_client_socket, args.message_arg, strlen(args.message_arg), 0);
    if (ret_send == -1) {
      fprintf(stderr, "Cannot send to server: %s\n", strerror(errno));
      exit(4);
    }

    //Recv key
    int ret_recv = recv(tcp_client_socket, &key, sizeof(key), 0);
  	if (ret_recv == -1) {
  		fprintf(stderr, "Cannot recv: %s\n", strerror(errno));
  		exit(5);
  	}else if(ret_recv == 0){
  		fprintf(stderr, "Server has closed connection :O\n");
  		close(tcp_client_socket);
  		exit(6);
  	}

    //Recv mensagem cifrada
    ret_recv = recv(tcp_client_socket, mensagem, sizeof(mensagem)-1, 0);
  	if (ret_recv == -1) {
  		fprintf(stderr, "Cannot recv: %s\n", strerror(errno));
  		exit(5);
  	}else if(ret_recv == 0){
  		fprintf(stderr, "Server has closed connection :O\n");
  		close(tcp_client_socket);
  		exit(6);
  	}
  	mensagem[ret_recv] = '\0';
    printf(">> KEY: %d\n", key);
  	printf(">> MESSAGE: %s \n", mensagem);
  }else{ //decifrar
    // send key
    int ret_send = send(tcp_client_socket, &key, sizeof(key), 0);
    if (ret_send == -1) {
      fprintf(stderr, "Cannot send to server: %s\n", strerror(errno));
      exit(4);
    }
    //send message
    ret_send = send(tcp_client_socket, args.message_arg, strlen(args.message_arg), 0);
    if (ret_send == -1) {
      fprintf(stderr, "Cannot send to server: %s\n", strerror(errno));
      exit(4);
    }

    //Recv mensagem decifrada
    int ret_recv = recv(tcp_client_socket, mensagem, sizeof(mensagem)-1, 0);
  	if (ret_recv == -1) {
  		fprintf(stderr, "Cannot recv: %s\n", strerror(errno));
  		exit(5);
  	}else if(ret_recv == 0){
  		fprintf(stderr, "Server has closed connection :O\n");
  		close(tcp_client_socket);
  		exit(6);
  	}
  	mensagem[ret_recv] = '\0';
    printf(">> MESSAGE: %s\n", mensagem);
  }

    close(tcp_client_socket);
    cmdline_parser_free(&args);

    return 0;
}
